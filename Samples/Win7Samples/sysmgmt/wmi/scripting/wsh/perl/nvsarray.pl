#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Named value set array manipulation (Perl Script)
#
# This script demonstrates the manipulation of named value sets, in the case 
# that the named value is an array type.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $Context, $str, $x, @v);

eval {$Context = new Win32::OLE 'WbemScripting.SWbemNamedValueSet'; };
unless($@)
{
	$Context->Add("n1", [1, 2, 3]);
	$str = "The initial value of n1 is {";
	for ($x = 0; $x < @{$Context->Item("n1")->{Value}}; $x++)
	{
		$str = $str.@{$Context->Item("n1")->{Value}}[$x];
		if ($x != (@{$Context->Item("n1")->{Value}}) - 1 )
		{
			$str = $str.", ";
		}
	}
	$str = $str."}";
	print $str,"\n\n";

	# report the value of an element of the context value
	@v = @{$Context->Item("n1")->{Value}};
	print "By indirection the first element of n1 has value:", $v[0], "\n";

	# report the value directly
	print "By direct access the first element of n1 has value:", @{$Context->Item("n1")->{Value}}[0], "\n";

	# set the value of a single named value element
	@{$Context->Item("n1")->{Value}}[1] = 11;
	print "After direct assignment the first element of n1 has value:", 
							@{$Context->Item("n1")->{Value}}[1], "\n";

	# set the value of a single named value element
	@v = @{$Context->Item("n1")->{Value}};
	$v[1] = 345;
	print "After indirect assignment the first element of n1 has value:", 
				@{$Context->Item("n1")->{Value}}[1], "\n";

	# set the value of an entire context value
	$Context->Item("n1")->{Value} = [5, 34, 178871];
	print "After direct array assignment the first element of n1 has value:", 
			@{$Context->Item("n1")->{Value}}[1], "\n";

	$str = "After direct assignment the entire value of n1 is {";
	for ($x = 0; $x < @{$Context->Item("n1")->{Value}}; $x++)
	{
		$str = $str.@{$Context->Item("n1")->{Value}}[$x];
		if ($x != (@{$Context->Item("n1")->{Value}}) - 1 )
		{
			$str = $str.", ";
		}
	}
	$str = $str."}";
	print $str,"\n";
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
