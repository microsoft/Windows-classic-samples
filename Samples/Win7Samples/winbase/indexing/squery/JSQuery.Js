//+---------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1999, Microsoft Corporation.  All Rights Reserved.
//
// SCRIPT:   JSQuery
//
// PURPOSE:  Illustrates how to execute an Indexing Service query
//           using Microsoft JScript.  The query uses the GroupBy
//           property of the Query object to create a chaptered
//           recordset of all files of a specified type in all
//           directories that contain the specified file type.
//
// PLATFORM: Windows 2000
//
//----------------------------------------------------------------------

var strGroupBy;         // Name of GroupBy column.
var intI, intJ;         // Index variables.
var objQ;               // Query object.
var strRecord;          // Output record of query results.
var objRS_Child;        // Child RecordSet object.
var objRS_Parent;       // Parent RecordSet object.
var intRS_Child_Count;  // Number of current record of child RecordSet.
var intRS_Parent_Count; // Number of current record of parent RecordSet.
var objU;               // Utility object.

// Create a Query object.
objQ = new ActiveXObject("IXSSO.Query");

// Set the properties of the Query object.
objQ.Columns = "filename, directory, size, write";
objQ.Query   = "#filename *.asp";
objQ.GroupBy = "directory[a]";
objQ.Catalog = "system";
objQ.OptimizeFor      = "recall";
objQ.AllowEnumeration = true;
objQ.MaxRecords       = 20000;

// Create a Utility object.
objU = new ActiveXObject("IXSSO.Util");

// Add the physical path and all subdirectories.
objU.AddScopeToQuery(objQ, "\\", "deep");

// Output the Query properties.
WScript.Echo(" Columns = " + objQ.Columns);
WScript.Echo(" Query = " + objQ.Query);
WScript.Echo(" GroupBy = " + objQ.GroupBy);
WScript.Echo(" Catalog = " + objQ.Catalog);
WScript.Echo(" CiScope = " + objQ.CiScope);
WScript.Echo(" CiFlags = " + objQ.CiFlags);
WScript.Echo(" OptimizeFor = " + objQ.OptimizeFor);
WScript.Echo(" AllowEnumeration = " + objQ.AllowEnumeration);
WScript.Echo(" MaxRecords = " + objQ.MaxRecords);

// Create a parent (grouped) RecordSet object for the Query.
objRS_Parent = objQ.CreateRecordSet("nonsequential");

// Determine the name of the GroupBy column.
strGroupBy = "";
for (intI=0; intI<objRS_Parent.Fields.Count; intI++) {
    if (objRS_Parent(intI).Name != "Chapter") {
        if (strGroupBy != "")
            strGroupBy = strGroupBy + "    " + objRS_Parent(intI).Name
        else
            strGroupBy = objRS_Parent(intI).Name;
    };
};

// Read through the parent RecordSet object.
intRS_Parent_Count = 0;
while (!objRS_Parent.EOF) {
    intRS_Parent_Count = intRS_Parent_Count + 1;
    strRecord = (intRS_Parent_Count + ".      ").slice(0,4);

    // Extract values for non-chaptered columns.
    for (intI=0; intI<objRS_Parent.Fields.Count; intI++) {
        if (objRS_Parent(intI).Name != "Chapter")
            strRecord = strRecord + "  " + objRS_Parent(intI).Value;
    };

    // Output the values for non-chaptered columns.
    WScript.Echo(strRecord);

    // Create a child RecordSet object for the chaptered columns.
    objRS_Child = objRS_Parent.Fields("Chapter").Value;

    // Read through the child (chaptered) RecordSet object.
    intRS_Child_Count = 0;
    while (!objRS_Child.EOF) {
        intRS_Child_Count = intRS_Child_Count + 1;
        strRecord = (intRS_Parent_Count + "." + intRS_Child_Count + ".      ").slice(0,8);

        // Extract values for chaptered columns.
        for (intJ=0; intJ<objRS_Child.Fields.Count; intJ++) {
            if (objRS_Child(intJ).Name != "Chapter") {
                if (objRS_Child(intJ).Name != strGroupBy)
                    strRecord = strRecord + "  " + objRS_Child(intJ).Value;
            };
        };

        // Output the values for chaptered columns.
        WScript.Echo(strRecord);
        objRS_Child.MoveNext;
    };

    // Close the child RecordSet object.
    objRS_Child.Close;
    objRS_Child = null;

    // Move to the next record in the parent RecordSet object.
    objRS_Parent.MoveNext;
};

// Close the parent RecordSet object.
objRS_Parent.Close;
objRS_Parent = null;

WScript.Echo("Done!");
