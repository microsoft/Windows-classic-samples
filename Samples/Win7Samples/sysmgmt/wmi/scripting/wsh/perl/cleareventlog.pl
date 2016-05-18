#! perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
#
# WMI Sample Script -  Clear event log (Perl Script)
#
# This script demonstrates how to clear the entries from the System event log file
# on the local machine from instances of Win32_NTEventLogFile.
#
# NOTE:  A backup file can be specified to the ClearEventlog() method if a backup of
# the data is desired before clearing the log.
#
# NOTE:  This script only applies to NT-based systems since Win9x does not support event logs
#***************************************************************************
use strict;
use Win32::OLE;

close(STDERR);

my ($LogFile, $LogFileSet);
eval {$LogFileSet = Win32::OLE->GetObject("winmgmts:{(Backup,Security)}")->
	ExecQuery("SELECT * FROM Win32_NTEventLogFile WHERE LogfileName='System'");};
if(!$@ && defined $LogFileSet)
{
	foreach $LogFile (in $LogFileSet)
	{
		my $RetVal = $LogFile->ClearEventlog();
		if ($RetVal == 0)
		{
			print "\nLog Cleared\n";
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
