/**
 * Name: MUX74HC4067
 * Author: Nick Lamprianidis { nlamprian@gmail.com }
 * Version: 1.0
 * Description: A library for interfacing the 74HC4067
 *				multiplexers/demultiplexers
 * Source: https://github.com/nlamprian/MUX74HC4067
 * License: Copyright (c) 2014 Nick Lamprianidis
 *          This library is licensed under the MIT license
 *          http://www.opensource.org/licenses/mit-license.php
 *
 * Filename: MUX74HC4067.cpp
 * File description: Implementations of methods for the MUX74HC4067 library
 */

#include "MUX74HC4067.h"

// Creates a MUX74HC4067 instance and initializes the instance variables
// Arguments:
// en - Arduino pin to which the EN pin connects
// s0 - Arduino pin to which the S0 pin connects
// s1-s3 - (Optional) Arduino pins to which the S1-S3 pin connect

MUX74HC4067::MUX74HC4067(uint8_t en, int8_t s0, int8_t s1, int8_t s2, int8_t s3) {
  enable_pin = en;
  pinMode(en, OUTPUT);
  digitalWrite(en, HIGH);  // Initially disables the connection of the SIG pin to the channels
  enable_status = DISABLED;

  signal_pin_status = -1;
  signal_pin = -1;
  signal_mode = 0;
  current_channel = 0;
  num_of_control_pins = 1;
  control_pin[0] = s0;

  num_of_control_pins += ((control_pin[1] = s1) != -1) ? 1 : 0;
  num_of_control_pins += ((control_pin[2] = s2) != -1) ? 1 : 0;
  num_of_control_pins += ((control_pin[3] = s3) != -1) ? 1 : 0;

  for (uint8_t i = 0; i < num_of_control_pins; ++i) {
    pinMode(control_pin[i], OUTPUT);
  }
}

// Selects the given channel, and enables its connection with the SIG pin by default
// Arguments:
// pin - The channel to select
// set - A flag (DISABLED, ENABLED) to indicate whether to leave the
//       connection of the channel to the SIG pin disabled or enable it
void MUX74HC4067::setChannel(int8_t pin, uint8_t set) {
  digitalWrite(enable_pin, HIGH);
  current_channel = pin;

  for (uint8_t i = 0; i < num_of_control_pins; ++i) {
    digitalWrite(control_pin[i], pin & 0x01);
    pin >>= 1;
  }

  enable_status = ENABLED;

  if (set == ENABLED) {
    digitalWrite(enable_pin, LOW);
  }
}

// Enables the connection of the SIG pin with a channel that was set earlier
void MUX74HC4067::enable() {
  enable_status = ENABLED;
  digitalWrite(enable_pin, LOW);
}

// Disables the connection of the SIG pin with a channel that was set earlier
void MUX74HC4067::disable() {
  enable_status = DISABLED;
  digitalWrite(enable_pin, HIGH);
}

// Configures the signal pin. If set to input, it reads from the signal pin.
// If set to output, it writes to the signal pin. If set to digital, it reads or writes
// a digital value to the signal pin. If set to analog, either it reads an analog value
// from the signal pin, or it writes a PWM output to the signal pin.
// Arguments:
// sig - Arduino pin to which the SIG pin connects
// mode - either INPUT or OUTPUT or INPUT_PULLUP
// type - either DIGITAL, ANALOG or DIGITAL(pulseIn())
void MUX74HC4067::signalPin(uint8_t sig, uint8_t mode, uint8_t type, unsigned long time) {
  signal_pin = sig;

  if (mode == INPUT || mode == INPUT_PULLUP) {
    signal_mode = INPUT;

    if (mode == INPUT_PULLUP) {
      signal_mode = INPUT_PULLUP;
      pinMode(sig, INPUT_PULLUP);
    } else {
      digitalWrite(sig, LOW);  // Disables pullup
      pinMode(sig, INPUT);
    }

    previousSteadyState = digitalRead(sig);
    lastSteadyState = previousSteadyState;
    lastFlickerableState = previousSteadyState;

    debounceTime = time;
    lastDebounceTime = 0;
  } else if (mode == OUTPUT) {
    signal_mode = OUTPUT;
    pinMode(sig, OUTPUT);
  }

  if (type == ANALOG) {
    signal_pin_status = 0;
  } else if (type == DIGITAL) {
    signal_pin_status = 1;
  } else if (type == DIGITAL_PULSE) {
    signal_pin_status = 2;
  }
}

