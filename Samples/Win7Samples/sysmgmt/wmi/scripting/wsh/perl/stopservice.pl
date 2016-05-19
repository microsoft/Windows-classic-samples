#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Stops a service (Perl Script)
#
# This script demonstrates how to stop a specific service from instances of
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

if (!$@ && defined $ServiceSet)
{
	foreach my $ServiceInst (in $ServiceSet)
	{
		my $Result = $ServiceInst->StopService();
		if ($Result == 0)
		{
			print "\nService stopped\n";
		}
		elsif ($Result == 5) 
		{
			print "\nService already stopped\n";
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
