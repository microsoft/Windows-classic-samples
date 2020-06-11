---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: ETW Consumer sample
urlFragment: etw-consumer-sample
description: Demonstrates the use of the ETW consumer APIs (for example, TDH) for translating events logged to the binary ETL format into simple message strings or XML.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# ETW Consumer API sample

## Description

This sample demonstrates the use of the ETW consumer APIs (for example, TDH) for translating events logged to the binary ETL format into simple message strings or XML.

## Sample language implementations 

This sample is available in the following language implementations:

  C++


## Files

| File | Description |
|------|-------------|
| *EtwConsumer.cpp* | Main program. Allows the user to specify an ETL file to translate into simple strings or XML.|
| *TdhUtil.h* | Header file containing #includes for required libraries, user-defined structures, and helper function prototypes. |
| *common.h* | Header file containing prototypes for the formatting functions for various TDH in-types and out-types. |
| *TdhUtil.cpp* | Contains the implementation of the functions defined in *TdhUtil.h*. |
| *common.cpp* | Contains the implementation of the functions defined in *common.h*. |

## Build

### To build the sample using msbuild

1. Open a Command Prompt window and navigate to the *Samples\WinBase\Eventing\EtwConsumer* directory.
1. Type **msbuild EtwConsumer.sln**.

### To build the sample using Visual Studio

1. Open Windows Explorer and navigate to the *Samples\WinBase\Eventing\EtwConsumer* directory.
2. Double-click the icon for the *EtwConsumer.sln* solution file to open the file in Visual Studio.
3. In the **Build** menu, select **Build Solution**. 

### To build the sample using the makefile

1. Open a Command Prompt window and navigate to the *Samples\WinBase\Eventing\EtwConsumer* directory.
1. Type **nmake** to build the sample.

## Run

### To generate an ETL file for consumption

1. From an elevated CMD prompt, navigate to the *Samples\WinBase\Eventing\EtwConsumer\Output*.
1. Run the following to start logging to the *LogFile.etl* file.

     `logman start "NT Kernel Logger" -o LogFile.etl -ets`

1. Run the following to stop logging.

     `logman stop "NT Kernel Logger" -ets`

### To translate events logged to the ETL file into simple message strings

1. From a CMD prompt, navigate to the *Samples\WinBase\Eventing\EtwConsumer\Output* directory.
1. Run the following command.

     `EtwConsumer LogFile.etl`

### To translate events logged to the ETL file into XML

1. From a CMD prompt, navigate to the *Samples\WinBase\Eventing\EtwConsumer\Output* directory.
1. Run the following command.

     `EtwConsumer LogFile.etl -xml`


