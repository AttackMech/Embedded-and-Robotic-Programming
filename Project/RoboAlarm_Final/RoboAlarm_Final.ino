#include <Wire.h>
#include <LiquidCrystal_SR3W.h>
#include <DS1302RTC.h>
#include <Time.h>
#include <EEPROM.h>

// set up lcd and rtc
LiquidCrystal_SR3W lcd(5,4,3);
DS1302RTC rtc(13,7,6);

// constants
// alarm controls
const int buzzerPin = 8;
const int upButtonPin = 0;
const int downButtonPin = 1;
const int setButtonPin = 2;
const int alarmRate = 1;
const int LCDOnTime = 7000;

// motors
const int motor1a = 12;
const int motor1b = 11;
const int motor2a = 9;
const int motor2b = 8;

// movement controls
const int moveTime = 30000;
const int stopper = 1000;
const int dangerZone = 15;
const int safeZone = 30;

// IR sensor
const int pirPin = 13;
const int pause = 2000;

// ultrasonic sensors
const int echoPinFront = A5;
const int trigPinFront = A4;
const int echoPinLeft = A3;
const int trigPinLeft = A2;
const int echoPinRight = A1;
const int trigPinRight = A0;

const int maxRange = 300;  // max distance needed
const int minRange = 0;  // min distance needed

// variables
// alarm/clock
time_t timeNow;
time_t alarmTime;
unsigned long int LCDTimer;
byte alarmHrs, alarmMins;

// buttons
bool setPress, upPress, downPress;
bool alarmOn = false;
bool LCDOn = true;

// IR and ultrasonic sensors
bool lockLow = true;
bool takeLowTime;
unsigned long int beginTime, lowIn, stopTime;
long duration, distance;
int sensorReadings[4] = {-1, -1, -1, -1};
int previousReadings[2][4] = {{-1, -1, -1, -1}, {-1, -1, -1, -1}};

// movement
bool moveNow = false;
bool stuckCheck = false;
int stuckReadings[2][3];
int stuckCount = 0;


