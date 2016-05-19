#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - System reboot (Perl Script)
#
# Invokes the Reboot method of the Win32_OperatingSystem class
#
# NOTE:  You must have the Shutdown privilege to successfully invoke the 
#		 Reboot method
#
#***************************************************************************
use Win32::OLE;
use strict;
my $OpSysSet;
eval { $OpSysSet = Win32::OLE->GetObject("winmgmts:{(Shutdown)}//./root/cimv2")->
	ExecQuery("SELECT * FROM Win32_OperatingSystem WHERE Primary=true"); };

if (!$@ && defined $OpSysSet)
{
	close(STDERR);
	foreach my $OpSys (in $OpSysSet)
	{
		my $RetVal = $OpSys->Reboot(); 
		if (!defined $RetVal || $RetVal != 0)
		{
			print Win32::OLE->LastError, "\n";	
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}