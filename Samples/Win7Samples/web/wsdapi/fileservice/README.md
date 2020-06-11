---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: FileService sample
urlFragment: fileservice-sample
description: Demonstrates the basic use of the Web Services for Devices API.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Summary

The FileService sample demonstrates the basic use of the Web Services for Devices API. FileService implements the functionality described by FileService.wsdl. The sample includes both a client and service.
 
## Files

### Supplied files

| File | Description |
|------|-------------|
| *ReadMe.txt* | This readme file|
| *FileServiceContract\CodeGen_All.config*| Config file for this sample |
| *FileServiceContract\CodeGen_Client.config*| Alternate config file (see note below) |
| *FileServiceContract\CodeGen_Host.config*| Alternate config file (see note below) |
| *FileServiceContract\FileService.wsdl*| FileService Contract |
| *FileServiceService\Service.cpp*| Service Implementation |
| *FileServiceService\Service.h*| Header for Service Implementation |
| *FileServiceClient\Client.cpp*| Client Implementation |
| *FileServiceClient\Client.h*| Header for Client Implementation |

### Generated files

These files are generated automatically by *WsdCodeGen.exe*, but are included in the sample for reference. You may rebuild these files by running *WsdCodeGen*.

| File | Description |
|------|-------------|
|*FileServiceContract\FileService.idl*| Interface file |
|*FileServiceContract\[FileService.h]*|Header file built from *FileService.idl* |
|*FileServiceContract\FileServiceProxy.cpp* | Proxy class implementations |
|*FileServiceContract\FileServiceProxy.h*| Proxy class definitions |
|*FileServiceContract\FileServiceStub.cpp*| Stub function implementations |
|*FileServiceContract\FileServiceTypes.cpp*| Type definitions |
|*FileServiceContract\FileServiceTypes.h*|Type declarations and structure definitions |

## Platforms supported

* Windows Vista
* Windows Server 2008
* Windows 7
* Windows Server 2008 R2

## Running the server and client applications

To build, type **nmake** on the command line in this directory. The client and service applications can run on the same Microsoft Windows Vista 
computer or a different one.

To run the service type:

     FileServiceService.exe <path>  (where <path> is the folder of files to be served)

To run the client type:

     FileServiceClient.exe <path> <device ID> (where <path> is the folder to receive files and <device ID> is the ID printed by FileServiceService.exe)


## Layout of classes and functions

### Service classes and functions

| Classes/functions | Description  |
|------|-------------|
| `CFileServiceService` | Implements the `IFileService` interface, which matches the `FileService` port type.  This class acts like a COM object (for example, has `AddRef`, `Release`, and `QueryInterface` methods) and also exposes the `GetFile` and `GetFileList` methods, which can be accessed across the wire. The `CFileServiceService` has `oneCFileChangeNotificationThread` object, which automatically launches a thread to monitor for file system changes. This thread will call back into the `CFileServiceService` object to issue events to clients. When `GetFile` is called on the `CFileServiceService` object, it creates a `CSendAttachmentThread` object, which automatically launches a thread to write to an attachment. The thread will copy the contents of a file into the attachment and, when it's finished, will close the attachment and destroy itself.|
| `CFileChangeNotificationThread` | Starts and makes a blocking request for file change notifications. When a notification is received, this class packages up the results and sends an event to all subscribed clients. |
| `CSendAttachmentThread` | Opens a file and writes all data in the file into the supplied attachment object. When the file has been completely consumed, the object will close the attachment and then delete itself. The proper way to use this object is to instantiate one, call Start, and then discard the pointer to the object. It will be responsible for its own cleanup.
| `StripCbPath` | Removes path information from a filename. |
| `CloneString` | Copies a string into a new buffer allocated with `WSDAllocateLinkedMemory`.|
| `CreateStringList` | Copy a string into a `PWCHAR_LIST` linked-list object. |
| `wmain` | Main function for processing command-line arguments and building a device host which advertises a `FileService` service.

### Client classes and functions

| Classes/functions | Description  |
|------|-------------|
| `CFileServiceEventNotify` | Notification class that receives callbacks when a host sends a FileChange event to this client. This object is passed in as a parameter when the client subscribes to the FileChange event.|
| `CGetFileAsyncCallback` | Notification class that is used when the client calls the `GetFile` method on the service. This async callback object is passed in as a parameter to the `BeginGetFile` proxy method, and when the operation completes, the `CGetFileAsyncCallback::AsyncOperationComplete` method is called. This object is then responsible for retrieving the results of the `GetFile` call and saving them to a local file.|
| `Directory` | Wrapper function that invokes the `GetFileList` method on the service. |
| `GetFile` | Wrapper function that sets up a `CGetFileAsyncCallback` object and then invokes the `GetFile` method on the service. |
| `wmain` | Main function for processing command-line arguments, creating a proxy for sending service messages to a `FileService` service, and finally presenting an interactive prompt for sending these messages. |

## Alternate Codegen Config Files

Three config files are included with this sample.
* *CodeGen_All.config*: Default, includes both host and client options
* *CodeGen_Client.config*: Client-only options
* *CodeGen_Host.config*: Host-only options

Only the first of these (*CodeGen_All.config*) is used when generating code for the FileService sample. *CodeGen_Client.config* and *CodeGen_Host.config* are provided to illustrate how to generate code for only the client and only the host.

If you would like to use these alternate config files, follow these steps:
1. Run `WsdCodeGen.exe /generatecode [alternate config file] /gbc`.
1. Rebuild the `FileServiceContract` project, and then rebuild either the `FileServiceClient` or `FileServiceService` project.