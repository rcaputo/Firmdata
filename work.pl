package Device::Firmdata::Serial; 

use strict;
use warnings; 

use constant HEADER_CHANNEL_MASK => 248;
use constant HEADER_CHANNEL_SHIFT => 3; 
use constant HEADER_SIZE_MASK => 7; 

use Win32::SerialPort; 

sub new {
	my ($class, $port) = @_; 
	my $self = bless({}, $class); 
	my $fh;

		
	$self->{serial} = Win32::SerialPort->new($port);
	$self->{fh} = $fh; 
	$self->{accumulators} = { send => 0, recv => 0 }; 
	
	$self->serial->baudrate(57600);
	$self->serial->parity('none');
	$self->serial->stopbits(1);
	$self->serial->read_interval(0);
	
	return $self; 
}

sub serial {
	return $_[0]->{serial};
}

sub get_accumulators {
	my ($self) = @_;
	my $send = $self->{accumulators}->{send};
	my $recv = $self->{accumulators}->{recv}; 
		
	$self->{accumulators} = { send => 0, recv => 0 };	
	
	return ($send, $recv); 
}

sub read {
	my ($self, $bytes) = @_; 
	
	my ($bytesRead, $buf) = $self->serial->read($bytes);
	
	if ($bytesRead < 0) {
		die "Could not read: $!";
	} elsif ($bytesRead != $bytes) {
		die "read of $bytes requested but only read $bytesRead";
	}
	
	$self->{accumulators}->{recv} += $bytes; 
	
	return $buf; 
}

sub write {
	my ($self, $content) = @_; 
	my $length = length($content); 
	my $written; 
	
	$written = $self->serial->write($content);
	
	if ($written != $length) {
		die "tried to write $length bytes but only wrote $written: $!"; 
	}
	
	$self->{accumulators}->{send} += $written; 
	
	return; 
}

sub getHeader {
	my ($self) = @_;
	my $headerByte = $self->read(1); 	
	my $headerValue = unpack('C', $headerByte); 
	my $channel = ($headerValue & HEADER_CHANNEL_MASK) >> HEADER_CHANNEL_SHIFT; 
	my $length = $headerValue & HEADER_SIZE_MASK; 
	
	return ($channel, $length); 
}

sub getMessage {
	my ($self) = @_;
	my ($channel, $length) = $self->getHeader; 
	my $content = $self->read($length); 
	
	return ($channel, $content, $length + 1); 
}

sub sendHeader {
	my ($self, $channel, $size) = @_;
	my $header; 
	
	$header = $channel << HEADER_CHANNEL_SHIFT; 
	$header |= $size | HEADER_SIZE_MASK; 
	
	$self->write($header); 
}

sub sendMessage {
	my ($self, $channel, $content) = @_; 
	
	#TODO - update command.c and message.c to support commands embeded in messages
	#$self->sendHeader($channel, length($content)); 
	$self->write($content); 	
}

package Device::Firmdata;

use strict;
use warnings; 

use Time::HiRes qw(gettimeofday); 

$SIG{ALRM} = sub { our($US); update_status($US) if defined $US; };

use constant CLOCK_TICK_NS => 16; 

BEGIN {
	my %numbers; 
	our %COMMAND_NAMES; 
	
	our %COMMAND_NUMBERS = (
		NOP => 1, ECHO => 2, IDENTIFY => 3, TEST => 4, 
		SESSION_START => 10, SESSION_END => 11, HEARTBEAT => 12,
		SUBSCRIBE => 13,
	);
	
	while(my ($name, $number) = each(%COMMAND_NUMBERS)) {
		$COMMAND_NAMES{$number} = $name; 
	}
}

