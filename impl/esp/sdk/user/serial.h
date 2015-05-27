#include "uart.h"
#include "osapi.h"
#include "user_config.h"

void ICACHE_RAM_ATTR serial_print(char* message);
void ICACHE_RAM_ATTR serial_println(char* message);
void ICACHE_RAM_ATTR debug(char* message);
int ICACHE_RAM_ATTR  serial_read(char* buffer, unsigned short length);
void ICACHE_RAM_ATTR serial_nprint(const char* message, unsigned short length);
