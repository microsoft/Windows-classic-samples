// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Method set manipulation (JScript)
//
// This script demonstrates the manipulation of method sets.
//
//***************************************************************************
try
{
	var MyClass = GetObject("winmgmts:win32_service");

	// Demonstrate the collection properties of WbemMethodSet
	var e = new Enumerator (MyClass.Methods_);

	for (;!e.atEnd();e.moveNext ())
	{
		var Method = e.item ();

		WScript.Echo ("***************************");
		WScript.Echo ("METHOD:", Method.Name, "from class", Method.Origin);
		WScript.Echo ();

		WScript.Echo (" Qualifiers:");
		var eQ = new Enumerator (Method.Qualifiers_);
		for (;!eQ.atEnd();eQ.moveNext ())
		{
			var Qualifier = eQ.item ();

			if (typeof (Qualifier.Value) == "unknown")
			{
				var a = new VBArray (Qualifier.Value);
				var b = a.toArray ();
				var valStr = "   " + Qualifier.Name + " = {";

				for (j = 0; j < b.length; j++)
				{
					if (j != 0)
						valStr = valStr + ", ";

					valStr = valStr + b[j];
				}

				valStr = valStr + "}";
				WScript.Echo (valStr);
			}
			else
			{
				WScript.Echo ("  ", Qualifier.Name, "=", Qualifier.Value);
			}
		}

		WScript.Echo ();
		WScript.Echo (" In Parameters:");
		var inParams = Method.InParameters;

		if (inParams != null)
		{
			var eP = new Enumerator (inParams.Properties_);

			for (;!eP.atEnd();eP.moveNext ())
			{
				var InParameter = eP.item ();
				WScript.Echo ("  ", InParameter.Name, "<", InParameter.CIMType, ">");
			}
		}

		WScript.Echo ();
		WScript.Echo (" Out Parameters");
		var outParams = Method.OutParameters;

		if (outParams != null)
		{	
			var eO = new Enumerator (outParams.Properties_);
			
			for (;!eO.atEnd();eO.moveNext ())
			{
				var OutParameter = eO.item ();
				WScript.Echo ("  ", OutParameter.Name, "<", OutParameter.CIMType, ">");
			}
		}

		WScript.Echo ();
		WScript.Echo ();
	}

	// Item and Count properties of IWbemMethodSet
	WScript.Echo (MyClass.Methods_("StartService").Name);
	WScript.Echo (MyClass.Methods_.Count);
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}