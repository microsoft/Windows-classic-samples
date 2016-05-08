Windows Color System Plugin Samples

SUMMARY
This package includes a sample Gamut Map Model Plugin. 

The sample Gamut Map Model Plugin performs a pass-through operation, mirroring the output from the input.  The sample demonstrate how to utilize plugin communication using standard COM APIs and the interface pointers provided to the plugins at runtime when used in combination with the sample Device Model Plugin.

BUILDING THE SAMPLES
This sample is to be built using Visual Studio 2005 and the provided Solution file

TEMPLATING FROM THE SAMPLES
When using the template as a sample for new plugins it is important to update all necessary names, GUIDs, and data. These include:
1. 	Creating new GUIDs for class, library, and interfaces
2. 	Changing filenames of the generated .dll files
3. 	Changing filenames of the XML Profile files
4. 	Adding data to the .GMMP Profile for use initializing baseline Gamut Map Model
5.	Addressing comments in the sample marked as "TODO"

ADDITIONAL NOTES
The sample Gamut Map Model Plugin mirror the input data to the output.  However, a real plugin should not output data that is outside of the Gamut Boundary Description of the destination gamut.

INSTALLING THE SAMPLES
To install the sample plugin, the plugin .DLL files and XML Profile files must be copied onto a Vista machine, and the plugin .DLL file must registered using RegSvr32.exe

© 2005 Microsoft Corporation. All Rights Reserved
