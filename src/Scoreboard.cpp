/**********************************************************************************
 *
 *	Scoreboard  --  Basketball shot clock and score counter
 *
 *	Counts successful hoops shot during a controlled countdown period 
 *	and displays on large 60mm 7-segment LED array
 *
 *  File:          Scoreboard.cpp
 * 
 *  Function:      Basketball Scoreboard main application file.
 * 
 *  Copyright:     Copyright (c) 2024 Kevin Barrell
 
 *                 Permission is hereby granted, free of charge, to anyone 
 *                 obtaining a copy of this document and accompanying files to do, 
 *                 whatever they want with them without any restriction, including,
 *                 but not limited to, copying, modification and redistribution.
 *                 The above copyright notice and this permission notice shall be 
 *                 included in all copies or substantial portions of the Software.
 * 
 *                 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY.
 * 
 *  License:       MIT License. See accompanying LICENSE file.
 * 
 *  Author:        Kevin Barrell
 * 
 *  Description:   Scoreboard and shot clock timer for indoor basketball hoop
 * ********************************************************************************
*/
#include 	<Arduino.h>
#include 	<Wire.h>
#include	"ezBuzzer.h" 		// ezBuzzer library
#include	"CountDown.h"		//  countdown timer library
#include 	<DFRobot_VL6180X.h> //  ranging ToF sensor
#include 	"LedControl_HW_SPI.h"
#include	"LedControl.h"		//  Digit-segment driver



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
const int driverAddr = 0;		// address of MAX7219 display driver
const int dispPin = 10;			// pin to select MAX7219 display controller
const unsigned long timeoutLimit = 300000;	// 5 min (in millisecs) timeout to shut down display

DFRobot_VL6180X VL6180X;
ezBuzzer buzzer(BUZZER_PIN); // create ezBuzzer object that attaches to a pin;
CountDown cdt;  			//  default millis

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl_HW_SPI lc = LedControl_HW_SPI();

volatile unsigned int contactBounceTime;		// Supports debouncing of pushbutton time
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
int	 remSecs	=	0;			// time remaining with seconds resolution
int	 preCount = 0;				// register for count during pre-shooting count
int	 currentBtnState;			// tracks state of trigger button
int  soundType = 0;
int	 loopcount = 0;
int  scoreCount = 0;							// current score total
unsigned long displayTimeout = 0;


//	ISR handler for ball detected through hoop
//  only triggered if shooting == true
void isr_scoreIt(){
	event = true;
}

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

//	Routine to display 2 digits for either Score count or Countdown timer
//       dispType = SCOREDISP (=0) or CLOCKDISP (=1)
//
void displayIt(int dispType, int numToDisp) {

	const bool decPt = false;	// never light up the decimal point
	static byte	tens[2], units[2];		// hold the digit values for display (score & counter)
	byte newUnits, newTens;		// from incoming numToDisp
	int	remToDisp;				// stipped of units value
	int digOffset;				// digits for countdown clock are displayed on score address + 2

	newUnits = numToDisp % 10;		// extract units value
	remToDisp = numToDisp / 10;
	newTens =  remToDisp % 10 ;		// extract tens value
	digOffset = 2 * dispType;		//  0 address offset for Score display, 2 for Countdown display

	if (units[dispType] != newUnits) {				// units is displayed on digits 0 & 2
		units[dispType] = newUnits;					// update units digit register
		lc.setDigit(driverAddr,0+digOffset,units[dispType],decPt);	// display new units value
	}

	if (tens[dispType] != newTens) {
		tens[dispType] = newTens;							//  tens displayed on digits 1 & 3
		lc.setDigit(driverAddr,1+digOffset,tens[dispType],decPt);	// and again for tens digit
	}

}

