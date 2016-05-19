#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation 
#
# WMI Sample Script -  Privilege manipulation (Perl Script) 
#
# This script demonstrates how to add privileges using the 
# SWbemPrivilegeSet object.
#
#***************************************************************************
use strict;
use Win32::OLE;

close(STDERR);

my ($locator, $Privilege);
my $wbemPrivilegeSecurity = 8;
my $wbemPrivilegeDebug = 20;

eval { $locator = new Win32::OLE 'WbemScripting.SWbemLocator';};

if (!$@ && defined $locator)
{
	# Add a single privilege using SWbemPrivilegeSet.Add
	$locator->{Security_}->{Privileges}->Add($wbemPrivilegeSecurity);
	$Privilege = $locator->{Security_}->Privileges($wbemPrivilegeSecurity);
	print "\n", $Privilege->{Name}, "\n\n";

	# Attempt to add an illegal privilege using SWbemPrivilegeSet.Add
	eval { $locator->{Security_}->{Privileges}->Add(6535); };
	print Win32::OLE->LastError, "\n" if ($@ || Win32::OLE->LastError);

	$locator->{Security_}->{Privileges}->Add($wbemPrivilegeDebug); 
	$locator->{Security_}->Privileges($wbemPrivilegeDebug)->{IsEnabled} = 0;

	# Add a single privilege using SWbemPrivilegeSet.AddAsString
	$Privilege = $locator->{Security_}->{Privileges}->AddAsString ("SeChangeNotifyPrivilege");
	print "\n", $Privilege->{Name}, "\n\n";

	# Attempt to add an illegal privilege using SWbemPrivilegeSet.AddAsString
	eval {$locator->{Security_}->{Privileges}->AddAsString ("SeChungeNotifyPrivilege"); };
	print Win32::OLE->LastError, "\n" if ($@ || Win32::OLE->LastError);
	print "\n";

	foreach $Privilege (in {$locator->{Security_}->{Privileges}})
	{
		printf "[%s] %d %s %d \n" , $Privilege->{DisplayName}, $Privilege->{Identifier}, $Privilege->{Name}, $Privilege->{IsEnabled};
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}
