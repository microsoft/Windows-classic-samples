#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Method invocation (Perl Script)
#
# This script demonstrates how to invoke a CIM method
# as if it were an automation method of SWbemObject
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($process, $outParam, $processid, $inParam, $objMethod);

eval { $process = 
	Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2:Win32_Process"); };

if (!$@ && defined $process)
{
	$objMethod = $process->Methods_("Create");

	#Spawn an instance of inParameters and assign the values.
	$inParam = $objMethod->inParameters->SpawnInstance_ if (defined $objMethod);
	$inParam->{CommandLine} = "notepad.exe";
	$inParam->{CurrentDirectory} = undef;
	$inParam->{ProcessStartupInformation} = undef;

	$outParam = $process->ExecMethod_("Create", $inParam) if (defined $inParam);
	if ($outParam->{ReturnValue})
	{
		print STDERR Win32::OLE->LastError, "\n";
	}
	else
	{
		print "Method returned result = $outParam->{ReturnValue}\n";
		print "Id of new process is $outParam->{ProcessId}\n"
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}