---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: DeskBands
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "A simple taskbar band."
---

# DeskBand sample

A simple taskbar band which illustrates the use of the different DeskBand APIs to create a custom desk band.

## Build

Build DeskBandSDKSample.dll with the provided solution.

## Install

Register the sample by running `regsvr32 DeskBandSDKSample.dll` from an elevated command prompt.

## Run

Right-click on the Task Bar, expand "Toolbars" menu option and choose "DeskBand Sample".  You will see the newly created Desk Band show up on the Task Bar.

Note: The sample may not appear until you open the Toolbars menu the second time.

## Uninstall

Unregister the sample by running `regsvr32 /u DeskBandSDKSample.dll` from an elevated command prompt.
