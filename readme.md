DrawerBot
=========

CNC source files and Arduino code to create a drawer that opens and lowers a scissor lift on request. Perfect for out of reach places where you would prefer to store things but don't want to pull out a ladder to do so.

The code has fault control built in which uses limit switches to determine the state of both the drawer and lift. There is also timeout timers to ensure that the DC motors don't run extensively and cause damage.

Versions
--------
###Version 0.1 (2 March 2017)
* Initial commits
* Keyboard control via serial port
* Limit switch checking
* Safety timeouts
* All pin and variable definitions
