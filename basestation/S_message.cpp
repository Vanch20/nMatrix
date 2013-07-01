/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "RF24Network_config.h"
#include "S_message.h"

/****************************************************************************/
char S_message::buffer[32];
char* S_message::toString(void)
{
  snprintf(buffer, sizeof(buffer), "%2u.%02u, %2u.%02u, ",
      voltage >> 8, ( voltage & 0xFF ) * 100 / 256,
      v1 >> 8, ( v1 & 0xFF ) * 100 / 256,
      v2 >> 8, ( v2 & 0xFF ) * 100 / 256,
      v3 >> 8, ( v3 & 0xFF ) * 100 / 256
      );
  return buffer;
};


char S_message_th::buffer[32];
char* S_message_th::toString(void)
{
  snprintf(buffer, sizeof(buffer), "%2u.%02u,%2u.%02u",
      voltage >> 8,
      ( voltage & 0xFF ) * 100 / 256,
       temperature >> 8,
      ( temperature & 0xFF ) * 100 / 256
      );
  return buffer;
}


char S_message_human::buffer[32];
char* S_message_human::toString(void)
{
  snprintf(buffer, sizeof(buffer), "%2u.%02u,%3u",
      voltage >> 8,
      ( voltage & 0xFF ) * 100 / 256,
      value
      );
  return buffer;
}

/****************************************************************************/

