#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List network adapters (Perl Script)
#
# This script demonstrates how to retrieve a list of adapters from instances of
# Win32_NetworkAdapterConfiguration.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $AdapterSet, $Adapter );

eval { $AdapterSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_NetworkAdapterConfiguration"); };
unless($@)
{
	print "\n";
	foreach $Adapter (in $AdapterSet) 
	{
		if ($Adapter->{DHCPEnabled} == 1)
		{
			print "Adapter:", $Adapter->{Description}, "\n";
			print "IP:", ${$Adapter->{IPAddress}}[0], "\n";
			print "Adapter:", $Adapter->{MACAddress}, "\n";
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}