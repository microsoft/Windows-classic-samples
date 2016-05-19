Coffee Tutorial Step 2

Demonstrates
============
CoffeeShop2 is the third sample application in a tutorial series named Coffee.
It uses a consistent coffee shop motif. Customers enter the shop, go to the
service counter, speak to order drinks or to enter the front office. The samples
are intended to demonstrate speech recognition capabilities within an
application. They are designed for the application-level (API) programmer and
for those not familiar with speech technology. Each sample will progressively
add new features and increase in complexity.

As the third example, CoffeeShop2 builds on the framework of its predecessors.
There is one window and an expanded vocabulary from which to order drinks and
move around the business. In addition to placing an order, you can move to both
the counter and the office. However, drinks may not be requested while in the
office.

CoffeeShop2 allows you to speak any of the CoffeeShop1 commands. If successful,
the window displays the action such as "please order when ready!" If the command
was not successfully recognized, no action will take place.

A new navigation command has been added allowing you to visit the office. You
may request "go to the office." The screen will change to "Welcome to the SAPI
coffee shop office." Drinks may not be ordered while in the office. As a result,
drink commands are unavailable while in the office. To leave, navigate to
another part of the business. Like CoffeeShop1, navigation commands involving
the counter, store, and shop take you to the same place.

A second new feature asks you to repeat the order if it is not understood. If a
recognition takes place but the word or phrase is not in the command list,
CoffeeShop2 will display "Sorry, I didn't get that order. Please try again."

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop2.h           Contains the base definitions for the CoffeeShop2 tutorial
                        application.

CoffeeShop2.cpp         Contains entry point for the CoffeeShop2 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop2
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop2 application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop2.rc.

coffee.xml              SAPI grammar file.

CoffeeShop2.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop2.ico         Icon files.
small.ico

CoffeeShop2.sln         Microsoft Visual Studio solution file.

CoffeeShop2.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop2.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop2.exe at the command line, or double-click the icon for
       CoffeeShop2.exe to launch it from Windows Explorer.
