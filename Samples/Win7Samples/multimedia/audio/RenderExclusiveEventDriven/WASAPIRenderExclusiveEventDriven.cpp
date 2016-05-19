// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//
// WASAPIRenderExclusiveEventDriven.cpp : Scaffolding associated with the WASAPI Render Exclusive Event Driven sample application.
//

#include "stdafx.h"

#include <functiondiscoverykeys.h>
#include "WASAPIRenderer.h"

#include "CmdLine.h"
#include "ToneGen.h"


int TargetFrequency = 440;
int TargetLatency = 10;
int TargetDurationInSec = 10;
bool ShowHelp;
bool UseConsoleDevice;
bool UseCommunicationsDevice;
bool UseMultimediaDevice;
bool DisableMMCSS;

wchar_t *OutputEndpoint;

CommandLineSwitch CmdLineArgs[] = 
{
    { L"?", L"Print this help", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&ShowHelp)},
    { L"h", L"Print this help", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&ShowHelp)},
    { L"f", L"Sine wave frequency (Hz)", CommandLineSwitch::SwitchTypeInteger, reinterpret_cast<void **>(&TargetFrequency), false},
    { L"l", L"Audio Render Latency (ms)", CommandLineSwitch::SwitchTypeInteger, reinterpret_cast<void **>(&TargetLatency), false},
    { L"d", L"Sine Wave Duration (s)", CommandLineSwitch::SwitchTypeInteger, reinterpret_cast<void **>(&TargetDurationInSec), false},
    { L"m", L"Disable the use of MMCSS", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&DisableMMCSS)},
    { L"console", L"Use the default console device", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&UseConsoleDevice)},
    { L"communications", L"Use the default communications device", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&UseCommunicationsDevice)},
    { L"multimedia", L"Use the default multimedia device", CommandLineSwitch::SwitchTypeNone, reinterpret_cast<void **>(&UseMultimediaDevice)},
    { L"endpoint", L"Use the specified endpoint ID", CommandLineSwitch::SwitchTypeString, reinterpret_cast<void **>(&OutputEndpoint), true},
};

size_t CmdLineArgLength = ARRAYSIZE(CmdLineArgs);

//
//  Print help for the sample
//
void Help(LPCWSTR ProgramName)
{
    printf("Usage: %S [-/][Switch][:][Value]\n\n", ProgramName);
    printf("Where Switch is one of the following: \n");
    for (size_t i = 0 ; i < CmdLineArgLength ; i += 1)
    {
        printf("    -%S: %S\n", CmdLineArgs[i].SwitchName, CmdLineArgs[i].SwitchHelp);
    }
}

