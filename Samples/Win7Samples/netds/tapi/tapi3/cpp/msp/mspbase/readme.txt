______________________________________________________________________________

  Copyright (C) 2001 Microsoft Corporation. All rights reserved.

______________________________________________________________________________



Media Streaming Provider Base Classes



Overview:


These are the sources for Media Streaming Provider Base Classes.

See Platform SDK and SampleMSP sample for more information on using MSP Base
Classes for writing Media Streaming Providers.





How to build MSP Base Classes:

To build the sample:

- install the DirectX9 SDK

- set the SDK build environment by running setenv.bat in the Platform SDK root directory

- Set the enviroment variable DXSDK_DIR to point to the root directory of the DirectX9 SDK
	(e.g. set DXSDK_DIR=C:\DXSDK )
  NOTE: With latest Direct X SDK , DXSDK_DIR is already set, set it manually only if the env var is not set

  You can add this to the setenv.bat so DXSDK_DIR will be set whenever you run setenv.bat.  A good place to add this 
	line is right after the set MSSDK= line

- type "nmake" in MSPBase directory. This will build MSP Base classes and 
produce MSPBaseSample.lib


Note that MSPBase' headers can be found in Platform SDK's include directory.







Building Your MSP with MSP Base Classes


Your MSP should link with the lib file produced from building MSP Base Classes.
SampleMSP is an example of an MSP that uses a custom-built msp.

