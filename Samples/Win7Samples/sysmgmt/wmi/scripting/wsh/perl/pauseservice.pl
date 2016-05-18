#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  Pauses a service (Perl Script)
#
# This script demonstrates how to pause a specific service from instances of
# Win32_Service.
#
# NOTE:  The service must support pausing and be running already.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support services
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($ServiceSet, $SupportsPause, $RetVal);  
eval {$ServiceSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_Service WHERE Name='Schedule'"); };
unless($@)
{
	foreach my $ServiceInst (in $ServiceSet)
	{
		if ($ServiceInst->{AcceptPause})
		{
			$RetVal = $ServiceInst->PauseService();
			if ($RetVal == 0)
			{
				print "\nService paused\n";
			}
			else
			{
				if ($RetVal == 1)
				{
					print "\nPause not supported\n" ;
				}
				else 
				{
					print "\nAn error occurred:", $RetVal, "\n";
				}
			}	
		}
		else
		{
			print "\nService does not support pause\n";
		}
	}
}
else
{
	print STDERR "\n", Win32::OLE->LastError, "\n";
}

