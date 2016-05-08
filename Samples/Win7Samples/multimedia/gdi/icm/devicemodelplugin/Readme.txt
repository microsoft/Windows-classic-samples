Windows Color System Plugin Samples

SUMMARY
This package includes a sample Device Model Plugin.

The sample Device Model Plugin performs an sRGB translation.  The device model sample implements a private interface in addition to IDeviceModel, and builds the proxy code for this interface in its binary.

BUILDING THE SAMPLES
This sample is to be built using Visual Studio 2005 and the provided Solution file

TEMPLATING FROM THE SAMPLES
When using the template as a sample for new plugins it is important to update all necessary names, GUIDs, and data. These include:
1. 	Creating new GUIDs for class, library, and interfaces
2. 	Changing filenames of the generated .dll files
3. 	Changing filenames of the XML Profile files
4. 	Adding data to the Device Model Profile for use initializing baseline Device Models
5.	Addressing comments in the sample marked as "TODO"

ADDITIONAL NOTES
The XML Device Model Profile's sample data is not intended to match the behavior of the plugin.

INSTALLING THE SAMPLES
To install the sample plugins, the plugin .DLL file and XML Profile file must be copied onto a Vista machine, and the plugin .DLL file must registered using RegSvr32.exe

© 2005 Microsoft Corporation. All Rights Reserved
