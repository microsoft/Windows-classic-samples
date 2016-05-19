#! perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Win32_ComputerSystem dump (Perl Script)
#
# This script demonstrates how to dump properties from instances of
# Win32_ComputerSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $SystemSet;
eval { $SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf ("Win32_ComputerSystem"); };
if(!$@ && defined $SystemSet)
{
	print "\n";
	foreach my $SystemInst (in $SystemSet)
	{
		print $SystemInst->{Caption}, "\n";
		print $SystemInst->{PrimaryOwnerName}, "\n";
		print $SystemInst->{Domain}, "\n";
		print $SystemInst->{SystemType}, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}