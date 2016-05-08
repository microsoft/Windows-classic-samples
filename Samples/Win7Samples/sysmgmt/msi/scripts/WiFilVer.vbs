' Windows Installer utility to report or update file versions, sizes, languages
' For use with Windows Scripting Host, CScript.exe or WScript.exe
' Copyright (c) Microsoft Corporation. All rights reserved.
' Demonstrates the access to install engine and actions
'
Option Explicit

' FileSystemObject.CreateTextFile and FileSystemObject.OpenTextFile
Const OpenAsASCII   = 0 
Const OpenAsUnicode = -1

' FileSystemObject.CreateTextFile
Const OverwriteIfExist = -1
Const FailIfExist      = 0

' FileSystemObject.OpenTextFile
Const OpenAsDefault    = -2
Const CreateIfNotExist = -1
Const FailIfNotExist   = 0
Const ForReading = 1
Const ForWriting = 2
Const ForAppending = 8

Const msiOpenDatabaseModeReadOnly = 0
Const msiOpenDatabaseModeTransact = 1

Const msiViewModifyInsert         = 1
Const msiViewModifyUpdate         = 2
Const msiViewModifyAssign         = 3
Const msiViewModifyReplace        = 4
Const msiViewModifyDelete         = 6

Const msiUILevelNone = 2

Const msiRunModeSourceShortNames = 9

Const msidbFileAttributesNoncompressed = &h00002000

Dim argCount:argCount = Wscript.Arguments.Count
Dim iArg:iArg = 0
If argCount > 0 Then If InStr(1, Wscript.Arguments(0), "?", vbTextCompare) > 0 Then argCount = 0
If (argCount < 1) Then
	Wscript.Echo "Windows Installer utility to updata File table sizes and versions" &_
		vbNewLine & " The 1st argument is the path to MSI database, at the source file root" &_
		vbNewLine & " The 2nd argument can optionally specify separate source location from the MSI" &_
		vbNewLine & " The following options may be specified at any point on the command line" &_
		vbNewLine & "  /U to update the MSI database with the file sizes, versions, and languages" &_
		vbNewLine & "  /H to populate the MsiFileHash table (and create if it doesn't exist)" &_
		vbNewLine & " Notes:" &_
		vbNewLine & "  If source type set to compressed, all files will be opened at the root" &_
		vbNewLine & "  Using CSCRIPT.EXE without the /U option, the file info will be displayed" &_
		vbNewLine & "  Using the /H option requires Windows Installer version 2.0 or greater" &_
		vbNewLine & "  Using the /H option also requires the /U option" &_
		vbNewLine &_
		vbNewLine & "Copyright (C) Microsoft Corporation.  All rights reserved."
	Wscript.Quit 1
End If

' Get argument values, processing any option flags
Dim updateMsi    : updateMsi    = False
Dim populateHash : populateHash = False
Dim sequenceFile : sequenceFile = False
Dim databasePath : databasePath = NextArgument
Dim sourceFolder : sourceFolder = NextArgument
If Not IsEmpty(NextArgument) Then Fail "More than 2 arguments supplied" ' process any trailing options
If Not IsEmpty(sourceFolder) And Right(sourceFolder, 1) <> "\" Then sourceFolder = sourceFolder & "\"
Dim console : If UCase(Mid(Wscript.FullName, Len(Wscript.Path) + 2, 1)) = "C" Then console = True

' Connect to Windows Installer object
On Error Resume Next
Dim installer : Set installer = Nothing
Set installer = Wscript.CreateObject("WindowsInstaller.Installer") : CheckError

Dim errMsg

' Check Installer version to see if MsiFileHash table population is supported
Dim supportHash : supportHash = False
Dim verInstaller : verInstaller = installer.Version
If CInt(Left(verInstaller, 1)) >= 2 Then supportHash = True
If populateHash And NOT supportHash Then
	errMsg = "The version of Windows Installer on the machine does not support populating the MsiFileHash table."
	errMsg = errMsg & " Windows Installer version 2.0 is the mininum required version. The version on the machine is " & verInstaller & vbNewLine
	Fail errMsg
