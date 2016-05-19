#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Object Collection (Perl Script)
#
# This script illustrates how SWbemObjectSet collections are manipulated.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($disks,$disk);

eval { $disks = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
			InstancesOf("CIM_LogicalDisk"); };
unless($@)
{
	print "\nThere are ", $disks->{Count}, " Disks \n";

	eval { $disk = $disks->Item("Win32_LogicalDisk.DeviceID=\"C:\""); };
	unless($@)
	{
		print $disk->{Path_}->{Path}, "\n";
	}
	else
	{
		print STDERR Win32::OLE->LastError, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
