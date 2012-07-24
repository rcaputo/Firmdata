#calculate jitter from a list of samples including timestamps in ms

use strict;
use warnings;

$| = 1;
print ''; 

my $lastSample = 0; 
my $lastDelay = 0; 

while(<>) {
	my ($channel, $time, $value) = split(/\s+/);

	my $delay = $time - $lastSample; 
	my $change = $delay - $lastDelay; 
	
	$lastSample = $time; 
	$lastDelay = $delay; 

	printf "$channel\t$time\t$value\t%.6f\n", $change * 1000;
}
