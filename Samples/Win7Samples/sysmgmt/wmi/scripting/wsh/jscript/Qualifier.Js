// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Qualifier manipulation (JScript)
//
// This script demonstrates the manipulation of qualifiers.
//
//***************************************************************************
try
{
	var Service = GetObject("winmgmts:root/default");

	var Class = Service.Get();
	Class.Path_.Class = "Qualtest00";

	var Qualifiers = Class.Qualifiers_;

	Qualifiers.Add ("qbool", true, true, true, false);
	Qualifiers.Add ("qsint32", 345);
	Qualifiers.Add ("qreal64", -345.675);
	Qualifiers.Add ("qstring", "freddy the frog");
	Qualifiers.Add ("qstring2", "freddy the froggier", false);
	Qualifiers.Add ("qstring3", "freddy the froggiest", false, false);
	Qualifiers.Add ("qstring4", "freddy the froggiesst", true, false);
	Qualifiers.Add ("qstring5", "wibble", true, true, false);
	Qualifiers.Add ("aqbool", new Array(true, false, true));
	Qualifiers.Add ("aqsint32", new Array (10, -12));
	Qualifiers.Add ("aqreal64", new Array(-2.3, 2.456, 12.356567897));
	Qualifiers.Add ("aqstring", new Array("lahdi", "dah", "wibble"));

	Qualifiers("qsint32").Value = 7677;

	WScript.Echo ("There are", Qualifiers.Count, "Qualifiers in the collection");

	for (var e = new Enumerator (Qualifiers);!e.atEnd();e.moveNext ())
	{
		var qualifier = e.item ();
		var value = qualifier.Value;

		if ((typeof(value) == "number") ||
			(typeof(value) == "string") ||
 			(typeof(value) == "boolean"))
		{
			WScript.Echo (qualifier.Name, "=", qualifier);
		}
		else
		{
			// Assume it's an array
			var arrayValue = new VBArray (value).toArray();
			var str = qualifier.Name + " = [";
			for (i = 0; i < arrayValue.length; i++) {
				if (i > 0) {
					str = str + ", ";
				}
				str = str + arrayValue[i];
			}
			str = str + "]";
			WScript.Echo (str);
		}
	}

	Class.Put_ ();
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}
