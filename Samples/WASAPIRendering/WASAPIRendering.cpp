//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
//
// WASAPIRendering.cpp : Scaffolding associated with the WASAPI Rendering sample application.
//

#include "pch.h"

#include <functiondiscoverykeys.h> // PKEY_Device_FriendlyName
#include "WASAPIRenderer.h"

#include "CmdLine.h"
#include "ToneGen.h"
#include <wil/result.h>
#include <wil/com.h>


int TargetFrequency = 440;
int TargetLatency = 30;
int TargetDurationInSec = 10;
bool ShowHelp;
bool UseConsoleDevice;
bool UseCommunicationsDevice;
bool UseMultimediaDevice;
bool DisableMMCSS;
bool EnableAudioViewManagerService;

wchar_t* OutputEndpoint;

CommandLineSwitch CmdLineArgs[] =
{
    { L"?", L"Print this help", CommandLineSwitch::SwitchTypeNone, &ShowHelp},
    { L"h", L"Print this help", CommandLineSwitch::SwitchTypeNone, &ShowHelp},
    { L"f", L"Sine wave frequency (Hz)", CommandLineSwitch::SwitchTypeInteger, &TargetFrequency, false},
    { L"l", L"Audio Render Latency (ms)", CommandLineSwitch::SwitchTypeInteger, &TargetLatency, false},
    { L"d", L"Sine Wave Duration (s)", CommandLineSwitch::SwitchTypeInteger, &TargetDurationInSec, false},
    { L"w", L"Enable call to AudioViewManagerService", CommandLineSwitch::SwitchTypeNone, &EnableAudioViewManagerService},
    { L"console", L"Use the default console device", CommandLineSwitch::SwitchTypeNone, &UseConsoleDevice},
    { L"communications", L"Use the default communications device", CommandLineSwitch::SwitchTypeNone, &UseCommunicationsDevice},
    { L"multimedia", L"Use the default multimedia device", CommandLineSwitch::SwitchTypeNone, &UseMultimediaDevice},
    { L"endpoint", L"Use the specified endpoint ID", CommandLineSwitch::SwitchTypeString, &OutputEndpoint, true},
};

size_t CmdLineArgLength = ARRAYSIZE(CmdLineArgs);

//
//  Print help for the sample
//
void Help(LPCWSTR ProgramName)
{
    printf("Usage: %ls [-/][Switch][:][Value]\n\n", ProgramName);
    printf("Where Switch is one of the following: \n");
    for (size_t i = 0; i < CmdLineArgLength; i += 1)
    {
        printf("    -%ls: %ls\n", CmdLineArgs[i].SwitchName, CmdLineArgs[i].SwitchHelp);
    }
}

//
//  Retrieves the device friendly name for a particular device in a device collection.
//
HRESULT GetDeviceName(IMMDeviceCollection* DeviceCollection, UINT DeviceIndex, LPWSTR* _deviceName)
{
    wil::com_ptr_nothrow<IMMDevice> device;
    wil::unique_cotaskmem_string deviceId;

    RETURN_IF_FAILED(DeviceCollection->Item(DeviceIndex, &device));

    RETURN_IF_FAILED(device->GetId(&deviceId));

    wil::com_ptr_nothrow<IPropertyStore> propertyStore;
    RETURN_IF_FAILED(device->OpenPropertyStore(STGM_READ, &propertyStore));

    wil::unique_prop_variant friendlyName;
    RETURN_IF_FAILED(propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName));

    wil::unique_cotaskmem_string deviceName;
    RETURN_IF_FAILED(wil::str_printf_nothrow(deviceName, L"%ls (%ls)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId.get()));

    *_deviceName = deviceName.release();

    return S_OK;
}
//
//  Based on the input switches, pick the specified device to use.
//
HRESULT PickDevice(IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole)
{
    wil::com_ptr_nothrow<IMMDeviceEnumerator> deviceEnumerator;
    wil::com_ptr_nothrow<IMMDeviceCollection> deviceCollection;
    wil::com_ptr_nothrow<IMMDevice> device;

    *IsDefaultDevice = false;   // Assume we're not using the default device.

    RETURN_IF_FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator)));

    //
    //  First off, if none of the console switches was specified, use the console device.
    //
    if (!UseConsoleDevice && !UseCommunicationsDevice && !UseMultimediaDevice && OutputEndpoint == nullptr)
    {
        //
        //  The user didn't specify an output device, prompt the user for a device and use that.
        //
        RETURN_IF_FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection));

        printf("Select an output device:\n");
        printf("    0:  Default Console Device\n");
        printf("    1:  Default Communications Device\n");
        printf("    2:  Default Multimedia Device\n");
        UINT deviceCount;
        RETURN_IF_FAILED(deviceCollection->GetCount(&deviceCount));

        for (UINT i = 0; i < deviceCount; i += 1)
        {
            wil::unique_cotaskmem_string deviceName;

            RETURN_IF_FAILED(GetDeviceName(deviceCollection.get(), i, &deviceName));
            printf("    %d:  %ls\n", i + 3, deviceName.get());
        }
        wchar_t choice[10];
        _getws_s(choice);   // Note: Using the safe CRT version of _getws.

        long deviceIndex;
        wchar_t* endPointer;

        deviceIndex = wcstoul(choice, &endPointer, 0);
        if (deviceIndex == 0 && endPointer == choice)
        {
            printf("unrecognized device index: %ls\n", choice);
            return E_UNEXPECTED;
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
            RETURN_IF_FAILED(deviceCollection->Item(deviceIndex - 3, &device));
            break;
        }
    }
    else if (OutputEndpoint != nullptr)
    {
        RETURN_IF_FAILED(deviceEnumerator->GetDevice(OutputEndpoint, &device));
    }

    if (device == nullptr)
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
        RETURN_IF_FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, deviceRole, &device));
        *IsDefaultDevice = true;
        *DefaultDeviceRole = deviceRole;
    }

    *DeviceToUse = device.detach();

    return S_OK;
}

