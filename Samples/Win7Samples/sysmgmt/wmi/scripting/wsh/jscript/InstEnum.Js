// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Instance enumeration (JScript)
//
// This script demonstrates the enumeration of instances of a class.
//
//***************************************************************************
try
{
	var e = new Enumerator (GetObject("winmgmts:").InstancesOf ("CIM_LogicalDisk"));

	for (;!e.atEnd();e.moveNext ())
	{
		var Disk = e.item ();
		WScript.Echo ("Instance:", Disk.Path_.Relpath);
	}
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}

