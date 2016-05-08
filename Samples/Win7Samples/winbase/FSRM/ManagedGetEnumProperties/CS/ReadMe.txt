ManagedGetEnumProperties

Demonstrates
How to get property definitions using com in managed code from FSRM. 
Also it states how to get all the property values from a file, or a specific property.

Languages

     C#

Files
	ManagedGetEnumProperties.csproj
		This is the main project file for VC# projects generated using an Application Wizard. 
		It contains information about the version of Visual C# that generated the file, and 
		information about the platforms, configurations, and project features selected with the
		Application Wizard.

	ManagedGetEnumProperties.cs
		Contains function to enumerate property definitions, properties of a file, or a specific property of a file
 
Prerequisites
Visual Studios 2005 or later

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Copy the file c:\windows\system32\srmlib.dll from a Windows Server 2008 R2 box to current directory
     3. Type msbuild ManagedGetEnumProperties.sln

To build the sample using Visual Studio 2005 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Copy the file c:\windows\system32\srmlib.dll from a Windows Server 2008 R2 box to current directory
     3. Double-click the icon for the ManagedGetEnumProperties.sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample
Currently the file to get the properties of is "c:\foo\cat.txt" so this file should be created or changed in the code.
Also the code tries to get the property "IsThisClassified" from the above file so this property should be added or changed in the code.
Failure to do the above 2 items will result in failures during reading properties from the above mentioned file

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type ManagedGetEnumProperties.exe at the command line

