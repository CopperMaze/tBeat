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

#ifndef TBEAT_h
#define TBEAT_h

#include "Arduino.h"

//==============================================================================
// tBeat parameters
//==============================================================================
#define TBEAT_TIMER_PERIOD 1000 //µs
#define TBEAT_HOOK_STACK_SIZE 8 //maximum hook numbers
#define TBEAT_TIMERn TIMER3 //The lib currently supports TIMER1 and TIMER3
//if other libraries are using timers, check which timer and adjust accordingly

//==============================================================================
// Precompiler calculus (do not modify)
//==============================================================================
#define TBEAT_PRESCALER 0x01
#define TBEAT_CYCLES ((F_CPU/1000000)*TBEAT_TIMER_PERIOD)-1
#if TBEAT_CYCLES > 65535
#define TBEAT_PRESCALER 0x02
#define TBEAT_CYCLES (((F_CPU/1000000)*TBEAT_TIMER_PERIOD)/4)-1
#endif
#if TBEAT_CYCLES > 65535
#define TBEAT_PRESCALER 0x03
#define TBEAT_CYCLES (((F_CPU/1000000)*TBEAT_TIMER_PERIOD)/64)-1
#endif
#if TBEAT_CYCLES > 65535
#define TBEAT_PRESCALER 0x04
#define TBEAT_CYCLES (((F_CPU/1000000)*TBEAT_TIMER_PERIOD)/256)-1
#endif
#if TBEAT_CYCLES > 65535
#define TBEAT_PRESCALER 0x05
#define TBEAT_CYCLES (((F_CPU/1000000)*TBEAT_TIMER_PERIOD)/1024)-1
#endif
#if TBEAT_CYCLES > 65535
#warning "tBeat: TBEAT_CYCLES is more than 65535, timings will be incorrect !"
#define TBEAT_CYCLES 65535
#endif

//==============================================================================
// Timer Selection (do not modify)
//==============================================================================
#if TBEAT_TIMERn == TIMER1
#define TBEAT_TCCRnA  TCCR1A
#define TBEAT_TCCRnB  TCCR1B
#define TBEAT_TCCRnC  TCCR1C
#define TBEAT_TCNTn   TCNT1
#define TBEAT_TIMSKn  TIMSK1
#define TBEAT_ICRn    ICR1
#define TBEAT_TIMERn_CAPT_vect TIMER1_CAPT_vect
#elif TBEAT_TIMERn == TIMER3
#define TBEAT_TCCRnA  TCCR3A
#define TBEAT_TCCRnB  TCCR3B
#define TBEAT_TCCRnC  TCCR3C
#define TBEAT_TCNTn   TCNT3
#define TBEAT_TIMSKn  TIMSK3
#define TBEAT_ICRn    ICR3
#define TBEAT_TIMERn_CAPT_vect TIMER3_CAPT_vect
#endif

struct tBeatHook;

extern "C" void TBEAT_TIMERn_CAPT_vect(void)  __attribute__ ((signal));
//declaring the ISR as an extern function, will be friend with tBeatCore class

//==============================================================================
// tBeatCore class
//==============================================================================
class tBeatCore {
public:
  tBeatCore() {
    for (uint8_t idx = 0; idx < TBEAT_HOOK_STACK_SIZE; idx++) {
      _hookList[idx] = NULL;
    }
  }

  //tBeat hardware timer commands
  void init();
  void start();
  void pause();

  //tBeat execute, must be called in main loop()
  void exec();

  //tBeat internal counter (overflows)
  int16_t globalCount;
  uint8_t interruptTime;
  uint8_t _hookSize;
  tBeatHook* _hookList[TBEAT_HOOK_STACK_SIZE];

  //hookList Management
  void registerHook(tBeatHook& hook);

  void _executeHook(uint8_t idx);
  uint8_t _seekHook(const tBeatHook& hook);
};

extern tBeatCore tBeat;

//==============================================================================
// tBeatHook structure
//==============================================================================
struct tBeatHook {
public:
  friend void TBEAT_TIMERn_CAPT_vect();
  friend class tBeatCore;

  //private:
  typedef void (*fptr)();
  fptr callback;
  /*============================================================================
  status byte description:
  bit:
        0: isEnabled  R/W
        1: isLooped   R/W
        2: interrupt  R/W
        3: warning    R
        4: error      R
        5: custom0    R/W
        6: custom1    R/W
        7: custom2    R/W
  ============================================================================*/
  uint8_t status;

  /*============================================================================
  Constructors
  ============================================================================*/
  tBeatHook(void):
    callback(NULL),
    period(0),
    count(-1),
    status(0) {}
  tBeatHook(fptr callback,
            int16_t period,
            int16_t initialCount = -1,
            bool isEnabled = true,
            bool isLooped = true,
            bool isInterrupt = false):
    callback(callback),
    period(period),
    count((initialCount >= 0) ? initialCount : period),
    status((uint8_t)(isEnabled | (isLooped << 1) | (isInterrupt << 2))) {
    tBeat.registerHook(*this);
  }
  int16_t period;
  int16_t count;

  /*============================================================================
  Warning and errors
  ============================================================================*/
  inline bool getWarning() {
    bool ret = status & 0x01 << 3;
    status &= ~(0x01 << 3);
    return ret;
  }
  inline void setWarning() {
    status |= 0x01 << 3;
  }
  inline bool getError() {
    bool ret = status & 0x01 << 4;
    status &= ~(0x01 << 4);
    return ret;
  }
  inline void setError() {
    status |= 0x01 << 4;
  }

  /*============================================================================
  Enable and disable
  ============================================================================*/
  inline bool isEnabled() {
    return status & 0x01 << 0;
  }
  inline void enable() {
    status |= 0x01 << 0;
  }
  inline void disable() {
    status &= ~(0x01 << 0);
  }

  /*============================================================================
  Looping and one shot
  ============================================================================*/
  inline bool isLooped() {
    return status & 0x01 << 1;
  }
  inline void setLooped() {
    status |= 0x01 << 1;
  }
  inline void clrLooped() {
    status &= ~(0x01 << 1);
  }

  /*============================================================================
  Interrupt
  ============================================================================*/
  inline bool isInterrupt() {
    return status & 0x01 << 2;
  }
  inline void setInterrupt() {
    status |= 0x01 << 2;
  }
  inline void clrInterrupt() {
    status &= ~(0x01 << 2);
  }

  /*============================================================================
  periodChange
  ============================================================================*/
  void setPeriod(int16_t newPeriod, int16_t initialCount = -1) {
    if (initialCount == -1) {
      initialCount = newPeriod;
    }
    period = newPeriod;
    count = initialCount;
  }

  /*============================================================================
  Operators overload
  ============================================================================*/
  bool operator==(const tBeatHook& x) const {
    return ((period == x.period) && (callback == x.callback));
  }

  bool operator!=(const tBeatHook& x) const {
    return !(*this == x);
  }
};

#endif //whole file
