#include <dht.h>
#include <Manchester.h>

dht DHT;

#define DHT22_PIN 1
#define DEBUG 3
#define TX_PIN 0

void _blink(int k, int wait) {
    for(int i = 0; i < k; i++) {
        digitalWrite(DEBUG, HIGH);
        delay(wait);
        digitalWrite(DEBUG, LOW);
        delay(wait);
    }
}

void blink_error() {
    _blink(3, 330);
}

void blink_ok() {
    _blink(1, 200);
}

void blink_startup() {
    _blink(2, 1000);
}

void setup()
{
    pinMode(DEBUG, OUTPUT);

    man.workAround1MhzTinyCore();
    man.setupTransmit(TX_PIN, MAN_1200);
    blink_startup();
}


void loop()
{
    int chk = DHT.read22(DHT22_PIN);

    if(chk != DHTLIB_OK) {

    }

    double humidity = DHT.humidity;
    double temperature = DHT.temperature;

    int hum = humidity;
    double hum_fd = humidity - hum;
    uint8_t hum_f = hum_fd * 100;

    int temp = temperature;
    double temp_fd = temperature - temp;
    uint8_t temp_f = temp_fd * 100;

    uint8_t data[] = {hum, hum_f, temp, temp_f};

    man.transmitArray(4, data);
    blink_ok();

    delay(2000);
}

