package Device::Firmdata::Session; 

use Moose;

with 'Device::Firmdata::Role::Session';

sub sessionOpen {
	my ($self) = @_;
	
	$self->outputData(1, 1500);
	$self->outputData(2, 1500);
	
	$self->subscribe(1, 1, 2000, 0);
}

sub sessionClose {
	my ($self) = @_;
	
}

sub inputData {
	my ($self, $channel, $when, $data) = @_; 
	
	warn "Got data on $channel at $when";
}

1;