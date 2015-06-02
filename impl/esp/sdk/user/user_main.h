#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "serial.h"
#include "httpclient.h"
#include "fuzzyengine.h"
#include "user_config.h"

/**********************************************/
/* *************** SERVER AUTH ****************/
// not yet used, this macro
#define THINGSPEAK_IP 184, 106, 153, 149
#define SERVER_AUTH_KEY "CUTRB50CQ2REU78M"


/*****************************************/
/* *************** METHODS ***************/
#define GET 1
#define POST 2
#define PUT 3
#define DELETE 4
#define SERIAL 5


/***********************************************/
/* *************** ACCESS CONFIG ***************/
#define AP_SSID "Your Thermostat"
#define AP_PASS "coolthermostat"

#define MDNS_HOST_NAME "thermostat"
#define MDNS_SERVER_NAME "therm"


/************************************************/
/* *************** HTTP CONSTANTS ***************/
#define WIFI_SERVER_LISTEN_PORT 80
#define URLS_MAX_REGISTER_NUMBER 10
#define REGISTERED_URL_MAX_LENGTH 35
#define MAX_HTPP_PARAM_NUMBER 10
#define MAX_HTPP_PARAM_NAME_LENGTH 30
#define MAX_HTPP_PARAM_VALUE_LENGTH 30


/*****************************************************/
/* *************** TIME SYNC CONSTANTS ***************/
// data read and publish time interval
#define PUBLISH_REPEAT_INTERVAL_SECONDS 10
#define PUBLISH_REPEAT_INTERVAL_MILIS PUBLISH_REPEAT_INTERVAL_SECONDS * 1000

// mdns retry time interval
#define MDNS_REPEAT_INTERVAL_SECONDS 10
#define MDNS_REPEAT_INTERVAL_MILIS MDNS_REPEAT_INTERVAL_SECONDS * 1000


/************************************************************/
/* *************** VARIABLE RELATED CONSTANTS ***************/
// maximum absolute difference between last recorded variable and current one
#define MAX_VAR_DIFF 40
#define MAX_VAR 0xFFFFFFFF
// how many minutes of history will be held in memory
#define TEMPERATURE_HISTORY_SPAN 5
// don't touch this. IT IS COMPUTED
#define TEMPERATURE_HISTORY_CONTAINER_COUNT 60 / PUBLISH_REPEAT_INTERVAL_SECONDS * TEMPERATURE_HISTORY_SPAN

/******************************************************/
/* *************** ATTINY COMM REQUESTS ***************/
const char REQUEST_ATTINTY_TEMP[] = "ASK";
#define MAX_SERIAL_PARAM_NUMBER 4


/***************************************/
/* *************** PAGES ***************/
#define PAGE_404 "<html>"\
                 "<h2>404 - Not Found</h2>"\
                 "</html>"
const char WIFI_FORM[] = "<html>"
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

const char TMP_CONTROL_FORM[] = "<html>"
                  "<h3>Current temperature: %s</h3>"
                  "<h3>Set temperature: %s</h3>"
                  "<form method='post' action='/control'>"
                  "<label>Set temperature</label>"
                  "<input type='text' pattern='[0-9]+[\\.]*[0-9]*' name='temperature'>"
                  "<input type='submit' value='Submit'>"
                  "</form>"
                  "</html>";

const char RELOAD_HTML[] = "<meta http-equiv='refresh' content='5'>";

// station stats
const char STA_IDLE[] = "not connected";
const char STA_CONNECTING[] = "connecting";
const char STA_WRONG_PASSWORD[] = "wrong password";
const char STA_NO_AP_FOUND[] = "no access point found";
const char STA_CONNECT_FAIL[] = "failed to connect";
const char STA_GOT_IP[] = "connected";
const char STA_ERROR[] = "error";


/*****************************************/
/* *************** MOCKING ***************/
#define MOCK_DATA 1
#define MOCK_TEMPERATURE_SET 28.5
#define MOCK_TEMPERATURE 28
#define MOCK_HUMIDITY 50
#define MOCK_LEST_SQUARE 0


/****************************************************/
/* *************** PROGRAM PARAMETERS ***************/
#define IS_PUBLISH_ENABLED 0
#define IS_FUZZY_ENABLED 0
#define IS_DATA_COLLECTION_ENABLED 1


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
struct url_mapping_struct {
    struct data_holder {
        char* url;
        process_request_func func;
    } data[URLS_MAX_REGISTER_NUMBER];
    uint8 next_empty;
} request_handlers;

// structure that holds the parameters from each request
struct request_params_struct {
    struct params_dynamic {
        char* key;
        char* value;
    } get[MAX_HTPP_PARAM_NUMBER], post[MAX_HTPP_PARAM_NUMBER];
    uint8 post_length;
    uint8 get_length;
} request_params;

// structure that holds the parameters from each request
struct serial_params_struct {
    struct params_static {
        char key[20];
        char value[30];
    } serial[MAX_SERIAL_PARAM_NUMBER];
    uint8 serial_length;
} serial_params;

struct publish_params_struct {
  float temperature_set;
  float temperature;
  float humidity;
  float tmp_trend_least_square;
  float tmp_trend_diff;
  float tmp_trend_avg;
} publish_params;


/**************************************************/
/* *************** GLOBAL VARIABLES ***************/
char print_buffer[500];
// wifi server
struct espconn server;
// fuzzy engine structure
fuzzy_engine* engine;
// timer structure to read and send data at given interval
static volatile os_timer_t collect_and_process_timer;
// timer structure that atempts to setup mdns at given interval
static volatile os_timer_t mdns_setup_timer;
// temperature history spanning TEMPERATURE_HISTORY_SPAN minutes
float temperature_history[TEMPERATURE_HISTORY_CONTAINER_COUNT];
uint8 temperature_history_count = 0;
