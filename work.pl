use Moose; 
use lib qw(lib/);

use Device::Firmdata; 

$| = 1;
print '';

my $firmdata = Device::Firmdata->new(commandLineArgs => { @ARGV } );

$firmdata->run;