// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Win32_ComputerSystem dump (JScript)
//
// This script demonstrates how to dump properties from instances of
// Win32_ComputerSystem.
//
//***************************************************************************

try
{
	var service = GetObject('winmgmts:')
	var Systems = new Enumerator (service.InstancesOf("Win32_ComputerSystem") );

	for (;!Systems.atEnd();Systems.moveNext())
	{
		var System = Systems.item();
		WScript.Echo (System.Caption);
		WScript.Echo (System.PrimaryOwnerName);
		WScript.Echo (System.Domain);
		WScript.Echo (System.SystemType);
	}
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}
	
