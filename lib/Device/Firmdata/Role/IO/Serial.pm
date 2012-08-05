package Device::Firmdata::Role::IO::Serial;

use Moose::Role;
use POSIX;
use Time::HiRes qw(time);

use constant DEBUG => 0;

with 'Device::Firmdata::Role::IO';

#this is a research hack and requires some other external means to configure the serial port properly 

has fh => ( is => 'ro', isa => 'GlobRef', required => 1, lazy => 1, builder => 'build_fh' );

sub build_fh {
	my ($self) = @_;
	my $path = $self->portName;
	my $fh;
	
	die "Could not open $path for read/write" unless open($fh, '+>', $path);
	binmode($fh);

	my $tio = POSIX::Termios->new();
	$tio->getattr(fileno $fh);

	$tio->setlflag(0);

	my $cflag = $tio->getcflag();
	$cflag = (
		CLOCAL |           # ignore modem status lines
		CREAD |            # enable receiver
		HUPCL              # hang up on last close
	);
	$cflag |= CS8;       # 8 bits
	$cflag &= ~PARENB;   # turn off parity enable
	$cflag &= ~CSTOPB;   # turn off 2 stop bits (enable 1 stop)
	$tio->setcflag($cflag);

	$tio->setiflag(
		IGNBRK |       # ignore break
		IGNPAR |       # ignore parity errors
		INPCK          # enable parity checking
	);

	$tio->setoflag(0);

	# Non-blocking mode.  Return whatever is there.
	$tio->setcc(VMIN, 0);
	$tio->setcc(VTIME, 0);

	$tio->setispeed(57600);
	$tio->setospeed(57600);

	# Apply Termios to the device now.
	$tio->setattr(fileno($fh), TCSANOW);

	return $fh; 
}

sub read {
	my ($self, $wantBytes) = @_; 
	my $bytesLeft = $wantBytes; 
	my $readBytes = 0;
	my $outputBuf;

	my $fh = $self->fh();
	my $tio = POSIX::Termios->new();
	$tio->getattr(fileno $fh);

	while($bytesLeft > 0) {
		my $readBuf; 

		# Blocking until there are enough bytes.
		# Remove this for a non-blocking library.
		$tio->setcc(VMIN, ($bytesLeft > 255) ? 255 : $bytesLeft);
		$tio->setattr(fileno($fh), TCSANOW);

		my $bytesRead = sysread($fh, $readBuf, $bytesLeft);
		
		if ($bytesRead == -1) {
			die "Could not read from fh: $!";
		}
		
		$bytesLeft -= $bytesRead; 
		
		$outputBuf .= $readBuf; 
	}
	
	if ($bytesLeft < 0) {
		die "read too much data";
	}

	if (DEBUG) {
		my $output_ascii = $outputBuf;
		$output_ascii =~ s/([^ -~])/sprintf '\\x%02x', ord $1/eg;
		warn sprintf "%.3f <-- %s\n", time(), $output_ascii;
	}

	return $outputBuf; 
}

sub write {
	my ($self, $content) = @_; 
	my $length = length($content);
	my $bytesLeft = length($content);

	if (DEBUG) {
		my $output_ascii = $content;
		$output_ascii =~ s/([^ -~])/sprintf '\\x%02x', ord $1/eg;
		warn sprintf "%.3f --> %s\n", time(), $output_ascii;
	}

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
