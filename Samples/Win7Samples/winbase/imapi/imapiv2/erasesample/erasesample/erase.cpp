/*--

Copyright (C) Microsoft Corporation, 2006

Implementation of CMsftEraseSample

--*/

#include "stdafx.h"
#include "erase.h"


// ISupportErrorInfo
STDMETHODIMP CMsftEraseSample::InterfaceSupportsErrorInfo(REFIID riid)
{
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL; // ISupportErrorInfo
}
VOID CMsftEraseSample::FinalRelease( void )
{
    SysFreeStringAndNull(m_ClientName);
    ReleaseAndNull(m_Recorder);
    ReleaseAndNull(m_RecorderEx);
}
HRESULT CMsftEraseSample::FinalConstruct( void )
{
    return S_OK;
}

// EraseSample methods
STDMETHODIMP CMsftEraseSample::put_Recorder(IDiscRecorder2* value)
{
    HRESULT             hr = S_OK;
    IDiscRecorder2Ex*   tmpRecorderEx = NULL;

    if (value != NULL)
    {
        // check if the recorder is supported
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL supported = VARIANT_FALSE;
            hr = IsRecorderSupported(value, &supported);
            if (SUCCEEDED(hr) && (supported == VARIANT_FALSE))
            {
                hr = E_IMAPI_ERASE_RECORDER_NOT_SUPPORTED;
            }
        }

        // get a pointer to the IDR2Ex interface as well
        if (SUCCEEDED(hr))
        {
            hr = value->QueryInterface(IID_PPV_ARGS(&tmpRecorderEx));
        }
    }

    // store the value
    if (SUCCEEDED(hr))
    {
        ReleaseAndNull(m_Recorder);
        ReleaseAndNull(m_RecorderEx);
        if (value != NULL) 
        { 
            value->AddRef(); 
        }
        m_Recorder = value;
        m_RecorderEx = tmpRecorderEx;
    }

    return hr;
}

STDMETHODIMP CMsftEraseSample::get_Recorder(IDiscRecorder2** value)
{
    HRESULT hr = S_OK;

    if (value == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *value= NULL;
    }

    if (SUCCEEDED(hr))
    {
        if (m_Recorder != NULL)
        {
            m_Recorder->AddRef();
        }
        *value = m_Recorder;
    }

    return hr;
}

