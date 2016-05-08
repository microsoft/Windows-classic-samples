#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation 
#
# WMI Sample Script -  SWbemObject.Derivation_ access (Perl Script)
#
# This script demonstrates the manipulation of the derivation_ property
# of SWbemObject.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($C, $D, @collection);

eval {$C = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
		InstancesOf ("win32_logicaldisk") };
unless ($@) 
{
	@collection = in $C;
	eval {$D = $collection[0]->Derivation_();};
	print "\n";
	unless ($@) 
	{
		print map{"$_\n"} @{$D};
	}
	else
	{
		print STDERR Win32::OLE->LastError, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
