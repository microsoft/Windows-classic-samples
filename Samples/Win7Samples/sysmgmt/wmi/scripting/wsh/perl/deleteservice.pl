#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script - Delete a service (Perl Script)
#
# This script demonstrates how to delete a specific service from instances of
# Win32_Service.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support services
#
# NOTE:  Deleting a service is a permanent change to the system and this script should
#        be used with caution.  To help avoid users from accidently deleting a useful
#        service, the script has been created with the string "MyService" as the name of
#        the service to be deleted.  Change this to the name of the service you wish to 
#        delete but be sure you wish to permanently remove the service before using this 
#        script.
#***************************************************************************
use strict;
use Win32::OLE;

my ($Service, $ServiceSet) ;
eval {$ServiceSet = Win32::OLE->GetObject("winmgmts:{impersonationLevel=impersonate}")->
	ExecQuery("SELECT * FROM Win32_Service WHERE Name='MyService'");};
unless($@)
{
	foreach $Service (in $ServiceSet)
	{
		my $RetVal = $Service->Delete();
		if ($RetVal == 0)  
		{
			print "Service deleted \n"; 
		}
		else  
		{
			print "Delete failed: %d", $RetVal;
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}
