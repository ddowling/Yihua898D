/*
 * This is a custom firmware for my 'Yihua 898D' hot-air soldering station.
 * It may or may not be useful to you, always double check if you use it.
 *
 * V2.5.0
 *
 * 2018    - Karol Hirsz
 * 2015/16 - Robert Spitzenpfeil
 * 2015    - Moritz Augsburger
 *
 * License: GNU GPL v2
 *
 *
 * Developed for / tested on by Robert Spitzenpfeil:
 * -------------------------------------------------
 *
 * Date:  2015-02-01
 * PCB version: 858D V6.0
 * Date code:   20140415
 *
 * Developed for / tested on by Moritz Augsburger:
 * -----------------------------------------------
 *
 * Date:  2015-02-01
 * PCB version: 858D V6.0
 * Date code:   20140415
 *
 * Reported to work with (I did not test these myself):
 * ----------------------------------------------------
 *
 * PCB version: 858D V4.3
 * Date code:   20130529
 * HW mods:     not tested!
 *
 * ---
 *
 * PCB version: 858D V4.10
 * Date code: 20140112
 * HW mods: not tested!
 *
 */

/*
 *  Make sure to read and understand '/Docs/modes_of_operation.txt'
 *
 * Change options in the .h file
 *
 */

#define FW_MAJOR_V 2
#define FW_MINOR_V_A 5
#define FW_MINOR_V_B 0
/*
 * #21: AREF <--- about 3.3V as analogue reference for ADC, i.e. from Arduino 3.3V pin.
 */

#include <avr/io.h>
#ifdef USE_WATCHDOG
#include <avr/wdt.h>
#endif
#include <avr/interrupt.h>
#include <stdint.h>
#include <EEPROM.h>
#include "TM1628.h"
#include "yihua898D.h"

#ifdef USE_WATCHDOG
/* allocate memory for a signature string which will be stored in RAM after
 *  powerup  */
const char wdt_signature [] = "WDT_RESET";
char * p = (char *) malloc(sizeof(wdt_signature));
#endif

// define a tm1628 module
TM1628 tm1628(DATA_PIN, SCLK_PIN, STB_PIN);

// HOT AIR configuration
DEV_CFG ha_cfg = {
  /* device type */        DEV_HA,
  /* display number */     DISP_2,
  /* p_gain */             { 0, 999, P_GAIN_DEFAULT_HA, P_GAIN_DEFAULT_HA, 2, 3, "P"},  // min, max, default, value, eep_addr_high, eep_addr_low, name
  /* p_scal */             { -128, 127, P_SCALING_DEFAULT_HA, P_SCALING_DEFAULT_HA, 4, 5, "PSL"}, // (p_gain x 10^p_scal)
  /* i_gain */             { 0, 999, I_GAIN_DEFAULT_HA, I_GAIN_DEFAULT_HA, 6, 7, "I"},
  /* i_scal */             { -128, 127, I_SCALING_DEFAULT_HA, I_SCALING_DEFAULT_HA, 8, 9, "ISL"}, // (i_gain x 10^i_scal)
  /* d_gain */             { 0, 999, D_GAIN_DEFAULT_HA, D_GAIN_DEFAULT_HA, 10, 11, "d"},
  /* d_scal */             { -128, 127, D_SCALING_DEFAULT_HA, D_SCALING_DEFAULT_HA, 12, 13, "dSL"}, // (d_gain x 10^d_scal)
  /* i_thresh */           { 0, 100, I_THRESH_DEFAULT_HA, I_THRESH_DEFAULT_HA, 14, 15, "ItH"},
  /* temp_gain_int_corr */ { 0, 999, TEMP_GAIN_INT_CORR_DEFAULT_HA, TEMP_GAIN_INT_CORR_DEFAULT_HA, 16, 17, "tgI"},
  /* temp_gain_dec_corr */ { 0, 999, TEMP_GAIN_DEC_CORR_DEFAULT_HA, TEMP_GAIN_DEC_CORR_DEFAULT_HA, 18, 19, "tgd"},
  /* temp_offset_corr */   { -999, 999, TEMP_OFFSET_CORR_DEFAULT_HA, TEMP_OFFSET_CORR_DEFAULT_HA, 20, 21, "toF"},
  /* temp_averages */      { 100, 999, TEMP_AVERAGES_DEFAULT, TEMP_AVERAGES_DEFAULT, 22, 23, "Avg"},
  /* slp_timeout */        { 0, 30, SLP_TIMEOUT_DEFAULT, SLP_TIMEOUT_DEFAULT, 24, 25, "SLP"},
  /* display_adc_raw */    { 0, 1, 0, 0, 26, 27, "Adc"},
#ifdef CURRENT_SENSE_MOD
  /* fan_current_min */    { 0, 999, FAN_CURRENT_MIN_DEFAULT, FAN_CURRENT_MIN_DEFAULT, 32, 33, "FcL"},
  /* fan_current_max */    { 0, 999, FAN_CURRENT_MAX_DEFAULT, FAN_CURRENT_MAX_DEFAULT, 34, 35, "FcH"},
#elif defined(SPEED_SENSE_MOD)
  //
  // See yihua898d.h if you want to use the 'FAN-speed mod' (HW changes required)
  // The following 2 CPARAM lines need changes in that case
  //
  /* fan_speed_min */      { 120, 180, FAN_SPEED_MIN_DEFAULT, FAN_SPEED_MIN_DEFAULT, 36, 37, "FSL"},
  /* fan_speed_max */      { 300, 400, FAN_SPEED_MAX_DEFAULT, FAN_SPEED_MAX_DEFAULT, 38, 39, "FSH"},
#endif
  // Not configurable in setting change mode
  /* temp_setpoint */      { 80, 500, TEMP_SETPOINT_DEFAULT, TEMP_SETPOINT_DEFAULT, 28, 29, "StP"},
  /* fan_only */           { 0, 1, 0, 0, 30, 31, "FnO"},
};
CPARAM * ha_set_order[] = {&ha_cfg.p_gain, &ha_cfg.p_scal, &ha_cfg.i_gain, &ha_cfg.i_scal,
                                      &ha_cfg.d_gain, &ha_cfg.d_scal, &ha_cfg.i_thresh,
                                      &ha_cfg.temp_gain_int_corr, &ha_cfg.temp_gain_dec_corr, &ha_cfg.temp_offset_corr,
                                      &ha_cfg.temp_averages, &ha_cfg.slp_timeout, &ha_cfg.display_adc_raw,
#ifdef CURRENT_SENSE_MOD
                                      &ha_cfg.fan_current_min, &ha_cfg.fan_current_max,
#elif defined(SPEED_SENSE_MOD)
                                      &ha_cfg.fan_speed_min, &ha_cfg.fan_speed_max,
#endif
                                      };
