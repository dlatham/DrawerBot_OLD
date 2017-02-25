#include <SimpleTimer.h>
/*
 * Automated Shelf by Dave Latham
 * 23 February 2017
 * --------------------------------------------
 */


int drawer_relay = 7;
int lift_relay = 8;
int drawer_in_limit = 9;
int drawer_out_limit = 10;
int lift_up_limit = 11;
int lift down_limit = 12;
int left_weight = 13;
int right_weight = 14;
bool error = false;
int error_code = 0;
/*
 * ERROR CODES:
 * #1 - Request for drawer open but the open sensor is on.
 * #2 - Drawer close timeout
 * #3 - Request for drawer close but the closed sensor is on.
 * #4 - Drawer open timeout
 * #5 - Request for lift lower but down sensor is on.
 * #6 - Lift down timeout.
 * #7 - Request for lift raise but up sensor is on.
 * #8 - Lift up timeout.
 * 
 */


void setup() {
  // setup the pins
  pinMode(drawer_relay, OUTPUT);
  pinMode(lift_relay, OUTPUT);
  pinMode(drawer_in_limit, INPUT_PULLUP);
  pinMode(drawer_out_limit, INPUT_PULLUP);
  pinMode(lift_up_limit, INPUT_PULLUP);
  pinMode(lift_down_limit, INPUT_PULLUP);
  pinMode(left_weight, INPUT);
  pinMode(right_weight, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void openDrawer() {
  // Confirm the drawer isn't out first
  if(drawer_out_relay == HIGH){
    Serial.println("Starting drawer open.");
    
  } else {
    error = true;
    error_code = 1;
    Serial.println("Error 1: Drawer open requested with drawer out sensor active.");
  }
}

