Sample Speech Recognition Engine

Demonstrates
============
The sample speech recognition (SR) engine demonstrates the design, compilation,
installation, and testing for engines. This sample engine implements some
functionality of an SR engine and can be created and used in applications, but
it does not actually perform any recognition, instead it generates valid, but
random, results. This is very useful example code for understanding how a real
SR engine might be implemented. The following is basic information you should
know about this sample SR engine:

    1. The sample engine provides a basic idea about how to develop an SR engine
       to interact with SAPI. 
    2. The engine does not perform the recognition based on an acoustic or
       language model. Instead, it retrieves the CFG information from SAPI and
       constructs random results.
    3. The sample engine does not pass the compliance tests.
    4. If you experience unexpected results for the real SR activity, make sure
       that the sample engine is not in use and that the sample engine has not
       been set as the default engine. 
 
Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
SampleSrEngine.cpp      Implementation of DLL Exports.

SampleSrEngine.idl      This file will be processed by the MIDL tool to produce
                        the type library (SampleSrEngine.tlb) and marshalling
                        code.
                        
stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.
                        
srengobj.h              Contains the declaration of the CSrEngine class, which
                        is the main recognition object                     
                        
srengobj.cpp            Implementation of the CSrEngine class.

srengalt.h              Contains the declaration of the CSrEngineAlternates
                        class which implements the interface ISpSRAlternates.
                        
srengalt.cpp            Implementation of the CSrEngineAlternates class.

srengext.h              Contains the declaration of the CSampleSRExtension class
                        which implements the custom interface
                        ISampleSRExtension.

srengext.cpp            Implementation of the CSampleSRExtension class.

srengui.h               Contains the declaration of the CSrEngineUI class which
                        implements ISpTokenUI. This is used by applications to
                        display UI.

srengui.cpp             Implementation of the CSrEngineUI class.

srengver.h              Version information.

SampleSrEngine.def      Export definition file.

SampleSrEngine.rgs      Registration scripts.
srengalt.rgs
srengext.rgs
srengui.rgs

resource.h              Microsoft Developer Studio generated include file. Used
                        by SampleSrEngine.rc.

SampleSrEngine.rc       Resource scripts.
version.rc2

SampleSrEngine.sln      Microsoft Visual Studio solution file.

SampleSrEngine.vcproj   Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. On Windows 7 open Visual Studio 2005 (or VS 2008) as administrator by
       right clicking the Visual Studio icon and selecting "Run as 
       administrator". Then open the solution file from the 
       "File -> Open ->Project/Solution" menu.
    3. In the Build menu, select Build Solution. The sample engine will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms. It will automatically
       register itself in the Post-Build event.
    
To run the sample:
=================
There are various ways to see the sample engine running:
    1. Use the reco.exe sample which allows you to experiment speech recognition
       engines. Select the "SAPI Developer Sample Engine" to use this sample
       engine. Or,
    2. Programatically create and use this sample engine. Or,
    3. On Windows 7, go to the "Control Panel -> Ease of Access -> Speech
       Recognition Options -> Advanced Speech Options" and select the "SAPI
       Developer Sample Engine" in the language section. Next time you start
       Windows Speech Recognition it will use this sample engine which will take
       random actions as you speak.

Note:
====
The Speech Recognition Engine sample utilizes the Microsoft Speech API (SAPI) 
version 5.3 available on Windows Vista or later Operating System versions. 