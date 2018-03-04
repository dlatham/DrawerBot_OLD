DrawerBot
=========

CNC source files and Arduino code to create a drawer that opens and lowers a scissor lift on request. Perfect for out of reach places where you would prefer to store things but don't want to pull out a ladder to do so.

The code has fault control built in which uses IR distance sensors to determine the state of both the drawer and lift. There is also timeout timers to ensure that the DC motors don't run extensively and cause damage.

Changelog
--------
### Version 0.1 (2 March 2017)
* Initial commits
* Keyboard control via serial port
* Limit switch checking
* Safety timeouts
* All pin and variable definitions

### Version 0.2 (June 2017)
* Switched from mechanical limit switches to IR distance sensors
* Added IR sensor calibration routines
* Updated cancel motion and error routines
* Timeout functions now have adjustable time variables
