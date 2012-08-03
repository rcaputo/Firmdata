package Device::Firmdata::Role::Session;

use Moose::Role;

has host => ( is => 'ro', isa => 'Device::Firmdata', required => 1, weak_ref => 1 );

requires 'sessionOpen';
requires 'sessionClose';
requires 'data';

sub subscribe {
	my ($self, $pin, $channel, $interval, $offset) = @_; 

	$offset = 0 unless defined $offset; 
	die "must specify interval" unless defined $interval;
	die "must specify a channel" unless defined $channel;
	die "must specify a pin" unless defined $pin; 	
	
	$self->host->sendCommand('SUBSCRIBE', pack('CCS<S<', $pin, $channel, $interval, $offset));
}

1;