#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Starts a service (Perl Script)
#
# This script demonstrates how to start a specific service from instances of
# Win32_Service.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not
# support services
#
#***************************************************************************
use strict;
use Win32::OLE;

my $ServiceSet;

eval { $ServiceSet = 
	Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_Service WHERE Name='ClipSrv'"); };

if(!$@ && defined $ServiceSet)
{
	foreach my $service (in $ServiceSet)
	{
		my $Result = $service->StartService();
		if ($Result == 0)	
		{
			print "\nService started\n";
		}
		elsif ($Result == 10)
		{
			print "\nService already running\n";
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}