CNTRL_STATE ha_state;

// SOLDERING IRON configuration
DEV_CFG si_cfg = {
  /* device type */        DEV_SI,
  /* display number */     DISP_1,
  /* p_gain */             { 0, 999, P_GAIN_DEFAULT_SI, P_GAIN_DEFAULT_SI, 100, 101, "P"},  // min, max, default, value, eep_addr_high, eep_addr_low, name
  /* p_scal */             { -128, 127, P_SCALING_DEFAULT_SI, P_SCALING_DEFAULT_SI, 102, 103, "PSL"}, // (p_gain x 10^p_scal)
  /* i_gain */             { 0, 999, I_GAIN_DEFAULT_SI, I_GAIN_DEFAULT_SI, 104, 105, "I"},
  /* i_scal */             { -128, 127, I_SCALING_DEFAULT_SI, I_SCALING_DEFAULT_SI, 106, 107, "ISL"}, // (i_gain x 10^i_scal)
  /* d_gain */             { 0, 999, D_GAIN_DEFAULT_SI, D_GAIN_DEFAULT_SI, 108, 109, "d"},
  /* d_scal */             { -128, 127, D_SCALING_DEFAULT_SI, D_SCALING_DEFAULT_SI, 110, 111, "dSL"}, // (d_gain x 10^d_scal)
  /* i_thresh */           { 0, 100, I_THRESH_DEFAULT_SI, I_THRESH_DEFAULT_SI, 112, 113, "ItH"},
  /* temp_gain_int_corr */ { 0, 999, TEMP_GAIN_INT_CORR_DEFAULT_SI, TEMP_GAIN_INT_CORR_DEFAULT_SI, 114, 115, "tgI"},
  /* temp_gain_dec_corr */ { 0, 999, TEMP_GAIN_DEC_CORR_DEFAULT_SI, TEMP_GAIN_DEC_CORR_DEFAULT_SI, 116, 117, "tgd"},
  /* temp_offset_corr */   { -999, 999, TEMP_OFFSET_CORR_DEFAULT_SI, TEMP_OFFSET_CORR_DEFAULT_SI, 118, 119, "toF"},
  /* temp_averages */      { 100, 999, TEMP_AVERAGES_DEFAULT, TEMP_AVERAGES_DEFAULT, 120, 121, "Avg"},
  /* slp_timeout */        CPARAM_NULL,
  /* display_adc_raw */    { 0, 1, 0, 0, 122, 123, "Adc"},
#ifdef CURRENT_SENSE_MOD
  /* fan_current_min */    CPARAM_NULL,
  /* fan_current_max */    CPARAM_NULL,
#elif defined(SPEED_SENSE_MOD)
  /* fan_speed_min */      CPARAM_NULL,
  /* fan_speed_max */      CPARAM_NULL,
#endif
  // Not configurable in setting change mode
  /* temp_setpoint */      { 80, 500, TEMP_SETPOINT_DEFAULT, TEMP_SETPOINT_DEFAULT, 124, 125, "StP"},
  /* fan_only */           CPARAM_NULL,
};
CPARAM * si_set_order[] = {&si_cfg.p_gain, &si_cfg.p_scal, &si_cfg.i_gain, &si_cfg.i_scal,
                                      &si_cfg.d_gain, &si_cfg.d_scal, &si_cfg.i_thresh,
                                      &si_cfg.temp_gain_int_corr, &si_cfg.temp_gain_dec_corr, &si_cfg.temp_offset_corr,
                                      &si_cfg.temp_averages, &si_cfg.display_adc_raw,};
CNTRL_STATE si_state;

volatile uint8_t key_state = 0;  // debounced and inverted key state: bit = 1: key pressed
volatile uint8_t key_state_l = 0; // key long press
volatile uint8_t key_state_s = 0; // key short press
volatile uint8_t sw_state = 0; // debounced switch state: bit = 1: sw on

static float scale10(int16_t x, int8_t scaling)
{
  float r = (float) x;
  if (scaling > 0) {
    while (scaling--)
       r *= 10.0;
  } else if (scaling < 0) {
    scaling = -scaling;
    while (scaling--)
       r /= 10.0;
  }

  return r;
}

void setup()
{
    setup_HW();

#ifdef USE_WATCHDOG
    watchdog_off();
#endif

    init_state(&ha_state);
    init_state(&si_state);

#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("\nRESET");
#endif

    tm1628.begin(ON, LED_POW);

    if (EEPROM.read(0) != 0x22) {
        // check if the firmware was just flashed and the EEPROM is therefore empty
        // assumption: full chip erase with ISP programmer (preserve eeprom fuse NOT set!)
        // if so, restore default parameter values & write a 'hint' to address 0
        restore_default_conf();
        EEPROM.write(0, 0x22);
#ifdef DEBUG
        Serial.println("Default config loaded");
#endif
    }

    key_scan();

    if (get_key_state(KEY_DOWN) && get_key_state(KEY_UP)) {
        restore_default_conf();
#ifdef DEBUG
        Serial.println("Default config loaded");
#endif
    } else if (get_key_state(KEY_UP)) {
        tm1628.showStr(DISP_2,"FAn");
        delay(1000);
        tm1628.showStr(DISP_2,"tSt");
        delay(1000);
        FAN_ON;
        while (1) {
            uint16_t fan;
            delay(500);
#ifdef CURRENT_SENSE_MOD
            fan = analogRead(FAN_CURRENT_PIN);
            tm1628.showNum(DISP_2,fan);
#elif defined(SPEED_SENSE_MOD)
            fan = analogRead(FAN_SPEED_PIN);
            tm1628.showNum(DISP_2,fan);
#endif        //CURRENT_SENSE_MOD

        }
    }

    load_cfg();

#ifdef USE_WATCHDOG
    if (watchdog_check())
    {
        // there was a watchdog reset - should never ever happen
        HEATERS_OFF;
        FAN_ON;
#ifdef DEBUG
        Serial.println("WDT reset!");
#endif
        while (1) {
            tm1628.showStr(DISP_1,"rSt");
            delay(1000);
            tm1628.clear(DISP_1);
            delay(1000);
            key_scan();
            if (get_key_state(KEY_ENTER)) {
                break;
            }
#ifdef DEBUG
            break;
#endif
        }
    }
#endif

    show_firmware_version();

#ifdef USE_WATCHDOG
    test_F_CPU_with_watchdog();
    watchdog_on();
#endif

    key_event_clear();

    pinMode(LED_PORT, OUTPUT);
}

static uint32_t button_scan_time = 0;
static bool led_state = false;

