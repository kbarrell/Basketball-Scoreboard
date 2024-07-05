/*
  FILENAME:   scoreKeeper.ino
  AUTHOR:     Orlando S. Hoilett
  EMAIL:      orlandohoilett@gmail.com
  VERSION:    0.3
  
  AFFILIATIONS:
  Calvary Engineering Family Group, USA
  A group of electronics and science enthusiasts who enjoy building things

  UPDATES:
  Version 0.0
  2014/11/18:1200>
              Added ultrasonic distance sensor code.
  2014/11/20:0740>
              Added code for triggering each segment of the seven
              segment display
  2014/11/20:1800>
              Added functionality to print number to custom
              7-segment display
  2014/11/27:1523> PRESUMED FUNCTIONAL
              This verison was tested on LEDs and is preseumed
              functional up to this point. Also, units control
              has been removed. I was mixing this code up with the
              temp-display-t-shirt that I am planning.
  Version 0.1
  2014/12/03:0509>
              Redefined methods using my Mux library. I am trying
              to use a mux to control the seven segment display.
              Functional.
  Version 0.3
  2014/12/27:1701>
              Made a few comments and changed name to "scoreKeeper."
              Stemmed from autoscore_basketball.ino codes. Functional.
                
  MATERIALS LIST
  1 x indoor basketball hoop
  1 x indoor basketballs
  1 x Arduino (I used an Arduino Pro Mini, but any Arduino will do)
  1 x battery for external power
  1 x 16-Channel Multiplexer (I used CD74HC4067)
  1 x 200 Ohm - 1 kOhm resistor
  1 x Ultrasonic distance sensor (I used HC-SR04)
  28 x LEDs
  Lots of wire
  Hot glue
  14 pieces of aluminum foil
  Appropriate material for the baseboard
              
  DESCRIPTION  
  Source code accompanying Score Detection Hoop System Instructable.
  Detects when a shot has been on an indoor basektball hoop using
  an ultrasonic distance sensor and increments the score. The score
  is displayed on a custom two digit 7-segment display which is
  controlled with a 4-bit (16 channel) analog multiplexer.
  
  SOURCES
  1.chickenparmi. "Arduino Tutorial #6 - HC-SR04 Ultrasonic sensor."
    Online video. YouTube. YouTube, 15 Feb. 2013. Web. 8 May 2013.
    <https://www.youtube.com/watch?v=PG2VhpkPqoA>.
  2.Kurt E. Clothier. "DIY Large LED Lit 7 Segment Display."
    Instructables.com. November 9, 2012.
  
  DISCLAIMER
  This code is in the public domain. Please feel free to modify,
  use, etc however you see fit. But, please give reference to
  original authors as a courtesy to Open Source developers.

*/
#include <Arduino.h>
#include "Mux.h"

/*
*    __A__
*   |     |
* F |     | B
*   |__G__|	  MSB > DOT | G | F | E| D | C | B | A | < LSB
*   |     |
* E |     | C
*   |__D__|   
*/
//Borrowed from Kurt E. Clothier and his "DIY Large LED Lit 7
//Segment Display" tutorial published on Instructables.com on
//November 9, 2012.

//Hexadecimal representation of each number if the segments of
//the segment represent a single bit of an 8-bit number.
const int SEGMENT0 = 0x3F;	// Zero
const int SEGMENT1 = 0x06;	// One
const int SEGMENT2 = 0x5B;	// Two
const int SEGMENT3 = 0x4F;	// Three
const int SEGMENT4 = 0x66;	// Four
const int SEGMENT5 = 0x6D;	// Five
const int SEGMENT6 = 0x7D;	// Six
const int SEGMENT7 = 0x07;	// Seven
const int SEGMENT8 = 0x7F;	// Eight
const int SEGMENT9 = 0x67;	// Nine
int HEXvalues[] = { SEGMENT0, SEGMENT1, SEGMENT2, SEGMENT3,
                    SEGMENT4, SEGMENT5, SEGMENT6, SEGMENT7,
                    SEGMENT8, SEGMENT9 };
//each index of the HEXvalues array is equal to that value I want
//to print to the digit. The 0th index is value = 0, the 1st
//index is value = 1, and so on. This is really helpful for
//minimizing code.


//distance sensor pins
const int trigPin = 12;
const int echoPin = 13;

Mux myMux = Mux(2,3,4,5,6);
volatile int index = 0;

const double scoreThreshold = 8; //8 cm
int score = 00;
unsigned long lastShot = 0;
unsigned long waitTime = 3000; //3 seconds

//boolean detectScore()
//@return      true if shot is detected, false if otherwise
//
//Detects whether or not a shot was made by checking if the
//distance from the ultrasonic distance sensor to the next closest
//object is under the "threshold."
boolean detectScore()
{
  return (distance() <= scoreThreshold);
}


//double distance()
//@return      the distance in centimeters (cm)
//
//Calculates the distance from the sensor to the next closest
//object.
double distance()
{
  double duration = 0;

  digitalWrite(trigPin, HIGH); //send out pulse
  delayMicroseconds(50); //give the pulse time
  digitalWrite(trigPin, LOW); //turn off pulse
  duration = pulseIn(echoPin, HIGH); //read echo pin
  
  return (duration/2) / 29.1; //in cm
}


//void incrementScore()
//Increments the socre variable by 1.
void incrementScore()
{
  score += 1;
}


//Interrupt Service Routine
//Displays the numbers for the score on the 7-segment display.
//It lights a single segment every 1 ms incrementing the segment
//index every iteration. The "if (bitRead(xxx))" statements
//determine which segment needs to be lit based upon the hexadecimal
//representation of the number we are trying to display and how
//it relates to the segments of the 7-segment display. See
//comments at the beginning of the code. Assumes index variable
//is non-negative.
ISR(TIMER1_COMPA_vect)
{
  //Ones digit
  if (index < 8) {
    if (bitRead(HEXvalues[score%10],index))
    {
      myMux.open(index);
    }
    index++;
  }
  //Tens digit
  else if (index >= 8 && index < 16) {
    if (bitRead(HEXvalues[score/10],index-8))
    {
      myMux.open(index);
    }
    index++;
  }
  //resets index if index > 15
  else {
    index = 0;
  }
}



void setup ()
{
  Serial.begin(9600);


  
  //set timer1 interrupt at 1Hz
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  TIMSK1 |= (1 << OCIE1A);
  sei();
}


//void loop()
//Main portion of the program. Loop runs continuously until the
//Arduino loses power. The Arduino checks to see if a shot has
//been made.
void loop()
{
  if(detectScore() && millis()-lastShot >= waitTime)
  {
    incrementScore();
    lastShot = millis();
  }
}


