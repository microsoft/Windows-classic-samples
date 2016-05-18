Coffee Tutorial Step 5

Demonstrates
============
CoffeeShop5 is the sixth sample application in a tutorial series named Coffee.
It uses a consistent coffee shop motif. Customers enter the shop, go to the
service counter, speak to order drinks or to enter the front office. The samples
are intended to demonstrate speech recognition capabilities within an
application. They are designed for the application-level (API) programmer and
for those not familiar with speech technology. Each sample will progressively
add new features and increase in complexity.

CoffeeShop5 expands the concepts of resources and resource management introduced
in CoffeeShop4. Using information that was learned by polling tokens about
available voices, CoffeeShop5 allows users to change the active voice. In doing
so, a dynamic grammar is used. In the previous Coffee samples, all the speech
commands were determined ahead of time and could not be changed. For example,
the drinks were limited to five basic types and a new one could not be added. A 
dynamic grammar allows adding or removing commands during the program execution.

To change the voices, enter the office by saying, "go to the office" or "enter
office." Once there, display the voice list by saying, "manage the employees." A
list of available voices will display on the right side of the screen. The
active voice will be indicated in red. To hear the employee speak, say, "hear
them speak." The statement "I will be the best employee you've ever had. Let me
work." will be spoken in the current voice.

To change the voice, say the voice name as it appears on the screen. For
example, if "Microsoft Mary" is displayed, say, "Microsoft Mary." The
highlighting will change to the selected voice. Having the employee speak will
do so in the voice. Additionally, the list of available voices may be filtered
by gender. The left side of the screen displays available commands for this. For
example, "Show males only," will display only the male voices.

Some voices may not be applicable to this example. For instance, Sample TTS
Voice is a composite voice for use with the SDK application MkVoice. The voice
contains only seven words with an eighth word being the default for all other
words. As a result, it will say "blah" most of the time. In the same way, the MS
Simplified Chinese Voice will spell the content rather than speak it.

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
CoffeeShop5.h           Contains the base definitions for the CoffeeShop5
                        tutorial application.

CoffeeShop5.cpp         Contains entry point for the CoffeeShop5 tutorial
                        application, as well as implementation of all
                        application features.

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

common.h                Contains the common definitions used in the CoffeeShop5
                        application.
                        
display.cpp             Contains the UI specifc code for the CoffeeShop5
                        application.
                        
resource.h              Microsoft Developer Studio generated include file. Used
                        by CoffeeShop5.rc.

coffee.xml              SAPI grammar file.

CoffeeShop5.rc          Resource scripts.
version.rc2

coffee.bmp              Bitmap resource.

CoffeeShop5.ico         Icon files.
small.ico

CoffeeShop5.sln         Microsoft Visual Studio solution file.

CoffeeShop5.vcproj      Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the CoffeeShop5.sln (solution) file to open the
       file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type CoffeeShop5.exe at the command line, or double-click the icon for
       CoffeeShop5.exe to launch it from Windows Explorer.
