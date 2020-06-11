---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Transcode sample
urlFragment: transcode-sample
description: Demonstrates using the transcode API to transcode a source file to Windows Media format.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Transcode sample

Demonstrates using the transcode API to transcode a source file to Windows Media format.

## Sample language implementations

C++

## Files

- *main.cpp*
- *README.md*
- *Transcode.cpp*
- *Transcode.h*
- *Transcode.sln*
- *Transcode.vcproj*

## Build the sample using the command prompt

1. Open the Command Prompt window and navigate to the *Transcode* directory.
2. Type **msbuild Transcode.sln**.

## Build the sample using Visual Studio (preferred method):

1. Open Windows Explorer and navigate to the *Transcode* directory.
2. Double-click the icon for the *Transcode.sln* file to open the file in Visual Studio.
3. In the **Build** menu, select **Build Solution**. The application will be built in the default *\Debug* or *\Release* directory.

## Run the sample

This sample is a command-line application. 

It uses the following command-line arguments:

     Transcode.exe inputfile outputfile

where

     inputfile:    The name of the source file.
     outputfile:   The name of the target file.

The file extension for the target file should be *.wma* or *.wmv*.

