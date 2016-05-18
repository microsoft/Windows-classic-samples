#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
#
# This sample illustrates how to retrieve perfmon counter values from WMI.  Note that the accompanying MOF
# (perfmon.mof) must be compiled and loaded for this sample to run correctly.
#
#***************************************************************************

use strict;
use Win32::OLE;

close(STDERR);

my ($process, $processSet, $memoryItem, $memorySet);
eval {$processSet = Win32::OLE->GetObject("winmgmts:root/perfmonScriptExample")->
		InstancesOf ("NTProcesses") };
unless($@)
{
	print "Displaying Process Counters\n";
	print "===========================\n";
	print "\n";

	foreach $process (in $processSet)
	{
		printf "  %s: #Threads=%d" , $process->{Process} , $process->{Threads};
		printf " Working Set=%d\n" ,  $process->{WorkingSet};
		print "\n";
	}

	print "Displaying Memory Counters\n";
	print "==========================\n";
	print "\n";

	eval {$memorySet = Win32::OLE->GetObject("winmgmts:root/perfmonScriptExample")->
			InstancesOf ("NTMemory") or die "Cannot access WMI.\n"; };
	unless($@)
	{
		foreach $memoryItem (in $memorySet)
		{
			print "  " , $memoryItem->{Memory} , ": Committed Bytes=" , $memoryItem->{CommittedBytes}, "\n";
		}
	}
	else
	{
		print Win32::OLE->LastError, "\n";
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}
