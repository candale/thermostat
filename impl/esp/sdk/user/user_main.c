#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "serial.h"
#include "httpclient.h"


/**********************************************/
/* *************** SERVER AUTH ****************/
#define SERVER_AUTH_KEY "CUTRB50CQ2REU78M"


/*****************************************/
/* *************** METHODS ***************/
#define GET 1
#define POST 2
#define PUT 3
#define DELETE 4
#define SERIAL 5


/************************************************/
/* *************** HTTP CONSTANTS ***************/
#define WIFI_SERVER_LISTEN_PORT 80
#define URLS_MAX_REGISTER_NUMBER 10
#define REGISTERED_URL_MAX_LENGTH 35
#define MAX_HTPP_PARAM_NUMBER 10
#define MAX_HTPP_PARAM_NAME_LENGTH 30
#define MAX_HTPP_PARAM_VALUE_LENGTH 30


/************************************************************/
/* *************** VARIABLE RELATED CONSTANTS ***************/
// maximum absolute difference between last recorded variable and current one
#define MAX_VAR_DIFF 60
#define MAX_VAR 0xFFFFFFFF

/*****************************************************/
/* *************** TIME SYNC CONSTANTS ***************/
// 30 seconds, in microseconds
#define PUBLISH_REPEAT_INTERVAL 20 * 1000


/******************************************************/
/* *************** ATTINY COMM REQUESTS ***************/
// 30 seconds, in microseconds
char REQUEST_ATTINTY_TEMP[] = "ASK";
#define MAX_SERIAL_PARAM_NUMBER 4


/***************************************/
/* *************** PAGES ***************/
#define PAGE_404 "<html>"\
                 "<h2>404 - Not Found</h2>"\
                 "</html>"
char WIFI_FORM[] = "<html>"
                  "%s"
                  "Connected to acess point status: %s. Ip address: %s"
                  "<form method='post' action='/wifi_setup'>"
                  "<label>SSID</label>"
                  "<input type='text' name='ssid'>"
                  "<label>Password</label>"
                  "<input type='password' name='pass'>"
                  "<input type='submit' value='Connect'>"
                  "</form>"
                  "</html>";

char RELOAD_HTML[] = "<meta http-equiv='refresh' content='5'>";

// station stats
char STA_IDLE[] = "not connected";
char STA_CONNECTING[] = "connecting";
char STA_WRONG_PASSWORD[] = "wrong password";
char STA_NO_AP_FOUND[] = "no access point found";
char STA_CONNECT_FAIL[] = "failed to connect";
char STA_GOT_IP[] = "connected";
char STA_ERROR[] = "error";


/***************************************************/
/* *************** TASKS DEFINITIONS ***************/
#define user_loop_prority        1
#define user_publish_piority     0
#define user_proc_queue_length   3
os_event_t user_proc_task_queue[user_proc_queue_length];
static void loop(os_event_t *events);


/*******************************************************/
/* *************** STRUCTURE DEFINITIONS ***************/
// process_page callback function
typedef void (* process_request_func)(
    struct espconn * conn, char* data, unsigned short data_length);
// table that holds a mapping from url to processing function
struct url_mapping_t {
    struct data_holder {
        char* url;
        process_request_func func;
    } data[URLS_MAX_REGISTER_NUMBER];
    uint8 next_empty;
} request_handlers;

// structure that holds the parameters from each request
struct request_params_t {
    struct params_dynamic {
        char* key;
        char* value;
    } get[MAX_HTPP_PARAM_NUMBER], post[MAX_HTPP_PARAM_NUMBER];
    uint8 post_length;
    uint8 get_length;
} request_params;


// structure that holds the parameters from each request
struct serial_params_t {
    struct params_static {
        char key[20];
        char value[30];
    } serial[MAX_SERIAL_PARAM_NUMBER];
    uint8 serial_length;
} serial_params;


struct time_keeping {
    uint32 last_publish_call;
} time_struct;


/**************************************************/
/* *************** GLOBAL VARIABLES ***************/
char print_buffer[500];
// wifi server
struct espconn server;
// timer structure to read and send data at given interval
static volatile os_timer_t read_publish_timer;
// variables that hold the information
float temperature = MAX_VAR;
float humidity = MAX_VAR;


/******************************* PROGRAM *******************************/


/*****************************************************************************/
/************************** INIT OF PROGRAM STRUCTURES************************/
/*****************************************************************************/