STDMETHODIMP CMsftEraseSample::put_ClientName(BSTR value)
{
    HRESULT hr = S_OK;
    BSTR    tmpClientName = NULL;

    // make a copy, and only use copy (prevents modification during use)
    if (SUCCEEDED(hr) && (::SysStringLen(value) != 0))
    {
        tmpClientName = SysAllocString(value);
        if (tmpClientName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // validate the input is a proper client name (both length and characters)
    if (SUCCEEDED(hr))
    {
        if (!BstrIsValidClientName(tmpClientName))
        {
            hr = E_IMAPI_ERASE_CLIENT_NAME_IS_NOT_VALID;
        }
    }

    // save the result on success
    if (SUCCEEDED(hr))
    {
        m_ClientName = tmpClientName;
    }
    else
    {
        SysFreeStringAndNull(tmpClientName);
    }

    return hr;
}

STDMETHODIMP CMsftEraseSample::get_ClientName(BSTR* value)
{
    HRESULT hr = S_OK;
    BSTR    tmpClientName = NULL;

    // Validate parameters and initialize OUT parameters
    if (value == NULL)
    {
        hr = E_POINTER;
    }

    // allocate a copy for the client
    if (SUCCEEDED(hr) && (::SysStringLen(m_ClientName) != 0))
    {
        tmpClientName = ::SysAllocString(m_ClientName);
        if (tmpClientName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        *value = tmpClientName;
    }
    else
    {
        SysFreeStringAndNull(tmpClientName);
    }
    return hr;
}

STDMETHODIMP CMsftEraseSample::put_FullErase(VARIANT_BOOL value)
{
    m_FullErase = value;

    return S_OK;
}

STDMETHODIMP CMsftEraseSample::get_FullErase(VARIANT_BOOL* value)
{
    HRESULT hr;

    if (value == NULL) 
    {
        hr = E_POINTER;
    } 
    else 
    {
        *value = m_FullErase;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CMsftEraseSample::IsRecorderSupported(IDiscRecorder2* recorder, VARIANT_BOOL* value)
{
    HRESULT             hr = S_OK;
    IDiscRecorder2Ex*   tmpRecorderEx = NULL;

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }
    if (value == NULL)
    {
        hr = E_POINTER;
    }

    // get a pointer to the IDR2Ex interface as well
    if (SUCCEEDED(hr))
    {
        hr = recorder->QueryInterface(IID_PPV_ARGS(&tmpRecorderEx));
    }

    if (SUCCEEDED(hr))
    {
        *value = CheckRecorderSupportsErase(tmpRecorderEx, FALSE) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    ReleaseAndNull(tmpRecorderEx);

    return hr;
}

STDMETHODIMP CMsftEraseSample::IsCurrentMediaSupported(IDiscRecorder2* Recorder, VARIANT_BOOL* Supported)
{
    IDiscRecorder2Ex*   tmpRecorderEx = NULL;
    BOOLEAN             tmpSupported = FALSE;
    HRESULT             hr = S_OK;
    PDISC_INFORMATION   discInfo = NULL;
    ULONG               discInfoSize = 0;
    ULONG               requiredSize;

    if (Recorder == NULL) 
    {
        hr = E_POINTER;
    }

    if (Supported == NULL) 
    {
        hr = E_POINTER;
    } 
    else 
    {
        *Supported = VARIANT_FALSE;
    }

    // get a pointer to the IDR2Ex interface as well
    if (SUCCEEDED(hr))
    {
        hr = Recorder->QueryInterface(IID_PPV_ARGS(&tmpRecorderEx));
    }

    if (SUCCEEDED (hr)) 
    {
        tmpSupported = CheckRecorderSupportsErase(tmpRecorderEx, TRUE);
    }

    //
    // If it's supported, and they are asking about the current media,
    // need to check that the disc is actually erasable via READ_DISC_INFO
    //
        
    if (SUCCEEDED (hr) && (tmpSupported)) 
    {
        tmpSupported = FALSE;

        requiredSize = RTL_SIZEOF_THROUGH_FIELD (DISC_INFORMATION, 
                                                 FirstTrackNumber);

        hr = GetDiscInformation (tmpRecorderEx, &discInfo, &discInfoSize, requiredSize);

        if (SUCCEEDED (hr)) 
        {
            assert (discInfo != NULL);
            assert (discInfoSize >= 12);

            if (discInfo->Erasable) 
            {
                tmpSupported = TRUE;
            }

            CoTaskMemFreeAndNull( discInfo );
        }
    }

    if (SUCCEEDED(hr)) 
    {
        *Supported = tmpSupported ? VARIANT_TRUE : VARIANT_FALSE;
    }

    // release the temporary IDiscRecorder2Ex
    ReleaseAndNull(tmpRecorderEx);
    return hr;

}

STDMETHODIMP CMsftEraseSample::get_CurrentPhysicalMediaType(IMAPI_MEDIA_PHYSICAL_TYPE* value)
{
    HRESULT hr = S_OK;

    if (m_RecorderEx == NULL)
    {
        hr = E_IMAPI_RECORDER_REQUIRED;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetCurrentPhysicalMediaType(m_RecorderEx, value);
    }

    return hr;
}

HRESULT CMsftEraseSample::get_MediaPhysicallyBlank(VARIANT_BOOL* value)
{
    HRESULT hr = S_OK;

    // Parameter Validation
    if ((m_RecorderEx == NULL) ||
        (value == NULL))
    {
        return E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetMediaPhysicallyBlank(m_RecorderEx, value);
    }

    return hr;
}

HRESULT CMsftEraseSample::get_MediaHeuristicallyBlank(VARIANT_BOOL* value)
{
    HRESULT hr = S_OK;

    // Parameter Validation
    if ((m_RecorderEx == NULL) ||
        (value == NULL))
    {
        return E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetMediaHeuristicallyBlank(m_RecorderEx, value);
    }

    return hr;
}

HRESULT CMsftEraseSample::get_SupportedMediaTypes(SAFEARRAY** value)
{
    HRESULT     hr = S_OK;
    SAFEARRAY*  tmpValue = NULL;

    // Validate parameters and initialize OUT parameters
    if (value == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *value = NULL;
    }
    // allocate a SAFEARRAY for the results
    if (SUCCEEDED(hr))
    {
        // Assert that the cast next line is safe
        assert(sizeof(g_EraseSupportedMediaTypes[0]) == sizeof(LONG));
        hr = CreateVariantSafeArrayFromEnums((LONG*)g_EraseSupportedMediaTypes,
                                              g_EraseSupportedMediaTypesCount,
                                              &tmpValue
                                              );
    }
    // save it
    if (SUCCEEDED(hr))
    {
        *value = tmpValue;
    }
    else
    {
        SafeArrayDestroyAndNull(tmpValue);
    }
    return hr;

}

HRESULT CMsftEraseSample::WaitForReadDiscInfoToWorkAfterBlank(ULONG EstimatedMillisecondsToCompletion)
{
    HRESULT                         hr = S_OK;
    CTaskTimeEstimator              tmpBlankTime(EstimatedMillisecondsToCompletion, 0x10000);
    ERASE_UPDATE_CALLBACK_CONTEXT   eraseCallbackContext;

    RtlZeroMemory(&eraseCallbackContext, sizeof(ERASE_UPDATE_CALLBACK_CONTEXT));

    eraseCallbackContext.Erase     = this;
    eraseCallbackContext.BlankTime = &tmpBlankTime;
    eraseCallbackContext.ExpectedCycleCount = 1;

    tmpBlankTime.StartNow();
    hr =  WaitForReadDiscInfo(m_RecorderEx,
                             1 * 60 * 60, // allow to run for upto 60 minutes (1 hour)
                             &eraseCallbackContext,
                             (READ_DISC_INFO_CALLBACK)EraseUpdateCallBack
                             );
    tmpBlankTime.EndNow();

    // See if the wait failed or not
    if (FAILED(hr))
    {
        if (tmpBlankTime.get_TotalMilliseconds() >= (1 * 60 * 60))
        {
            hr = E_IMAPI_ERASE_TOOK_LONGER_THAN_ONE_HOUR;
        }
    }

    return hr;
}

BOOLEAN CMsftEraseSample::CheckRecorderSupportsErase(__in IDiscRecorder2Ex* DiscRecorder, BOOLEAN CheckCurrentMedia)
{
    PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE incrementalStreaming = NULL;
    ULONG                                       incrementalStreamingSize = 0;
    PFEATURE_DATA_RESTRICTED_OVERWRITE          restrictedOverwrite = NULL;
    ULONG                                       restrictedOverwriteSize = 0;
    PFEATURE_DATA_CD_TRACK_AT_ONCE              cdTao = NULL;
    ULONG                                       cdTaoSize = 0;
    PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE   rigidOverwrite = NULL;
    ULONG                                       rigidOverwriteSize = 0;
    PFEATURE_DATA_DVD_RECORDABLE_WRITE          dvdDash = NULL;
    ULONG                                       dvdDashSize = 0;
    PFEATURE_DATA_DDCD_RW_WRITE                 ddCdrwWrite = NULL;
    ULONG                                       ddCdrwWriteSize = 0;
    PCDVD_CAPABILITIES_PAGE                     capabilities;
    ULONG                                       capabilitiesSize;
    HRESULT                                     hr = S_OK;
    ULONG                                       requiredSize = 0;
    BOOLEAN                                     supported = FALSE;

    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_INCREMENTAL_STREAMING_WRITABLE,
        FALSE,
        (BYTE**)&incrementalStreaming,
        &incrementalStreamingSize);
   
    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_RESTRICTED_OVERWRITE,
        FALSE,
        (BYTE**)&restrictedOverwrite,
        &restrictedOverwriteSize);

    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_CD_TRACK_AT_ONCE,
        FALSE,
        (BYTE**)&cdTao,
        &cdTaoSize);

    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_RIGID_RESTRICTED_OVERWRITE,
        FALSE,
        (BYTE**)&rigidOverwrite,
        &rigidOverwriteSize);

    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_DVD_DASH_WRITE,
        FALSE,
        (BYTE**)&dvdDash,
        &dvdDashSize);
    
    DiscRecorder->GetFeaturePage (
        IMAPI_FEATURE_PAGE_TYPE_DOUBLE_DENSITY_CD_RW_WRITE,
        FALSE,
        (BYTE**)&ddCdrwWrite,
        &ddCdrwWriteSize);

    //
    // The recorder is supported if any of the following is true:
    // incremental streaming write requires blank
    // restricted overwrite requires blank
    // cdtao requires blank
    // rigid restricted overwrite requires blank if blank bit set
    // DVD-R/-RW requires blank if DVD-RW bit set
    // DDCD-RW feature requires blank if blank bit set
    //

    if ((incrementalStreaming != NULL) && 
        (incrementalStreaming->Header.Current || !CheckCurrentMedia))
    {
        supported = TRUE;
    }
        
    if ((restrictedOverwrite != NULL) &&
        (restrictedOverwrite->Header.Current || !CheckCurrentMedia)) 
    {
        supported = TRUE;
    }

    if ((cdTao != NULL) && 
        (cdTao->Header.Current || !CheckCurrentMedia))
    {
        supported = TRUE;
    }
    
    if ((rigidOverwrite != NULL) &&
        (rigidOverwrite->Header.Current || !CheckCurrentMedia) &&
        (rigidOverwriteSize >= 8) &&
        (rigidOverwrite->Blank != 0)) 
    {
        supported = TRUE;
    }

    if ((dvdDash != NULL) &&
        (dvdDash->Header.Current || !CheckCurrentMedia) &&
        (dvdDashSize >= 8) &&
        (dvdDash->DVD_RW != 0)) 
    {
        supported = TRUE;
    }

    if ((ddCdrwWrite != NULL) &&
        (ddCdrwWrite->Header.Current || !CheckCurrentMedia) &&
        (ddCdrwWriteSize >= 8) &&
        (ddCdrwWrite->Blank != 0)) 
    {
        supported = TRUE;
    }

    //
    // some older CD-R/RW drives didn't support GET_CONFIGURATION
    //

    if (!supported) 
    {
        capabilities = NULL;
        capabilitiesSize = 0;
        
        //
        // if none of the features are available, it's possible we may
        // still need to support this device if it's a cd recorder according
        // to mode page 2Ah.
        //

        requiredSize = RTL_SIZEOF_THROUGH_FIELD (CDVD_CAPABILITIES_PAGE,
                                                 NumberVolumeLevels);

        
        hr = GetDiscCapabilities (DiscRecorder,
                                  &capabilities, 
                                  &capabilitiesSize,
                                  requiredSize);
        
        if (SUCCEEDED (hr)) 
        {
            assert (capabilities != NULL);
            assert (capabilitiesSize >=8);
            
            if (capabilities->CDEWrite) 
            {
                supported = TRUE;
            }
            
            CoTaskMemFreeAndNull (capabilities);
        }
    }
    
    CoTaskMemFreeAndNull (ddCdrwWrite);
    CoTaskMemFreeAndNull (dvdDash);
    CoTaskMemFreeAndNull (rigidOverwrite);
    CoTaskMemFreeAndNull (cdTao);
    CoTaskMemFreeAndNull (restrictedOverwrite);
    CoTaskMemFreeAndNull (incrementalStreaming);

    return supported;
}

