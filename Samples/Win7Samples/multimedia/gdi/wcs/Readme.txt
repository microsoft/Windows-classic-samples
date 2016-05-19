Windows Color System Color Translation Sample

SUMMARY
This package includes source code to provide examples of using color management APIs with Windows Color System profiles.  This code creates various types of color transforms and color translations for demonstration purposes.

The sample performns these tasks on an input image file which is in the "Resources" subdirectory.  The sample is not capable of generic bitmap data processing and is not intended for templating for such purpose.  The sample works exclusively with the format of the provided input image file.

The sample uses WCS profiles that invoke the sample Device Model Plugin and Gamut Map Model Plugin from the seperate WCS sample plugins package.  If these profiles are not present or not registered, the fallback models will be invoked instead when the sample uses these profiles.

BUILDING THE SAMPLES
This sample is to be built using Visual Studio 2005.  Please copy Windows Vista versions of mscms.lib and icm.h into the "Lib" and "Include" subdirectories, respectively, before building this project.

KNOWN ISSUES
A known Windows Vista bug causes the WcsGetDefaultRenderingIntent API to fail when a user has not yet set a default intent (instead of returning the system default in that case).  As a temporary workaround, please use regedit.exe to import the file DefaultIntentWorkaround.reg included in this package before running the sample executable.  This sets the user's default rendering intent to be Perceptual.

INDEX OF CONTENTS:
Following is an index of the sample code, listing the purpose and the output of each method:

CTransformCreationDemo::DemonstrateSystemGmmp
This method demonstrates using system GMMP profiles in order to invoke a specific gamut mapping algorithm during color transform creation.  This method outputs the image file "DemonstrateSystemGmmp Output.bmp", which is the result of translating the input image file from the system profile wsRGB.cdmp to RgbPrinter.cdmp using the Sigmoid Gaussian Cusp Knee gamut mapping algorithm.

CTransformCreationDemo::DemonstrateCustomPluginProfiles
This method demonstrates using custom CDMP and GMMP profiles in order to invoke a plugin Device Model and Gamut Map Model.  This method outputs the image file "DemonstrateCustomPluginProfiles Output.bmp", which is the result of translating the input image file from the Device Model plugin to RGBPrinter.cdmp using the plugin Gamut Map Model plugin.

CTransformCreationDemo::DemonstrateSystemDefaultIntent
This method demonstrates using the system default rendering intent.  This method outputs the image file "DemonstrateSystemDefaultIntent Output.bmp", which is the result of translating the input image file from the system profile wsRGB.cdmp to RgbPrinter.cdmp.  This translation uses the gamut mapper from the GMMP profile that is set as the current user's default selection for the rendering intent that is set as the default.

CTransformCreationDemo::DemonstrateIccWcsMixedMode
This method demonstrates using a combination of ICC and WCS transforms to create a WCS color transform.  This method outputs the image file "DemonstrateIccWcsMixedMode Output.bmp", which is the result of translating the input image file from the system profile "sRGB Color Space Profile.icm" to RgbPrinter.cdmp using the gamut mapper from the GMMP profile that is set as the current user's default selection for the Absolute Colorimetric rendering intent.

CTransformCreationDemo::DemonstrateIccOnlyWcsTransform
This method demonstrates using a sequence of only ICC profiles to create a color transform that uses the WCS engine.  This method outputs the image file "DemonstrateIccOnlyWcsTransform Output.bmp", which is the result of translating the input image file from the system profile "sRGB Color Space Profile.icm" to itself using the gamut mapper from the GMMP profile that is set as the current user's default selection for the Absolute Colorimetric rendering intent.

CTranslationDemo::DemonstrateS2Dot13BitmapTranslation
This method demonstrates translation of bitmap bits in S2Dot13 fixed point format. This method creates a basic transform from PluginProfile.cdmp to RgbPrinter.cdmp and translates the input image using this transform.  The result of the translation is in S2Dot13 format, and the pixel data itself is written to the file, "DemonstrateS2Dot13BitmapTranslation Output.dat".

CTranslationDemo::DemonstrateFloatColorTranslation
This method demonstrates translation of colors to floating point CMYK format.  This method creates a basic transform from PluginProfile.cdmp to CMYKPrinter.cdmp and translates the input image using this transform.  The result of the translation is in floating point CMYK format, and the pixel data itself is written to the file, "DemonstrateFloatColorTranslation Output.dat".

© 2005 Microsoft Corporation. All Rights Reserved
