Set Author Sample


Description
===========
A simple console application that demonstrates how to use the Packaging API to
in conjunction with MSXML to read and modify an Office Open XML word processing
file, such as a Microsoft Word 2007 or Windows WordPad document.

The sample demonstrates the following features:
    * Demonstrate using the Packaging API in conjunction with MSXML.
    * Display text an Office Open XML word processing document package.
    * Modify core properties of the document package.
    * Save the modified package as a new word processing document.


Sample Language Implementations
===============================
    This sample is available in the following language implementations:
    C++


Sample Files
============
_File_              _Description_
setauthor.vcproj    Visual Studio 2008 project file.
setauthor.cpp       Entry point for the sample.
wordlib.h           Declarations of utility functions for Open Office XML word
                    processing files.
wordlib.cpp         Definitions of utility functions for Open Office XML word
                    processing files.
opclib.h            Declarations of utility functions for packages in general.
opclib.cpp          Definitions of utility functions for packages in general.
util.h              Helper definitions for using COM.
sample.docx         An Office Open XML word processing document for use with 
                    the sample.


Requirements
============
_Product_           _Version_
Windows SDK         Windows 7


Building the Sample
===================
The following procedure describes how to build the sample.

1. Install the Windows SDK.
2. Register the SDK.
3. Follow steps for building using Visual Studio or the command prompt as
   needed.

       Using Visual Studio
       a. Open Windows Explorer. 
       b. Navigate to the directory that contains the "setauthor.sln" file.
       c. Double click the "setauthor.vcproj" file to open it with Visual 
          Studio 2008.
       d. Go to the Build menu in Visual Studio and select Build Solution.

       Using the command prompt
       a. Open a command prompt. 
       b. Navigate to the directory that contains the "setauthor.vcproj" file.
       c. Enter the command: msbuild setauthor.vcproj.
            Note  Either Debug or Release build mode may be selected.


Running the Sample
==================
Before running the sample application, ensure that the sample project has been 
built in Debug or Release mode that corresponds to the version of the sample
application to be run.

The following procedures describe how to run the sample using Visual Studio and
the command prompt.

Using Visual Studio
1. Open project properties.
2. Select the Debugging project setting command arguments
       + To view text and authorship information, enter:
         "setauthor.exe sample.docx"
       + To view and modify authorship information, enter:
         "setauthor.exe sample.docx <NewAuthorName> <NewDocumentName>.docx"

Using the command prompt
1. Open a command prompt.
2. Navigate to the directory that contains to either the \Debug or \Release 
   project directory, as appropriate.
       + To view text and authorship information, enter the command:
         setauthor.exe ..\sample.docx
       + To view and modify authorship information, enter the command: 
         setauthor.exe ..\sample.docx <NewAuthorName> <NewDocumentName>.docx


Additional Information
======================
The modifications made to the package can also been seen in the package file 
itself after the package has been saved. Rename the output file to a ".zip"
extension and open the package with explorer. Browse to the Core Properties 
part and open it with Notepad or Internet Explorer to view the properties XML 
markup.


Downloading the Sample
======================
This sample is available in the following locations.

_Location_      _Path/URL_
Windows SDK     \Program Files\Microsoft SDKs\Windows\v7.0\Samples\dataaccess\OPC
Code Gallery    http://go.microsoft.com/fwlink/?LinkID=125499
