#include "messages.h"

char message_dht11::buffer[32];
char* message_dht11::toSendString(void)
{
  sprintf(buffer, "%.1f,%.1f", temperature, humidity);
  return buffer;
};
char* message_dht11::toString(void)
{
  sprintf(buffer, "Temperature:%.1f, Humidity: %.1f", temperature, humidity);
  return buffer;
};
