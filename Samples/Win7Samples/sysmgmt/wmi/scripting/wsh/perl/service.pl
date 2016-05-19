#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Service info (Perl Script)
#
# This script displays all currently installed services.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $ServiceSet;
eval { $ServiceSet =
	Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	InstancesOf ("Win32_Service"); };
if(!$@ && defined $ServiceSet)
{
	foreach my $Service (in $ServiceSet)
	{
		print "\n";
		print $Service->{Name}, "\n";
		if( $Service->{Description} ) 
			{
				print "  $Service->{Description}\n";
			}
		else
			{
				print "  <No description>\n";
			}
		print "  Executable:   $Service->{PathName}\n";
		print "  Status:       $Service->{Status}\n";
		print "  State:        $Service->{State}\n";
		print "  Start Mode:   $Service->{StartMode}\n";
		print "  Start Name:   $Service->{StartName}\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}