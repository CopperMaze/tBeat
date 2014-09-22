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

#include "Arduino.h"
#include "tBeat.h"

#define LED 13

int period = 200;

void ledOn();
void ledOff();
void modifyPeriod();
void pause2Sec();

void setup() {
  pinMode(LED, OUTPUT);

  /*============================================================================
  The init function will set the base period. An interrupt event will occure
  from the selected timer (in tbeat.h) every x µs. The base period is defined in
  tBeat.h. By default, the base period is 1000µs. This period can be modified in
  the defined parameters of tBeat.h .
  ============================================================================*/
  tBeat.init();

  /*============================================================================
  The newHook period parameter act like a divider of the base period. So if you
  set a hook with a period of 1000 and the base period is 1ms, the function will
  be called every 1 seconds. One of the limit of tBeat is that the callback
  function MUST be in the global scope.
  ============================================================================*/
  tBeat.newHook(period, ledOn);
  tBeat.newHook(2000, modifyPeriod);

  /*============================================================================
  By default, the initial count of a newHook is set equal to the period. If you
  want, you can set the inital count yourself. Here, the inital count is 989ms.
  ============================================================================*/
  tBeat.newHook(10000, 989, pause2Sec);

  /*============================================================================
  When initializing tBeat, the hardware timer is never started automaticaly, you
  have to start it manually preferably after you registered your hooks
  ============================================================================*/
  tBeat.start();
}

void loop() {
  /*============================================================================
  The tBeat.exec must be called as fast as possible. Ideally, when using tBeat,
  your sketch should be organized so every functions are called periodically by
  tBeat. If a function have to wait, do not use delay(). tBeat will handle
  function delay like so for example:

  char step = 1;
  void printTime()
  {
    if (step == 1)
    {
    Serial.println("Hey, it's 1am...");
    tBeat.newHook(10000, printTime);
    step = 2;
    }

    if (step == 2)
    {
      Serial.println("past 10 seconds.");
      tBeat.killHook(10000, printTime);
      step = 1;
    }
  }

  The advantage of this method is that the function is non blocking. If other
  operations should be executed during the 10 seconds delay, they will.

  exec will check if a base period have elapsed (a timer interrupt) and will
  decrease the count of every hook registered. If a count is 0, the associated
  callback is called and the count is reloaded with the period parameter of the
  hook. If the count is negative (meaning some base beats have been missed), the
  counter is reloaded with period minus the missed beats.
  ============================================================================*/
  tBeat.exec();
}

void ledOn() {
  digitalWrite(LED, 1);//lighting up Led.

  /*============================================================================
  Here we create a hook with much smaller period length than the others. tBeat
  automatically gives higher priority to hooks with smaller period value. As
  they are called more often, it's expected they take less time to execute. The
  "longest period" hook will be executed last. If some hooks have the same
  period, the "first in" will be executed first.

  In case of suspicious behavior, the tBeat.globalCount can be used
  to check elapsed time vs executed code, and/or to measure the execution time
  of functions. globalCount is not used in the engine so you can write and read
  on it at will.
  ============================================================================*/
  tBeat.newHook(10, ledOff);
}

void ledOff() {
  digitalWrite(LED, 0);// setting led off

  /*============================================================================
  killHook removes a hook from the list based on its period and callback
  pointer.
  ============================================================================*/
  tBeat.killHook(10, ledOff);
}

void modifyPeriod() {
  int last_period = period;
  if (period == 200) {
    period = 100;
  }
  else {
    period = 200;
  }
  /*============================================================================
    modifyHook modifies the period of a hook based on its initial period and
    callback pointer.
  ============================================================================*/
  tBeat.modifyHook(last_period, ledOn, period);
}

void pause2Sec() {
  /*============================================================================
    in case you want a very long operation to be made, in order to avoid missing
    beats and keep counts straight, you may want to pause and start the timer.
  ============================================================================*/
  tBeat.pause();
  delay(2000);
  tBeat.start();
}