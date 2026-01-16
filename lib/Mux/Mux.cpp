/*
* FILENAME:	Mux.cpp
* AUTHOR:	Orlando S. Hoilett
* EMAIL:	orlandohoilett@gmail.com
* VERSION:	0.0


* AFFILIATIONS
* Calvary Engineering Family Group, USA
*	A group of electronics and science enthusiasts who enjoy building things


* DESCRIPTION
* This source file provides function definitions for controlling a multiplexer.
* This is a very basic code that allows the user to declare a multiplexer object
* and programmatically open different channels of the multiplexer. It provides
* functionality for a multiplexer with a maximum of 32 channels (5-bit mux) by
* overloading the class constructor. This library utilizes high level Arduino
* functions.
*
* NOTE: "multiplexer" and "mux" are used interchangeable in this code.
	

* UPDATES
* Version 0.0
* 2014/07/03:1200>
*			Started writing the code. Stopped writing for a while because I
*			wasn't sure the method I wanted to use.
* 2014/12/02:1816>
*			Decided to go the direction of creating a single multiplexer up to
*			16 outputs.
* 2014/12/02:2022>
*			Wrote a few functions to control a single multiplexer with a maximum
*			of 32 outputs. There are constructors to initalize the mux, and a
*			write() function to output to a given channel.
* 2014/12/11:1834>
*			Changed "write()" function to "open()" to reflect the physical action
*			taking place with the mux. Added disclaimer and desciption. Added
*			"const" modifier to several functions: initializePins(), open(),
*			enable(), and disable().


* DISCLAIMER
* This code is in the public domain. Please feel free to use, modify, distribute,
* etc. as needed, but please give reference to original author as a courtesy to
* open source developing/-ers.
*
* If you find any bugs in the code, or have any questions, please feel free to
* contact me.


*/


#include "Mux.h"


//CONSTRUCTORS
//*****************************************************************************

//Mux(int S0, int E)
//This method is the constructor for a 1-bit (2-channel) multiplexer.
Mux::Mux(int S0, int E):S0(S0), E(E), bits(1)
{
	pins[0] = S0;
	pins[1] = E;

	initializePins();
	disable();
}


//Mux(int S0, int S1, int E)
//This method is the constructor for a 2-bit (4-channel) multiplexer.
Mux::Mux(int S0, int S1, int E):S0(S0), S1(S1), E(E), bits(2)
{
	pins[0] = S0;
	pins[1] = S1;
	pins[2] = E;

	initializePins();
	disable();
}


//Mux(int S0, int S1, int S2, int E)
//This method is the constructor for a 3-bit (8-channel) multiplexer.
Mux::Mux(int S0, int S1, int S2, int E):S0(S0), S1(S1), S2(S2), E(E), bits(3)
{
	pins[0] = S0;
	pins[1] = S1;
	pins[2] = S2;
	pins[3] = E;

	initializePins();
	disable();
}


//Mux(int S0, int S1, int S2, int S3, int E)
//This method is the constructor for a 4-bit (16-channel) multiplexer.
Mux::Mux(int S0, int S1, int S2, int S3, int E)
	:S0(S0), S1(S1), S2(S2), S3(S3), E(E), bits(4)
{
	pins[0] = S0;
	pins[1] = S1;
	pins[2] = S2;
	pins[3] = S3;
	pins[4] = E;

	initializePins();
	disable();
}


//Mux(int S0, int S1, int S2, int S3, int S4, int E)
//This method is the constructor for a 5-bit (32-channel) multiplexer.
Mux::Mux(int S0, int S1, int S2, int S3, int S4, int E)
	:S0(S0), S1(S1), S2(S2), S3(S3), S4(S4), E(E), bits(5)
{
	pins[0] = S0;
	pins[1] = S1;
	pins[2] = S2;
	pins[3] = S3;
	pins[4] = S4;
	pins[5] = E;
	
	initializePins();
	disable();
}

//*****************************************************************************


//void initializePins() const
//This method initializes the pins that write to the mux's selector pins
void Mux::initializePins() const
{
	for(int i = 0; i < bits+1; ++i)
	{
		pinMode(pins[i], OUTPUT);
	}
}


//void open(int channel) const
//Opens the selected channel of the mux.
void Mux::open(int channel) const
{
	disable();
	for(int i = 0; i < bits; ++i)
	{
		digitalWrite(pins[i], bitRead(channel, i));
	}
	enable();
}


//void enable() const
//Allows the mux to output to a channel.
void Mux::enable() const
{
	digitalWrite(E, LOW);
}


//void disable() const
//Does not allow mux to output to a channel.
void Mux::disable() const
{
	digitalWrite(E, HIGH);
}