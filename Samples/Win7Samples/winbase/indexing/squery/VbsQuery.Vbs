'+----------------------------------------------------------------------
'
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
' ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
' THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
' PARTICULAR PURPOSE.
'
' Copyright 1999, Microsoft Corporation.  All Rights Reserved.
'
' SCRIPT:    VBSQuery
'
' PURPOSE:   Illustrates how to execute an Indexing Service query
'            using Microsoft Visual Basic Scripting Edition.  The
'            query uses the GroupBy property of the Query object
'            to create a chaptered recordset of all files of a
'            specified type in all directories that contain the
'            specified file type.
'
' PLATFORM:  Windows 2000
'
'-----------------------------------------------------------------------

Option Explicit

Dim strGroupBy          ' Name of GroupBy column.
Dim intI, intJ          ' Index variables.
Dim objQ                ' Query object.
Dim strRecord           ' Output record of query results.
Dim objRS_Child         ' Child RecordSet object.
Dim objRS_Parent        ' Parent RecordSet object.
Dim intRS_Child_Count   ' Number of current record of child RecordSet.
Dim intRS_Parent_Count  ' Number of current record of parent RecordSet.
Dim objU                ' Utility object.

' Create a Query object.
Set objQ = WScript.CreateObject("IXSSO.Query")

' Set the properties of the Query object.
objQ.Columns = "filename, directory, size, write"
objQ.Query   = "#filename *.asp"
objQ.GroupBy = "directory[a]"
objQ.Catalog = "system"
objQ.OptimizeFor      = "recall"
objQ.AllowEnumeration = TRUE
objQ.MaxRecords       = 20000

' Create a Utility object.
Set objU = WScript.CreateObject("IXSSO.Util")

' Add the physical path and all subdirectories.
objU.AddScopeToQuery objQ, "\", "deep"

' Output the Query properties.
WScript.Echo " Columns = " & objQ.Columns
WScript.Echo " Query = " & objQ.Query
WScript.Echo " GroupBy = " & objQ.GroupBy
WScript.Echo " Catalog = " & objQ.Catalog
WScript.Echo " CiScope = " & objQ.CiScope
WScript.Echo " CiFlags = " & objQ.CiFlags
WScript.Echo " OptimizeFor = " & objQ.OptimizeFor
WScript.Echo " AllowEnumeration = " & CStr(objQ.AllowEnumeration)
WScript.Echo " MaxRecords = " & objQ.MaxRecords

' Create a parent (grouped) RecordSet object for the Query.
Set objRS_Parent = objQ.CreateRecordSet("nonsequential")

' Determine the name of the GroupBy column.
strGroupBy = ""
For intI = 0 to objRS_Parent.Fields.Count - 1
    If objRS_Parent(intI).Name <> "Chapter" Then
        If strGroupBy <> "" Then
            strGroupBy = strGroupBy & "    " & objRS_Parent(intI).Name
        Else
            strGroupBy = objRS_Parent(intI).Name
        End If
    End If
Next

' Read through the parent RecordSet object.
intRS_Parent_Count = 0
Do While Not objRS_Parent.EOF
    intRS_Parent_Count = intRS_Parent_Count + 1
    strRecord = Left(CStr(intRS_Parent_Count) & ".      ", 4)

    ' Extract values for non-chaptered columns.
    For intI = 0 to objRS_Parent.Fields.Count - 1
        If objRS_Parent(intI).Name <> "Chapter" Then
            strRecord = strRecord & "  " & objRS_Parent(intI).Value
        End If
    Next

    ' Output the values for non-chaptered columns.
    WScript.Echo strRecord

    ' Create a child RecordSet object for the chaptered columns.
    Set objRS_Child = objRS_Parent.Fields("Chapter").Value

    ' Read through the child (chaptered) RecordSet object.
    intRS_Child_Count = 0
    Do While Not objRS_Child.EOF
        intRS_Child_Count = intRS_Child_Count + 1
        strRecord = Left(CStr(intRS_Parent_Count) + "." + CStr(intRS_Child_Count) + ".      ", 8)

        ' Extract values for chaptered columns.
        For intJ = 0 to objRS_Child.Fields.Count - 1
            If objRS_Child(intJ).Name <> "Chapter" Then
                If objRS_Child(intJ).Name <> strGroupBy Then
                    strRecord = strRecord & "  " & objRS_Child(intJ).Value
                End If
            End If
        Next

        ' Output the values for chaptered columns.
        WScript.Echo strRecord
        objRS_Child.MoveNext
    Loop

    ' Close the child RecordSet object.
    objRS_Child.Close
    Set objRS_Child = Nothing

    ' Move to the next record in the parent RecordSet object.
    objRS_Parent.MoveNext
Loop

' Close the parent RecordSet object.
objRS_Parent.Close
Set objRS_Parent = Nothing

WScript.Echo "Done!"
