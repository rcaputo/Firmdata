Firmdata - Send and receive data using the IO ports on an AVR Atmega328p based Arduino Duemilanove
========

Firmdata is a firmware load for the Arduino ATmega328p Duemilanove that allows a client to connect to the Arduino
then use it to send and receive data from the AVR IO ports such as the ADC input and PWM output. This
provides functionality similar to the Firmata software for the Arduino platform but with the following
major differences:

	* Data sent to a Firmdata client includes enough information to calculate the absolute time
	  the data was collected with a precision down to 16us regardless of communication line latency. 
	* All IO with Firmdata is done using a simple message format providing 30 user configurable
	  data channels for input and output. The message header is a single byte and each 
	  message can include up to 7 bytes of content. 
	* Each of the 6 ADC pins can be sampled at an arbitrary rate configured in the session. Sampling 
	* The protocol does not deal with connected device types. For instance When controlling a
	  servo using PWM it is up to the client to send the time data for the pulse width that will control
	  the servo position and not the explicit desired servo angle. 
	* The processor consumption on the microcontroller can be calculated from the session events. 
	* The Arduino development kit is not used. The firmware is written in C99, cross compiled with GCC,
	  and linked against GNU avr-libc.

STATUS

The following features are operational:

	* Device identification and presence broadcast.
	
	* Session creation, termination, and expiration.
	
	* Multiple data subscriptions at arbitrary intervals on arbitrary ADC pins.
	
	* Clock events and time data with samples giving a precision down to 16us regardless
	  of latency on the serial line.
	  
	* Reference client written in Perl that tracks serial speed and microcontroller processor usage and
	  is broken up into a device class, a message driver class, and a session class. 

The following problems are known to exist:

	* The client interaction and sessions do not quite work as documented here; see the reference client
	
	* The reference client isn't exactly elegant right now

	* ADC sample jitter can be up to 10ms at times but is usually under 16us on first data subscription.
	  The later data subscriptions are much worse. 
	  
	* It looks like timers are being skipped. This is a huge contribution to but does not seem to be the 
	  only cause of the jitter issues. 
	  
	* Clock overflow events might be getting dropped or might not be coming in order at times because of 
	  interrupt overlap with the timers. 
	  
Even with the known issues Firmdata is able to provide the data to calculate the speed of a shaft spinning at 
1300 RPM when it has 3 magnets attached to it and field strength readings coming from a linear fluxgate 
magnetometer sampled at about 800 hz. As well it is easy to measure the jitter and test changes to see the impact. 
	  
The following features remain to be implemented:

	* Timer1 configured for 16 bit PWM for both output ports it is connected to running at a precision
	  of .5us as well as a command for mapping an input message channel to the timer compare registers. 
	* Optional 8 bit CRC byte with each message.
	* Allow client to configure the session
	 	* clock precision and minimum timer interval
	 	* generation of processor overflow events
	* Allow the client to configure
		* Sample precision of a pin such as 8 or 10 bits on the ADC
		* Precision of a data channel - 8, 16, 24, 32 bits etc
	* Synchronization between client clock and AVR clock
	* Allow a default session configuration to be stored in the AVR with fail safe values configured
	  for the output pins when a session is not open. The session configuration could be run with out
	  having the client issue any subscribe or publish commands. 
	  	  
