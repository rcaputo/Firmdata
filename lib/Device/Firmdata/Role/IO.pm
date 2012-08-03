package Device::Firmdata::Role::IO;

use Moose::Role;

use Device::Firmdata::Util::Accumulator;

has portName => ( is => 'ro', isa => 'Str', required => 1);
has bytesRead => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new(autoReset => 1) } );
has bytesSent => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new(autoReset => 1) } );

requires 'read';
requires 'write'; 

use constant HEADER_CHANNEL_MASK => 248;
use constant HEADER_CHANNEL_SHIFT => 3; 
use constant HEADER_SIZE_MASK => 7; 
use constant HEADER_LENGTH => 1;

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
	my $bytesRead = $length + HEADER_LENGTH; 
	
	$self->bytesRead->add($bytesRead); 
	
	return ($channel, $content, $bytesRead); 
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
	my $bytesSent = length($content); 
	
	$self->bytesSent->add($bytesSent); 
	
	#TODO - update command.c and message.c to support commands embeded in messages
	#$self->sendHeader($channel, length($content)); 
	$self->write($content); 	
}

1;