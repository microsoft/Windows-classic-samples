// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Class enumeration (JScript)
//
// This script demonstrates the enumeration of sublcasses of a class.
//
//***************************************************************************
try
{
	var e = new Enumerator (GetObject("winmgmts:").SubclassesOf ("CIM_LogicalElement"));
	for (;!e.atEnd();e.moveNext ())
	{
		var Subclass = e.item ();
		WScript.Echo ("Subclass name:", Subclass.Path_.Class);
	}
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}