void loop()
{

#ifdef USE_WATCHDOG
    wdt_reset();
#endif

    if (millis() - button_scan_time > BUTTON_SCANN_CYCLE)
    {
        key_scan();
        button_scan_time = millis();

        led_state = !led_state;
        digitalWrite(LED_PORT, led_state);
    }

    tm1628.update();

#if defined(WATCHDOG_TEST) && defined(USE_WATCHDOG)
    // watchdog test
    if (ha_state.temp_average > 100)
      delay(150);
#endif

    // Control HA
    dev_cntrl(&ha_cfg, &ha_state);

    // Control SI
    dev_cntrl(&si_cfg, &si_state);

    // UI
    UI_hndl();
}

void dev_cntrl(DEV_CFG *pDev_cfg, CNTRL_STATE *pDev_state)
{
    uint8_t sw_mask;
    uint8_t analog_pin;
#ifdef DEBUG
    int32_t start_time = micros();
#endif
    if (pDev_cfg->dev_type == DEV_HA) {
        // HA device
        sw_mask = HA_SW;
        analog_pin = HA_TEMP_PIN;
    } else {
        // SI device
        sw_mask = SI_SW;
        analog_pin = SI_TEMP_PIN;
    }

    if (!pDev_state->enabled && get_sw_state(sw_mask))
    {
        // Enable device
        pDev_state->enabled = 1;
#ifdef DEBUG
        if (pDev_cfg->dev_type == DEV_HA) {
            Serial.println("HA on!");
        }
        else
        {
            Serial.println("SI on!");
        }
#endif
        // Clear old presses
        key_event_clear();
        if (pDev_cfg->dev_type == DEV_HA) { // Only for HA
            pDev_state->test_state = HA_test(TEST_INIT);
        }
    }
    else if (pDev_state->enabled && !get_sw_state(sw_mask))
    {
        // Disable Device
        pDev_state->enabled = 0;
        if (pDev_cfg->dev_type == DEV_HA) {
            HA_HEATER_OFF;
            FAN_OFF;
        } else {
            SI_HEATER_OFF;
        }
#ifdef DEBUG
        if (pDev_cfg->dev_type == DEV_HA) {
            Serial.println("HA off!");
        } else {
            Serial.println("SI off!");
        }
#endif
        tm1628.clear(pDev_cfg->disp_n);
        // Clear state
        init_state(pDev_state);
    }

    if (pDev_state->enabled)
    {
        if (pDev_cfg->dev_type == DEV_HA && pDev_state->test_state != TEST_ALL_OK)
        { //Startup test is still running
            pDev_state->test_state = HA_test(pDev_state->test_state);
            return;
        }
        pDev_state->adc_raw = analogRead(analog_pin); // need raw value later, store it here and avoid 2nd ADC read
#ifdef DEBUG
        if (start_time % 1000 == 0)
        {
            if (pDev_cfg->dev_type == DEV_HA) {
                Serial.print("HA adc=");
            } else {
                Serial.print("SI adc=");
            }
            Serial.print(pDev_state->adc_raw);
            Serial.println();
        }
#endif

        if (pDev_state->adc_raw < ADC_TEMP_ZERO) { // Set temperature to zero for low ADC values (needed with temperature offset)
            pDev_state->temp_inst = 0;
        } else {
            pDev_state->temp_inst = pDev_state->adc_raw * pDev_cfg->temp_gain_int_corr.value
                + ((int16_t)((((int32_t)pDev_state->adc_raw) * ((int32_t)pDev_cfg->temp_gain_dec_corr.value)) / ((int32_t)1000)))
                + pDev_cfg->temp_offset_corr.value;  // approx. temp in °C
        }

        if (pDev_state->temp_inst < 0) {
            pDev_state->temp_inst = 0;
        }
        // pid loop / heater handling
        if (pDev_cfg->dev_type == DEV_HA && (pDev_cfg->fan_only.value == 1 || REEDSW_CLOSED)) { // Only for HA
            HA_HEATER_OFF;
            //TODO
            pDev_state->heater_start_time = millis();
            tm1628.clearDot(pDev_cfg->disp_n,0);
        } else if ((pDev_cfg->dev_type == DEV_SI || REEDSW_OPEN)
                   && (pDev_cfg->temp_setpoint.value >= pDev_cfg->temp_setpoint.value_min)
                   && (pDev_state->temp_average < MAX_TEMP_ERR)
                   && (pDev_cfg->dev_type == DEV_SI || ((millis() - pDev_state->heater_start_time) < ((uint32_t) (pDev_cfg->slp_timeout.value) * 60 * 1000)))) {
            // Run PID regulation
            if (pDev_cfg->dev_type == DEV_HA) {
                FAN_ON;
            }

            pDev_state->error = pDev_cfg->temp_setpoint.value - pDev_state->temp_average;
            pDev_state->velocity = pDev_state->temp_average_previous - pDev_state->temp_average;

            if (abs(pDev_state->error) < pDev_cfg->i_thresh.value) {
                // if close enough to target temperature use PID control
                pDev_state->error_accu += pDev_state->error;
            } else {
                // otherwise only use PD control (avoids issues with pDev_state->error_accu growing too large
                pDev_state->error_accu = 0;
            }

            pDev_state->PID_drive =
                pDev_state->error * scale10(pDev_cfg->p_gain.value, pDev_cfg->p_scal.value) +
                pDev_state->error_accu * scale10(pDev_cfg->i_gain.value, pDev_cfg->i_scal.value) +
                pDev_state->velocity * scale10(pDev_cfg->d_gain.value, pDev_cfg->d_scal.value);

            // Check range before casting to avoid int16 overflow
            if (pDev_state->PID_drive > (float) HEATER_DUTY_CYCLE_MAX) {
                pDev_state->PID_drive = (float) HEATER_DUTY_CYCLE_MAX;
            }

            if (pDev_state->PID_drive < 0.0) {
                pDev_state->PID_drive = 0.0;
            }

            pDev_state->heater_duty_cycle = (int16_t) (pDev_state->PID_drive);

            if (pDev_state->heater_ctr < pDev_state->heater_duty_cycle) {
                tm1628.setDot(pDev_cfg->disp_n,0);
                if (pDev_cfg->dev_type == DEV_HA) {
                    HA_HEATER_ON;
                } else {
                    SI_HEATER_ON;
                }
            } else {
                if (pDev_cfg->dev_type == DEV_HA) {
                    HA_HEATER_OFF;
                } else {
                    SI_HEATER_OFF;
                }
                tm1628.clearDot(pDev_cfg->disp_n,0);
            }

            pDev_state->heater_ctr++;
            if (pDev_state->heater_ctr == PWM_CYCLES) {
                pDev_state->heater_ctr = 0;
            }
        } else {
            if (pDev_cfg->dev_type == DEV_HA) {
                HA_HEATER_OFF;
            } else {
                SI_HEATER_OFF;
            }
            tm1628.clearDot(pDev_cfg->disp_n,0);
        }

        pDev_state->temp_accu += pDev_state->temp_inst;
        pDev_state->temp_avg_ctr++;

        if (pDev_state->temp_avg_ctr == (uint16_t) (pDev_cfg->temp_averages.value)) {
            pDev_state->temp_average_previous = pDev_state->temp_average;
            pDev_state->temp_average = pDev_state->temp_accu / pDev_cfg->temp_averages.value;
            pDev_state->temp_accu = 0;
            pDev_state->temp_avg_ctr = 0;
        }
        // fan/cradle handling
        if (pDev_cfg->dev_type == DEV_HA) { // HA only
            if (REEDSW_OPEN) {
                FAN_ON;
            } else if (pDev_state->temp_average >= FAN_ON_TEMP) {
                FAN_ON;
            } else if (REEDSW_CLOSED && (pDev_state->temp_average <= FAN_OFF_TEMP)) {
                FAN_OFF;
            }
        }

        // security first!
        if (pDev_state->temp_average >= MAX_TEMP_ERR) {
            // something might have gone terribly wrong
            if (pDev_cfg->dev_type == DEV_HA) {
                HA_HEATER_OFF;
                FAN_ON;
            } else {
                SI_HEATER_OFF;
            }
            // TODO
            HEATERS_OFF;
#ifdef USE_WATCHDOG
            watchdog_off();
#endif
#ifdef DEBUG
            if (pDev_cfg->dev_type == DEV_HA) {
                Serial.println("Hot air temperature meas. error!");
            } else {
                Serial.println("Soldering iron temperature meas. error!");
            }
#endif
            while (1) {
                // stay here until the power is cycled
                // make sure the user notices the error by blinking "FAn"
                // and don't resume operation if the error goes away on its own
                //
                // possible reasons to be here:
                //
                // * wand is not connected (false temperature reading)
                // * thermo couple has failed
                // * true over-temperature condition
                //
                tm1628.showStr(pDev_cfg->disp_n,"*C");
                delay(1000);
                tm1628.showStr(pDev_cfg->disp_n,"Err");
                delay(2000);
                tm1628.clear(pDev_cfg->disp_n);
                delay(1000);
            }
        }

#ifdef DEBUG
        int32_t stop_time = micros();
        if (start_time % 1000 == 0)
        {
            if (pDev_cfg->dev_type == DEV_HA) {
                Serial.print("HA Loop time: ");
            } else {
                Serial.print("SI Loop time: ");
            }
            Serial.println(stop_time - start_time);
        }
#endif
    }
}

