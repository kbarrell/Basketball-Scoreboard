/*
	EasyBuzzer - Beep Sequence
	This example shows you how to create a sequence of beeps, at a given frequency.
	Copyright (c) 2017, Evert Arias
	MIT License
*/
#include 	<Arduino.h>
#include 	<Wire.h>
#include	"ezBuzzer.h" 		// ezBuzzer library
#include	"CountDown.h"		//  countdown timer library
#include 	"VL6180X_WE.h"		//  distance ToF sensor
#include	"Mux.h"				//  Multiplexor 


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
	cdt.stop();
	shooting = false;

	pinMode(trigPin, INPUT_PULLUP);
  	attachInterrupt(digitalPinToInterrupt(trigPin), isr_scoreIt, FALLING);
	noInterrupts();
  	Wire.begin(); //Start I2C library

  	if(sensor.VL6180xInit() != 0){
   		 Serial.println("FAILED TO INITALIZE"); //Initialize device and check for errors
    }

  	sensor.VL6180xDefaultSettings(); //Load default settings to get started.
  	delay(100); // delay 0.1s

  //Input GAIN for light levels, 
  // GAIN_20     // Actual ALS Gain of 20
  // GAIN_10     // Actual ALS Gain of 10.32
  // GAIN_5      // Actual ALS Gain of 5.21
  // GAIN_2_5    // Actual ALS Gain of 2.60
  // GAIN_1_67   // Actual ALS Gain of 1.72
  // GAIN_1_25   // Actual ALS Gain of 1.28
  // GAIN_1      // Actual ALS Gain of 1.01
  // GAIN_40     // ActualALS Gain of 40

  /* Range Threshold Interrupt:
   * The interrupt is set up with VL6180xSetDistInt(low limit / high limit);
   * The interrupt is triggered if the measured distance value is OUTSIDE these 
   * limits. Keep in mind that the VL6180x will return a distance of 255 if 
   * nothing is in the measuring range.
   * Examples: 
   * low limit = 50, high limit = 150 => interrupt is triggered at < 50 and > 150
   * low limit = 50, high limit = 255 => interrupt is triggered at < 50
   * low limit = 0, high limit = 50 => interrupts is triggered at > 50
   */
	sensor.VL6180xSetDistInt(50,255); 
	sensor.getDistanceContinously();
  
  // ALS Threshold Interrupt:
  // sensor.VL6180xSetALSInt(GAIN_1,30,200);
  // sensor.getAmbientLightContinously(GAIN_1); 
}

void loop() {
	
	buzzer.loop(); // MUST call the buzzer.loop() function in loop()

	if (shooting) {							// Push button etc. is diabled while on shot clock
		if (cdt.isStopped()) {				// timer has expired
			shooting = false;
			soundIt(TIMESUP);
			noInterrupts();	
		} 
		remSecs = cdt.remaining();			
	} else {								// Push button enabled
   		currentBtnState = digitalRead(BUTTON_PIN);
		if (lastButtonState == HIGH && currentBtnState == LOW) {
			remSecs	=	Fullcount;
			preCount = remSecs;
			scoreCount = 0;
			cdt.start(0,0,0,Fullcount);		//  start countdown clock in secs	
		} 
		lastButtonState = currentBtnState;
		if (cdt.isRunning() ) {						// clock has started
			remSecs = cdt.remaining();
			if (remSecs <= ShotClock) {
				shooting = true;
				sensor.VL6180xClearInterrupt();
				interrupts();
			}
			else if (preCount > remSecs){
				soundIt(LAUNCHCOUNT);				// in pre-count phase
				preCount = remSecs;
			} 
		}	
	}
	displayIt(SCOREDISP, scoreCount);
	displayIt(CLOCKDISP, remSecs);
}


