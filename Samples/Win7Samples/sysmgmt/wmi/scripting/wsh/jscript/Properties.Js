// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Property enumeration (JScript)
//
// This script demonstrates the manipulation of the Properties collection
// of SWbemObject.
//
//***************************************************************************
try
{
	var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
	var services = locator.ConnectServer (".", "root/cimv2");
	var classa = services.Get ("Win32_baseservice");

	var props = classa.Properties_;
	WScript.Echo ("There are", props.Count, "properties");
	var e = new Enumerator (props);


	for (;!e.atEnd();e.moveNext())
	{
		p = e.item ();

		if (p.Value == null)
			WScript.Echo (p.Name, p.Origin, p.IsLocal, p.IsArray);
		else
			WScript.Echo (p.Name, "=", p, p.Origin, p.IsLocal, p.IsArray);

		var eq = new Enumerator (p.Qualifiers_);

		WScript.Echo ("\nQualifiers\n");
		for (;!eq.atEnd();eq.moveNext())
		{
			q = eq.item ();
			
			WScript.Echo (q.Name, "[", typeof(q.Value), "]");
			if (typeof(q.Value) != "unknown")
				WScript.Echo (q.Name, "=", q); 
			else
			{
				var arrayval = new VBArray (q.Value).toArray();
				WScript.Echo (arrayval.length);
				for (i = 0; i < arrayval.length; i++)
					WScript.Echo (arrayval[i]);
			}
		}
	}
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}