void UI_hndl(void)
{
  static uint8_t sp_mode = 0;
  static uint32_t blink_time = millis();
  static uint8_t blink_state = 0;
  uint8_t HA_start;
  DEV_CFG *pDev_cfg = NULL;

  // Blinking feature
  if (millis() - blink_time > BLINK_CYCLE) {
    blink_time = millis();
    if (++blink_state > BLINK_STATE_MAX) {
      blink_state = 0;
    }
  }

  if (ha_state.enabled && ha_state.test_state == TEST_ALL_OK) {
    HA_start = 1;
  } else {
    HA_start = 0;
  }

  if (!HA_start && !si_state.enabled) {
    sp_mode = 0;
    if (ha_state.enabled) {
      // Configuration mode is possible in test state of HA
      if (get_key_event_long(KEY_UP | KEY_DOWN)) {
        config_mode();
      }
    }
    // Nothing to do
    return;
  } else if (!HA_start && sp_mode == DEV_HA) { // Device is disabled now
    sp_mode = 0;
  } else if (!si_state.enabled && sp_mode == DEV_SI) { // Device is disabled now
    sp_mode = 0;
  }

  // menu key handling
  if (get_key_event(KEY_ENTER)) {
    sp_mode++;
    if (sp_mode == DEV_HA && !HA_start) {
      //HA disabled, skip this state
      sp_mode++;
    } else if (sp_mode == DEV_SI && !si_state.enabled) {
      //SI disabled, skip this state
      sp_mode++;
    }
    if (sp_mode > DEV_SI) {
      sp_mode = 0;
      eep_save(&ha_cfg.temp_setpoint);
      eep_save(&ha_cfg.fan_only);
      eep_save(&si_cfg.temp_setpoint);
    }
  }
  // Select configuration for sp mode
  if (sp_mode == DEV_HA) {
    pDev_cfg = &ha_cfg;
  } else if (sp_mode == DEV_SI) {
    pDev_cfg = &si_cfg;
  }

  if (sp_mode) {
    if (get_key_event_short(KEY_UP | KEY_DOWN)) {// Fan only mode
      if (sp_mode == DEV_HA) {
        pDev_cfg->fan_only.value ^= 0x01;
      }
    } else if (get_key_event_short(KEY_UP)) {
      if (pDev_cfg->temp_setpoint.value < pDev_cfg->temp_setpoint.value_max) {
        pDev_cfg->temp_setpoint.value++;
      }
    } else if (get_key_event_short(KEY_DOWN)) {
      if (pDev_cfg->temp_setpoint.value > pDev_cfg->temp_setpoint.value_min) {
        pDev_cfg->temp_setpoint.value--;
      }
    } else if (get_key_event_long(KEY_UP)) {
      if (pDev_cfg->temp_setpoint.value < (pDev_cfg->temp_setpoint.value_max - 10)) {
        pDev_cfg->temp_setpoint.value += 10;
      } else {
        pDev_cfg->temp_setpoint.value = pDev_cfg->temp_setpoint.value_max;
      }

    } else if (get_key_event_long(KEY_DOWN)) {

      if (pDev_cfg->temp_setpoint.value > (pDev_cfg->temp_setpoint.value_min + 10)) {
        pDev_cfg->temp_setpoint.value -= 10;
      } else {
        pDev_cfg->temp_setpoint.value = pDev_cfg->temp_setpoint.value_min;
      }
    }

    // Display
    if (blink_state > 7) {
      if (sp_mode == DEV_HA && pDev_cfg->fan_only.value == 1) {
        tm1628.showStr(pDev_cfg->disp_n,"FAn");
      } else {
        tm1628.clear(pDev_cfg->disp_n);
      }
    } else {
      tm1628.showNum(pDev_cfg->disp_n, pDev_cfg->temp_setpoint.value);  // show temperature setpoint
    }
  }

  //Normal operation display
  if (sp_mode == DEV_HA) {
    temperature_display(&si_cfg, &si_state, blink_state);
  } else if (sp_mode == DEV_SI) {
    temperature_display(&ha_cfg, &ha_state, blink_state);
  } else if (!sp_mode) {
    // Not in sp mode
    temperature_display(&ha_cfg, &ha_state, blink_state);
    temperature_display(&si_cfg, &si_state, blink_state);

    // Configuration mode
    if (get_key_event_long(KEY_UP | KEY_DOWN)) {
      config_mode();
    }
  }
}

