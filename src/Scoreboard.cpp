/*
	EasyBuzzer - Beep Sequence
	This example shows you how to create a sequence of beeps, at a given frequency.
	Copyright (c) 2017, Evert Arias
	MIT License
*/
#include 	<Arduino.h>
#include	<ezBuzzer.h> 		// ezBuzzer library
#include 	<Wire.h>


#define LAUNCHCOUNT 0			// sound countdown to start of shot timer
#define BASKET 	1				// sound when score detected
#define TIMESUP 2				// sound end of shooting window
#define  BounceInterval	15		// millsecs to allow for contact or detector bounce

const int BUTTON_PIN = 2;
const int BUZZER_PIN = 3;

volatile unsigned int contactBounceTime;		// Supports debouncing of pushbutton time
volatile int scoreCount = 0;							// current score total

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

int lastButtonState = HIGH; // the previous state from the input pin
int soundType = 0;
int	loopcount = 0;
unsigned long buzzerStartTime = 0;


ezBuzzer buzzer(BUZZER_PIN); // create ezBuzzer object that attach to a pin;

//	ISR handler for ball detected through hoop
void isr_scoreIt(){
	if ((millis() - contactBounceTime) > BounceInterval ) {  // debounce the switch contact.
        scoreCount++;
        contactBounceTime = millis();
    }
}

void soundIt( int eventType) {

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

void setup() {
	Serial.begin(115200);
    pinMode (LED_BUILTIN,OUTPUT);
  	pinMode(BUTTON_PIN, INPUT_PULLUP);
	soundType = 0;
	contactBounceTime = millis();
}

void loop() {
	
	buzzer.loop(); // MUST call the buzzer.loop() function in loop()

//	Monitor button for launch

   	int currentState = digitalRead(BUTTON_PIN);
	if (lastButtonState == HIGH && currentState == LOW) {

		runCountdown();
//   		 Serial.print("The button is pressed \t for soundType =");
//		 Serial.println(soundType);
//		 soundIt(soundType);
//		 soundType++;
	}
	lastButtonState = currentState;
}


