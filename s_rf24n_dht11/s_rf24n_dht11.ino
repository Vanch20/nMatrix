/**
 * TRANSMITTER NODE
 * Every 2 seconds, send a payload to the receiver node.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <DHT.h>
#include "messages.h"

const int pin_sensor = 2;

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(8,9);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

// How often to send 'hello world to the other unit
const unsigned long interval = 10000; //ms

// When did we last send?
unsigned long last_sent;

float humidity;
float temperature;

DHT dht;

void setup(void)
{
  Serial.begin(57600);
  Serial.println("RF24Network TRANSMITTER");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
  dht.setup(pin_sensor);
}

void loop(void)
{
  // Pump the network regularly
  network.update();

  // If it's time to send a message, send it!
  unsigned long now = millis();
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
    
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    
    Serial.print(temperature);
    
    Serial.print("Sending...");
    message_dht11 payload = { millis(), 1, 0, temperature, humidity};
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header,&payload,sizeof(payload));
    if (ok)
      Serial.println("ok.");
    else
      Serial.println("failed.");
  }
}
