#!perl -w
# Copyright (c)  Microsoft Corporation
#***************************************************************************
# 
# WMI Sample Script - Subclass creation (Perl Script)
#
# This script demonstrates how to create a subclass.
#
#'***************************************************************************
use strict;
use Win32::OLE;

my ($tService, $tClass, $tSubClass);
close(STDERR);
MAIN:
{
	eval 
	{ 	
		  $tService = Win32::OLE->GetObject("winmgmts://./root/default");
	  	  $tClass = $tService->Get(); 
    };
	unless($@)
	{
		$tClass->Path_->{Class} = "MyBaseClass";
		eval { $tClass->Put_(); };
		unless($@)
		{
			eval { $tSubClass = $tService->Get("MyBaseClass")->SpawnDerivedClass_(); };
			unless ($@)
			{
				$tSubClass->Path_()->{Class} = "SPAWNCLASSTEST00";
				eval { $tSubClass->Put_(); };
				last MAIN if ($@);
			}
		}
		last MAIN if ($@);		
	}
	last MAIN if ($@);
	exit(0);
}
print Win32::OLE->LastError, "\n";
exit(1);