//
//  Retrieves the device friendly name for a particular device in a device collection.
//
LPWSTR GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex)
{
    IMMDevice *device;
    LPWSTR deviceId;
    HRESULT hr;

    hr = DeviceCollection->Item(DeviceIndex, &device);
    if (FAILED(hr))
    {
        printf("Unable to get device %d: %x\n", DeviceIndex, hr);
        return NULL;
    }
    hr = device->GetId(&deviceId);
    if (FAILED(hr))
    {
        printf("Unable to get device %d id: %x\n", DeviceIndex, hr);
        return NULL;
    }

    IPropertyStore *propertyStore;
    hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
    SafeRelease(&device);
    if (FAILED(hr))
    {
        printf("Unable to open device %d property store: %x\n", DeviceIndex, hr);
        return NULL;
    }

    PROPVARIANT friendlyName;
    PropVariantInit(&friendlyName);
    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
    SafeRelease(&propertyStore);

    if (FAILED(hr))
    {
        printf("Unable to retrieve friendly name for device %d : %x\n", DeviceIndex, hr);
        return NULL;
    }

    wchar_t deviceName[128];
    hr = StringCbPrintf(deviceName, sizeof(deviceName), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId);
    if (FAILED(hr))
    {
        printf("Unable to format friendly name for device %d : %x\n", DeviceIndex, hr);
        return NULL;
    }

    PropVariantClear(&friendlyName);
    CoTaskMemFree(deviceId);

    wchar_t *returnValue = _wcsdup(deviceName);
    if (returnValue == NULL)
    {
        printf("Unable to allocate buffer for return\n");
        return NULL;
    }
    return returnValue;
}
//
//  Based on the input switches, pick the specified device to use.
//
bool PickDevice(IMMDevice **DeviceToUse, bool *IsDefaultDevice, ERole *DefaultDeviceRole)
{
    HRESULT hr;
    bool retValue = true;
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    IMMDeviceCollection *deviceCollection = NULL;

    *IsDefaultDevice = false;   // Assume we're not using the default device.

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        retValue = false;
        goto Exit;
    }

    IMMDevice *device = NULL;

    //
    //  First off, if none of the console switches was specified, use the console device.
    //
    if (!UseConsoleDevice && !UseCommunicationsDevice && !UseMultimediaDevice && OutputEndpoint == NULL)
    {
        //
        //  The user didn't specify an output device, prompt the user for a device and use that.
        //
        hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
        if (FAILED(hr))
        {
            printf("Unable to retrieve device collection: %x\n", hr);
            retValue = false;
            goto Exit;
        }

        printf("Select an output device:\n");
        printf("    0:  Default Console Device\n");
        printf("    1:  Default Communications Device\n");
        printf("    2:  Default Multimedia Device\n");
        UINT deviceCount;
        hr = deviceCollection->GetCount(&deviceCount);
        if (FAILED(hr))
        {
            printf("Unable to get device collection length: %x\n", hr);
            retValue = false;
            goto Exit;
        }
        for (UINT i = 0 ; i < deviceCount ; i += 1)
        {
            LPWSTR deviceName;

            deviceName = GetDeviceName(deviceCollection, i);
            if (deviceName == NULL)
            {
                retValue = false;
                goto Exit;
            }
            printf("    %d:  %S\n", i + 3, deviceName);
            free(deviceName);
        }
        wchar_t choice[10];
        _getws_s(choice);   // Note: Using the safe CRT version of _getws.

        long deviceIndex;
        wchar_t *endPointer;

        deviceIndex = wcstoul(choice, &endPointer, 0);
        if (deviceIndex == 0 && endPointer == choice)
        {
            printf("unrecognized device index: %S\n", choice);
            retValue = false;
            goto Exit;
        }
        switch (deviceIndex)
        {
        case 0:
            UseConsoleDevice = 1;
            break;
        case 1:
            UseCommunicationsDevice = 1;
            break;
        case 2:
            UseMultimediaDevice = 1;
            break;
        default:
            hr = deviceCollection->Item(deviceIndex - 3, &device);
            if (FAILED(hr))
            {
                printf("Unable to retrieve device %d: %x\n", deviceIndex - 3, hr);
                retValue = false;
                goto Exit;
            }
            break;
        }
    } 
    else if (OutputEndpoint != NULL)
    {
        hr = deviceEnumerator->GetDevice(OutputEndpoint, &device);
        if (FAILED(hr))
        {
            printf("Unable to get endpoint for endpoint %S: %x\n", OutputEndpoint, hr);
            retValue = false;
            goto Exit;
        }
    }

    if (device == NULL)
    {
        ERole deviceRole = eConsole;    // Assume we're using the console role.
        if (UseConsoleDevice)
        {
            deviceRole = eConsole;
        }
        else if (UseCommunicationsDevice)
        {
            deviceRole = eCommunications;
        }
        else if (UseMultimediaDevice)
        {
            deviceRole = eMultimedia;
        }
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, deviceRole, &device);
        if (FAILED(hr))
        {
            printf("Unable to get default device for role %d: %x\n", deviceRole, hr);
            retValue = false;
            goto Exit;
        }
        *IsDefaultDevice = true;
        *DefaultDeviceRole = deviceRole;
    }

    *DeviceToUse = device;
    retValue = true;
Exit:
    SafeRelease(&deviceCollection);
    SafeRelease(&deviceEnumerator);

    return retValue;
}

