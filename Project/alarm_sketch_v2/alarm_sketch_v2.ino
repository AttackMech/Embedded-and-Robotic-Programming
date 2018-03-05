#include <Wire.h>
#include <LiquidCrystal_SR3W.h>
#include <DS1302RTC.h>
#include <Time.h>
#include <EEPROM.h>

// set up lcd and rtc
LiquidCrystal_SR3W lcd(2,4,3);
DS1302RTC rtc(6,8,7);

// constants
const int buzzerPin = 5;
const int upButtonPin = 11;
const int downButtonPin = 10;
const int setButtonPin = 12;
const int alarmRate = 1;
const int LCDOnTime = 7000;

// variables
time_t timeNow;
time_t alarmTime;

unsigned long int LCDTimer;

bool setPress, upPress, downPress;
bool alarmOn = false;
bool LCDOn = true;

byte alarmHrs, alarmMins;


void setup() {

Serial.begin(9600);

  // set system time from RTC
  setTime(rtc.get());

  // set alarm setting from stored values
  alarmHrs = EEPROM.read(0);
  alarmMins = EEPROM.read(1);
  alarmTime = setAlarm(alarmHrs, alarmMins);
    
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
  
  // reset alarm in case it passes in the time elapsed
  alarmTime = setAlarm(alarmHrs, alarmMins);

  // print current time
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Current Time");
  printTimeToLCD(4,1);

  // set LCD to turn off
  LCDTimer = millis() + LCDOnTime;

  // set pins for operation
  pinMode(buzzerPin, OUTPUT);
  pinMode(setButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

}

void loop() {
  
  // get time and display on LCD
  if (timeNow < now()) {
    timeNow = now();
    printTimeToLCD(4,1);
  
    // check for alarm
    if (timeNow == alarmTime)
      alarmOn = true;
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
  }

  // play alarm sound (beeps every second)
  if (alarmOn && ((timeNow - alarmTime) % 2 == 0)) {
    tone(buzzerPin, 500, 50);
    lcd.display();
    LCDOn = true;
  }

  // turn off LCD after 7 sec
  if (millis() >= LCDTimer && LCDOn && !alarmOn) {
    turnLCDOff();
  }
}


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
}


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
}


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
}


// checks if passed value is a single digit
bool zeroDigit(int value) {
  
  if (value < 10)
    return true;
  
  else
    return false;
}


// sets the alarm time
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
}


// turns the LCD off
void turnLCDOff() {
  Serial.println("Turn OFF LCD");
  lcd.noDisplay();
  LCDOn = false;    
}


// set the alarm
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
}

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
}

