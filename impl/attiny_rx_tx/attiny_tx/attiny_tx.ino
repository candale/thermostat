#include <dht.h>
#include <Manchester.h>

dht DHT;

#define DHT22_PIN 1
#define DEBUG 3
#define TX_PIN 0

void setup()
{
    pinMode(DEBUG, OUTPUT);
    digitalWrite(DEBUG, HIGH);
    
    man.workAround1MhzTinyCore();
    man.setupTransmit(TX_PIN, MAN_1200);
}

void blink() {
  digitalWrite(DEBUG, LOW);
  delay(200);
  digitalWrite(DEBUG, HIGH);
}

void loop()
{
    int chk = DHT.read22(DHT22_PIN);
    char buffer[20];
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
    blink();
    
 
    delay(2000);
}

