
/**
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

#include <EtherCard.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

byte Ethernet::buffer[700];
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

const pin_rf24_ce = 9;
const pin_rf24_csn = 10;

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(pin_rf24_ce, pin_rf24_csn);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

// Address of the other node
const uint16_t other_node = 1;

// Structure of our payload
struct payload_t
{
  unsigned long ms;
  float temperature;
  float humidity;
};

void setup(void)
{
  Serial.begin(57600);
  Serial.println("Base Station Starting...");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
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
}

void loop(void)
{
  // Pump the network regularly
  network.update();

  // Is there anything ready for us?
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    Serial.print("Received Temperature:");
    Serial.print(payload.temperature);
    Serial.print(" Humidity:");
    Serial.print(payload.humidity);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}
