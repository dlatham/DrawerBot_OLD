#include <EEPROM.h>
/*
 * Automated Shelf by Dave Latham
 * Version 0.2 - in-progress
 * This version will substitute the mechanical limit switches for an IR distance sensor and add a calibration routine
 * 22 May 2017
 * --------------------------------------------
 */

float vers = 0.2;
int request = 2; //Interupt pin for raise / lower request signals
//RELAY PINS
int motor_direction_A = 4;
int motor_direction_B = 5;
int drawer_relay = 6;
int lift_relay = 7;
//INPUT PINS
int drawer_sense = A0;
int lift_sense = A1;
int left_weight = 13;
int right_weight = 14;
//VARIABLES
volatile int state = LOW;
bool error = false;
int error_code = 0;
int incomingByte = 0; //Incoming serial data
int drawerTimer;
int liftTimer;
bool motion = false;
int startTime;
int stopTime;
/*
 * ERROR CODES:
 * #1 - Drawer movement timeout
 * #2 - Lift movement timeout.
 * #3 - Motor relay error
 * 
 */
 //STORED EEPROM LIMIT VALUES
 int lift_limit_out = EEPROM.read(0);
 int lift_limit_in = EEPROM.read(1);
 int drawer_limit_out = EEPROM.read(2);
 int drawer_limit_in = EEPROM.read(3);


