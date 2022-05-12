#pragma once

#include <AudioClient.h>
#include <mmdeviceapi.h>
#include <initguid.h>
#include <guiddef.h>
#include <mfapi.h>

#include <wrl\implements.h>
#include <wil\com.h>
#include <wil\result.h>

#include <thread>

class CAECCapture :
    public Microsoft::WRL::RuntimeClass< Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::ClassicCom >, Microsoft::WRL::FtmBase >
{
public:
    HRESULT StartCapture();
    HRESULT StopCapture();

    HRESULT SetEchoCancellationRenderEndpoint(PCWSTR aecReferenceEndpointId);

private:
    HRESULT InitializeCaptureClient();
    HRESULT IsAcousticEchoCancellationEffectPresent(bool* result);
    HRESULT RecordCommunicationsStream();

    std::thread _captureThread;
    wil::com_ptr_nothrow<IAudioCaptureClient> _captureClient;
    wil::com_ptr_nothrow<IAudioClient2> _audioClient;
    
    wil::unique_event_nothrow _terminationEvent;
};
