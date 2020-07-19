/* $Id$
 *
 * Copyright   : (c) 2020 by Denis Dowling.  All Rights Reserved
 * Project     : Open Source Solutions
 * File        : test_led.cpp
 *
 * Author      : Denis Dowling
 * Created     : 10/7/2020
 *
 * Description : Test the interface to the TM1628 LED and keyboard driver
 */
#include <Arduino.h>
#include "TM1628.h"
#include "yihua898D.h"

// define a tm1628 module
TM1628 tm1628(DATA_PIN, SCLK_PIN, STB_PIN);

const int led_intensity = 2;
const int led_port = 13;

void setup()
{
    tm1628.begin(ON, led_intensity);

    tm1628.showStr(DISP_2,"HEL");
    tm1628.showStr(DISP_1,"LO");
    tm1628.update();
    delay(1000);

    pinMode(led_port, OUTPUT);
}

void loop()
{
    uint32_t t = millis();
    tm1628.showNum(DISP_2, t/1000);
    tm1628.showNum(DISP_1, t%1000);
    tm1628.update();

    // One second flash of led
    digitalWrite(led_port, ((t/500) % 2 == 0) );
}
