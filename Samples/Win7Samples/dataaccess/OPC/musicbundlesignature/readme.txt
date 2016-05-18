
Music Bundle Digital Signatures sample
===============================

     This sample demonstrates how to use OPC digitial signature APIs to sign & validate the signature on an OPC package.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
===============================

     main.cpp - main entry point for the sample application

     sign.cpp / sign.h - contains sample code showing how to sign a package

     validate.cpp / validate.h - contains sample code showing how to validate the signature in a package

     util.cpp / util.h - common utility functions for working with OPC packages & managing resources

     SampleMusicBundle.zip - An unsigned package containing sample music files

     MusicBundleSignature.vcproj - build configuration for this sample

To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing MusicBundleSignature.vcproj

     2. Type: msbuild MusicBundleSignature.vcproj
     

To build the sample using Visual Studio 2008 (preferred method):
================================================

     1. Open Windows Explorer and navigate to the  directory.

     2. Double-click the icon for the MusicBundleSignature.vcproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample using the command prompt:
=============================================

	Debug\MusicBundleSignature.exe
	
	or
	
	Release\MusicBundleSignature.exe

Note: 

    Your system must have at least one X.509 certificate installed to run this sample successfully. 
    You can use Makecert.exe to create an X.509 certificate for testing purpose if you don't have one
    on your system.

Output files
=============================================
     SampleMusicBundle_signed.zip - The signed version of the SampleMusicBundle.zip package
     
     signer1.cer - The certificate file that you can used to identify who signed the package
