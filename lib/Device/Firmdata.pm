package Device::Firmdata; 

use Moose; 
use Time::HiRes qw(gettimeofday); 

use Device::Firmdata::Util::Accumulator; 
use Device::Firmdata::Session; 

#TODO: refactor the command code into it's own class instead of 
#hammering the code into the hashref that backs the instance

has config => ( is => 'ro', isa => 'HashRef', required => 1 );
has io => ( is => 'ro', does => 'Device::Firmdata::Role::IO', required => 1, builder => 'build_io', lazy => 1 ); 
has session => ( is => 'rw', does => 'Device::Firmdata::Role::Session' );
has clockCounterOverflow => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new } );
has processorCounterOverflow => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new } );
has lastHeartBeat => ( is => 'rw', isa => 'Num', required => 1, default => 0 );
has sessionStartWait => ( is => 'rw', isa => 'Bool', required => 1, default => 0 ); 

#The clock counter is incrimented on the microcontroller at this interval and this is the minimum amount of time that can
#be represented
use constant CLOCK_TICK_US => 64; 
#This is the frequency that the internal timers run at - it is only possible to sample data from the analog to digital converter
#once per any timer tick but it is possible to sample the ADC on every single timer tick. 
use constant TIMER_HZ => 2048; 

BEGIN {
	our %COMMAND_NAMES; 
	our %COMMAND_NUMBERS = (
		NOP => 1, ECHO => 2, IDENTIFY => 3, TEST => 4, 
		SESSION_START => 10, SESSION_END => 11, HEARTBEAT => 12,
		SUBSCRIBE => 13, 
		
		#TODO: this is a hack here for now until mapping is done between message channels and outputs
		SERVO => 14
	);
	
	while(my ($name, $number) = each(%COMMAND_NUMBERS)) {
		$COMMAND_NAMES{$number} = $name; 
	}
}

#constructs an anonymous class from the IO roles - the anonymous
#class implements message parsing in a base role and physical message 
#transfers in subroles 
sub build_io {
	my ($self) = @_;
	my $portName = $self->config->{portName};
	my $ioRole; 
	
	if ($^O eq 'MSWin32') {
		$ioRole = 'Device::Firmdata::Role::IO::Win32Serial';
	} else {
		$ioRole = 'Device::Firmdata::Role::IO::Serial';
	}
	
	unless(defined($portName)) {
		die "Must specify portName as command line argument";
	}
	
	my $metaclass = Moose::Meta::Class->create_anon_class(
		roles => [ $ioRole ], cache => 1
	); 
	
	return $metaclass->new_object(portName => $portName);
}

#create a default session or load a session from a file and
#use an instance of that session
sub build_session {
	my ($self) = @_;
	my $sessionFile = $self->config->{sessionFile};
	my $sessionClass;
	
	if (defined($sessionFile)) {
		$sessionClass = require $sessionFile;
	} else {
		$sessionClass = 'Device::Firmdata::Session';
	}
		
	return $sessionClass->new(host => $self);	
}

#the API user calls this function to enter the run loop and begin
#interacting with a Firmdata device. Control is handed from the 
#caller of this function to Firmdata and does not return. The API
#user will interface with Firmdata and the connected hardware via
#an instance of a custom Device::Firmdata::Session object
sub run {
	my ($self) = @_;
		
	while(1) {
		$self->poll; 
	}
}

#blocks on reading a full message from the Firmdata device and once
#a message is received it is dispatched to the appropriate handler
sub poll {
	my ($self) = @_; 
	my ($channel, $content) = $self->io->getMessage; 

	if ($channel == 31) {
		my ($type) = unpack('C', $content); 
		#session control information
		$self->handleSystemMessage($type, substr($content, 1)); 
	} elsif ($channel == 0) {
		my ($fdno) = unpack('C', $content); 
		my $byte = substr($content, 1, 1); 
		#encapsalated stdio data
		$self->handleStdio($fdno, $byte); 
	} else {
		my ($when) = unpack('C', $content); 
		#data generated by subscriptions 
		$self->handleData($channel, $when, substr($content, 1));
	}	
	
}

#sends a command to Firmdata and waits for a response then returns to the caller.
#the returned value is either undef or the content of the result of the command
sub sendCommand {
	my ($self, $commandName, $args) = @_; 
	my $commandNumber = $self->getCommandNumber($commandName);
	my $message = pack('C', $commandNumber);
	my $retArgs; 
	
	#some commands return data to the caller of this function and some don't
	#the logic for handling commands gets a bit twisted because of this
	if ($commandName ne 'HEARTBEAT' && $commandName ne 'SERVO') {
		#this is a command that will generate a response - check for an outstanding command that has not yet been responded to
		if (defined($self->{sentCommand})) {
			die "attempt to send command '$commandName' while waiting for the response from command ", $self->getCommandName($self->{sentCommand}); 
		}
		
		#set state info we need to reconstruct the command that is being responded to
		$self->{sentCommand} = $commandNumber; 
		$self->{gotCommandResponse} = 0; 
		$self->{commandResponseArgs} = undef; 
	}
	
	
	if (defined($args)) {
		$message .= $args;
	}
	
	#commands to firmdata go out on message channel 31
	$self->io->sendMessage(31, $message);

	#if the command won't generate arguments there's nothing left to do		
	if ($commandName eq 'HEARTBEAT' || $commandName eq 'SERVO') {
		return; 
	}
		
	#some later event is going to include the command response information - the event handler
	#will set this flag true when the command response comes in. This will cause the main loop to execute
	#under the control of this function until the command response happens. 
	while(! $self->{gotCommandResponse}) {
		$self->poll; 
	}
	
	$retArgs = $self->{commandResponseArgs}; 
	
	#clean up our state
	$self->{commandResponseArgs} = undef; 
	$self->{sentCommand} = undef; 
	$self->{gotCommandResponse} = 0; 
	
	return $retArgs; 
}

