' Windows Installer utility to report the differences between two databases
' For use with Windows Scripting Host, CScript.exe only, lists to stdout
' Copyright (c) Microsoft Corporation. All rights reserved.
' Simply generates a transform between the databases and then view the transform
'
Option Explicit

Const icdLong       = 0
Const icdShort      = &h400
Const icdObject     = &h800
Const icdString     = &hC00
Const icdNullable   = &h1000
Const icdPrimaryKey = &h2000
Const icdNoNulls    = &h0000
Const icdPersistent = &h0100
Const icdTemporary  = &h0000

Const msiOpenDatabaseModeReadOnly     = 0
Const msiOpenDatabaseModeTransact     = 1
Const msiOpenDatabaseModeCreate       = 3
Const iteViewTransform       = 256

If Wscript.Arguments.Count < 2 Then
	Wscript.Echo "Windows Installer database difference utility" &_
		vbNewLine & " Generates a temporary transform file, then display it" &_
		vbNewLine & " 1st argument is the path to the original installer database" &_
		vbNewLine & " 2nd argument is the path to the updated installer database" &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Quit 1
End If

' Cannot run with GUI script host, as listing is performed to standard out
If UCase(Mid(Wscript.FullName, Len(Wscript.Path) + 2, 1)) = "W" Then
	WScript.Echo "Cannot use WScript.exe - must use CScript.exe with this program"
	Wscript.Quit 2
End If

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

' Create path for temporary transform file
Dim WshShell : Set WshShell = Wscript.CreateObject("Wscript.Shell") : CheckError
Dim tempFilePath:tempFilePath = WshShell.ExpandEnvironmentStrings("%TEMP%") & "\diff.tmp"

' Open databases, generate transform, then list transform
Dim database1 : Set database1 = installer.OpenDatabase(Wscript.Arguments(0), msiOpenDatabaseModeReadOnly) : CheckError
Dim database2 : Set database2 = installer.OpenDatabase(Wscript.Arguments(1), msiOpenDatabaseModeReadOnly) : CheckError
Dim different : different = Database2.GenerateTransform(Database1, tempFilePath) : CheckError
If different Then
	database1.ApplyTransform tempFilePath, iteViewTransform + 0 : CheckError' should not need error suppression flags
	ListTransform database1
End If

' Open summary information streams and compare them
Dim sumInfo1 : Set sumInfo1 = database1.SummaryInformation(0) : CheckError
Dim sumInfo2 : Set sumInfo2 = database2.SummaryInformation(0) : CheckError
Dim iProp, value1, value2
For iProp = 1 to 19              
	value1 = sumInfo1.Property(iProp) : CheckError
	value2 = sumInfo2.Property(iProp) : CheckError
	If value1 <> value2 Then
		Wscript.Echo "\005SummaryInformation   [" & iProp & "] {" & value1 & "}->{" & value2 & "}"
		different = True
	End If
Next
If Not different Then Wscript.Echo "Databases are identical"
Wscript.Quit 0

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
	Set view = database.OpenView("SELECT * FROM `_TransformView` ORDER BY `Table`, `Row`")
	If Err <> 0 Then Wscript.Echo "Transform viewing supported only in builds 4906 and beyond of MSI.DLL" : Wscript.Quit 2
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
