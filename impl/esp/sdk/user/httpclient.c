/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Martin d'Allens <martin.dallens@gmail.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

// FIXME: sprintf->snprintf everywhere.
// FIXME: support null characters in responses.

#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "httpclient.h"
#include "espmissingincludes.h"
#include "serial.h"


// serial_debug output.
#if 0
#define serial_debug(...) serial_debug(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// Internal state.
typedef struct {
	char * path;
	int port;
	char * post_data;
	char * hostname;
	char * buffer;
	int buffer_size;
	http_callback user_callback;
} request_args;

char print_buffer[2048];

static char * esp_strdup(const char * str)
{
	if (str == NULL) {
		return NULL;
	}
	char * new_str = (char *)os_malloc(os_strlen(str) + 1); // 1 for null character
	if (new_str == NULL) {
		serial_debug("esp_strdup: malloc error");
		return NULL;
	}
	os_strcpy(new_str, str);
	return new_str;
}

static void ICACHE_FLASH_ATTR receive_callback(void * arg, char * buf, unsigned short len)
{
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	if (req->buffer == NULL) {
		return;
	}

	// Let's do the equivalent of a realloc().
	const int new_size = req->buffer_size + len;
	char * new_buffer;
	if (new_size > BUFFER_SIZE_MAX || NULL == (new_buffer = (char *)os_malloc(new_size))) {
		os_sprintf(print_buffer, "Response too long (%d)", new_size);
		serial_debug(print_buffer);
		req->buffer[0] = '\0'; // Discard the buffer to avoid using an incomplete response.
		espconn_disconnect(conn);
		return; // The disconnect callback will be called.
	}

	os_memcpy(new_buffer, req->buffer, req->buffer_size);
	os_memcpy(new_buffer + req->buffer_size - 1 /*overwrite the null character*/, buf, len); // Append new data.
	new_buffer[new_size - 1] = '\0'; // Make sure there is an end of string.

	os_free(req->buffer);
	req->buffer = new_buffer;
	req->buffer_size = new_size;
}

static void ICACHE_FLASH_ATTR sent_callback(void * arg)
{
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	if (req->post_data == NULL) {
		serial_debug("All sent");
	}
	else {
		// The headers were sent, now send the contents.
		serial_debug("Sending request body");
		espconn_sent(conn, (uint8_t *)req->post_data, strlen(req->post_data));
		os_free(req->post_data);
		req->post_data = NULL;
	}
}

static void ICACHE_FLASH_ATTR connect_callback(void * arg)
{
	serial_debug("Connected");
	struct espconn * conn = (struct espconn *)arg;
	request_args * req = (request_args *)conn->reverse;

	espconn_regist_recvcb(conn, receive_callback);
	espconn_regist_sentcb(conn, sent_callback);

	const char * method = "GET";
	char post_headers[128] = "";

	if (req->post_data != NULL) { // If there is data this is a POST request.
		method = "POST";
		os_sprintf(post_headers,
				   "Content-Type: application/x-www-form-urlencoded\r"
				   "Content-Length: %d\r", strlen(req->post_data));
	}

	char buf[2048];
	int len = os_sprintf(buf,
						 "%s %s HTTP/1.1\r\n"
						 "Host: %s:%d\r\n"
						 "Connection: close\r\n"
						 "User-Agent: ESP8266\r\n"
						 "%s"
						 "\r\n",
						 method, req->path, req->hostname, req->port, post_headers);

	serial_debug(buf);
	espconn_sent(conn, (uint8_t *)buf, len);
	serial_debug("Sending request header");
}

static void ICACHE_FLASH_ATTR disconnect_callback(void * arg)
{
	serial_debug("Disconnected");
	struct espconn *conn = (struct espconn *)arg;

	if(conn == NULL) {
		return;
	}

	if(conn->proto.tcp != NULL) {
		os_free(conn->proto.tcp);
	}
	if(conn->reverse != NULL) {
		request_args * req = (request_args *)conn->reverse;
		int http_status = -1;
		char * body = "";
		if (req->buffer == NULL) {
			serial_debug("Buffer shouldn't be NULL");
		}
		else if (req->buffer[0] != '\0') {
			// FIXME: make sure this is not a partial response, using the Content-Length header.

			const char * version = "HTTP/1.1 ";
			if (os_strncmp(req->buffer, version, strlen(version)) != 0) {
				os_sprintf(print_buffer, "Invalid version in %s", req->buffer);
				serial_debug(print_buffer);
			}
			else {
				http_status = atoi(req->buffer + strlen(version));
				body = (char *)os_strstr(req->buffer, "\r\r") + 4;
			}
		}

		if (req->user_callback != NULL) { // Callback is optional.
			req->user_callback(body, http_status, req->buffer);
		}

		os_free(req->buffer);
		os_free(req->hostname);
		os_free(req->path);
		os_free(req);
	}
	os_free(conn);
}

