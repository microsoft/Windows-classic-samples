' Windows Installer database utility to merge data from another database              
' For use with Windows Scripting Host, CScript.exe or WScript.exe
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates the use of the Database.Merge method and MsiDatabaseMerge API
'
Option Explicit

Const msiOpenDatabaseModeReadOnly     = 0
Const msiOpenDatabaseModeTransact     = 1
Const msiOpenDatabaseModeCreate       = 3
Const ForAppending = 8
Const ForReading = 1
Const ForWriting = 2
Const TristateTrue = -1

Dim argCount:argCount = Wscript.Arguments.Count
Dim iArg:iArg = 0
If (argCount < 2) Then
	Wscript.Echo "Windows Installer database merge utility" &_
		vbNewLine & " 1st argument is the path to MSI database (installer package)" &_
		vbNewLine & " 2nd argument is the path to database containing data to merge" &_
		vbNewLine & " 3rd argument is the optional table to contain the merge errors" &_
		vbNewLine & " If 3rd argument is not present, the table _MergeErrors is used" &_
		vbNewLine & "  and that table will be dropped after displaying its contents." &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Quit 1
End If

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

' Open databases and merge data
Dim database1 : Set database1 = installer.OpenDatabase(WScript.Arguments(0), msiOpenDatabaseModeTransact) : CheckError
Dim database2 : Set database2 = installer.OpenDatabase(WScript.Arguments(1), msiOpenDatabaseModeReadOnly) : CheckError
Dim errorTable : errorTable = "_MergeErrors"
If argCount >= 3 Then errorTable = WScript.Arguments(2)
Dim hasConflicts:hasConflicts = database1.Merge(database2, errorTable) 'Old code returns void value, new returns boolean
If hasConflicts <> True Then hasConflicts = CheckError 'Temp for old Merge function that returns void
If hasConflicts <> 0 Then
	Dim message, line, view, record
	Set view = database1.OpenView("Select * FROM `" & errorTable & "`") : CheckError
	view.Execute
	Do
		Set record = view.Fetch
		If record Is Nothing Then Exit Do
		line = record.StringData(1) & " table has " & record.IntegerData(2) & " conflicts"
		If message = Empty Then message = line Else message = message & vbNewLine & line
	Loop
	Set view = Nothing
	Wscript.Echo message
End If
If argCount < 3 And hasConflicts Then database1.OpenView("DROP TABLE `" & errorTable & "`").Execute : CheckError
database1.Commit : CheckError
Quit 0

Function CheckError
	Dim message, errRec
	CheckError = 0
	If Err = 0 Then Exit Function
	message = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then message = message & vbNewLine & errRec.FormatText : CheckError = errRec.IntegerData(1)
	End If
	If CheckError = 2268 Then Err.Clear : Exit Function
	Wscript.Echo message
	Wscript.Quit 2
End Function