int16_t MUX74HC4067::read(int8_t chan_pin) {
  int16_t data;
  uint8_t last_channel;
  uint8_t last_en_status = enable_status;

  if (chan_pin != -1) {
    last_channel = current_channel;
    setChannel(chan_pin);
  }

  if (signal_pin_status == 0) {
    data = analogRead(signal_pin);
  } else if (signal_pin_status == 1) {
    data = digitalRead(signal_pin);
  } else if (signal_pin_status == 2) {
    data = pulseIn(signal_pin, LOW);
  } else {
    data = -1;
  }

  if (chan_pin != -1) {
    setChannel(last_channel, last_en_status);
  }

  return data;
}

// It writes to the requested channel. If the signal pin was set to DIGITAL, it writes HIGH or LOW.
// If the signal pin was set to ANALOG, it writes a PWM output. There is an option to override the
// ANALOG/DIGITAL configuration of the signal pin.
// chan_pin - Channel to which to write
// data - Data to write to channel chan_pin. If the signal pin was set to digital or DIGITAL was
//        specified in the argument list, data should be HIGH or LOW. If the signal pin was set
//        to analog or ANALOG was specified in the argument list, data should the duty cycle of
//        the PWM output.
// type - An optional argument that is either ANALOG or DIGITAL, and overrides the ANALOG/DIGITAL
//        configuration of the signal pin

uint8_t MUX74HC4067::write(int8_t chan_pin, uint8_t data, int8_t type) {
  if (signal_mode == INPUT || signal_mode == INPUT_PULLUP) {
    return -1;
  }

  disable();

  if (type == ANALOG) {
    analogWrite(signal_pin, data);
  } else if (type == DIGITAL) {
    digitalWrite(signal_pin, data);
  }

  setChannel(chan_pin);

  return 0;
}

// ezButtons loop implementation
void MUX74HC4067::checkTiming() {

  if (signal_pin_status == 1) {
    // read the state of the switch/button:
    int currentState = digitalRead(signal_pin);

    unsigned long currentTime = millis();

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch/button changed, due to noise or pressing:
    if (currentState != lastFlickerableState) {
      // reset the debouncing timer
      lastDebounceTime = currentTime;
      // save the the last flickerable state
      lastFlickerableState = currentState;
    }

    if ((currentTime - lastDebounceTime) >= debounceTime) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      // save the the steady state
      previousSteadyState = lastSteadyState;
      lastSteadyState = currentState;
    }
  }
}

// ezButtons isReleased implementation
bool MUX74HC4067::isReleased(int8_t chan_pin) {
  bool data = false;
  uint8_t last_channel;
  uint8_t last_en_status = enable_status;

  if (chan_pin != -1) {
    last_channel = current_channel;
    setChannel(chan_pin);
  }

  if (signal_pin_status == 1) {
    data = (previousSteadyState == LOW && lastSteadyState == HIGH);
  }

  return data;
}

// ezButtons isPressed implementation
bool MUX74HC4067::isPressed(int8_t chan_pin) {
  bool data = false;
  uint8_t last_channel;
  uint8_t last_en_status = enable_status;

  if (chan_pin != -1) {
    last_channel = current_channel;
    setChannel(chan_pin);
  }

  if (signal_pin_status == 1) {
    data = (previousSteadyState == HIGH && lastSteadyState == LOW);
  }

  return data;
}
