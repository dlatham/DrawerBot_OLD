DrawerBot
=========

CNC source files and Arduino code to create a drawer that opens and lowers a scissor lift on request. Perfect for out of reach places where you would prefer to store things but don't want to pull out a ladder to do so.

The code has fault control built in which uses IR distance sensors (https://www.amazon.com/Sharp-GP2Y0A21YK0F-Distance-Sensor-1-75/dp/B003EA5WYM/) to determine the state of both the drawer and lift. There is also timeout timers to ensure that the DC motors don't run extensively and cause damage.

Versions
--------
### Version 0.1 (2 March 2017)
* Initial commits
* Keyboard control via serial port
* Limit switch checking
* Safety timeouts
* All pin and variable definitions

### Version 0.2 (14 June 2017)
* Updated limits to distance sensors over mechanical switches
* Added calibration routines to adjust limit distances
* Better routine management
* Initial hardware tests
