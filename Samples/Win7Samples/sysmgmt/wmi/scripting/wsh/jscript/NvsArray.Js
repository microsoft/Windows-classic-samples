// Copyright (c)  Microsoft Corporation
//***************************************************************************
// 
// WMI Sample Script - Named value set array manipulation (JScript)
//
// This script demonstrates the manipulation of named value sets, in the case 
// that the named value is an array type.
//
//***************************************************************************
try
{
	var Context = new ActiveXObject ("WbemScripting.SWbemNamedValueSet");

	Context.Add ("n1", new Array (1, 20, 3));
	var arrayValue = Context("n1").Value;

	var str = "The initial value of n1 is [1,20,3]: {";

	for (var i = 0; i < arrayValue.length; i++)
	{
		str = str + arrayValue[i];
		if (i < arrayValue.length - 1)
			str = str + ", ";
	}

	str = str + "}"
	WScript.Echo (str);

	WScript.Echo ("");

	// Report the value of an element of the context value
	var v = Context("n1").Value;
	WScript.Echo ("By indirection n1[0] has value [1]:",v[0]);

	// Report the value of a single named value element
	Context("n1").Value[1] = 14;
	WScript.Echo ("After direct assignment n1[1] has value [14]:", 
				(Context("n1").Value)[1]);


	// Set the value of a single named value element
	v[1] = 11;
	Context("n1").Value = v;
	WScript.Echo ("After direct assignment n1[1] has value [11]:", 
				(Context("n1").Value)[1]);

	// Set the value of an entire context value
	Context("n1").Value = new Array (5, 34, 178871);
	WScript.Echo ("After direct array assignment n1[1] has value [34]:",
				(Context("n1").Value)[1]);

	arrayValue = Context("n1").Value;
	str = "After direct assignment the entire value of n1 is [5,34,178871]: {";
	for (var i = 0; i < arrayValue.length; i++)
	{
		str = str + arrayValue[i];
		if (i < arrayValue.length - 1)
			str = str + ", ";
	}

	str = str + "}"
	WScript.Echo (str);

	WScript.Echo ("");

}
catch (e)
{
	WScript.Echo ("Error in script: " + e);
}