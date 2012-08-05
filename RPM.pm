#This is enough code to read the output of a magnetometer and convert field strength readings into
#magnetic pole events. The sensor is mounted near a shaft that has 3 magnets attached to it with
#the sensor reading two north poles and one south pole per rotation. When a south pole is encountered
#it is considered a turn and the RPM of the shaft is calculated from the duration of the turn.

#With two north poles and one south pole it's possible to count the number of north poles detected
#and look at that value when the turn event is generated. If the count of the north poles is not two
#then some kind of error happened - it's possible to then reconstruct which pole was not counted

#Sample Output:

#Tyler@Tyler-PC ~/work/Firmdata
#$ perl work.pl portName com3 sessionFile RPM.pm
#Session started at Fri Aug  3 22:24:00 2012
#RPM is 122.581066945607 at RPM.pm line 99.
#RPM is 147.220477386935 at RPM.pm line 99.
#RPM is 160.97184065934 at RPM.pm line 99.
#RPM is 168.858069164266 at RPM.pm line 99.
#RPM is 174.386160714285 at RPM.pm line 99.
#RPM is 177.556818181819 at RPM.pm line 99.
#RPM is 180.288461538461 at RPM.pm line 99.
#RPM is 182.535046728972 at RPM.pm line 99.
#RPM is 183.10546875 at RPM.pm line 99.

package RPMController;

use Moose;

with 'Device::Firmdata::Role::Session';

has currentPole => ( is => 'rw', isa => 'Maybe[Str]', required => 1, default => undef );
has lastTurn => ( is => 'rw', isa => 'Maybe[Num]', required => 1, default => undef );

sub sessionOpen {
	my ($self) = @_;
	
	#connect output pin 1 and 2 to messages from channel 1 and 2
	#args: pin, channel
	#this is to send data to two servos
	$self->publish(1, 1);
	$self->publish(2, 2);
	
	#connect input pin 1 to message channel 1; receive data at an interval of 2 timer ticks with
	#zero phase offset
	#args: pin, channel, interval, offset
	#this is the magnetometer data and has to be sampled very quickly, 2 timer ticks gives a frequency of 1024hz
	$self->subscribe(1, 1, 2, 0);
	
	#connect input pin 2 to message channel 2; receive data at an interval of 100 timer ticks with a phase offset of 1 timer tick
	#this reads the knob position around 204 hz; with the phase offset of 1 timer tick the sample will always be done at odd timer
	#intervals and the magnetometer always happens at even ones - the sample requests will never colide with each other because 
	#a sample takes less time than a timer tick
	$self->subscribe(2, 2, 100, 1);
}

sub sessionClose {
	my ($self) = @_;
}

sub inputData {
	my ($self, $channel, $when, $value) = @_; 
	
	if ($channel == 1) {
		$self->got_magnetometer($when, $value);
	} elsif ($channel == 2) {
		$self->got_knob($when, $value); 
	}
}

#the magnetometer returns a value of 127 when there is no detected magnetic field
#the value moves towards 255 for a south pole and towards 0 for a north pole
sub got_magnetometer {
	my ($self, $when, $value) = @_;
	my $currentPole = $self->currentPole; 
	
	if (defined($currentPole)) {
		if ($currentPole eq 's' && $value < 132) {
			$self->outside_pole($when);
			$self->currentPole(undef); 
		} elsif ($currentPole eq 'n' && $value > 125 ) {
			$self->outside_pole($when);
			$self->currentPole(undef);				
		}
	} 
	
	#a single reading may both indicate that a pole is no longer present
	#and that a new one has been detected so this should not be in an else
	#block with the logic above that clears out the current pole 
	if ($value > 135) {
		$self->currentPole('s');
		$self->inside_pole($when);					
	} elsif ($value < 122) {
		$self->currentPole('n');
		$self->inside_pole($when);	
	}
}

sub inside_pole {
	my ($self, $when) = @_;
	#this is where north poles could be counted
}

#counting a turn when a pole is left instead of when it is entered
#solves the problem of the system starting when a pole is being
#read and that counting as a turn causing a false RPM value to be
#calculated even if the shaft is not turning 
sub outside_pole {
	my ($self, $when) = @_; 
	
	if ($self->currentPole eq 's') {
		if (defined($self->lastTurn)) {
			my $duration = $when - $self->lastTurn; 
			my $rpm = 1 / $duration * 60; 
			
			warn "RPM is $rpm";
		}
		
		$self->lastTurn($when);
	}
}

sub got_knob {
	my ($self, $when, $value) = @_; 
	
	warn "Knob at $when";
}

return "RPMController";