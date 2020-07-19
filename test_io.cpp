/* $Id$
 *
 * Copyright   : (c) 2020 by Denis Dowling.  All Rights Reserved
 * Project     : Open Source Solutions
 * File        : test_io.cpp
 *
 * Author      : Denis Dowling
 * Created     : 18/7/2020
 *
 * Description : Exercise the IO ports for the YH898D iron
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
    pinMode(led_port, OUTPUT);

    Serial.begin(9600);
    Serial.println("\nRESET");

    tm1628.begin(ON, led_intensity);

    tm1628.showStr(DISP_2,"RES");
    tm1628.showStr(DISP_1,"ET");
    tm1628.update();

    pinMode(led_port, OUTPUT);

    HA_HEATER_INIT;
    HA_HEATER_OFF;

    SI_HEATER_INIT;
    SI_HEATER_OFF;

    FAN_INIT;
    FAN_OFF;

    REEDSW_INIT;
    HA_SW_INIT;
    SI_SW_INIT;

    pinMode(HA_TEMP_PIN, INPUT);
    pinMode(SI_TEMP_PIN, INPUT);

#ifdef CURRENT_SENSE_MOD
    pinMode(FAN_CURRENT_PIN, INPUT);  // set as input
#elif defined(SPEED_SENSE_MOD)
    pinMode(FAN_SPEED_PIN, INPUT);  // set as input
#endif

    // use external 3.3V (i.e. from Arduino pin) as ADC reference voltage
    analogReference(EXTERNAL);
}

bool reed = false;
bool ha_sw = false;
bool si_sw = false;

bool fan = false;
bool ha = false;
bool si = false;

void loop()
{
    uint32_t t = millis();
    tm1628.showNum(DISP_2, t/1000);
    tm1628.showNum(DISP_1, t%1000);
    tm1628.update();

    bool new_reed = REEDSW_OPEN;
    if (new_reed != reed)
    {
        reed = new_reed;
        if (reed)
            Serial.println("Reed open, Hot Air not in holster");
        else
            Serial.println("Reed closed, Hot Air in holster");
    }

    bool new_ha_sw = HA_SW_ON;
    if (new_ha_sw != ha_sw)
    {
        ha_sw = new_ha_sw;
        if (ha_sw)
            Serial.println("Hot Air Switch On");
        else
            Serial.println("Hot Air Switch Off");
    }

    bool new_si_sw = SI_SW_ON;
    if (new_si_sw != si_sw)
    {
        si_sw = new_si_sw;
        if (si_sw)
            Serial.println("Soldering Iron Switch On");
        else
            Serial.println("Soldering Iron Switch Off");
    }

    if (Serial.available() > 0)
    {
        int c = Serial.read();

        if (c == 'h')
        {
            ha = !ha;
            if (ha)
            {
                Serial.println("Hot Air Heater On");
                HA_HEATER_ON;
            }
            else
            {
                Serial.println("Hot Air Heater Off");
                HA_HEATER_OFF;
            }
        }
        else if (c == 's')
        {
            si = !si;
            if (si)
            {
                Serial.println("Soldering Iron Heater On");
                SI_HEATER_ON;
            }
            else
            {
                Serial.println("Soldering Iron Heater Off");
                SI_HEATER_OFF;
            }
        }
        else if (c == 'f')
        {
            fan = !fan;
            if (fan)
            {
                Serial.println("Fan On");
                FAN_ON;
            }
            else
            {
                Serial.println("Fan Off");
                FAN_OFF;
            }
        }
        else
        {
            // Any other character will sample the analogue io
            int iron_temp = analogRead(SI_TEMP_PIN);
            int hot_air_temp = analogRead(HA_TEMP_PIN);

            Serial.print("Iron Temp = ");
            Serial.print(iron_temp);
            Serial.print(" Hot Air Temp = ");
            Serial.print(hot_air_temp);
            Serial.println();
        }
    }

    // One second flash of led
    digitalWrite(led_port, ((t/500) % 2 == 0) );
}
