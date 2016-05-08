Coffee Tutorial Step 3

Demonstrates
============
CoffeeShop3 is the fourth sample application in a tutorial series named Coffee.
It uses a consistent coffee shop motif. Customers enter the shop, go to the
service counter, speak to order drinks or to enter the front office. The samples
are intended to demonstrate speech recognition capabilities within an
application. They are designed for the application-level (API) programmer and
for those not familiar with speech technology. Each sample will progressively
add new features and increase in complexity.

CoffeeShop3 is identical to the previous CoffeeShop2 sample. However, a
synthesized voice has been used for this module. At certain times, the
text-to-speech engine will speak the command, an order, or a greeting. For
example, the initial screen will not only display the greeting, but will also
say, "Welcome to the SAPI coffee shop. Speak for service."

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop3.h           Contains the base definitions for the CoffeeShop3
                        tutorial application.

CoffeeShop3.cpp         Contains entry point for the CoffeeShop3 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop3
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop3
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop3.rc.

coffee.xml              SAPI grammar file.

CoffeeShop3.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop3.ico         Icon files.
small.ico

CoffeeShop3.sln         Microsoft Visual Studio solution file.

CoffeeShop3.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop3.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop3.exe at the command line, or double-click the icon for
       CoffeeShop3.exe to launch it from Windows Explorer.
