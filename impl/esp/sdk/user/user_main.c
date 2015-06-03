#include "user_main.h"
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

    // collected data
    publish_params.temperature_set = 24;
    publish_params.temperature = MAX_VAR;
    publish_params.humidity = MAX_VAR;
    publish_params.tmp_trend_least_square = 0;
    publish_params.tmp_trend_diff = 0;
    publish_params.tmp_trend_avg = 0;
}


/*****************************************************************************/
/*************************** STATION AND AP SETUP ****************************/
/*****************************************************************************/

void ICACHE_RAM_ATTR
setup_mdns(void *arg) {
    serial_debug("Trying to setup mdns...", DEBUG_1);

    uint8 station_status = wifi_station_get_connect_status();
    struct ip_info station_info;
    if(station_status == STATION_GOT_IP &&
            wifi_get_ip_info(STATION_IF, &station_info)) {

        // kill softap - mdns works without AP
        wifi_set_opmode(0x1);
        wifi_softap_free_station_info();

        struct mdns_info *info = (struct mdns_info *)os_zalloc(
            sizeof(struct mdns_info));

        info->host_name = MDNS_HOST_NAME;
        info->ipAddr = station_info.ip.addr;
        info->server_name = MDNS_SERVER_NAME;
        info->server_port = 80;
        info->txt_data = "version = now";

        espconn_mdns_init(info);
        espconn_mdns_enable();
        espconn_mdns_server_register();

        // reset timer at larger interval to keep publishing
        // itself on the network
        if(MDNS_RUNNING == 0) {
            os_timer_disarm(&mdns_setup_timer);
            os_timer_setfn(&mdns_setup_timer,
                           (os_timer_func_t *)setup_mdns, NULL);
            os_timer_arm(&mdns_setup_timer, MDNS_REPEAT_PUBLISH_INTERVAL_MILIS, 1);
            MDNS_RUNNING = 1;
        }

        serial_debug("Successfull mdns setup", DEBUG_1);
    } else {
        serial_debug("Don't have station ip yet. Next time", DEBUG_1);
    }
}