void temperature_display(DEV_CFG *pDev_cfg, CNTRL_STATE *pDev_state, uint8_t blink_state)
{
  if (!pDev_state->enabled || pDev_state->test_state != TEST_ALL_OK) {
    return;
  }

  if (pDev_state->temp_average <= DISPLAY_OFF_TEMP) {
    pDev_state->temp_disp_on = 0;
  } else if (pDev_state->temp_average >= DISPLAY_ON_TEMP) {
    pDev_state->temp_disp_on = 1;
  }

  if (!pDev_state->temp_disp_on) {
    if (pDev_cfg->dev_type == DEV_HA && pDev_cfg->fan_only.value == 1) {
      tm1628.showStr(pDev_cfg->disp_n, "FAn");
    } else {
      tm1628.showStr(pDev_cfg->disp_n, "---");
    }
  } else if (pDev_cfg->dev_type == DEV_HA && pDev_cfg->fan_only.value == 1) {
    if (blink_state < 5) {
      tm1628.showStr(pDev_cfg->disp_n, "FAn");
    } else {
      tm1628.showNum(pDev_cfg->disp_n, pDev_state->temp_average);
    }
  } else if (pDev_cfg->display_adc_raw.value == 1) {
    if ((blink_state % 5) == 0) { // Avoid too fast changes
      tm1628.showNum(pDev_cfg->disp_n, pDev_state->adc_raw);
    }
  } else if (abs((int16_t) (pDev_state->temp_average) - (int16_t) (pDev_cfg->temp_setpoint.value)) < TEMP_REACHED_MARGIN) {
    tm1628.showNum(pDev_cfg->disp_n, pDev_cfg->temp_setpoint.value);  // avoid showing insignificant fluctuations on the display (annoying)
  } else {
    tm1628.showNum(pDev_cfg->disp_n, pDev_state->temp_average);
  }
}

void config_mode(void)
{
  uint32_t button_scan_time = 0;
  uint32_t blink_time = millis();
  uint8_t blink_state = 0;
  uint8_t param_num = 0;
  uint8_t mode = MODE_DEV_SEL;
  uint8_t disp = 0;
  uint8_t dev_type = DEV_HA;
  uint8_t param_max_num = 0;
  CPARAM ** pSet_order = NULL;

  HEATERS_OFF;  // security reasons, delay below!
#ifdef USE_WATCHDOG
  watchdog_off();
#endif

  if (ha_state.enabled && ha_state.test_state != TEST_ALL_OK) { // Test fail or in progress, restart it
    ha_state.test_state = TEST_INIT;
  }

  // Check if no device select mode
  if (ha_state.enabled && !si_state.enabled) {
    dev_type = DEV_HA;
    mode = MODE_VAR_SW;
    disp = ha_cfg.disp_n;
    param_max_num = NELEMS(ha_set_order);
    pSet_order = (CPARAM **)&ha_set_order;
  } else if (!ha_state.enabled && si_state.enabled) {
    dev_type = DEV_SI;
    mode = MODE_VAR_SW;
    disp = si_cfg.disp_n;
    param_max_num = NELEMS(si_set_order);
    pSet_order = (CPARAM **)&si_set_order;
  }

  while(1)
  {
    // Blinking feature
    if (millis() - blink_time > BLINK_CYCLE) {
      blink_time = millis();
      if (++blink_state > BLINK_STATE_MAX) {
        blink_state = 0;
      }
    }
    // Key scanning
    if (millis() - button_scan_time > BUTTON_SCANN_CYCLE)
    {
      key_scan();
      button_scan_time = millis();
    }

    // Check SW state
    if (!get_sw_state(HA_SW) && !get_sw_state(SI_SW)) {
      //Nothing to do, exit
      break;
    } else if ((ha_state.enabled == 1) != (get_sw_state(HA_SW) == HA_SW)) {
      // HA state change, exit config mode
      break;
    } else if ((si_state.enabled == 1) != (get_sw_state(SI_SW) == SI_SW)) {
      // SI state change, exit config mode
      break;
    }

    //Configure device
    if (mode == MODE_DEV_SEL) {
      // Device select mode
      if (get_key_event_short(KEY_UP | KEY_DOWN)) { // Exit
        break;
      } else if (get_key_event(KEY_ENTER)) {
        mode = MODE_VAR_SW;
        if (dev_type == DEV_HA) {
          disp = ha_cfg.disp_n;
          param_max_num = NELEMS(ha_set_order);
          pSet_order = ha_set_order;
        } else {
          disp = si_cfg.disp_n;
          param_max_num = NELEMS(si_set_order);
          pSet_order = si_set_order;
        }
      } else if (get_key_event(KEY_UP)) {
        dev_type++;
        if (dev_type > DEV_SI) {
          dev_type = DEV_HA;
        }
      } else if (get_key_event(KEY_DOWN)) {
        dev_type--;
        if (dev_type < DEV_HA) {
          dev_type = DEV_SI;
        }
      }
      // Display
      if (dev_type == DEV_HA) {
        tm1628.showStr(si_cfg.disp_n,"Sol");  // show device name
        if (blink_state > 7) {
          tm1628.clear(ha_cfg.disp_n);
        } else {
          tm1628.showStr(ha_cfg.disp_n,"Hot");  // show device name
        }
      } else {
        tm1628.showStr(ha_cfg.disp_n,"Hot");  // show parameter name
        if (blink_state > 7) {
          tm1628.clear(si_cfg.disp_n);
        } else {
          tm1628.showStr(si_cfg.disp_n,"Sol");  // show parameter name
        }
      }
    } else if (mode == MODE_VAR_SW) {
      // Variable switching mode
      if (get_key_event_short(KEY_UP | KEY_DOWN)) { // To device select mode or exit
        param_num = 0;
        if (ha_state.enabled ^ si_state.enabled) {
          // Only one device active, exit
          break;
        } else {
          mode = MODE_DEV_SEL;
        }
      } else if (get_key_event(KEY_ENTER)) {
        mode = MODE_VAL_SET;
      } else if (get_key_event(KEY_DOWN)) {
        if (param_num+1 < param_max_num) {
          param_num++;
          tm1628.showStr(disp,pSet_order[param_num]->szName);
        }
      } else if (get_key_event(KEY_UP)) {
        if (param_num) {
          param_num--;
          tm1628.showStr(disp,pSet_order[param_num]->szName);
        }
      }
      // Display
      if (blink_state > 7) {
        tm1628.clear(disp);
      } else {
        tm1628.showStr(disp,pSet_order[param_num]->szName);  // show parameter name
      }
    } else {
      // Edit value mode
      if (get_key_event(KEY_UP | KEY_DOWN)) { //Exit without saving
        eep_load(pSet_order[param_num]);
        mode = MODE_VAR_SW;
      } else if (get_key_event(KEY_ENTER)) {
        eep_save(pSet_order[param_num]);
        mode = MODE_VAR_SW;
        tm1628.showNum(disp,pSet_order[param_num]->value);
        delay(1000);
        key_event_clear();
      } else if (get_key_event_long(KEY_UP)) {
        if (pSet_order[param_num]->value < pSet_order[param_num]->value_max - 10) {
          pSet_order[param_num]->value += 10;
        }
      } else if (get_key_event_long(KEY_DOWN)) {
        if (pSet_order[param_num]->value > pSet_order[param_num]->value_min + 10) {
          pSet_order[param_num]->value -= 10;
        }
      } else if (get_key_event_short(KEY_UP)) {
        if (pSet_order[param_num]->value < pSet_order[param_num]->value_max) {
          pSet_order[param_num]->value++;
        }
      } else if (get_key_event_short(KEY_DOWN)) {
        if (pSet_order[param_num]->value > pSet_order[param_num]->value_min) {
          pSet_order[param_num]->value--;
        }
      }
      // Display
      if (blink_state > 7) {
        tm1628.clear(disp);
      } else {
        tm1628.showNum(disp,pSet_order[param_num]->value);  // show parameter value
      }
    }
  }
  key_event_clear();

#ifdef USE_WATCHDOG
  watchdog_on();
#endif
}

