---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DDXGI
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Combines Direct2D and Direct3D."
---

# Direct2D/DXGI Interoperation Sample

Shows how to use Direct3D and Direct2D together.

This sample is written in C++.

## Files

* d3dmath.h: Defines Direct3D helper methods.
* DeclareDPIAware.manifest: Specifies that the sample application is DPI-aware.
* dxgisample.fx: Defines pixel and vertex shader information.
* DxgiSample.cpp: Contains the application entry point and the implementation of the DXGISampleApp class.
* DxgiSample.h: The header file for the DXGISampleApp class.
* DxgiSample.rc: Defines the resources used by the DXGISampleApp class.
* DxgiSample.sln: The sample's solution file.
* DxgiSample.vcproj: The sample project file.
* resource.h: Defines additional resources used by the DXGISampleApp class. 
* sampleImage.jpg: An image displayed by the sample.
 
## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 
* March 2009 DirectX SDK

## Configuring without Visual Studio

1. Open a Windows SDK command shell. 

| Bitness        | Path                                                                                   |
|----------------|----------------------------------------------------------------------------------------|
| 32-bit Windows | `"C:\Program Files\Microsoft DirectX SDK <version>\Utilities\bin\dx_setenv.cmd"`       |
| 64-bit Windows | `"C:\Program Files (x86)\Microsoft DirectX SDK <version>\Utilities\bin\dx_setenv.cmd"` |

Type the command line above, adjusting the path as necessary depending on where you installed the DirectX SDK. For example,

```
"C:\Program Files (x86)\Microsoft DirectX SDK (March 2009)\Utilities\bin\dx_setenv.cmd"
```

You must perform this step each type you compile the sample.

2. To compile the sample, type `vcbuild /u DXGISample.sln`.

## Configuring for Visual Studio (preferred method)

After installing the DirectX SDK, you must configure Visual Studio
to use the executables and include files provided by the DirectX SDK. You must configure Visual Studio for
each platform (Win32 or x64) you want to build against. 

For building against the Win32 (x86) platform:

1. Launch Visual Studio 2008.

2. Open the Tools menu and select Options. The Options dialog box appears.

3. In the left pane of the Options dialog box, expand the Projects and Solutions node.

4. Under Project and Solutions, select VC++ Directories.

5. In the right pane, set the "Platform" drop-down list box to Win32 and the 
   "Show directories for" drop-down list box to Executable files.

6. At the bottom of the list of executable file directories, create a new entry for the DirectX SDK:

| Bitness        | Path                                                                       |
|----------------|----------------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Utilities\bin\x86`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Utilities\bin\x86` |

(If there was already such an entry, move it to the bottom of the list.)
  
7. Set the "Show directories for" drop-down list box to "Include" files.

8. At the bottom of the list of directories, create a new entry for the DirectX SDK: 

| Bitness        | Path                                                             |
|----------------|------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Include`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Include` |

If there was already such an entry, move it to the bottom of the list.

9. Set the "Show directories for" drop-down list box to "Library" files.

10. At the bottom of the list of directories, create a new entry for the DirectX SDK:

| Bitness        | Path                                                             |
|----------------|------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Lib\x86`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Lib\x86` |

If there was already such an entry, move it to the bottom of the list.

11. Click OK.

For building against the x64 platform:

1. Launch Visual Studio 2008.

2. Open the Tools menu and select Options. The Options dialog box appears.

3. In the left pane of the Options dialog box, expand the Projects and Solutions node.

4. Under Project and Solutions, select VC++ Directories.

5. In the right pane, set the "Platform" drop-down list box to x64 and the 
 "Show directories for" drop-down list box to Executable files. 

6. At the bottom of the list of executable file directories, create a new entry for the DirectX SDK:  

| Bitness        | Path                                                                       |
|----------------|----------------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Utilities\bin\x86`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Utilities\bin\x64` |

If there was already such an entry, move it to the bottom of the list.

7. Set the "Show directories for" drop-down list box to "Include" files.

8. At the bottom of the list of directories, create a new entry for the DirectX SDK: 

| Bitness        | Path                                                             |
|----------------|------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Include`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Include` |

If there was already such an entry, move it to the bottom of the list.

9.  Set the "Show directories for" drop-down list box to "Library" files.

10. At the bottom of the list of directories, create a new entry for the DirectX SDK:

| Bitness        | Path                                                             |
|----------------|------------------------------------------------------------------|
| 32-bit Windows | `C:\Program Files\Microsoft DirectX SDK <version>\Lib\x64`       |
| 64-bit Windows | `C:\Program Files (x86)\Microsoft DirectX SDK <version>\Lib\x64` |

If there was already such an entry, move it to the bottom of the list.

11. Click OK.

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and run the dx_setenv.cmd (see the _Configuring without Visual Studio_ section for more information.)
2. Navigate to the sample directory.
3. Type `vcbuild /u DxgiSample.sln`.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
