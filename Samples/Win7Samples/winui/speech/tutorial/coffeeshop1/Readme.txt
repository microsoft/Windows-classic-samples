Coffee Tutorial Step 1

Demonstrates
============
CoffeeShop1 is the second sample application in a tutorial series named Coffee.
It uses a consistent coffee shop motif. You will eventually be able to enter the
shop, go to the order counter to order drinks, go to the gift store, or speak to
management. The samples are intended to demonstrate speech recognition
capabilities within an application. They are designed for the application-level
(API) programmer and for those not familiar with speech technology. Each sample
will progressively add new features and increase in complexity.

As the second example, CoffeeShop1 builds on the framework of CoffeeShop0. There
is one window and although there is a limited vocabulary from which to speak, it
is expanded from CoffeeShop0. CoffeeShop0 restricted you to moving to the
counter only. In contrast, CoffeeShop1 now lets you order one of a variety of
coffee drinks. After opening CoffeeShop1, you may speak any of the commands. If 
successfully recognized, the window displays the response, "Please order when
ready!" If the command was not successfully recognized, no action will take
place.

Commands are grouped into two major (or top-level) categories: navigation and
drink ordering. Navigation allows you to move around the coffee shop to places
such as the counter, shop, or store. In CoffeeShop1, the effects of going to any
location are minimal and any navigation command takes you to the counter. The
distinction allows the code sample to demonstrate additional SR features.
Subsequent Coffee examples will expand on this. Drink ordering allows you to
place drink requests.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop1.h           Contains the base definitions for the CoffeeShop1
                        tutorial application.

CoffeeShop1.cpp         Contains entry point for the CoffeeShop1 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop1
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop1
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop1.rc.

coffee.xml              SAPI grammar file.

CoffeeShop1.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop1.ico         Icon files.
small.ico

CoffeeShop1.sln         Microsoft Visual Studio solution file.

CoffeeShop1.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop1.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop1.exe at the command line, or double-click the icon for
       CoffeeShop1.exe to launch it from Windows Explorer.
