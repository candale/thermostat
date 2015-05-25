#include "uart.h"

void serial_print(char* message);
void serial_println(char* message);
void debug(char* message);
int serial_read(char* buffer, unsigned short length);
void serial_nprint(char* message, unsigned short length);
