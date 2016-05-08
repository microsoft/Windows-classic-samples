#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Error and error object handling (Perl Script)
#
# This script demonstrates how to inspect error and error object
# information.
#
#***************************************************************************
use strict;
use Win32::OLE;

my ( $t_Service, $t_Object, $t_Object2, $strDescr, $strPInfo, $strCode );

# Close STDERR file handle to eliminate redundant error message
close(STDERR);

# Ask for non-existent class to force error 
$t_Service = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\default"); 
$t_Object = $t_Service->Get("Nosuchclass000");

if (defined $t_Object)
{
	print "Got a class\n"; 
}
else
{
	print "\nErr Information:\n\n";
	print Win32::OLE->LastError, "\n";

	# Create the last error object
	$t_Object = new Win32::OLE 'WbemScripting.SWbemLastError';
	print "\nWMI Last Error Information:\n\n";
	print " Operation: ", $t_Object->{Operation}, "\n";
	print " Provider: ", $t_Object->{ProviderName}, "\n";
	
	$strDescr = $t_Object->{Description};
	$strPInfo = $t_Object->{ParameterInfo};
	$strCode = $t_Object->{StatusCode};

	if (defined $strDescr)
	{
		print " Description: ", $strDescr, "\n";		
	}

	if (defined $strPInfo)
	{
		print " Parameter Info: ", $strPInfo, "\n";		
	}

	if (defined $strCode)
	{
		print " Status: ", $strCode, "\n";		
	}

	print "\n";

	$t_Object2 = new Win32::OLE 'WbemScripting.SWbemLastError';
	if (defined $t_Object2)
	{
		print "Got the error object again - this shouldn't have happened!\n";
	}
	else
	{
		print "Couldn't get last error again - as expected\n";
	}
}