void setup() {
Serial.begin(9600);
  // set pins for operation
  pinMode(buzzerPin, OUTPUT);
  pinMode(setButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

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

  // set system time from RTC
  setTime(rtc.get());

  // get alarm settings from stored values
  alarmHrs = EEPROM.read(0);
  alarmMins = EEPROM.read(1);
      
  // set up LCD
  lcd.begin(16,2); 
  lcd.clear();

  // print lcd message with date
  lcd.print("Robo-Alarm");
  printDateToLCD(0,1);
  delay(3000);

  // print alarm time
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Alarm set for:");
  printAlarmToLCD(6,1);
  delay(3000);

  // print current time
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Current Time");
  printTimeToLCD(4,1);

  // set alarm 
  alarmTime = setAlarm(alarmHrs, alarmMins);

  // set LCD to turn off after time
  LCDTimer = millis() + LCDOnTime;


} // end of setup()



void loop() {

  // get time and display on LCD
  if (timeNow < now()) {
    timeNow = now();
    printTimeToLCD(4,1);
  
    // check for alarm
    if (!alarmOn && timeNow == alarmTime) {
      alarmOn = true;
      moveNow = true;
      beginTime = millis();    
    }
  }

  // check if buttons pushed
  checkButtons();

  // check if LCD display stay/turn on
  if (setPress || upPress || downPress) {
    if (LCDOn) {
      
      // check if user wants to set alarm
      if (setPress  && !alarmOn && LCDTimer - millis() <= 6500) {
        setAlarm();
      }
      LCDTimer = millis() + LCDOnTime;
    }
    else {  
      LCDOn = true;
      lcd.display();
      LCDTimer = millis() + LCDOnTime;
    }
  }

  // check for alarm off button
  if (alarmOn && (setPress || upPress || downPress)) {
      alarmOn = false;
      setAlarm(alarmHrs, alarmMins);
      LCDOn = true;
      LCDTimer = millis() + LCDOnTime;
      moveNow = false;
  }

  // play alarm sound 
  if (alarmOn && ((timeNow - alarmTime) % 2 == 0)) {
    tone(buzzerPin, 500, 150);
    lcd.display();
    LCDOn = true;
  }

  // turn off LCD after 7 sec
  if (millis() >= LCDTimer && LCDOn && !alarmOn) {
    turnLCDOff();
  }
  
  // check move timing
  if (moveNow && millis() - beginTime >= moveTime) {
    moveNow = false;
    moveRobot('s', 200);
    stopTime = millis();
    
    for (int i = 0; i < 4; i++) {
      previousReadings[1][i] = -1;//previousReadings[0][i];
      previousReadings[0][i] = -1;//sensorReadings[i];
    }
  }

  if (alarmOn) {
    for (int i = 0; i < 4; i++) {
      previousReadings[1][i] = previousReadings[0][i];
      previousReadings[0][i] = sensorReadings[i];
    }
    // check if robot needs to move
    if (moveNow) {

      readSensors(false);
      
      // if front detect
      if (sensorReadings[0] <= dangerZone && sensorReadings[0] >= 0
        && previousReadings[0][0] <= dangerZone && previousReadings[0][0] >= 0)
      {
        
        moveRobot('s', 150);  // stop
        
        // check both sides similar
        if (sensorReadings[1] <= safeZone && sensorReadings[1] >= 0 &&
          sensorReadings[2] <= safeZone && sensorReadings[2] >= 0)
        {
          moveRobot('b', 300); // move back
            
          // select random turn direction
          int randMove = random(0,2);
          if (randMove == 0) {
            moveRobot('l', 500);
          }
          else {
            moveRobot('r', 500);
          }
          moveRobot('f', 100);
        }
        
        // check left closer
        else if (sensorReadings[1] < sensorReadings[2] && sensorReadings[1] <= safeZone
          && sensorReadings[1] >= 0)
        {
          moveRobot('r', 150);
          moveRobot('f', 100);
        }
        
        // check right closer
        else if (sensorReadings[2] > sensorReadings[1] && sensorReadings[2] <= safeZone
          && sensorReadings[2] >= 0)
        {
          moveRobot('l', 150);
          moveRobot('f', 100);
        }
        
        // turn around randomly when room
        else {
          moveRobot('b', 200);
          
          // select random turn direction
          int randMove = random(0,2);
          if (randMove == 0) {
            moveRobot('l', 500);
          }
          else {
            moveRobot('r', 500);
          }
          moveRobot('f', 100);
        }  // end of else
      }  // end of if
      
      // check for left detect
      else if (sensorReadings[1] <= dangerZone && sensorReadings[1] >= 0
        && previousReadings[0][1] <= dangerZone && previousReadings[0][1] >= 0)
      {
        
        // turn right based on how close to obstacle
        moveRobot('l', 300 - sensorReadings[1]);
        moveRobot('f', 100);
      }
      
      // check for right detect
      else if (sensorReadings[2] <= dangerZone && sensorReadings[2] >= 0
        && previousReadings[0][2] <= dangerZone && previousReadings[0][2] >= 0)
      {
        // turn left based on how close to obstacle
        
        moveRobot('r', 300 - sensorReadings[2]);
        moveRobot('f', 100);
      }
      
      // move forward when no detections in danger zone
      else {
        moveRobot('f', 50);
      }

      // check if stuck by comparing previous readings
      if (!stuckCheck) {
        if (abs(sensorReadings[0] - previousReadings[0][0]) <= 5 && abs(previousReadings[0][0] - previousReadings[1][0]) <= 5
          && abs(sensorReadings[1] - previousReadings[0][1]) <= 5 && abs(previousReadings[0][1] - previousReadings[1][1]) <= 5
          && abs(sensorReadings[2] - previousReadings[0][2]) <= 5 && abs(previousReadings[0][2] - previousReadings[1][2]) <= 5)
        {
            // take average of all stored readings
            stuckReadings[0][0] = (sensorReadings[0] + previousReadings[0][0] + previousReadings[1][0]) / 3;
            stuckReadings[0][1] = (sensorReadings[1] + previousReadings[0][1] + previousReadings[1][1]) / 3;
            stuckReadings[0][2] = (sensorReadings[2] + previousReadings[0][2] + previousReadings[1][2]) / 3;
            stuckCheck = true;
        }
      }
      else {
        // wait for old readings to clear
        if (stuckCount < 3) {
          ++stuckCount;
        }
        else {
          // calculate average readings
          stuckReadings[1][0] = (sensorReadings[0] + previousReadings[0][0] + previousReadings[1][0]) / 3;
          stuckReadings[1][1] = (sensorReadings[1] + previousReadings[0][1] + previousReadings[1][1]) / 3;
          stuckReadings[1][2] = (sensorReadings[2] + previousReadings[0][2] + previousReadings[1][2]) / 3;

          // check if average readings similar to previous
          if (abs(stuckReadings[0][0] - stuckReadings[1][0]) <= 5
            && abs(stuckReadings[0][1] - stuckReadings[1][1]) <= 5 
            && abs(stuckReadings[0][2] - stuckReadings[1][2]) <= 5)
          {  
            // robot stuck so try to back up
            moveRobot('b', 500);
          }
          stuckCheck = false;
          stuckCount = 0;
        } // end of else
      } // end of else
    } // end of if

    // when stopped, check for motion and move away
    else {
      moveRobot('s', 10);
      readSensors(true);
        
      // check for motion in rear
      if (sensorReadings[3] == 1 && previousReadings[0][3] == 1 &&
        previousReadings[1][3] == 1)
      {
        moveNow = true;
        beginTime = millis();
        // move forward
        moveRobot('f', 50);
      }
      
      // check for motion in front
      else if (sensorReadings[0] < previousReadings[0][0] - 5
        && abs(previousReadings[0][0] - previousReadings[1][0]) <= 10 && sensorReadings[0] >= 0)
      {
        moveNow = true;
        beginTime = millis();
        // move back, turn around, move forward
        moveRobot('b', 1000);
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
      
      // check for motion on left
      else if (sensorReadings[1] < previousReadings[0][1] - 5 &&
        abs(previousReadings[0][1] - previousReadings[1][1]) <= 10 && sensorReadings[1] >= 0)
      {
        moveNow = true;
        beginTime = millis();
        // turn right, move forward
        moveRobot('r', 500);
        moveRobot('f', 50);
      }
      
      // check for motion on right
      else if (sensorReadings[2] < previousReadings[0][2] - 5 &&
        abs(previousReadings[0][2] - previousReadings[1][2]) <= 10 && sensorReadings[2] >= 0)
      {
        moveNow = true;
        beginTime = millis();
        // turn left, move forward
        moveRobot('l', 500);
        moveRobot('f', 50);
      }
  
      // restart movement if stopped for too long
      if (millis() - stopTime >= stopper) {
        moveNow = true;
        beginTime = millis();
        // move forward
        moveRobot('f', 50);
      } // end of if
    } // end of else
  } // end of if

  // alarm is off so no movement
  else {
    moveRobot('s', 0);
  }
  
} // end of loop()



// prints date to LCD as DD-MM-YYYY
void printDateToLCD(int x, int y) {
  
  lcd.setCursor(x,y);
  
  if (zeroDigit(day()))
    lcd.print("0");
  
  lcd.print(day());
  lcd.print("-");
  
  if (zeroDigit(month()))
    lcd.print("0");
  
  lcd.print(month());
  lcd.print("-");
  lcd.print(year());
} // end of function printDateToLCD(int, int)


// prints time to LCD as HH:MM:SS
void printTimeToLCD(int x, int y) {
  
  lcd.setCursor(x,y);
  
  if (zeroDigit(hour()))
    lcd.print("0");
  
  lcd.print(hour());
  lcd.print(':');
  
  if (zeroDigit(minute()))
    lcd.print("0");
  
  lcd.print(minute());
  lcd.print(':');
  
  if (zeroDigit(second()))
    lcd.print("0");
  
  lcd.print(second());
} // end of function printTimeToLCD(int, int)


// prints the alarm time on the LCD display
void printAlarmToLCD(int x, int y) {
  
  lcd.setCursor(x,y);
  
  if (zeroDigit((int) alarmHrs))
    lcd.print("0");
  
  lcd.print((int) alarmHrs);
  lcd.print(':');
  
  if (zeroDigit((int) alarmMins))
    lcd.print("0");
  
  lcd.print((int) alarmMins);
} // end of function printAlarmToLCD(int, int)


// checks if passed value is a single digit
bool zeroDigit(int value) {
  
  if (value < 10)
    return true;
  
  else
    return false;
} // end of function zeroDigit(int)


// sets the alarm time for the system
time_t setAlarm(byte hrs, byte mins) {
  
  tmElements_t setTime;
  
  setTime.Year = (byte) year() - 1970;
  setTime.Month = (byte) month();

  // check if alarm is for next day
  if ((int) hrs < hour() || ((int) hrs == hour() && (int) mins <= minute()))
    setTime.Day = (day() + 1);
  
  else
    setTime.Day = day();
  
  setTime.Hour = hrs;
  setTime.Minute = mins;
  setTime.Second = 0;

  // write values to EEPROM memory
  EEPROM.write(0, hrs);
  EEPROM.write(1, mins);

  // return time value as time_t
  return makeTime(setTime);
} // end of function setAlarm(byte, byte)


// turns the LCD off
void turnLCDOff() {
  lcd.noDisplay();
  LCDOn = false;    
} // end of function turnLCDOff()


// allow user to set the alarm
void setAlarm() {
  
  lcd.clear();
  lcd.home();
  lcd.print("Alarm time:");
  printAlarmToLCD(0,1);
  delay(1500);
  
  lcd.home();
  lcd.print("Set hours: ");
    
  // enter setting mode
  while (true) {
    
    // set the hours
    while (true) {
      if (digitalRead(upButtonPin) == LOW) {
        if ((int) ++alarmHrs > 24)
          alarmHrs = (byte) 0;
          delay(500);
      }
      if (digitalRead(downButtonPin) == LOW) {
        if ((int) --alarmHrs == 255)
          alarmHrs = (byte) 24;
          delay(500);
      }
      printAlarmToLCD(0,1);
      if (digitalRead(setButtonPin) == LOW) {
        delay(500);
        break;
      }
    }

    lcd.home();
    lcd.print("Set minutes:");
    
    // set the minutes
    while (true) {
      if (digitalRead(upButtonPin) == LOW) {
        if ((int) ++alarmMins > 59)
          alarmMins = (byte) 0;
          delay(500);
      }
      if (digitalRead(downButtonPin) == LOW) {
        if ((int) --alarmMins == 255)
          alarmMins = (byte) 59;
          delay(500);
      }
      printAlarmToLCD(0,1);
      if (digitalRead(setButtonPin) == LOW) {
        delay(500);
        break;
      }
    }

    alarmTime = setAlarm(alarmHrs, alarmMins);
        
    lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Alarm set for:");
  printAlarmToLCD(6,1);
  delay(3000);
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Current Time");
    printTimeToLCD(4,1);
    break;    
  }
  LCDTimer = millis() + LCDOnTime;
} // end of function setAlarm()


// check button states
void checkButtons() {
  
  if (digitalRead(setButtonPin) == LOW)
    setPress = true;
  else
    setPress = false;
  if (digitalRead(upButtonPin) == LOW)
    upPress = true;
  else
    upPress = false;
  if (digitalRead(downButtonPin) == LOW)
    downPress = true;
  else
    downPress = false;
} // end of function checkButtons()


// reads ultrasonic sensors + IR sensor if required
void readSensors(bool all) {

  // read front ultrasonic sensor
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
 
  // read left ultrasonic sensor
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
  
  // read right ultrasonic sensor
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
  delay(50);
  
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
      } // end of if
      
    } // end of else
  } // end of if
} // end of function readSensors()


// controls motors to move robot in specified direction
// for specified time length
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
      analogWrite(motor1a, 220);
      analogWrite(motor2a, 250);
      digitalWrite(motor1b, LOW);
      digitalWrite(motor2b, LOW);
      delay(amt);
      break;
    case 'l':  // left
      digitalWrite(motor1a, HIGH);
      digitalWrite(motor1b, LOW);
      analogWrite(motor2a,25);
      digitalWrite(motor2b,LOW);
      delay(amt);
      break;
    case 'r':  // right
      analogWrite(motor1a, 25);
      digitalWrite(motor1b, LOW);
      digitalWrite(motor2a, HIGH);
      digitalWrite(motor2b, LOW);
      delay(amt);
      break;
    case 'b':  // backward
      analogWrite(motor1b, 220);
      digitalWrite(motor1a, LOW);
      analogWrite(motor2b, 250);
      digitalWrite(motor2a, LOW);
      delay(amt);
      break;
  } // end of switch
} // end of function moveRobot(char, int);

