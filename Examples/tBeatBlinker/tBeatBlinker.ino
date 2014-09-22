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

  tBeat.init();/*
The init function will set the base period. An interrupt event will occure from
the selected timer (in tbeat.h) every x µs. The base period is defined in
tBeat.h. By default, the base period is 1000µs. This period can be modified in
the defined parameters of tBeat.h .
*/

  tBeat.newHook(period, ledOn);
  tBeat.newHook(2000, modifyPeriod);/*
The newHook period parameter act like a divider of the base period. So if you
set a hook with a period of 1000 and the base period is 1ms, the function will
be called every 1 seconds. One of the limit of tBeat is that the callback
function MUST be in the global scope.
*/

  tBeat.newHook(10000, 989, pause2Sec);/*
By default, the initial count of a newHook is set equal to the period. If you
want, you can set the inital count yourself. Here, the inital count is 989ms.
*/

  //starting the timer
  tBeat.start();
}

void loop() {
  tBeat.exec();/*
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
*/
}

void ledOn() {
  digitalWrite(LED, 1);

  tBeat.newHook(10, ledOff);/*
Here we create a hook with much smaller period length than the others. tBeat
automatically gives higher priority to hooks with smaller period value. As
they are called more often, it's expected they take less time to execute. The
"longest period" hook will be executed last. If some hooks have the same
period, the "first in" will be executed first.

In case of suspicious behavior, the tBeat.globalCount can be used
to check elapsed time vs executed code, and/or to measure the execution time
of functions. globalCount is not used in the engine so you can write and read
on it at will.
*/
}

void ledOff() {
  digitalWrite(LED, 0);

  tBeat.killHook(10, ledOff);/*
  killHook removes a hook from the list based on its period and callback
  pointer.
  */
}

void modifyPeriod() {
  int last_period = period;
  if (period == 200) {
    period = 100;
  }
  else {
    period = 200;
  }

  tBeat.modifyHook(last_period, ledOn, period);/*
  modifyHook modifies the period of a hook based on its initial period and
  callback pointer.
  */
}

void pause2Sec() {
  tBeat.pause();
  delay(2000);
  tBeat.start();  /*
  in case you want a very long operation to be made, in order to avoid missing
  beats and keep counts straight, you may want to pause and start the timer.
  */
}