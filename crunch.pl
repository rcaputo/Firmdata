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

	printf "$time\t%.6f\n", $change * 1000;
}
