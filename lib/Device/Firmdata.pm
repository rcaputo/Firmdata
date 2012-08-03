package Device::Firmdata; 

use Moose; 
use Time::HiRes qw(gettimeofday); 

use Device::Firmdata::Util::Accumulator; 
use Device::Firmdata::Session; 

has commandLineArgs => ( is => 'ro', isa => 'HashRef', required => 1 );
has io => ( is => 'ro', does => 'Device::Firmdata::Role::IO', required => 1, builder => 'build_io', lazy => 1 ); 
has session => ( is => 'rw', does => 'Device::Firmdata::Role::Session' );
has clockCounterOverflow => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new } );
has processorCounterOverflow => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new } );
has lastHeartBeat => ( is => 'rw', isa => 'Num', required => 1, default => 0 );

use constant CLOCK_TICK_US => 64; 

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


sub build_io {
	my ($self) = @_;
	my $portName = $self->commandLineArgs->{portName};
	my $ioRole; 
	
	if ($^O eq 'MSWin32') {
		$ioRole = 'Device::Firmdata::Role::IO::Win32Serial';
	} else {
		die "$^O is not currently a supported operating system";
	}
	
	unless(defined($portName)) {
		die "Must specify portName as command line argument";
	}
	
	my $metaclass = Moose::Meta::Class->create_anon_class(
		roles => [ $ioRole ], cache => 1
	); 
	
	return $metaclass->new_object(portName => $portName);
}

sub run {
	my ($self) = @_;
		
	while(1) {
		$self->poll; 
	}
}

sub poll {
	my ($self) = @_; 
	my ($channel, $content, $size) = $self->io->getMessage; 

	if ($channel == 31) {
		my ($type) = unpack('C', $content); 
		$self->handleSystemMessage($type, substr($content, 1)); 
	} elsif ($channel == 0) {
		my ($fdno) = unpack('C', $content); 
		my $byte = substr($content, 1, 1); 
		$self->handleStdio($fdno, $byte); 
	} else {
		my ($when) = unpack('C', $content); 
		$self->handleData($channel, $when, substr($content, 1));
	}	
	
}

sub sendCommand {
	my ($self, $commandName, $args) = @_; 
	my $commandNumber = $self->getCommandNumber($commandName);
	my $message = pack('C', $commandNumber);
	my $retArgs; 
	
	if ($commandName ne 'HEARTBEAT' && $commandName ne 'SERVO') {
		if (defined($self->{sentCommand})) {
			die "attempt to send command '$commandName' while waiting for the response from command ", $self->getCommandName($self->{sentCommand}); 
		}
		
		$self->{sentCommand} = $commandNumber; 
		$self->{gotCommandResponse} = 0; 
		$self->{commandResponseArgs} = undef; 
	}
	
	
	if (defined($args)) {
		$message .= $args;
	}
	
	$self->io->sendMessage(31, $message);
	
	if ($commandName eq 'HEARTBEAT' || $commandName eq 'SERVO') {
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

sub getCommandName {
	my ($self, $number) = @_;
	our(%COMMAND_NAMES); 

	unless(exists($COMMAND_NAMES{$number})) {
		die "Could not look up command name for '$number'"
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

sub handleSystemMessage {
	my ($self, $type, $content) = @_; 
			
	if ($type == 2) {
		$self->handleSystemMessage_beacon; 
	} elsif ($type == 0) {
		$self->handleSystemMessage_clockCounterOverflow; 
	} elsif ($type == 1) {
		$self->handleSystemMessage_commandResponse($content); 
	} elsif ($type == 3) {
		$self->handleSystemMessage_processorCounterOverflow; 
	} else {
		die "Unknown system message type: $type";
	}
}

sub handleSystemMessage_beacon {
	my ($self) = @_; 

	if (defined($self->session)) {
		die "Received beacon message while the session was active"; 
	}
	
	$self->sendCommand('SESSION_START'); 
	print STDERR "Session started at ", scalar(localtime()), "\n";

	$self->session(Device::Firmdata::Session->new(host => $self));	
	
	$self->session->sessionOpen;
}

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

sub handleSystemMessage_clockCounterOverflow {
	my ($self) = @_;
	my $when; 
	
	$self->clockCounterOverflow->add(1);
	
	$when = $self->calculateClockOverflowTime; 
		
	if ($when - $self->lastHeartBeat > .1) {
		$self->lastHeartBeat($when);		
		$self->sendCommand("HEARTBEAT") if defined $self->session;
	}
}

sub handleSystemMessage_processorCounterOverflow {
	my ($self) = @_;
	
	$self->processorCounterOverflow->add(1);
}

sub handleStdio {
	
}

sub handleData {
	my ($self, $channel, $clockValue, $content) = @_;
	my $baseTime = $self->calculateClockOverflowTime;
	my $clockTime = $clockValue * CLOCK_TICK_US / 1000000;
		
	if (defined($self->session)) {
		$self->session->data($channel, $baseTime + $clockTime, unpack('C', $content));
	}
}

sub calculateClockOverflowTime {
	my ($self) = @_;
	
	return $self->clockCounterOverflow->value * 256 * CLOCK_TICK_US / 1000000; 
}
1; 