STDMETHODIMP CMsftEraseSample::EraseMedia()
{
    HRESULT                     hr = S_OK;
    BOOLEAN                     mediaLocked = FALSE;
    BOOLEAN                     exclusiveAccessObtained = FALSE;
    IMAPI_MEDIA_PHYSICAL_TYPE   mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
    ULONG                       millisecondsToErase = MILLISECONDS_FROM_SECONDS(60); // some reasonable default

    if (m_Recorder == NULL)
    {
        return E_POINTER;
    }

    // check that the current media is supported before continuing
    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL isMediaSupported = VARIANT_FALSE;
        hr = IsCurrentMediaSupported(m_Recorder, &isMediaSupported);

        if (SUCCEEDED(hr) &&
            (isMediaSupported == VARIANT_FALSE))
        {
            // media not supported, fail out!
            hr = E_IMAPI_ERASE_MEDIA_IS_NOT_SUPPORTED;
        }
    }

    // get exclusive access, so no other applications could access the drive.
    if (SUCCEEDED(hr))
    {
        hr = m_Recorder->AcquireExclusiveAccess(TRUE, m_ClientName);
        if (SUCCEEDED(hr))
        {
            exclusiveAccessObtained = TRUE;
        }
    }    

    // lock the media in the tray
    if (SUCCEEDED(hr))
    {
        hr = PreventAllowMediumRemoval(m_RecorderEx, TRUE);
        if (SUCCEEDED(hr))
        {
            mediaLocked = TRUE;
        }
    }

    // don't spinup until we have exclusive access
    if (SUCCEEDED(hr))
    {
        hr = SendStartStopUnitCommand(m_RecorderEx, StartSpinning);

        if (FAILED(hr))
        {
            hr = E_IMAPI_ERASE_DRIVE_FAILED_SPINUP_COMMAND;
        }
    }

    // cache the media type
    if (SUCCEEDED(hr))
    {
        hr = GetCurrentPhysicalMediaType(m_RecorderEx, &mediaType);
    }

    // set media to fastest speed
    if (SUCCEEDED(hr))
    {
        hr = SendSetCDSpeed(m_Recorder, mediaType, 0xFFFFFFFF, 0);
        if (FAILED(hr))
        {
            hr = S_OK;
        }
    }

    millisecondsToErase = 0;

    // determine the estimate erasing time.
    if (SUCCEEDED (hr))
    {
        ULONG estimatedMilliSecondsForQuickFormat = MILLISECONDS_FROM_SECONDS(30);
        ULONG estimatedMilliSecondsForFullFormat  = MILLISECONDS_FROM_SECONDS(15*60);

        switch (mediaType)
        {
            case IMAPI_MEDIA_TYPE_CDROM:
            case IMAPI_MEDIA_TYPE_CDR:
            case IMAPI_MEDIA_TYPE_CDRW:
            {
                //use initialized value.
                break;
            }

            case IMAPI_MEDIA_TYPE_DVDROM:
            case IMAPI_MEDIA_TYPE_DVDPLUSR:
            case IMAPI_MEDIA_TYPE_DVDPLUSRW:
            case IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER:
            case IMAPI_MEDIA_TYPE_DVDDASHR:
            case IMAPI_MEDIA_TYPE_DVDDASHRW:
            case IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER:
            {
                estimatedMilliSecondsForQuickFormat = MILLISECONDS_FROM_SECONDS(90);
                estimatedMilliSecondsForFullFormat  = MILLISECONDS_FROM_SECONDS(15*60);
                break;
            }

            case IMAPI_MEDIA_TYPE_DVDRAM:
            case IMAPI_MEDIA_TYPE_DISK:
            case IMAPI_MEDIA_TYPE_UNKNOWN:
            {
                estimatedMilliSecondsForQuickFormat = MILLISECONDS_FROM_SECONDS(30);
                estimatedMilliSecondsForFullFormat  = MILLISECONDS_FROM_SECONDS(10*60);
                break;
            }
            default:
            {
                hr = E_FAIL;
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (m_FullErase)
            {
                millisecondsToErase = estimatedMilliSecondsForFullFormat;
            }
            else
            {
                millisecondsToErase = estimatedMilliSecondsForQuickFormat;
            }
        }
    }

    // perform the actual erase
    if (SUCCEEDED(hr))
    {
        switch (mediaType)
        {
            case IMAPI_MEDIA_TYPE_DISK:
            case IMAPI_MEDIA_TYPE_DVDRAM:
            case IMAPI_MEDIA_TYPE_DVDPLUSRW:
            {
                hr = EraseByWrite(mediaType);
                break;
            }
            default:
            {
                hr = SendEraseCommand();
                if (SUCCEEDED (hr))
                {
                    hr = WaitForReadDiscInfoToWorkAfterBlank(millisecondsToErase);
                }
                break;
            }
        }
    }

    //Allow the media removal if it's prevented.
    if (mediaLocked)
    {
        // retry this up to five times, as it's a critical user experience
        HRESULT tmpHr = E_FAIL;
        for (ULONG i = 0; FAILED(tmpHr) && (i < 5); i++)
        {
            tmpHr = PreventAllowMediumRemoval(m_RecorderEx, FALSE);
            if (FAILED(tmpHr))
            {
                Sleep(1000);
            }
        }
    }

    //Release exclusive access lock.
    if (exclusiveAccessObtained)
    {
        // retry this up to five times, as it's a critical user experience
        HRESULT tmpHr = E_FAIL;
        for (ULONG i = 0; FAILED(tmpHr) && (i < 5); i++)
        {
            tmpHr = m_Recorder->ReleaseExclusiveAccess();
            if (FAILED(tmpHr))
            {
                Sleep(1000);
            }
        }
    }
    
    return hr;
}

