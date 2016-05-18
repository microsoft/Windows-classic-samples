#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Current boot configuration (Perl Script)
#
# This script demonstrates how to retrieve the current boot configuration 
# setting of the local machine from instances of Win32_OperatingSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $BootSet, $Boot );

eval { $BootSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
							InstancesOf("Win32_OperatingSystem"); };
unless($@)
{
	print "\n" ;
	for $Boot (in $BootSet) 
	{
		print $Boot->{Name}, "\n" ;
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}