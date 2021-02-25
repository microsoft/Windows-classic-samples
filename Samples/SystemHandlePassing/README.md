---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: System handle passing sample
urlFragment: system-handle-passing
description: Demonstrates how to pass kernel handles using COM.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# System handle passing sample

This sample demonstrates how to pass handles to system objects between a client and server application using COM.

A server application is created and registered that will start up and accept an incoming remote procedure call from a client application.

The server will receive a handle to a file and an event. It will save these handles for later and enqueue a background task, returning another event for when it's done with the final touches. The background task waits for the event to be signaled, then writes data to the file, signals the final event and exits the task thread.

The client application will start up, open a file, and create an event. It will send these handles to the server application and receive another event back. Then the client will fill out the file with some text and signal the event so the server can put its final touches on the file. The client waits on the event received from the server to know when it's done working and then prints a joyful message and exits.

Here's a summary of the timeline:

|Client                 |Server                 |
|-----------------------|-----------------------|
|create file handle     |                       |
|create event handle 1  |                       |
|call server            |                       |
|                       |receive file handle    |
|                       |receive event handle 1 |
|                       |create event handle 2  |
|                       |return to client       |
|receive event handle 2 |                       |
|                       |wait for event handle 1|
|signal event handle 1  |                       |
|                       |write to file handle   |
|wait for event handle 2|                       |
|                       |signal event handle 2  |
|clean up               |clean up               |

## Related topics

- [system_handle attribute](https://docs.microsoft.com/windows/win32/midl/system-handle)

## Operating system requirements

### Client

Windows 10 Anniversary Update (version 1607, build 14393)

### Server

Windows Server 2016 (build 14393)

## Build the sample

### Pre-Requisites

1. A copy of the [Windows Implementation Library](https://github.com/microsoft/wil) is required for this sample. The solution and project files are pre-configured to retrieve it as a NuGet package.

### To build the sample by using Visual Studio 2019 or later (preferred method)

1. Open Windows Explorer and navigate to the directory.
1. Double-click the icon for the **.sln** (solution) file to open the file in Visual Studio.
1. In the **Build** menu, select **Build Solution**. The application is built in the default `\Debug` or `\Release` directory.

### To build the sample by using the command line

1. Open the **Command Prompt** window and navigate to the directory.
1. Type **msbuild systemhandlepassing.sln**.

## Run the sample

### Pre-requisites

1. Find and run an instance of PowerShell.
1. Switch into the solution directory for this sample (the `cpp` directory where the `.sln` file lives).
1. Identify the output directory for the binaries built from the build step. This will be something like `x64\Release` or `bin\x86\Debug` relative to the solution directory. This parameter is required to tell the script where to look for registration.

### To run the sample

1. Navigate to the solution directory.
1. Register the COM Proxy, Interfaces, and Out-of-Proc Class Server: `PowerShell.exe -ExecutionPolicy Unrestricted -File .\helpers.ps1 -register -path x64\release`. Change `x64\release` in the `-path` parameter to the output directory identified earlier.
1. Navigate to the build output path and run `.\client.exe`.
1. Enter a file name for the client to open and emit its text. The open file handle will be passed to the server for finishing without needing to communicate the path further.
1. Open the file when it is done. Observe both the initial wash/dry steps by `client.exe` and the 'sparkle finish' steps applied by the `server.exe`.

### To clean up

1. Navigate to the solution directory.
1. Remove the COM Proxy, Interfaces, and Out-of-Proc Class Server: `PowerShell.exe -ExecutionPolicy Unrestricted -File .\helpers.ps1 -clean -path x64\release`. As before, change `x64\release` to match your output directory.

## Notes

- The x86 build of the Proxy requires `__stdcall` or the compiler flag `/Gz`. It is already set in this sample project. This is needed because we force `NT100` for MIDL generating the stub, which requires `ObjectStublessClientN` instead of an implicit `-1` for the stubs. To resolve these, we must use `__stdcall` convention.
