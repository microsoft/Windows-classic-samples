#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Machine name (Perl Script)
#
# This script demonstrates how to retrieve the local machine name from instances of
# Win32_ComputerSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $SystemSet, $System );

eval { $SystemSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf("Win32_ComputerSystem"); };
unless($@)
{
	print "\n";
	foreach $System (in $SystemSet)
	{
		print $System->{Name}, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}