#include <SoftwareSerial.h>
#include <Manchester.h>

#define RADIO_RX 2
#define SERIAL_RX 3
#define SERIAL_TX 4
#define BUFFER_SIZE 4
#define BAUD 9600
#define DEBUG_LED 0
#define DEBUG 0
#define REQUEST_TEMP "ASK"

uint8_t buffer[BUFFER_SIZE] = {0, 0, 0, 0};
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

void debug_led() {
  digitalWrite(DEBUG_LED, HIGH);
  delay(70);
  digitalWrite(DEBUG_LED, LOW);
}

void reset_buffer() {
  request_buffer[0] = request_buffer[1] = 0;
  request_buffer[2] = request_buffer[3] = 0;
}

void send_data() {
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
  serial.begin(19200);
  debug("program started");

  // ====== setup rf reading ======
  man.setupReceive(RADIO_RX, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE, buffer);
  debug("setup done");
}

void loop() {
  // ==== check if anything on radio =====
  if (man.receiveComplete()) {
    debug_led();
    // ==== start to receive again ======
    man.beginReceiveArray(BUFFER_SIZE, buffer);
  }
  if(serial.available()) {
      debug("we have data");
      int index = 0;
      while(serial.available() && index < 3) {
        request_buffer[index] = serial.read();
        index++;
      }
      request_buffer[index] = 0;
      debug(index);
      debug(request_buffer);
      if(strcmp(request_buffer, REQUEST_TEMP) == 0 && !serial.available()) {
        debug("sending data...");
        send_data();
      }
  }
}

