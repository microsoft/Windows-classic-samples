#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
#
# WMI Sample Script -  CIMOM Identification (Perl Script)
#
# This script demonstrates the display of WMI installation info.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ($Cimomid, $locator, $services);

eval { $Cimomid = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\default")->
	Get("__CIMOMIdentification=@"); };

unless ($@)
{
	print "\n", $Cimomid->Path_()->{displayname}, "\n";
	print $Cimomid->{versionusedtocreatedb}, "\n";
}
else
{	
	print STDERR "\n", Win32::OLE->LastError, "\n";
}
		