#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  Printer status (Perl Script)
#
# This script demonstrates how to retrieve printer stats from instances of
# Win32_Printer.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $PrinterSet;

eval { $PrinterSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf ("Win32_Printer"); };
unless($@)
{
	if ($PrinterSet->{Count} == 0) 
	{
		print "No Printers Installed!\n";
	}

	foreach my $PrinterInst (in $PrinterSet)
	{
		if ($PrinterInst->{PrinterStatus} == 3) 
		{
			print "\n$PrinterInst->{Name}\nStatus:  Idle\n";
		}
		if ($PrinterInst->{PrinterStatus} == 4) 
		{
			print "\n$PrinterInst->{Name}\nStatus:  Printing\n";
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}