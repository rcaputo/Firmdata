package Device::Firmdata::Util::Accumulator;

use Moose;

has value => ( is => 'rw', isa => 'Num', required => 1, default => 0 ); 
has autoReset => ( is => 'rw', isa => 'Bool', required => 1, default => 0 );

sub add {
	my ($self, $newValue) = @_;
	
	$self->value($self->value + $newValue); 
}

sub get {
	my ($self) = @_; 
	my $value = $self->value;
	
	if ($self->autoReset) {
		$self->value(0);	
	}
	
	return $value; 
}

1;