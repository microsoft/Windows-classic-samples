#include <windows.h>
#include <wchar.h>
#include <AudioSessionTypes.h>

#include "AECCapture.h"

#if !defined(NTDDI_WIN10_NI) || (NTDDI_VERSION < NTDDI_WIN10_NI)
#error This sample requires SDK version 22540 or higher.
#endif

#define REFTIMES_PER_SEC  10000000

HRESULT CAECCapture::IsAcousticEchoCancellationEffectPresent(bool* result)
{
    *result = false;

    // IAudioEffectsManager requires build 22000 or higher.
    wil::com_ptr_nothrow<IAudioEffectsManager> audioEffectsManager;
    HRESULT hr = _audioClient->GetService(IID_PPV_ARGS(&audioEffectsManager));
    if (hr == E_NOINTERFACE)
    {
        // Audio effects manager is not supported, so clearly not present.
        return S_OK;
    }

    wil::unique_cotaskmem_array_ptr<AUDIO_EFFECT> effects;
    UINT32 numEffects;
    RETURN_IF_FAILED(audioEffectsManager->GetAudioEffects(&effects, &numEffects));

    for (UINT32 i = 0; i < numEffects; i++)
    {
        // Check for acoustic echo cancellation Audio Processing Object (APO)
        if (effects[i].id == AUDIO_EFFECT_TYPE_ACOUSTIC_ECHO_CANCELLATION)
        {
            *result = true;
            break;
        }
    }
    return S_OK;
}

HRESULT CAECCapture::InitializeCaptureClient()
{
    wil::com_ptr_nothrow<IMMDeviceEnumerator> enumerator;
    RETURN_IF_FAILED(::CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)));

    wil::com_ptr_nothrow<IMMDevice> device;
    RETURN_IF_FAILED(enumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &device));

    RETURN_IF_FAILED(device->Activate(__uuidof(IAudioClient2), CLSCTX_INPROC_SERVER, NULL, (void**)&_audioClient));

    // Set the category as communications.
    AudioClientProperties clientProperties = {};
    clientProperties.cbSize = sizeof(AudioClientProperties);
    clientProperties.eCategory = AudioCategory_Communications;
    RETURN_IF_FAILED(_audioClient->SetClientProperties(&clientProperties));

    wil::unique_cotaskmem_ptr<WAVEFORMATEX> wfxCapture;
    RETURN_IF_FAILED(_audioClient->GetMixFormat(wil::out_param(wfxCapture)));

    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    RETURN_IF_FAILED(_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        hnsRequestedDuration,
        0,
        wfxCapture.get(),
        NULL));

    bool aecEffectPresent;
    RETURN_IF_FAILED(IsAcousticEchoCancellationEffectPresent(&aecEffectPresent));

    if (!aecEffectPresent)
    {
        wprintf(L"Warning: Capture stream is not echo cancelled.\n");

        // An APO vendor can add code here to insert an in-app
        // acoustic echo cancellation APO before starting the capture stream.
    }

    wil::unique_cotaskmem_string deviceId;
    RETURN_IF_FAILED(device->GetId(&deviceId));
    wprintf(L"Created communications stream on capture endpoint %ls\n", deviceId.get());

    RETURN_IF_FAILED(_audioClient->GetService(IID_PPV_ARGS(&_captureClient)));

    return S_OK;
}

HRESULT CAECCapture::RecordCommunicationsStream()
{
    DWORD mmcssTaskIndex = 0;
    HANDLE mmcssTaskHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
    RETURN_HR_IF(HRESULT_FROM_WIN32(GetLastError()), mmcssTaskHandle == 0);

    auto avRevertMmThreadCharacteristicsOnExit = wil::scope_exit([&]()
        {
            AvRevertMmThreadCharacteristics(mmcssTaskHandle);
        });

    wil::unique_event_nothrow bufferComplete;
    RETURN_IF_FAILED(bufferComplete.create());
    RETURN_IF_FAILED(_audioClient->SetEventHandle(bufferComplete.get()));

    RETURN_IF_FAILED(_audioClient->Start());

    wprintf(L"Started communications capture stream.\n");

    HANDLE events[] = { _terminationEvent.get(), bufferComplete.get() };

    while (WAIT_OBJECT_0 != WaitForMultipleObjects(ARRAYSIZE(events), events, FALSE, INFINITE))
    {
        UINT32 packetLength = 0;
        while (SUCCEEDED(_captureClient->GetNextPacketSize(&packetLength)) && packetLength > 0)
        {
            PBYTE buffer;
            UINT32 numFramesRead;
            DWORD flags = 0;
            RETURN_IF_FAILED(_captureClient->GetBuffer(&buffer, &numFramesRead, &flags, nullptr, nullptr));

            // At this point, the app can send the buffer to the capture pipeline.
            // This program just discards the buffer without processing it.

            RETURN_IF_FAILED(_captureClient->ReleaseBuffer(numFramesRead));
        }
    }

    RETURN_IF_FAILED(_audioClient->Stop());

    return S_OK;
}

HRESULT CAECCapture::StartCapture() try
{
    RETURN_IF_FAILED(_terminationEvent.create());

    RETURN_IF_FAILED(InitializeCaptureClient());

    _captureThread = std::thread(
        [this]()
        {
            RecordCommunicationsStream();
        });

    return S_OK;
} CATCH_RETURN()

HRESULT CAECCapture::StopCapture()
{
    _terminationEvent.SetEvent();
    _captureThread.join();
    return S_OK;
}

HRESULT CAECCapture::SetEchoCancellationRenderEndpoint(PCWSTR aecReferenceEndpointId)
{
    wil::com_ptr_nothrow<IAcousticEchoCancellationControl> aecControl;
    HRESULT hr = _audioClient->GetService(IID_PPV_ARGS(&aecControl));

    if (hr == E_NOINTERFACE)
    {
        // For this app, we ignore any failure to to control acoustic echo cancellation.
        // (Treat as best effort.)
        wprintf(L"Warning: Acoustic echo cancellation control is not available.\n");
        return S_OK;
    }

    RETURN_IF_FAILED_MSG(hr, "_audioClient->GetService(IID_PPV_ARGS(&aecControl))");

    // Call SetEchoCancellationRenderEndpoint to change the endpoint of the auxiliary input stream.
    RETURN_IF_FAILED(aecControl->SetEchoCancellationRenderEndpoint(aecReferenceEndpointId));

    return S_OK;
}
