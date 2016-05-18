#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - View Page File settings (Perl Script)
#
# This script demonstrates how to retrieve page file stats from instances of
# Win32_PageFile.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $PageFileSet;

eval { $PageFileSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf ("Win32_PageFile"); };
if (!$@ && defined $PageFileSet)
{
	foreach my $PageFileInst (in $PageFileSet)
	{
		print "\n$PageFileInst->{Name}\n";
		print " Initial: $PageFileInst->{InitialSize} MB\n";
		print " Maximum: $PageFileInst->{MaximumSize} MB\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}