void setup_HW(void)
{
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

  analogReference(EXTERNAL);  // use external 3.3V (i.e. from Arduino pin) as ADC reference voltage
}

void init_state(CNTRL_STATE *pDev_state) {
  if (pDev_state == NULL) {
    return;
  }
  memset(pDev_state, 0, sizeof(CNTRL_STATE));
}

void load_cfg(void)
{
  eep_load(&ha_cfg.p_gain);
  eep_load(&ha_cfg.p_scal);
  eep_load(&ha_cfg.i_gain);
  eep_load(&ha_cfg.i_scal);
  eep_load(&ha_cfg.d_gain);
  eep_load(&ha_cfg.d_scal);
  eep_load(&ha_cfg.i_thresh);
  eep_load(&ha_cfg.temp_gain_int_corr);
  eep_load(&ha_cfg.temp_gain_dec_corr);
  eep_load(&ha_cfg.temp_offset_corr);
  eep_load(&ha_cfg.temp_setpoint);
  eep_load(&ha_cfg.temp_averages);
  eep_load(&ha_cfg.slp_timeout);
  eep_load(&ha_cfg.fan_only);
  eep_load(&ha_cfg.display_adc_raw);
#ifdef CURRENT_SENSE_MOD
  eep_load(&ha_cfg.fan_current_min);
  eep_load(&ha_cfg.fan_current_max);
#elif defined(SPEED_SENSE_MOD)
  eep_load(&ha_cfg.fan_speed_min);
  eep_load(&ha_cfg.fan_speed_max);
#endif

  eep_load(&si_cfg.p_gain);
  eep_load(&si_cfg.p_scal);
  eep_load(&si_cfg.i_gain);
  eep_load(&si_cfg.i_scal);
  eep_load(&si_cfg.d_gain);
  eep_load(&si_cfg.d_scal);
  eep_load(&si_cfg.i_thresh);
  eep_load(&si_cfg.temp_gain_int_corr);
  eep_load(&si_cfg.temp_gain_dec_corr);
  eep_load(&si_cfg.temp_offset_corr);
  eep_load(&si_cfg.temp_setpoint);
  eep_load(&si_cfg.temp_averages);
  eep_load(&si_cfg.slp_timeout);
  eep_load(&si_cfg.display_adc_raw);
}

void eep_save(CPARAM * param)
{
  // make sure NOT to save invalid parameter values
  if ((param->value >= param->value_min) && (param->value <= param->value_max)) {
    // nothing to do
  } else {
    // reset to sensible minimum
    param->value = param->value_default;
  }
  EEPROM.update(param->eep_addr_high, highByte(param->value));
  EEPROM.update(param->eep_addr_low, lowByte(param->value));
}

void eep_load(CPARAM * param)
{
  int16_t tmp = (EEPROM.read(param->eep_addr_high) << 8) | EEPROM.read(param->eep_addr_low);

  // make sure NOT to restore invalid parameter values
  if ((tmp >= param->value_min) && (tmp <= param->value_max)) {
    // the value was good, so we use it
    param->value = tmp;
  } else {
    // reset to sensible value
    param->value = param->value_default;
  }
}