End If

' Check if multiple language package, and force use of primary language
REM	Set sumInfo = database.SummaryInformation(3) : CheckError

' Open database
Dim database, openMode, view, record, updateMode, sumInfo
If updateMsi Then openMode = msiOpenDatabaseModeTransact Else openMode = msiOpenDatabaseModeReadOnly
Set database = installer.OpenDatabase(databasePath, openMode) : CheckError

' Create MsiFileHash table if we will be populating it and it is not already present
Dim hashView, iTableStat, fileHash, hashUpdateRec
iTableStat = Database.TablePersistent("MsiFileHash")
If populateHash Then
	If NOT updateMsi Then
		errMsg = "Populating the MsiFileHash table requires that the database be open for writing. Please include the /U option"
		Fail errMsg		
	End If

	If iTableStat <> 1 Then
		Set hashView = database.OpenView("CREATE TABLE `MsiFileHash` ( `File_` CHAR(72) NOT NULL, `Options` INTEGER NOT NULL, `HashPart1` LONG NOT NULL, `HashPart2` LONG NOT NULL, `HashPart3` LONG NOT NULL, `HashPart4` LONG NOT NULL PRIMARY KEY `File_` )") : CheckError
		hashView.Execute : CheckError
	End If

	Set hashView = database.OpenView("SELECT `File_`, `Options`, `HashPart1`, `HashPart2`, `HashPart3`, `HashPart4` FROM `MsiFileHash`") : CheckError
	hashView.Execute : CheckError

	Set hashUpdateRec = installer.CreateRecord(6)
End If

' Create an install session and execute actions in order to perform directory resolution
installer.UILevel = msiUILevelNone
Dim session : Set session = installer.OpenPackage(database,1) : If Err <> 0 Then Fail "Database: " & databasePath & ". Invalid installer package format"
Dim shortNames : shortNames = session.Mode(msiRunModeSourceShortNames) : CheckError
If Not IsEmpty(sourceFolder) Then session.Property("OriginalDatabase") = sourceFolder : CheckError
Dim stat : stat = session.DoAction("CostInitialize") : CheckError
If stat <> 1 Then Fail "CostInitialize failed, returned " & stat

' Join File table to Component table in order to find directories
Dim orderBy : If sequenceFile Then orderBy = "Directory_" Else orderBy = "Sequence"
Set view = database.OpenView("SELECT File,FileName,Directory_,FileSize,Version,Language FROM File,Component WHERE Component_=Component ORDER BY " & orderBy) : CheckError
view.Execute : CheckError

' Create view on File table to check for companion file version syntax so that we don't overwrite them
Dim companionView
set companionView = database.OpenView("SELECT File FROM File WHERE File=?") : CheckError

' Fetch each file and request the source path, then verify the source path, and get the file info if present
Dim fileKey, fileName, folder, sourcePath, fileSize, version, language, delim, message, info
Do
	Set record = view.Fetch : CheckError
	If record Is Nothing Then Exit Do
	fileKey    = record.StringData(1)
	fileName   = record.StringData(2)
	folder     = record.StringData(3)
REM	fileSize   = record.IntegerData(4)
REM	companion  = record.StringData(5)
	version    = record.StringData(5)
