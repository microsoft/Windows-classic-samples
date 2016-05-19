# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Schema browsing (Perl Script)
#
# This script demonstrates browsing of qualifiers, properties and methods.
#
#***************************************************************************
use strict;
use Win32::OLE;

my $Process;

eval { $Process = 
	Win32::OLE->
	GetObject("winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2:Win32_Process");};

if (!$@ && defined $Process)
{
	printf ("\nClass name is %s\n", $Process->Path_()->{Class});

	my ($PropertySet, $QualifierSet, $MethodsSet);

	# Get the properties
	print "\nProperties:\n";
	eval { $PropertySet = $Process->Properties_; };
	unless ($@)
	{
		foreach (in $PropertySet)
		{
			printf (" %s\n", $_->{Name});
		}
	}

	# Get the qualifiers
	print "\nQualifiers:\n";
	eval { $QualifierSet = $Process->Qualifiers_; };
	unless ($@)
	{
		foreach (in $QualifierSet)
		{
			printf (" %s\n", $_->{Name});
		}
	}

	# Get the methods
	print "\nMethods:\n";
	eval { $MethodsSet = $Process->Methods_; };
	unless ($@)
	{
		foreach (in $MethodsSet)
		{
			printf (" %s\n", $_->{Name});
		}
	}
}
else
{
	print STDERR Win32::OLE->LastError, "\n";
}