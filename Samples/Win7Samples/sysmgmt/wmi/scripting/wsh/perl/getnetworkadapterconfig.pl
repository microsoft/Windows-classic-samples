#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List network adapter configuration (Perl Script)
#
# This script demonstrates how to retrieve configuration info from an instance
# of Win32_NetworkAdapterConfiguration.  In this case we specify the adapter 
# with an Index of 0.  The correct index should be selected from 
# Win32_NetworkAdapter instances for other interfaces.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $Adapter );

eval { $Adapter = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
						Get("Win32_NetworkAdapterConfiguration=\"0\""); };
unless ($@)
{
	print "\n";
	if ( defined $Adapter->{Description} )
	{
		print "Adapter: ", $Adapter->{Description}, "\n";
	}
	else
	{
		print "Adapter: \n";
	}

	if ( defined ${$Adapter->{IPAddress}}[0] )
	{
		print "IP: ", ${$Adapter->{IPAddress}}[0], "\n";
	}
	else
	{
		print "IP: \n";
	}

	if ( defined $Adapter->{MACAddress} )
	{
		print "MAC: ", $Adapter->{MACAddress}, "\n";
	}
	else
	{
		print "MAC: \n";
	}

	if ($Adapter->{DHCPEnabled} == 1)
	{
		print "DHCP Enabled\n";
	}
	else
	{
		print "DHCP Disabled\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}