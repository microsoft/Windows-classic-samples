Windows Animation Priority Comparison Sample


Demonstrates
===============================
This sample shows how to use Windows Animation with your own Priority Comparison. This sample uses Direct2D for rendering.


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
* PriorityComparison.h: The implementation of the custom priority comparison
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
    1. Execute PriorityComparison.exe

To use the application:
=======================
    1. Images are loaded from the Pictures Library
    2. Resize the window or press the space bar and the images will move to the center
    3. Use the left and right arrow keys to select different images (the faster the key is pressed the faster the selection will change)
    4. Use the up and down arrow keys to create a wave through the images (the faster the key is pressed the faster the wave)
