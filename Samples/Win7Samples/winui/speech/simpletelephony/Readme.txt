Simple Telephony

Demonstrates
============
Simple Telephony is a speech recognition (SR) and text-to-speech (TTS)
application that uses telephony (TAPI) interfaces. It is intended for use with a
phone system, allowing you to dial in and talk to Simple Telephony.

The functionality of Simple Telephony is very similar to the SDK sample
TalkBack. When prompted, you may speak a word, phrase, or a short sentence and
Simple Telephony will attempt recognition. It then speaks the result using a
synthesized voice. It will also play back your recorded voice so that you can
confirm the accuracy of the recognition.

The application's interface is simple. The phone line or connection displays the
status. Commonly it will display "Waiting for a call..." or "Answering..." or
"Connected." Several error messages may also be displayed.

Simple Telephony will not automatically open if a phone call is made to it. It
must be already running prior to answering calls. If Auto Answer is selected,
Simple Telephony will pick up the call; otherwise, you must click the Answer
button. Simple Telephony will greet you and ask you to speak something. After
the recognition and subsequent playbacks, the call will automatically terminate.
Only one recognition is allowed per session.

Prerequisites
=============
You must have a modem, since it is a telephony application. There is no
restriction on the type or speed of modem, although it must be properly
connected and able to process data. Follow the instructions provided by the
modem manufacturer for specific loading and installation details. 

Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
operator.h              Definition of the COperator class

operator.cpp            Contains the entry point for the SimpleTelephony
                        application and definitions of methods of COperator
                        class.
                        
callnot.h               Declaration of the CTAPIEventNotification class which
                        implements the TAPI 3.0 interface
                        ITTAPIEventNotification.

callnot.cpp             Implementation of the CTAPIEventNotification class.

resource.h              Microsoft Developer Studio generated include file. Used
                        by SimpleTelephony.rc.

SimpleTelephony.rc      Resource scripts.
version.rc2

SimpleTelephony.ico     Icon file.

SimpleTelephony.sln     Microsoft Visual Studio solution file.

SimpleTelephony.vcproj  Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the SimpleTelephony.sln (solution) file to open
       the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type SimpleTelephony.exe at the command line, or double-click the icon
       for SimpleTelephony.exe to launch it from Windows Explorer.
