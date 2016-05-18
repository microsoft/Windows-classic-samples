#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List services (Perl Script)
#
# This script demonstrates how to retrieve a list of running services from instances of
# Win32_Service.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support services
#
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $ServiceSet, $Service );

eval { $ServiceSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_Service WHERE State=\"Running\""); };
unless ($@)
{
	print "\n";
	foreach $Service (in $ServiceSet) 
	{
		print $Service->{Name}, "\n";
		if( $Service->{Description} ) 
			{
				print "  $Service->{Description}\n";
			}
		else
			{
				print "  <No description>\n";
			}
		print "  Process ID: ", $Service->{ProcessId}, "\n";
		print "  Start Mode: ", $Service->{StartMode}, "\n";
		print "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}