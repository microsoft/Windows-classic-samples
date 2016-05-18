// --------------------------------------------------------------------------
// MFWMAAEC (Aec-MicArray DMO) sample code.
//
// Note: 
//  1. DirectX SDK must be installed before compiling. 
//  2. DirectX SDK include path should be added after the VC include
//     path, because strsafe.h in DirectX SDK may be older.
//  3. platform SDK lib path should be added before the VC lib
//     path, because uuid.lib in VC lib path may be older
//  4. To run the demo properly for AEC enabled modes (mode 0 and 4), 
//     users must play some audio signals through the SAME speaker  
//     device specified for the DMO (i.e. the device specified by 
//     "-spkdev" option). These audio signals simulate far-end voices 
//     in a two-way chatting scenario. Users may use any player to play 
//     any audio signals. If there is no active render stream on the
//     selected speaker device, the AEC DMO will fail to process.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2004-2006, Microsoft Corporation. All rights reserved.
//---------------------------------------------------------------------------

#include <windows.h>
#include <dmo.h>
#include <Mmsystem.h>
#include <objbase.h>
#include <mediaobj.h>
#include <uuids.h>
#include <propidl.h>
#include <wmcodecdsp.h>

#include <atlbase.h>
#include <ATLComCli.h>
#include <audioclient.h>
#include <MMDeviceApi.h>
#include <AudioEngineEndPoint.h>
#include <DeviceTopology.h>
#include <propkey.h>
#include <strsafe.h>
#include <conio.h>

#include "mediabuf.h"
#include "AecKsBinder.h"

#define SAFE_ARRAYDELETE(p) {if (p) delete[] (p); (p) = NULL;}
#define SAFE_RELEASE(p) {if (NULL != p) {(p)->Release(); (p) = NULL;}}

#define VBFALSE (VARIANT_BOOL)0
#define VBTRUE  (VARIANT_BOOL)-1

#define STREAM_BUFFER_LENGTH 0.1f  //streaming buffer is 0.1 second long.

#define CHECK_RET(hr, message) if (FAILED(hr)) { puts(message); goto exit;}
#define CHECKHR(x) hr = x; if (FAILED(hr)) {printf("%d: %08X\n", __LINE__, hr); goto exit;}
#define CHECK_ALLOC(pb, message) if (NULL == pb) { puts(message); goto exit;}

class CStaticMediaBuffer : public CBaseMediaBuffer {
public:
    STDMETHODIMP_(ULONG) AddRef() {return 2;}
    STDMETHODIMP_(ULONG) Release() {return 1;}
    void Init(BYTE *pData, ULONG ulSize, ULONG ulData) {
        m_pData = pData;
        m_ulSize = ulSize;
        m_ulData = ulData;
    }
};

void OutputUsage();

