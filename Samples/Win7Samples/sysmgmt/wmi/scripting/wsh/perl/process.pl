#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Win32_Process enumeration (Perl Script)
#
# This script demonstrates how to enumerate processes.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $ProcessSet;

eval { $ProcessSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf ("Win32_Process"); };
if (!$@ && defined $ProcessSet)
{
	foreach my $ProcessInst (in $ProcessSet)
	{
		printf "%5d %s\n", $ProcessInst->{ProcessId}, $ProcessInst->{Name};
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}