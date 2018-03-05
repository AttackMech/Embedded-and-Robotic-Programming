/*
 * Alarm Clock
 * Jason Bishop
 * 3042012
 * 
 * v1.0
 * 
 * Explain here
 * 
 */

//includes
// motor ctrl
#include <Time.h>

// constants
const int moveTime = 5000;
const int dangerZone = 10;
const int safeZone = 20;

const int pirPin = 12;    //the digital pin connected to the PIR sensor's output
const int pause = 2000;
const int stopper = 5000;

const int echoPinFront = 3;
const int trigPinFront = 2;
const int echoPinLeft = 6;
const int trigPinLeft = 5;
const int echoPinRight = 9;
const int trigPinRight = 10;

const int maxRange = 200;  // max distance needed
const int minRange = 0;  // min distance needed


// variables
bool alarmOn = false;
bool moveNow = false;

bool lockLow = true;
bool takeLowTime;
unsigned long int beginTime, lowIn, stopTime;

long duration, distance; // used to calculate distance

int sensorReadings[4] = {0, 0, 0, 0};
int previousReadings[3] = {0, 0, 0};


void setup() {
  //pin setup
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);

  pinMode(echoPinFront, INPUT);
  pinMode(trigPinFront, OUTPUT);
  pinMode(echoPinLeft, INPUT);
  pinMode(trigPinLeft, OUTPUT);
  pinMode(echoPinRight, INPUT);
  pinMode(trigPinRight, OUTPUT);
  
  Serial.begin(9600);
  Serial.print("On and waiting...");
  for(int i = 1; i < 9; i++) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nAlarm in one second...");
  delay(1000);
  Serial.println("ALARM!");
  alarmOn = true;
  moveNow = true;
  beginTime = millis();
//  stopTime = 0;
}