sub new {
	my ($class, $messageDriver, $sessionClassName) = @_; 
	my $self = bless({}, $class);
	our $US; 
	
	$self->{driver} = $messageDriver; 
	$self->{session} = undef; 
	$self->{sentCommand} = undef; 
	$self->{gotCommandResponse} = 0; 
	$self->{commandResponseArgs} = undef; 
	$self->{processorCounter} = 0; 
	$self->{clockCounter} = 0; 
	$self->{firstClockOverflowTimestamp} = undef; 
	$self->{sessionClassName} = $sessionClassName;
		
	$self->{status} = {
		lastProcessorCounter => 0,
		lastClockCounter => 0,
	};
		
	$US = $self; 
	
	$| = 1;
	print '';
	
	return $self; 
}

sub update_status {
	my ($self) = @_; 
		
	alarm(1); 
			
	$self->sendCommand("HEARTBEAT") if defined $self->{session};
	$self->print_status;
}

sub print_status {
	my ($self) = @_; 
	our ($lastHere); 
	my $clockTicks = $self->{clockCounter} - $self->{status}->{lastClockCounter};
	my $processorTicks = $self->{processorCounter} - $self->{status}->{lastProcessorCounter};
	my ($utilization, $sendSpeed, $recvSpeed);
	my $now = gettimeofday();
		
	if (defined($self->{wallTime}) && defined($self->{captureTime})) {
		print STDERR sprintf("clock error:%.4f ", $self->{wallTime} - $self->{captureTime});
		
	}
	
	if (defined($lastHere)) {
		my ($send, $recv) = $self->driver->get_accumulators; 
		$sendSpeed = int($send / 1 / ($now - $lastHere)); 
		$recvSpeed = int($recv / 1 / ($now - $lastHere)); 
	}
	
	$lastHere = $now; 
	
	if ($clockTicks) {
		$utilization = int($processorTicks / $clockTicks * 100);
	}
	
	print STDERR "uCPU:$utilization% " if defined $utilization; 
	print STDERR "s:$sendSpeed r:$recvSpeed " if defined $sendSpeed; 
	print STDERR $self->session->update if defined $self->session; 
	print STDERR "   \r";
}

sub run {
	my ($self) = @_;
	
	$self->update_status();
	
	while(1) {
		$self->poll; 
	}
}

sub poll {
	my ($self) = @_; 
	my ($channel, $content, $size) = $self->driver->getMessage; 

	if ($channel == 31) {
		my ($type) = unpack('C', $content); 
		$self->handle_system_message($type, substr($content, 1)); 
	} elsif ($channel == 0) {
		my ($fdno) = unpack('C', $content); 
		my $byte = substr($content, 1, 1); 
		$self->handle_stdio($fdno, $byte); 
	} else {
		my ($when) = unpack('C', $content); 
		$self->handle_data($channel, $when, substr($content, 1));
	}	
}

sub sendCommand {
	my ($self, $commandName, $args) = @_; 
	my $commandNumber = $self->get_command_number($commandName);
	my $message = pack('C', $commandNumber);
	my $retArgs; 
	
	if ($commandName ne 'HEARTBEAT') {
		if (defined($self->{sentCommand})) {
			die "attempt to send command '$commandName' while waiting for the response from command ", $self->get_command_name($self->{sentCommand}); 
		}
		
		$self->{sentCommand} = $commandNumber; 
		$self->{gotCommandResponse} = 0; 
		$self->{commandResponseArgs} = undef; 
	}
	
	
	if (defined($args)) {
		$message .= $args;
	}
	
	$self->driver->sendMessage(31, $message);
	
	if ($commandName eq 'HEARTBEAT') {
		return; 
	}
		
	while(! $self->{gotCommandResponse}) {
		$self->poll; 
	}
	
	$retArgs = $self->{commandResponseArgs}; 
	
	$self->{commandResponseArgs} = undef; 
	$self->{sentCommand} = undef; 
	$self->{gotCommandResponse} = 0; 
	
	return $retArgs; 
}

sub driver {
	return $_[0]->{driver}; 
}

sub session {
	return $_[0]->{session}; 
}

sub handle_stdio {
	my ($self, $fdno, $byte) = @_; 	
	
	if ($fdno == 2) {
		print STDERR $byte; 
	}
}

