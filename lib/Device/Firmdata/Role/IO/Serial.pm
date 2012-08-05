package Device::Firmdata::Role::IO::Serial;

use Moose::Role;

with 'Device::Firmdata::Role::IO';

has fh => ( is => 'ro', isa => 'GlobRef', required => 1, lazy => 1, builder => 'build_fh' );

sub build_fh {
	my ($self) = @_;
	my $path = $self->portName;
	my $fh;
	
	die "Could not open $path for read/write" unless open($fh, '+>', $path);
	
	return $fh; 
}

sub read {
	my ($self, $wantBytes) = @_; 
	my $bytesLeft = $wantBytes; 
	my $readBytes = 0;
	my $outputBuf;  
	
	while($bytesLeft > 0) {
		my $readBuf; 
		my $bytesRead = sysread($self->fh, $readBuf, $bytesLeft);
		
		if ($bytesRead == -1) {
			die "Could not read from fh: $!";
		}
		
		$bytesLeft -= $bytesRead; 
		
		$outputBuf .= $readBuf; 
	}
	
	if ($bytesLeft < 0) {
		die "read too much data";
	}
	
	return $outputBuf; 
}

sub write {
	my ($self, $content) = @_; 
	my $length = length($content);
	my $bytesLeft = length($content);
	
	while($bytesLeft > 0) {
		my $bytesSent = syswrite($self->fh, $content, $bytesLeft, $length - $bytesLeft);
		
		if ($bytesSent == -1) {
			die "could not write to fh: $!";
		}
		
		$bytesLeft -= $bytesSent; 
	}
	
	return; 
}

1;
