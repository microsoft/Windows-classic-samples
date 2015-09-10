Hyper-V networking sample
=========================

This sample demonstrates how to use the Hyper-V WMI networking APIs to create, configure, and remove networking related objects such as switches and ports.The sample demonstrates how to perform each of the following operations:

-   Create a network switch using the [**DefineSystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850040) method.
-   Delete a network switch using the [**DestroySystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850044) method.
-   Modify an existing network switch using the [**ModifySystemSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850103) method.
-   Add network ports using the [**AddResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850019) method, and remove network ports using the [**RemoveResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850277) method.
-   Modify an existing network port using the [**ModifyResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850098) method.
-   Gather and display information about a network switch using various classes and methods.
-   Determine if an external adapter supports trunk mode using the [**Msvm\_VLANEndpoint**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850260) class.
-   Connect a network adapter to a virtual machine, disconnect a network adapter from a virtual machine, or modify an existing a connected network adapter.
-   Add, modify, or remove feature settings for a virtual machine's connections.
-   Connect a virtual machine to a network adapter from a resource pool.
-   Enable, disable, or reorder a network switch extension.
-   Modify the list of required features on all connections associated with a virtual machine.
-   Add, modify, or remove advanced properties for a network switch.

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V networking API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850074)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Networking.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in several different modes. To obtain a list of the operations, use the following command line:

**NetworkingSamples.exe /?**

To obtain the parameters for each command, use the following command line format:

**NetworkingSamples.exe** *command* **/?**

where *command* is the specific command to obtain the parameters for. For example, the following command line will list the parameters for the `CreateSwitch` operation:

**NetworkingSamples.exe CreateSwitch /?**

To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

### Create a network switch

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe CreateSwitch** *Type* **** [*ExternalNetwork*] **** [*SwitchName*] **** [*SwitchNotes*]

    where the parameters are as follows:

    -   *Type* specifies the type of switch to create. This can be one of the following values:
        -   "Private"
        -   "Internal"
        -   "ExternalOnly"
        -   "External"
    -   *ExternalNetwork* the name of the external network. This parameter is required when *Type* is "External" or "ExternalOnly", and must not be present otherwise.
    -   *SwitchName* is the optional name for the switch. A default name will be used if this parameter is not specified.
    -   *SwitchNotes* is the optional notes for the switch. This string must be within quotes if it contains any spaces.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Delete a network switch

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe DeleteSwitch** *SwitchName*

    where *SwitchName* is the name of the switch to delete.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify an existing network switch

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe ModifySwitch** *SwitchName* **** *NewSwitchName* **** [*SwitchNotes*]

    where the parameters are as follows:

    -   *SwitchName* is the name of the switch to modify.
    -   *NewSwitchName* is the new name of the switch to be applied.
    -   *SwitchNotes* is the optional new notes for the switch. This string must be within quotes if it contains any spaces.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Add or remove network ports

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe AddAndRemovePorts** *Command* **** *SwitchName* **** [*ExternalNetwork*]

    where the parameters are as follows:

    -   *Command* is the command to execute and must be one of the following values:
        -   "Add" adds an external connection to *ExternalNetwork*.
        -   "Remove" removes the ports from *SwitchName*.
    -   *SwitchName* is the name of the switch.
    -   *ExternalNetwork* is the name of the external network to add a connection to. This parameter is not used if *Command* is "Remove".

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify network ports

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe ModifyPorts** *SwitchName* **** *ExternalNetwork*

    where the parameters are as follows:

    -   *SwitchName* is the name of the switch to modify the ports for.
    -   *ExternalNetwork* is the name of the external network to connect the ports to.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display information about a network switch

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe EnumerateSwitch** *SwitchName*

    where *SwitchName* is the name of the switch to display the information for.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Determine if an external adapter supports trunk mode

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe SupportsTrunkMode** *ExternalNetwork*

    where *ExternalNetwork* is the name of the external adapter in question.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Connect a network adapter to a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe ConnectVmToSwitch** {**Connect**|**Disconnect**|**Modify**} **** *VirtualMachineName* **** *SwitchName* **** [*NewSwitchName*]

    The first parameter is one of the following:

    -   "Connect": Adds *SwitchName* to *VirtualMachineName*. *NewSwitchName* is not used.
    -   "Disconnect": Removes *SwitchName* from *VirtualMachineName*. *NewSwitchName* is not used.
    -   "Modify": Changes the switch name from *SwitchName* to *NewSwitchName*.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Add, modify, or remove feature settings for a virtual machine's connections

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe AdvancedConnectionProperties** {**Add**|**Modify**|**Remove**} **** *VirtualMachineName* **** [*FeatureSetting*]

    The first parameter is one of the following:

    -   "Add": Adds *FeatureSetting* to all connections from *VirtualMachineName*. *FeatureSetting* can be either "acl" or "security".
    -   "Modify": Changes the security setting for each connection from *VirtualMachineName* to enable MAC spoofing . *FeatureSetting* is not used.
    -   "Remove": Removes *FeatureSetting* from all connections from *VirtualMachineName*. *FeatureSetting* can be either "acl" or "security".

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Connect a virtual machine to a network adapter from a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe ConnectVmUsingResourcePool** *VirtualMachineName* **** *ResourcePoolName*

    where the parameters are as follows:

    -   *VirtualMachineName* is the name of the virtual machine.
    -   *ResourcePoolName* is the name of the resource pool to use for the connection.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Enable, disable, or reorder a network switch extension

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe ManageExtension** *SwitchName* **** *ExtensionName* **** *Command*

    where the parameters are as follows:

    -   *SwitchName* is the name of the switch.
    -   *ExtensionName* is the name of the switch extension.
    -   *Command* is the command to execute on the switch extension. This must be one of the following values.
        <table>
        <colgroup>
        <col width="50%" />
        <col width="50%" />
        </colgroup>
        <thead>
        <tr class="header">
        <th align="left">Command
        Description</th>
        </tr>
        </thead>
        <tbody>
        <tr class="odd">
        <td align="left"><p>&quot;Enable&quot;</p>
        <p>Enable the switch extension.</p></td>
        <td align="left"><p>&quot;Disable&quot;</p>
        <p>Disable the switch extension.</p></td>
        </tr>
        </tbody>
        </table>

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the list of required features on all connections associated with a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe SetRequiredFeature** *VirtualMachineName* **** *FeatureName* **** {**true**|**false**}

    where the parameters are as follows:

    -   *VirtualMachineName* is the name of the virtual machine.
    -   *FeatureName* is the name of the feature.
    -   The last parameter is "true" to make the feature required, or "false" to not make the feature required.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Add, modify, or remove advanced properties for a network switch

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **NetworkingSamples.exe AdvancedSwitchProperties** {**Add**|**Modify**|**Remove**} **** *SwitchName*

    The first parameter is one of the following:

    -   "Add": Adds bandwidth settings to *SwitchName* to set the **DefaultFlowReservation** property of the [**Msvm\_VirtualEthernetSwitchBandwidthSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850243) class.
    -   "Modify": Increases the **DefaultFlowReservation** property of the [**Msvm\_VirtualEthernetSwitchBandwidthSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850243) class for *SwitchName* by 1Mbps.
    -   "Remove": Removes the bandwidth settings from *SwitchName*.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

