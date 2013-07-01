/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#ifndef __S_MESSAGE_H__
#define __S_MESSAGE_H__

// STL headers
// C headers
// Framework headers
// Library headers
// Project headers

/**
 * Sensor message (type 'S') 
 */
struct S_message
{
  uint16_t v1;
  uint16_t v2;
  uint16_t v3;
  uint16_t voltage;
  uint16_t type;
  static char buffer[];
  S_message(void): v1(0), v2(0), v3(0), voltage(0), type(0) {}
  char* toString(void);
};

struct S_message_th
{
  uint16_t temperature;
  uint16_t humidity;
  uint16_t voltage;
  static char buffer[];
  S_message_th(void): temperature(0), humidity(0), voltage(0) {}
  char* toString(void);
};

struct S_message_human
{
  uint16_t value;
  uint16_t voltage;
  static char buffer[];
  S_message_human(void): value(0), voltage(0) {}
  char* toString(void);
};

#endif // __S_MESSAGE_H__
// vim:cin:ai:sts=2 sw=2 ft=cpp
