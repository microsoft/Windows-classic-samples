---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: DUSM network cost sample
description: The network cost sample demonstrates the features of Data Usage and Subscription Management (DUSM).
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# DUSM network cost sample

The network cost sample demonstrates the features of Data Usage and Subscription Management (DUSM).
This sample allows the user to get local machine cost, destination cost and connection cost. The user can register for cost change
notifications for machine cost, destination cost and connection cost and receive the new cost when there is a cost change event.

These are few points to note about this sample app:

1. The DUSM feature API allows the user to register for cost change notifications for multiple destination addresses, using
SetDestinationAddresses; although the NetCostSample SDK restricts the user to register for cost change notifications for single
destination address at a time. This is done to make the sample SDK simple.

2. When the user registers for cost change notifications, since the events are out-of-band this sample, the events have to be generated
by doing one of the following:

    - Connect or disconnect Ethernet
    - Switch between mobile broadband and Wi-Fi
    - Change cost for mobile broadband using WCM Sample SDK which defines the use of set cost feature for WCM API.

3. While the DUSM feature allows the user to register for cost change notifications for multiple destination addresses, this sample restricts registration for cost change notifications to a single destination address at a time for the sake of simplicity.

**Note** The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://developer.microsoft.com/windows/). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

To get a copy of Windows, go to [Downloads and tools](https://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](https://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[**INetworkCostManager**](https://learn.microsoft.com/windows/win32/api/netlistmgr/nn-netlistmgr-inetworkcostmanager)

[**INetworkConnectionCostEvents**](https://learn.microsoft.com/windows/win32/api/netlistmgr/nn-netlistmgr-inetworkconnectioncostevents)

## Related technologies

[Network List Manager](https://learn.microsoft.com/windows/win32/nla/portal)

## System requirements

- Requires Windows SDK 10.0.22621.0 to build and Windows 8.1 to run.

## Build the sample

1. Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2. Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.

3. Press F6 or use **Build** \> **Build Solution** to build the sample.

## Run the sample

To debug the app and then run it, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.
