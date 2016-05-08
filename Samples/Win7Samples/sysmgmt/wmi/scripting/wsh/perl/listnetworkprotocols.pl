#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - List network protocols (Perl Script)
#
# This script demonstrates how to retrieve a list of running services from instances of
# Win32_NetworkProtocol.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $ProtocolSet, $Protocol );

eval { $ProtocolSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_NetworkProtocol"); };
unless($@)
{
	print "\n";
	foreach $Protocol (in $ProtocolSet) 
	{
		print $Protocol->{Name}, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}