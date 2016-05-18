// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Qualifier array manipulation (JScript)
//
// This script demonstrates the manipulation of qualifiers, in the case 
// that the qualifier has an array value.
//
// Note that array values of qualifiers are always VB-style arrays.
// Therefore we must use the VBArray helper object to transform
// them to a format understandable by JScript.
//
//***************************************************************************
try
{
	var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
	var service = locator.ConnectServer (".", "root/default");
	var Class = service.Get();

	Class.Path_.Class = "ARRAYQUAL00";
	var Qualifier = Class.Qualifiers_.Add ("q", new Array (1, 2, 33));

	var str = "After direct assignment the initial value of q is {";
	var value = new VBArray (Qualifier.Value).toArray ();
	WScript.Echo ("The length of the array is", value.length);
	for (var x=0; x < value.length; x++) {
		if (x != 0) {
			str = str + ", ";
		}
		str = str + value[x];
	}

	str = str + "}";
	WScript.Echo (str);

	// Report the value of an element of the qualifier value
	var v = new VBArray (Qualifier.Value).toArray ();
	WScript.Echo ("By indirection the third element of q has value:",v[2]);

	// Report the value through the collection
	var w = new VBArray (Class.Qualifiers_("q").Value).toArray ();
	WScript.Echo ("By direct access the first element of q has value:", w[2]);

	// Set the value of a single qualifier value element
	var p = new VBArray (Qualifier.Value).toArray ();
	p[1] = 345;
	Qualifier.Value = p;
	var value = new VBArray (Qualifier.Value).toArray ();
	WScript.Echo ("After indirect assignment the second element of q has value:", value[1]);
}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}

	// Set the value of an entire qualifier value
	Qualifier.Value = new Array (5, 34, 178871);
	var str = "After direct assignment the initial value of q is {";
	var value = new VBArray (Class.Qualifiers_("q").Value).toArray ();
	WScript.Echo ("The length of the array is", value.length);
	for (var x=0; x < value.length; x++) {
		if (x != 0) {
			str = str + ", ";
		}
		str = str + value[x];
	}

	str = str + "}";
	WScript.Echo (str);

	// Set the value of a property array element using the "dot" notation
	var Qualifier = Class.Qualifiers_("q");
	Qualifier.Value(2) = 8889;
	WScript.Echo ("By direct access the second element of p1 has been set to:", (new VBArray(Class.Qualifiers_("q").Value).toArray())[2]);

	Class.Put_ ();
