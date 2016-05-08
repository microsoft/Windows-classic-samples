////////////////////////////////////////////////////////////////////
// Description
////////////////////////////////////////////////////////////////////
The TranscodeImage solution is a console application that 
demonstrates ITranscodeImage::TranscodeImage.
The application takes two command line arguments.  
First argument: the path of the image file to transcode.
Second argument: the path at which to save the transcoded file.

////////////////////////////////////////////////////////////////////
// Files
////////////////////////////////////////////////////////////////////

The solution contains the following files:
TranscodeImage.sln		-- Visual Studio solution file.
TranscodeImage.vcproj	-- Visual Studio project file.
TranscodeImage.cpp		-- Application code.


////////////////////////////////////////////////////////////////////
// Build instructions
////////////////////////////////////////////////////////////////////

Using Visual Studio 2005, in Project Properties, under 
Configuration Properties->Linker->Input, make sure the following 
are added to Additional Dependencies: 

shlwapi.lib
photoacquireuid.lib 

In Tools->Options->Projects and Solutions->VC++ directories
make sure that the SDK directories for library files and include files
are added.
