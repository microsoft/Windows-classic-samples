// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Named value set manipulation (JScript)
//
// This script demonstrates the manipulation of named value sets.
//
//***************************************************************************
try
{
	var context = WScript.CreateObject ("WbemScripting.SWbemNamedValueSet");

	context.Add ("J", null);
	context.Add ("fred", 23);
	context("fred").Value = 12;
	context.Add ("Hah", true);
	context.Add ("Whoah", "Freddy the frog");

	// A string array
	var bam = new Array ("whoops", "a", "daisy");
	context.Add ("Bam", bam);

	WScript.Echo ("There are", context.Count , "elements in the context");

	context.Remove("hah");

	WScript.Echo ("There are", context.Count , "elements in the context");

	context.Remove("Hah");

	WScript.Echo ("There are", context.Count , "elements in the context");

	var bam = context("Bam").Value;

	WScript.Echo ("");
	WScript.Echo ("Here are the names:");
	WScript.Echo ("==================");

	for (var x = 0; x < bam.length; x++) {
		WScript.Echo (bam[x]);
	}

	WScript.Echo ("");
	WScript.Echo ("Here are the names & values:");
	WScript.Echo ("===========================");

	// Use the Enumerator helper to manipulate collections
	e = new Enumerator (context);
	s = "";

	for (;!e.atEnd();e.moveNext ())
	{
		var y = e.item ();
		s += y.Name;
		s += "=";
		if (null != y.Value)
			s += y;
		s += "\n";
	}

	WScript.Echo (s);
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}
