/*
March 19, 2014
 Tannis Bo
 #2225604
 
 This sketch takes in the number of degrees the user
 wants to move the elbow and then move the elbow accordingly.
 If the result goes below 0 degrees, it will stop there and if it 
 goes beyond 170 it will stop there.
 */

/* Use the Servo library */
#include <Servo.h>

/* The servo is connected to PIN 12 on this circuit */
const int servoPin  = 12;
const int lowerLimit = 0; /* The elbow can go straight */
const int upperLimit = 170; /*The maximum the elbow can extend*/
const int startPosition = 90; /* Position it starts at every time*/
Servo myElbow; 


/* Setup for the elbow.  Set the servo up and open up serial 
 connection. */
void setup()
{
  myElbow.attach(servoPin);
  Serial.begin(9600);
}

/* Obtain the input from the user and move the elbow as requested
 within the boundaries of 0 and 170 degrees */
void loop()
{
  /* Provide a message to the user what they should enter */
  int degreesMove;
  Serial.println("Type the number of degrees you want the ");
  Serial.println("elbow to move in the box above, then click ");
  Serial.println("send or press enter.");
  Serial.println();

  /*Infinitly wait for input */
  while(true)
  {
    /* If they have entered something get it. */
    while (Serial.available() > 0)
    {
      degreesMove = Serial.parseInt();
      /* Ensure the value entered is acceptable by constraining
       it between the lower and upper limits of the elbow. */
      degreesMove = constrain(degreesMove, lowerLimit, upperLimit);

      /*Display to user on serial output what is happening. */
       Serial.print("Moving the elbow to ");
       Serial.print(degreesMove);
       Serial.println(" degrees.");
       
       /* Move to the starting position */
       myElbow.write(startPosition);
       /* Give it time to move there */
       delay(1000);
       /* Move it to the location provided. */
       myElbow.write(degreesMove);
      /* Pause to allow the servo to move */
      delay(100);
    }
  }

}



