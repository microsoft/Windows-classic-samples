Cluster Aware Updating plug-in sample
=====================================

This sample demonstrates a Cluster Aware Updating (CAU) plug-in that runs an arbitrary, administrator-specified, command on each cluster node.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[**ClusterAwareUpdating**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh873034)

Operating system requirements
-----------------------------

Client

None supported

Server

Windows Server 2012 R2

Build the sample
----------------

This sample requires the ClusterAwareUpdating.dll assembly which is available for Windows 8.1 in the RSAT package download. The FailOver Cluster feature must be installed in order to build this sample on Windows Server 2012 R2.

To build this sample, open the solution (.sln) file titled CauSamplePlugin.sln from Visual Studio Professional 2012, Visual Studio 2013, or a later version (any SKU). Press F6 or go to **Build-\>Build Solution** from the top menu after the sample has loaded.

**Warning**  This sample requires Visual Studio Professional 2012, Visual Studio 2013, or later versions of Visual Studio (any SKU) and does not compile in Microsoft Visual Studio Express 2013 for Windows. It also requires .NET Framework 4.5.

Run the sample
--------------

The CAU plug-in has one required argument, Command, which is the command that will be run. Here's an example command line.

**Invoke-CauRun -ClusterName MyTestCluster -CauPluginName FabrikamCauPlugin -CauPluginArguments @{ "Command"="cmd.exe /c echo Hello." } -Verbose**

**Important**  The "Command" string is case-sensitive, so "command" returns an error.

When the plug-in performs a scan, it detects a single "update," which is the command to run, applicable to each node. Staging doesn't do anything except report that it's ready to install the update (run the command). When the plug-in "installs" the update, it runs the command on the target machine using WMI, and waits for it to exit. If the remote process exits with a non-zero error code, the update is considered to have failed.

Because this plug-in uses WMI, it requires that your firewalls on both the orchestrator machine and the cluster nodes have the appropriate rules enabled. It's easy to overlook that the orchestrator must allow inbound WMI connections in order to receive events from the cluster nodes when the commands finish. The plug-in tests its ability to use WMI with each node.