void setup() {
  // Setup the pins
  attachInterrupt(request, requestChange, CHANGE);
  pinMode(drawer_relay, OUTPUT);
  digitalWrite(drawer_relay, HIGH);
  pinMode(lift_relay, OUTPUT);
  digitalWrite(lift_relay, HIGH);
  pinMode(drawer_sense, INPUT);
  pinMode(lift_sense, INPUT);
  pinMode(left_weight, INPUT);
  pinMode(right_weight, INPUT);
  pinMode(motor_direction_A, OUTPUT);
  digitalWrite(motor_direction_A, HIGH);
  pinMode(motor_direction_B, OUTPUT);
  digitalWrite(motor_direction_B, HIGH);

  // Setup the serial monitor and print welcome
  Serial.begin(9600);
  Serial.print("ShelfBot version ");
  Serial.println(vers);
  Serial.println("--------------------");
  Serial.print("Listening for state change on interrupt PIN ");
  Serial.println(request);
  Serial.println("[o]pen drawer, [c]lose drawer, [l]ower lift, [r]aise lift, [d]ismiss Error, [s]ensor calibration, [h]elp");
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
      case 115: {
        cancelMotion();
        Serial.println("Sensor calibration requested.");
        calibrate();
      }
      break;
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
  if (digitalRead(analogRead(drawer_sense)<drawer_limit_in)){
    Serial.println("Starting drawer open.");
    Serial.print("Confirming motor relay forward... ");
    if (motorForward()){
      Serial.println("OK"); //Motors are in forward relay switching - start open
      Serial.print("Opening the drawer... ");
      startTime = millis();
      while (analogRead(drawer_sense) > drawer_limit_out && error==false){
        motion = true;
        digitalWrite(drawer_relay, LOW);
        if ((millis()-startTime) > 10000){
          drawerTimeout();
        }
      }
      digitalWrite(drawer_relay, HIGH);
      if (error == true){
        Serial.println ("ERROR 1");
        //Pause for 2 seconds before telling the interrupt that motion stopped (just in case)
        delay(2000);
        motion = false;
        return false;
      } else {
        Serial.print("OK - Completed in ");
        stopTime = millis() - startTime;
        Serial.print(stopTime/1000);
        Serial.println("sec");
        //Pause for 2 seconds before telling the interrupt that motion stopped (just in case)
        delay(2000);
        motion = false;
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
  if (analogRead(lift_sense)<lift_limit_in){
    Serial.println("Starting lift lower.");
    Serial.print("Confirming motors in forward...");
    if (motorForward()){
      Serial.println ("OK"); //Motors are forward start lowering the lift
      Serial.print("Lowering the lift... ");
      startTime = millis();
      while (analogRead(lift_sense) > lift_limit_out && error==false){
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
  digitalWrite(motor_direction_A, HIGH);
  digitalWrite(motor_direction_B, HIGH);
  return true;
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

void calibrate() {
  Serial.println("CURRENT SENSOR LIMITS:");
  Serial.println("------------------------------");
  Serial.print("\t");
  Serial.print("OPEN");
  Serial.print("\t");
  Serial.print("CLOSE");
  Serial.print("\t");
  Serial.println("CURRENT");
 
  Serial.print("Drawer:");
  Serial.print("\t");
  Serial.print(drawer_limit_out);
  Serial.print("\t");
  Serial.print(drawer_limit_in);
  Serial.print("\t");
  Serial.println(analogRead(drawer_sense));
  
  Serial.print("Lift:");
  Serial.print("\t");
  Serial.print(lift_limit_out);
  Serial.print("\t");
  Serial.print(lift_limit_in);
  Serial.print("\t");
  Serial.println(analogRead(lift_sense));
  Serial.println("------------------------------");
  Serial.println("Ready to calibrate the sensor limits.");
  Serial.println("[d]rawer sensor, [l]ift sensor, [c]ancel or any other key to repeat sensor read.");

  //Wait here for input
  while(Serial.available() == 0) { }
  incomingByte = Serial.read();

  //Parse the menu selection
  switch (incomingByte){
      case 100: { // d
        cancelMotion();
        Serial.println("Drawer sensor calibration requested.");
        calibrateDrawer();
      }
      break;
      case 108: { // l
        cancelMotion();
        Serial.println("Lift sensor calibration requested.");
        calibrateLift();
      }
      break;
      case 99: { // c
        Serial.println("[o]pen drawer, [c]lose drawer, [l]ower lift, [r]aise lift, [d]ismiss Error, [s]ensor calibration, [h]elp");
        Serial.println("READY");
        return;
      }
      break;
  }
  calibrate(); //Return to the top of the function if any key other than d,l, or c were presseed 
}

void calibrateDrawer(){
  //First check to see if the lift is down and see if we want to raise it or calibrate that first
  if(analogRead(lift_sense) < 500) { //less than somewhere around 2.5v on the sense pin
    Serial.println("WARNING: It appears as though the lift is down so moving the drawer could cause damage.");
    Serial.println("[r]aise the lift, [c]ontinue with drawer calibration, any other key to cancel.");
    
    //Wait for input
    while(Serial.available() == 0) { }
    incomingByte = Serial.read();
    if(incomingByte==114 && lift_limit_in != 0) {
      if(!raiseLift()) {
        return;
      } 
      
    } else if (incomingByte==114 && lift_limit_in==0){
      Serial.println("Unable to raise the lift because it hasn't been calibrated yet. You can calibrate the lift first, raise it manually, or re-execute drawer calibration and ignore the warning.");
      delay(300);
      Serial.println("");
      return;
    } else if (incomingByte==99){
      Serial.println("WARNING, WARNING: You are choosing to move the drawer despite the sensors measuring the lift as being down. This could cause damage.");
    } else {
      return;
    }
  }

  //Select order of calibration
  Serial.println("NOTICE: Please confirm the lift is up and it is safe to move the drawer.");
  Serial.println("Set the drawer [o]pen limit, [c]losed limit or any other key to cancel.");

  //Wait for input
  while(Serial.available() == 0) { }
  incomingByte = Serial.read();

  //Calibration selection start
  if(incomingByte==111){ // o for the open limit
    Serial.println("Ready to open the drawer. The monitor will stream the drawer sensor reading. PRESS ANY KEY TO STOP THE DRAWER.");
    do {
      Serial.print("Current sensor reading: ");
      Serial.println(analogRead(drawer_sense));
      Serial.println("[o]pen drawer, [s]ave current reading, [c]ancel");
      while(Serial.available() == 0) { }
      incomingByte = Serial.read();
      if(incomingByte==111){ // o for open the drawer
        // Open the drawer
        Serial.println("Starting drawer open.");
        Serial.print("Confirming motor relay forward... ");
        if (motorForward()){
          Serial.println("OK"); //Motors are in forward relay switching - start open
          Serial.print("Opening the drawer... ");
          startTime = millis();
          while (Serial.available()==0){
            motion = true;
            digitalWrite(drawer_relay, LOW);
            if ((millis()-startTime) > 10000){
              drawerTimeout();
              return;
            }
            Serial.print(analogRead(drawer_sense));
            Serial.println(" PRESS ANY KEY TO STOP");
          }
          digitalWrite(drawer_relay, HIGH);
        
        } else {
          Serial.println("ERROR: Motor direction control failure.");
          return;
        }
        delay(300);
        // Go back to the top of the do
        
    } else if(incomingByte==115){ //s for save the current reading
      drawer_limit_out == analogRead(drawer_sense);
      EEPROM.write(drawer_limit_out, 1);
      Serial.println("New drawer out limit SAVED!");
      delay(300);
      return;
    } else {
      return;
    }
    
  } while(2>1);
  }
  
  
}

void calibrateLift(){
  
}

void cancelMotion(){
  digitalWrite(drawer_relay, HIGH);
  digitalWrite(lift_relay, HIGH);
  //Determine if motion was indeed canceled and pause to prevent mechanical damage
  if (motion == true){
    Serial.println("CANCELED");
    delay(3000); 
  }
}

