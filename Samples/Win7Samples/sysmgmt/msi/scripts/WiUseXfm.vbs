' Windows Installer utility to applay a transform to an installer database
' For use with Windows Scripting Host, CScript.exe or WScript.exe
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates use of Database.ApplyTransform and MsiDatabaseApplyTransform
'
Option Explicit

' Error conditions that may be suppressed when applying transforms
Const msiTransformErrorAddExistingRow         = 1 'Adding a row that already exists. 
Const msiTransformErrorDeleteNonExistingRow   = 2 'Deleting a row that doesn't exist. 
Const msiTransformErrorAddExistingTable       = 4 'Adding a table that already exists. 
Const msiTransformErrorDeleteNonExistingTable = 8 'Deleting a table that doesn't exist. 
Const msiTransformErrorUpdateNonExistingRow  = 16 'Updating a row that doesn't exist. 
Const msiTransformErrorChangeCodePage       = 256 'Transform and database code pages do not match 

Const msiOpenDatabaseModeReadOnly     = 0
Const msiOpenDatabaseModeTransact     = 1
Const msiOpenDatabaseModeCreate       = 3

If (Wscript.Arguments.Count < 2) Then
	Wscript.Echo "Windows Installer database tranform application utility" &_
		vbNewLine & " 1st argument is the path to an installer database" &_
		vbNewLine & " 2nd argument is the path to the transform file to apply" &_
		vbNewLine & " 3rd argument is optional set of error conditions to suppress:" &_
		vbNewLine & "     1 = adding a row that already exists" &_
		vbNewLine & "     2 = deleting a row that doesn't exist" &_
		vbNewLine & "     4 = adding a table that already exists" &_
		vbNewLine & "     8 = deleting a table that doesn't exist" &_
		vbNewLine & "    16 = updating a row that doesn't exist" &_
		vbNewLine & "   256 = mismatch of database and transform codepages" &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Quit 1
End If

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

' Open database and apply transform
Dim database : Set database = installer.OpenDatabase(Wscript.Arguments(0), msiOpenDatabaseModeTransact) : CheckError
Dim errorConditions:errorConditions = 0
If Wscript.Arguments.Count >= 3 Then errorConditions = CLng(Wscript.Arguments(2))
Database.ApplyTransform Wscript.Arguments(1), errorConditions : CheckError
Database.Commit : CheckError

Sub CheckError
	Dim message, errRec
	If Err = 0 Then Exit Sub
	message = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then message = message & vbNewLine & errRec.FormatText
	End If
	Wscript.Echo message
	Wscript.Quit 2
End Sub
