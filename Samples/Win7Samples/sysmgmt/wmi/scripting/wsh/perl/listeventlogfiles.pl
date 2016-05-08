#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List event log files (Perl Script)
#
# This script demonstrates how to retrieve the info about the event log files 
# on the local machine from instances of Win32_NTEventLogFile.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support event logs
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $LogFileSet, $LogFile );

eval { $LogFileSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
						InstancesOf("Win32_NTEventLogFile"); };

unless ($@)
{
	print "\n";
	foreach $LogFile (in $LogFileSet)
	{
		print "Log Name: ", $LogFile->{LogfileName}, "\n";
		if(defined ($LogFile->{NumberOfRecords}))
		{
			print "Number of Records: ", $LogFile->{NumberOfRecords}, "\n";
		}
		else
		{
			print "Number of Records: \n";
		}
		print "Max Size: ", $LogFile->{MaxFileSize}, " bytes", "\n";
		print "File name: ", $LogFile->{Name}, "\n";
		print "\n";
	}
}
else
{
	print STDERR Win32->LastError, "\n";
}						