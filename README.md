tBeat
=====

The tBeat library manages periodic or timed function calls. The library uses the Timer1 or the Timer3 to generate an interruption every 1ms. What I call hook is a little structure holding a period, a countdown, and a function pointer. When the count is zero, tBeat callbacks the function pointer of the hook. By default you can have a maximum of 8 hooks. Every hooks are modifiable, deletable and so on... Priority is given to the hooks which have the smallest period.

## The tBeatCore Class

This class is the only class in this library. An instance of it named *tBeat* is declared in *tBeat.h* and *tBeat.cpp*. You shall not instanciate another tBeatCore instance. 

## The tBeatHook Struct

tBeatCore contains a struct named tBeatHook used internally. For simplification purpose, creating a new hook doesn't require the instanciation of a tBeatHook struct, making the library more usable.

## More details

Please have a sight on the tBeatExample.ino file, you will find details about how to use the library and what it can do.

## TODO:

 - Hooks with a 1ms period could interrupt the longer executing hook, and/or a priority based system could take the decision to force an execution from within the ISR.
 - Some statistics about the execution time of the hooks could be saved in the hook itself and made public for debugging purpose.
 - Another type of hook could be added: One shot hooks. The one shot hook should be automatically deleted once executed. Ideal for delays.
