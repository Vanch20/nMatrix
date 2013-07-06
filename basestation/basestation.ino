/**
 * iHome Centure Router
 *
 * @author Linln 2013-6-17
 */
 
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <EtherCard.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
//#include "nodeconfig.h"
#include "S_message.h"
#include "printf.h"

// Website Config
#define REQUEST_RATE 5000 // milliseconds

// Array of nodes we are aware of
const short max_active_nodes = 10;
int active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;
const int node_id_set[10] = { 00, 02, 05, 012, 015, 022, 025, 032, 035, 045 };

// Prototypes for functions to send & handle messages
bool send_T(int to);
bool send_N(int to);
bool send_S(int to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void handle_S(RF24NetworkHeader& header);
void add_node(int node);

const char webhost[] PROGMEM = "api.rarasun.com";
const char url_basereg[] PROGMEM  = "/basepoint/reg/";
const char url_update[] PROGMEM  = "/device/update/";
static long timer; // Call Web timer
const int node_id_address = 10; // in eeprom
long node_id = 0;

// EtherCard Config
// TODO : auto mac config
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
String mac_str = "";
byte Ethernet::buffer[700];
//String postdate = "";

// Pins for radio
const int rf_ce = 9;
const int rf_csn = 10;

// RF24 Config
RF24 radio(rf_ce, rf_csn);
RF24Network rfnetwork(radio);

void setup () 
{
  Serial.begin(57600);
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
  {
    Serial.println( "Failed to access Ethernet controller");
  }
  
  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup()) 
  {
    Serial.println( "DHCP failed");
  }
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.mymask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
  
  if (!ether.dnsLookup(webhost))
    Serial.println("DNS failed");
  ether.printIp("Server: ", ether.hisip);
  
  for (byte i = 0; i < 6; ++i) {
    mac_str.concat(String(mymac[i], HEX));
  }
  Serial.println(mac_str);
  
  // Read Node id from eeprom
  eeprom_read_block((void*)&node_id, (void*)node_id_address, sizeof(long));
  Serial.print("node_id=");
  Serial.println(node_id);
  
  //Serial.println("\n>>> REQ");
  //ether.httpPost(url_basereg, webhost, "", buf, cb_reg);
  
  //ether.packetLoop(ether.packetReceive());

  // Setting up RF24
  SPI.begin();
  radio.begin();
  rfnetwork.begin(/*channel*/ 90, /*node address*/ node_id);
  
  timer = - REQUEST_RATE;
}

void loop () 
{
  /*** connect to webserver check wether registered ***/
  while (node_id == 0)
  {
    ether.packetLoop(ether.packetReceive());
  
    if (millis() > timer + REQUEST_RATE) {
      timer = millis();
      String postdata = "{\"mac\":\"" + mac_str + "\"}";
      char* ttt;
      postdata.toCharArray(ttt, postdata.length());
      Serial.println("\n>>> REQ --> Get Node Id");
      ether.browseUrl(PSTR("/basepoint/reg/"), ttt, webhost, cb_reg);
    }
  }
  
  rfnetwork.update();
  while ( rfnetwork.available() )
  {
    RF24NetworkHeader header;
    rfnetwork.peek(header);
    
    switch (header.type)
    {
      case 'T':  // time sync
        handle_T(header);
        break;
        
      case 'N':  // new node 
        handle_N(header);
        break;
        
      case 'S':  // sensor messages
        handle_S(header);
        break;
        
      default:
        printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"), header.type);
        rfnetwork.read(header, 0, 0);
        break;
    };
    
    /*** listen for sensor node reg ***/
    
  
    /*** Get info from nodes ***/
  
  
//    S_message message;
//    network.read(header, &message, sizeof(message));
//    Serial.println(message.toString());
//    //printf_P(PSTR("%lu: APP Received #%u %s from 0%o\n\r"), millis(), header.id, message.toString(), header.from_node);
//    
//    // Send message to webserver
//    char buf = get_send_string(&message);
//    
//    ether.packetLoop(ether.packetReceive());
//    ether.httpPost(url_update, webhost, "", buf, cb_update);
  }
}

/**
 * Write noid to eeprom
 */
void set_node_id(long id)
{
  eeprom_write_block((void*)&id, (void*)node_id_address, sizeof(long));
}

/**
 * Handle a 'T' message (time sync)
 *
 * Add the node to the list of active nodes
 */
void handle_T(RF24NetworkHeader& header)
{
  // The 'T' message is just a ulong, containing the time
  unsigned long message;
  rfnetwork.read(header, &message, sizeof(unsigned long));
  printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"), millis(), message, header. from_node);

  // If this message is from ourselves or the base, don't bother adding it to the active nodes.
  if ( header.from_node != node_id || header.from_node > 00 )
    add_node(header.from_node);
}

/**
 * Handle an 'N' message, reg a new node
 */
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  rfnetwork.read(header,&incoming_nodes,sizeof(incoming_nodes));
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"),millis(),header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
 * Handle an 'S' message for sensor messages
 */
void handle_S(RF24NetworkHeader& header)
{
  S_message message;
  rfnetwork.read(header, &message, sizeof(message));
  printf_P(PSTR("%lu: APP Received %s from 0%o\n\r"), millis(), message.toString(), header.from_node);
  
  // send to web
  
  char post_string[300];
  switch (message.type)
  {
    case '1':  // Sensor for temperature and humidity
      snprintf(post_string, sizeof(post_string), "{\"node_id\":\"%o\", \"voltage\":\"%s\", \"v1\":\"%s\"}", header.from_node, message.voltage, message.v1);
      break;
      
    default:
      printf_P(PSTR("*** WARNING *** Unknown message type %c from %s\n\r"), message.type, header.from_node);
      break;
  };
  
  Serial.println("\n>>> REQ --> Get Update Sensor Message");
  ether.packetLoop(ether.packetReceive());
  ether.httpPost(PSTR("/node/update/"), webhost, "", post_string, cb_update);
  //ether.browseUrl(PSTR("/node/update/"), post_string, webhost, cb_update);
}

/**
 * Add a particular node to the current list of active nodes
 */
void add_node(uint16_t node)
{
  // Do we already know about this node?
  short i = num_active_nodes;
  while (i--)
  {
    if ( active_nodes[i] == node )
      break;
  }
  // If not, add it to the table
  if ( i == -1 && num_active_nodes < max_active_nodes )
  {
    active_nodes[num_active_nodes++] = node; 
    printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"),millis(),node);
  }
}

static void cb_update (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}

static void cb_reg (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
  // TO test: which is server return message ? buffer of off
  
  // If not reg 
}
