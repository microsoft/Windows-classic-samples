PowerShellHostClassifier Project

Demonstrates
	This is an introduction to the extensibility model for FSRM Classification. 
	It demonstrates how to develop a custom managed classifier that will be a part of the FSRM pipeline and 
	use a powershell script per rule to classifiy files. See "How to write the script:" on the structure of the script

Languages
     This sample is available in the following language implementation:
     C#
     hosted scripts are in powershell
     
Files

	PowerShellHostClassifier.csproj
		This is the main project file for VC# projects generated using an Application Wizard. 
		It contains information about the version of Visual C# that generated the file, and 
		information about the platforms, configurations, and project features selected with the
		Application Wizard.

	PowerShellHostClassifier.cs
		This contains the class that implements the IFsrmClassifierModuleImplementation interface
		enabling this classifier to become a part of the FSRM classification pipeline and load a powershell 
		script for each file.

	PowerShellRuleHoster.cs
		This class abstracts the rule, and hosts the powershell pipeline allowing classification using a script.

	BlockablePropertyBagEumerator.cs
		This class contains an IEnumerator implementation and allows for putting propertyBags into powershell
		and finding out when powershell has finished consuming the current propertyBag
	
	OverloadStream.cs
		This is a Stream implementation that wrapps IStream. This is useful because IStream isn't very useful in managedcode / powershell.

        ModuleSuppressions.cs
                This file contains all compile time warnings suppressions for FXCOP.

	keyfile.snk
		A key to strong name sign the dll

	classreg2.reg
		Registry settings to enable classifier registration

	eventlog.reg
		Registry settings for creating the eventlog

	powershell_scripts\content_classifier.ps1
		An example powershell script that shows how to parse rule additional parameters and classify based on file content
		This runs in Classifier Supplied Value mode

	powershell_scripts\demo_script.ps1
		A very minimal example powershell script that simply classifies .txt files as their filename minus .txt
		This runs in Classifier Supplied Value mode
	Powershell_scripts\yes_no_script.ps1
		A very minimal exmaple powershell script that classifies .txt files to the set value
		This runs in yes/no classifier mode
		
	install_as_external.cmd/register_fsrm_module.vbs/register_with_fsrm_as_external.vbs
		These scripts register the dlls, register with COM+ (for debugging ease) and register with FSRM respectively.
		For ease of debugging, the script registers the server as external and the process running the classifier
                can be identified as the service with name of the classifier, PowerShellHostClassifier.

	install_as_network_service.cmd/RegisterWithFsrmAsNetworkService.vbs
		These scripts register the dlls and register with FSRM respectively.
		The script registers the server to run under network service account and during classification the server runs
		under network service account in a dllhost.exe process. This method of installation eliminates the extra layer of COM+
                registration but it is tougher to debug owing to the reason that a debugger cannot be attached during DLL load since to
                identify the dllhost.exe hosting the server when classification runs requires looking for the dllhost.exe
		process (among the many potentially running on the machine) which has loaded the sample DLL, namely PowerShellHostClassifier.dll.
 
Prerequisites

	Windows Server 2008 R2

Building the Sample

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild PowerShellHostClassifier.sln

