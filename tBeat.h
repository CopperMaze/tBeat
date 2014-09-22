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

//==============================================================================
// tBeatCore class
//==============================================================================

extern "C" void TBEAT_TIMERn_CAPT_vect(void)  __attribute__ ((signal));
//declaring the ISR as an extern function, will be friend with tBeatCore class

class tBeatCore {
public:
  typedef void (*fptr)();//function pointer

  //tBeat hardware timer commands
  void init();
  void start();
  void pause();

  //tBeat execute, must be called in main loop()
  void exec();

  //hook attribution
  void newHook(int16_t period, fptr callback);
  void newHook(int16_t period, int16_t initialCount, fptr callback);

  void modifyHook(int16_t period,
                  fptr callback,
                  int16_t newPeriod,
                  int16_t initialCount);

  void modifyHook(int16_t period,
                  fptr callback,
                  int16_t newPeriod);

  void killHook(int16_t period, fptr callback);

  //declaring the ISR vector as a public friend of the class
  friend void TBEAT_TIMERn_CAPT_vect();

  //tBeat internal counter
  int16_t globalCount;

private:
  struct Hook {
    //default constructor
    Hook(void):
      period(0),
      count(0),
      callback(NULL) {}

    //constructor with parameters
    Hook(int16_t period, int16_t count, fptr callback):
      period(period),
      count(count),
      callback(callback) {}

    bool operator!=(const Hook& x) const {
      return ((period != x.period) || (callback != x.callback));
    }

    bool operator==(const Hook& x) const {
      return ((period == x.period) && (callback == x.callback));
    }

    int16_t period;
    int16_t count;
    fptr callback;
  };

  uint8_t _seekHook(const Hook& hook);
  void _registerHook(Hook const& hook);
  void _deleteHook(Hook const& hook);

  Hook _hookData[TBEAT_HOOK_STACK_SIZE];
  uint8_t _hookSize;

  uint8_t _elapsed;
};

extern tBeatCore tBeat;

#endif //whole file