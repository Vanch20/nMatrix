/**
 * iHome Sensor for temperature and humidity
 *
 */

#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Tictocs.h>
#include <Button.h>
#include <TictocTimer.h>
#include "nodeconfig.h"
#include "sleep.h"
#include "S_message.h"
#include "printf.h"

// Config
const int node_type = 1;
long node_id = 0;
const int node_id_address = 10; // in eeprom
const int base_node_id_address = 100; // in eeprom
const int node_unique_id = "a"; 

// How often to send result to the other unit
const unsigned long interval = 2000; //ms

// Pins for sensors
const int voltage_pin = A2;
const int temp_pin = A3;

// Pins for LEDs, or '0' for no LED connected
const int led_red = 0; 
const int led_green = 0;

// What voltage is a reading of 1023?
const unsigned voltage_reference = 2.4 * 256; // 4.1V

// How many measurements to take.  64*1024 = 65536, so 64 is the max we can fit in a uint16_t.
const int num_measurements = 64;

// When did we last send?
unsigned long last_sent;

// Pins for radio
const int rf_ce = 9;
const int rf_csn = 10;

RF24 radio(rf_ce, rf_csn);
RF24Network network(radio);

void setup(void)
{
  Serial.begin(57600);
  Serial.print("iHome sensor for temperature and humidity starting up..");
  
  // Read Node id from eeprom
  eeprom_read_block((void*)&node_id, (void*)node_id_address, sizeof(long));
  Serial.print("node_id=");
  Serial.println(node_id);
  
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ node_id);
}

void loop(void)
{
  /*** TODOs: auto find basenode and get nodeid ***/
  
  
  network.update();

  // send every 2s
  unsigned long now = millis();
  if ( now - last_sent >= interval )
  {
    last_sent = now;
    
    S_message message;
    
    int i;
    uint32_t reading = 0;
    
    /***** Take the voltage reading *****/ 
    i = num_measurements;
    reading = 0;
    while(i--)
    {
      reading += analogRead(voltage_pin);
    }
    // Convert the voltage reading to volts*256
    message.voltage = ( reading * voltage_reference ) >> 16; 
    /***** end take the voltage reading ******/
    
    
    /***** Take the temp reading *****/ 
    i = num_measurements;
    while(i--)
    {
      reading += analogRead(temp_pin);
    }
    // Convert the reading to celcius*256
    // This is the formula for MCP9700.
    // C = reading * 1.1
    // C = ( V - 1/2 ) * 100
    message.v1 = ( ( ( reading * 0x120 ) - 0x800000 ) * 0x64 ) >> 16;
    /***** end take the temp reading ******/
    
    message.type = node_type;

    Serial.print("Sending...");
    RF24NetworkHeader header(/*to node*/ 0, 'S');
    bool ok = network.write(header, &message, sizeof(message));
    if (ok)
    {
      Serial.println("ok.");
    }
    else
    {
      Serial.println("failed.");
    }
  }
}
