---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Network List Manager API sample
urlFragment: network-list-manager-api-sample
description: Demonstrates how to use Windows APIs to determine the network connectivity status, and shows how to use this information to determine when to attempt to connect to the Internet.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Network List Manager API sample

Apps can use INetworkListManager APIs to check the network connectivity status before
attempting to connect to the Internet. This check is recommended as it saves apps the complex task
of determining connectivity for various network configurations on their own. However this check
is not required as higher-level APIs (HttpClient, etc.) provide network connectivity status
through failures/results at point of connection.

This sample also demonstrates how to register for network connectivity change notifications.
Apps can listen to the notifications published by Windows instead of building their
own state machine to track network connectivity changes.

## System requirements

- Requires Windows SDK 10.0.22621.0 to build and Windows 8 to run.

## Build the sample

1. Open File Explorer and navigate to the directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## Run the sample

1. Press F5 in Visual Studio or later.
