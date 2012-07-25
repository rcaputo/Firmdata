package TestSession; 

use strict;
use warnings;

use base qw(Device::Firmdata::Session);

sub init {
	my ($self) = @_;
	
	$self->SUPER::init();
	
	$self->subscribe(1, 1, 5, 0);
	$self->subscribe(2, 2, 100, 1);	
}

sub data {
	my ($self, $channel, $when, $content) = @_; 
	
	$self->SUPER::data($channel, $when, $content); 

	print "$channel\t$when\t$content\n";
}

sub update {
	my ($self) = @_; 
	my $buf = '';
	return $buf; 

} 

__PACKAGE__;  