void clean_request_params() {
    uint8 index;
    for(index = 0; index < request_params.get_length; index++) {
        os_free(request_params.get[index].key);
        os_free(request_params.get[index].value);
        request_params.get[index].key = 0;
        request_params.get[index].value = 0;
    }
    for(index = 0; index < request_params.post_length; index++) {
        os_free(request_params.get[index].key);
        os_free(request_params.get[index].value);
        request_params.get[index].key = 0;
        request_params.get[index].value = 0;
    }
    request_params.post_length = 0;
    request_params.get_length = 0;
}

void reset_serial_params() {
    serial_params.serial_length = 0;
}


void init_program_data() {
    request_handlers.next_empty = 0;
    request_params.post_length = 0;
    request_params.get_length = 0;
    serial_params.serial_length = 0;
    time_struct.last_publish_call = 0;
}


/*****************************************************************************/
/******************************** HELPERS ************************************/
/*****************************************************************************/


uint8 nstrcmp(char* first, char* second, unsigned short length) {
    if(os_strlen(first) < length || os_strlen(second) < length) {
        return -2;
    }
    unsigned short index = 0;
    while(index < length) {
        if(first[index] > second[index]) {
            return 1;
        } else if(first[index] < second[index]) {
            return -1;
        }
        index++;
    }

    return 0;
}

