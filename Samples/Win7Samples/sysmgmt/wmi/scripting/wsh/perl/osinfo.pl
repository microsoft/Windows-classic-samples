#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Information about the OS (Perl Script)
#
# This script demonstrates how to retrieve the info about the OS on the local machine from instances of
# Win32_OperatingSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $SystemSet, $system );

eval { $SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
						InstancesOf("Win32_OperatingSystem"); };
unless($@)
{
	print "\n" ;
	for $system (in $SystemSet) 
	{
		print $system->{Caption}, "\n" ;
		print $system->{Manufacturer}, "\n" ;
		print $system->{BuildType}, "\n" ;
		print " Version: " , $system->{Version}, "\n" ;
		print " Locale: " , $system->{Locale}, "\n" ;
		print " Windows Directory: " , $system->{WindowsDirectory}, "\n" ;
		print " Total memory: " , $system->{TotalVisibleMemorySize}, " bytes \n" ;
		print " Serial Number: " , $system->{SerialNumber} ,"\n" ;
		print "\n" ;
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}