package Device::Firmdata::Role::Session;

use Moose::Role;

has host => ( is => 'ro', isa => 'Device::Firmdata', required => 1, weak_ref => 1 );

requires 'sessionOpen';
requires 'sessionClose';
requires 'inputData';

sub subscribe {
	my ($self, $pin, $channel, $interval, $offset) = @_; 

	$offset = 0 unless defined $offset; 
	die "must specify interval" unless defined $interval;
	die "must specify a channel" unless defined $channel;
	die "must specify a pin" unless defined $pin; 	
	
	$self->host->sendCommand('SUBSCRIBE', pack('CCS<S<', $pin, $channel, $interval, $offset));
}

#TODO: update to support sending data to channels and having the channels map to output pins
sub publish {
	my ($self, $pin, $channel) = @_; 
	
	if (($pin != 1 && $pin != 2) || ($channel != 1 && $channel != 2)) {
		die "only pin and channel 1 and 2 is supported for publish because this is a hack";
	}
	
	if ($pin != $channel) {
		die "pin and channel must match on publish because this is a hack";
	}
}

sub outputData {
	my ($self, $channel, $value) = @_; 
	
	if ($channel != 1 && $channel != 2) {
		return; 
	}
		
	$self->host->sendCommand('SERVO', pack('S<C', $value, $channel));
}

1;