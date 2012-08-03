package Device::Firmdata::Session; 

use Moose;

with 'Device::Firmdata::Role::Session';

sub sessionOpen {
	my ($self) = @_;
	
	$self->subscribe(1, 1, 2, 0);
}

sub sessionClose {
	my ($self) = @_;
	
}

sub data {
	my ($self, $channel, $when, $data) = @_; 
	
	warn "Got data on $channel at $when";
}

1;