void restore_default_conf(void)
{
  tm1628.showStr(DISP_2,"dEF");
  delay(1000);
  tm1628.showStr(DISP_1,"CFg");
  delay(1000);
  tm1628.clear(DISP_ALL);
  ha_cfg.p_gain.value = ha_cfg.p_gain.value_default;
  ha_cfg.p_scal.value = ha_cfg.p_scal.value_default;
  ha_cfg.i_gain.value = ha_cfg.i_gain.value_default;
  ha_cfg.i_scal.value = ha_cfg.i_scal.value_default;
  ha_cfg.d_gain.value = ha_cfg.d_gain.value_default;
  ha_cfg.d_scal.value = ha_cfg.d_scal.value_default;
  ha_cfg.i_thresh.value = ha_cfg.i_thresh.value_default;
  ha_cfg.temp_gain_int_corr.value = ha_cfg.temp_gain_int_corr.value_default;
  ha_cfg.temp_gain_dec_corr.value = ha_cfg.temp_gain_dec_corr.value_default;
  ha_cfg.temp_offset_corr.value = ha_cfg.temp_offset_corr.value_default;
  ha_cfg.temp_setpoint.value = ha_cfg.temp_setpoint.value_default;
  ha_cfg.temp_averages.value = ha_cfg.temp_averages.value_default;
  ha_cfg.slp_timeout.value = ha_cfg.slp_timeout.value_default;
  ha_cfg.fan_only.value = ha_cfg.fan_only.value_default;
  ha_cfg.display_adc_raw.value = ha_cfg.display_adc_raw.value_default;
#ifdef CURRENT_SENSE_MOD
  ha_cfg.fan_current_min.value = ha_cfg.fan_current_min.value_default;
  ha_cfg.fan_current_max.value = ha_cfg.fan_current_max.value_default;
#elif defined(SPEED_SENSE_MOD)
  ha_cfg.fan_speed_min.value = ha_cfg.fan_speed_min.value_default;
  ha_cfg.fan_speed_max.value = ha_cfg.fan_speed_max.value_default;
#endif

  eep_save(&ha_cfg.p_gain);
  eep_save(&ha_cfg.p_scal);
  eep_save(&ha_cfg.i_gain);
  eep_save(&ha_cfg.i_scal);
  eep_save(&ha_cfg.d_gain);
  eep_save(&ha_cfg.d_scal);
  eep_save(&ha_cfg.i_thresh);
  eep_save(&ha_cfg.temp_gain_int_corr);
  eep_save(&ha_cfg.temp_gain_dec_corr);
  eep_save(&ha_cfg.temp_offset_corr);
  eep_save(&ha_cfg.temp_setpoint);
  eep_save(&ha_cfg.temp_averages);
  eep_save(&ha_cfg.slp_timeout);
  eep_save(&ha_cfg.fan_only);
  eep_save(&ha_cfg.display_adc_raw);
#ifdef CURRENT_SENSE_MOD
  eep_save(&ha_cfg.fan_current_min);
  eep_save(&ha_cfg.fan_current_max);
#elif defined(SPEED_SENSE_MOD)
  eep_save(&ha_cfg.fan_speed_min);
  eep_save(&ha_cfg.fan_speed_max);
#endif


  si_cfg.p_gain.value = si_cfg.p_gain.value_default;
  si_cfg.p_scal.value = si_cfg.p_scal.value_default;
  si_cfg.i_gain.value = si_cfg.i_gain.value_default;
  si_cfg.i_scal.value = si_cfg.i_scal.value_default;
  si_cfg.d_gain.value = si_cfg.d_gain.value_default;
  si_cfg.d_scal.value = si_cfg.d_scal.value_default;
  si_cfg.i_thresh.value = si_cfg.i_thresh.value_default;
  si_cfg.temp_gain_int_corr.value = si_cfg.temp_gain_int_corr.value_default;
  si_cfg.temp_gain_dec_corr.value = si_cfg.temp_gain_dec_corr.value_default;
  si_cfg.temp_offset_corr.value = si_cfg.temp_offset_corr.value_default;
  si_cfg.temp_setpoint.value = si_cfg.temp_setpoint.value_default;
  si_cfg.temp_averages.value = si_cfg.temp_averages.value_default;
  si_cfg.display_adc_raw.value = si_cfg.display_adc_raw.value_default;

  eep_save(&si_cfg.p_gain);
  eep_save(&si_cfg.p_scal);
  eep_save(&si_cfg.i_gain);
  eep_save(&si_cfg.i_scal);
  eep_save(&si_cfg.d_gain);
  eep_save(&si_cfg.d_scal);
  eep_save(&si_cfg.i_thresh);
  eep_save(&si_cfg.temp_gain_int_corr);
  eep_save(&si_cfg.temp_gain_dec_corr);
  eep_save(&si_cfg.temp_offset_corr);
  eep_save(&si_cfg.temp_setpoint);
  eep_save(&si_cfg.temp_averages);
  eep_save(&si_cfg.display_adc_raw);
}

// Returns zero if ok
uint8_t HA_test(uint8_t state)
{
  uint8_t result = TEST_INIT;
  static uint32_t test_start = 0;

  if (millis() - test_start > HA_TEST_CYCLE) {
    test_start = millis();

    HA_HEATER_OFF;
    if (state == TEST_INIT) {
      tm1628.clear(ha_cfg.disp_n);
    }
#if defined(CURRENT_SENSE_MOD) || defined(SPEED_SENSE_MOD)
    if ((state & FAN_TEST_MASK) != 0x10) {// else skip to fan test
#endif
      result = cradle_fail_check(state);
      if (result != CRADLE_OK) {
#ifdef DEBUG
        Serial.print("HA test state: ");
        Serial.println(result,HEX);
#endif
        return result;
      }
#if defined(CURRENT_SENSE_MOD) || defined(SPEED_SENSE_MOD)
    }
#endif

#if defined(CURRENT_SENSE_MOD) || defined(SPEED_SENSE_MOD)
    if (result == CRADLE_OK) {
      // Init fan test
      tm1628.clear(ha_cfg.disp_n);
      result = fan_fail_check(TEST_INIT);
    } else {
      result = fan_fail_check(state);
    }
    if (result != FAN_OK) {
#ifdef DEBUG
      Serial.print("HA test state: ");
      Serial.println(result,HEX);
#endif
      return result;
    }
#endif

#ifdef DEBUG
    Serial.print("HA test state: ");
    Serial.println(TEST_ALL_OK,HEX);
#endif
    key_event_clear();
    return TEST_ALL_OK;
  } else {
    return state;
  }
}

uint8_t cradle_fail_check(uint8_t state)
{
  if (state == CRADLE_OK) {
    return state; // Test already ended
  }
  // If the wand is not in the cradle when powered up, go into a safe mode
  // and display an error
  if (REEDSW_CLOSED) {
    state = CRADLE_OK;
    return state;
  } else if (state == TEST_INIT) {
    //Cradle error
#ifdef DEBUG
    Serial.println("Cradle error!");
#endif
    state = CRADLE_FAIL1;
  }

  switch(state)
  {
    case CRADLE_FAIL1:
      tm1628.showStr(ha_cfg.disp_n,"crA");
      state = CRADLE_FAIL2;
      break;
    case CRADLE_FAIL2:
      tm1628.showStr(ha_cfg.disp_n,"dLE");
      state = CRADLE_FAIL3;
      break;
    case CRADLE_FAIL3:
      tm1628.clear(ha_cfg.disp_n);
#ifdef DEBUG
      // Skip safe mode in debug!
      state = CRADLE_OK;
      return state;
#endif
      state = CRADLE_FAIL1;
      break;
  }

  return state;
}