HRESULT CMsftEraseSample::SendEraseCommand()
{
    HRESULT     hr;
    CDB         cdb;
    SENSE_DATA  sense;

    RtlZeroMemory (&sense, sizeof(SENSE_DATA));
    RtlZeroMemory (&cdb, sizeof(CDB));

    cdb.BLANK_MEDIA.OperationCode = SCSIOP_BLANK;
    cdb.BLANK_MEDIA.Immediate = 1;
    
    if (m_FullErase == VARIANT_FALSE)
    {
        cdb.BLANK_MEDIA.BlankType = 1;
    }

    hr = m_RecorderEx->SendCommandNoData ((BYTE*)&cdb,
                                          sizeof(cdb.BLANK_MEDIA),
                                          (BYTE*)&sense,
                                          IMAPI2_DEFAULT_COMMAND_TIMEOUT
                                          );

    if (FAILED(hr) && (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA))
    {
        hr = E_IMAPI_ERASE_DRIVE_FAILED_ERASE_COMMAND;
    }

    return hr;
}

HRESULT CMsftEraseSample::EraseByWrite(IMAPI_MEDIA_PHYSICAL_TYPE mediaType)
{
    HRESULT             hr = S_OK;
    IStream*            firstStream = NULL;
    IStream*            secondStream = NULL;
    IWriteEngine2*      writeEngine = NULL;
    PDISC_INFORMATION   discInfo = NULL;
    ULONG               discInfoSize = 0;
    LONG                totalSectorsToWrite = 0;

    // 0) For DVD+RW media, check if never formatted
    if (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSRW)
    {
        if (SUCCEEDED (hr)) 
        {
            ULONG requiredSize = RTL_SIZEOF_THROUGH_FIELD (DISC_INFORMATION, 
                                                           FirstTrackNumber);
            hr = GetDiscInformation (m_RecorderEx, &discInfo, &discInfoSize, requiredSize);
        }

        if (SUCCEEDED(hr))
        {
            //    0.1) Exit early if media is totally blank and unused for both
            //         quick and full formats.
            if (discInfo->DiscStatus == 0)
            {                
                hr = S_OK;
                goto EraseByWriteExit;
            }
        }
    }

    // 1) Determine the size of the media
    if (SUCCEEDED(hr))
    {
        ULONG bytesPerBlock = 0;
        ULONG userSectors = 0;

        hr = ReadMediaCapacity(m_RecorderEx, &bytesPerBlock, &userSectors);
        totalSectorsToWrite = userSectors;
    }

    // For a quick erase on DVD+RW media we need to write over the first 2MB of the media
    // and the last 2MB of formatted media, not the last 2MB of total media.  
    // Currently do not have a way to determine this so will do two writes over the beginning of the media.
    // In the event there is less than 2mb of formatted media, the drive will format
    // the entire media, which we do not want.
    if (SUCCEEDED(hr))
    {
        if ((mediaType == IMAPI_MEDIA_TYPE_DVDPLUSRW) && (!m_FullErase))
        {
            // This will take us into our pathological case below
            totalSectorsToWrite = DefaultSectorsPerWriteForQuickErase;
        }
    }

    // 2) Determine the size of the first write
    //    2.1) For quick-erase, always 2MB
    //    2.2) For full erase, capacity of media minus 2MB
    if (SUCCEEDED(hr))
    {
        if (totalSectorsToWrite < (2*DefaultSectorsPerWriteForQuickErase))
        {
            // pathological case 
            m_SectorsFirstWrite = totalSectorsToWrite;
            m_SectorsSecondWrite = totalSectorsToWrite;
        }
        else if (m_FullErase)
        {
            m_SectorsFirstWrite = totalSectorsToWrite - DefaultSectorsPerWriteForQuickErase;
            m_SectorsSecondWrite = DefaultSectorsPerWriteForQuickErase;
        }
        else
        {
            m_SectorsFirstWrite = DefaultSectorsPerWriteForQuickErase;
            m_SectorsSecondWrite = DefaultSectorsPerWriteForQuickErase;
        }
    }

    // 3) Create two MsftStreamZero objects
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftStreamZero,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&firstStream)
                              );
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_MsftStreamZero,
                                  NULL, CLSCTX_ALL,
                                  IID_PPV_ARGS(&secondStream)
                                  );
        }
    }
    //    3.1) First one is size of first write
    if (SUCCEEDED(hr))
    {
        ULARGE_INTEGER newSize; 
        RtlZeroMemory(&newSize, sizeof(ULARGE_INTEGER));
        newSize.QuadPart = ((ULONGLONG)2048) * m_SectorsFirstWrite;
        hr = firstStream->SetSize(newSize);
    }
    //    3.2) Second one is always 2MB
    if (SUCCEEDED(hr))
    {
        ULARGE_INTEGER newSize; 
        RtlZeroMemory(&newSize, sizeof(ULARGE_INTEGER));
        newSize.QuadPart = ((ULONGLONG)2048) * m_SectorsSecondWrite;
        hr = secondStream->SetSize(newSize);
    }

    // 4) Setup write engine
    //    4.1) Setup write engine
    {
        // Create a WriteEngine
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_MsftWriteEngine2,
                                  NULL, CLSCTX_ALL,
                                  IID_PPV_ARGS(&writeEngine)
                                  );
        }
        // set the disc recorder and options for the write engine
        if (SUCCEEDED(hr))
        {
            hr = writeEngine->put_Recorder(m_RecorderEx);
        }
        // NOT UseStreamingWrite12
        if (SUCCEEDED(hr))
        {
            hr = writeEngine->put_UseStreamingWrite12(VARIANT_FALSE);
        }
        // StartingBlocksPerSecond
        if (SUCCEEDED(hr))
        {
            hr = writeEngine->put_StartingSectorsPerSecond(75*40); // 40x CD, 4x DVD
        }
        // EndingBlocksPerSecond
        if (SUCCEEDED(hr))
        {
            hr = writeEngine->put_EndingSectorsPerSecond(75*40); // 40x CD, 4x DVD
        }
        // BytesPerBlock
        if (SUCCEEDED(hr))
        {
            hr = writeEngine->put_BytesPerSector(2048);
        }
    }
    //    4.2) initialize event information
    // connect the events for the write engine
    if (SUCCEEDED(hr))
    {
        hr = EraseWriteEngineEventSimpleImpl::DispEventAdvise(writeEngine, &IID_DWriteEngine2Events);
    }

    //    4.3) Create the required timekeeping objects
    if (SUCCEEDED(hr))
    {
        ULONG expectedMilliseconds = MILLISECONDS_FROM_SECONDS(15);
        m_TimeKeeperWrite = new CTaskTimeEstimator(expectedMilliseconds, m_SectorsFirstWrite + m_SectorsSecondWrite);
        if (m_TimeKeeperWrite == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Set the drive's OPC
    if (SUCCEEDED(hr))
    {
        hr = RequestAutomaticOPC(m_RecorderEx);
        if (FAILED(hr))
        {
            hr = S_OK;
        }
    }

    // 5) Write the data (zeros)
    if (SUCCEEDED(hr))
    {
        m_TimeKeeperWrite->StartNow();
    }

    //    5.1) Write the first section
    // Ask write engine to write  (/dev/zero stream)
    if (SUCCEEDED(hr))
    {
        m_WritingFirstSection = TRUE;

        // start LBA == 0
        hr = writeEngine->WriteSection(firstStream, 0, m_SectorsFirstWrite);
    }

    //    5.2) Write the second section
    // Ask write engine to write  (/dev/zero stream)
    if (SUCCEEDED(hr))
    {
        m_WritingFirstSection = FALSE;
        // start LBA == totalSectorsToWrite - m_SectorsSecondWrite; 

        hr = writeEngine->WriteSection(secondStream, totalSectorsToWrite - m_SectorsSecondWrite, m_SectorsSecondWrite);
    }
    m_TimeKeeperWrite->EndNow();

    // 6) Finalize the media as required (per-media routines)
    if (SUCCEEDED(hr))
    {
        hr = SendSynchronizeCacheCommand(m_RecorderEx, SYNCHRONIZE_CACHE_TIMEOUT, FALSE);
    }


EraseByWriteExit:
    // CLEANUP FOLLOWS ///////////////////////////////
    delete m_TimeKeeperWrite; 
    m_TimeKeeperWrite = NULL; // delete of NULL is harmless
    CoTaskMemFreeAndNull(discInfo);
    ReleaseAndNull(firstStream);
    ReleaseAndNull(secondStream);
    ReleaseAndNull(writeEngine);

    return hr;
}

VOID CMsftEraseSample::EraseUpdateCallBack(__in PVOID Object, __in SENSE_DATA* Sense)
{
    if ((Object == NULL) || (Sense == NULL))
    {
        return;
    }

    // The Object == the context
    ERASE_UPDATE_CALLBACK_CONTEXT*  context = (ERASE_UPDATE_CALLBACK_CONTEXT*)Object;
    ULONG                           completed = 0;

    // 1. Determine if the sense data has progress info
    if (Sense->SenseKeySpecific[0] & 0x80)  //sensekey specific field is valid
    {
        completed =
            (Sense->SenseKeySpecific[1] << (8*1)) |
            (Sense->SenseKeySpecific[2] << (8*0)) ;
    }

    // 2. Determine if we're cycling and/or going downward (semi-common bugs)
    if (!context->IsDecreasing)
    {
        // need to check if completed is more than about 1/2 way
        // to avoid false warnings when the bug is that the drive
        // cycles from 0..FFFF multiple times.
        if ((completed < context->LastProgress) && (completed > 0x7000))
        {
            context->IsDecreasing = TRUE;
            // Also, invert the saved last-progress data, since
            // we'll also be inverting the completed value....
            context->LastProgress = 0xFFFF - context->LastProgress;
        }
        // Need to also check for non-zero here to avoid false
        // warnings when a drive reports zero right before succeeding
        // the READ DISC INFO command.
        else if ((completed < context->LastProgress) && (completed != 0))
        {
            context->CycleCount++;
            if (context->CycleCount >= context->ExpectedCycleCount)
            {
                context->ExpectedCycleCount++;
            }
            // Reset the LastProgress to zero.
            context->LastProgress = 0;
        }
        else if ((completed < context->LastProgress) && (completed == 0))
        {
            // just set to fully completed this time, as some drives report
            // zero after progress just before succeeding the RDI command.
            completed = 0xFFFF;
        }
    }

    // 3. Invert the completed value if detecting that cycle goes backwards
    if (context->IsDecreasing)
    {
        completed = 0xFFFF - completed;
    }

    // 4. Update percentage completed for cycle count
    {
        // Use the expected cycle count to temper the number of steps
        // which have been completed.  In common case, this results
        // in multiplication and division by one (no change).
        completed *= (context->CycleCount + 1);
        completed /= context->ExpectedCycleCount;
    }

    // 5. Print the progress info for debugging purposes

    // 6. Update our timekeeping class with progress update
    context->BlankTime->put_CompletedSteps(completed);

    // 8. Fire the event
    context->Erase->Fire_Update(MILLISECONDS_TO_SECONDS(context->BlankTime->get_ElapsedMilliseconds()),
                                MILLISECONDS_TO_SECONDS(context->BlankTime->get_TotalMilliseconds())
                                );

    // 9. Return
    return;
}

HRESULT CMsftEraseSample::GetDiscInformation(
                                            IDiscRecorder2Ex*   Recorder,
                                            PDISC_INFORMATION*  DiscInfo,
                                            PULONG              DiscInfoSize,
                                            ULONG               RequiredSize
                                            )
{
    HRESULT hr;

    if (DiscInfo == NULL) 
    {
        return E_POINTER;
    }

    if (DiscInfoSize == NULL) 
    {
        return E_POINTER;
    }

    hr = Recorder->GetDiscInformation ((BYTE**)DiscInfo, DiscInfoSize);

    if (SUCCEEDED (hr)) 
    {
        if (*DiscInfoSize < RequiredSize)
        {
            hr = E_IMAPI_ERASE_DISC_INFORMATION_TOO_SMALL;

            //
            // Freeing memory as the client would not on seeing an error.
            //

            if (*DiscInfo) 
            {
                CoTaskMemFreeAndNull (*DiscInfo);
            }
            *DiscInfoSize = 0;
        }
    }

    return hr;
}

HRESULT CMsftEraseSample::GetDiscCapabilities(
                                __in                                  IDiscRecorder2Ex* recorder,
                                __deref_out_bcount(*CapabilitiesSize) CDVD_CAPABILITIES_PAGE ** Capabilities, 
                                __out                                 ULONG * CapabilitiesSize,
                                                                      ULONG   RequiredSize
                                )
{
    HRESULT hr;

    if (Capabilities == NULL) 
    {
        return E_POINTER;
    }

    if (CapabilitiesSize == NULL) 
    {
        return E_POINTER;
    }

    hr = recorder->GetModePage (
                                IMAPI_MODE_PAGE_TYPE_LEGACY_CAPABILITIES,
                                IMAPI_MODE_PAGE_REQUEST_TYPE_CURRENT_VALUES,
                                (BYTE **)Capabilities,
                                CapabilitiesSize                                  
                                );
    
    if (SUCCEEDED (hr)) 
    {
        if ( *CapabilitiesSize < RequiredSize ) 
        {
            hr = E_IMAPI_ERASE_MODE_PAGE_2A_TOO_SMALL;

            //
            // Freeing memory as the client would not on seeing an error.
            //

            if (*Capabilities) 
            {
                CoTaskMemFreeAndNull (*Capabilities);        
            }

            *CapabilitiesSize = 0;
        }
    }

    return hr;
}

STDMETHODIMP_(VOID) CMsftEraseSample::WriteEngineUpdate(IDispatch* objectDispatch, IDispatch* progressDispatch)
{
    UNREFERENCED_PARAMETER(objectDispatch);   

    HRESULT                 hr = S_OK;
    IWriteEngine2EventArgs* tmpProgress = NULL;
    ULONG                   sectorsWritten = 0;

    // 1. get the IWriteEngine2EventArgs pointer for internal use
    if (SUCCEEDED(hr))
    {
        hr = progressDispatch->QueryInterface(IID_PPV_ARGS(&tmpProgress));
    }

    // 2. get the sectors written from the write engine
    if (SUCCEEDED(hr))
    {
        LONG lastWrittenLba = 0;
        LONG startLba = 0;
        hr = tmpProgress->get_LastWrittenLba(&lastWrittenLba);

        if (SUCCEEDED(hr))
        {
            hr = tmpProgress->get_StartLba(&startLba);
        }

        if (SUCCEEDED(hr))
        {
            sectorsWritten = (ULONG)(lastWrittenLba - startLba);
            // update sectorsWritten if already on the second write
            if (!m_WritingFirstSection)
            {
                sectorsWritten += m_SectorsFirstWrite;
            }
        }
    }

    // 3. Update the timekeeper object with sectors written
    if (SUCCEEDED(hr))
    {
        m_TimeKeeperWrite->put_CompletedSteps(sectorsWritten);
    }

    // 4. fire the events
    if (SUCCEEDED(hr))
    {
        ULONG elapsed = MILLISECONDS_TO_SECONDS(m_TimeKeeperWrite->get_ElapsedMilliseconds());
        ULONG total   = MILLISECONDS_TO_SECONDS(m_TimeKeeperWrite->get_TotalMilliseconds()  );
        Fire_Update(elapsed, total);
    }

    // always release the QI'd progress info
    ReleaseAndNull(tmpProgress);

    return;
}