static void ICACHE_FLASH_ATTR error_callback(void *arg, sint8 errType)
{
	serial_debug("Disconnected with error");
	disconnect_callback(arg);
}

static void ICACHE_FLASH_ATTR dns_callback(const char * hostname, ip_addr_t * addr, void * arg)
{
	request_args * req = (request_args *)arg;

	if (addr == NULL) {
		os_sprintf(print_buffer, "DNS failed for %s", hostname);
		serial_debug(print_buffer);
		if (req->user_callback != NULL) {
			req->user_callback("", -1, "");
		}
		os_free(req);
	}
	else {
		serial_debug("DNS found");

		struct espconn * conn = (struct espconn *)os_malloc(sizeof(struct espconn));
		conn->type = ESPCONN_TCP;
		conn->state = ESPCONN_NONE;
		conn->proto.tcp = (esp_tcp *)os_malloc(sizeof(esp_tcp));
		conn->proto.tcp->local_port = espconn_port();
		conn->proto.tcp->remote_port = req->port;
		conn->reverse = req;

		os_memcpy(conn->proto.tcp->remote_ip, addr, 4);

		espconn_regist_connectcb(conn, connect_callback);
		espconn_regist_disconcb(conn, disconnect_callback);
		espconn_regist_reconcb(conn, error_callback);

		if(espconn_connect(conn) != 0) {
			serial_debug("non successfull");
		}
	}
}

void ICACHE_FLASH_ATTR http_raw_request(
		const char * hostname, ip_addr_t* ip_addr, int port, const char * path, const char * post_data,
		http_callback user_callback)
{
	serial_debug("in http_raw_request");

	request_args * req = (request_args *)os_malloc(sizeof(request_args));
	req->hostname = esp_strdup(hostname);
	req->path = esp_strdup(path);
	req->port = port;
	req->post_data = esp_strdup(post_data);
	req->buffer_size = 1;
	req->buffer = (char *)os_malloc(1);
	req->buffer[0] = '\0'; // Empty string.
	req->user_callback = user_callback;

	dns_callback(hostname, ip_addr, req);
}

/*
 * Parse an URL of the form http://host:port/path
 * <host> can be a hostname or an IP address
 * <port> is optional
 */
void ICACHE_FLASH_ATTR http_post(const char * url, const char * post_data,
								 ip_addr_t* ip_addr, http_callback user_callback)
{
	// FIXME: handle HTTP auth with http://user:pass@host/
	// FIXME: make https work.
	// FIXME: get rid of the #anchor part if present.

	char hostname[128] = "";
	int port = 80;

	if (os_strncmp(url, "http://", strlen("http://")) != 0) {
		os_sprintf(print_buffer, "URL is not HTTP %s", url);
		serial_debug(print_buffer);
		return;
	}
	url += strlen("http://"); // Get rid of the protocol.

	char * path = os_strchr(url, '/');
	if (path == NULL) {
		path = os_strchr(url, '\0'); // Pointer to end of string.
	}

	char * colon = os_strchr(url, ':');
	if (colon > path) {
		colon = NULL; // Limit the search to characters before the path.
	}

	if (colon == NULL) { // The port is not present.
		os_memcpy(hostname, url, path - url);
		hostname[path - url] = '\0';
	}
	else {
		port = atoi(colon + 1);
		if (port == 0) {
			serial_debug("Port error");
			return;
		}

		os_memcpy(hostname, url, colon - url);
		hostname[colon - url] = '\0';
	}


	if (path[0] == '\0') { // Empty path is not allowed.
		path = "/";
	}

	os_sprintf(print_buffer, "hostname=%s", hostname);
	serial_debug(print_buffer);
	os_sprintf(print_buffer, "port=%d", port);
	serial_debug(print_buffer);
	os_sprintf(print_buffer, "path=%s", path);
	serial_debug(print_buffer);
	http_raw_request(hostname, ip_addr, port, path, post_data, user_callback);
}

void ICACHE_FLASH_ATTR http_get(const char * url, ip_addr_t* ip_addr,
								http_callback user_callback)
{
	http_post(url, NULL, ip_addr, user_callback);
}

void ICACHE_FLASH_ATTR http_callback_example(char * response, int http_status, char * full_response)
{
	os_sprintf(print_buffer, "http_status=%d", http_status);
	serial_debug(print_buffer);
	if (http_status != HTTP_STATUS_GENERIC_ERROR) {
		os_sprintf(print_buffer, "strlen(full_response)=%d\n", strlen(full_response));
		serial_debug(print_buffer);
		os_sprintf(print_buffer, "response=%s<EOF>\n", full_response);
		serial_debug(print_buffer);
	}
}
