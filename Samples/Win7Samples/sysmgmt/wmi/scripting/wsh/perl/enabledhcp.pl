#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Enable DHCP on adapter (Perl Script)
#
# This script demonstrates how to enable DHCP use on an instance of
# Win32_NetworkAdapterConfiguration.  In this case we specify the adapter with
# an Index of 0.  The correct index should be selected from Win32_NetworkAdapter
# instances for other interfaces.
#
# Supported on NT platforms only
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $Adapter, $RetVal );

eval { $Adapter = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
							Get("Win32_NetworkAdapterConfiguration=0"); };
unless ($@)
{
	print "\n";
	$RetVal = $Adapter->EnableDHCP();
	if ( $RetVal == 0)
	{
		print "DHCP Enabled\n";
	}
	else
	{
		print "DHCP enable failed\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}