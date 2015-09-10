Windows PowerShell Script Line Profiler Sample
==============================================

This sample shows how to create a script line profiler using the new Windows PowerShell 3.0 AST (Abstract Syntax Tree) support.

The sample defines the InstrumentAst class (derived from the [**ICustomAstVisitor**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485441) interface) that builds an AST object. The AST object inserts a Profile object callback that measures the execution time of each script statement. The profiler can be run on script files using the provided `Measure-Script` cmdlet.

**Sample Objectives**

1.  Creating a script line profiler using Windows PowerShell
2.  AST support.
3.  Running the provided `Measure-Script` cmdlet to profile a script file.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**ICustomAstVisitor**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485441)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Make sure the PSProfiler project references the System.Management.Automation.dll assembly.
4.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The executable will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Run `Import-Module` with the full path to the sample DLL.
2.  Run the `Measure-Script` cmdlet on a script file.

