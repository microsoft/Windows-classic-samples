#include <Windows.h>
#include <propkeydef.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "AECCapture.h"

HRESULT GetRenderEndpointId(PWSTR* endpointId)
{
    *endpointId = nullptr;

    wil::com_ptr_nothrow<IMMDeviceEnumerator> deviceEnumerator;
    RETURN_IF_FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator));

    wil::com_ptr_nothrow<IMMDeviceCollection> spDeviceCollection;
    RETURN_IF_FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &spDeviceCollection));

    UINT deviceCount;
    RETURN_IF_FAILED(spDeviceCollection->GetCount(&deviceCount));

    wprintf(L"0: system default\n");

    for (UINT i = 0; i < deviceCount; i++)
    {
        // Get the device from the collection.
        wil::com_ptr_nothrow<IMMDevice> device;
        RETURN_IF_FAILED(spDeviceCollection->Item(i, &device));

        // Get the device friendly name.
        wil::com_ptr_nothrow<IPropertyStore> properties;
        RETURN_IF_FAILED(device->OpenPropertyStore(STGM_READ, &properties));
        wil::unique_prop_variant variant;
        RETURN_IF_FAILED(properties->GetValue(PKEY_Device_FriendlyName, &variant));

        wprintf(L"%d: %ls\n", i + 1, variant.pwszVal);
    }

    wprintf(L"Choose a device to use as the acoustic echo cancellation render endpoint: ");
    fflush(stdout);

    UINT index;
    if (wscanf_s(L"%u", &index) != 1)
    {
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    if (index == 0)
    {
        // nullptr means "use the system default"
        *endpointId = nullptr;
        return S_OK;
    }

    // Convert from 1-based index to 0-based index.
    index = index - 1;

    if (index > deviceCount)
    {
        wprintf(L"Invalid choice.\n");
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    // Get the chosen device from the collection.
    wil::com_ptr_nothrow<IMMDevice> device;
    RETURN_IF_FAILED(spDeviceCollection->Item(index, &device));

    // Get and return the endpoint ID for that device.
    RETURN_IF_FAILED(device->GetId(endpointId));

    return S_OK;
}

int wmain(int argc, wchar_t* argv[])
{
    // Print diagnostic messages to the console for developer convenience.
    wil::SetResultLoggingCallback([](wil::FailureInfo const& failure) noexcept
        {
            wchar_t message[1024];
            wil::GetFailureLogString(message, ARRAYSIZE(message), failure);
            wprintf(L"Diagnostic message: %ls\n", message);
        });


    RETURN_IF_FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED));
    wil::unique_couninitialize_call uninitialize;

    wil::unique_cotaskmem_string endpointId;
    RETURN_IF_FAILED(GetRenderEndpointId(&endpointId));

    CAECCapture aecCapture;
    RETURN_IF_FAILED(aecCapture.StartCapture());

    // Make sure we Stop capture even if an error occurs.
    auto stop = wil::scope_exit([&]
        {
            aecCapture.StopCapture();
        });

    RETURN_IF_FAILED(aecCapture.SetEchoCancellationRenderEndpoint(endpointId.get()));

    // Capture for 10 seconds.
    wprintf(L"Capturing for 10 seconds...\n");
    Sleep(10000);
    wprintf(L"Finished.\n");

    return 0;
}
