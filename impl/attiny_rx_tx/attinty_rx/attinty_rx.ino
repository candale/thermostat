#include <SoftwareSerial.h>
#include <Manchester.h>

#define RADIO_RX 2
#define SERIAL_RX 3
#define SERIAL_TX 4
#define BUFFER_SIZE 4
#define BAUD 19200
#define DEBUG_LED 1
#define DEBUG 0
#define REQUEST_TEMP "ASK"

uint8_t buffer[BUFFER_SIZE] = {0, 0, 0, 0};
uint8_t buffer_reset;
char request_buffer[4];
char command[50];
SoftwareSerial serial(SERIAL_RX, SERIAL_TX); // RX, TX

void debug(char msg[]) {
  if(DEBUG) {
    serial.println(msg);
  }
}

void debug(uint8_t msg) {
  if(DEBUG) {
    serial.println(msg);
  }
}

void _blink(int k, int wait) {
    for(int i = 0; i < k; i++) {
        digitalWrite(DEBUG_LED, HIGH);
        delay(wait);
        digitalWrite(DEBUG_LED, LOW);
        delay(wait);
    }
}

void blink_error() {
    _blink(3, 700);
}

void blink_ok() {
    _blink(1, 200);
}

void blink_startup() {
    _blink(2, 1000);
}


void reset_buffer() {
  request_buffer[0] = request_buffer[1] = 0;
  request_buffer[2] = request_buffer[3] = 0;
  buffer_reset = 1;
}

void send_data() {
    if(buffer_reset == 1) {
        serial.println("NO_DATA");
        blink_error();
        return;
    }
    sprintf(command, "humidity=%i.%i&temperature=%i.%i",
            buffer[0], buffer[1], buffer[2], buffer[3]);
    serial.println(command);
    reset_buffer();
}

void setup() {
  // ======== debug stuff =========
  pinMode(DEBUG_LED, OUTPUT);
  // ======== setup serial ========
  pinMode(SERIAL_RX, INPUT);
  pinMode(SERIAL_TX, OUTPUT);

  delay(1000);
  // ======== init serial ========
  serial.begin(BAUD);
  debug("Serial started");

  // ====== setup rf reading ======
  man.setupReceive(RADIO_RX, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE, buffer);
  debug("Setup done");

  reset_buffer();
  blink_startup();
}

void loop() {
  // ==== check if anything on radio =====
  if (man.receiveComplete()) {
    blink_ok();
    // ==== start to receive again ======
    man.beginReceiveArray(BUFFER_SIZE, buffer);
    buffer_reset = 0;
  }

  if(serial.available()) {
      debug("We received something on serial...");
      int index = 0;

      while(serial.available() && index < 3) {
        request_buffer[index] = serial.read();
        index++;
      }

      request_buffer[index] = 0;
      debug(index);
      debug(request_buffer);

      if(strcmp(request_buffer, REQUEST_TEMP) == 0 && !serial.available()) {
        debug("Asked for data. Sending data...");

        _blink(5, 100);
        send_data();
        debug("Done.");
      }
  }
}


