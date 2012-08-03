package Device::Firmdata::Role::IO::Win32Serial;

use Moose::Role;
use Win32::SerialPort; 

with 'Device::Firmdata::Role::IO';

has driver => ( is => 'ro', isa => 'Win32::SerialPort', required => 1, builder => 'build_driver', lazy => 1 );

sub build_driver {
	my ($self) = @_; 
	my $serial = Win32::SerialPort->new($self->portName);
	
	$serial->baudrate(57600) or die "Could not set baud: $^E";
	$serial->parity('none') or die "Could not set parity: $^E";
	$serial->stopbits(1) or die "Could not set stopbits: $^E";
	$serial->read_interval(0);
	
	return $serial; 
}

sub read {
	my ($self, $bytes) = @_; 
	
	my ($bytesRead, $buf) = $self->driver->read($bytes);
	
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
	
	$written = $self->driver->write($content);
	
	if ($written != $length) {
		die "tried to write $length bytes but only wrote $written: $!"; 
	}
		
	return; 
}

1;