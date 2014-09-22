tBeat
=====

The tBeat library manages recurring function calls based on a period. The library uses the Timer1 or the Timer3 to generate an interruption every 1ms. What I call hook is a little structure holding a period, a countdown, and a function pointer. When the count is zero, the function pointer is called. By default you can have a maximum of 8 hooks. Every hooks are modifiable, deletable and so on...
