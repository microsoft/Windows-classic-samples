Windows Animation Grid Layout Sample


Demonstrates
===============================
This sample shows how to use Windows Animation, using Direct2D to animate a grid of images.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
    C++


Files
===============================
* LayoutManager.h: The header file for the LayoutManager class
* LayoutManager.cpp: The implementation of the LayoutManager class
* MainWindow.h: The header file for the MainWindow class
* MainWindow.cpp: The implementation of the MainWindow class
* ManagerEventHandler.h: The implementation of the ManagerEventHandler
* Thumbnail.h: The header file for the Thumbnail class
* Thumbnail.cpp: The implementation of the LayoutManager class
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
    1. Execute GridLayout.exe

To use the application:
=======================
    1. Images are loaded from the Pictures Library
    2. Resize the window and the images will arrange themselves into a grid
