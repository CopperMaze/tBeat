#ifndef TBEAT_cpp
#define TBEAT_cpp

#include "tBeat.h"
#include "avr/io.h"
#include "avr/interrupt.h"

tBeatCore tBeat;

//==============================================================================
// Interrupt Service Routine
//==============================================================================
ISR(TBEAT_TIMERn_CAPT_vect)
{
  tBeat._elapsed++;
  tBeat.globalCount++;
}

//==============================================================================
// tBeatCore timer control
//==============================================================================

void tBeatCore::init()
{
  cli();//security first
  TBEAT_TCCRnA  = 0b00000000;
  TBEAT_TCCRnB  = 0b00011000;
  TBEAT_ICRn    = (uint16_t)TBEAT_CYCLES;
  TBEAT_TIMSKn  = 0b00100000;
  TBEAT_TCNTn   = 0;
  sei();
}

void tBeatCore::start()
{
  TBEAT_TCCRnB |= TBEAT_PRESCALER;
}

void tBeatCore::pause()
{
  TBEAT_TCCRnB &= ~TBEAT_PRESCALER;
}

//==============================================================================
// Count and Execution routine
//==============================================================================

void tBeatCore::exec()
{
  if (_elapsed)
  {
    uint16_t tmp_elapsed = _elapsed;
    _elapsed = 0;//safely reset _elapsed in case interrupts occur during exec

    for (uint8_t idx = 0; idx < _hookSize; idx++)
    {
      _hookData[idx].count -= tmp_elapsed;
      if (_hookData[idx].count <= 0)
      {
        _hookData[idx].count += _hookData[idx].period;
        _hookData[idx].callback();
      }
    }
  }
}

//==============================================================================
// tBeat Hook list control
//==============================================================================

void tBeatCore::newHook(int16_t period, fptr callback)
{
  Hook hook(period, period, callback);
  _registerHook(hook);
}

void tBeatCore::newHook(int16_t period, int16_t initialCount,
                        fptr callback)
{
  Hook hook(period, initialCount, callback);
  _registerHook(hook);
}

void tBeatCore::killHook(int16_t period, fptr callback)
{
  Hook hook(period, period, callback);
  _deleteHook(hook);
}

void tBeatCore::modifyHook(int16_t period,
                           fptr callback,
                           int16_t newPeriod)
{
  Hook hook(period, period, callback);
  uint8_t idx = _seekHook(hook);
  if (idx < _hookSize)
  {
    _hookData[idx].period = newPeriod;
  }
}

//==============================================================================
// Private members
//==============================================================================

uint8_t tBeatCore::_seekHook(Hook const &hook)
{
  uint8_t idx;
  for (idx = 0; (idx <= _hookSize) && (hook != _hookData[idx]); idx++);
  return idx;
}

void tBeatCore::_registerHook(Hook const &hook)
{
  uint8_t idx;
  for (idx = _hookSize;
       (idx > 0) && (_hookData[idx - 1].period > hook.period);
       idx--)
  {
    _hookData[idx] = _hookData[idx - 1];
  }
  _hookSize++;
  _hookData[idx] = hook;
}

void tBeatCore::_deleteHook(Hook const &hook)
{
  uint8_t idx = _seekHook(hook);
  if (idx < _hookSize)
  {
    _hookSize--;
    for (idx; idx < _hookSize; idx++)
    {
      _hookData[idx] = _hookData[idx + 1];
    }
  }
}

#endif//whole file