#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  List print jobs (Perl Script)
#
# This script demonstrates how to retrieve printer jobs stats from instances of
# Win32_PrintJob.
#
#***************************************************************************
use strict;
use Win32::OLE;

close (STDERR);

my ($PrintJobset, $PrintJob);
eval {$PrintJobset = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}")->
	InstancesOf ("Win32_PrintJob") };
if (!$@ && defined $PrintJobset)
{
	if ($PrintJobset->{Count} == 0 ) 
	{
		print "\nNo print jobs!\n";
	}

	foreach $PrintJob (in $PrintJobset)
	{
		print $PrintJob->{Name} , "\n";
		print $PrintJob->{JobId} , "\n";
		print $PrintJob->{Status} , "\n";
		print $PrintJob->{TotalPages} , "\n";
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}
