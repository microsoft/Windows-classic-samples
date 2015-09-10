Hyper-V replication sample
==========================

This sample demonstrates how to use the Hyper-V WMI replication APIs to configure and control virtual machine replication.The sample demonstrates how to perform each of the following operations:

-   Enable or disable the replication service using the [**ModifyServiceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850101) method.
-   Start a replication using the [**StartReplication**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850303) method.
-   Create a test replica virtual machine using the [**TestReplicaSystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850305) method.
-   Initiate a failover for a virtual machine using the [**InitiateFailover**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850083) method.
-   Reverse a replication relationship for a virtual machine using the [**ReverseReplicationRelationship**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850289) method.
-   Remove an authorization entry using the [**RemoveAuthorizationEntry**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850271) method.
-   Remove a replication relationship for a virtual machine using the [**RemoveReplicationRelationshipEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280626) method.
-   Add an authorization entry using the [**AddAuthorizationEntry**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850014) method.
-   Set an authorization entry for a virtual machine using the [**SetAuthorizationEntry**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850293) method.
-   Create a replication relationship for a virtual machine using the [**CreateReplicationRelationship**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850036) method.
-   Change the replication state for a virtual machine using the [**RequestReplicationStateChangeEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280622) method.

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V replication API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850076)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Replica.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in several different modes. To obtain a list of the operations, use the following command line:

**ReplicaSamples.exe /?**

To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

Enable or disable the replication service
-----------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe ModifyReplicationService** {**0**|**1**}

    Pass 0 for the last parameter to disable the replication service or 1 to enable the replication service.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Start a replication
-------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe StartReplication** *VirtualMachineName*

    *VirtualMachineName* is the name of the virtual machine to start replicating.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Create a test replica virtual machine
-------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe TestReplicaSystem** *VirtualMachineName*

    *VirtualMachineName* is the name of the virtual machine to create a test replica virtual machine for.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Initiate a failover for a virtual machine
-----------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe InitiateFailover** *VirtualMachineName*

    *VirtualMachineName* is the name of the virtual machine to initiate a failover for.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Reverse a replication relationship for a virtual machine
--------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe ReverseReplicationRelationship** *VirtualMachineName*

    *VirtualMachineName* is the name of the virtual machine to reverse the replication relationship for.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Remove an authorization entry
-----------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe RemoveAuthorizationEntry** *FQDN*

    *FQDN* is the fully qualified domain name of the primary server to remove.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Remove a replication relationship for a virtual machine
-------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe RemoveReplicationRelationshipEx** *VirtualMachineName* **** *Relationship Type*

    *VirtualMachineName* is the name of the virtual machine to remove the replication relationship for.

    *Relationship Type* is the relationship type which should be removed. It should be either 0 or 1.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Add an authorization entry
--------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe AddAuthorizationEntry** *FQDN* **** *TrustGroup*

    where the parameters are as follows:

    -   *FQDN* is the fully qualified domain name of the primary server to add.
    -   *TrustGroup* identifies the group of trusted primary servers for the authorization entry.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Set an authorization entry for a virtual machine
------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe SetAuthorizationEntry** *VirtualMachineName* **** *FQDN*

    where the parameters are as follows:

    -   *VirtualMachineName* is the name of the virtual machine to set the authorization entry for.
    -   *FQDN* is the fully qualified domain name of the primary server.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Create a replication relationship for a virtual machine
-------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe CreateReplicationRelationship** *VirtualMachineName* **** *RecoveryServerName*

    where the parameters are as follows:

    -   *VirtualMachineName* is the name of the virtual machine to create a replication relationship for.
    -   *RecoveryServerName* is the name of the recovery server.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Change the replication state for a virtual machine
--------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ReplicaSamples.exe RequestReplicationStateChangeEx**  *VirtualMachineName*  *RequestedState RelationshipType*

    where the parameters are as follows:

    -   *VirtualMachineName* is the name of the virtual machine to change the replication state for.
    -   *RequestedState* is one of the values for the *RequestedState* parameter of the [**RequestReplicationStateChangeEx**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280622) method that specifies the new replication state.
    -   *RelationshipType* is the relationship type whose replication state is to be changed. It should be either 0 or 1.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

