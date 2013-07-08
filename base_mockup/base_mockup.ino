
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

String server_result;

static void my_result_cb (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
//  Serial.println( (const char*) Ethernet::buffer + off);

  /** need test **/
  //char* test = (char*) Ethernet::buffer + off;
  char r[100];
  sprintf(r, "%s", Ethernet::buffer + off);
  Serial.print("r:");
  Serial.print(r);
  /** **/
  
  //String r = char(Ethernet::buffer + off);
//  server_result = r.substring(r.indexOf("{"));
//  Serial.print("result:");
//  Serial.println(r);
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
  
  timer = - REQUEST_RATE;
}

void loop(void)
{
  
  ether.packetLoop(ether.packetReceive());
  
  network.update();

  while ( network.available() )
  {
    RF24NetworkHeader header;
    message_dht11 message;
    network.read(header, &message, sizeof(message));
    Serial.print("Received Temperature:");
    Serial.print(message.temperature);
    Serial.print(" Humidity:");
    Serial.print(message.humidity);
    Serial.print(" at ");
    Serial.print(message.ms);
    Serial.print(" from ");
    Serial.println(header.from_node);
    
//    char* voltage;
//    dtostrf(message.voltage, 1, 2, voltage);
    String send_string = "?type=" + String(message.type);
//    send_string += "&vlotage=" + String(voltage);
    char* sends;
    send_string.toCharArray(sends, send_string.length());
    Serial.print("string:");
    Serial.println(sends);
    
    timer = millis();
    Serial.println("Sending to web...");
    ether.browseUrl(PSTR("/node/update/"), "", webhost, my_result_cb);
  }
}
