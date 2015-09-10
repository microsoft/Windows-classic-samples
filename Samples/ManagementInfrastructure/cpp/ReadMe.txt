====================================================================================
                     MANAGEMENT INFRASTRUCTURE API SAMPLE
====================================================================================

This document explains the samples of Windows Management Infrastructure API, which 
includes 4 sections:

1. OVERVIEW - Gives overview of Windows Management Infrastructure API.
2. PREPARATION - Instructions of preparations to build samples.
3. SAMPLE FILES STRUCTURE - Describes the samples folder structure.
4. TOOLS - Tools used to build samples.




====================================================================================
1. OVERVIEW
====================================================================================

These samples demonstrate how to use the Windows Management Infrastructure API to 
expose data through the new provider model, and consume the data through the new 
client APIs. 

Windows 8 / Windows Server 2012 introduce a new set of management APIs, called the 
Windows Management Infrastructure (MI) APIs. This includes provider C APIs, client C 
APIs, and client .Net APIs. These new APIs are more tightly aligned with the CIM 
standard that is the basis for WMI, and provide support for the standard WS-Man 
protocol via WinRM.

These new APIs will interoperate directly with existing Windows Management 
Instrumentation (WMI) clients and providers. An application written using the new 
client APIs work with pre-existing WMI providers as well as providers based off the 
new MI C APIs. Likewise, providers written using the new APIs can be called from 
client applications written using the older version of the WMI .Net and C APIs.

In addition to the APIs, Windows 8 and Windows Server 2012 also introduce a new 
provider model, with code generation and provider registration tools to support them. 
A C based provider code skeleton can be generated from user-defined schema in the 
form of a MOF file, using the tool called Convert-MofToProvider.exe, which ships in 
the Windows 8 SDK. Register-CimProvider.exe is a new provider registration tool, and 
ships with Windows 8 and Windows Server 2012, and is used in place of the older 
MOFCOMP.EXE. These tools work only for providers based on the new APIs.

A CIM-Based Cmdlet model was also introduced to support creation of PowerShell 
cmdlets, which expose the data and the operations of any provider to PowerShell 
users. It introduces a new way to create cmdlets by using an XML file to describe the 
cmdlet functionality, and then importing the new module into PowerShell. This new way 
of writing cmdlets works with both new providers written using the MI APIs, and 
providers written using the older WMI APIs.

A set of generic cmdlets was introduced in Windows 8/ Windows Server 2012, called CIM 
Cmdlets. These cmdlets are based on the MI client .Net API, and help consume the data 
and operations from any new or existing providers. 



====================================================================================
2. PREPARATION
====================================================================================

The following steps are mandatory to build these samples. All environment variables 
created using the steps below will be used by samples.

1) Install the Windows 8 SDK and create environment variables to reference it.
Create an environment variable called SDKDIR to point to installation folder. For 
example: 
    SDKDIR=C:\Program Files (x86)\Windows Kits\8.0

Create environment variables to point to the MI.net API installation folder
    SDKMIDOTNETDIR=C:\Program Files (x86)\Reference Assemblies\Microsoft\WMI\v1.0

Create additional environment variables as follows,
    SDKBINDIR=%SDKDIR%\bin\x64

NOTE: The above assumes target machine's processor is amd64. If the target machine is 
x86, please use the following instead:
    SDKBINDIR=%SDKDIR%\bin\x86

2) Install Windows 8 SDK samples.
Create environment variables to point to installation folder.
    SDKSAMPLEDIR=%SDKDIR%\Samples\Desktop
    MISAMPLEDIR=%SDKSAMPLEDIR%\MIAPI

3) Download the CIM schema 2.26 from DMTF website: http://dmtf.org/sites/default/files/cim/cim_schema_v2260/cim_schema_2.26.0Final-MOFs.zip, 
and unzip the downloaded schema to a local folder.

Create an environment variable point to the schema unzip location. For example,
    CIM2260DIR=c:\cim_schema_2.26.0

4) Install Visual Studio 2012. 



====================================================================================
3. SAMPLE FILES STRUCTURE
====================================================================================

ReadMe.txt
    Gives an introduction to the MI sample

MSFT_Qualifiers.mof
    Defines 2 qualifiers referenced by sample.mof

Sample.mof
    Defines the schema of the MI sample

Misample.sln
    The Visual Studio solution file that contains all projects of MI sample

Process\  
    This folder contains source code demonstrating an end to end use of the MI API
    basic features.

    ReadMe.txt
        Instructions for the process sample
    CIM-BasedCmdlet\ 
        This folder is described in the Readme.Txt in the Process folder
    CIMCmdlet\
        This folder is described in the Readme.Txt in the Process folder
    ManagedClient\
        This folder is described in the Readme.Txt in the Process folder
    NativeClient\
        This folder is described in the Readme.Txt in the Process folder
    Provider\
        This folder is described in the Readme.Txt in the Process folder

Service\
    This directory contains source code of demonstrating an end to end scenario 
    using MI API advanced features, such as, PS semantics and Error APIs.

    ReadMe.txt
        Instructions for the service sample
    CIM-BasedCmdlet\
        This folder is described in the Readme.Txt in the Service folder
    CIMCmdlet\
        This folder is described in the Readme.Txt in the Service folder
    ManagedClient\
        This folder is described in the Readme.Txt in the Service folder
    NativeClient\
        This folder is described in the Readme.Txt in the Service folder
    Provider\
        This folder is described in the Readme.Txt in the Service folder

Indication\
    This directory contains source code of demostrating end to end scenario of
    CIM indication, which is equivalent to WMI event.

    ReadMe.txt
        Instructions for the indication sample
    CIMCmdlet\
        This folder is described in the Readme.Txt in the Indication folder
    ManagedClient\
        This folder is described in the Readme.Txt in the Indication folder
    NativeClient\
        This folder is described in the Readme.Txt in the Indication folder
    Provider\
        This folder is described in the Readme.Txt in the Indication folder


DecoupledHost\
    This directory contains source code of demonstrating how to write a decoupled
    host application using MIAPI.

    ReadMe.txt
        Instructions for hosting a decoupled provider using MI API




====================================================================================
4. TOOLS
====================================================================================

Convert-MofToProvider.EXE
    This tool will be installed as part of Windows 8 SDK, under %SDKBINDIR%.
    It is used to generate provider code files in C language from schema mof.
    Please see Convert-MofToProvider.EXE -help for the detail usage.

Register-CimProvider.EXE
    This tool will be installed as part of Windows 8, under %WINDIR%\system32.
    Used to register cim provider into test box.
    Please see Register-CimProvider.EXE -help for the detail usage.

Visual Studio 2012
    Used to build the sample projects.