#if defined(CURRENT_SENSE_MOD) || defined(SPEED_SENSE_MOD)
uint8_t fan_fail_check(uint8_t state)
{
  uint8_t fan_current;
  if (state == FAN_OK) {
    return state; // Test already ended
  }

  switch(state)
  {
    case TEST_INIT:
      FAN_ON;
      state = FAN_TEST1;
      tm1628.setDot(ha_cfg.disp_n,2);
      break;
    case FAN_TEST1:
      state = FAN_TEST2;
      tm1628.setDot(ha_cfg.disp_n,1);
      break;
    case FAN_TEST2:
      state = FAN_TEST3;
      tm1628.setDot(ha_cfg.disp_n,0);
      break;
    case FAN_TEST3:
      tm1628.clear(ha_cfg.disp_n);
#ifdef CURRENT_SENSE_MOD
      fan_current = analogRead(FAN_CURRENT_PIN);
      FAN_OFF;
      if ((fan_current < (uint16_t) (ha_cfg.fan_current_min.value))
          || (fan_current > (uint16_t) (ha_cfg.fan_current_max.value))) {
        // FAN fail !
        state = FAN_FAIL1;
#ifdef DEBUG
        Serial.println("Fan current meas. error!");
#endif
#elif defined(SPEED_SENSE_MOD)
      fan_current = analogRead(FAN_SPEED_PIN);
      FAN_OFF;
      if ((fan_current < (uint16_t) (ha_cfg.fan_speed_min.value))
          || (fan_current > (uint16_t) (ha_cfg.fan_speed_max.value))) {
        // FAN fail !
        state = FAN_FAIL1;
#ifdef DEBUG
        Serial.println("Fan speed meas. error!");
#endif
#endif
      } else {
        state = FAN_OK;
      }
      break;
    case FAN_FAIL1:
      tm1628.showStr(ha_cfg.disp_n,"FAn");
      state = FAN_FAIL2;
      break;
    case FAN_FAIL2:
#ifdef CURRENT_SENSE_MOD
      tm1628.showStr(ha_cfg.disp_n,"cur");
#elif defined(SPEED_SENSE_MOD)
      tm1628.showStr(ha_cfg.disp_n,"SPd");
#endif
      state = FAN_FAIL3;
      break;
    case FAN_FAIL3:
      tm1628.clear(ha_cfg.disp_n);
#ifdef DEBUG
      // Skip safe mode in debug!
      state = FAN_OK;
      return state;
#endif
      state = FAN_FAIL1;
      break;
  }

  return state;
}
#endif

void show_firmware_version(void)
{
  tm1628.setDig(DISP_1,0,FW_MINOR_V_B); // dig0
  tm1628.setDig(DISP_1,1,FW_MINOR_V_A); // dig1
  tm1628.setDig(DISP_1,2,FW_MAJOR_V); // dig2
  tm1628.setDot(DISP_1,1);  // dig1.dot
  tm1628.setDot(DISP_1,2);  // dig2.dot
  tm1628.update();
#ifdef DEBUG
  tm1628.showStr(DISP_2,"dbg");
  Serial.print("FW v");
  Serial.print(FW_MAJOR_V);
  Serial.print(".");
  Serial.print(FW_MINOR_V_A);
  Serial.print(".");
  Serial.print(FW_MINOR_V_B);
  Serial.println();
#endif
  delay(2000);
  tm1628.clear(DISP_ALL);
}

void key_scan(void)
{
  static uint32_t long_press_scan_time = millis();
  static uint8_t old_key_state_lscan = 0;
  static uint8_t key_state_ldetected = 0;
  uint8_t tmp_state = 0;
  // For debouncing on/off SW
  static uint8_t sw_state_bounce = 0;

  //SW handling
  if (HA_SW_ON) {
    if (sw_state_bounce & HA_SW) { // State on
      tmp_state |= HA_SW;
    }
    sw_state_bounce |= HA_SW;
  } else {
    if (!(sw_state_bounce & HA_SW)) { // State off
      tmp_state &= ~HA_SW;
    }
    sw_state_bounce &= ~HA_SW;
  }
  if (SI_SW_ON) {
    if (sw_state_bounce & SI_SW) { // State on
      tmp_state |= SI_SW;
    }
    sw_state_bounce |= SI_SW;
  } else {
    if (!(sw_state_bounce & SI_SW)) { // State off
      tmp_state &= ~SI_SW;
    }
    sw_state_bounce &= ~SI_SW;
  }
#ifdef DEBUG
  if (sw_state ^ tmp_state) {
    Serial.print("SW = 0x");
    Serial.println(tmp_state,HEX);
  }
#endif
  sw_state = tmp_state;

  tmp_state = tm1628.getButtons();
#ifdef DEBUG
  if (key_state ^ tmp_state) {
    Serial.print("BT = 0x");
    Serial.println(tmp_state,HEX);
  }
#endif

  // Key handling
  key_state_s |= (key_state & ~tmp_state) & ~old_key_state_lscan;
  key_state = tmp_state;

  if (millis() - long_press_scan_time > LONG_PRESS_SCANN_CYCLE)
  {
    long_press_scan_time = millis();
    key_state_s |= (old_key_state_lscan & ~key_state) & ~key_state_ldetected;
    key_state_ldetected &= ~(old_key_state_lscan & ~key_state); // clear this long press if ended
    key_state_l |= (old_key_state_lscan & key_state); // new key state for long press
    key_state_ldetected |= (old_key_state_lscan & key_state); // save this long press
    old_key_state_lscan = key_state;
  }
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_sw_state(uint8_t sw_mask)
{
  sw_mask &= sw_state;
  return sw_mask;
}

///////////////////////////////////////////////////////////////////
//
// check if a key is pressed right now
//
uint8_t get_key_state(uint8_t key_mask)
{
  key_mask &= key_state;
  return key_mask;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_event_short(uint8_t key_mask)
{
  if ((key_state_s & key_mask) == key_mask) {
    key_state_s &= ~key_mask;
    return key_mask;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_event_long(uint8_t key_mask)
{
  if ((key_state_l & key_mask) == key_mask) {
    key_state_l &= ~key_mask;
    return key_mask;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_event(uint8_t key_mask)
{
  if ((key_state_s & key_mask) == key_mask
      || (key_state_l & key_mask) == key_mask) {
    key_state_s &= ~key_mask;
    key_state_l &= ~key_mask;
    return key_mask;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////
//
void key_event_clear(void)
{
  key_state_l = 0;
  key_state_s = 0;
}

#ifdef USE_WATCHDOG
void watchdog_off(void)
{
  wdt_reset();
  wdt_disable();
}

void watchdog_on(void)
{
  wdt_reset();
  wdt_enable(WDTO_120MS);
}

uint8_t watchdog_check(void)
{
  if (strcmp (p, wdt_signature) == 0)
  { // signature is in RAM this was reset
    return 1;
  }
  else
  {  // signature not in RAM this was a power on
    // add the signature to be retained in memory during reset
    memcpy(p, wdt_signature, sizeof(wdt_signature));  // copy signature into RAM
    return 0;
  }
}

void test_F_CPU_with_watchdog(void)
{
/*
 * Hopefully cause a watchdog reset if the CKDIV8 FUSE is set (F_CPU 1MHz instead of 8MHz)
 *
 */
  wdt_reset();
  wdt_enable(WDTO_120MS);
  delay(40);    // IF "CKDIV8" fuse is erroneously set, this should delay by 8x40 = 320ms & cause the dog to bite!

  watchdog_off();   // IF we got to here, F_CPU is OK.
}
#endif
