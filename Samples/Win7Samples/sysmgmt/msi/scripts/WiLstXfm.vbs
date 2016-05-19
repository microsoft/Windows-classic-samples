' Windows Installer transform viewer for use with Windows Scripting Host
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates the use of the database APIs for viewing transform files
'
Option Explicit

Const iteAddExistingRow      = 1
Const iteDelNonExistingRow   = 2
Const iteAddExistingTable    = 4
Const iteDelNonExistingTable = 8
Const iteUpdNonExistingRow   = 16
Const iteChangeCodePage      = 32
Const iteViewTransform       = 256

Const icdLong       = 0
Const icdShort      = &h400
Const icdObject     = &h800
Const icdString     = &hC00
Const icdNullable   = &h1000
Const icdPrimaryKey = &h2000
Const icdNoNulls    = &h0000
Const icdPersistent = &h0100
Const icdTemporary  = &h0000

Const idoReadOnly = 0

Dim gErrors, installer, base, database, argCount, arg, argValue
gErrors = iteAddExistingRow + iteDelNonExistingRow + iteAddExistingTable + iteDelNonExistingTable + iteUpdNonExistingRow + iteChangeCodePage
Set database = Nothing

' Check arg count, and display help if no all arguments present
argCount = WScript.Arguments.Count
If (argCount < 2) Then
	WScript.Echo "Windows Installer Transform Viewer for Windows Scripting Host (CScript.exe)" &_
		vbNewLine & " 1st non-numeric argument is path to base database which transforms reference" &_
		vbNewLine & " Subsequent non-numeric arguments are paths to the transforms to be viewed" &_
		vbNewLine & " Numeric argument is optional error suppression flags (default is ignore all)" &_
		vbNewLine & " Arguments are executed left-to-right, as encountered" &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Quit 1
End If

' Cannot run with GUI script host, as listing is performed to standard out
If UCase(Mid(Wscript.FullName, Len(Wscript.Path) + 2, 1)) = "W" Then
	WScript.Echo "Cannot use WScript.exe - must use CScript.exe with this program"
	Wscript.Quit 2
End If

' Create installer object
On Error Resume Next
Set installer = CreateObject("WindowsInstaller.Installer") : CheckError

' Process arguments, opening database and applying transforms
For arg = 0 To argCount - 1
	argValue = WScript.Arguments(arg)
	If IsNumeric(argValue) Then
		gErrors = argValue
	ElseIf database Is Nothing Then
		Set database = installer.OpenDatabase(argValue, idoReadOnly)
	Else
		database.ApplyTransform argValue, iteViewTransform + gErrors
	End If
	CheckError
Next
ListTransform(database)

Function DecodeColDef(colDef)
	Dim def
	Select Case colDef AND (icdShort OR icdObject)
	Case icdLong
		def = "LONG"
	Case icdShort
		def = "SHORT"
	Case icdObject
		def = "OBJECT"
	Case icdString
		def = "CHAR(" & (colDef AND 255) & ")"
	End Select
	If (colDef AND icdNullable)   =  0 Then def = def & " NOT NULL"
	If (colDef AND icdPrimaryKey) <> 0 Then def = def & " PRIMARY KEY"
	DecodeColDef = def
End Function

Sub ListTransform(database)
	Dim view, record, row, column, change
	On Error Resume Next
	Set view = database.OpenView("SELECT * FROM `_TransformView` ORDER BY `Table`, `Row`") : CheckError
	view.Execute : CheckError
	Do
		Set record = view.Fetch : CheckError
		If record Is Nothing Then Exit Do
		change = Empty
		If record.IsNull(3) Then
			row = "<DDL>"
			If NOT record.IsNull(4) Then change = "[" & record.StringData(5) & "]: " & DecodeColDef(record.StringData(4))
		Else
			row = "[" & Join(Split(record.StringData(3), vbTab, -1), ",") & "]"
			If record.StringData(2) <> "INSERT" AND record.StringData(2) <> "DELETE" Then change = "{" & record.StringData(5) & "}->{" & record.StringData(4) & "}"
		End If
		column = record.StringData(1) & " " & record.StringData(2)
		if Len(column) < 24 Then column = column & Space(24 - Len(column))
		WScript.Echo column, row, change
	Loop
End Sub

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
