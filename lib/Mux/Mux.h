/*
* FILENAME:	Mux.h
* AUTHOR:	Orlando S. Hoilett
* EMAIL:	orlandohoilett@gmail.com
* VERSION:	0.0


* AFFILIATIONS
* Calvary Engineering Family Group, USA
*	A group of electronics and science enthusiasts who enjoy building things


* DESCRIPTION
* This header file provides function declarations for controlling a multiplexer.
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


#ifndef MUX_H
#define MUX_H

#include "Arduino.h"

const int MAX_BITS = 6; //maximum # of selector pins + 1 enable pin

class Mux
{
private:

	//Maximum number of selector pins is 5. This would be for a 32-channel mux.
	int S0;
	int S1;
	int S2;
	int S3;
	int S4;
	int E;

	//number of selector pins and array containing
	//# of selector pins + 1 enable pin.
	int bits;
	int pins[MAX_BITS];


public:

	Mux(int S0, int E);

	Mux(int S0, int S1, int E);

	Mux(int S0, int S1, int S2, int E);

	Mux(int S0, int S1, int S2, int S3, int E);

	Mux(int S0, int S1, int S2, int S3, int S4, int E);

	void initializePins() const;

	void open(int channel) const;

	void enable() const;
	
	void disable() const;

};

#endif