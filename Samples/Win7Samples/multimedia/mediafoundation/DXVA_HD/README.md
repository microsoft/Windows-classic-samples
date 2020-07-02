---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: DXVA-HD sample
urlFragment: dxva-hd
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This sample demonstrates DirectX Video Acceleration High Definition. DXVA-HD is a low-level video processing API.
---

# DXVA-HD sample

This sample demonstrates DirectX Video Acceleration High Definition (DXVA-HD). DXVA-HD is a low-level video processing API.

This sample is essentially a port of the DXVA2_VideoProc sample to use the DXVA-HD interfaces. It has the same basic functionality, and most of the keyboard commands are the same.

DXVA-HD has additional capabilities that make it more powerful for video processing than the original DXVA-VP APIs introduced in Windows Vista:

- Any stream can be either RGB or YUV.
- Any stream can be either progressive or interlaced.
- The background color can be RGB.
- Luma keying.
- The device can switch between blend, bob, and adaptive deinterlacing.
- Mandatory output formats: X8R8G8B8 and A8R8G8B8
- Mandatory input formats: X8R8G8B8, A8R8G8B8, YUY2 and AYUV

## Sample language implementations

C++

## Files

Application.cpp
Application.h
D3DHelper.cpp
D3DHelper.h
DXVA2_HD.rc
DXVAHD_Sample.h
dxvahd_utils.cpp
dxvahd_utils.h
DXVA_HD.sln
DXVA_HD.vcproj
readme.txt
resource.h
settings.h
stdafx.h
substream.bmp
utils.h
video.cpp
video.h
winmain.cpp

## To build the sample using the command prompt

1. Open the Command Prompt window and navigate to the DXVA_HD directory.
2. Type msbuild DXVA_HD.sln.

## To build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the DXVA_HD directory.
2. Double-click the icon for the DXVA_HD.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## To run the sample

1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type DXVA_HD.exe at the command line, or double-click the icon for DXVA_HD.exe to launch it from Windows Explorer.

Command line options:

```cmd
-hh : Hardware Direct3D device and hardware DXVA-HD device.
-hs : Hardware Direct3D device and software DXVA-HD device.
-ss : Software Direct3D device and software DXVA-HD device.
```

## Modes

The user can toggle between different modes by pressing the F1 through F9 keys.

The arrow keys control different settings in each mode:

F1 : Alpha values.

- UP/DOWN: Main video planar alpha
- LEFT/RIGHT: Substream pixel alpha

F2 : Resize the main video source rectangle.

F3 : Move the main video source rectangle.

F4 : Resize the main video destination rectangle.

F5 : Move the main video destination rectangle.

F6 : Change the background color or extended color information.

- UP/DOWN: Change YCbCr standard and RGB color range.
- LEFT/RIGHT: Cycle through background colors.

F7 : Ajust brightness and contrast.

- UP/DOWN: Brightness
- LEFT/RIGHT: Contrast

F8 : Adjust hue and saturation.

- UP/DOWN: Hue
- LEFT/RIGHT: Saturation

F9: Resize the target rectangle.

HOME : Resets all mode settings to their default values.

ALT + ENTER: Switch between windowed and full-screen mode.