int wmain(int argc, wchar_t* argv[])
{
    int result = 0;
    IMMDevice *device = NULL;
    bool isDefaultDevice;
    ERole role;

    printf("WASAPI Render Exclusive Event Driven Sample\n");
    printf("Copyright (c) Microsoft.  All Rights Reserved\n");
    printf("\n");

    if (!ParseCommandLine(argc, argv, CmdLineArgs, CmdLineArgLength))
    {
        result = -1;
        goto Exit;
    }
    //
    //  Now that we've parsed our command line, do some semantic checks.
    //

    //
    //  First off, show the help for the app if the user asked for it.
    //
    if (ShowHelp)
    {
        Help(argv[0]);
        goto Exit;
    }

    //
    //  The user can only specify one of -console, -communications or -multimedia or a specific endpoint.
    //
    if (((UseConsoleDevice != 0) + (UseCommunicationsDevice != 0) + (UseMultimediaDevice != 0) + (OutputEndpoint != NULL)) > 1)
    {
        printf("Can only specify one of -Console, -Communications or -Multimedia\n");
        result = -1;
        goto Exit;
    }


    //
    //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
    //
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM: %x\n", hr);
        result = hr;
        goto Exit;
    }

    //
    //  Now that we've parsed our command line, pick the device to render.
    //
    if (!PickDevice(&device, &isDefaultDevice, &role))
    {
        result = -1;
        goto Exit;
    }

    printf("Render a %d hz Sine wave for %d seconds\n", TargetFrequency, TargetDurationInSec);

    //
    //  Instantiate a renderer and play a sound for TargetDuration seconds
    //
    //  Configure the renderer to enable stream switching on the specified role if the user specified one of the default devices.
    //
    {
        CWASAPIRenderer *renderer = new (std::nothrow) CWASAPIRenderer(device);
        if (renderer == NULL)
        {
            printf("Unable to allocate renderer\n");
            return -1;
        }

        if (renderer->Initialize(TargetLatency))
        {
            //
            //  We've initialized the renderer.  Once we've done that, we know some information about the
            //  mix format and we can allocate the buffer that we're going to render.
            //
            //
            //  The buffer is going to contain "TargetDuration" seconds worth of PCM data.  That means 
            //  we're going to have TargetDuration*samples/second frames multiplied by the frame size.
            //
            UINT32 renderBufferSizeInBytes = (renderer->BufferSizePerPeriod()  * renderer->FrameSize());
            size_t renderDataLength = (renderer->SamplesPerSecond() * TargetDurationInSec * renderer->FrameSize()) + (renderBufferSizeInBytes-1);
            size_t renderBufferCount = renderDataLength / (renderBufferSizeInBytes);
            //
            //  Render buffer queue. Because we need to insert each buffer at the end of the linked list instead of at the head, 
            //  we keep a pointer to a pointer to the variable which holds the tail of the current list in currentBufferTail.
            //
            RenderBuffer *renderQueue = NULL;
            RenderBuffer **currentBufferTail = &renderQueue;

            double theta = 0;

            for (size_t i = 0 ; i < renderBufferCount ; i += 1)
            {
                RenderBuffer *renderBuffer = new (std::nothrow) RenderBuffer();
                if (renderBuffer == NULL)
                {
                    printf("Unable to allocate render buffer\n");
                    return -1;
                }
                renderBuffer->_BufferLength = renderBufferSizeInBytes;
                renderBuffer->_Buffer = new (std::nothrow) BYTE[renderBufferSizeInBytes];
                if (renderBuffer->_Buffer == NULL)
                {
                    printf("Unable to allocate render buffer\n");
                    return -1;
                }
                //
                //  Generate tone data in the buffer.
                //
                switch (renderer->SampleType())
                {
                case CWASAPIRenderer::SampleTypeFloat:
                    GenerateSineSamples<float>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
                                                renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
                    break;
                case CWASAPIRenderer::SampleType16BitPCM:
                    GenerateSineSamples<short>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
                                                renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
                    break;
                }
                //
                //  Link the newly allocated and filled buffer into the queue.  
                //
                *currentBufferTail = renderBuffer;
                currentBufferTail = &renderBuffer->_Next;
            }

            //
            //  The renderer takes ownership of the render queue - it will free the items in the queue when it renders them.
            //
            if (renderer->Start(renderQueue))
            {
                do
                {
                    printf(".");
                    Sleep(1000);
                } while (--TargetDurationInSec);
                printf("\n");

                renderer->Stop();
                renderer->Shutdown();
                SafeRelease(&renderer);
            }
        }
        else
        {
            renderer->Shutdown();
            SafeRelease(&renderer);
        }
    }

Exit:
    SafeRelease(&device);
    CoUninitialize();
    return 0;
}

