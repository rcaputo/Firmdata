package Device::Firmdata::Role::IO::DeviceSerial;

use Moose::Role;
use Device::SerialPort;
use Symbol qw(gensym);

with 'Device::Firmdata::Role::IO';

has driver => ( is => 'ro', isa => 'Device::SerialPort', required => 1, builder => 'build_driver', lazy => 1 );

sub build_portName {
	my ($self) = @_;

	opendir my $dir, "/dev" or die $!;
	my @modems = grep /^cu\.usbmodem/, readdir $dir;

	die "Can't find a modem in /dev ...\n" if @modems < 1;
	die "Don't know which modem to use: @modems\n" if @modems > 1;

	return "/dev/$modems[0]";
}


sub build_driver {
	my ($self) = @_; 

	my $driver = Device::SerialPort->new($self->portName());
	$driver->debug(1);

	#$driver->datatype("raw");
	$driver->baudrate(57600) or die "Could not set baud: $^E";
	$driver->databits(8);
	$driver->parity('none') or die "Could not set parity: $^E";
	$driver->stopbits(1) or die "Could not set stopbits: $^E";

	$driver->stty_echo(0);

	$driver->read_const_time(10000);
	$driver->read_char_time(0);

	$driver->write_settings();
	
	return $driver;
}

sub read {
	my ($self, $bytes) = @_; 
	my $buf;
		
	while(1) {
		my ($bytesRead);
				
		($bytesRead, $buf) = $self->driver->read($bytes);
		
		next if $bytesRead == 0; 
	
		if ($bytesRead != $bytes) {
			die "Tried to read $bytes bytes but only got $bytesRead";
		}
		
		last; 
	}
	
	return $buf; 
	
}

sub write {
	my ($self, $content) = @_; 

	use bytes;
	my $length = length($content); 
	my $written; 

	$written = $self->driver->write($content);

	if ($written != $length) {
		die "tried to write $length bytes but only wrote $written: $!"; 
	}

	return; 
}

1;