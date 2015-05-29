#include "uart.h"
#include "osapi.h"
#include "user_config.h"

#define DEBUG_0 0
#define DEBUG_1 1
#define DEBUG_2 2

#define DEBUG_LEVEL 1


void ICACHE_RAM_ATTR serial_print(char* message);
void ICACHE_RAM_ATTR serial_println(char* message);
void ICACHE_RAM_ATTR serial_debug(char* message, uint8 debug_level);
int ICACHE_RAM_ATTR  serial_read(char* buffer, unsigned short length);
void ICACHE_RAM_ATTR serial_nprint(const char* message, unsigned short length);
