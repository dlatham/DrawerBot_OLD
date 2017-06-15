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
 int lift_down = 0;
 int lift_up = 1;
 int drawer_out = 2;
 int drawer_in = 3;
 int lift_down_limit = EEPROM.read(0);
 int lift_up_limit = EEPROM.read(1);
 int drawer_out_limit = EEPROM.read(2);
 int drawer_in_limit = EEPROM.read(3);


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
  if (digitalRead(analogRead(drawer_sense)<drawer_in_limit)){
    Serial.println("Starting drawer open.");
    if (motorForward()){
      Serial.print("Opening the drawer... ");
      startTime = millis();
      while (analogRead(drawer_sense) > drawer_out_limit){
        motion = true;
        digitalWrite(drawer_relay, LOW);
        if ((millis()-startTime) > 10000){
          drawerTimeout();
        }
      }
      digitalWrite(drawer_relay, HIGH);
      Serial.print("OK - Completed in ");
      stopTime = millis() - startTime;
      Serial.print(stopTime/1000);
      Serial.println("sec");
      return true;
      
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
  if (analogRead(lift_sense)<lift_up_limit){
    Serial.println("Starting lift lower.");
    if (motorForward()){
      Serial.print("Lowering the lift... ");
      startTime = millis();
      while (analogRead(lift_sense) > lift_down_limit){
        digitalWrite(lift_relay, LOW);
          if ((millis()-startTime) > 10000){
            liftTimeout();
          }
        }
      digitalWrite(lift_relay, HIGH);
      Serial.print("OK - Completed in ");
      stopTime = millis() - startTime;
      Serial.print(stopTime/1000);
      Serial.println("sec");
      return true;
      
    }
    
  } else {
    Serial.println("Lift already lowered.");
    return true;
  }
}

bool raiseLift() {
}




//-------------------------------------CALIBRATION CODE STARTS HERE--------------------->
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
  Serial.print(drawer_out_limit);
  Serial.print("\t");
  Serial.print(drawer_in_limit);
  Serial.print("\t");
  Serial.println(analogRead(drawer_sense));
  
  Serial.print("Lift:");
  Serial.print("\t");
  Serial.print(lift_down_limit);
  Serial.print("\t");
  Serial.print(lift_up_limit);
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
        returnToMain();
      }
      break;
  }
  calibrate(); //Return to the top of the function if any key other than d,l, or c were presseed 
}

//-------------------DRAWER CALIBRATION---------------------->
void calibrateDrawer(){
  
  //Select order of calibration
  Serial.println("\nNOTICE: Please confirm the lift is up and it is safe to move the drawer.");
  Serial.println("Set the drawer [o]pen limit, [c]losed limit or any other key to cancel.");

  //Wait for input
  while(Serial.available() == 0) { }
  incomingByte = Serial.read();

  //Calibration selection start - start with parsing the open or close selections and then loop the open/close and save code until they exit
  if(incomingByte==111){ // o for the open limit
    Serial.println("\nReady to open the drawer. The monitor will stream the drawer sensor reading. PRESS ANY KEY TO STOP THE DRAWER.\n");
    do {
      Serial.print("Current sensor reading: ");
      Serial.println(analogRead(drawer_sense));
      Serial.println("[o]pen drawer, [s]ave current reading, any other key to cancel.");
      while(!Serial.available()) { }
      incomingByte = Serial.read();
      if(incomingByte==111){ // o for open the drawer
        // Open the drawer
        Serial.println("\nStarting drawer open...");
        delay(3000);
        if (motorForward()){
          while(Serial.available()){Serial.read();} //CLEAR THE BUFFER
          startTime = millis();
          while (!Serial.available()){ //DRAWER CALIBRATION OPEN MOVEMENT
            if((millis()-startTime)>10000){
              drawerTimeout();
            }
            digitalWrite(drawer_relay, LOW);
            Serial.print(analogRead(drawer_sense));
            Serial.println(" PRESS ANY KEY TO STOP");
          }
          //DRAWER STOP HERE
          digitalWrite(drawer_relay, HIGH);
          Serial.println("\nDrawer Stopped - Pausing 2 seconds...\n");
          while(Serial.available()){Serial.read();} //CLEAR THE BUFFER
        
        } else {
          Serial.println("\n The motor direction forward function failed (this shouldn't happen).\n\n");
          return;
        }
        delay(3000);
        // Go back to the top of the do
        
      } else if(incomingByte==115){ //s for save the current reading
        drawer_out_limit = analogRead(drawer_sense);
        saveLimit(drawer_out, drawer_out_limit);
        return;
      } else {
        return;
      }
    
    } while(2>1);

  
  } else if(incomingByte==99){ // c for setting the closed drawer limit
    Serial.println("\nReady to close the drawer. The monitor will stream the drawer sensor reading. PRESS ANY KEY TO STOP THE DRAWER.\n");
    do {
      Serial.print("Current sensor reading: ");
      Serial.println(analogRead(drawer_sense));
      Serial.println("[c]lose drawer, [s]ave current reading, any other key to cancel.");
      while(!Serial.available()) { }
      incomingByte = Serial.read();
      if(incomingByte==99){ // c for close the drawer
        // Clsoe the drawer
        Serial.println("\nStarting drawer close...");
        delay(3000);
        if (motorReverse()){
          while(Serial.available()){Serial.read();} //CLEAR THE BUFFER
          startTime = millis();
          while (!Serial.available()){ //DRAWER CALIBRATION CLOSE MOVEMENT
            if((millis()-startTime)>10000){
              drawerTimeout();
            }
            digitalWrite(drawer_relay, LOW);
            Serial.print(analogRead(drawer_sense));
            Serial.println(" PRESS ANY KEY TO STOP");
          }
          //DRAWER STOP HERE
          digitalWrite(drawer_relay, HIGH);
          Serial.println("\nDrawer Stopped - Pausing 2 seconds...\n");
          while(Serial.available()){Serial.read();} //CLEAR THE BUFFER
        
        } else {
          Serial.println("\n The motor direction reverse function failed (this shouldn't happen).\n\n");
          return;
        }
        delay(3000);
        // Go back to the top of the do
        
      } else if(incomingByte==115){ //s for save the current reading
        drawer_in_limit = analogRead(drawer_sense);
        saveLimit(drawer_in, drawer_in_limit);
        return;
      } else {
        return;
      }
    
    } while(2>1);

    
  }
  
  
}

