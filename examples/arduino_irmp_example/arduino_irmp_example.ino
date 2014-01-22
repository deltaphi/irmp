/* This is a simple example for using the IRMP library
 * original home of IRMP is:  http://www.mikrocontroller.net/articles/IRMP
 * arduino port of IRMP home: https://gitorious.org/arduino-addons/irmp-arduino
 *
 * (C) 2012 Stefan Seyfried
 * (C) 2014 Falko Thomale
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. 
 */

/* use TimerOne http://arduino.cc/playground/Code/Timer1 for interrupts */
#include <TimerOne.h>
/* first include Arduino.h, the IDE includes it after irmp*.h ... */
#include "Arduino.h"
/* ... and then chokes on uintX_t ... */

extern "C" {
#include <irmp.h>
#include <irsnd.h>
}

/* undefine this if you don't want blinking LED for diagnosis */
#define LED_PIN 4
#define SER_BAUD 9600

/* F_INTERRUPTS is the interrupt frequency defined in irmpconfig.h */
#define US (1000000 / F_INTERRUPTS)
void setup()
{
  Serial.begin(SER_BAUD);
  /* greeting string and debugging ouput */
  Serial.println("IRMP test sketch");
  Serial.print("US: ");
  Serial.println(US);
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
#endif
  //irmp_init();
  irsnd_init();
  //sei();
  led(HIGH);
  delay(20); /* make sure the greeting string is out before starting */
  led(LOW);
  Timer1.initialize(US);
  Timer1.attachInterrupt(timerinterrupt);
}

IRMP_DATA irmp_data;
int incomingByte = 0;   // for incoming serial data
void loop()
{
  if (irmp_get_data(&irmp_data))
  {
    led(HIGH);
    Serial.print("P:");
    Serial.print(irmp_data.protocol, HEX);
    Serial.print(" A:");
    Serial.print(irmp_data.address, HEX);
    Serial.print(" C:");
    Serial.print(irmp_data.command, HEX);
    Serial.print(" ");
    Serial.print(irmp_data.flags, HEX);
    Serial.println("");
    /* Serial.print is asynchronous, so the LED is only flashing very lightly */
    led(LOW);

    irmp_data.flags = 0;    // reset flags!
    int result = irsnd_send_data(&irmp_data, TRUE);
    if (result != 1)
      Serial.println("ERROR");
  }

  if (Serial.available() == 18 && readAndCheck('P') && readAndCheck(':')) {
    // read the protocol
    irmp_data.protocol = readHex() * 16 + readHex();

    if (readAndCheck(' ') && readAndCheck('A') && readAndCheck(':')) {
      // read the address
      irmp_data.address = ((readHex() * 16 + readHex()) * 16 + readHex()) * 16 + readHex();

      if (readAndCheck(' ') && readAndCheck('C') && readAndCheck(':')) {
        // read the address
        irmp_data.command = ((readHex() * 16 + readHex()) * 16 + readHex()) * 16 + readHex();

        // send ir data
        irmp_data.flags = 0;
        int result = irsnd_send_data(&irmp_data, TRUE);
        if (result)
        {
          Serial.println("Send IR data: ");
          Serial.print("P:");
          Serial.print(irmp_data.protocol, HEX);
          Serial.print(" A:");
          Serial.print(irmp_data.address, HEX);
          Serial.print(" C:");
          Serial.print(irmp_data.command, HEX);
          Serial.println("");
        }
      }
    }
  }
}

/* helper function: attachInterrupt wants void(), but irmp_ISR is uint8_t() */
void timerinterrupt()
{
  if (! irsnd_ISR())          // call irsnd ISR
  {                           // if not busy...
    irmp_ISR();               // call irmp ISR
  }
}

static inline void led(int state)
{
#ifdef LED_PIN
  digitalWrite(LED_PIN, state);
#endif
}

static inline int readAndCheck(int c)
{
  return c == Serial.read();
}

static inline int readHex()
{
  int c = Serial.read();
  if (c >= '0' && c <= '9')
  {
    return c - '0';
  }
  else if (c >= 'a' && c <= 'f')
  {
    return c + 10 - 'a';
  }
  else if (c >= 'A' && c <= 'F')
  {
    return c + 10 - 'A';
  }

  return -1;
}
