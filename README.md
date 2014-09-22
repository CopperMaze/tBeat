tBeat
=====

The tBeat library manages periodic or timed function calls. The library uses the Timer1 or the Timer3 to generate an interruption every 1ms. What I call hook is a little structure holding a period, a countdown, and a function pointer. When the count is zero, tBeat callbacks the function pointer of the hook. By default you can have a maximum of 8 hooks. Every hooks are modifiable, deletable and so on... Priority is given to the hooks which have the smallest period.

## The tBeatCore Class

This class is the only class in this library. An instance of it named *tBeat* is declared in *tBeat.h* and *tBeat.cpp*. You shall not instanciate another tBeatCore instance. tBeatCore contains a struct named tBeatHook used internally. For simplification purpose, creating a new hook doesn't require the instanciation of a tBeatHook struct, making the library more usable.
