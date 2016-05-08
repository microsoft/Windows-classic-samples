// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Win32_Process enumeration (JScript)
//
// This script demonstrates how to enumerate processes.
//
//***************************************************************************
try
{
	var e = new Enumerator (GetObject("winmgmts:").InstancesOf("Win32_process"));

	for (;!e.atEnd();e.moveNext())
	{
		var p = e.item ();
		WScript.Echo (p.Name, p.Handle);
	}
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}