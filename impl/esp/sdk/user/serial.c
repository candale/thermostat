#include "serial.h"
#include "osapi.h"

#define DEBUG 0
#define WIFI_DEBUG 0


void ICACHE_FLASH_ATTR
uart_ignore_char(char c)
{
}

void ICACHE_RAM_ATTR serial_init(int baud) {
    uart_init(baud, baud);
}

void ICACHE_RAM_ATTR serial_print(char* message) {
    int index = 0;
    while(index < os_strlen(message)) {
        uart_tx_one_char(message[index]);
        index++;
    }
}

void ICACHE_RAM_ATTR serial_nprint(char* message, unsigned short length) {
    int index = 0;
    while(index < os_strlen(message) && index < length) {
        uart_tx_one_char(message[index]);
        index++;
    }
}

void ICACHE_RAM_ATTR serial_println(char* message) {
    serial_print(message);
    uart_tx_one_char('\n');
}

void ICACHE_RAM_ATTR serial_debug(char* message) {
    if(DEBUG) {
        serial_println(message);
    }
}

int ICACHE_RAM_ATTR serial_read(char* buffer, unsigned short length) {
    int character = 0;
    char character_byte = -1;
    unsigned short count = 0;
    while(character != -1 && count < length) {
        character = uart0_rx_one_char();
        character_byte = character & 0xFF;
        // accept only ASCII
        if(character_byte > 127 || character_byte < 0 || character_byte == '\n') {
            continue;
        }

        os_delay_us(160);
        buffer[count] = character_byte;
        count++;
    }
    buffer[count] = 0;
    return count;
}
