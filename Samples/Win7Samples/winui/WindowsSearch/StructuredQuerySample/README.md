---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Structured Query sample
urlFragment: structuredQuery-sample
description: Demonstrates using the Structured Query API for parsing queries in Advanced Query Syntax (AQS).
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Structured Query sample

This sample demonstrates using the Structured Query API for parsing queries in Advanced Query Syntax (AQS),
with the results represented by condition trees, a recursively defined data structure. The sample also demonstrates
recursive traversal of condition trees. Finally, the sample demonstrates some Windows 7 improvements in the
Structured Query API by having some sections of the code in two versions: a more elegant version that will run
only on Windows 7 and a version using only APIs also available on Windows Vista.

This sample is available in the following language implementations:
C++

## Sample files

| File | Description |
|------|-------------|
| *ReadMe.txt* | This file. |
| *StructuredQuerySample.cpp* | The main file, containing all code for the sample. In the begnning there is a commented out line that should be uncommented to build an executable that will run successfully also on Vista. |
| *StructuredQuerySample.sln* | Visual Studio 2008 solution file. |
| *StructuredQuerySample.vcproj* | Visual Studio 2008 project file. |

## To build the sample using the command prompt

1. Open the Command Prompt window and navigate to the directory.
1. Type **msbuild StructuredQuerySample.sln**.

## To build the sample using Visual Studio 2008 (preferred method)

1. Open Windows Explorer and navigate to the  directory.
1. Double-click the icon for the *StructuredQuerySample.sln* file to open it in Visual Studio.
1. In the **Build** menu, select **Build Solution**. The application will be built in the default *Debug* or *Release* directory.

## To run the sample

1. Navigate to the directory that contains the new executable by using the command prompt or Windows Explorer.
1. Type **StructuredQuerySample.exe** at the command line, or double-click the icon for *StructuredQuerySample.exe* to launch it from Windows Explorer.
1. Type an AQS query (such as `from:bill` or `file:\*.txt modified:this month`) followed by **Enter**. The program will display a tree representation of the resulting condition tree.
1. Repeat step 3 as desired. Terminate the program by typing **Enter**.
