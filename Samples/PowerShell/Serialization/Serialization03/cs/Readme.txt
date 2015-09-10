Serialization Sample 03
=======================
     This sample looks at an existing .NET class and shows how to make sure that instances of this
     class and of derived classes are deserialized (rehydrated) into live .NET objects.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Setting serialization depth for a given .NET class
     2. Creating a type converter that can rehydrate a deserialized property bag into a live .NET
        object
     3. Declaring that deserialized property bags of a given .NET class need to be rehydrated using
        the type converter


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#
     - Types.ps1xml file


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Serialization03 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample binaries.
     3. Run Serialization03.exe
