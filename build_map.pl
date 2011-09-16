#!/usr/bin/perl
use strict;

#MAPS ALL CHANNELS 1 - 127 to CCs On/Off pattern
my @output;
my $line = "[{CHANNEL}]=[{CTRL}, {VAL}]";
my $start_cc = 25;
for (my $x = 1; $x <= 128; $x++)
	{
	my $pline = $line;
	$pline =~ s/{CHANNEL}/$x/g;
	my $cc = $start_cc++;
	$pline =~ s/{CTRL}/$cc/g;
	my $pline2 = $pline;
	$pline =~ s/{VAL}/0/g;
	push (@output, $pline);
	$pline2 =~ s/{VAL}/127/g;
	push (@output, $pline2);
	}
foreach (@output)
	{
	print "$_\n";
	}