sub getCommandName {
	my ($self, $number) = @_;
	our(%COMMAND_NAMES); 

	unless(exists($COMMAND_NAMES{$number})) {
		die "Could not look up command name for '$number'"; 
	}
	
	return $COMMAND_NAMES{$number}; 
}

sub getCommandNumber {
	my ($self, $name) = @_; 
	our(%COMMAND_NUMBERS); 
	
	unless(exists($COMMAND_NUMBERS{$name})) {
		die "Could not look up command number for '$name'";
	}
	
	return $COMMAND_NUMBERS{$name}; 
}

#dispatch out the different type of system events to their various handlers 
sub handleSystemMessage {
	my ($self, $type, $content) = @_; 
			
	if ($type == 2) {
		#beacons occur until a session is running 
		$self->handleSystemMessage_beacon; 
	} elsif ($type == 0) {
		#every time the 8bit counter overflows this event comes in
		$self->handleSystemMessage_clockCounterOverflow; 
	} elsif ($type == 1) {
		$self->handleSystemMessage_commandResponse($content); 
	} elsif ($type == 3) {
		#every time the processor usage counter overflows this event is delivered
		$self->handleSystemMessage_processorCounterOverflow; 
	} else {
		#this is a fatal error because now the session between Firmdata and the client 
		#can't be guranteed to be in sync anymore
		die "Unknown system message type: $type";
	}
}

#once a beacon is received send a command to start a session then create
#an instance of the session object
sub handleSystemMessage_beacon {
	my ($self) = @_; 

	if (defined($self->session)) {
		die "Received beacon message while the session was active"; 
	}
	
	if (! $self->sessionStartWait) {
		$self->sessionStartWait(1);
		$self->sendCommand('SESSION_START'); 		
	}
	
	print STDERR "Session started at ", scalar(localtime()), "\n";
	
	$self->session($self->build_session);
	$self->session->sessionOpen;
}

#see the comments on sendCommand() for info on what is going on here
sub handleSystemMessage_commandResponse {
	my ($self, $content) = @_; 
	my $lastSentCommand = $self->{sentCommand}; 
	my $commandNumber = unpack('C', $content); 
	my $retArgs;
	
	if (! defined($lastSentCommand)) {
		die "received a response to command $commandNumber but there is no record of an outstanding command"; 
	} elsif ($lastSentCommand != $commandNumber) {
		die "the last sent command was $lastSentCommand but the response is to command $commandNumber";
	}
		
	if (defined($content) && length($content) > 1) {
		$retArgs = substr($content, 1); 
	}
	
	$self->{commandResponseArgs} = $retArgs; 
	
	$self->{gotCommandResponse} = 1; 
	
}

#tracks the number of clock overflow events so that the client can reconstruct
#time information from the session 
sub handleSystemMessage_clockCounterOverflow {
	my ($self) = @_;
	my $when; 
	
	$self->clockCounterOverflow->add(1);
	
	$when = $self->calculateClockOverflowTime; 
	
	#this just seemed like a good spot for the heartbeat to go	
	if ($when - $self->lastHeartBeat > .1) {
		$self->lastHeartBeat($when);		
		$self->sendCommand("HEARTBEAT") if defined $self->session;
	}
}

#the value of this counter and the value of the clock counter can be
#used to calculate processor consumption on the microcontroller
sub handleSystemMessage_processorCounterOverflow {
	my ($self) = @_;
	
	$self->processorCounterOverflow->add(1);
}

#stderr and stdout on the microcontroller and sent to the Firmdata client by 
#encapsalating the content in messages to channel 0 - this callback is invoked 
#every time a single byte of stdio data is ready. 
sub handleStdio {
	my ($self, $fileno, $byte) = @_; 
}

#data is sent to the client as a message to a configured channel number
#and includes the time the value was read as well as the actual value read. This 
#callback is invoked every time data is ready 
sub handleData {
	my ($self, $channel, $clockValue, $content) = @_;
	#find the duration that the session has been running, this is in floating point seconds 
	my $baseTime = $self->calculateClockOverflowTime;
	#convert the clock counter value into floating point seconds 
	my $clockTime = $clockValue * CLOCK_TICK_US / 1000000;
		
	#if a session is running deliver the data to it	
	if (defined($self->session)) {
		$self->session->inputData($channel, $baseTime + $clockTime, unpack('C', $content));
	}
}

#computes the absolute time of the last clock counter overflow using the start of the session as 0 seconds
#returns the time as a floating point value in seconds 
sub calculateClockOverflowTime {
	my ($self) = @_;
	
	return $self->clockCounterOverflow->value * 256 * CLOCK_TICK_US / 1000000; 
}
1; 