//-----------------LIFT CALIBRATION------------------>
void calibrateLift(){
//LETS GET THIS RIGHT FOR DRAWER AND THEN COPY THE CODE HERE
  
}


//-----------------------------SAVING AND RETURNING LIMIT VALUES FROM EEPROM-------------------->

bool saveLimit(int limit, int value){ //CODE TO SAVE THE APPROPRIATE EEPROM VALUES
  value = value/3;
  switch (limit) {
    case 0: {
      EEPROM.write(0, value);
      Serial.print("\nNew lift down limit saved\n"); delay(3000); return true;
    }
    break;
    case 1: {
      EEPROM.write(1, value);
      Serial.print("\nNew lift up limit saved\n"); delay(3000); return true;
    }
    break;
    case 2: {
      EEPROM.write(2, value);
      Serial.print("\nNew drawer open limit saved\n"); delay(3000); return true;
    }
    break;
    case 3: {
      EEPROM.write(3, value);
      Serial.print("\nNew drawer close limit saved\n"); delay(3000); return true;
    }
    break;
  }
  Serial.println("\nERROR SAVING LIMIT: This shouldn't happen.\n");
  return false;
}

int getLimit(int limit){ //CODE TO RETURN THE APPROPRIATE LIMIT
  switch (limit){
    case 0: {
      limit = EEPROM.read(0);
    }
    case 1: {
      limit = EEPROM.read(1);
    }
    case 2: {
      limit = EEPROM.read(2);
    }
    case 3: {
      limit = EEPROM.read(3);
    }
  }
  limit = limit * 3;
  return limit;
}

// -------------------------------Error functions and exit to main menu functions------------------------------------------------------->

void returnToMain(){
  cancelMotion();
  Serial.println("\n\n--------------------");
  Serial.print("Listening for state change on interrupt PIN ");
  Serial.println(request);
  Serial.println("[o]pen drawer, [c]lose drawer, [l]ower lift, [r]aise lift, [d]ismiss Error, [s]ensor calibration, [h]elp");
  Serial.println("READY");
  loop();
}

bool motorForward() {
  digitalWrite(motor_direction_A, LOW);
  digitalWrite(motor_direction_B, LOW);
  Serial.println("Motor direction relays set in the forward position.");
  return true;
}

bool motorReverse() {
  digitalWrite(motor_direction_A, HIGH);
  digitalWrite(motor_direction_B, HIGH);
  Serial.println("Motor direction relays set in the reverse position.");
  return true;
}

void drawerTimeout() {
  cancelMotion();
  Serial.println("\nERROR: The drawer timed out. Please check the mechanics and try again.");
  delay(3000);
  returnToMain();
}
void liftTimeout() {
  cancelMotion();
  Serial.println("\nERROR: The lift timed out. Please check the mechanics and try again.");
  delay(3000);
  returnToMain();
}

void cancelMotion(){
  if((digitalRead(drawer_relay)==LOW)||(digitalRead(lift_relay)==LOW)){
    digitalWrite(drawer_relay, HIGH);
    digitalWrite(lift_relay, HIGH);
    Serial.println("\nMOTION CANCELED\n");
    delay(3000); //Prevent damage from moving parts
  }
}


