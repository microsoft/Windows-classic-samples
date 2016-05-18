Search Folder API Sample
===============================
This sample demonstrates how to use ISearchFolderItemFactory to create a search with query constraints using the shell programming model. 
ISearchFolderItemFactory constructs a folder shell item (aka container) that represents the query.
It uses conditions (ICondition/IConditionFactory) to express the query constraints and then displays the result using the file open dialog.

Sample Language Implementations
===============================
C++

Files
=============================================
SearchFolder.cpp
SearchFolder.sln
SearchFolder.vcproj
 
To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild SearchFolder.sln 


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the sample directory.
     2. Double-click the icon for the SearchFolder.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type SearchFolderSample.exe at the command line, or double-click the icon for SearchFolderSample.exe to launch it from Windows Explorer.

Note:  The sample creates a search against the document library looking for with System.Kind property equal to 'Document' and System.Size property greater than 10240 (10K).  Make sure you have items that match those predicates in your document library or search will display 0 results.
