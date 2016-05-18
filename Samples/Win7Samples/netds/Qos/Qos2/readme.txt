QOSSAMPLE – QOS2 sample program

SUMMARY
This sample is an executable that demonstrates the use of the QOS2 api. This 
program implements a simple traffic generator which uses the QOS2 api to not 
overload the network path between the source and its destination.

Although the application can be run as a receiver to process the traffic
generated, the client side of the application is the important part of this
sample. 

BUILDING THE SAMPLE
Run the build command from this directory to build the sample, or build it 
using Visual Studio 2005. This sample can be used on any Vista supported 
platform. All the files in this directory are required, and it produces a single 
file: QOSSAMPLE.EXE. 

RESOURCES
Not applicable. 

CODE TOUR
File Manifest
Files               Description
makefile            The generic makefile for building the code sample outside of 
                    Visual Studio
qossample.rc        The resource file for the code sample
qossample.c         Code for the sample
qossample.vcproj    Visual Studio 2005 project file
qossample.sln       Visual Studio 2005 solution file
SOURCES             The generic file for building the code sample outside of 
                    Visual Studio
readme.txt          This file

