Script Line Profiler Sample
===========================
     This sample shows how to create a script line profiler using the Windows PowerShell AST 
     (Abstract Syntax Tree) support. The sample defines the InstrumentAst class (derived from 
     the ICustomAstVisitor interface) that builds an AST object. The AST object inserts a Profile
     object callback that measures the execution time of each script statement. The profiler 
     can be run on script files using the provided Measure-Script cmdlet.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a script line profiler using the Windows PowerShell AST support.
     2. Running the provided Measure-Script cmdlet to profile a script file.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open Windows Explorer and navigate to the PSProfiler directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. Make sure the PSProfiler project references the System.Management.Automation.dll assembly.
     4. In the Build menu, select Build Solution.
     5. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Run Import-Module with the full path to the sample DLL.
     2. Run the Measure-Script cmdlet on a script file