float os_atof(const char* s)
{
    float rez = 0, fact = 1;
    while (*s && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n'))
        s++;
    if (*s == 0)
        return 0; // can't read

    if (*s == '-')
    {
        s++;
        fact = -1;
    };
    int point_seen;
    for (point_seen = 0; *s; s++)
    {
        if (*s == '.')
        {
            point_seen = 1;
            continue;
        };
        int d = *s - '0';
        if (d >= 0 && d <= 9)
        {
            if (point_seen)
                fact /= 10.0;
            rez = rez * 10.0f + (float) d;
        };
    };
    return rez * fact;
}


/*****************************************************************************/
/*************************** STAION AND AP SETUP *****************************/
/*****************************************************************************/

void ICACHE_FLASH_ATTR
setup_station(char* ssid, char* password) {
    struct station_config stationConf;

    os_memcpy(stationConf.ssid, ssid, 32);
    os_memcpy(stationConf.password, password, 32);
    stationConf.bssid_set = 0;
    if(!wifi_station_set_config(&stationConf)) {
        serial_println("Error setting station config");
    } else {
        serial_println("Station config set");
    }
    if(!wifi_station_connect()) {
        serial_println("Error while connecting station");
    } else {
        serial_println("Station connected");
    }

    if(!wifi_station_dhcpc_start()) {
        serial_println("Error while starting station DHCP");
    } else {
        serial_println("Station DHCP started");
    }
    wifi_station_set_auto_connect(1);
    wifi_set_phy_mode(PHY_MODE_11N);
}

void ICACHE_FLASH_ATTR
setup_access_point() {
    /*
     * Setup and start access point
     */
    const char ssid[32] = "andrei";
    const char password[32] = "coolthermostat";

    struct softap_config ap_conf;

    // get current access point config
    wifi_softap_get_config(&ap_conf);
    // set config of access point
    os_memcpy(ap_conf.ssid, ssid, os_strlen(ssid));
    os_memcpy(ap_conf.password, password, os_strlen(password));
    ap_conf.ssid_len = strlen(ssid);
    ap_conf.max_connection = 4;
    ap_conf.beacon_interval = 100;
    ap_conf.channel = 4;
    ap_conf.authmode = AUTH_WPA_WPA2_PSK;

    if(!wifi_softap_set_config(&ap_conf)) {
        serial_println("Error config AP");
    } else {
        serial_println("AP config set");
    }

    // start dhcp server
    if(!wifi_softap_dhcps_start()) {
        serial_println("Error start AP DHCP");
    } else {
        serial_println("Started AP DHCP");
    }
}

char* get_station_status() {
    uint8 station_status = wifi_station_get_connect_status();
    switch(station_status) {
        case STATION_CONNECTING:
            return STA_CONNECTING;
            break;
        case STATION_WRONG_PASSWORD:
            return STA_WRONG_PASSWORD;
            break;
        case STATION_NO_AP_FOUND:
            return STA_NO_AP_FOUND;
            break;
        case STATION_CONNECT_FAIL:
            return STA_CONNECT_FAIL;
            break;
        case STATION_GOT_IP:
            return STA_GOT_IP;
            break;
        case STATION_IDLE:
            return STA_IDLE;
            break;
        default:
            return STA_ERROR;
            break;
    }
}

void get_station_ip(char* ip) {
    struct ip_info station_info;
    if(wifi_get_ip_info(STATION_IF, &station_info)) {
         os_sprintf(ip, "%d.%d.%d.%d", IP2STR(&station_info.ip));
    }
}


/*****************************************************************************/
/****************************** HTTP PROCESSING ******************************/
/*****************************************************************************/

void dump_serial_params() {
    uint8 index;
    char buffer[100];
    for(index = 0; index < serial_params.serial_length; index++) {
        os_sprintf(buffer, "%s: %s", serial_params.serial[index].key,
                   serial_params.serial[index].value);
        serial_println(buffer);
    }
}

uint8 get_http_method(char* request) {
    if(nstrcmp("GET", request, 3) == 0) {
        return GET;
    } else if(nstrcmp("POST", request, 4) == 0) {
        return POST;
    }
}

char* get_request_url(char* request) {
    unsigned short index = 0;
    unsigned short url_index = 0;
    char* url = (char*)os_malloc(REGISTERED_URL_MAX_LENGTH);
    while(request[index] != ' ') {
        index++;
    }
    index++;

    while(request[index] != ' ') {
        url[url_index] = request[index];
        index++;
        url_index++;
    }
    url[url_index] = 0;

    return url;
}

char* get_url_without_params(char* url) {
    int question_mark = 63;
    char* params_start = os_strchr(url, question_mark);
    if(!params_start) {
        return url;
    }
    uint8 len_url_no_params = params_start - url - 1;
    char* url_no_params = (char*)os_malloc(len_url_no_params + 1);
    os_memcpy(url_no_params, url, len_url_no_params);
    url_no_params[len_url_no_params] = 0;

    return url_no_params;
}

void register_url(char* url, process_request_func func) {
    uint8 index = request_handlers.next_empty;
    request_handlers.data[index].url = (char*)os_malloc(REGISTERED_URL_MAX_LENGTH);
    os_memcpy(request_handlers.data[index].url, url, os_strlen(url));
    request_handlers.data[index].func = func;
    request_handlers.next_empty++;
}

void _process_method_args(char* args, uint8 method) {
    char* var = strtok(args, "&");
    uint8 token_index = 0;
    uint8 aux_index = 0;
    while(var != 0) {
        char* var_name = (char*)os_malloc(MAX_HTPP_PARAM_NAME_LENGTH);
        char* var_value = (char*)os_malloc(MAX_HTPP_PARAM_VALUE_LENGTH);
        os_memset(var_name, 0, MAX_HTPP_PARAM_NAME_LENGTH);
        os_memset(var_value, 0, MAX_HTPP_PARAM_VALUE_LENGTH);
        // get param name
        token_index = 0;
        aux_index = 0;
        while(var[token_index] != '=' && aux_index < MAX_HTPP_PARAM_NAME_LENGTH) {
            var_name[aux_index] = var[token_index];
            token_index++;
            aux_index++;
        }
        var_name[aux_index] = 0;

        // in case wee exceeded MAX_HTPP_PARAM_NAME_LENGTH
        while(var[token_index] != '=') {
            token_index++;
        }

        // get param value
        token_index++;
        aux_index = 0;
        while(token_index < os_strlen(var) && aux_index < MAX_HTPP_PARAM_VALUE_LENGTH) {
            var_value[aux_index] = var[token_index];
            token_index++;
            aux_index++;
        }
        var_value[aux_index] = 0;

        // set param in request_params
        if(method == GET) {
            uint8 get_index = request_params.get_length;
            request_params.get[get_index].key = var_name;
            request_params.get[get_index].value = var_value;
            request_params.get_length++;
        } else if(method == POST) {
            uint8 post_index = request_params.post_length;
            request_params.post[post_index].key = var_name;
            request_params.post[post_index].value = var_value;
            request_params.post_length++;
        } else if(method == SERIAL) {
            uint8 serial_index = serial_params.serial_length;
            os_memcpy(serial_params.serial[serial_index].key, var_name,
                      os_strlen(var_name));
            serial_params.serial[serial_index].key[os_strlen(var_name)] = 0;
            os_memcpy(serial_params.serial[serial_index].value, var_value,
                      os_strlen(var_value));
            serial_params.serial[serial_index].value[os_strlen(var_value)] = 0;
            os_free(var_name);
            os_free(var_value);
            serial_params.serial_length++;
        }

        var = strtok(0, "&");
    }
}

char* get_param(char* name, uint8 method) {
    uint8 index = 0;
    if(method == GET) {
        for(index = 0; index < request_params.get_length; index++) {
            if(os_strcmp(name, request_params.get[index].key) == 0) {
                return request_params.get[index].value;
            }
        }
        return 0;
    } else if(method == POST) {
        for(index = 0; index < request_params.post_length; index++) {
            if(os_strcmp(name, request_params.post[index].key) == 0) {
                return request_params.post[index].value;
            }
        }
        return 0;
    } else if(method == SERIAL) {
        for(index = 0; index < MAX_SERIAL_PARAM_NUMBER; index++) {
            if(os_strcmp(name, serial_params.serial[index].key) == 0) {
                return serial_params.serial[index].value;
            }
        }
        return 0;
    }
}

void process_request_params(char* request, char* url) {
    unsigned short index = 0;
    char* post_args_str = 0;
    char* get_args_str = 0;
    uint8 method = get_http_method(request);
    if(method == GET) {
        while(url[index] != '?' && index < os_strlen(url))    {
            index++;
        }
        if(index == os_strlen(url)) {
            return;
        }
        index++;
        get_args_str = url + index;
        _process_method_args(get_args_str, GET);
    }
    if(method == POST) {
        while(!(request[index] == '\r' && request[index + 1] == '\n' &&
              request[index + 2] == '\r' && request[index + 3] == '\n') &&
              index < os_strlen(request) - 4) {
            index ++;
        }
        post_args_str = request + index + 4;
        _process_method_args(post_args_str, POST);
    }
}

void process_page_404(struct espconn* conn, char* data,
                      unsigned short length) {
    espconn_sent(conn, PAGE_404, os_strlen(PAGE_404));
}

void process_request(struct espconn* conn, char* data,
                     unsigned short data_length) {
    uint8 index = 0;
    uint8 found = 0;
    char* url = get_request_url(data);
    process_request_params(data, url);
    char* url_no_params = get_url_without_params(url);
    // find a request handler
    for(index = 0; index < request_handlers.next_empty; index++) {
        // serial_println(url_no_params);
        if(os_strcmp(url_no_params, request_handlers.data[index].url) == 0)
        {
            request_handlers.data[index].func(conn, data, data_length);
            found = 1;
            break;
        }
    }

    if(!found) {
        process_page_404(conn, data, data_length);
    }

    if(url == url_no_params) {
        os_free(url);
    } else {
        os_free(url);
        os_free(url_no_params);
    }
}

void process_wifi_page(struct espconn* conn, char* data,
                       unsigned short length) {
    uint8 method = get_http_method(data);
    char* ip_str = (char*)os_malloc(16);
    ip_str[0] = 0;
    if(method == POST) {
        setup_station(get_param("ssid", POST), get_param("pass", POST));
        get_station_ip(ip_str);
    }

    unsigned short wifi_page_length = os_strlen(WIFI_FORM);
    char page_buffer[wifi_page_length + 70];
    // if we are waiting for connection to access point
    uint8 station_status = wifi_station_get_connect_status();
    if(ip_str[0] == 0) {
        ip_str = (char*)os_malloc(5);
        os_sprintf(ip_str, "%s", "NaN");
    }
    if(station_status != STATION_IDLE && station_status != STATION_GOT_IP
            && station_status != 255 && station_status != STATION_NO_AP_FOUND) {
        os_sprintf(page_buffer, WIFI_FORM, " ", get_station_status(),
                   ip_str);
    } else {
        os_sprintf(page_buffer, WIFI_FORM, " ", get_station_status(),
                   ip_str);
    }
    espconn_sent(conn, page_buffer, os_strlen(page_buffer));
    os_free(ip_str);
}

/*****************************************************************************/
/************************** READ AND PUBLISH FUNCTIONS ***********************/
/*****************************************************************************/

void publish_data() {
    serial_println("in publish_data");
    ip_addr_t addr;
    IP4_ADDR(&addr, 184, 106, 153, 149);
    char url[200];
    int tmp_whole = (int)(temperature);
    int tmp_fract = (int)((temperature - (int)temperature)*100);

    int hum_whole = (int)(humidity);
    int hum_fract = (int)((humidity - (int)humidity)*100);

    os_sprintf(url,
        "http://api.thingspeak.com/update?key=%s&field1=%d.%d&field2=%d.%d",
        SERVER_AUTH_KEY, tmp_whole, tmp_fract, hum_whole, hum_fract);
    http_get(url, &addr, http_callback_example);
}

void collect_data() {
    serial_nprint(REQUEST_ATTINTY_TEMP, 3);
    os_delay_us(1000 * 20);

    char buffer[50];
    buffer[0] = 0;

    if(serial_read(buffer, 50) == 0) {
        return;
    }

    _process_method_args(buffer, SERIAL);

    float tmp = os_atof((char*)get_param("temperature", SERIAL));
    float hum = os_atof((char*)get_param("humidity", SERIAL));

    // somethings we get values that are very high ( ~= 150) out of nowhere
    // this is a temporary fix that solves that. the problem seems to originate
    // in the attiny code
    if(temperature != MAX_VAR) {
        if(abs(tmp -  temperature) <= MAX_VAR_DIFF) {
            temperature = tmp;
        }
    } else {
        temperature = tmp;
    }

    if(humidity != MAX_VAR) {
        if(abs(hum -  humidity) <= MAX_VAR_DIFF) {
            humidity = hum;
        }
    } else {
        humidity = hum;
    }
}

void read_and_publish_func(void* arg) {

    if(wifi_station_get_connect_status() == STATION_GOT_IP) {
        collect_data();
        publish_data();
    } else {
        serial_println("Station does not have ip");
    }
    reset_serial_params();
}

/*****************************************************************************/
/****************************** SERVER CALLBCKS ******************************/
/*****************************************************************************/

void http_recieve(void *arg, char* data, unsigned short length) {
    // serial_println(data);
    struct espconn *conn = (struct espconn *)arg;
    process_request(conn, data, length);

    char size[10];
    os_sprintf(size, "heap: %d", system_get_free_heap_size());
    espconn_sent(conn, size, os_strlen(size));

    espconn_disconnect(conn);
}

void ICACHE_FLASH_ATTR
http_disconnect(void *arg) {
    clean_request_params();
}

void ICACHE_FLASH_ATTR
webserver_listen(void *arg) {
    struct espconn *conn = (struct espconn *)arg;

    espconn_regist_recvcb(conn, http_recieve);
    espconn_regist_disconcb(conn, http_disconnect);
}

void ICACHE_FLASH_ATTR
setup_tcp_listener() {
    // wipe clean memory
    os_memset(server, 0, sizeof(struct espconn));

    LOCAL esp_tcp esptcp;

    server.type = ESPCONN_TCP;
    server.state = ESPCONN_NONE;
    server.proto.tcp = &esptcp;
    server.proto.tcp->local_port = WIFI_SERVER_LISTEN_PORT;
    // register on connect callback`
    espconn_regist_connectcb(&server, webserver_listen);

    // create the TCP server
    espconn_accept(&server);
    espconn_regist_time(&server, 15, 0);
}

/*****************************************************************************/
/*********************************** MAIN ************************************/
/*****************************************************************************/

static void ICACHE_FLASH_ATTR
loop(os_event_t *events) {
    system_os_post(user_loop_prority, 0, 0);
    os_delay_us(10);
}

ICACHE_FLASH_ATTR void flash_erase_all() {
  spi_flash_erase_sector(0x3C);
  spi_flash_erase_sector(0x3D);
  spi_flash_erase_sector(0x3E);
  spi_flash_erase_sector(0x3F);
}

void ICACHE_FLASH_ATTR user_init() {
    init_program_data();
    serial_init(19200);

    wifi_set_opmode(0x03);
    setup_access_point();
    setup_tcp_listener();
    register_url("/wifi_setup", process_wifi_page);

    os_timer_disarm(&read_publish_timer);
    os_timer_setfn(&read_publish_timer,
                   (os_timer_func_t *)read_and_publish_func, NULL);
    os_timer_arm(&read_publish_timer, PUBLISH_REPEAT_INTERVAL, 1);

    system_os_task(loop, user_loop_prority, user_proc_task_queue, user_proc_queue_length);
    system_os_post(user_loop_prority, 0, 0 );
}
