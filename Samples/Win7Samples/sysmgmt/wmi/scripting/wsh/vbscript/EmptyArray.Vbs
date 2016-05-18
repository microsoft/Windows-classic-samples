' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Empty array manipulation (VBScript)
'
' This script demonstrates the manipulation of empty arrays on properties, 
' qualifiers and named value sets.
'
'***************************************************************************
On Error Resume Next
Set Service = GetObject("winmgmts:root/default")
Set MyClass = Service.Get
MyClass.Path_.Class = "EMPTYARRAYTEST00"

'*************************
'CASE 1: Property values
'*************************
Set Prop = MyClass.Properties_.Add ("p1", 2, true)

Prop.Value = Array

value = MyClass.Properties_("p1").Value

WScript.Echo "Array upper bound for property value is [-1]:", UBound(value)
WScript.Echo "Base CIM property type is [2]", Prop.CIMType
WScript.Echo

if Err <> 0 Then
	WScript.Echo Err.Number, Err.Description, Err.Source
	Err.Clear
End if

'*************************
'CASE 2: Qualifier values
'*************************
MyClass.Qualifiers_.Add "q1", Array
value = MyClass.Qualifiers_("q1").Value

WScript.Echo "Array upper bound for qualifier value is [-1]:", UBound(value)
WScript.Echo 
MyClass.Put_

'Now read them back and assign "real values"
Set MyClass = Service.Get("EMPTYARRAYTEST00")
MyClass.Properties_("p1").Value = Array (12, 34, 56)
value = MyClass.Properties_("p1").Value
WScript.Echo "Array upper bound for property value is [2]:", UBound(value)
WScript.Echo "Base CIM property type is [2]", Prop.CIMType
WScript.Echo
MyClass.Properties_("p1").Value = Array
value = MyClass.Properties_("p1").Value
WScript.Echo "Array upper bound for property value is [-1]:", UBound(value)
WScript.Echo "Base CIM property type is [2]", Prop.CIMType
WScript.Echo

MyClass.Qualifiers_("q1").Value = Array ("Providence", "Melo")
value = MyClass.Qualifiers_("q1").Value
WScript.Echo "Array upper bound for qualifier value is [1]:", UBound(value)
MyClass.Qualifiers_("q1").Value = Array
value = MyClass.Qualifiers_("q1").Value
WScript.Echo "Array upper bound for qualifier value is [-1]:", UBound(value)
WScript.Echo
MyClass.Put_

'*************************
'CASE 3:Named Values
'*************************
Set NValueSet = CreateObject("WbemScripting.SWbemNamedValueSet")
Set NValue = NValueSet.Add ("Muriel", Array)
value = NValueSet("Muriel").Value
WScript.Echo "Array upper bound for context value is [-1]:", UBound(value)

if Err <> 0 Then
	WScript.Echo Err.Number, Err.Description, Err.Source
	Err.Clear
End if