void loop() {
  Serial.println(moveNow);
  // check move timing
  if (moveNow && millis() - beginTime >= moveTime) {
    moveNow = false;
    stopTime = millis();
    for (int i = 0; i < 3; i++) {
      previousReadings[i] = sensorReadings[i];
    }
    Serial.print("now = ");
    Serial.print(millis());
    Serial.print("  stopTime = ");
    Serial.print(stopTime);
    Serial.print("  beginTime = ");
    Serial.print(beginTime);
    delay(2000);
  }

  if (alarmOn) {
    
    if (moveNow) {

      readSensors(false);
      
      // if front detect
      if (sensorReadings[0] <= dangerZone) {
        // stop
        moveRobot('s', 50);
        // check both
        if (sensorReadings[1] <= safeZone && sensorReadings[2] <= safeZone) {
          // check which side is closer
          if (sensorReadings[1] < sensorReadings[2]) {
            moveRobot('l', 50);
          }
          else if (sensorReadings[1] > sensorReadings[2]) {
            moveRobot('r', 50);
          }
          else {
            moveRobot('l', 1000);
          }
          
        }
        else if (sensorReadings[1] <= safeZone) {
          // turn right 90
          moveRobot('r', 500);
        }
        else {
          // turn left 90
          moveRobot('l' , 500);
        }
      }
      
      // if left detect
      else if (sensorReadings[1] <= dangerZone) {
        // turn right 5?
        moveRobot('r', 50);
      }
      
      // if right detect
      else if (sensorReadings[2] <= dangerZone) {
        // turn left 5?
        moveRobot('l', 50);
      }
      
      // move forward
      else {
        // move forward
        moveRobot('f', 50);
      }
    }
    else {
      moveRobot('s', 0);
      readSensors(true);
      
      // stop
      if (sensorReadings[3] == 1) {
        moveNow = true;
        beginTime = millis();
        // move forward
        moveRobot('f', 50);
      }
      else if (previousReadings[0] - sensorReadings[0] > 5 ||
        previousReadings[0] - sensorReadings[0] < -5) {
        moveNow = true;
        beginTime = millis();
        // move back, turn around 180, move forward
        moveRobot('b', 2000);
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
      else if (previousReadings[1] - sensorReadings[1] > 5 ||
        previousReadings[1] - sensorReadings[1] < -5) {
        moveNow = true;
        beginTime = millis();
        // turn right 90, move forward
        moveRobot('r', 500);
        moveRobot('f', 50);
      }
      else if (previousReadings[2] - sensorReadings[2] > 5 ||
        previousReadings[2] - sensorReadings[2] < -5) {
        moveNow = true;
        beginTime = millis();
        // turn left 90, move forward
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
      else {
        for (int i = 0; i < 3; i++) {
          previousReadings[i] = sensorReadings[i];
        }
      }
      if (millis() - stopTime >= stopper) {
        moveNow = true;
        beginTime = millis();
        // move forward
        moveRobot('f', 50);
        Serial.print("stopTime = ");
        Serial.print(stopTime);
        Serial.print("  millis = ");
        Serial.print(millis());
        Serial.print("  stopper = ");
        Serial.print(stopper);
        Serial.print("  total = ");
        Serial.println(millis() - stopTime);
        Serial.println(moveNow);
        delay(1000);
      }
    }
    // if time fin
      // stop
      // start stop timer

    // if stop time fin
    

    // if stopped && movement
      // turn away
      // restart timer
      // move forward
  }
  else {
    moveRobot('s', 0);
  }

}

void readSensors(bool all) {
  // read front
  digitalWrite(trigPinFront, LOW); 
  delayMicroseconds(2);

  digitalWrite(trigPinFront, HIGH);
  delayMicroseconds(10); 
   
  digitalWrite(trigPinFront, LOW);
  duration = pulseIn(echoPinFront, HIGH);
  
  // Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
  
  if (distance >= maxRange || distance <= minRange) {
    sensorReadings[0] = -1;
  }
  else {
    sensorReadings[0] = distance;
  }

  delay(25);
 
  // read left
  digitalWrite(trigPinLeft, LOW); 
  delayMicroseconds(2);

  digitalWrite(trigPinLeft, HIGH);
  delayMicroseconds(10); 
   
  digitalWrite(trigPinLeft, LOW);
  duration = pulseIn(echoPinLeft, HIGH);
  
  // Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
  
  if (distance >= maxRange || distance <= minRange) {
    sensorReadings[1] = -1;
  }
  else {
    sensorReadings[1] = distance;
  }

  delay(25);
  
  // read right
  digitalWrite(trigPinRight, LOW); 
  delayMicroseconds(2);

  digitalWrite(trigPinRight, HIGH);
  delayMicroseconds(10); 
   
  digitalWrite(trigPinRight, LOW);
  duration = pulseIn(echoPinRight, HIGH);
  
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;
  
  if (distance >= maxRange || distance <= minRange) {
    sensorReadings[2] = -1;
  }
  else {
    sensorReadings[2] = distance;
  }

  delay(25);
  sensorReadings[3] = -1;
  
  // read motion detector
  if (all) {
    if (digitalRead(pirPin) == HIGH) {
      sensorReadings[3] = 1;
      if (lockLow) {
        lockLow = false;
      }
      takeLowTime = true;
    }
    else {
      sensorReadings[3] = 0;
      
      if (takeLowTime) {
        lowIn = millis();
        takeLowTime = false;
      }
      
      if (!lockLow && millis() - lowIn > pause) {
        lockLow = true;
      }
    }
  }
  Serial.print("Moving = ");
  if (moveNow) {
    Serial.println("TRUE");
  }
  else {
    Serial.println("FALSE");
  }
  Serial.print("Front = ");
  Serial.print(sensorReadings[0]);
  Serial.print("  Left = ");
  Serial.print(sensorReadings[1]);
  Serial.print(" Right = ");
  Serial.print(sensorReadings[2]);
  Serial.print(" Motion = ");
  if (sensorReadings[3] == 1) {
    Serial.println("true");
  }
  else if (sensorReadings[3] == 0) {
    Serial.println("false");
  }
  else {
    Serial.println("n/a");
  }

} // end of function readSensors()

void moveRobot(char dir, int amt) {
  switch (dir) {
    case 's':  // stop
      Serial.println("STOP");
      delay(amt);
      break;
    case 'f':  // forward
      Serial.println("FORWARD");
      delay(amt);
      break;
    case 'l':  // left
      Serial.println("LEFT");
      delay(amt);
      break;
    case 'r':  // right
      Serial.println("RIGHT");
      delay(amt);
      break;
    case 'b':  // backward
      Serial.println("BACK");
      delay(amt);
      break;
  }
}

