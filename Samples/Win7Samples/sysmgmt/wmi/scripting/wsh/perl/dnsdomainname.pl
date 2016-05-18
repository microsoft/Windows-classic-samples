#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  DNS Domain name (Perl Script)
#
# This script demonstrates how to retrieve the DNS domain name of the local machine 
# from instances of Win32_ComputerSystem.
#
#***************************************************************************
use strict;
use Win32::OLE;

close (STDERR);

my ($NICSet, $NIC);  
eval {$NICSet = Win32::OLE->GetObject("winmgmts:!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled=true"); };
if (!$@ && defined $NICSet)
{
	foreach $NIC (in $NICSet)
	{
		if(defined $NIC->{DNSDomain})
		{
			print "\n", $NIC->{DNSDomain}, "\n";
		}
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}

