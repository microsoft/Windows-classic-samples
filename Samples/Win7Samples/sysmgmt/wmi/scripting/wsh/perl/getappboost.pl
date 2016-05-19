#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Get application boost value (Perl Script)
#
# This script demonstrates how to retrieve the info about the application 
# boost on the local machine from instances of Win32_OperatingSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $SystemSet, $System );

eval { $SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
							InstancesOf("Win32_OperatingSystem"); };
unless($@)
{
	print "\n";
	for $System (in $SystemSet) 
	{
		print "Application boost: ", $System->{ForegroundApplicationBoost}, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}