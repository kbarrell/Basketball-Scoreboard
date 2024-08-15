//We always have to include the library
#include <Arduino.h>
#include <Wire.h>
#include "LedControl_HW_SPI.h"
#include "LedControl.h"
#include	"ezBuzzer.h" 		// ezBuzzer library

#define LAUNCHCOUNT 0			// sound countdown to start of shot timer
#define BASKET 	1				// sound when score detected
#define TIMESUP 2				// sound end of shooting window

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl_HW_SPI lc=LedControl_HW_SPI();
const int BUZZER_PIN = 5;
ezBuzzer buzzer(BUZZER_PIN); // create ezBuzzer object that attaches to a pin;

/* we always wait a bit between updates of the display */
unsigned long delaytime=250;
int ledPin = 13;

// notes in the melody:
int melody[] = {
  NOTE_E5, NOTE_E4, NOTE_C4, NOTE_G4,
  NOTE_E5, NOTE_A5
};
// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
int noteDurations[] = {
	8, 8, 8, 2, 
	8, 1
};

//	Routine to sound out alerts for each event occurrence
void soundIt(int eventType) {

	int length = sizeof(noteDurations) / sizeof(int);

	switch (eventType) {
		case LAUNCHCOUNT:			// Get Ready beeps
			buzzer.beep(400);
			break;
		case BASKET:				// score detected
			buzzer.beep(200);	//  single beep for Score!
			break;
		case TIMESUP:			//  shooting window over
			buzzer.playMelody(melody, noteDurations, length);	// melody signals end
			break;
		default:
			Serial.println("Buzzer case switch fail!");
	}
}




/*
 This method will display the characters for the
 word "Arduino" one after the other on digit 0. 
 */
void writeArduinoOn7Segment() {
  lc.setChar(0,0,'a',false);
  delay(delaytime);
  lc.setRow(0,0,0x05);
  delay(delaytime);
  lc.setChar(0,0,'d',false);
  delay(delaytime);
  lc.setRow(0,0,0x1c);
  delay(delaytime);
  lc.setRow(0,0,B00010000);
  delay(delaytime);
  lc.setRow(0,0,0x15);
  delay(delaytime);
  lc.setRow(0,0,0x1D);
  delay(delaytime);
  lc.clearDisplay(0);
  delay(delaytime);
} 

/*
  This method will scroll all the hexa-decimal
 numbers and letters on the display. You will need at least
 four 7-Segment digits. otherwise it won't really look that good.
 */
void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc.setDigit(0,3,i,false);
    lc.setDigit(0,2,i+1,false);
    lc.setDigit(0,1,i+2,false);
    lc.setDigit(0,0,i+3,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
}


void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */

  Serial.begin(115200);
    pinMode (LED_BUILTIN,OUTPUT);
  

  lc.begin(10,1,10000000);
	Serial.println("lc begun");
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
  
}

void loop() { 
  buzzer.loop();
  writeArduinoOn7Segment();
  scrollDigits();
}
