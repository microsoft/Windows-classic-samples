#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation 
#
# WMI Sample Script - Date and time (Perl Script)
#
# This script demonstrates how to retrieve the date and time of the local machine 
# from instances of Win32_OperatingSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($System, $SystemSet) ;
eval {$SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}")->
	InstancesOf ("Win32_OperatingSystem") };
unless($@)
{
	foreach $System (in $SystemSet)
	{
		print "\n", $System->{LocalDateTime}, "\n";
	}
}
else
{
	print STDERR "\n", Win32::OLE->LastError, "\n";
}
