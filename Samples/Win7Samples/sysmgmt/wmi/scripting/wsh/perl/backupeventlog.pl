#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
# 
# WMI Sample Script -  Backup event log (Perl Script)
#
# This script demonstrates how to backup the entries from the Application event log file
# on the local machine from instances of Win32_NT
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support event logs
#***************************************************************************
use strict;
use Win32::OLE;

close(STDERR);

my ($LogFile, $LogFileSet);
eval { $LogFileSet = Win32::OLE->GetObject("winmgmts:{(Backup,Security)}!\\\\.\\root\\cimv2")->
	ExecQuery("SELECT * FROM Win32_NTEventLogFile WHERE LogfileName='Application'");};

if (!$@ && defined $LogFileSet) 
{
	foreach $LogFile (in $LogFileSet)
	{
		my $RetVal = $LogFile->BackupEventlog("c:\\BACKUP.LOG");
		if (defined $RetVal && $RetVal == 0)
		{
			print "\nLog Backed Up\n";
		}
		else
		{
			print Win32::OLE->LastError, "\n";
		}
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}
