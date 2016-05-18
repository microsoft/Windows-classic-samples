#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Resumes a service (Perl Script)
#
# This script demonstrates how to resume a paused service from instances of
# Win32_Service.
#
# NOTE:  The service must support pausing and be running already.
#
# NOTE:  This script only applies to NT-based systems since Win9x does 
#		 not support services
#
#***************************************************************************
use strict;
use Win32::OLE;

my $ServiceSet;

eval { $ServiceSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_Service WHERE Name='Schedule'"); };

if (!$@ && defined $ServiceSet)
{
	foreach my $Service (in $ServiceSet)
	{
		my $SupportsPause = $Service->{AcceptPause};
		if ($SupportsPause)
		{
			my $RetVal = $Service->ResumeService();
			if ($RetVal == 0)
			{
				print "\nService resumed\n";
			}
			else
			{
				if ($RetVal == 1)
				{
					print STDERR "\nPause not supported\n";
				}
				else
				{
					print STDERR "\nAn error occurred: ", $RetVal, "\n";
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
	print STDERR Win32::OLE->LastError, "\n";
}