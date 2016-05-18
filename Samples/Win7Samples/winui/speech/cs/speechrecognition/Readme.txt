SpeechRecognition

Demonstrates
============
SpeechRecognition demonstrates the two speech recognition classes that are
present in System.Speech.Recognition namespace. System's default shared speech
recognizer can be used by SpeechRecognizer class. Users have limited control
over this shared recognizer. In Windows 7 the settings of the shared
recognizer can be accessed in Control Panel under "Ease of Access -> Speech
Recognition Options -> Advanced Speech Options". Creating a SpeechRecognizer
object will also activate the Windows Speech Recognition if it's not running
already. For more control over speech recognition, SpeechRecognitionEngine class
can be used.

This sample also demostrates how to load and use dictation grammars, how to use
the GrammarBuilder class to easily create grammars programatically and how to
load SRGS grammars from files. The events that are generated during the
recognition process are also logged to inform the user.
 
Sample Language Implementations
===============================
This sample is available in C#.

Files
=====
SpeechRecognition.xaml          The Application Markup file for
                                SpeechRecognition application.

SpeechRecognitionWindow.xaml    The Application Markup file for
                                SpeechRecognitionWindow class.

SpeechRecognitionWindow.cs      Implementation details for
                                SpeechRecognitionWindow class.

Properties\AssemblyInfo.cs      General information about SpeechRecognition
                                assembly.
                                
Info.xaml                       Markup file which defines the FlowDocument that
                                is displayed by the application.
                                    
SpeechRecognition.csproj        Visual C# project file.

SpeechRecognition.sln           Microsoft Visual Studio solution file.

Readme.txt                      This file.

To build the sample using the command prompt:
=============================================
    1. Open the Command Prompt window and navigate to the directory.
    2. Type "msbuild".

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the SpeechRecognition.sln (solution) file
       to open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The sample will be built in the
       "bin\Debug" or "bin\Release" directory.
    
To run the sample:
=================
    1. Navigate to the directory that contains the executable, using the command
       prompt or Windows Explorer.
    2. Type SpeechRecognition.exe at the command line, or double-click the icon
       for SpeechRecognition.exe to launch it from Windows Explorer.

Note:
====
The SpeechRecognition sample utilizes the Windows Presentation Foundation (WPF)
available in .NET Framework 3.0. Compiling this sample requires .NET Framework
3.0 or higher. 

Known issues:
============
The Windows Speech Recognition dictation scratchpad might pop up when running the 
SpeechRecognition sample while Windows Speech Recognition is listening. The 
dictation scratchpad is a new feature in Windows 7 that allows for dictation in 
any Windows application that supports text input. 
