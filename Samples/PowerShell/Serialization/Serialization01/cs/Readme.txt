Serialization Sample 01
=======================
     This sample looks at an existing .NET class and shows how to make sure that information from
     selected public properties of this class is preserved across serialization and
     deserialization. The sample uses a types.ps1xml file to declare which properties of the class
     should get serialized.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Setting serialization depth for a given .NET class
     2. Restricting which set of properties from a class are serialized
     3. Demonstrating the effects of using a types.ps1xml file and DeserializingTypeConverter


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#
     - Types.ps1xml file


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Serialization01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample binaries.
     3. Run Serialization01.exe