void setup() {
	Serial.begin(115200);
	Wire.begin(); //Start I2C library
    pinMode (LED_BUILTIN,OUTPUT);
  	pinMode(BUTTON_PIN, INPUT_PULLUP);
	
	soundType = 0;
	scoreCount = 0;
	displayTimeout = millis();
	cdt.stop();
	shooting = false;

	while(!(VL6180X.begin())){
    	Serial.println("Please check that the IIC device is properly connected!");
    	delay(1000);
  	}  
 	 /** Enable the notification function of the INT pin
 	  * mode：
 	  * VL6180X_DIS_INTERRUPT          Not enable interrupt
 	  * VL6180X_LOW_INTERRUPT          Enable interrupt, by default the INT pin outputs low level
  	 * VL6180X_HIGH_INTERRUPT         Enable interrupt, by default the INT pin outputs high level
 	  * Note: When using the VL6180X_LOW_INTERRUPT mode to enable the interrupt, please use "RISING" to trigger it.
 	  *       When using the VL6180X_HIGH_INTERRUPT mode to enable the interrupt, please use "FALLING" to trigger it.
 	  */
 	VL6180X.setInterrupt(/*mode*/VL6180X_HIGH_INTERRUPT); 

  	/** Set the interrupt mode for collecting ambient light
  	 * mode 
  	 * interrupt disable  :                       VL6180X_INT_DISABLE             0
  	 * value < thresh_low :                       VL6180X_LEVEL_LOW               1 
  	 * value > thresh_high:                       VL6180X_LEVEL_HIGH              2
  	 * value < thresh_low OR value > thresh_high: VL6180X_OUT_OF_WINDOW           3
  	 * new sample ready   :                       VL6180X_NEW_SAMPLE_READY        4
  	 */
  	VL6180X.rangeConfigInterrupt(VL6180X_OUT_OF_WINDOW);

  	/*Set the range measurement period*/
  	VL6180X.rangeSetInterMeasurementPeriod(/* periodMs 0-25500ms */200);

  	/*Set threshold value*/
  	VL6180X.setRangeThresholdValue(/*thresholdL 0-255mm */120,/*thresholdH 0-255mm*/255);

  	#if defined(ESP32) || defined(ESP8266)||defined(ARDUINO_SAM_ZERO)
  	attachInterrupt(digitalPinToInterrupt(D9)/*Query the interrupt number of the D9 pin*/,interrupt,FALLING);
  	#else
 	 /*    The Correspondence Table of AVR Series Arduino Interrupt Pins And Terminal Numbers
  	 * ---------------------------------------------------------------------------------------
  	 * |                                        |  DigitalPin  | 2  | 3  |                   |
  	 * |    Uno, Nano, Mini, other 328-based    |--------------------------------------------|
  	 * |                                        | Interrupt No | 0  | 1  |                   |
 	 * |-------------------------------------------------------------------------------------|
   	* |                                        |    Pin       | 2  | 3  | 21 | 20 | 19 | 18 |
   	* |               Mega2560                 |--------------------------------------------|
   	* |                                        | Interrupt No | 0  | 1  | 2  | 3  | 4  | 5  |
   	* |-------------------------------------------------------------------------------------|
   	* |                                        |    Pin       | 3  | 2  | 0  | 1  | 7  |    |
   	* |    Leonardo, other 32u4-based          |--------------------------------------------|
   	* |                                        | Interrupt No | 0  | 1  | 2  | 3  | 4  |    |
   	* |--------------------------------------------------------------------------------------
   */
  
  	attachInterrupt(/*Interrupt No*/1,isr_scoreIt,FALLING);	//Enable the external interrupt 1, connect INT1/2 to the digital pin of the main control: 
    //UNO(2), Mega2560(2), Leonardo(3), microbit(P0).
  	#endif

  	/*Start continuous range measuring mode */
  	VL6180X.rangeStartContinuousMode();

	/*
   	The MAX72XX is in power-saving mode on startup,
   	we have to do a wakeup call
   	*/

    lc.begin(dispPin,1,10000000);
	lc.shutdown(0,false);
  	lc.setIntensity(0,10);	// Set the brightness to a medium values 
  	lc.clearDisplay(0);		// and clear the display

}

void loop() {
	
	buzzer.loop(); // MUST call the buzzer.loop() function in loop()

	if (shooting) {							// Push button etc. is diabled while on shot clock
		if (remSecs == 0) {					// timer has expired
			shooting = false;
			soundIt(TIMESUP);
			cdt.stop();
			displayTimeout = millis();		// record current time to start display timeout
		} else if (event) {					// hoop detected
			scoreCount += 1;
			soundIt(BASKET);
			delay(200);					// allow time for ball to pass through without retriggering
			event = false;
			VL6180X.clearRangeInterrupt();
		}
		remSecs = cdt.remaining();			
	} else {								// Push button enabled
   		currentBtnState = digitalRead(BUTTON_PIN);
		if (lastButtonState == HIGH && currentBtnState == LOW) {
			remSecs	=	Fullcount;
			preCount = remSecs;
			scoreCount = 0;
			lc.shutdown(0,false);			//  make shure display is awake
			displayTimeout = millis();		//  renew display timer
			cdt.start(0,0,0,Fullcount);		//  start countdown clock in secs	
		} 
		else if ((millis() - displayTimeout) > timeoutLimit) {
			lc.shutdown(0, true);					// shutdown display after inactivity period
		}
		lastButtonState = currentBtnState;
		if (cdt.isRunning() ) {						// clock has started
			remSecs = cdt.remaining();
			if (remSecs <= ShotClock) {
				shooting = true;
				event = false;
				VL6180X.clearRangeInterrupt();
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
