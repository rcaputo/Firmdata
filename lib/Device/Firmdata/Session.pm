package Device::Firmdata::Session; 

use Moose;

with 'Device::Firmdata::Role::Session';

#invoked when the session has been established with the Firmdata host
#perform any initialization here
sub sessionOpen {
	my ($self) = @_;
	
	#receive data from pin 1 on channel 1 with an interval of 2000 timer ticks
	#args: pin number, channel number, interval in timer ticks, timer phase offset in timer ticks
	$self->subscribe(1, 1, 2000, 0);
	
	#connect pin 1 to messages from channel 1
	#args: pin, channel
	$self->publish(1, 1);
	
	#set some initial output values	
	#args: pin, channel
	$self->outputData(1, 1500);
	
}

#TODO: set this up to be invoked when the session has finished either through a command
#or via host initiated session timeout
sub sessionClose {
	my ($self) = @_;
	
}

#invoked when the session has received data that has been previously subscribed too
#args: channel number, absolute time data was capture as floating point seconds since session was created, captured value
sub inputData {
	my ($self, $channel, $when, $data) = @_; 
	
	warn "Got data on $channel at $when";
}

1;