int __cdecl _tmain(int argc, const TCHAR ** argv)
{
    HRESULT hr = S_OK;
    CoInitialize(NULL);

    IMediaObject* pDMO = NULL;    
    IPropertyStore* pPS = NULL;
    
    CStaticMediaBuffer outputBuffer;
    DMO_OUTPUT_DATA_BUFFER OutputBufferStruct = {0};
    OutputBufferStruct.pBuffer = &outputBuffer;
    DMO_MEDIA_TYPE mt = {0};

    ULONG cbProduced = 0;
    DWORD dwStatus;

    // Parameters to config DMO
    int  iSystemMode = MODE_NOT_SET;    // AEC-MicArray DMO system mode
    int  iOutFileIdx = -1;              // argument index for otuput file name
    int  iMicDevIdx = -2;               // microphone device index
    int  iSpkDevIdx = -2;               // speaker device index
    BOOL bFeatrModeOn = 0;              // turn feature mode on/off
    BOOL bNoiseSup = 1;                 // turn noise suppression on/off
    BOOL bAGC = 0;                      // turn digital auto gain control on/off
    BOOL bCntrClip = 0;                 // turn center clippng on/off

    // control how long the Demo runs
    int  iDuration = 60;   // seconds
    int  cTtlToGo = 0;

    FILE * pfMicOutPCM;  // dump output signal using PCM format

    DWORD cOutputBufLen = 0;
    BYTE *pbOutputBuffer = NULL;

    WAVEFORMATEX wfxOut = {WAVE_FORMAT_PCM, 1, 16000, 32000, 2, 16, 0};
    AUDIO_DEVICE_INFO *pCaptureDeviceInfo = NULL, *pRenderDeviceInfo = NULL;

    int i;
    for ( i = 1; i < argc-1; i++ )
    {
        if (argv[i][0] == '-') 
        {
            if ( !_tcscmp (_T("-micdev"), argv[i]))
            {   // microphone device index. The valid range is -1, 0~N-1
                // where N is the number of capture device. Use -1 if
                // you want to use the default device.
                iMicDevIdx = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-spkdev"), argv[i]))
            {   // speaker device index. The valid values are -1, 0~N-1
                // where N is the number of capture device. Use -1 if
                // you want to use the default device.
                iSpkDevIdx = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-out"), argv[i]))
            {   // output file name
                iOutFileIdx = i + 1; i++;
            }
            else if ( !_tcscmp (_T("-mod"), argv[i]))
            {   // AEC-MicArray system mode. The valid modes are
                //   SINGLE_CHANNEL_AEC = 0
                //   OPTIBEAM_ARRAY_ONLY = 2
                //   OPTIBEAM_ARRAY_AND_AEC = 4
                //   SINGLE_CHANNEL_NSAGC = 5
                //
                // Mode 1 and 3 are reserved for future features.
                iSystemMode = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-feat"), argv[i]))
            {   // turn feature mode on/off. The valid values are 0 or 1
                // The feature mode must be turned on in order to config
                // noise suppression, AGC, centerclip, and other AEC features.
                bFeatrModeOn = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-ns"), argv[i]))
            {   // turn noise suppression on/off. The valid values are 0 or 1
                // Feature mode must be on in order to set noise suppression
                bNoiseSup = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-agc"), argv[i]))
            {   // turn digital AGC on/off. The valid values are 0 or 1
                // Feature mode must be on in order to set digital AGC
                bAGC = _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-cntrclip"), argv[i]))
            {   // turn center clipping on/off. The valid values are 0 or 1
                // Center clipping is an post process to remove small echo residuals 
                // which are not completely cancelled. Comfort noise with a same level
                // of background noise will be filled after the removal.
                // Feature mode must be on in order to set center clipping
                bCntrClip = (BOOL) _ttoi ( argv[i+1] ); i++;
            }
            else if ( !_tcscmp (_T("-duration"), argv[i]))
            {   // control program running duration in seconds. The default 
                // value is 60 seconds.
                iDuration =  _ttoi ( argv[i+1] ); i++;
            }
            else
            {
                OutputUsage();
                goto exit;
            }
        }
    }

    // display usage info if required arguments are not specified
    if (iSystemMode == MODE_NOT_SET || iOutFileIdx == -1)
    {
        OutputUsage();
        goto exit;
    }

    HANDLE currThread;
    HANDLE currProcess;
    BOOL iRet;
    currProcess = GetCurrentProcess ();
    currThread = GetCurrentThread ();

    iRet = SetPriorityClass (currProcess, HIGH_PRIORITY_CLASS);
    if ( 0 == iRet )
    {
        // call getLastError.
        puts("failed to set process priority\n");
        goto exit;
    }

    // DMO initialization
    CHECKHR(CoCreateInstance(CLSID_CWMAudioAEC, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&pDMO));
    CHECKHR(pDMO->QueryInterface(IID_IPropertyStore, (void**)&pPS));

    // Select capture device
    UINT uCapDevCount = 0;
    UINT uRenDevCount = 0;
    char  pcScanBuf[256]= {0};

    hr = GetCaptureDeviceNum(uCapDevCount);
    CHECK_RET(hr, "GetCaptureDeviceNum failed");

    pCaptureDeviceInfo = new AUDIO_DEVICE_INFO[uCapDevCount];
    hr = EnumCaptureDevice(uCapDevCount, pCaptureDeviceInfo);
    CHECK_RET(hr, "EnumCaptureDevice failed");

    printf("\nSystem has totally %d capture devices\n", uCapDevCount);
    for (i=0; i<(int)uCapDevCount; i++)
    {
        _tprintf(_T("Device %d is %s"), i, pCaptureDeviceInfo[i].szDeviceName);
        if (pCaptureDeviceInfo[i].bIsMicArrayDevice)
            _tprintf(_T(" -- Mic Array Device \n"));
        else
            _tprintf(_T("\n"));
    }

    if (iMicDevIdx<-1 || iMicDevIdx>=(int)uCapDevCount)
    {
        do{
            printf("Select device ");
            scanf_s("%255s", pcScanBuf, 255); 
            iMicDevIdx = atoi(pcScanBuf);
            if (iMicDevIdx < -1 || iMicDevIdx >= (int)uCapDevCount)
                printf("Invalid Capture Device ID \n");
            else
                break;
        }while(1);
    }
    if (iMicDevIdx == -1)
        _tprintf(_T("\n Default device will be used for capturing \n"));
    else
        _tprintf(_T("\n %s is selected for capturing\n"), pCaptureDeviceInfo[iMicDevIdx].szDeviceName);
    SAFE_ARRAYDELETE(pCaptureDeviceInfo);
    

    // Select render device
    if( iSystemMode == SINGLE_CHANNEL_AEC ||
        iSystemMode == ADAPTIVE_ARRAY_AND_AEC ||
        iSystemMode == OPTIBEAM_ARRAY_AND_AEC )
    {
        hr = GetRenderDeviceNum(uRenDevCount);
        CHECK_RET(hr, "GetRenderDeviceNum failed");

        pRenderDeviceInfo = new AUDIO_DEVICE_INFO[uRenDevCount];
        hr = EnumRenderDevice(uRenDevCount, pRenderDeviceInfo);
        CHECK_RET(hr, "EnumRenderDevice failed");

        printf("\nSystem has totally %d render devices\n", uRenDevCount);
        for (i=0; i<(int)uRenDevCount; i++)
        {
            _tprintf(_T("Device %d is %s \n"), i, pRenderDeviceInfo[i].szDeviceName);
        }

        if (iSpkDevIdx<-1 || iSpkDevIdx>=(int)uRenDevCount)
        {
            do{
                printf("Select device ");
                scanf_s("%255s", pcScanBuf, 255); 
                iSpkDevIdx = atoi(pcScanBuf);
                if (iSpkDevIdx < -1 || iSpkDevIdx >= (int)uRenDevCount)
                    printf("Invalid Render Device ID \n");
                else
                    break;
            }while(1);
        }
        if (iSpkDevIdx == -1)
            _tprintf(_T("\n Default device will be used for rendering \n"));
        else
            _tprintf(_T("\n %s is selected for rendering \n"), pRenderDeviceInfo[iSpkDevIdx].szDeviceName);
    }else{
    iSpkDevIdx = -1;
    }

    SAFE_ARRAYDELETE(pRenderDeviceInfo);
    
    // --- PREPARE OUTPUT --- //
    if (NULL != _tfopen_s(&pfMicOutPCM, argv[iOutFileIdx], _T("wb")))
    { 
        puts("cannot open file for output.\n"); 
        goto exit; 
    }

    // Set AEC mode and other parameters
    // Not all user changeable options are given in this sample code.
    // Please refer to readme.txt for more options.

    // Set AEC-MicArray DMO system mode.
    // This must be set for the DMO to work properly
    puts("\nAEC settings:");
    PROPVARIANT pvSysMode;
    PropVariantInit(&pvSysMode);
    pvSysMode.vt = VT_I4;
    pvSysMode.lVal = (LONG)(iSystemMode);
    CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode));
    CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, &pvSysMode));
    printf("%20s %5d \n", "System Mode is", pvSysMode.lVal);
    PropVariantClear(&pvSysMode);

    // Tell DMO which capture and render device to use 
    // This is optional. If not specified, default devices will be used
    if (iMicDevIdx >= 0 || iSpkDevIdx >= 0)
    {
        PROPVARIANT pvDeviceId;
        PropVariantInit(&pvDeviceId);
        pvDeviceId.vt = VT_I4;
        pvDeviceId.lVal = (unsigned long)(iSpkDevIdx<<16) + (unsigned long)(0x0000ffff & iMicDevIdx);
        CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, pvDeviceId));
        CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_DEVICE_INDEXES, &pvDeviceId));
        PropVariantClear(&pvDeviceId);
    }

    if ( bFeatrModeOn )
    {
        // Turn on feature modes
        PROPVARIANT pvFeatrModeOn;
        PropVariantInit(&pvFeatrModeOn);
        pvFeatrModeOn.vt = VT_BOOL;
        pvFeatrModeOn.boolVal = bFeatrModeOn? VBTRUE : VBFALSE;
        CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_FEATURE_MODE, pvFeatrModeOn));
        CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATURE_MODE, &pvFeatrModeOn));
        printf("%20s %5d \n", "Feature Mode is", pvFeatrModeOn.boolVal);
        PropVariantClear(&pvFeatrModeOn);

        // Turn on/off noise suppression
        PROPVARIANT pvNoiseSup;
        PropVariantInit(&pvNoiseSup);
        pvNoiseSup.vt = VT_I4;
        pvNoiseSup.lVal = (LONG)bNoiseSup;
        CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_FEATR_NS, pvNoiseSup));
        CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_NS, &pvNoiseSup));
        printf("%20s %5d \n", "Noise suppresion is", pvNoiseSup.lVal);
        PropVariantClear(&pvNoiseSup);

        // Turn on/off AGC
        PROPVARIANT pvAGC;
        PropVariantInit(&pvAGC);
        pvAGC.vt = VT_BOOL;
        pvAGC.boolVal = bAGC ? VBTRUE : VBFALSE;
        CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_FEATR_AGC, pvAGC));
        CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_AGC, &pvAGC));
        printf("%20s %5d \n", "AGC is", pvAGC.boolVal);
        PropVariantClear(&pvAGC);

        // Turn on/off center clip
        PROPVARIANT pvCntrClip;
        PropVariantInit(&pvCntrClip);
        pvCntrClip.vt = VT_BOOL;
        pvCntrClip.boolVal = bCntrClip? VBTRUE : VBFALSE;
        CHECKHR(pPS->SetValue(MFPKEY_WMAAECMA_FEATR_CENTER_CLIP, pvCntrClip));
        CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_CENTER_CLIP, &pvCntrClip));
        printf("%20s %5d \n", "Center clip is", (BOOL)pvCntrClip.boolVal);
        PropVariantClear(&pvCntrClip);
    }

    // Set DMO output format
    hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    CHECK_RET(hr, "MoInitMediaType failed");
    
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;
    memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));
    
    hr = pDMO->SetOutputType(0, &mt, 0); 
    CHECK_RET(hr, "SetOutputType failed");
    MoFreeMediaType(&mt);

    // Allocate streaming resources. This step is optional. If it is not called here, it
    // will be called when first time ProcessInput() is called. However, if you want to 
    // get the actual frame size being used, it should be called explicitly here.
    hr = pDMO->AllocateStreamingResources();
    CHECK_RET(hr, "AllocateStreamingResources failed");
    
    // Get actually frame size being used in the DMO. (optional, do as you need)
    int iFrameSize;
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, &pvFrameSize));
    iFrameSize = pvFrameSize.lVal;
    PropVariantClear(&pvFrameSize);

    // allocate output buffer
    cOutputBufLen = wfxOut.nSamplesPerSec * wfxOut.nBlockAlign;
    pbOutputBuffer = new BYTE[cOutputBufLen];
    CHECK_ALLOC (pbOutputBuffer, "out of memory.\n");

   // number of frames to play
    cTtlToGo = iDuration * 100;

    // main loop to get mic output from the DMO
    puts("\nAEC-MicArray is running ... Press \"s\" to stop");
    while (1)
    {
        Sleep(10); //sleep 10ms

        if (cTtlToGo--<=0)
            break;

        do{
            outputBuffer.Init((byte*)pbOutputBuffer, cOutputBufLen, 0);
            OutputBufferStruct.dwStatus = 0;
            hr = pDMO->ProcessOutput(0, 1, &OutputBufferStruct, &dwStatus);
            CHECK_RET (hr, "ProcessOutput failed");

            if (hr == S_FALSE) {
                cbProduced = 0;
            } else {
                hr = outputBuffer.GetBufferAndLength(NULL, &cbProduced);
                CHECK_RET (hr, "GetBufferAndLength failed");
            }

            // dump output data into a file with PCM format.
            if (fwrite(pbOutputBuffer, 1, cbProduced, pfMicOutPCM) != cbProduced) 
            { 
                puts("write error"); 
                goto exit; 
            }
        } while (OutputBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

        // check keyboard input to stop
        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 's' || ch == 'S')
                break;
        }
    }

exit:

    SAFE_ARRAYDELETE(pbOutputBuffer);
    SAFE_ARRAYDELETE(pCaptureDeviceInfo);
    SAFE_ARRAYDELETE(pRenderDeviceInfo);

    SAFE_RELEASE(pDMO);
    SAFE_RELEASE(pPS);

    CoUninitialize();

    return hr;
}

void OutputUsage()
{
    printf("MFWMAAEC (Aec-MicArray DMO) Demo. \n");
    printf("Copyright (c) 2004-2006, Microsoft Corporation. All rights reserved. \n\n");
    printf("Usage: AecSDKDemo.exe -out mic_out.pcm -mod 0 [-feat 1] [-ns 1] [-agc 0] \n");
    printf("       [-cntrclip 0] [-micdev 0] [-spkdev 0] [-duration 60]\n");
    return;
}
