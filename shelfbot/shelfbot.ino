#include <SimpleTimer.h>
/*
 * Automated Shelf by Dave Latham
 * Version 0.1
 * 2 March 2017
 * --------------------------------------------
 */


int request = 2; //Interupt pin for raise / lower request signals
//RELAY PINS
int motor_direction_A = 4;
int motor_direction_B = 5;
int drawer_relay = 6;
int lift_relay = 7;
//INPUT PINS
int drawer_in_limit = 9;
int drawer_out_limit = 10;
int lift_up_limit = 11;
int lift_down_limit = 12;
int left_weight = 13;
int right_weight = 14;
//VARIABLES
volatile int state = LOW;
bool error = false;
int error_code = 0;
int incomingByte = 0; //Incoming serial data
int drawerTimer;
int liftTimer;
unsigned long startTime;
unsigned long stopTime;
SimpleTimer timer;
/*
 * ERROR CODES:
 * #1 - Drawer movement timeout
 * #2 - Lift movement timeout.
 * #3 - Motor relay error
 * 
 */


void setup() {
  // Setup the pins
  attachInterrupt(request, requestChange, CHANGE);
  pinMode(drawer_relay, OUTPUT);
  digitalWrite(drawer_relay, HIGH);
  pinMode(lift_relay, OUTPUT);
  digitalWrite(lift_relay, HIGH);
  pinMode(drawer_in_limit, INPUT_PULLUP);
  pinMode(drawer_out_limit, INPUT_PULLUP);
  pinMode(lift_up_limit, INPUT_PULLUP);
  pinMode(lift_down_limit, INPUT_PULLUP);
  pinMode(left_weight, INPUT);
  pinMode(right_weight, INPUT);
  pinMode(motor_direction_A, OUTPUT);
  digitalWrite(motor_direction_A, HIGH);
  pinMode(motor_direction_B, OUTPUT);
  digitalWrite(motor_direction_B, HIGH);
  // Setup the motion timers
  
  timer.disable(drawerTimer);
  liftTimer = timer.setTimeout(10000, liftTimeout);
  timer.disable(liftTimer);
  // Setup the serial monitor and print welcome
  Serial.begin(9600);
  Serial.println("ShelfBot version 0.1");
  Serial.println("--------------------");
  Serial.print("Listening for state change on interrupt PIN ");
  Serial.println(request);
  Serial.println("[o]pen drawer, [c]lose drawer, [l]ower lift, [r]aise lift, [d]ismiss Error, [h]elp");
  Serial.println("READY");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) { //Serial input from keyboard received
    incomingByte = Serial.read();
    switch (incomingByte){
      case 111: {
        cancelMotion();
        Serial.println("Open drawer requested.");
        openDrawer();
      }
      break;
      case 99: {
        cancelMotion();
        Serial.println("Close drawer requested.");
        closeDrawer();
      }
      break;
      case 108: {
        cancelMotion();
        Serial.println("Lower lift requested.");
        lowerLift();
      }
      break;
      case 114: {
        cancelMotion();
        Serial.println("Raise lift requested.");
        raiseLift();
      }
      break;
      case 100: {
        if (error = true) {
          cancelMotion();
          Serial.println("Error state reset.");
          error = false;
        }
        break;
      }
      case 104: {
        //ADD HELP CODE HERE
      }
      break;
    }
  }
  delay(500);

}


// Main interrupt request controller starts here---------------------------------->
void requestChange(){
  state = digitalRead(request);
  //First, stop all motion
  cancelMotion();
  //Start the raise or lower programs depending on request pin state
  if (state == HIGH){
    //Lower sequence
    Serial.println("Lower request received.");
    if (openDrawer()){
      if(lowerLift()){
        Serial.println("Lower request complete.");
        //ADD CODE FOR MEASURING WEIGHT HERE
      }
    } 
  } else if (state = LOW){
    //Raise sequence
  }
}


// Drawer motion controllers start here--------------------------------------------->
bool openDrawer() {
  // Confirm the drawer isn't out first
  if (digitalRead(drawer_out_limit) == HIGH){
    Serial.println("Starting drawer open.");
    Serial.print("Confirming motor relay forward... ");
    if (motorForward()){
      Serial.println("OK"); //Motors are in forward relay switching - start open
      Serial.print("Opening the drawer... ");
      startTime = millis();
      while (digitalRead(drawer_out_limit) == HIGH && error==false){
        digitalWrite(drawer_relay, LOW);
        if ((millis()-startTime) > 10000){
          drawerTimeout();
        }
      }
      digitalWrite(drawer_relay, HIGH);
      if (error == true){
        Serial.println ("ERROR 1");
        return false;
      } else {
        Serial.print("OK - Completed in ");
        stopTime = millis() - startTime;
        Serial.print(stopTime/1000);
        Serial.println("sec");
        return true;
      }
      
    } else {
      error = true;
      error_code = 3;
      Serial.println("ERROR 3");
      return false;
    }
    
  } else {
    Serial.println("Drawer already extended.");
    return true;
  }
}


bool closeDrawer() {
}

bool lowerLift() {
  //Confirm the lift isn't lowered already
  if (digitalRead(lift_down_limit) == HIGH){
    Serial.println("Starting lift lower.");
    Serial.print("Confirming motors in forward...");
    if (motorForward()){
      Serial.println ("OK"); //Motors are forward start lowering the lift
      Serial.print("Lowering the lift... ");
      startTime = millis();
      while (digitalRead(lift_down_limit) == HIGH && error==false){
        digitalWrite(lift_relay, LOW);
                if ((millis()-startTime) > 10000){
          liftTimeout();
        }
      }
      digitalWrite(lift_relay, HIGH);
      if (error == true){
        Serial.println ("ERROR 2");
        return false;
      } else {
        Serial.print("OK - Completed in ");
        stopTime = millis() - startTime;
        Serial.print(stopTime/1000);
        Serial.println("sec");
        return true;
      }
      
    } else {
      error = true;
      error_code = 3;
      Serial.println("ERROR 3");
      return false;
    }
    
  } else {
    Serial.println("Lift already lowered.");
    return true;
  }
}

bool raiseLift() {
}

bool motorForward() {
  digitalWrite(motor_direction_A, LOW);
  digitalWrite(motor_direction_B, LOW);
  return true;
}

bool motorReverse() {
}

void drawerTimeout() {
  error = true;
  error_code = 1;
  //MAY NEED TO ADD TIMER REENABLE HERE
}
void liftTimeout() {
  error = true;
  error_code = 2;
  //MAY NEED TO ADD TIMER REENABLE HERE
}

void cancelMotion(){
  digitalWrite(drawer_relay, HIGH);
  digitalWrite(lift_relay, HIGH);
  //Determine if motion was indeed canceled and pause to prevent mechanical damage
  if (timer.isEnabled(drawerTimer) || timer.isEnabled(liftTimer)){
    Serial.println("CANCELED");
    timer.deleteTimer(drawerTimer);
    timer.deleteTimer(liftTimer);
    delay(3000); 
  }
}

