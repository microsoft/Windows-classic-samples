//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//      Class declarations of WCS color samples
//
//----------------------------------------------------------------------------

//
// Enumeration of basic bitmap types
// used by this sample application
//
typedef enum SampleBitmapType
{
    BasicRGBTripletBitmap = 0,
    // Basic bitmap format using BM_RGBTRIPLETS
    BasicCMYKQuadData
    // Basic bitmap format using BM_CMYKQUADS
};

//
// Enumeration of color profiles used by
// this sample application
//
typedef enum ResourceProfileType
{
    SampleRGBPrinter,
    // RGBPrinter.cdmp included with this sample application
    SampleCMYKPrinter,
    // CMYKPrinter.cdmp included with this sample application
    SampleCRTProfileWithPlugin,
    // PluginProfile.cdmp included with this sample, which 
    // references a GUID that will invoke the plugin sample
    // from the seperate WCS plugin sample package if present
    SampleGmmpWithPlugin
};

//
// Base class for classes doing color demonstrations to inherit
// from in this sample.  This exposes a common method to run
// the demos and utility methods to load and save images.
// LIMITATION: The input and output format and header must 
// be equivalent in this sample!
//
class IWcsDemo{
public:
    virtual HRESULT RunDemos() = 0;

protected:
    static HRESULT 
    LoadsRGBImage(
        __deref_out_bcount(*pcbBitmapData) BYTE **ppaBitmapData, 
        __out DWORD *pcbBitmapData,
        __out DWORD *pdwWidth,
        __out DWORD *pdwHeight,
        __out DWORD *pdwHorizontalResolution,
        __out DWORD *pdwVerticalResolution
        );

    static HRESULT 
    SaveImage(
        __in PCWSTR pwszImageFilename,
        __in SampleBitmapType BitmapType,
        __in_bcount(cbBitmapData) BYTE *paBitmapData, 
        __in DWORD cbBitmapData,
        __in DWORD dwWidth,
        __in DWORD dwHeight,
        __in DWORD dwHorizontalResolution,
        __in DWORD dwVerticalResolution
        );

    static HRESULT GetProfileFromResource(
        __in ResourceProfileType ProfileID,
        __out PROFILE *pProfile
        );

    static PPROFILE GetD50CAMP();

    static PPROFILE GetD65CAMP();

    static HRESULT SavePixelData(
        __in PCWSTR pwszFilename,
        __in_bcount(cbBitmapData) BYTE *paData, 
        __in DWORD cbData
        );
    
};

//
// This class exposes methods which demonstrate
// creation of WCS transforms for different profile
// types and rendering intents
//
class CTransformCreationDemo: public IWcsDemo
{
public:
    HRESULT RunDemos();

private:
    HRESULT DemonstrateSystemGmmp();

    HRESULT DemonstrateCustomPluginProfiles();

    HRESULT DemonstrateSystemDefaultIntent();

    HRESULT DemonstrateIccWcsMixedMode();

    HRESULT DemonstrateIccOnlyWcsTransform();

    static HRESULT TranslateAndOutputRGBImage(
        __in HTRANSFORM hTransform, 
        __in PCWSTR pwszOutputFilename
        );
};

//
// This class exposes methods which demonstrate
// color and bitmap translation using different
// formats and APIs
//
class CTranslationDemo: public IWcsDemo
{
public:
    HRESULT RunDemos();

private:
    HRESULT DemonstrateS2Dot13BitmapTranslation();

    HRESULT DemonstrateFloatColorTranslation();

    static HRESULT BuildBasicTransform(
        __in ResourceProfileType InputProfileType,
        __in PPROFILE InputViewingConditionsProfile,
        __in ResourceProfileType OutputProfileType,
        __in PPROFILE OutputViewingConditionsProfile,
        __deref_out HTRANSFORM *phTransform
        );
};