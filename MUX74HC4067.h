/**
 * Name: MUX74HC4067
 * Author: Nick Lamprianidis { nlamprian@gmail.com }
 * Version: 1.0
 * Description: A library for interfacing the 74HC4067
 *              multiplexers/demultiplexers
 * Source: https://github.com/nlamprian/MUX74HC4067
 * License: Copyright (c) 2014 Nick Lamprianidis
 *          This library is licensed under the MIT license
 *          http://www.opensource.org/licenses/mit-license.php
 *
 * Filename: MUX74HC4067.h
 * File description: Definitions and methods for the MUX74HC4067 library
 */

#ifndef MUX74HC4067_h
#define MUX74HC4067_h

#include <Arduino.h>

#define ANALOG 0
#define DIGITAL 1
#define DIGITAL_PULSE 2

#define DISABLED 0
#define ENABLED 1

// Class for interfacing the 74HC4067 multiplexers/demultiplexers
class MUX74HC4067 {
public:
  MUX74HC4067(uint8_t en, int8_t s0, int8_t s1 = -1, int8_t s2 = -1, int8_t s3 = -1);
  void setChannel(int8_t pin, uint8_t set = ENABLED);
  void enable();
  void disable();
  void checkTiming();
  bool isReleased(int8_t chan_pin = -1);
  bool isPressed(int8_t chan_pin = -1);
  int16_t read(int8_t chan_pin = -1);
  uint8_t write(int8_t chan_pin, uint8_t data, int8_t type = -1);
  void signalPin(uint8_t sig, uint8_t mode, uint8_t type = DIGITAL, unsigned long time = 50);

private:
  // -1 - Not Applicable, 0 - Analog Mode, 1 - Digital Mode, 2 Digital Mode(pulseIn())
  int8_t signal_pin_status;

  uint8_t signal_mode;  // INPUT, OUTPUT, INPUT_PULLUP
  uint8_t num_of_control_pins;
  uint8_t enable_status;  // DISABLED, ENABLED
  uint8_t enable_pin;
  int8_t signal_pin;
  int8_t control_pin[4];
  uint8_t current_channel;

  int btnPin;
  unsigned long debounceTime;

  int previousSteadyState;   // the previous steady state from the input pin, used to detect pressed and released event
  int lastSteadyState;       // the last steady state from the input pin
  int lastFlickerableState;  // the last flickerable state from the input pin

  unsigned long lastDebounceTime;  // the last time the output pin was toggled
};

#endif  // MUX74HC4067
