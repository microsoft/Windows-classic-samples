#!perl -w
#***************************************************************************
# Copyright (c)  Microsoft Corporation
#
# This sample illustrates how to retrieve registry data from WMI.  Note that the accompanying MOF
# (registry.mof) must be compiled and loaded for this sample to run correctly.
#
#***************************************************************************

use strict;
use Win32::OLE;

close(STDERR);

my ($transport, $transportSet);  
eval {$transportSet = Win32::OLE->GetObject("winmgmts:root/registryScriptExample")->
		InstancesOf ("RegTrans") };
unless ($@) 
{
	foreach $transport (in $transportSet)
	{
		printf "\nTransport %s has name [%s] and Enabled=%d\n", $transport->{TransportsGUID}, $transport->{Name}, $transport->{Enabled};
	}
}
else
{
	print Win32::OLE->LastError, "\n";
}
