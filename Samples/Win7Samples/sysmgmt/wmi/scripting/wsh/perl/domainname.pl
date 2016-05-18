#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  Domain name (Perl Script)
#
# This script demonstrates how to retrieve the domain name of the local machine 
# from instances of Win32_ComputerSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($SystemSet, $System);  
eval {$SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
		InstancesOf ("Win32_ComputerSystem") };
		
unless($@)
{
	foreach $System (in $SystemSet)
	{
		print "\n", $System->{Domain}, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