REM	language   = record.StringData(6)

	' Check to see if this is a companion file
	Dim companionRec
	Set companionRec = installer.CreateRecord(1) : CheckError
	companionRec.StringData(1) = version
	companionView.Close : CheckError
	companionView.Execute companionRec : CheckError
	Dim companionFetch
	Set companionFetch = companionView.Fetch : CheckError
	Dim companionFile : companionFile = True
	If companionFetch Is Nothing Then
		companionFile = False
	End If

	delim = InStr(1, fileName, "|", vbTextCompare)
	If delim <> 0 Then
		If shortNames Then fileName = Left(fileName, delim-1) Else fileName = Right(fileName, Len(fileName) - delim)
	End If
	sourcePath = session.SourcePath(folder) & fileName
	If installer.FileAttributes(sourcePath) = -1 Then
		message = message & vbNewLine & sourcePath
	Else
		fileSize = installer.FileSize(sourcePath) : CheckError
		version  = Empty : version  = installer.FileVersion(sourcePath, False) : Err.Clear ' early MSI implementation fails if no version
		language = Empty : language = installer.FileVersion(sourcePath, True)  : Err.Clear ' early MSI implementation doesn't support language
		If language = version Then language = Empty ' Temp check for MSI.DLL version without language support
		If Err <> 0 Then version = Empty : Err.Clear
		If updateMsi Then
			' update File table info
			record.IntegerData(4) = fileSize
			If Len(version)  > 0 Then record.StringData(5) = version
			If Len(language) > 0 Then record.StringData(6) = language
			view.Modify msiViewModifyUpdate, record : CheckError

			' update MsiFileHash table info if this is an unversioned file
			If populateHash And Len(version) = 0 Then
				Set fileHash = installer.FileHash(sourcePath, 0) : CheckError
				hashUpdateRec.StringData(1) = fileKey
				hashUpdateRec.IntegerData(2) = 0
				hashUpdateRec.IntegerData(3) = fileHash.IntegerData(1)
				hashUpdateRec.IntegerData(4) = fileHash.IntegerData(2)
				hashUpdateRec.IntegerData(5) = fileHash.IntegerData(3)
				hashUpdateRec.IntegerData(6) = fileHash.IntegerData(4)
				hashView.Modify msiViewModifyAssign, hashUpdateRec : CheckError
			End If
		ElseIf console Then
			If companionFile Then
				info = "* "
				info = info & fileName : If Len(info) < 12 Then info = info & Space(12 - Len(info))
				info = info & "  skipped (version is a reference to a companion file)"
			Else
				info = fileName : If Len(info) < 12 Then info = info & Space(12 - Len(info))
				info = info & "  size=" & fileSize : If Len(info) < 26 Then info = info & Space(26 - Len(info))
				If Len(version)  > 0 Then info = info & "  vers=" & version : If Len(info) < 45 Then info = info & Space(45 - Len(info))
				If Len(language) > 0 Then info = info & "  lang=" & language
			End If
			Wscript.Echo info
		End If
	End If
Loop
REM Wscript.Echo "SourceDir = " & session.Property("SourceDir")
If Not IsEmpty(message) Then Fail "Error, the following files were not available:" & message

' Update SummaryInformation
If updateMsi Then
	Set sumInfo = database.SummaryInformation(3) : CheckError
	sumInfo.Property(11) = Now
	sumInfo.Property(13) = Now
	sumInfo.Persist
End If

' Commit database in case updates performed
database.Commit : CheckError
Wscript.Quit 0

' Extract argument value from command line, processing any option flags
Function NextArgument
	Dim arg
	Do  ' loop to pull in option flags until an argument value is found
		If iArg >= argCount Then Exit Function
		arg = Wscript.Arguments(iArg)
		iArg = iArg + 1
		If (AscW(arg) <> AscW("/")) And (AscW(arg) <> AscW("-")) Then Exit Do
		Select Case UCase(Right(arg, Len(arg)-1))
			Case "U" : updateMsi    = True
			Case "H" : populateHash = True
			Case Else: Wscript.Echo "Invalid option flag:", arg : Wscript.Quit 1
		End Select
	Loop
	NextArgument = arg
End Function

Sub CheckError
	Dim message, errRec
	If Err = 0 Then Exit Sub
	message = Err.Source & " " & Hex(Err) & ": " & Err.Description
	If Not installer Is Nothing Then
		Set errRec = installer.LastErrorRecord
		If Not errRec Is Nothing Then message = message & vbNewLine & errRec.FormatText
	End If
	Fail message
End Sub

Sub Fail(message)
	Wscript.Echo message
	Wscript.Quit 2
End Sub
