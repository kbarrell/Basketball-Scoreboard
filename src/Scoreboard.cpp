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
#include	"Mux.h"				//  Multiplexor 
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

VL6180xIdentification identification;
VL6180x sensor(VL6180X_ADDRESS);
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

unsigned long dispDelay=250;	/* we always wait a bit between updates of the display */
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
    pinMode (LED_BUILTIN,OUTPUT);
  	pinMode(BUTTON_PIN, INPUT_PULLUP);

	soundType = 0;
	scoreCount = 0;
	contactBounceTime = millis();
	cdt.stop();
	shooting = false;

	pinMode(trigPin, INPUT_PULLUP);
  	attachInterrupt(digitalPinToInterrupt(trigPin), isr_scoreIt, FALLING);
  	Wire.begin(); //Start I2C library

  	if(sensor.VL6180xInit() != 0){
   		 Serial.println("FAILED TO INITALIZE"); //Initialize device and check for errors
    }

  	sensor.VL6180xDefaultSettings(); //Load default settings to get started.
  	delay(100); // delay 0.1s

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
	sensor.VL6180xSetDistInt(100,255); 
	sensor.getDistanceContinously();
 
	/*
   	The MAX72XX is in power-saving mode on startup,
   	we have to do a wakeup call
   	*/
    lc.begin(dispPin,1,10000000);
	lc.shutdown(0,false);
  	lc.setScanLimit(0,4);	// Only 4 digits in our scoreboard readout
  	lc.setIntensity(0,8);	// Set the brightness to a medium values 
  	lc.clearDisplay(0);		// and clear the display

  	noInterrupts();
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


