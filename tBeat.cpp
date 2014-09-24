// Copyright (c) 2014 Benoît Roehr <benoit.roehr@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TBEAT_cpp
#define TBEAT_cpp

#include "tBeat.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "Arduino.h"

tBeatCore tBeat;

//==============================================================================
// Interrupt Service Routine
//==============================================================================
ISR(TBEAT_TIMERn_CAPT_vect) {
  int16_t tmpTime = micros();
  tBeat.globalCount++;//for debug and time tracking
  for (uint8_t idx = 0; idx < tBeat._hookSize; idx++) {
    if (tBeat._hookList[idx]->isEnabled()) {
      tBeat._hookList[idx]->count -= 1;
    }
    if (tBeat._hookList[idx]->isInterrupt()) {
      tBeat._executeHook(idx);
    }
  }
  tBeat.interruptTime = micros() - tmpTime;
}

//==============================================================================
// Count and Execution routine
//==============================================================================
void tBeatCore::exec() {
  for (uint8_t idx = 0; idx < tBeat._hookSize; idx++) {
    if (_hookList[idx]->isEnabled()) {
      tBeat._executeHook(idx);
    }
  }
}

void tBeatCore::_executeHook(uint8_t idx) {
  if (_hookList[idx]->count <= 0) {
    if (!_hookList[idx]->isLooped()) {
      _hookList[idx]->disable();
    }
    if (_hookList[idx]->count < 0) {
      _hookList[idx]->setWarning();
    }
    _hookList[idx]->count += _hookList[idx]->period;
    if (_hookList[idx]->count < 0) {
      _hookList[idx]->setError();
    }
    _hookList[idx]->callback();
  }
}

//==============================================================================
// tBeatCore timer control
//==============================================================================
void tBeatCore::init() {
  cli();//security first
  TBEAT_TCCRnA  = 0b00000000;
  TBEAT_TCCRnB  = 0b00011000;
  TBEAT_ICRn    = (uint16_t)TBEAT_CYCLES;
  TBEAT_TIMSKn  = 0b00100000;
  TBEAT_TCNTn   = 0;
  sei();
}

void tBeatCore::start() {
  TBEAT_TCCRnB |= TBEAT_PRESCALER;
}

void tBeatCore::pause() {
  TBEAT_TCCRnB &= ~TBEAT_PRESCALER;
}

//==============================================================================
// tBeatHook Management
//==============================================================================
void tBeatCore::registerHook(tBeatHook& hook) {
  uint8_t idx;
  for (idx = _hookSize;
       (idx > 0) && (_hookList[idx - 1]->period > hook.period);
       idx--) {
    _hookList[idx] = _hookList[idx - 1];
  }
  _hookSize++;
  _hookList[idx] = &hook;
}

#endif//whole file