rem THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
rem ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
rem THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
rem PARTICULAR PURPOSE.
rem 
rem Copyright (c) Microsoft Corporation. All rights reserved

rem Windows SDK sample that demonstrates how to query Windows Search from script
rem using ADO. To run this script use the following command line: cscript QueryEverything.vbs

On Error Resume Next

rem create the ADO objects
Set objConnection = CreateObject("ADODB.Connection")
Set objRecordSet = CreateObject("ADODB.Recordset")

rem This is the Windows Search connection string to use
objConnection.Open "Provider=Search.CollatorDSO;Extended Properties='Application=Windows';"

rem SQL SELECT statement specifies what properties to return, you can add more if you want
rem     FROM - use SystemIndex for a local query or MACHINENAME.SystemIndex for remote
rem     WHERE - specify restrictions including SCOPE and other conditions that must be true

rem This is a very simple query over the whole index. To add scope restriction append "WHERE SCOPE='file:c:/users'" to the query string.
objRecordSet.Open "SELECT System.ItemName, System.ItemTypeText, System.Size FROM SystemIndex", objConnection

objRecordSet.MoveFirst
Do Until objRecordset.EOF
    rem Access the column values that were specified in the SELECT statement here
    Wscript.Echo objRecordset.Fields.Item("System.ItemName")
    Wscript.Echo objRecordset.Fields.Item("System.ItemTypeText")
    Wscript.Echo objRecordset.Fields.Item("System.Size")
    Wscript.Echo 
    objRecordset.MoveNext
Loop

objRecordset.Close
Set objRecordset = Nothing
objConnection.Close
Set objConnection = Nothing