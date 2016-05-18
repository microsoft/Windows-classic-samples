Coffee Tutorial Step 0

Demonstrates
============
CoffeeShop0 is the first sample application in a series called Coffee. It uses a
consistent coffee shop motif. You will eventually be able to enter the shop, go
to the order counter to order drinks, go to the gift store, or speak to
management. The samples are intended to demonstrate adding speech recognition to
an application. They are designed for the application-level (API) programmer and
for those not familiar with speech technology. Each sample will progressively
add new features and increase in complexity.

As the introductory sample, CoffeeShop0 is a simple application. There is one
window and a limited vocabulary from which to speak. If successfully recognized,
the window displays the response, "Please order when ready!" To keep the
application simple at this point, no other commands may be used. If the command
was not successfully recognized, no action will take place.

For example, you can say, "enter counter," "please enter counter," or
"please enter the counter." All three are recognizable commands. Since
CoffeeShop0 can only take you to the ordering counter, "please enter the shop,"
and "please enter the store," will have the same effect.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop0.h           Contains the base definitions for the coffee tutorial
                        application.

CoffeeShop0.cpp         Contains entry point for the coffee tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop0
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop0
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop0.rc.

coffee.xml              SAPI grammar file.

CoffeeShop0.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop0.ico         Icon files.
small.ico

CoffeeShop0.sln         Microsoft Visual Studio solution file.

CoffeeShop0.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop0.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop0.exe at the command line, or double-click the icon for
       CoffeeShop0.exe to launch it from Windows Explorer.
