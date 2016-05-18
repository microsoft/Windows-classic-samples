// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Empty array manipulation (JScript)
//
// This script demonstrates the manipulation of empty arrays on properties, 
// qualifiers and named value sets.
//
//***************************************************************************
try
{
	var Service = GetObject("winmgmts:root/default");
	var MyClass = Service.Get();
	MyClass.Path_.Class = "EMPTYARRAYTEST00";

	//*************************
	//CASE 1: Property values
	//*************************
	var Property = MyClass.Properties_.Add ("p1", 2, true);
	Property.Value = new Array ();

	// Note that array values of properties are always VB-style arrays.
	// Therefore we must use the VBArray helper object to transform
	// them to a format understandable by JScript.
	var arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();

	WScript.Echo ("Array length for property value is [0]:", arrayValue.length);
	WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
	WScript.Echo ();

	//*************************
	//CASE 2: Qualifier values
	//*************************
	MyClass.Qualifiers_.Add ("q1", Array);

	// Note that array values of qualifiers are always VB-style arrays.
	// Therefore we must use the VBArray helper object to transform
	// them to a format understandable by JScript.
	arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();

	WScript.Echo ("Array length for qualifier value is [0]:", arrayValue.length);
	WScript.Echo (); 

	MyClass.Put_();

	//Now read them back and assign "real values"
	var MyClass = Service.Get("EMPTYARRAYTEST00");
	MyClass.Properties_("p1").Value = new Array (12, 34, 56);
	arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();
	WScript.Echo ("Array length for property value is [3]:", arrayValue.length);
	WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
	WScript.Echo ();

	MyClass.Properties_("p1").Value = new Array ();
	var arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();
	WScript.Echo ("Array length for property value is [0]:", arrayValue.length);
	WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
	WScript.Echo ();

	MyClass.Qualifiers_("q1").Value = new Array ("Hello", "World");
	arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();
	WScript.Echo ("Array length for qualifier value is [2]:", arrayValue.length);
	WScript.Echo (); 

	MyClass.Qualifiers_("q1").Value = new Array ();
	arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();
	WScript.Echo ("Array length for qualifier value is [0]:", arrayValue.length);
	WScript.Echo (); 

	MyClass.Put_ ();

	//*************************
	//CASE 3:Named Values
	//*************************
	var NValueSet = new ActiveXObject("WbemScripting.SWbemNamedValueSet");

	// Note that array values of named values are always "native"
	// (i.e. JScript or VBScript style, depending on the environment) arrays.
	// Therefore we should not use the VBArray helper object here.
	var NValue = NValueSet.Add ("Foo", new Array ());
	var arrayValue = NValueSet("Foo").Value;
	WScript.Echo ("Array length for named value is [0]:", arrayValue.length);

}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}