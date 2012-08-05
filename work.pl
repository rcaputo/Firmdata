use Moose; 
use lib qw(lib/);

use Device::Firmdata; 

$| = 1;
print '';

my $firmdata = Device::Firmdata->new(config => { @ARGV } );

$firmdata->run;
