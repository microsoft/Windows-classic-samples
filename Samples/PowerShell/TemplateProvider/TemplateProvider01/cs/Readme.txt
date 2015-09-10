Template Provider 01 Sample
===========================
     This sample creates a template for a provider that hooks into the Windows PowerShell
     namespaces. It contains all possible provider overrides and interfaces. A provider developer
     can copy this file, change the name of the file, delete those interfaces and methods the 
     provider doesn't need to implement or override, and use the remaining code as a template
     to create a fully functional provider.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. How to create a TemplateProvider class that implements the following interfaces:
         - NavigationCmdletProvider
         - IPropertyCmdletProvider
         - IContentCmdletProvider
         - IDynamicPropertyCmdletProvider
         - ISecurityDescriptorCmdletProvider


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to TemplateProvider01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
     4. See the output results and the corresponding code.
