#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - REMOTE system reboot (Perl Script)
#
# Invokes the Reboot method of the Win32_OperatingSystem class
# 
# Fill in REMOTE_SYSTEM_NAME with the name of the remote system to reboot
# USERNAME and PASSWORD with the user name and password details.
#
# NOTE:  You must have the RemoteShutdown privilege to successfully invoke
#	   the Reboot method
#
#***************************************************************************
use strict;
use Win32::OLE;

use constant REMOTE_SYSTEM_NAME => "MACHINENAME";
use constant USERNAME => "USER";
use constant PASSWORD => "PASSWORD";
use constant NAMESPACE => "root\\cimv2";
use constant wbemPrivilegeRemoteShutdown => 23;
use constant wbemImpersonationLevelImpersonate => 3;
close(STDERR);
my ($locator, $services, $OpSysSet);
eval {
	 $locator = Win32::OLE->new('WbemScripting.SWbemLocator');
	 $locator->{Security_}->{impersonationlevel} = wbemImpersonationLevelImpersonate;
	 $services = $locator->ConnectServer(REMOTE_SYSTEM_NAME, NAMESPACE, USERNAME, PASSWORD);
	 $services->{Security_}->{Privileges}->Add(wbemPrivilegeRemoteShutdown);
	 $OpSysSet = $services->ExecQuery("SELECT * FROM Win32_OperatingSystem WHERE Primary=true");
	};

if (!$@ && defined $OpSysSet)
{
	foreach my $OpSys (in $OpSysSet)
	{
		$OpSys->Reboot();
	}
}
else
{
	print Win32::OLE->LastError, "\n";
	exit(1);
}