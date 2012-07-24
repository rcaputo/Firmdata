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
	  left-adjusted mode.
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
	  	  
CLIENT

A Firmdata client must implement the message protocol and properly digest a number of asynchronous events
that will be delivered to the client for the duration of the session. These events include the data that
the client has subscribed to as well as session control information. A session with a Firmdata device
flows like this:

	* The client connects to the Firmdata communication channel which is currently the Arduino serial
  	  port at 57,600 bps. 
	* The client waits until a broadcast event is received.
	* The client sends a session open command and waits for a session open event.
	* The client empties any receive buffers that contain data from the Firmdata device and begins to send
	  a heart beat. 
	* The client issues subscription and publish commands to configure the data that will be sent and received
      on specific message channels during the session. 
    * The client issues a run command indicating the start of capture and output (not implemented yet)
    * The client begins receiving subscribed data and transmitting published data as well as responding to 
      session events for the duration of the session.
    * The client optionally sends a session end command or just stops communication with the Firmdata device. 
    
Messages

A message contains a 1 byte header which itself contains the message channel and the message length. 
There are 32 distinct channels available with a maximum message length of 7 bytes; a zero size message is still
a valid message. The message header stores the channel value in the upper 5 bits and the message length 
in the lower 3. 

A client starts receiving a message by reading a single byte from the communication device. The size of the
message is retrieved from the message header and the message contents are read from the device if the length
is positive. The message value is both the message channel id and the content which is then provided to the 
consumer of a message. 
 
Channels

Firmdata provides channels 1-30 for data transmission and reception and reserves channel 0 and 31 for system use. 
Channel 31 is used to send commands to Firmdata and to receive session events while channel 0 is used to receive
standard out, standard error, and other standard io data from Firmdata. A message on the data channel will always
include an 8 bit timestamp as the first byte of content followed by the data values. 

A client must receive a message on channel 0 but is free to ignore it. Standard io data is only transmitted outside of
an open session during normal operation but can be used for debugging when hacking on the firmware itself. The first byte
of a message to channel 0 is the fileno and the second byte is the character that has been received. For reference
stdout is filno 1 and stderr is fileno 2. 

Encapsulated stdout and stderr characters will display properly on most terminals even though there is 3 bytes of
data for each character sent. The fileno byte value is itself either 2 or 1 depending on stderr or stdout being 
used; stdout maps to ASCII "start of heading" while stderr maps to "start of text." The header for a 2 byte message 
on channel 0 is itself the value of 2 which is ASCII "start of text." Both of those ASCII codes are ignored by
most terminals. 
   
Session

As described above a client must initiate and maintain a session with Firmdata by sending commands and receiving 
session events. A session event is identified by a unique number and includes up to 6 bytes of returned data. Each
command sent includes up to 6 bytes of command arguments and has a corresponding session event that acts as a 
command confirmation. The session event and the command numbers are identical though there are additional session
events that will occur with out any commands being sent such as the clock and processor overflow events. 

When a session is open Firmdata will send a stream of session events to channel 31 interleaved with data on channels
1 to 30. The client must process session events and data in order so it can properly update the internal state to match
that of the hardware that Firmdata is running on. Failure to process messages in order will result in incorrect command
responses being identified, invalid absolute time stamps being generated, and other undefined behavior. 
   
The first step to opening a session is to connect to the Firmdata device using the physical communication channel; currently
this is the AVR UART attached to an RS-232 interface running at 57,600 bps. Once the connection is open the client immediately
begins to receive messages and searches through them for a beacon event. A beacon is generated once a second that includes
a plain text identification sent to stdout (which the client may ignore or capture as desired) followed by the beacon event. 

For instance, the sequence of messages for a full beacon would be something like this:

 Channel | Byte 1 | Byte 2
+--------+--------+--------+
|    0   |    1   |   'F'  | stdout character message
|    0   |    1   |   'i'  | stdout character message  
|    0   |    1   |   'r'  | stdout character message 
|    0   |    1   |   'm'  | stdout character message    
|    0   |    1   |   'd'  | stdout character message    
|    0   |    1   |   'a'  | stdout character message    
|    0   |    1   |   't'  | stdout character message   
|    0   |    1   |   'a'  | stdout character message    
|    0   |    1   |   ' '  | stdout character message 
|   31   |	  2   |	   -   | beacon event message - no second byte
+--------+--------+--------+

