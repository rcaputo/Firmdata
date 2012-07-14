use strict;
use warnings; 

#my $num = 0; 
#
#$num |= 1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | 1 << 3; 
#
#die $num; 

open(my $serial, "+<", '/dev/ttyS2') or die "could not open for read/write: $!";

my $buf; 

print $serial pack('CCCCCCC', 3, 1, 2, 3, 4, 5, 6) or die "could not write";

$| = 1;
print ''; 

read($serial, $buf, 4) == 4 or die "could not read";
my ($messageHeader, $command, $resp) = unpack('CCS<', $buf);
my $messageChannel = ($messageHeader & 248);# >> 3; 
my $messageSize = $messageHeader & 7; 

print "$messageChannel $messageSize $command $resp\n";

