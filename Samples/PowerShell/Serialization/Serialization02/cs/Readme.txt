Serialization Sample 02
=======================
     By default, serialization preserves all public properties of an object. This sample looks at
     an existing .NET class and shows how to make sure that information from an instance of this
     class is preserved across serialization and deserialization when the information is not
     available in public properties of the class.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Setting serialization depth for a given .NET class
     2. Adding an extended property that carries information not available in public properties of
        the .NET class
     3. Hiding the extended property (an implementation detail needed for serialization) from users


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#
     - Types.ps1xml file


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Serialization02 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample binaries.
     3. Run Serialization02.exe
