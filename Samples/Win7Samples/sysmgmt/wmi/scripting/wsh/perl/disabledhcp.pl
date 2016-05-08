#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation 
#
# WMI Sample Script -  Disable DHCP on adapter (Perl Script)
#
# This script demonstrates how to disable DHCP use on an instance of
# Win32_NetworkAdapterConfiguration.  In this case we specify the adapter with an Index
# of 0.  The correct index should be selected from Win32_NetworkAdapter instances for other
# interfaces.
#
# NOTE:  This script only applies to NT-based systems
# NOTE:  Change the ipaddr and subnet variables below to the values you wish to apply
#        to the adapter.
#***************************************************************************
use strict;
use Win32::OLE;

my ($Adapter, @ipaddr, @subnet, $RetVal);  
eval { $Adapter = 
	Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2:Win32_NetworkAdapterConfiguration.Index=\"0\""); };

unless ($@) 
{
	push @ipaddr, "192.168.144.107";
	push @subnet, "255.255.255.0";

	$RetVal = $Adapter->EnableStatic(\@ipaddr, \@subnet);

	if ($RetVal == 0) 
	{
		print "\nDHCP disabled, using static IP address\n";
	}
	else 
	{
		print "\nDHCP disable failed\n";
	}
}
else
{
	print STDERR "\n", Win32::OLE->LastError, "\n";
}
