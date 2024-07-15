/*
	Scoreboard  --  Basketball shot clock and score counter

	Counts successful hoops shot during a controlled countdown period 
	and displays on large 60mm 7-segment LED array

*/
#include 	<Arduino.h>
#include 	<Wire.h>
#include	"ezBuzzer.h" 		// ezBuzzer library
#include	"CountDown.h"		//  countdown timer library
#include 	"VL6180X_WE.h"		//  distance ToF sensor



#define LAUNCHCOUNT 0			// sound countdown to start of shot timer
#define BASKET 	1				// sound when score detected
#define TIMESUP 2				// sound end of shooting window
#define BounceInterval	15		// millsecs to allow for contact or detector bounce
#define	ShotClock  30			// 30 sec shot window 
#define	Precount	5			//    ...plus 5sec count-in
#define	Fullcount  35			//	Clock start setting
#define SCOREDISP  0			// 	Select the score display digits
#define	CLOCKDISP  1			//  Select the timer display digits
#define VL6180X_ADDRESS 0x29

const int BUTTON_PIN = 2;
const int BUZZER_PIN = 5;
const int trigPin = 3;			//distance sensor pin

VL6180xIdentification identification;
VL6180x sensor(VL6180X_ADDRESS);
ezBuzzer buzzer(BUZZER_PIN); // create ezBuzzer object that attaches to a pin;
CountDown cdt;  			//  default millis

volatile unsigned int contactBounceTime;		// Supports debouncing of pushbutton time
volatile int scoreCount = 0;							// current score total
volatile bool event = false;			// distance sensor triggered  event

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

int lastButtonState = HIGH; 	// the previous state from the input pin
bool shooting = false;			// is shooting in progress?
int	 remSecs	=	Fullcount;	// time remaining with seconds resolution
int	preCount = 0;				// register for count during pre-shooting count
int	 currentBtnState;			// tracks state of trigger button
int soundType = 0;
int	loopcount = 0;
unsigned long buzzerStartTime = 0;
int gain;						//  sensor gain


//	ISR handler for ball detected through hoop
//  only triggered if shooting == true
void isr_scoreIt(){
    scoreCount += 1;
    sensor.VL6180xClearInterrupt();
}


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

void displayIt(int dispType, int numToDisp) {

}

void setup() {
	Serial.begin(115200);
    pinMode (LED_BUILTIN,OUTPUT);
  	pinMode(BUTTON_PIN, INPUT_PULLUP);

	soundType = 0;
	scoreCount = 0;
	contactBounceTime = millis();
	
	
  	Wire.begin(); //Start I2C library

}

void loop() {
  buzzer.loop(); // MUST call the buzzer.loop() function in loop()

  int startButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && startButtonState == LOW) {
    Serial.println("The START button is pressed");
    if (buzzer.getState() == BUZZER_IDLE) {
      int length = sizeof(noteDurations) / sizeof(int);
      buzzer.playMelody(melody, noteDurations, length); // playing
    }
	
  }

  lastButtonState = startButtonState;

}