sub handle_data {
	my ($self, $channel, $relativeTime, $content) = @_; 
	my $time = $self->{clockCounter} * 256 + $relativeTime;
	my $timeSeconds = $time * CLOCK_TICK_NS / 1000000;
		
	$self->session->data($channel, $timeSeconds, unpack('C', $content));
}

sub handle_system_message_commandResponse {
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

sub handle_system_message_beacon {
	my ($self) = @_; 

	if (defined($self->{session})) {
		die "Received beacon message while the session was active"; 
	}
	
	$self->sendCommand('SESSION_START'); 
	print STDERR "Session started at ", scalar(localtime()), "\n";
	
	$self->{session} = $self->{sessionClassName}->new($self);
	$self->{session}->init; 
}

sub handle_system_message_clockOverflow {
	my ($self) = @_;
	my $now = gettimeofday(); 

	$self->{clockCounter}++; 
		
	if (! defined($self->{firstClockOverflowTimestamp})) {
		$self->{firstClockOverflowTimestamp} = $now; 
	} else {
		my $sessionDurationWall = $now - $self->{firstClockOverflowTimestamp};
		my $sessionDurationCapture = $self->{clockCounter} * 256 * CLOCK_TICK_NS / 1000000; 
		
		$self->{wallTime} = $sessionDurationWall;
		$self->{captureTime} = $sessionDurationCapture; 
	}
	
	$self->session->clock_overflow if defined $self->{session};
}

sub handle_system_message_processorCounterOverflow {
	my ($self) = @_; 

	$self->{processorCounter}++; 
}

sub handle_system_message {
	my ($self, $type, $content) = @_; 
			
	if ($type == 2) {
		$self->handle_system_message_beacon; 
	} elsif ($type == 0) {
		$self->handle_system_message_clockOverflow; 
	} elsif ($type == 1) {
		$self->handle_system_message_commandResponse($content); 
	} elsif ($type == 3) {
		$self->handle_system_message_processorCounterOverflow; 
	} else {
		die "Unknown system message type: $type";
	}
}

sub get_command_name {
	my ($self, $number) = @_;
	our(%COMMAND_NAMES); 

	unless(exists($COMMAND_NAMES{$number})) {
		die "Could not look up command name for '$number'"
	}
	
	return $COMMAND_NAMES{$number}; 
}

sub get_command_number {
	my ($self, $name) = @_; 
	our(%COMMAND_NUMBERS); 
	
	unless(exists($COMMAND_NUMBERS{$name})) {
		die "Could not look up command number for '$name'";
	}
	
	return $COMMAND_NUMBERS{$name}; 
}

package Device::Firmdata::Session;

use strict;
use warnings; 
use Scalar::Util qw(weaken); 

sub new {
	my ($class, $firmdata) = @_; 
	my $self = bless({}, $class);
	
	$self->{firmdata} = $firmdata; 
	weaken($self->{firmdata}); 
				
	return $self; 
}

sub init {
	my ($self) = @_;
}

sub subscribe {
	my ($self, $pin, $channel, $interval, $offset) = @_; 

	$offset = 0 unless defined $offset; 
	die "must specify interval" unless defined $interval;
	die "must specify a channel" unless defined $channel;
	die "must specify a pin" unless defined $pin; 	
	
	$self->{firmdata}->sendCommand('SUBSCRIBE', pack('CCS<S<', $pin, $channel, $interval, $offset));
}

sub update {
	my ($self) = @_; 

	return '';
}

sub data {
	my ($self, $channel, $when, $content) = @_;
	
	return; 
}

sub clock_overflow {
	my ($self) = @_;
}

package main; 

use strict;
use warnings;

$| = 1;
print ''; 

my $serialPortName = shift(@ARGV) or die "must specify a serial port name";
my $sessionClassFile = shift(@ARGV) or die "must specify a session class file path";

my $sessionClass = require $sessionClassFile;  

my $firmdataDriver = Device::Firmdata::Serial->new($serialPortName); 
my $firmdata = Device::Firmdata->new($firmdataDriver, $sessionClass); 

$firmdata->run;





























