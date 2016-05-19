#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Object path manipulation (Perl Script)
#
# This script demonstrates how to build object paths.
#
#***************************************************************************
use strict;
use Win32::OLE;
use constant FALSE=>"false";

my ( $ObjectPath, $Privileges );

eval { $ObjectPath = new Win32::OLE 'WbemScripting.SWbemObjectPath'; };
unless($@)
{
	eval
	{
		$ObjectPath->{Server} = "dingle";
		$ObjectPath->{Namespace} = "root/default/something";
		$ObjectPath->{Class} = "ho";
		$ObjectPath->{Keys}->Add("fred1", 10);
		$ObjectPath->{Keys}->Add("fred2", -34);
		$ObjectPath->{Keys}->Add("fred3", 65234654);
		$ObjectPath->{Keys}->Add("fred4", "Wahaay");
		$ObjectPath->{Keys}->Add("fred5", -786186777);
	};
	unless($@)
	{
		print "\n";
		print "Pass 1:\n";
		print $ObjectPath->{Path}, "\n";
		print $ObjectPath->{DisplayName} , "\n\n";

		$ObjectPath->{Security_}->{ImpersonationLevel} = 3 ;

		print "Pass 2:\n";
		print $ObjectPath->{Path}, "\n";
		print $ObjectPath->{DisplayName} , "\n\n";

		$ObjectPath->{Security_}->{AuthenticationLevel} = 5 ;

		print "Pass 3:\n";
		print $ObjectPath->{Path}, "\n";
		print $ObjectPath->{DisplayName} , "\n\n";

		eval { $Privileges = $ObjectPath->{Security_}->{Privileges}; };
		unless($@)
		{
			$Privileges->Add(8);
			$Privileges->Add(20,FALSE);

			print "Pass 4:\n";
			print $ObjectPath->{Path}, "\n";
			print $ObjectPath->{DisplayName} , "\n\n";

			$ObjectPath->{DisplayName} = "winmgmts:{impersonationLevel=impersonate,authenticationLevel=pktprivacy,(Debug,!IncreaseQuota, CreatePagefile ) }!//fred/root/blah";

			print "Pass 5:\n";
			print $ObjectPath->{Path}, "\n";
			print $ObjectPath->{DisplayName} , "\n";
		}
		else
		{
			print STDERR Win32::OLE->LastError, "\n";
		}
	}
	else
	{
		print STDERR Win32::OLE->LastError, "\n";
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}