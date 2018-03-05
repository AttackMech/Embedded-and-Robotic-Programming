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

const int motor1a = 11;
const int motor1b = 10;
const int motor2a = 8;
const int motor2b = 9;

// constants
const int moveTime = 15000;
const int stopper = 10000;
const int dangerZone = 15;
const int safeZone = 30;

const int pirPin = 13;    //the digital pin connected to the PIR sensor's output
const int pause = 2000;

const int echoPinFront = A5;
const int trigPinFront = A4;
const int echoPinLeft = A3;
const int trigPinLeft = A2;
const int echoPinRight = A1;
const int trigPinRight = A0;

const int maxRange = 300;  // max distance needed
const int minRange = 0;  // min distance needed


// variables
bool alarmOn = false;
bool moveNow = false;

bool lockLow = true;
bool takeLowTime;
unsigned long int beginTime, lowIn, stopTime;

long duration, distance; // used to calculate distance

int sensorReadings[4] = {-1, -1, -1, -1};
int previousReadings[2][4] = {{-1, -1, -1, -1}, {-1, -1, -1, -1}};

bool stuckCheck = false;
int stuckReadings[2][3];
int stuckCount = 0;


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

  pinMode(motor1a, OUTPUT);
  pinMode(motor1b, OUTPUT);
  pinMode(motor2a, OUTPUT);
  pinMode(motor2b, OUTPUT);
  
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
  Serial.println();
  
  // check move timing
  if (moveNow && millis() - beginTime >= moveTime) {
    moveNow = false;
    moveRobot('s', 500);
    stopTime = millis();
    for (int i = 0; i < 4; i++) {
      previousReadings[1][i] = -1;//previousReadings[0][i];
      previousReadings[0][i] = -1;//sensorReadings[i];
    }
    Serial.print("now = ");
    Serial.print(millis());
    Serial.print("  stopTime = ");
    Serial.print(stopTime);
    Serial.print("  beginTime = ");
    Serial.println(beginTime);
    //delay(2000);
  }

  if (alarmOn) {
    for (int i = 0; i < 4; i++) {
      previousReadings[1][i] = previousReadings[0][i];
      previousReadings[0][i] = sensorReadings[i];
    }
    if (moveNow) {

      readSensors(false);
      
      // if front detect
      if (sensorReadings[0] <= dangerZone && sensorReadings[0] >= 0
        && previousReadings[0][0] <= dangerZone && previousReadings[0][0] >= 0) {
        // stop
        
        moveRobot('s', 150);
        // check both sides similar
        if (sensorReadings[1] <= safeZone && sensorReadings[1] >= 0 &&
        sensorReadings[2] <= safeZone && sensorReadings[2] >= 0) {
          // check which side is closer
//          if (sensorReadings[1] < sensorReadings[2]) {
            moveRobot('b', 200);
            int randMove = random(0,2);
            Serial.print("Random # = ");
            Serial.println(randMove);
            if (randMove == 0) {
              moveRobot('l', 500);
            }
            else {
              moveRobot('r', 500);
            }
            moveRobot('f', 50);
          }
          // check left closer
          else if (sensorReadings[1] < sensorReadings[2] && sensorReadings[1] <= safeZone
            && sensorReadings[1] >= 0) {
            moveRobot('r', 150);
            moveRobot('f', 100);
          }
          // check right closer
          else if (sensorReadings[2] > sensorReadings[1] && sensorReadings[2] <= safeZone
            && sensorReadings[2] >= 0) {
            moveRobot('l', 150);
            moveRobot('f', 100);
          }
          // turn around?
          else {
//            Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAAH!");
//            delay(5000);
            moveRobot('b', 200);
            int randMove = random(0,2);
            Serial.print("Random # = ");
            Serial.println(randMove);
            delay(1000);
            if (randMove == 0) {
              moveRobot('l', 500);
            }
            else {
              moveRobot('r', 500);
            }
          }
        }
//        else if (sensorReadings[1] <= safeZone && sensorReadings[1] >= 0) {
//          // turn right 90
//          moveRobot('r', 500);
//          moveRobot('f', 50);
//        }
//        else {
//          // turn left 90
//          moveRobot('l' , 500);
//          moveRobot('f', 50);
//        }
//      }
      
      // if left detect
      else if (sensorReadings[1] <= dangerZone && sensorReadings[1] >= 0
        && previousReadings[0][1] <= dangerZone && previousReadings[0][1] >= 0) {
        
        // turn right 5?
        moveRobot('l', 300 - sensorReadings[1]);
        moveRobot('f', 100);
        Serial.print("Turn ");
        Serial.println(300 - sensorReadings[1]);
      }
      
      // if right detect
      else if (sensorReadings[2] <= dangerZone && sensorReadings[2] >= 0
        && previousReadings[0][2] <= dangerZone && previousReadings[0][2] >= 0) {
        // turn left 5?
        
        moveRobot('r', 300 - sensorReadings[2]);
        moveRobot('f', 100);
        Serial.print("Turn ");
        Serial.println(300 - sensorReadings[2]);
      }
      
      // move forward
      else {
        // move forward
        moveRobot('f', 50);
      }

// check if stuck 
if (!stuckCheck) {
  if (abs(sensorReadings[0] - previousReadings[0][0]) <= 5 && abs(previousReadings[0][0] - previousReadings[1][0]) <= 5
    && abs(sensorReadings[1] - previousReadings[0][1]) <= 5 && abs(previousReadings[0][1] - previousReadings[1][1]) <= 5
    && abs(sensorReadings[2] - previousReadings[0][2]) <= 5 && abs(previousReadings[0][2] - previousReadings[1][2]) <= 5) {
      Serial.println("STUCK");
      //delay(2000);
      stuckReadings[0][0] = (sensorReadings[0] + previousReadings[0][0] + previousReadings[1][0]) / 3;
      stuckReadings[0][1] = (sensorReadings[1] + previousReadings[0][1] + previousReadings[1][1]) / 3;
      stuckReadings[0][2] = (sensorReadings[2] + previousReadings[0][2] + previousReadings[1][2]) / 3;
      stuckCheck = true;
  }
}
else {
  if (stuckCount < 3) {
    ++stuckCount;
  }
  else {
    
//  if (abs(sensorReadings[0] - previousReadings[0][0]) <= 5 && abs(previousReadings[0][0] - previousReadings[1][0]) <= 5
//    && abs(sensorReadings[1] - previousReadings[0][1]) <= 5 && abs(previousReadings[0][1] - previousReadings[1][1]) <= 5
//    && abs(sensorReadings[2] - previousReadings[0][2]) <= 5 && abs(previousReadings[0][2] - previousReadings[1][2]) <= 5) {
      Serial.println("STILL STUCK");
      //delay(2000);
      stuckReadings[1][0] = (sensorReadings[0] + previousReadings[0][0] + previousReadings[1][0]) / 3;
      stuckReadings[1][1] = (sensorReadings[1] + previousReadings[0][1] + previousReadings[1][1]) / 3;
      stuckReadings[1][2] = (sensorReadings[2] + previousReadings[0][2] + previousReadings[1][2]) / 3;

      if (abs(stuckReadings[0][0] - stuckReadings[1][0]) <= 5
        && abs(stuckReadings[0][1] - stuckReadings[1][1]) <= 5 
        && abs(stuckReadings[0][2] - stuckReadings[1][2]) <= 5) {
        Serial.println("STUCK AND BACKING UP");
        //delay(2000);
        moveRobot('b', 500);
      }
//  }
    stuckCheck = false;
    stuckCount = 0;
    Serial.println("STUCK RESULTS:");
    Serial.print("1st read = ");
    Serial.print(stuckReadings[0][0]);
    Serial.print(",  ");
    Serial.print(stuckReadings[0][1]);
    Serial.print(",  ");
    Serial.println(stuckReadings[0][2]);
    Serial.print("2nd read = ");
    Serial.print(stuckReadings[1][0]);
    Serial.print(",  ");
    Serial.print(stuckReadings[1][1]);
    Serial.print(",  ");
    Serial.println(stuckReadings[1][2]);
}
}
Serial.println(stuckCount);
//if (sensorReadings[1] >=0 && previousReadings[0][1] >= 0 && previousReadings[1][1] >=0 ||
//  sensorReadings[2] >=0 && previousReadings[0][2] >= 0 && previousReadings[1][2] >=0) {
//      if (sensorReadings[1] < dangerZone && sensorReadings[1] >=0 &&
//        (sensorReadings[1] + previousReadings[0][1] + previousReadings[1][1]) / 3 < dangerZone) {
//          moveRobot('b', 200);
//      }
//      else if (sensorReadings[2] < dangerZone && sensorReadings[2] >=0 && 
//        (sensorReadings[2] + previousReadings[0][2] + previousReadings[1][2]) / 3 < dangerZone) {
//          moveRobot('b', 200);
//      }
//}
    }
    else {
      moveRobot('s', 0);
      readSensors(true);
      
      // stop
      if (sensorReadings[3] == 1 && previousReadings[0][3] == 1 &&
        previousReadings[1][3] == 1) {
        moveNow = true;
        beginTime = millis();
        // move forward
        moveRobot('f', 50);
      }
      else if (sensorReadings[0] < previousReadings[0][0] - 5
        && abs(previousReadings[0][0] - previousReadings[1][0]) <= 10 && sensorReadings[0] >= 0) {
        Serial.println("FRONT DETECT");
        delay(1000);
        moveNow = true;
        beginTime = millis();
        // move back, turn around 180, move forward
        moveRobot('b', 1000);
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
      else if (sensorReadings[1] < previousReadings[0][1] - 5 &&
        abs(previousReadings[0][1] - previousReadings[1][1]) <= 10 && sensorReadings[1] >= 0) {
        Serial.println("LEFT DETECT");
        delay(1000);
        moveNow = true;
        beginTime = millis();
        // turn right 90, move forward
        moveRobot('r', 500);
        moveRobot('f', 50);
      }
      else if (sensorReadings[2] < previousReadings[0][2] - 5 &&
        abs(previousReadings[0][2] - previousReadings[1][2]) <= 10 && sensorReadings[2] >= 0) {
        Serial.println("RIGHT DETECT");
        delay(1000);
        moveNow = true;
        beginTime = millis();
        // turn left 90, move forward
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
      else {
//        for (int i = 0; i < 4; i++) {
//          previousReadings[1][i] = previousReadings[0][i];
//          previousReadings[0][i] = sensorReadings[i];
        ////////}
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
int reading[3][3], totalRead;

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

  delay(50);
 
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

  delay(50);
  
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

//  delay(50);

  
  // read motion detector
  sensorReadings[3] = -1;
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

// moves the robot in the specified direction for the specified
// amount of time by controlling the motors
void moveRobot(char dir, int amt) {
  switch (dir) {
    case 's':  // stop
      digitalWrite(motor1a, LOW);
      digitalWrite(motor1b, LOW);
      digitalWrite(motor2a, LOW);
      digitalWrite(motor2b, LOW);
      delay(amt);
      break;
    case 'f':  // forward
//      digitalWrite(motor1a, HIGH);
        analogWrite(motor1a, 200);
      digitalWrite(motor1b, LOW);
      digitalWrite(motor2a, HIGH);
      digitalWrite(motor2b, LOW);
      delay(amt);
      break;
    case 'l':  // left
      digitalWrite(motor1a, HIGH);
      digitalWrite(motor1b, LOW);
//      digitalWrite(motor2a, LOW);
//      digitalWrite(motor2b, HIGH);
        analogWrite(motor2a,25);
        digitalWrite(motor2b,LOW);
      delay(amt);
      break;
    case 'r':  // right
//      digitalWrite(motor1a, LOW);
//      digitalWrite(motor1b, HIGH);
        analogWrite(motor1a, 25);
        digitalWrite(motor1b, LOW);
      digitalWrite(motor2a, HIGH);
      digitalWrite(motor2b, LOW);
      delay(amt);
      break;
    case 'b':  // backward
      digitalWrite(motor1b, HIGH);
      digitalWrite(motor1a, LOW);
      digitalWrite(motor2b, HIGH);
      digitalWrite(motor2a, LOW);
      delay(amt);
      break;
  }
} // end of function moveRobot(char, int)

