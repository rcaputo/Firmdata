package Device::Firmdata::Role::IO;

use Moose::Role;

use Device::Firmdata::Util::Accumulator;

has portName => ( is => 'ro', isa => 'Str', required => 1);
has bytesRead => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new(autoReset => 1) } );
has bytesSent => ( is => 'ro', isa => 'Device::Firmdata::Util::Accumulator', required => 1, default => sub { Device::Firmdata::Util::Accumulator->new(autoReset => 1) } );

#each of these methods must be implemented by another role. the combination of the subrole and this role
#together form a complete message parser and transfer role. the read() method will be called with a single
#argument which is the number of bytes to read. the method must not return until that number of bytes has been
#read. similiarly the write method will be called with a single argument, the data to transmit, and it must not
#return until all that data has been transmitted. 

#read() returns a single value which is the content that was read with a length identical to the value passed in as an argument
#write() does not return any value 
requires 'read';
requires 'write'; 

use constant HEADER_CHANNEL_MASK => 248;
use constant HEADER_CHANNEL_SHIFT => 3; 
use constant HEADER_SIZE_MASK => 7; 
use constant HEADER_LENGTH => 1;

#reads the header from the IO channel and returns the contents of the header as a list of scalars
#the first value is the channel number and the second value is the message content length 
sub getHeader {
	my ($self) = @_;
	my $headerByte = $self->read(HEADER_LENGTH); 	
	my $headerValue = unpack('C', $headerByte); 
	my $channel = ($headerValue & HEADER_CHANNEL_MASK) >> HEADER_CHANNEL_SHIFT; 
	my $length = $headerValue & HEADER_SIZE_MASK; 
	
	return ($channel, $length); 
}

#reads a complete message from the IO channel - that is a header and possibly 
#additional content up to 7 bytes long. The message channel number and the content
#of the message or undef if there is none is returned as a list. 
sub getMessage {
	my ($self) = @_;
	my ($channel, $length) = $self->getHeader; 
	my $content = $self->read($length) if $length > 0;
	my $bytesRead = $length + HEADER_LENGTH; 
	
	$content = '' unless defined $content; 
	
	$self->bytesRead->add($bytesRead); 
	
	return ($channel, $content); 
}

#composes and transmits a header constructed from the two 
#arguments: message channel and content length
sub sendHeader {
	my ($self, $channel, $size) = @_;
	my $header; 
	
	$header = $channel << HEADER_CHANNEL_SHIFT; 
	$header |= $size | HEADER_SIZE_MASK; 
	
	$self->write($header); 
	
	return; 
}

#sends a complete message composed from the arguments: message channel and message content
#the content can be from 0 to 7 bytes long
sub sendMessage {
	my ($self, $channel, $content) = @_; 
	my $bytesSent = length($content); 
	
	$self->bytesSent->add($bytesSent); 
	
	#TODO - update command.c and message.c to support commands embeded in messages
	#$self->sendHeader($channel, length($content)); 
	$self->write($content); 
	
	return; 	
}

1;