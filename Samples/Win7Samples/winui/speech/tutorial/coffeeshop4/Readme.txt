Coffee Tutorial Step 4

Demonstrates
============
CoffeeShop4 is the fifth sample application in a tutorial series named Coffee.
It uses a consistent coffee shop motif. Customers enter the shop, go to the
service counter, speak to order drinks or to enter the front office. The samples
are intended to demonstrate speech recognition capabilities within an
application. They are designed for the application-level (API) programmer and
for those not familiar with speech technology. Each sample will progressively
add new features and increase in complexity.

CoffeeShop4 introduces the concepts of resources and resource management. SAPI
stores information in the form of tokens. These tokens are used later to
instantiate features such as voices and recognizers. However, programmers can
query SAPI for the presence of tokens to learn more about available features.
For example, each available voice is kept as a token.

CoffeeShop4 displays available voices and may even speak using the currently
active voice. Three new commands are used. To do so, enter the office by saying,
"go to the office" or "enter office." Once there, display the voice list by
saying, "manage the employees." A list of available voices will display on the
right side of the screen. The active voice will be indicated in red.

To have the employee speak, say, "hear them speak." The statement "I will be the
best employee you've ever had. Let me work." will be spoken in the current
voice. The voice may be changed using Speech properties in Control Panel.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop4.h           Contains the base definitions for the CoffeeShop4
                        tutorial application.

CoffeeShop4.cpp         Contains entry point for the CoffeeShop4 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop4
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop4
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop4.rc.

coffee.xml              SAPI grammar file.

CoffeeShop4.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop4.ico         Icon files.
small.ico

CoffeeShop4.sln         Microsoft Visual Studio solution file.

CoffeeShop4.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop4.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop4.exe at the command line, or double-click the icon for
       CoffeeShop4.exe to launch it from Windows Explorer.
