
/**
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

#include <EtherCard.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "messages.h"

#define REQUEST_RATE 10000
#define SEND_TIMEOUT 10000

const char webhost[] PROGMEM = "api.rarasun.com";
const char url_basereg[] PROGMEM  = "/basepoint/reg/";
const char url_update[] PROGMEM  = "/node/update/";

byte Ethernet::buffer[700];
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

const int pin_rf24_ce = 9;
const int pin_rf24_csn = 10;

RF24 radio(pin_rf24_ce, pin_rf24_csn);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

// Address of the other node
const uint16_t other_node = 1;

static long timer;

int send_ok = 1;

char result[100];

int bytereaded;

static void my_result_cb (byte status, word off, word len) {
  send_ok = 1;
  //Serial.println(status);
  //Serial.println(len);
  //Serial.print(millis() - timer);
  //Serial.println(" ms");
  //Serial.println( (char*) Ethernet::buffer + off);
  
  /** need test **/
  sscanf((const char *)Ethernet::buffer + off, "%[^{]%s", &result); 
  
  Serial.print("result: ");
  Serial.println(result);
  /** **/
}

void setup(void)
{
  Serial.begin(57600);
  Serial.println("Base Station Starting...");
  
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.println();
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  
  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup())
    Serial.println( "DHCP failed");
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.mymask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
    
  //ether.staticSetup(myip, gwip);
  if (!ether.dnsLookup(webhost))
    Serial.println("DNS failed");
  ether.printIp("Server: ", ether.hisip);
  
  
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
  //timer = - SEND_TIMEOUT;
}

void loop(void)
{
  if (millis() > timer + SEND_TIMEOUT) {
    send_ok = 1;
  }
  
  ether.packetLoop(ether.packetReceive());
  
  network.update();

  while ( network.available() )
  {
    RF24NetworkHeader header;
    message_dht11 message;
    network.read(header, &message, sizeof(message));
    Serial.print("Received: ");
    Serial.print(message.toString());
    Serial.print(" from ");
    Serial.println(header.from_node);
    
    if (send_ok == 1)
    {
      send_ok = 0;
      timer = millis();
      
      /** need test **/
      char data[100];
      sprintf(data, "?type=%d&voltage=%.2f&v=%s", message.type, message.voltage, message.toPostString());
      Serial.print("data:");
      Serial.println(data);
      /**  **/
      
      Serial.println("Sending to web...");
      ether.browseUrl(PSTR("/node/update"), data, webhost, my_result_cb);
    }
  }
}
