#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - NT Event Logger (Perl Script)
#
# This script demonstrates how to display NT events using a notification
# query.
#
#***************************************************************************
use strict;
use Win32::OLE;

close(STDERR);

my ( $locator, $events ,$NTEvent );

eval { $locator = new Win32::OLE 'WbemScripting.SWbemLocator'; };
if(!$@ && defined $locator)
{
	# Access to the NT event log requires the security privilege
	$locator->{Security_}->{Privileges}->AddAsString( "SeSecurityPrivilege" );

	eval { $events = $locator->ConnectServer()->ExecNotificationQuery
		("select * from __instancecreationevent where targetinstance isa 'Win32_NTLogEvent'"); };
	if(!$@ && defined $events)
	{
		print "\nWaiting for NT events...\n\n";

		# Note this next call will wait indefinitely - a timeout can be specified 
		while (1)
		{
			$NTEvent = $events->NextEvent();
			if ( $@ ) 
			{
				print Win32::OLE->LastError, "\n";
				last;
			}
			else
			{
				if ( defined( $NTEvent->{TargetInstance}->{Message} ) )
				{
					print $NTEvent->{TargetInstance}->{Message};
				}
				else
				{
					print "Event received, but it did not contain a message.\n";
				}
			}
		}

		print "finished\n";

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