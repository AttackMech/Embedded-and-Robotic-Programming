


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

#include <SPI.h>
#include <LiquidCrystal.h>
#include <DS1302RTC.h>
#include <Time.h>
#include <EEPROM.h>
#include <SimpleTimer.h>

LiquidCrystal lcd(3);  // initalize LCD
/*   RTC pins
 *   I/O = 19
 *   SCLK = 18
 *   CE = 17
 *   set to CE,I/O,CLK
 */
DS1302RTC rtc(6 ,8, 7);            // initalize real-time clock (RTC)

const int buzzerPin = 5;
const int upButtonPin = 11;
const int downButtonPin = 10;
const int setButtonPin = 12;
const int alarmRate = 1;

time_t timeNow;
time_t alarmTime;

SimpleTimer timer;

bool alarmOn = false;
bool LCDOn = true;
byte alarmHrs, alarmMins;
int LCDTimer;

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
  LCDTimer = timer.setTimeout(1000, changeLCDState);

  lcd.noDisplay();
  delay(1000);
  lcd.display();
  
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

  // check if LCD display stay/turn on
  if (digitalRead(setButtonPin) == LOW || digitalRead(upButtonPin) == LOW || digitalRead(downButtonPin) == LOW) {
    Serial.println("BUTTON DETECTED");
    if (LCDOn)
      timer.restartTimer(LCDTimer);
    
    else {  
      changeLCDState();
      LCDTimer = timer.setTimeout(1000, changeLCDState);
    }
  }

  // check if user wants to set alarm
  if (LCDOn && digitalRead(setButtonPin) == LOW) {
    setAlarm();
  }

  // check for alarm off button
  if (alarmOn && (digitalRead(setButtonPin) == LOW)) {
      alarmOn = false;
      alarmTime = now() + 10;
  }

  // play alarm sound (beeps every second)
  if (alarmOn && ((timeNow - alarmTime) % 2 == 0)) {
    tone(buzzerPin, 500, 50);
    Serial.println("ALARM! ALARM! ALARM!");
  }

  Serial.print("  LCD = ");
  Serial.print(LCDOn);
  Serial.print("  Timer status = ");
  Serial.println(timer.isEnabled(LCDTimer));
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
  
  setTime.Year = (byte) year();
  setTime.Month = (byte) month();
  
  // check if alarm is for next day
  if ((int) hrs < hour() || ((int) hrs == hour() && (int) mins <= minute()))
    setTime.Day = (byte) (day() + 1);
  
  else
    setTime.Day = (byte) day();
  
  setTime.Hour = hrs;
  setTime.Minute = mins;
  setTime.Second = 0;

  // write values to EEPROM memory
  EEPROM.write(0, hrs);
  EEPROM.write(1, mins);

  // return time value as time_t
  return makeTime(setTime);
}


// turns the LCD on or off
void changeLCDState() {
Serial.print("  Changing LCD state to ");
  if (LCDOn) {
    lcd.noDisplay();
    LCDOn = false;
  }
  else {
    lcd.display();
    LCDOn = true;
  }
Serial.print(LCDOn);  
delay(5000);
}


// set the alarm
void setAlarm() {

  timer.disable(LCDTimer);
  
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
        if ((int) --alarmHrs < 0)
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
        if ((int) --alarmMins < 0)
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
  lcd.setCursor(0,1);
  lcd.print(hour(alarmTime));
  lcd.print(":");
  lcd.print(minute(alarmTime));
  lcd.print("--");
  lcd.print(day(alarmTime));
  delay(3000);
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Current Time");
    printTimeToLCD(4,1);
    timer.enable(LCDTimer);
    timer.restartTimer(LCDTimer);
    break;    
  }
}