[NOTE: PowerShellHostClassifier.sln has an assembly reference to "C:\Program Files\Reference Assemblies\Microsoft\FSRM\v7.0\srmlib.dll" as the default path assuming the SDK was installed into "C:\Program Files\" and its references imported for Visual Studio (using Windows SDK COnfiguration tool). If the SDK was installed elsewhere, please point to the correct srmlib.dll for the build to succeed]



To build the sample using Visual Studio 2005 (preferred method):
================================================
     1. Double-click the icon for the PowerShellHostClassifier.sln (solution) file to open the file in Visual Studio.
     2. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

[NOTE: PowerShellHostClassifier.sln has an assembly reference to "C:\Program Files\Reference Assemblies\Microsoft\FSRM\v7.0\srmlib.dll" as the default path assuming the SDK was installed into "C:\Program Files\" and its references imported for Visual Studio (using Windows SDK COnfiguration tool). If the SDK was installed elsewhere, please point to the correct srmlib.dll for the build to succeed]

Installing the Sample
=====================
	
This sample provides 2 methods to install the sample:

Method 1:
---------

        Run the attached install_as_external.cmd script.

	This script will need to be edited if any changes to the classifier or its hosting model is required.
	This does the following:
		Installs the binaries to %systemdrive%\PowerShellHostClassifier
		regsvr32 /s PowerShellHostClassifier.dll
		register_fsrm_module.vbs registers with COM+
		register_with_fsrm_as_external.vbs registers this classifier with the FSRM pipeline

Method 2:
---------

        Run the attached install_as_network_service.cmd script.

	This script will need to be edited if any changes to the classifier or its hosting model is required.
	This does the following:
		Installs the binaries to %systemdrive%\PowerShellHostClassifier
		regsvr32 /s PowerShellHostClassifier.dll
		register_with_fsrm_as_external.vbs registers this classifier with the FSRM pipeline

As mentioned above, Method 1 is easier to debug since the classifier (server) can be identified as the windows service with name of the classifier,
PowerShellHostClassifier while Method 2 runs the server under a dllhost.exe process. But Method 2 eliminates the additional layer of complexity
involved with COM+ and COM+ registration which Method 1 requires.
		
Modes of install:
================
The classifier can be installed in Classifier Supplied Value mode(NeedsExplicitValue = false) and yes no mode (NeedsExplicitValue = true)
The default scripts install the Classifier in Classifier Supplied Value mode. To change it either change the module paramter to NeedsExplicitValue to true
Or install another instance with a different name and the NeedsExplicitValue module parameters as true (by editing the .vbs scripts used in the install)



Running the Sample

To run the sample:
=================
     1. Run File Server Resource Manager (FSRM.msc)
     2. Go to Classification Management -> Classification Properties
     3. Define a custom property 
     4. Go to Classification Rules
     5. Create a new rule, provide rule name, scope, and on Classification Tab select 'Powershell Host Classifier' in classification mechanism.
     5.1 click Advanced and ensure that the additional parameters has the Name filed populated with the string "ScriptFileName" and the Value field is populated with the "actual path to the powershell script file". Details of writing script are below.
     6. Create several files in the scope directory
     7. Run classification
     8. Notice that the files will get classified appropriatelly





How to write the script:


Structure:
=========
To create a script you need to create a text that contains minimally a process block, and optionally a begin and end block. 
Do not contain any other blocks. Functions can be declared inside the begin block.
Samples are provided inside the powershell_scripts\ folder

Yes/No Mode:
===========
In Yes/No mode inside the process block, for each property bag emit $true if you want the value to be applied, $false or nothing if you don't 


Classifier Supplied Value mode:
==============================
In Classifier Supplied Value mode inside the process block, for each property bag if you want the property updated emit the correct object, otherwise emit no object

Bool - a boolean object either $true or $false

Number - an integer

string - a string

orderedlist - a string

multichoice - a string composed of values optionally concatenated by pipe ex1: "value1"  ex2: "value1|value2|value3"

multistring - a string composed of multiple strings concatenated by pipe ex1: "some random string"  ex2: "anystring|some other random string|the number 5"

date-time - a datetime object


Pipeline Input:
==============
In the process block the pipeline input will be an IFsrmPropertyBag with an additional method called .GetStream() 
this Method returns a Stream instead of an IStream which is much more usefull in powershell
powershell_scripts\content_classifier.ps1 shows an example of this additional function being called


Additional Input:
================
Other parameters that are passed into the script that are just globally acessable are the following

$Rule - an IFsrmClassificationRule that contains the definition of the rule that the script is part of

$ModuleDefinition - an IFsrmPipelineModuleDefinition that contains the definition of the moduledefinition

$PropertyDefinition - an IFsrmPropertyDefinition that states the definition of the property that is being optionally set

$PropertyBagList - Do not override this global object it is being used to control input into the pipeline