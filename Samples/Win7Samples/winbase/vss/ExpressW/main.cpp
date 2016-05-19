/*++

Copyright (c) 2008  Microsoft Corporation

Title:

    Express Writer sample application

Abstract:

    Sample Express Writer application main code file

--*/


#include "stdafx.h"


//
// This is sample Express Writer GUID
//
// Do not reuse it: your GUID should be unique and consistent
// between writer releases
//
// In order to create your own GUID run GUID creation tool
// that comes with Visual Studio
//
// Please remember that your writer should be exposed with
// the same GUID on every system it is installed on
//
// If you want to version your Writer, please use the version
// parameter during writer creation and/or registration
//
GUID EXPRESS_WRITER_SAMPLE_GUID =
{
    0x12345678,
    0x1234,
    0x1234,
    {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
};


//
// This is the name of the file where Express Writer metadata
// will be saved by one of the sample scenarios
//
// Please do not modify its content by hand
//
const PCWSTR g_wszFileName = L"ExpressWriter.xml";


///////////////////////////////////////////////////////////////////////////////
//
// CreateAndRegister
//
///////////////////////////////////////////////////////////////////////////////
bool CreateAndRegister()
{
    HRESULT hr              = S_OK;
    bool    bResult         = false;

    CComPtr<IVssExpressWriter>                  spExpressWriter;
    CComPtr<IVssCreateExpressWriterMetadata>    spMetadata;

    wprintf(L"INFO: CreateAndRegister called\n");

    //
    // Create Express Writer
    //
    wprintf(L"INFO: Creating Express Writer...\n");
    CHECK_HR(
        CreateVssExpressWriter(&spExpressWriter),
        L"CreateVssExpressWriter failed");

    //
    // Create metadata
    //
    wprintf(L"INFO: Creating metadata...\n");
    CHECK_HR(
        spExpressWriter->CreateMetadata(
            EXPRESS_WRITER_SAMPLE_GUID,         // Express Writer GUID
            L"Sample Express Writer",           // Express Writer friendly name
            VSS_UT_BOOTABLESYSTEMSTATE,         // VSS_USAGE_TYPE
            1,                                  // Major version
            0,                                  // Minor version
            0,                                  // Reserved (must be 0)
            &spMetadata),
        L"CreateVssExpressWriter failed");

    CHECK_CONDITION(
        ConstructWriterDefinition(spMetadata),
        L"ConstructWriterDefinition failed");

    bResult = true;

    //
    // Register writer in the system
    //
    CHECK_HR(spExpressWriter->Register(), L"Register failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// CreateAndSaveToFile
//
///////////////////////////////////////////////////////////////////////////////
bool CreateAndSaveToFile()
{
    HRESULT hr              = S_OK;
    bool    bResult         = false;

    CComPtr<IVssExpressWriter>                  spExpressWriter;
    CComPtr<IVssCreateExpressWriterMetadata>    spMetadata;

    wprintf(L"INFO: CreateAndSaveToFile called\n");

    //
    // Create Express Writer
    //
    wprintf(L"INFO: Creating Express Writer...\n");
    CHECK_HR(
        CreateVssExpressWriter(&spExpressWriter),
        L"CreateVssExpressWriter failed");

    //
    // Create metadata
    //
    wprintf(L"INFO: Creating metadata...\n");
    CHECK_HR(
        spExpressWriter->CreateMetadata(
            EXPRESS_WRITER_SAMPLE_GUID,         // Express Writer GUID
            L"Sample Express Writer",           // Express Writer friendly name
            VSS_UT_BOOTABLESYSTEMSTATE,         // VSS_USAGE_TYPE
            1,                                  // Major version
            0,                                  // Minor version
            0,                                  // Reserved (must be 0)
            &spMetadata),
        L"CreateVssExpressWriter failed");

    CHECK_CONDITION(
        ConstructWriterDefinition(spMetadata),
        L"ConstructWriterDefinition failed");

    bResult = true;

    //
    // Save results
    //
    CHECK_CONDITION(SaveToFile(spMetadata, g_wszFileName), L"SaveToFile failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// LoadFromFileAndRegister
//
///////////////////////////////////////////////////////////////////////////////
bool LoadFromFileAndRegister()
{
    HRESULT hr      = S_OK;
    bool    bResult = false;
    PWSTR   wszData = NULL;

    CComPtr<IVssExpressWriter>  spExpressWriter;

    wprintf(L"INFO: LoadFromFileAndRegister called\n");

    //
    // Create Express Writer
    //
    wprintf(L"INFO: Creating Express Writer...\n");
    CHECK_HR(
        CreateVssExpressWriter(&spExpressWriter),
        L"CreateVssExpressWriter failed");

    //
    // Load metadata
    //
    wprintf(L"INFO: Loading metadata...\n");

    CHECK_CONDITION(
        LoadMetadata(spExpressWriter, g_wszFileName, &wszData),
        L"LoadMetadata failed");

    //
    // Save results
    //
    CHECK_HR(spExpressWriter->Register(), L"Register failed");

    bResult = true;

_exit:
    if (wszData)
    {
        free(wszData);
        wszData = NULL;
    }
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// UnregisterWriter
//
///////////////////////////////////////////////////////////////////////////////
bool UnregisterWriter()
{
    HRESULT hr              = S_OK;
    bool    bResult         = false;

    CComPtr<IVssExpressWriter>                  spExpressWriter;
    CComPtr<IVssCreateExpressWriterMetadata>    spMetadata;

    wprintf(L"INFO: Unregister called\n");

    //
    // Create Express Writer
    //
    wprintf(L"INFO: Creating Express Writer...\n");
    CHECK_HR(
        CreateVssExpressWriter(&spExpressWriter),
        L"CreateVssExpressWriter failed");

    //
    // Create metadata
    //
    wprintf(L"INFO: Unregistering Express Writer...\n");
    CHECK_HR(
        spExpressWriter->Unregister(
            EXPRESS_WRITER_SAMPLE_GUID),            // Express Writer GUID
        L"Unregister failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// RunSamples
//
///////////////////////////////////////////////////////////////////////////////
bool RunSamples()
{
    HRESULT hr      = S_OK;
    bool    bResult = false;

    wprintf(L"INFO: RunSamples called\n");

    //
    // Create new Express Writer and register it in the system
    //
    // This scenario allows you to perform full Express Writer
    // creation and registration on the target machine
    //
    // This code can be used by your application to configure its
    // writer during installation or on the first execution
    // assuming that your storage varies between client machines
    //

    bResult = CreateAndRegister();          // Execute on target machine to
                                            // create and register Writer

    CHECK_CONDITION(bResult, L"CreateAndRegister failed");

    bResult = UnregisterWriter();           // Cleanup for the next sample

    CHECK_CONDITION(bResult, L"UnregisterWriter failed");

    //
    // Create new Express Writer and save it to a file
    // Then load it from file and register in the system
    //
    // This scenario allows you to perform full Express Writer
    // creation offline (during development) and later perform
    // registration on the target machine by reading the file
    //
    // This code can be used by your application to configure its
    // writer during installation or on the first execution
    // assuming that your storage location is the same on each
    // machine where your application is being deployed
    //

    bResult = CreateAndSaveToFile();        // Execute during development to
                                            // create Express Writer metadata file

    CHECK_CONDITION(bResult, L"CreateAndSaveToFile failed");

    bResult = LoadFromFileAndRegister();    // Execute on the target machine to
                                            // load Express Writer metadata from
                                            // file and register Express Writer in
                                            // the system

    CHECK_CONDITION(bResult, L"LoadFromFileAndRegister failed");

    bResult = UnregisterWriter();           // Cleanup after last sample

    CHECK_CONDITION(bResult, L"UnregisterWriter failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// wmain
//
///////////////////////////////////////////////////////////////////////////////
int __cdecl wmain(
    __in                int     argc,
    __in_ecount(argc)   WCHAR   *argv[]
)
{
    bool    bCoInitializeCalled = false;
    int     iResult             = -1;
    int     iTestIndex          = -1;
    HRESULT hr                  = S_OK;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    hr = ::CoInitialize(NULL);
    CHECK_HR(hr, L"CoInitialize failed");
    bCoInitializeCalled = true;

    //
    // Run sample Express Writer code
    //
    CHECK_CONDITION(RunSamples(), L"RunSamples failed");

    iResult = 0;

_exit:
    if (bCoInitializeCalled)
    {
        ::CoUninitialize();
    }

    wprintf(L"INFO: Application returning %d\n", iResult);
    return iResult;
}


