/* $Id$
 *
 * Copyright   : (c) 2012 by Denis Dowling.  All Rights Reserved
 * Project     : Open Source Solutions
 * File        : test_led.cpp
 *
 * Author      : Denis Dowling
 * Created     : 8/4/2013
 *
 * Description : Simple program to toggle the LED on the standard Arduino
 * port D13 (PB5).
 */
#include <Arduino.h>

const int led_port = 13;
bool led = false;

void setup()
{
    pinMode(led_port, OUTPUT);

    digitalWrite(led_port, HIGH);
}

void loop()
{
    if (!led)
    {
	digitalWrite(led_port, HIGH);
	led = true;
    }
    else
    {
	digitalWrite(led_port, LOW);
	led = false;
    }

    delay(100);
}
