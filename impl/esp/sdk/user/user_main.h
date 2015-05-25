#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "serial.h"
#include "httpclient.h"
#include "fuzzyengine.h"


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
