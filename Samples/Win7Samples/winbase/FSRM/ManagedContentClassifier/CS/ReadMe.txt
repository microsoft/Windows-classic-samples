ManagedContentClassifier Project

Demonstrates
	This is an introduction to the extensibility model for FSRM Classification. 
	It demonstrates how to develop a custom managed classifier that will be a part of the FSRM pipeline and 
	enable applying custom rules during the FSRM classification phase.

	In this example, the classifier can be setup to search for strings in the file's content.
	When a classification rule is defined, the Additional Classification Parameter property sheet 
	(Rule Properties -> Classification -> Advanced -> Additional Classification Parameters) can be used
	to pass parameters to the classifier. This classifier accepts any number of parameters in the following format:

	RegularExpression=Some.*RegularExpression
	
	*Note if a regular expression would be matched it might not get hit due to the content getting read in Chunks and regular expression limited to a chunk.

Languages
     This sample is available in the following language implementation:
     C#
     
Files

	ManagedContentClassifier.csproj
		This is the main project file for VC# projects generated using an Application Wizard. 
		It contains information about the version of Visual C# that generated the file, and 
		information about the platforms, configurations, and project features selected with the
		Application Wizard.

	ManagedContentClassifier.cs
		This contains the class that implements the IFsrmClassifierModuleImplementation interface
		enabling this classifier to become a part of the FSRM classification pipeline and search file contents with
		a regular expression. It also contains a helper class to help managing fsrm rules.

	keyfile.snk
		A file to 

	classreg2.reg
		Registry settings to enable classifier registration
		
	install_as_external.cmd/register_fsrm_module.vbs/register_with_fsrm_as_external.vbs
		These scripts register the dlls, register with COM+ (for debugging ease) and register with FSRM respectively.
		For ease of debugging, the script registers the server as external. and the process running the classifier
                can be identified as the service with name of the classifier, managedsampleclassifier.
 
Prerequisites

	Windows Server 2008 R2

Building the Sample

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Copy the file c:\windows\system32\srmlib.dll from a Windows Server 2008 R2 box to current directory
     3. Type msbuild ManagedContentClassifier.sln


To build the sample using Visual Studio 2005 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Copy the file c:\windows\system32\srmlib.dll from a Windows Server 2008 R2 box to current directory
     3. Double-click the icon for the ManagedContentClassifier.sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

NOTE: Optionally, srmlib.dll could be copied from the SDK installation path instead of a Windows Server 2008 R2 box. If SDK was installed in its default path, srmlib.dll could be found at "C:\Program Files\Reference Assemblies\Microsoft\FSRM\v7.0\srmlib.dll"

Installing the Sample

	Run the attached install_as_external.cmd script.

	This script will need to be edited if any changes to the classifier or its hosting model is required.
	This does the following:
		Installs the binaries to %systemdrive%\ManagedContentClassifier
		regsvr32 /s ManagedContentClassifier.dll
		register_fsrm_module.vbs registers with COM+
		register_with_fsrm_as_external.vbs registers this classifier with the FSRM pipeline
		

Running the Sample

To run the sample:
=================
     1. Run File Server Resource Manager (FSRM.msc)
     2. Go to Classification Management -> Classification Properties
     3. Define a custom property 
     4. Go to Classification Rules
     5. Create a new rule, provide rule name, scope, and on Classification Tab select 'Managed Content Classifier' in classification mechanism.
	 6. Click Advanced and go to Additional Classification Parameters
	 7. Specify RegularExpression   Some.*RegularExpression
	 8. Click OK 
	 9. Create several files in the scope directory, some containing the word foo and/or bar as content.
	 10. Run 'Run Classification Now' from the Actions pane.
     11. The files containing Some.*RegularExpression in contents are classified.

