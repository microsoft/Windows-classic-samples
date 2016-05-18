Windows Animation App Driven Sample


Demonstrates
===============================
This sample shows how to use Windows Animation in the Application-Driven configuration, using Direct2D to animate
the background color of a window and syncing to the refresh rate.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
    C++


Files
===============================
* MainWindow.h: The header file for the MainWindow class
* MainWindow.cpp: The implementation of the MainWindow class
* ManagerEventHandler.h: The implementation of the ManagerEventHandler
* UIAnimationHelper.h: The header file for various UIAnimation helper functions
* Application.cpp: Contains the application entry point


Building the Sample
===============================

To build the sample using the command prompt:
=============================================
    1. Open a Command Prompt window and navigate to the solution directory
    2. Type msbuild

To run the application using the command prompt:
================================================
    1. Execute AppDriven.exe

To use the application:
=======================
    1. Left click anywhere in the client area and the background color of the window will animate to a random color