int wmain(int argc, wchar_t* argv[])
{
    printf("WASAPI Render Shared Event Driven Sample\n");
    printf("Copyright (c) Microsoft.  All Rights Reserved\n");
    printf("\n");

    if (!ParseCommandLine(argc, argv, CmdLineArgs, CmdLineArgLength))
    {
        Help(argv[0]);
        return -1;
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
        return 0;
    }

    //
    //  The user can only specify one of -console, -communications or -multimedia or a specific endpoint.
    //
    if (((UseConsoleDevice != 0) + (UseCommunicationsDevice != 0) + (UseMultimediaDevice != 0) + (OutputEndpoint != nullptr)) > 1)
    {
        printf("Can only specify one of -Console, -Communications, -Multimedia, or a specific endpoint.\n");
        return -1;
    }


    //
    //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
    //
    if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        printf("Unable to initialize COM\n");
        return -1;
    }
    wil::unique_couninitialize_call comUninitialize;

    //
    //  Now that we've parsed our command line, pick the device to render.
    //
    wil::com_ptr_nothrow<IMMDevice> device;
    bool isDefaultDevice;
    ERole role;
    if (PickDevice(&device, &isDefaultDevice, &role) != S_OK)
    {
        return -1;
    }

    printf("Render a %d hz Sine wave for %d seconds\n", TargetFrequency, TargetDurationInSec);

    //
    //  Instantiate a renderer and play a sound for TargetDuration seconds
    //
    //  Configure the renderer to enable stream switching on the specified role if the user specified one of the default devices.
    //
    CWASAPIRenderer renderer;
    renderer.SetUp(device.get(), isDefaultDevice, role, EnableAudioViewManagerService);

    if (renderer.Initialize(TargetLatency) == S_OK)
    {
        //
        //  We've initialized the renderer.  Once we've done that, we know some information about the
        //  mix format and we can allocate the buffer that we're going to render.
        //
        //  The buffer is going to contain "TargetDuration" seconds worth of PCM data.  That means
        //  we're going to have TargetDuration*samples/second frames multiplied by the frame size.
        //
        UINT32 renderBufferSizeInBytes = (renderer.BufferSizePerPeriod() * renderer.FrameSize());
        size_t renderDataLength = (renderer.SamplesPerSecond() * TargetDurationInSec * renderer.FrameSize()) + (renderBufferSizeInBytes - 1);
        size_t renderBufferCount = renderDataLength / (renderBufferSizeInBytes);

        //
        //  Build the render buffer queue.
        //
        std::forward_list<RenderBuffer> renderQueue;
        // Keep an iterator to the tail of the list so we can append elements.
        auto renderQueueTail = renderQueue.before_begin();

        double theta = 0;

        for (size_t i = 0; i < renderBufferCount; i += 1)
        {
            try
            {
                // Append another buffer to the queue.
                renderQueueTail = renderQueue.emplace_after(renderQueueTail, renderBufferSizeInBytes);
            }
            catch (std::bad_alloc const&)
            {
                printf("Unable to allocate render buffer\n");
                return -1;
            }

            RenderBuffer& renderBuffer = *renderQueueTail;

            //
            //  Generate tone data in the buffer.
            //
            switch (renderer.SampleType())
            {
            case CWASAPIRenderer::RenderSampleType::Float:
                GenerateSineSamples<float>(renderBuffer._buffer.get(), renderBuffer._bufferLength, TargetFrequency,
                    renderer.ChannelCount(), renderer.SamplesPerSecond(), &theta);
                break;
            case CWASAPIRenderer::RenderSampleType::Pcm16Bit:
                GenerateSineSamples<short>(renderBuffer._buffer.get(), renderBuffer._bufferLength, TargetFrequency,
                    renderer.ChannelCount(), renderer.SamplesPerSecond(), &theta);
                break;
            }
        }

        if (SUCCEEDED(renderer.Start(std::move(renderQueue))))
        {
            do
            {
                printf(".");
                Sleep(1000);
            } while (--TargetDurationInSec);
            printf("\n");
            renderer.Stop();
        }
    }
    renderer.Shutdown();

    return 0;
}

