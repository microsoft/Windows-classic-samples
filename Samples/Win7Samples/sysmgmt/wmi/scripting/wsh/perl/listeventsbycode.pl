#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List event log events of a particular type (Perl Script)
#
# This script demonstrates how to retrieve the events of a particular type (Event code)
# from the Win32_NTLogEvent class
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support event logs
#
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $EventSet, $LogEvent );

eval { $EventSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
					ExecQuery("SELECT * FROM Win32_NTLogEvent WHERE EventCode=4097"); };
unless( $@ )
{
	print "\n";
	if ( $EventSet->{Count} == 0 )
	{
		print "No Events\n";
	}
	else
	{
		foreach $LogEvent ( in $EventSet )
		{
			print "Event Number: ", $LogEvent->{RecordNumber}, "\n";
			print "Log File: ", $LogEvent->{LogFile}, "\n";
			print "Type: ", $LogEvent->{Type}, "\n"; 
			print "Message: ", $LogEvent->{Message}, "\n";
			print "Time: ", $LogEvent->{TimeGenerated}, "\n";
			print "\n";
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}