Coffee Tutorial Step 6

Demonstrates
============
In CoffeeShop6, the last of the tutorial chapters, no new programming code is
introduced, per se. Rather, CoffeeShop6 makes a variation on an existing theme.
The past several Coffee tutorials demonstrated working with context-free
grammars, also called grammar rules. In short, they were predetermined lists of
words that needed to be matched exactly. Even dynamic grammars, though more
flexible, still had to match exact words once the word list was determined. For
all the promise of speech recognition, using the models presented so far, you
have not been able to dictate or use free-formed speech. With CoffeeShop6, you
can use unrestrained speech in your applications. It uses the simple case of
renaming the coffee shop to anything you want.

For instance, go to office and give the order "manage store name." A new screen
displays and if you follow the instructions, you can say, "Rename the coffee
shop to" and provide any name you want. CoffeeShop6 will echo back the new name
as "Welcome to the X coffee shop," X, of course, being the moniker. The name
will even display in all in the subsequent windows.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop6.h           Contains the base definitions for the CoffeeShop6
                        tutorial application.

CoffeeShop6.cpp         Contains entry point for the CoffeeShop6 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop6
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop6
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop6.rc.

coffee.xml              SAPI grammar file.

CoffeeShop6.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop6.ico         Icon files.
small.ico

CoffeeShop6.sln         Microsoft Visual Studio solution file.

CoffeeShop6.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop6.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop6.exe at the command line, or double-click the icon for
       CoffeeShop6.exe to launch it from Windows Explorer.