void setup_station(char* ssid, char* password) {
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

void setup_access_point() {
    /*
     * Setup and start access point
    */
    struct softap_config ap_conf;

    // get current access point config
    wifi_softap_get_config(&ap_conf);
    // set config of access point
    os_memcpy(ap_conf.ssid, AP_SSID, os_strlen(AP_SSID));
    os_memcpy(ap_conf.password, AP_PASS, os_strlen(AP_PASS));
    ap_conf.ssid_len = strlen(AP_SSID);
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

const char* get_station_status() {
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
                       unsigned short length)
{
    uint8 method = get_http_method(data);
    char* ip_str = (char*)os_malloc(16);
    unsigned short wifi_page_length = os_strlen(WIFI_FORM);
    char page_buffer[wifi_page_length + 70];

    ip_str[0] = 0;
    if(method == POST) {
        setup_station(get_param("ssid", POST), get_param("pass", POST));
        get_station_ip(ip_str);
    }

    // if we are waiting for connection to access point
    uint8 station_status = wifi_station_get_connect_status();
    if(ip_str[0] == 0) {
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

void process_temperature_control_page(struct espconn* conn, char* data,
                                      unsigned short length)
{
    unsigned short control_page_length = os_strlen(TMP_CONTROL_FORM);
    char page_buffer[control_page_length + 20];

    uint8 method = get_http_method(data);
    if(method == POST) {
        char* set_tmp = get_param("temperature", POST);
        publish_params.temperature_set = os_atof(set_tmp);
    }

    char current_temperature_str[10], set_temperature_str[10];
    current_temperature_str[0] = set_temperature_str[0] = 0;

    ftoa(publish_params.temperature, current_temperature_str);
    ftoa(publish_params.temperature_set, set_temperature_str);

    os_sprintf(page_buffer, TMP_CONTROL_FORM,
               current_temperature_str, set_temperature_str);
    espconn_sent(conn, page_buffer, os_strlen(page_buffer));
}


/*****************************************************************************/
/****************************** DATA STATISTICS ******************************/
/*****************************************************************************/
/*
 * Computes a function that aproximates the given data points
*/
 inline static double sqr(double x) {
    return x*x;
}

int linreg(int n, const double y[], double* m, double* b, double* r)
{
    double sumx = 0.0;                        /* sum of x                      */
    double sumx2 = 0.0;                       /* sum of x**2                   */
    double sumxy = 0.0;                       /* sum of x * y                  */
    double sumy = 0.0;                        /* sum of y                      */
    double sumy2 = 0.0;                       /* sum of y**2                   */

    int i;
    for (i = 0; i < n; i++)
    {
        sumx  += i;
        sumx2 += sqr(i);
        sumxy += i * y[i];
        sumy  += y[i];
        sumy2 += sqr(y[i]);
    }

    double denom = (n * sumx2 - sqr(sumx));
    if (denom == 0) {
           // singular matrix. can't solve the problem.
        *m = 0;
        *b = 0;
        *r = 0;
        return 1;
    }

    *m = (n * sumxy  -  sumx * sumy) / denom;
    *b = (sumy * sumx2  -  sumx * sumxy) / denom;
    if (r!=NULL) {
        *r = (sumxy - sumx * sumy / n) /
            (n_sqrt((sumx2 - sqr(sumx) / n) * (sumy2 - sqr(sumy) / n)));
    }

    return 0;
}

double get_temperature_trend()
{
    double avg = MAX_VAR, last_tmp = MAX_VAR;
    uint8 i, j;
    char buf[500];
    buf[0] = 0;
    // if(temperature_history_count != TEMPERATURE_HISTORY_CONTAINER_COUNT) {
    //     return 0;
    // }
    uint8 records_per_minute = (
        (uint8)TEMPERATURE_HISTORY_CONTAINER_COUNT / TEMPERATURE_HISTORY_SPAN);
    avg = temperature_history[1] -  temperature_history[0];
    for(i = 1; i < TEMPERATURE_HISTORY_CONTAINER_COUNT - 1; i++) {
        avg = (avg + temperature_history[i + 1] - temperature_history[i]) / 2;
    }
    for(i = 0; i < TEMPERATURE_HISTORY_CONTAINER_COUNT; i++) {
        // sprintf_double(buf, temperature_history[i]);
        buf[0] = 0;
        ftoa(temperature_history[i], buf);
        serial_debug(buf, DEBUG_1);
    }

    if(temperature_history_count != TEMPERATURE_HISTORY_CONTAINER_COUNT) {
        return 0;
    }

    os_sprintf(buf, "average: ");
    ftoa(avg, buf);
    serial_debug(buf, DEBUG_1);
    publish_params.tmp_trend_avg = avg;

    os_sprintf(buf, "last and first difference: ");
    ftoa(temperature_history[temperature_history_count - 1] -
         temperature_history[0], buf);
    serial_debug(buf, DEBUG_1);
    publish_params.tmp_trend_diff = (
        temperature_history[temperature_history_count - 1] -
        temperature_history[0]);

    double m, b, r;
    linreg(temperature_history_count, temperature_history, &m, &b, &r);
    os_sprintf(buf, "least squares: ");
    ftoa(m, buf);
    serial_debug(buf, DEBUG_1);
    publish_params.tmp_trend_least_square = m;

    return avg;
}


/*****************************************************************************/
/************************** READ AND PUBLISH FUNCTIONS ***********************/
/*****************************************************************************/

void reset_temperature_history()
{
    int i;
    for(i = 0; i < TEMPERATURE_HISTORY_CONTAINER_COUNT; i++) {
        temperature_history[i] = MAX_VAR;
    }
}

void record_temperature_history(double temp)
{
    int i;
    if(temperature_history_count == TEMPERATURE_HISTORY_CONTAINER_COUNT) {
        for(i = 0; i < temperature_history_count - 1; i++) {
            temperature_history[i] = temperature_history[i + 1];
        }
        temperature_history[i] = temp;
    } else {
        temperature_history[temperature_history_count++] = temp;
    }
}

void ICACHE_RAM_ATTR
publish_data()
{
    if(!IS_PUBLISH_ENABLED) {
        serial_debug("Publish disabled", DEBUG_1);
        return;
    }

    serial_println("Publishing data");
    ip_addr_t addr;
    IP4_ADDR(&addr, 184, 106, 153, 149);
    char url[200];

    char tmp[16], hum[16], least_square[16], diff[16], avg[16];
    os_memset(tmp, 0, 16);
    os_memset(hum, 0, 16);
    os_memset(least_square, 0, 16);
    os_memset(diff, 0, 16);
    os_memset(avg, 0, 16);

    ftoa(publish_params.temperature, tmp);
    ftoa(publish_params.humidity, hum);
    ftoa(publish_params.tmp_trend_least_square, least_square);
    ftoa(publish_params.tmp_trend_diff, diff);
    ftoa(publish_params.tmp_trend_avg, avg);

    os_sprintf(url,
        "http://api.thingspeak.com/update?key=%s&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s",
        SERVER_AUTH_KEY, tmp, hum,least_square, diff, avg);
    serial_debug(url, DEBUG_1);

    http_get(url, &addr, http_callback_example);
}

void mock_data() {
    publish_params.temperature_set = MOCK_TEMPERATURE_SET;
    publish_params.temperature = MOCK_TEMPERATURE;
    publish_params.humidity = MOCK_HUMIDITY;
    publish_params.tmp_trend_least_square = MOCK_LEST_SQUARE;
}

uint8 ICACHE_RAM_ATTR
collect_data()
{
    if(!IS_DATA_COLLECTION_ENABLED) {
        serial_debug("Data collection is not enabled", DEBUG_1);
        return 0;
    }

    if(MOCK_DATA) {
        mock_data();
        return 1;
    }

    serial_nprint(REQUEST_ATTINTY_TEMP, 3);
    os_delay_us(1000 * 20);

    char buffer[50];
    buffer[0] = 0;

    if(serial_read(buffer, 50) == 0) {
        serial_debug("Did not receive any data form sensors", DEBUG_1);
        return 0;
    }

    _process_method_args(buffer, SERIAL);

    double tmp = os_atof((char*)get_param("temperature", SERIAL));
    double hum = os_atof((char*)get_param("humidity", SERIAL));

    // sometimes we get values that are very high ( ~= 150) out of nowhere
    // this is a temporary fix that solves that. the problem seems to originate
    // in the attiny code
    if(publish_params.temperature != MAX_VAR) {
        if(abs(tmp -  publish_params.temperature) <= MAX_VAR_DIFF) {
            publish_params.temperature = tmp;
        }
    } else {
        publish_params.temperature = tmp;
    }

    if(publish_params.humidity != MAX_VAR) {
        if(abs(hum -  publish_params.humidity) <= MAX_VAR_DIFF) {
            publish_params.humidity = hum;
        }
    } else {
        publish_params.humidity = hum;
    }

    record_temperature_history(publish_params.temperature);
    get_temperature_trend();
    return 1;
}

void ICACHE_RAM_ATTR
handle_fuzzy()
{
    if(!IS_FUZZY_ENABLED) {
        serial_debug("Fuzzy is not enabled", DEBUG_1);
        return;
    }

    serial_debug("Registering fuzzy values ...", DEBUG_2);

    register_value_by_name(engine, "temp_err",
                           publish_params.temperature_set -
                           publish_params.temperature);
    register_value_by_name(engine, "humidity", publish_params.humidity);
    register_value_by_name(engine, "roc",
                           publish_params.tmp_trend_least_square);
    register_value_by_name(engine, "roh",
                           publish_params.tmp_trend_least_square);

    point* p = run_fuzzy(engine);

    if(p == 0) {
        serial_debug("Rules do not cover everything. Area is 0", DEBUG_1);
        return;
    }

    char buf[50];
    os_strcpy(buf, "x: ");
    ftoa(p->x, buf);
    serial_debug(buf, DEBUG_2);

    os_strcpy(buf, "y: ");
    ftoa(p->y, buf);
    serial_debug(buf, DEBUG_2);
    os_free(p);
}

void ICACHE_RAM_ATTR
collect_and_process_data(void* arg)
{
    // publish and run fuzzy only if we have read some data
    if(collect_data()) {
        handle_fuzzy();

        if(wifi_station_get_connect_status() == STATION_GOT_IP &&
                !IS_PUBLISH_ENABLED) {
            publish_data();
        } else {
            serial_println("Station does not have ip");
        }
    }
    reset_serial_params();
}

/*****************************************************************************/
/****************************** SERVER CALLBCKS ******************************/
/*****************************************************************************/

void ICACHE_RAM_ATTR
http_recieve(void *arg, char* data, unsigned short length)
{
    // serial_println(data);
    struct espconn *conn = (struct espconn *)arg;
    process_request(conn, data, length);

    char size[10];
    os_sprintf(size, "heap: %d", system_get_free_heap_size());
    espconn_sent(conn, size, os_strlen(size));

    espconn_disconnect(conn);
}

void ICACHE_RAM_ATTR
http_disconnect(void *arg)
{
    clean_request_params();
    os_free((struct espconn*) arg);
}

void ICACHE_RAM_ATTR
webserver_listen(void *arg) {
    struct espconn *conn = (struct espconn *)arg;

    espconn_regist_recvcb(conn, http_recieve);
    espconn_regist_disconcb(conn, http_disconnect);
}

void setup_tcp_listener()
{
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
/****************************** FUZZY ENGINE *********************************/
/*****************************************************************************/

void init_fuzzy_engine()
{
    engine = create_fuzzy_engine();
// TEMPERATURE
    ling_var* temp_err = create_linguistic_variable("temp_err", 1, INPUT);
    ling_val* low_temp = create_linguistic_value("low", -100, -100, 1.75, 2.75);
    ling_val* moderate_temp = create_linguistic_value("moderate",
                                                      1.5, 2.75, 3.75, 5);
    ling_val* high_temp = create_linguistic_value("high", 3.75, 5.5, 100, 100);
    add_ling_val(temp_err, low_temp);
    add_ling_val(temp_err, moderate_temp);
    add_ling_val(temp_err, high_temp);

    // HUMIDITY -- INPUT
    ling_var* humidity = create_linguistic_variable("humidity", 2, INPUT);
    ling_val* low_hum = create_linguistic_value("low", 0, 0, 20, 40);
    ling_val* moderate_hum = create_linguistic_value("moderate", 20, 47, 47, 70);
    ling_val* high_hum = create_linguistic_value("high", 50, 70, 100, 100);
    add_ling_val(humidity, low_hum);
    add_ling_val(humidity, moderate_hum);
    add_ling_val(humidity, high_hum);

    // RATE OF COOLING -- INPUT
    ling_var* rate_of_cooling = create_linguistic_variable("roc", 3, INPUT);
    ling_val* high_roc = create_linguistic_value("low", 0, 0, -0.11, -0.28);
    ling_val* moderate_roc = create_linguistic_value("moderate",
                                                     -0.11, -0.32, -0.32, -0.5);
    ling_val* low_roc = create_linguistic_value("high", -0.35,- 0.5, -2, -2);
    add_ling_val(rate_of_cooling, low_roc);
    add_ling_val(rate_of_cooling, moderate_roc);
    add_ling_val(rate_of_cooling, high_roc);

    // RATE OF HEATING -- INPUT
    ling_var* rate_of_heating = create_linguistic_variable("roh", 3, INPUT);
    ling_val* high_roh = create_linguistic_value("low", 0, 0, 0.11, 0.28);
    ling_val* moderate_roh = create_linguistic_value("moderate",
                                                     0.11, 0.32, 0.32, 0.5);
    ling_val* low_roh = create_linguistic_value("high", 0.35, 0.5, 2, 2);
    add_ling_val(rate_of_heating, low_roh);
    add_ling_val(rate_of_heating, moderate_roh);
    add_ling_val(rate_of_heating, high_roh);

    // HEATING STATUS -- OUTPUT
    ling_var* heat_status = create_linguistic_variable("heat_status", 3,
                                                       OUTPUT);
    ling_val* heat_on = create_linguistic_value("on", 0, 0, 30, 65);
    ling_val* heat_off = create_linguistic_value("off", 35, 50, 100, 100);
    add_ling_val(heat_status, heat_on);
    add_ling_val(heat_status, heat_off);

    add_ling_var(engine, temp_err);
    add_ling_var(engine, humidity);
    add_ling_var(engine, rate_of_cooling);
    add_ling_var(engine, rate_of_heating);
    add_ling_var(engine, heat_status);

    // ============================== RULES ==================================

    // if temp_err is high and rate_of_heating is high heat_status is OFF
    rule_antecedent* antecedent_4 = create_rule_antecedent();
    condition* cond_41 = create_condition(temp_err, high_temp, AND);
    condition* cond_42 = create_condition(humidity, high_hum, NONE);
    add_condition_to_antecedent(antecedent_4, cond_41);
    add_condition_to_antecedent(antecedent_4, cond_42);
    rule_consequent* consequent_4 = create_rule_consequent(heat_status,
                                                           heat_off);
    fuzzy_rule* rule_4 = create_rule(engine, antecedent_4, consequent_4);
    add_rule(engine, rule_4);

    // if temp_err is higg and rate_of_heating is low then heat_status is ON
    rule_antecedent* antecedent_1 = create_rule_antecedent();
    condition* cond_11 = create_condition(temp_err, high_temp, AND);
    condition* cond_12 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent_1, cond_11);
    add_condition_to_antecedent(antecedent_1, cond_12);
    rule_consequent* consequent_1 = create_rule_consequent(heat_status,
                                                           heat_on);
    fuzzy_rule* rule_1 = create_rule(engine, antecedent_1, consequent_1);
    add_rule(engine, rule_1);

    // if temp_err is low and rate_of_heating is low heat_status is ON
    rule_antecedent* antecedent_2 = create_rule_antecedent();
    condition* cond_21 = create_condition(temp_err, low_temp, AND);
    condition* cond_22 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent_2, cond_21);
    add_condition_to_antecedent(antecedent_2, cond_22);
    rule_consequent* consequent_2 = create_rule_consequent(heat_status,
                                                           heat_on);
    fuzzy_rule* rule_2 = create_rule(engine, antecedent_2, consequent_2);
    add_rule(engine, rule_2);

    // if temp_err is low and rate_of_heating is high heat_status is OFF
    rule_antecedent* antecedent_3 = create_rule_antecedent();
    condition* cond_31 = create_condition(temp_err, low_temp, AND);
    condition* cond_32 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent_3, cond_31);
    add_condition_to_antecedent(antecedent_3, cond_32);
    rule_consequent* consequent_3 = create_rule_consequent(heat_status,
                                                           heat_off);
    fuzzy_rule* rule_3 = create_rule(engine, antecedent_3, consequent_3);
    add_rule(engine, rule_3);
}

/*****************************************************************************/
/********************************** MAIN ************************************s*/
/*****************************************************************************/

static void ICACHE_RAM_ATTR loop(os_event_t *events)
{
    system_os_post(user_loop_prority, 0 , 0);
    os_delay_us(10);
}

 void flash_erase_all()
 {
  spi_flash_erase_sector(0x3C);
  spi_flash_erase_sector(0x3D);
  spi_flash_erase_sector(0x3E);
  spi_flash_erase_sector(0x3F);
}

void init_timers()
{
    // runs every PUBLISH_REPEAT_INTERVAL_MILIS to collect and send data
    os_timer_disarm(&collect_and_process_timer);
    os_timer_setfn(&collect_and_process_timer,
                   (os_timer_func_t *)collect_and_process_data, NULL);
    os_timer_arm(&collect_and_process_timer, PUBLISH_REPEAT_INTERVAL_MILIS, 1);

    // this timer will run until station has got ip. when that happnes
    // it will try to setup mdns. if successfull, it will disarm the timer
    os_timer_disarm(&mdns_setup_timer);
    os_timer_setfn(&mdns_setup_timer,
                   (os_timer_func_t *)setup_mdns, NULL);
    os_timer_arm(&mdns_setup_timer, MDNS_REPEAT_PROBE_INTERVAL_MILIS, 1);
}

void ICACHE_RAM_ATTR user_init() {
    serial_init(19200);

    init_program_data();
    init_fuzzy_engine();

    wifi_set_opmode(0x03);
    setup_access_point();
    setup_tcp_listener();
    register_url("/wifi_setup", process_wifi_page);
    register_url("/control", process_temperature_control_page);

    init_timers();

    system_os_task(loop, user_loop_prority, user_proc_task_queue, user_proc_queue_length);
    system_os_post(user_loop_prority, 0, 0 );
}
