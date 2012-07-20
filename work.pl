package Device::Firmdata::Serial; 

use strict;
use warnings; 

use constant HEADER_CHANNEL_MASK => 248;
use constant HEADER_CHANNEL_SHIFT => 3; 
use constant HEADER_SIZE_MASK => 7; 

sub new {
	my ($class, $path) = @_; 
	my $self = bless({}, $class); 
	my $fh;
	
	open($fh, "+<", '/dev/ttyS2') or die "could not open $path for read/write: $!";
	
	$self->{fh} = $fh; 
	
	return $self; 
}

sub read {
	my ($self, $bytes) = @_; 
	my $buf;
	
	my $bytesRead = read($self->{fh}, $buf, $bytes);
	
	if ($bytesRead < 0) {
		die "Could not read: $!";
	} elsif ($bytesRead != $bytes) {
		die "read of $bytes requested but only read $bytesRead";
	}
	
	return $buf; 
}

sub write {
	my ($self, $content) = @_; 
	my $length = length($content); 
	my $written; 
	
	$written = syswrite($self->{fh}, $content, $length);
	
	if ($written != $length) {
		die "tried to write $length bytes but only wrote $written: $!"; 
	}
	
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
	
	return ($channel, $content, $length); 
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

BEGIN {
	my %numbers; 
	our %COMMAND_NAMES; 
	
	our %COMMAND_NUMBERS = (
		NOP => 1, ECHO => 2, IDENTIFY => 3, TEST => 4, 
		SESSION_START => 10, SESSION_END => 11,
	);
	

	while(my ($name, $number) = each(%COMMAND_NUMBERS)) {
		$COMMAND_NAMES{$number} = $name; 
	}
}

sub new {
	my ($class, $messageDriver) = @_; 
	my $self = bless({}, $class);
	
	$self->{driver} = $messageDriver; 
	$self->{clockAccumulator} = 0; 
	$self->{session} = undef; 
	$self->{sentCommand} = undef; 
	$self->{gotCommandResponse} = 0; 
	$self->{commandResponseArgs} = undef; 
	
	return $self; 
}

sub run {
	my ($self) = @_;
	
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
		$self->handle_data($when, substr($content, 1));
	}	
}

sub sendCommand {
	my ($self, $commandName, $args) = @_; 
	my $commandNumber = $self->get_command_number($commandName);
	my $message = pack('C', $commandNumber);
	my $retArgs; 
	
	if (defined($self->{sentCommand})) {
		die "attempt to send a command while waiting for the response from a command"; 
	}
	
	$self->{sentCommand} = $commandNumber; 
	$self->{gotCommandResponse} = 0; 
	$self->{commandResponseArgs} = undef; 
	
	if (defined($args)) {
		$message .= $args;
	}
	
	$self->driver->sendMessage(31, $message);
	
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

sub handle_stdio {
	my ($self, $fdno, $byte) = @_; 	
}

sub handle_system_message_commandResponse {
	my ($self, $content) = @_; 
	my $lastSentCommand = $self->{sentCommand}; 
	my $commandNumber = unpack('C', $content); 
	my $retArgs;
	
	if (! defined($lastSentCommand)) {
		die "received a response to a command but there is no record of an outstanding command"; 
	} elsif ($lastSentCommand != $commandNumber) {
		die "the last sent command was $lastSentCommand but the response is to command $commandNumber";
	}
	
	print "command response $commandNumber ", length(substr($content, 1)), "\n";
	
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

	die "ready to have a session object made";
}

sub handle_system_message_clockOverflow {
	my ($self) = @_;
	$self->{clockAccumulator}++; 
	
	if ($self->{clockAccumulator} >= 10000) {
		print "Stopping session\n";
		$self->sendCommand('SESSION_END'); 
	}
}

sub handle_system_message {
	my ($self, $type, $content) = @_; 
			
	if ($type == 2) {
		$self->handle_system_message_beacon; 
	} elsif ($type == 0) {
		$self->handle_system_message_clockOverflow; 
	} elsif ($type == 1) {
		$self->handle_system_message_commandResponse($content); 
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

package main; 

#my $buf; 
#
#print $serial pack('CCCCCCC', 4, 80, 2, 9, 4, 5, 9) or die "could not write";
#
#$| = 1;
#print ''; 
#
#read($serial, $buf, 4) == 4 or die "could not read";
#my ($messageHeader, $command, $resp) = unpack('CCS<', $buf);
#my $messageChannel = ($messageHeader & 248) >> 3; 
#my $messageSize = $messageHeader & 7; 
#
#print "$messageHeader $messageChannel $messageSize $command $resp\n";
#

use strict;
use warnings;

$| = 1;
print ''; 

my $firmdataDriver = Device::Firmdata::Serial->new(shift(@ARGV)); 
my $firmdata = Device::Firmdata->new($firmdataDriver); 

while(1) {
	$firmdata->poll(); 
}



































