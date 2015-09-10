Windows PowerShell Event Registration Sample
============================================

This sample shows how to create a cmdlet for event registration by deriving from [**ObjectEventRegistrationBase**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd432311).

The sample creates the `Register-FileSystemEvent` cmdlet which subscribes to events raised by [**System.IO.FileSystemWatcher**](http://msdn.microsoft.com/en-us/library/windows/desktop/x7t1d0ky). With this cmdlet, users can register an action to execute when a specific event is raised, such as when a file is created under an specific directory.

**Sample Objectives**

This sample demonstrates the following:

-   How to how to derive from the [**ObjectEventRegistrationBase**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd432311) class to create a cmdlet for event registration.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**System.IO.FileSystemWatcher**](http://msdn.microsoft.com/en-us/library/windows/desktop/x7t1d0ky)

[**ObjectEventRegistrationBase**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd432311)

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
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Start PowerShell and import the library file in order to make the `Register-FileSystemEvent` cmdlet available in PowerShell.
2.  Use the `Register-FileSystemEvent` cmdlet to register an action that will write a message when a file is created under the TEMP directory.
3.  Create a file under the TEMP directory and note that the action is executed (i.e. the message is displayed).

This is the sample output from executing these 3 steps:

<table>
<colgroup>
<col width="100%" />
</colgroup>
<thead>
<tr class="header">
<th align="left">cmd</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td align="left"><pre><code>PS&gt; Import-Module .\bin\Debug\Events01.dll
PS&gt; Register-FileSystemEvent $env:temp Created -filter &quot;*.txt&quot; -action { Write-Host &quot;A file was created in the TEMP directory&quot; }

Id  Name            State      HasMoreData  Location  Command
--  ----            -----      -----------  --------  -------
1   26932870-d3b... NotStarted False                  Write-Host &quot;A f...

PS&gt; Set-Content $env:temp\test.txt &quot;This is a test file&quot;
A file was created in the TEMP directory
PS&gt;</code></pre></td>
</tr>
</tbody>
</table>