Once the client has received the beacon event it may send a session open command. The client must ignore all messages from Firmdata until it receives 
the session open event. The session open event indicates the starting point for state synchronization between Firmdata and the client. Any messages received
prior to the session open event is old buffered data; messages received after the session open event must be processed sequentially or the states are not 
guaranteed to remain consistent between the client and Firmdata which will result in undefined behavior. 

Once the session is open the client must immediately begin sending a heart beat; Failure to send a timely heart beat will result in Firmdata forcefully
ending the client session. At this point the client may begin to configure data input and output by issuing subscribe and publish commands.  A subscribe 
command includes the pin to receive data from, the channel to send the data on, a mode of operation for the pin (ADC, GPIO, etc), an interval to sample at, 
and a phase offset for that interval. A publish command includes the pin to output data to, the channel to receive the data on, and the mode of operation of 
the output (PWM, GPIO, etc).

After the session is configured via the publish and subscribe commands the client may send a run command. A running session can not be further confiured and 
will immediately begin sending data to the client according to the configured subscriptions and send data to the IO pins according to the configured 
publications. In addition to the data messages the client must also process a number of session events to maintain synchronization with Firmdata. 

Session Events

The session event messages are sent to the client as messages on channel 31 interleaved with the messages carying the subscribed data. Using the session 
events to update the internal state of the client is required for the client to reconstruct the absolute time of a received piece of data using the 
time stamp included with the data message. A session event can be sent to the client either as a response to a command or because of some condition happening
inside Firmdata. Some events are sent only because of internal changes or as command responses but some are sent as both. Only events acting as a 
command response alone include content with the event; the client should implement the required behavior upon reception of the events regardless of them being 
generated as the response to a command or not. 

The following is a table of session event ids, event name, if the event is sent only as a command response, and what actions the client must perform 
when receiving that event:

ID	Name       					Response Only	Required action
0	BEACON						no				send session open command
												empty any receive buffers
												ignore all messages until session open event
									
1	CLOCK_OVERFLOW				no				update clock overflow accumulator
	
2	PROCESSOR_OVERFLOW			no				update processor overflow accumulator
	
3	CLOSE						no				stop publishing data
												destroy client session
												disconnect or open a new session
												
4	OPEN						yes				configure subscriptions and publications

5	RUN							yes				receive messages on channels 1-30 as configured
												process session events on channel 31
												
6	SUBSCRIBE					yes				none

7	PUBLISH						yes				none

8	NOP							yes				none

9	TEST						yes				none

10	ECHO						yes				none
												
											
Commands

A command is sent to Firmdata as a message on channel 31. The first byte of the message is the command number and each command takes between 0 and 6
bytes of data as an argument and returns between 0 and 6 bytes of data in the response. Each command includes a corresponding session event with the 
same number that acts as a command confirmation. The following table lists the command names, arguments, and returned data; see the session event 
table to get the command numbers for the command name.

Name			Arguments					Returns			Description
OPEN			none						none			Initiate a session with Firmdata

CLOSE			none						none			Destroy a session with Firmdata

RUN				none						none		

SUBSCRIBE		uint8 	input pin			none			Configure session to send data messages to client
				uint8 	output channel
				uint16 	sample interval
				uint16 	phase offset
				
PUBLISH			uint8 	output pin			none			Configure session to receive data messages from client 
				uint8	input channel
				
NOP				none						none			no-operation performed

ECHO			uint8	byte				uint8	byte	Returns argument to client unmodified 

TEST			uint8	number1				uint16	sum		Sums all 6 unsigned 8 bit integers specified
				uint8	number2								in the arguments and returns the value as a 
				uint8	number3								16 bit unsigned value
				uint8	number4
				uint8	number5
				uint8	number6							
											
HEARTBEAT		none						none			Client must send periodic heart beat or the session will be
															forcefully terminated 


	  