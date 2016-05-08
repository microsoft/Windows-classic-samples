/*--

Copyright (C) Microsoft Corporation, 2006

Utility functions

--*/

#include "stdafx.h"

SENSE_INFOMATION AllowedSenseForLongOperations[] = {
    {SCSI_SENSE_NOT_READY,       SCSI_ADSENSE_LUN_NOT_READY, 0xFF, 0}, // not ready
    {SCSI_SENSE_ILLEGAL_REQUEST, 0xFF, 0xFF, 0},
    {SCSI_SENSE_UNIT_ATTENTION,  SCSI_ADSENSE_INSUFFICIENT_TIME_FOR_OPERATION, 0xFF, 0},
};

HRESULT PreventAllowMediumRemoval(__in IDiscRecorder2Ex* recorder, const BOOLEAN lockMedia, const BOOLEAN persistentBit)
{
    HRESULT     hr = S_OK;
    CDB         cdb; 
    SENSE_DATA  sense; 
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&sense, sizeof(SENSE_DATA));

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        cdb.MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb.MEDIA_REMOVAL.Prevent       = (lockMedia     ? 1 : 0);
        cdb.MEDIA_REMOVAL.Persistant    = (persistentBit ? 1 : 0);

        hr = recorder->SendCommandNoData((BYTE*)&cdb,
                                          sizeof(cdb.MEDIA_REMOVAL),
                                          (BYTE*)&sense,
                                          IMAPI2_DEFAULT_COMMAND_TIMEOUT
                                          );
        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            if (!TranslateSenseInfoToHResult(&cdb, &sense, &hr))
            {
                hr = E_FAIL;
            }            
        }
    }

    return hr;
}

HRESULT SendStartStopUnitCommand(IDiscRecorder2Ex* recorder, const START_STOP_OPTION option)
{
    HRESULT hr = S_OK;

    if ((option != StopSpinning ) &&
        (option != StartSpinning) &&
        (option != EjectMedia   ) &&
        (option != LoadMedia    ) )
    {
        hr = E_INVALIDARG;
    }

    // Start or stop the disc
    if (SUCCEEDED(hr))
    {
        CDB cdb;
        SENSE_DATA sense;
        ULONG timeout = IMAPI2_DEFAULT_COMMAND_TIMEOUT;

        RtlZeroMemory(&sense, sizeof(SENSE_DATA));
        RtlZeroMemory(&cdb, sizeof(CDB));

        cdb.START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;

        if (option == StopSpinning)
        {
            cdb.START_STOP.Start = FALSE;
            cdb.START_STOP.LoadEject = FALSE;
        }
        else if (option == EjectMedia)
        {
            cdb.START_STOP.Start = FALSE;
            cdb.START_STOP.LoadEject = TRUE;
        }
        else if (option == StartSpinning)
        {
            cdb.START_STOP.Start = TRUE;
            cdb.START_STOP.LoadEject = FALSE;
            timeout = IMAPI2_DEFAULT_COMMAND_TIMEOUT * 6;
        }
        else if (option == LoadMedia)
        {
            cdb.START_STOP.Start = TRUE;
            cdb.START_STOP.LoadEject = TRUE;
            timeout = IMAPI2_DEFAULT_COMMAND_TIMEOUT * 6;
        }
        hr = recorder->SendCommandNoData((BYTE*)&cdb,
                                         sizeof(cdb.START_STOP),
                                         (BYTE*)&sense,
                                         timeout);
        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            TranslateSenseInfoToHResult(&cdb, &sense, &hr);
        }
    }
    return hr;
}

HRESULT GetPhysicalDvdStructure(__in IDiscRecorder2Ex* recorder, __deref_out_bcount_full(*byteSize) BYTE ** dvdStructureInformation, __out ULONG * byteSize)
{
    HRESULT hr = S_OK;
    BYTE*   tmpDescriptor = NULL;
    ULONG   tmpDescriptorSize = 0;

    // Validate parameters and initialize OUT parameters
    if (dvdStructureInformation == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *dvdStructureInformation = NULL;
    }

    if (byteSize == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *byteSize = 0;
    }

    // Read the DVD structure
    if (SUCCEEDED(hr))
    {
        hr = recorder->ReadDvdStructure(0, 0, 0, 0, &tmpDescriptor, &tmpDescriptorSize);
    }

    if (SUCCEEDED(hr))
    {
        if (tmpDescriptorSize < sizeof(DVD_LAYER_DESCRIPTOR))
        {
            hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
        }
    }

    // save the results
    if (SUCCEEDED(hr))
    {
        *dvdStructureInformation = (BYTE*)tmpDescriptor;
        *byteSize = tmpDescriptorSize;
    }
    else
    {
        CoTaskMemFreeAndNull(tmpDescriptor);
    }

    return hr;
}

HRESULT GetCurrentPhysicalMediaType(__in IDiscRecorder2Ex* recorder, __out IMAPI_MEDIA_PHYSICAL_TYPE * value)
{
    HRESULT                     hr = S_OK;
    IMAPI_MEDIA_PHYSICAL_TYPE   tmpValue = IMAPI_MEDIA_TYPE_UNKNOWN;
    BOOLEAN                     mediaTypeDetermined = FALSE;
    BOOLEAN                     supportsGetConfiguration = TRUE; // avoid legacy checks by default
    BOOLEAN                     readDvdStructureCurrent = FALSE;
    BOOLEAN                     readDvdStructureSupported = FALSE;

    CDiscInformation            discInfo;

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }

    if (value == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *value = IMAPI_MEDIA_TYPE_UNKNOWN;
    }

    // 
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        hr = discInfo.Init(recorder);
    }

    // determine if READ_DVD_STRUCTURE is a supported command
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        BYTE*   dvdRead = NULL;
        ULONG   dvdReadSize = 0;
        HRESULT tmpHr = recorder->GetFeaturePage(IMAPI_FEATURE_PAGE_TYPE_DVD_READ,
                                                 FALSE,
                                                 &dvdRead,
                                                 &dvdReadSize
                                                 );
        if (SUCCEEDED(tmpHr))
        {
            // the feature page is supported
            readDvdStructureSupported = TRUE;

            // check if DvdRead feature is current
            // (Data is guaranteed to be the right size by GetFeaturePage)
            readDvdStructureCurrent = ((PFEATURE_HEADER)dvdRead)->Current ? TRUE : FALSE;
        }
        else if (tmpHr == E_IMAPI_RECORDER_GET_CONFIGURATION_NOT_SUPPORTED)
        {
            hr = S_OK;
        }
        else if ((tmpHr != E_IMAPI_RECORDER_NO_SUCH_FEATURE) &&
                 (tmpHr != E_IMAPI_RECORDER_FEATURE_IS_NOT_CURRENT))
        {
            hr = tmpHr;
        }
        CoTaskMemFreeAndNull(dvdRead);
    }

    // if media is not erasable and is finalized, return a -ROM type
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        // discInfo is either initialized or mediaType has already been determined
        if ((discInfo.get_DiscStatus() == 0x02) && // complete, non-appendable
            !discInfo.get_Erasable())
        {
            if (readDvdStructureCurrent)
            {
                tmpValue = IMAPI_MEDIA_TYPE_DVDROM;
            }
            else
            {
                tmpValue = IMAPI_MEDIA_TYPE_CDROM;
            }
            mediaTypeDetermined = TRUE;
        }
    }

    // check for DVD+RW media
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        FEATURE_HEADER* feature = NULL;
        ULONG           featureSize = 0;
        hr = recorder->GetFeaturePage(IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_RW,
                                      TRUE,
                                      (BYTE**)&feature,
                                      &featureSize
                                      );

        if (hr == E_IMAPI_RECORDER_GET_CONFIGURATION_NOT_SUPPORTED)
        {
            hr = S_OK;
        }
        else if ((hr == E_IMAPI_RECORDER_FEATURE_IS_NOT_CURRENT) ||
                 (hr == E_IMAPI_RECORDER_NO_SUCH_FEATURE))
        {
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSRW;
            mediaTypeDetermined = TRUE;
        }
        CoTaskMemFreeAndNull(feature);
    }   

    // check for DVD+R dual-layer media
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        FEATURE_HEADER* feature = NULL;
        ULONG           featureSize = 0;
        hr = recorder->GetFeaturePage(IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_R_DUAL_LAYER,
                                      TRUE,
                                      (BYTE**)&feature,
                                      &featureSize
                                      );

        if (hr == E_IMAPI_RECORDER_GET_CONFIGURATION_NOT_SUPPORTED)
        {
            hr = S_OK;
        }
        else if ((hr == E_IMAPI_RECORDER_FEATURE_IS_NOT_CURRENT) ||
                 (hr == E_IMAPI_RECORDER_NO_SUCH_FEATURE))
        {
            //Not choosing DVD+R dual layer media due to page DNE or isn't current
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER;
            mediaTypeDetermined = TRUE;
        }
        CoTaskMemFreeAndNull(feature);
    }    

    // check for DVD+R media
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        FEATURE_HEADER* feature = NULL;
        ULONG           featureSize = 0;
        hr = recorder->GetFeaturePage(IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_R,
                                      TRUE,
                                      (BYTE**)&feature,
                                      &featureSize
                                      );

        if (hr == E_IMAPI_RECORDER_GET_CONFIGURATION_NOT_SUPPORTED)
        {
            hr = S_OK;
        }
        else if ((hr == E_IMAPI_RECORDER_FEATURE_IS_NOT_CURRENT) ||
                 (hr == E_IMAPI_RECORDER_NO_SUCH_FEATURE))
        {
            //Not choosing DVD+R media due to page DNE or isn't current\n"
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSR;
            mediaTypeDetermined = TRUE;
        }
        CoTaskMemFreeAndNull(feature);
    }    

    // Use ReadDvdStructure (ignore errors)
    if ((!mediaTypeDetermined) && SUCCEEDED(hr) && readDvdStructureSupported)
    {
        DVD_LAYER_DESCRIPTOR*   descriptor = NULL;
        ULONG                   descriptorSize = 0;
        HRESULT tmpHr = GetPhysicalDvdStructure(recorder, (BYTE**)&descriptor, &descriptorSize);

        if (FAILED(tmpHr))
        {
            // ignore this error, since it's possibly not even DVD media
        }
        else if (descriptor->BookType == 0x0) // DVD-ROM
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDROM;
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0x1) // DVD-RAM
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDRAM;
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0x2) // DVD-R
        {
            if (descriptor->NumberOfLayers == 0x1)  // 0x1 indicates 2 layers on this side
            {
                tmpValue = IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER;
            }
            else
            {
                tmpValue = IMAPI_MEDIA_TYPE_DVDDASHR;
            }    
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0x3) // DVD-RW
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDDASHRW;
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0x9) // DVD+RW
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSRW;
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0xA) // DVD+R
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSR;
            mediaTypeDetermined = TRUE;
        }
        else if (descriptor->BookType == 0xE) // DVD+R Dual Layer
        {
            tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER;
            mediaTypeDetermined = TRUE;
        }
        CoTaskMemFreeAndNull(descriptor); descriptorSize = 0;
    }

    // use profiles to allow CD-R, CD-RW, randomly writable media to be detected
    if ((!mediaTypeDetermined) && SUCCEEDED(hr))
    {
        IMAPI_PROFILE_TYPE* profiles = NULL;
        ULONG               profileCount = 0;
        
        hr = recorder->GetSupportedProfiles(TRUE,
                                            &profiles,
                                            &profileCount
                                            );

        if (hr == E_IMAPI_RECORDER_GET_CONFIGURATION_NOT_SUPPORTED)
        {
            supportsGetConfiguration = FALSE; // allows legacy checks to occur
            hr = S_OK;
        }
        else if ((hr == E_IMAPI_RECORDER_FEATURE_IS_NOT_CURRENT) ||
                 (hr == E_IMAPI_RECORDER_NO_SUCH_FEATURE))
        {
            //GET_CONFIG returned no PROFILE feature -- THIS IS A DRIVE FIRMWARE BUG
            hr = S_OK;
        }
        else if (FAILED(hr))
        {
            //
        }
        else
        {
            // according to the specs, the features shall be listed in order
            // of drive's usage preference.
            for (ULONG  i = 0; (!mediaTypeDetermined) && (i < profileCount); i++)
            {
                switch(profiles[i])
                {
                    case IMAPI_PROFILE_TYPE_NON_REMOVABLE_DISK:
                    case IMAPI_PROFILE_TYPE_REMOVABLE_DISK:
                    //case IMAPI_PROFILE_TYPE_MO_ERASABLE:
                    //case IMAPI_PROFILE_TYPE_MO_WRITE_ONCE:
                    //case IMAPI_PROFILE_TYPE_AS_MO:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DISK;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_CDROM:
                    case IMAPI_PROFILE_TYPE_DDCDROM:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_CDROM;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_CD_RECORDABLE:
                    case IMAPI_PROFILE_TYPE_DDCD_RECORDABLE:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_CDR;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_CD_REWRITABLE:
                    case IMAPI_PROFILE_TYPE_DDCD_REWRITABLE:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_CDRW;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVDROM:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDROM;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVD_RAM:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDRAM;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVD_PLUS_R:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSR;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVD_PLUS_RW:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDPLUSRW;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVD_DASH_RECORDABLE:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDDASHR;
                        break;
                    }
                    case IMAPI_PROFILE_TYPE_DVD_DASH_REWRITABLE:
                    case IMAPI_PROFILE_TYPE_DVD_DASH_RW_SEQUENTIAL:
                    {
                        mediaTypeDetermined = TRUE;
                        tmpValue = IMAPI_MEDIA_TYPE_DVDDASHRW;
                        break;
                    }
                    default:
                        break;
                }
            } // end of loop through all profiles
        } 
        CoTaskMemFreeAndNull(profiles);

    }   

    // For the final, last-ditch attempt for legacy drives
    if ((!mediaTypeDetermined) && (!supportsGetConfiguration) && SUCCEEDED(hr))
    {
        // NOTE: this works because -ROM media was determined earlier
        //      discInfo is either initialized
        // _or_ mediaType has already been determined
        if (discInfo.get_Erasable())
        {
            tmpValue = IMAPI_MEDIA_TYPE_CDRW;
        }
        else
        {
            tmpValue = IMAPI_MEDIA_TYPE_CDR;
        }
    }

    // copy the final result to the caller
    if (SUCCEEDED(hr))
    {
        *value = tmpValue;
    }

    return hr;
}

HRESULT SendSetCDSpeed(__in IDiscRecorder2* discRecorder, const IMAPI_MEDIA_PHYSICAL_TYPE mediaType, const ULONG KBps, const ULONG rotationType)
{
    HRESULT             hr = S_OK;
    IDiscRecorder2Ex*   dr2ex = NULL;
    bool                setStreamingFailed = false;
    ULONG               oldSPS = (ULONG)-1;
    ULONG               oldKBps = (ULONG)-1;
    VARIANT_BOOL        oldRotation = VARIANT_FALSE;

    if (discRecorder == NULL)
    {
        hr = E_POINTER;
    }

    // Get a DR2Ex pointer
    if (SUCCEEDED(hr))
    {
        hr = discRecorder->QueryInterface(IID_PPV_ARGS(&dr2ex));
    }

    if (SUCCEEDED(hr))
    {
        // First, get the current drive properties for comparison later
        hr = UpdateCurrentDriveProperties(discRecorder, mediaType, &oldSPS, &oldKBps, &oldRotation);
    }

    // Allocate the buffer for the SET_STREAMING command
    BYTE* dataBuffer = NULL;
    if (SUCCEEDED(hr))
    {
        dataBuffer = (BYTE*)LocalAlloc(LPTR, 28*sizeof(BYTE));
        if (dataBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }


    // Now, try sending a SET_STREAMING command
    // variables for SET_STREAMING command
    if (SUCCEEDED(hr))
    {
        // clear out data structures
        CDB         cdb;
        SENSE_DATA  sense;
        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(&sense, sizeof(SENSE_DATA));
        RtlZeroMemory(dataBuffer, 28*sizeof(BYTE));

        // create the SET_STREAMING CDB
        cdb.SET_STREAMING.OperationCode = SCSIOP_SET_STREAMING;
        cdb.SET_STREAMING.ParameterListLength[0] = 0;
        cdb.SET_STREAMING.ParameterListLength[1] = 0x1C;

        // now set up the data buffer (performance descriptor)
        {
            LONG endLBA = 0x231260;
            LONG time = 0x3E8;
            dataBuffer[0] = (BYTE) (rotationType & 0x3) << 3;
        
            // now set the END LBA
            dataBuffer[8] = (BYTE)((endLBA & 0xFF000000) >> 24); 
            dataBuffer[9] = (BYTE)((endLBA & 0x00FF0000) >> 16);
            dataBuffer[10] = (BYTE)((endLBA & 0x0000FF00) >> 8);
            dataBuffer[11] = (BYTE)(endLBA & 0x000000FF);

            // now set the read time
            dataBuffer[16] = (BYTE)((time & 0xFF000000) >> 24); 
            dataBuffer[17] = (BYTE)((time & 0x00FF0000) >> 16);
            dataBuffer[18] = (BYTE)((time & 0x0000FF00) >> 8);
            dataBuffer[19] = (BYTE)(time & 0x000000FF);

            // now set the write size
            dataBuffer[20] = (BYTE)((KBps & 0xFF000000) >> 24); 
            dataBuffer[21] = (BYTE)((KBps & 0x00FF0000) >> 16);
            dataBuffer[22] = (BYTE)((KBps & 0x0000FF00) >> 8);
            dataBuffer[23] = (BYTE)(KBps & 0x000000FF);

            // now set the write time
            dataBuffer[24] = (BYTE)((time & 0xFF000000) >> 24); 
            dataBuffer[25] = (BYTE)((time & 0x00FF0000) >> 16);
            dataBuffer[26] = (BYTE)((time & 0x0000FF00) >> 8);
            dataBuffer[27] = (BYTE)(time & 0x000000FF);
        }

        // Send the CBD
        hr = dr2ex->SendCommandSendDataToDevice((BYTE*)&cdb,
                                                (ULONG)12,
                                                (BYTE*)&sense,
                                                IMAPI2_DEFAULT_COMMAND_TIMEOUT,
                                                dataBuffer,
                                                (ULONG)28);

        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            // this usually means the SET_STREAMING did not set the speed properly
            // In practice this could simply mean that the rotation type did not stick
            setStreamingFailed = true;
        }
        else if (SUCCEEDED(hr))
        {
            // check to make sure the new speed stuck
            ULONG        curSPS = (ULONG)-1;
            ULONG        curKBps = (ULONG)-1;
            VARIANT_BOOL rotation;
            hr = UpdateCurrentDriveProperties(discRecorder, mediaType, &curSPS, &curKBps, &rotation);

            if (curKBps != oldKBps)
            {
                // the speed was correctly set
                setStreamingFailed = false;
            }
            else
            {
                // either the speed was not set or the speed was not supported
                // try SET_CD_SPEED
                setStreamingFailed = true;
            }
        }
        else
        {
            // the command itself FAILED
            setStreamingFailed = true;
        }
    }

    if (setStreamingFailed)
    {
        CDB cdb;
        SENSE_DATA sense;

        // clear out data structures
        RtlZeroMemory(&cdb, sizeof(CDB));
        RtlZeroMemory(&sense, sizeof(SENSE_DATA));

        // create the SET_CD_SPEED CDB
        cdb.SET_CD_SPEED.OperationCode = SCSIOP_SET_CD_SPEED;
        cdb.SET_CD_SPEED.RotationControl = rotationType;
        cdb.SET_CD_SPEED.ReadSpeed[0] = (BYTE)((KBps & 0x0000FF00) >> 8);
        cdb.SET_CD_SPEED.ReadSpeed[1] = (BYTE)(KBps & 0x000000FF);
        cdb.SET_CD_SPEED.WriteSpeed[0] = (BYTE)((KBps & 0x0000FF00) >> 8);
        cdb.SET_CD_SPEED.WriteSpeed[1] = (BYTE)(KBps & 0x000000FF);

        // Send the CBD
        hr = dr2ex->SendCommandNoData((BYTE*)&cdb,
                                      12,
                                      (BYTE*)&sense,
                                      IMAPI2_DEFAULT_COMMAND_TIMEOUT);

    }

    LocalFreeAndNull(dataBuffer);
    return hr;
}

BOOLEAN BstrIsValidClientName(__in_xcount(SysStringLen(value)) BSTR value)
{
    ULONG       stringLength = ::SysStringLen(value);
    OLECHAR*    t = value;

    // check the length
    // NOTE: SysStringLen does not include NULL termination
    // A NULL or empty client name is not acceptable.
    if (stringLength >= (CDROM_EXCLUSIVE_CALLER_LENGTH-1) ||
        (stringLength == 0)
        )
    {
        return FALSE;
    }

    // NOTE: stringLength is zero when value is NULL, so this is safe
    for (ULONG i = 0; i < stringLength; i++, t++)
    {
        if (   ((*t >=  (OLECHAR)'0') && (*t <= (OLECHAR)'9'))
            || ((*t >=  (OLECHAR)'a') && (*t <= (OLECHAR)'z'))
            || ((*t >=  (OLECHAR)'A') && (*t <= (OLECHAR)'Z'))
            || (*t == (OLECHAR)' ')
            || (*t == (OLECHAR)'.')
            || (*t == (OLECHAR)',')
            || (*t == (OLECHAR)':')
            || (*t == (OLECHAR)';')
            || (*t == (OLECHAR)'_')
            || (*t == (OLECHAR)'-')
               )
               
        {
            // nothing... this is a valid char
        }
        else
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

// Helper Functions
HRESULT UpdateCurrentDriveProperties(__in IDiscRecorder2* discRecorder, const IMAPI_MEDIA_PHYSICAL_TYPE mediaType, __out ULONG* currentSpeedSectorsPerSecond, __out ULONG* currentSpeedKBps, __out VARIANT_BOOL* currentRotationTypeIsPureCAV)
{
    // two options: GetPerformance->WritePerformance or ModeSense->CDVDCapabilities
    HRESULT             hr = S_OK;
    IDiscRecorder2Ex*   dr2ex = NULL;
    bool                writeSpeedSet = false;

    if ((discRecorder == NULL) ||
        (currentSpeedSectorsPerSecond == NULL) ||
        (currentSpeedKBps == NULL) ||
        (currentRotationTypeIsPureCAV == NULL))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = discRecorder->QueryInterface(IID_PPV_ARGS(&dr2ex));
    }

    if (SUCCEEDED(hr))
    {
        // some new variables that would be used to get the mode page
        CDVD_CAPABILITIES_PAGE *page = NULL;
        ULONG pageSize = 0;
        LONG tmpKBps = 0;

        // retrieve the mode page
        if (SUCCEEDED(hr))
        {
        
            // the drive may not support GET_PERFORMANCE, so just try legacy page
            hr = dr2ex->GetModePage(IMAPI_MODE_PAGE_TYPE_LEGACY_CAPABILITIES,
                                    IMAPI_MODE_PAGE_REQUEST_TYPE_CURRENT_VALUES,
                                    (BYTE**)&page,
                                    &pageSize
                                    );
        }

        // need to check for malicious data (RPC after all...)
        if (SUCCEEDED(hr))
        {
            // ensuring that the size is at least 2 allows checking the page->PageLength field.
            if (pageSize < 2)
            {
                hr = E_IMAPI_UNEXPECTED_RESPONSE_FROM_DEVICE;
            }
            else if (((ULONG)page->PageLength) + 2 != pageSize)
            {
                hr = E_IMAPI_UNEXPECTED_RESPONSE_FROM_DEVICE;
            }
        }

        // parse the results
        if (SUCCEEDED(hr))
        {
            // check length of the mode page
            if (page->PageLength < 20) // MMC1 == pageLength 20, size 22
            {
                // does not conform to any mmc spec
                hr = E_FAIL;
            }
            else if (page->PageLength < 24) // MMC2 == pageLength 24, size 26
            {
                // mmc 1.0 no descriptors, nothing obsolete
                // current write speed is in bytes 20,21
                BYTE *bytePage = (BYTE*)page;

                tmpKBps = ((bytePage[20] << 8) |
                           (bytePage[21]     ) );
            }
            else if (page->PageLength < 28) // MMC3,4 == pageLength 28, size 30
            { 
                // mmc 2.0 ... everything necessary is marked as obsolete
                // however, the SET_CD_SPEED section states that the actual 
                // speed will be returned on this page ... so just use it
                BYTE *bytePage = (BYTE*)page;
                
                tmpKBps = ((bytePage[20] << 8) |
                           (bytePage[21]     ) );
            }
            else if (page->PageLength >= 28) // MMC3,4 and later w/ descriptors
            {
                // mmc 3.0, 4.0 w/descriptors ... currents are not obsolete
                // current write speed is in bytes 28,29
                // current rotation control is in lowest 2 bits of byte 27
                BYTE *bytePage = (BYTE*)page;

                tmpKBps = ((bytePage[28] << 8) |
                           (bytePage[29]     ) );
                
                // set the current rotation type
                if ((bytePage[27] & 0x3) == 0) 
                {
                    *currentRotationTypeIsPureCAV = 0;
                } 
                else if ((bytePage[27] & 0x3) == 1) 
                {
                    *currentRotationTypeIsPureCAV = 1;
                }
                else
                {
                    // RESERVED value
                    *currentRotationTypeIsPureCAV = 0;
                }
            }

            if (SUCCEEDED(hr))
            {
                // set the write speed if necessary
                if (writeSpeedSet == false)
                {
                    *currentSpeedKBps = tmpKBps;
                    hr = FuzzyConvert_KBps2SectorsPerSecond(mediaType, tmpKBps, currentSpeedSectorsPerSecond);
                }
            }
        }

    }

    return hr;
}

HRESULT WaitForReadDiscInfo(__in IDiscRecorder2Ex* recorder, const ULONG secondsToTry, __in_opt PVOID object, __in_opt __callback READ_DISC_INFO_CALLBACK callback)
{
    DECLSPEC_ALIGN(16) DISC_INFORMATION discInfo;
    HRESULT hr = S_OK;
    BOOLEAN readDiscInfoSucceeded = FALSE;
    ULONG   attempts = secondsToTry;

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        // prevent division resulting in zero attempts
        if (attempts == 0) 
        { 
            attempts++; 
        }

        // Loop for a limited time, or until failure or successful Read Disc Info
        for (; SUCCEEDED(hr) && (!readDiscInfoSucceeded) && (attempts > 0); attempts--)
        {
            // Always sleep at least one second
            Sleep(1000);

            CDB         cdb;
            SENSE_DATA  sense;
            ULONG       bytesReceived;
            RtlZeroMemory(&sense, sizeof(SENSE_DATA));
            RtlZeroMemory(&cdb, sizeof(CDB));
            RtlZeroMemory(&discInfo, sizeof(DISC_INFORMATION));

            cdb.READ_DISC_INFORMATION.OperationCode  = SCSIOP_READ_DISC_INFORMATION;
            cdb.READ_DISC_INFORMATION.AllocationLength[0] = (UCHAR)(sizeof(DISC_INFORMATION) >> (8*1));
            cdb.READ_DISC_INFORMATION.AllocationLength[1] = (UCHAR)(sizeof(DISC_INFORMATION) >> (8*0));
            
            hr = recorder->SendCommandGetDataFromDevice((BYTE*)&cdb,
                                                        sizeof(cdb.READ_DISC_INFORMATION),
                                                        (BYTE*)&sense,
                                                        IMAPI2_DEFAULT_COMMAND_TIMEOUT,
                                                        (BYTE*)&discInfo,
                                                        sizeof(DISC_INFORMATION),
                                                        &bytesReceived
                                                        );
            if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
            {
                if (IsSenseDataInTable(AllowedSenseForLongOperations, RTL_NUMBER_OF(AllowedSenseForLongOperations), &sense))
                {
                    if (callback != NULL)
                    {
                        (*callback)(object, &sense);
                    }
                    hr = S_OK;
                }
                else
                {
                    hr = E_FAIL;
                    TranslateSenseInfoToHResult(&cdb, &sense, &hr);
                }
            }
            else if (SUCCEEDED(hr))
            {
                readDiscInfoSucceeded = TRUE;
            }
        }
    }

    return hr;
}

HRESULT ReadMediaCapacity(__in IDiscRecorder2Ex* recorder, __out ULONG* bytesPerBlock, __out ULONG* userSectors)
{
    HRESULT             hr = S_OK;
    READ_CAPACITY_DATA  capacity; 
    CDB                 cdb; 
    SENSE_DATA          sense; 
    ULONG               retrievedBytes = 0;

    RtlZeroMemory(&capacity, sizeof(READ_CAPACITY_DATA));
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&sense, sizeof(SENSE_DATA));
    *bytesPerBlock = 2048;
    *userSectors   = 0;

    if ((recorder == NULL) ||
        (bytesPerBlock == NULL) ||
        (userSectors == NULL))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        cdb.CDB10.OperationCode = SCSIOP_READ_CAPACITY;
        
        hr = recorder->SendCommandGetDataFromDevice((BYTE*)&cdb,
                                                    sizeof(cdb.CDB10),
                                                    (BYTE*)&sense,
                                                    10, // BUGBUG -- different timeout?
                                                    (BYTE*)&capacity,
                                                    sizeof(READ_CAPACITY_DATA),
                                                    &retrievedBytes
                                                    );

        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            if (!TranslateSenseInfoToHResult(&cdb, &sense, &hr))
            {
                hr = E_FAIL;
            }            
        }
        else if (FAILED(hr))
        {
        }
        else if (retrievedBytes < sizeof(READ_CAPACITY_DATA))
        {
            hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
        }
        else
        {
            // swap the byte order
            REVERSE_LONG(&(capacity.LogicalBlockAddress));
            REVERSE_LONG(&(capacity.BytesPerBlock));
            
            // fixup zero-size reporting to be correct
            if ((capacity.LogicalBlockAddress == ((ULONG)-1)) ||
                (capacity.LogicalBlockAddress == 0))
            {
                capacity.LogicalBlockAddress = ((ULONG)-1);
            }
            
            if (CountOfSetBits(capacity.BytesPerBlock) != 1)
            {
                hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
            }
            else
            {
                *bytesPerBlock = capacity.BytesPerBlock;
                *userSectors   = capacity.LogicalBlockAddress + 1;
            }            
        }
    }

    return hr;
}

HRESULT RequestAutomaticOPC(__in IDiscRecorder2Ex* recorder)
{
    HRESULT     hr = S_OK;
    CDB         cdb; 
    SENSE_DATA  sense; 
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&sense, sizeof(SENSE_DATA));

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        cdb.SEND_OPC_INFORMATION.OperationCode = SCSIOP_SEND_OPC_INFORMATION;
        cdb.SEND_OPC_INFORMATION.DoOpc = 1;

        hr = recorder->SendCommandNoData((BYTE*)&cdb,
                                          sizeof(cdb.SEND_OPC_INFORMATION),
                                          (BYTE*)&sense,
                                          DEFAULT_OPC_TIMEOUT
                                          );

        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            if (!TranslateSenseInfoToHResult(&cdb, &sense, &hr))
            {
                hr = E_FAIL;
            }            
        }
    }

    return hr;
}

HRESULT SendSynchronizeCacheCommand(__in IDiscRecorder2Ex* recorder, const ULONG timeout, const BOOLEAN immediate)
{
    HRESULT     hr = S_OK;
    CDB         cdb; 
    SENSE_DATA  sense; 
    RtlZeroMemory(&cdb, sizeof(CDB));
    RtlZeroMemory(&sense, sizeof(SENSE_DATA));

    if (recorder == NULL)
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        cdb.SYNCHRONIZE_CACHE10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;
        cdb.SYNCHRONIZE_CACHE10.Immediate     = (immediate ? 1 : 0);

        hr = recorder->SendCommandNoData((BYTE*)&cdb,
                                          sizeof(cdb.SYNCHRONIZE_CACHE10),
                                          (BYTE*)&sense,
                                          timeout
                                          );
        if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
        {
            if (!TranslateSenseInfoToHResult(&cdb, &sense, &hr))
            {
                hr = E_FAIL;
            }            
        }
    }

    return hr;
}

// Uses the command set to determine if the media is blank with 100% certainty
HRESULT GetMediaPhysicallyBlank(__in IDiscRecorder2Ex* discRecorder, __out VARIANT_BOOL* pPhysicallyBlank)
{
    HRESULT                     hr = S_OK;
    CDiscInformation            discInfo;
    BOOL                        bDiscBlank = TRUE;
    BOOL                        bFound = FALSE;
    IMAPI_MEDIA_PHYSICAL_TYPE   mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;

    if ((discRecorder == NULL) ||
        (pPhysicallyBlank == NULL))
    {
        hr = E_POINTER;
    }

    // get the current media type
    if (SUCCEEDED(hr))
    {
        hr = GetCurrentPhysicalMediaType(discRecorder, &mediaType);
    }

    // shortcut answer for some media types
    if (SUCCEEDED(hr))
    {
        if ((mediaType == IMAPI_MEDIA_TYPE_CDROM) ||
            (mediaType == IMAPI_MEDIA_TYPE_DVDROM))
        {
            bDiscBlank = FALSE;
            bFound = TRUE;
        }
    }

    // Get the DISC_INFO
    if (SUCCEEDED(hr) && !bFound)
    {
        hr = discInfo.Init(discRecorder);
    }

    // Check the DISC_INFO data
    if (SUCCEEDED(hr) && !bFound)
    {
        ULONG discStatus = discInfo.get_DiscStatus();
     
        // check disc status is empty
        if (discStatus != 0)
        {
            bDiscBlank = FALSE;
        }
    }

    // Set the output properly
    if (SUCCEEDED(hr))
    {
        if (bDiscBlank)
        {
            *pPhysicallyBlank = VARIANT_TRUE;
        }
        else
        {
            *pPhysicallyBlank = VARIANT_FALSE;
        }
    }

    return hr;
}

// Uses heuristics to determine if the media is blank
// Mainly used for DVD+RW and DVD-RAM media types
// (in fact, for all other media types, heuristically blank = physically blank)
// Current heuristic checks:
//   "media is heuristically blank if..."
//      . The entire first 2MB of the disc is 0's
HRESULT GetMediaHeuristicallyBlank(__in IDiscRecorder2Ex* discRecorder, __out VARIANT_BOOL* pHeuristicallyBlank)
{
    HRESULT                   hr = S_OK;
    BOOL                      bDiscBlank = TRUE;
    ULONG                     bytesPerSector = 0;
    VARIANT_BOOL              vbPhysicallyBlank = VARIANT_FALSE;
    IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;

    if ((discRecorder == NULL) ||
        (pHeuristicallyBlank == NULL))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetMediaPhysicallyBlank(discRecorder, &vbPhysicallyBlank);
    }

    if (SUCCEEDED(hr))
    {
        hr = GetCurrentPhysicalMediaType(discRecorder, &mediaType);
    }

    // determine how to determine if the media is heuristically blank
    if (SUCCEEDED(hr) && (vbPhysicallyBlank == VARIANT_TRUE))
    {
        // any media that is physically blank is also heuristically blank
        bDiscBlank = TRUE;
    }
    else if ((mediaType == IMAPI_MEDIA_TYPE_CDROM) ||
             (mediaType == IMAPI_MEDIA_TYPE_CDR) ||
             (mediaType == IMAPI_MEDIA_TYPE_CDRW) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDROM) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSR) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHR) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHRW) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER))
    {
        // these media types can deterministically be determined to be blank or not
        bDiscBlank = (vbPhysicallyBlank == VARIANT_FALSE) ? FALSE : TRUE;
    }
    else
    {
        // for DVD+RW, DVD-RAM, DISK, and UNKNOWN media types, proceed with heuristic blank detection
        BOOL bAnswerFound = FALSE;

        // determine how many bytes per sector
        if (SUCCEEDED(hr) && !bAnswerFound)
        {
            ULONG usedSectors = 0;
            hr = ReadMediaCapacity(discRecorder, &bytesPerSector, &usedSectors);
        }

        // now check the first 2MB of the media
        if (SUCCEEDED(hr) && !bAnswerFound)
        {
            ULONG       remainingSectors    = ((2 * 1024 * 1024) / bytesPerSector); // 2MB
            ULONG       sectorsThisRead     = 0;        
            CDB         readCdb;
            SENSE_DATA  senseData;
            ULONG       bufferLength        = 0;
            ULONG       retryCnt            = 0;
            ULONG       startSector         = 0;
            BYTE*       pBuffer             = (BYTE*) LocalAlloc(LPTR, 0x10 * bytesPerSector);

            // check and zero out the buffer
            if (pBuffer == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                RtlZeroMemory(pBuffer, 0x10 * bytesPerSector);
            }

            // perform the read and check
            while ((remainingSectors > 0) && bDiscBlank && SUCCEEDED(hr))
            {
                if (remainingSectors < 0x10)
                {
                    sectorsThisRead = remainingSectors;
                }
                else
                {
                    sectorsThisRead = 0x10;
                }

                do
                {
                    bufferLength = sectorsThisRead * bytesPerSector;

                    RtlZeroMemory( &readCdb, sizeof(CDB) );
                    readCdb.CDB10.OperationCode = SCSIOP_READ;
                    readCdb.CDB10.LogicalBlockByte0 = HIBYTE( HIWORD( startSector));
                    readCdb.CDB10.LogicalBlockByte1 = LOBYTE( HIWORD( startSector));
                    readCdb.CDB10.LogicalBlockByte2 = HIBYTE( LOWORD( startSector));
                    readCdb.CDB10.LogicalBlockByte3 = LOBYTE( LOWORD( startSector));
                    readCdb.CDB10.TransferBlocksMsb = HIBYTE( LOWORD( sectorsThisRead));
                    readCdb.CDB10.TransferBlocksLsb = LOBYTE( LOWORD( sectorsThisRead));

                    hr = discRecorder->SendCommandGetDataFromDevice(
                                                                    (LPBYTE)&readCdb,
                                                                    sizeof(readCdb.CDB10),
                                                                    (LPBYTE)&senseData,
                                                                    12,
                                                                    (LPBYTE)pBuffer,    
                                                                    bufferLength,
                                                                    &bufferLength
                                                                    );
                    if ((hr == S_IMAPI_COMMAND_HAS_SENSE_DATA) ||
                        (FAILED(hr)))

                    {
                        // retry the read blindly for now
                        Sleep(MILLISECONDS_FROM_SECONDS(2));
                        retryCnt++;
                    }
                } while ( (FAILED(hr) || (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)) && (retryCnt < 5) );

                // Translate SENSE_DATA into meaningful hr
                if (hr == S_IMAPI_COMMAND_HAS_SENSE_DATA)
                {
                    TranslateSenseInfoToHResult(&readCdb, &senseData, &hr);
                }

                // now check the data for equality to 0
                if (SUCCEEDED(hr))
                {
                    ULONG   bytesRead       = sectorsThisRead * bytesPerSector;
                    ULONG  *checkData       = (ULONG*)pBuffer;
                    ULONG   bytesChecked    = 0;

                    while ((bytesChecked < bytesRead) && bDiscBlank)
                    {
                        if ((*checkData) != 0)
                        {
                            bDiscBlank = FALSE;
                        }
                        else
                        {
                            bytesChecked += sizeof(ULONG);
                            checkData += 1;
                        }
                    }
                }
                else
                {
                    // if the read still failed, we're going to error out so pretend the disc isn't blank
                    bDiscBlank = FALSE;
                }

                // increment the read if we still think the disc is blank                
                if (SUCCEEDED(hr) && bDiscBlank)
                {
                    remainingSectors -= sectorsThisRead;
                    startSector += sectorsThisRead;
                }
            }

            // cleanup
            LocalFreeAndNull(pBuffer);
        }
    }

    // now set the output properly
    if (SUCCEEDED(hr))
    {
        if (bDiscBlank)
        {
            *pHeuristicallyBlank = VARIANT_TRUE;
        }
        else
        {
            *pHeuristicallyBlank = VARIANT_FALSE;
        }
    }

    return hr;
}

BOOLEAN IsSenseDataInTable(__in_ecount(entries) PSENSE_INFOMATION table, const LONG entries, __in PSENSE_DATA senseData)
{
    LONG    i;
    UCHAR   sense = senseData->SenseKey & 0xf;
    UCHAR   asc   = senseData->AdditionalSenseCode;
    UCHAR   ascq  = senseData->AdditionalSenseCodeQualifier;

    for (i = 0; i < entries; i++ ) 
    {
        if (((table[i].Sense == 0xFF) || (table[i].Sense == sense)) &&
            ((table[i].Ascq  == 0xFF) || (table[i].Ascq  == ascq )) &&
            ((table[i].Asc   == 0xFF) || (table[i].Asc   == asc  ))
            ) 
        {
            return TRUE;
        }
    }
    return FALSE;
}

typedef struct _SENSE_ERROR_TABLE {
    UCHAR   SenseKey;
    UCHAR   Asc;
    UCHAR   AscQ;
    UCHAR   Opcode;
    HRESULT Result;
} SENSE_ERROR_TABLE, *PSENSE_ERROR_TABLE;

// NOTE: Order of this table is important.
//
// Taken linearly until a match is found.
// btw, a value of 0xFF in Sense/ASC/ASCQ/Opcode is a wildcard match.
// Therefore a { 0x00, 0x00, 0xff, 0xff ... } entry will match if both
// the Sense and ASC values are zero, regardless of the ASCQ or opcode.
//

// S_OK is the only allowed SUCCESS table here!
const SENSE_ERROR_TABLE GenericErrors[] = {
    { 0x00, 0x00, 0xFF, 0xFF, S_OK                                       },
    // { 0x08, 0xFF, 0xFF, // Blank check
    { 0xFF, 0x02, 0x06, 0xFF, E_IMAPI_RECORDER_MEDIA_UPSIDE_DOWN         }, // standard-defined error (no reference position found)
    { 0x02, 0x04, 0x01, 0xFF, E_IMAPI_RECORDER_MEDIA_BECOMING_READY      }, // standard-defined error (becoming ready)
    { 0x02, 0x04, 0x03, 0xFF, E_IMAPI_RECORDER_MEDIA_NO_MEDIA            }, // !(from ClassInterpretSenseInfo)
    // { 0x00, 0x04, 0x04, 0xFF, // standard-defined error (background format in progress, need to close disc before eject)
    { 0x02, 0x04, 0x04, 0xFF, E_IMAPI_RECORDER_MEDIA_FORMAT_IN_PROGRESS  }, // standard-defined error (format in progress)
    { 0x02, 0x04, 0x07, 0xFF, E_IMAPI_RECORDER_MEDIA_BUSY                }, // standard-defined error (operation in progress)
    { 0x02, 0x04, 0x08, 0xFF, E_IMAPI_RECORDER_MEDIA_BUSY                }, // standard-defined error (long write in progress)
    { 0x02, 0x04, 0xFF, 0xFF, E_IMAPI_RECORDER_MEDIA_BECOMING_READY      }, // standard-defined error (becoming ready)
    { 0x02, 0x30, 0xFF, 0xAD, E_IMAPI_RECORDER_DVD_STRUCTURE_NOT_PRESENT }, // incompatible medium installed for READ_DVD_STRUCTURE means it's not there (CD-ROM drive)
    { 0x03, 0x0C, 0x09, 0x35, S_OK                                       }, // loss of streaming is OK for a synchronize_cache command
    { 0x03, 0x0C, 0x09, 0xFF, E_IMAPI_LOSS_OF_STREAMING                  }, // loss of streaming in normal case

    { 0x04, 0x09, 0x01, 0xFF, E_IMAPI_RECORDER_MEDIA_UPSIDE_DOWN         }, // !(Plextor PX-W4012A)
    { 0x04, 0x09, 0x02, 0xFF, E_IMAPI_RECORDER_MEDIA_INCOMPATIBLE        }, // Ricoh DVD+RW when inserting -R media
    // { 0x01, 0x17, 0xff, 0xFF, // recovered read/write errors!
    { 0x05, 0x20, 0x00, 0xAD, E_IMAPI_RECORDER_DVD_STRUCTURE_NOT_PRESENT }, // invalid CDB for READ_DVD_STRUCTURE means it's not there (CD-ROM drive)
    // { 0x05, 0x20, 0x00, 0xFF, // invalid CDB
    { 0x05, 0x24, 0x00, 0xAD, E_IMAPI_RECORDER_DVD_STRUCTURE_NOT_PRESENT }, // standard-defined error (READ DVD STRUCTURE, LU + Media combination invalid)
    { 0x05, 0x30, 0x02, 0xAD, E_IMAPI_RECORDER_DVD_STRUCTURE_NOT_PRESENT }, // standard-defined error (READ DVD STRUCTURE for non-dvd media)
    { 0x05, 0x24, 0x00, 0x5A, E_IMAPI_RECORDER_NO_SUCH_MODE_PAGE         }, // invalid field in CDB for MODE_SENSE means unsupported mode page
    // { 0x05, 0x24, 0x00, 0xFF, // invalid field in CDB
    { 0x05, 0x26, 0xff, 0x55, E_IMAPI_RECORDER_INVALID_MODE_PARAMETERS   }, // invalid parameters for MODE_SELECT means unsupported bits in the mode page
    // { 0x05, 0x26, 0xff, 0xFF, // invalid parameters provided
    { 0x07, 0x27, 0xFF, 0xFF, E_IMAPI_RECORDER_MEDIA_WRITE_PROTECTED     }, // standard-defined error
    { 0x06, 0x28, 0x00, 0xFF, E_IMAPI_RECORDER_MEDIA_BECOMING_READY      }, // media may have changed!
    // { 0x06, 0x28, 0xFF, 0xFF, // IMPORT OR EXPORT ELEMENT ACCESSED (6/28/01) -- FORMAT-LAYER MAY HAVE CHANGED (6/28/02)
    // { 0x06, 0x29, 0xFF, 0xFF, // reset occurred!
    // { 0x06, 0x2A, 0xFF, 0xFF, // parameters changed
    // { 0x05, 0x2C, 0x00, 0xFF, // command sequence error == loss of streaming?
    // { 0x06, 0x2E, 0xFF, 0xFF, // insufficient time for operation == timeout
    { 0xFF, 0x30, 0xFF, 0xFF, E_IMAPI_RECORDER_MEDIA_INCOMPATIBLE        }, // standard-defined error (incompatible media), !(may also be media upside down (Sony DRU-500A))
    { 0x02, 0x3A, 0xFF, 0xFF, E_IMAPI_RECORDER_MEDIA_NO_MEDIA            }, // standard-defined error (no media in device)
    // { 0xFF, 0x3E, 0xFF, 0xFF, // timeout?!
    // { 0xFF, 0x57, 0xFF, 0xFF, // (unable to recover TOC), this means the media/drive combination is incompatible, and will cause coasters!
    // { 0x05, 0x64, 0x00, 0xFF, // (illegal mode for this track), cmd-dependent error
    // { 0x05, 0x65, 0x01, 0xFF, // (invalid packet size), write failure
    // { 0x05, 0x6F, 0xFF, 0xFF, // (copy protection failure)
    // { 0xFF, 0x72, 0x00, 0xFF, // (session fixation error), write/close session failure
    // { 0xFF, 0x72, 0x01, 0xFF, // (session fixation error writing lead-in)
    // { 0xFF, 0x72, 0x02, 0xFF, // (session fixation error writing lead-out)
    // { 0xFF, 0x72, 0x03, 0xFF, // (session fixation error - incomplete track in session)
    // { 0xFF, 0x72, 0x04, 0xFF, // (session fixation error - empty/partially written track)
    // { 0xFF, 0x72, 0x05, 0xFF, // (session fixation error - no more rzone reservations allowed)
    // { 0xFF, 0x73, 0x00, 0xFF, // (CD Controller error)
    // { 0xFF, 0x73, 0x01, 0xFF, // (PCA almost full) -- media is going bad, should finalize disc and not use again
    // { 0xFF, 0x73, 0x02, 0xFF, // (PCA full) -- media is bad, should finalize disc and not use again
    { 0xFF, 0x73, 0x03, 0xFF, E_IMAPI_RECORDER_MEDIA_SPEED_MISMATCH     } // (PCA error) -- may be high-speed media in low-speed drive?
    // { 0xFF, 0x73, 0x04, 0xFF, // (PMA/RMA update failure) -- this means temp TOC invalid, must finalize disc to keep data!
    // { 0xFF, 0x73, 0x05, 0xFF, // (PMA/RMA is full) -- this means temp TOC invalid, must finalize disc to keep data!
    // { 0xFF, 0x73, 0x06, 0xFF, // (PMA/RMA is almost full) -- this means temp TOC invalid, must finalize disc to keep data!
    //
};

const BOOLEAN TranslateSenseInfoToHResult(
        __in_bcount(1)                  const CDB* Cdb,           // some sense codes are cdb-specific
        __in_bcount(sizeof(SENSE_DATA)) const SENSE_DATA* Sense,  // sense data is key
        __out                                 HRESULT* HResult   // return the hr
        )
{
    BOOLEAN translated = FALSE;
    // do not modify HResult unless match is made!

    for ( ULONG i = 0; (!translated) && (i < RTL_NUMBER_OF(GenericErrors)); i++ )
    {
        if ( ((GenericErrors[i].SenseKey == 0xFF) ||
              (GenericErrors[i].SenseKey == Sense->SenseKey))
             &&
             ((GenericErrors[i].Asc      == 0xFF) ||
              (GenericErrors[i].Asc      == Sense->AdditionalSenseCode))
             &&
             ((GenericErrors[i].AscQ     == 0xFF) ||
              (GenericErrors[i].AscQ     == Sense->AdditionalSenseCodeQualifier))
             &&
             ((GenericErrors[i].Opcode   == 0xFF) ||
              (GenericErrors[i].Opcode   == Cdb->AsByte[0]))
             )
        {
            *HResult = GenericErrors[i].Result;
            translated = TRUE;
        }
    }

    if (!translated)
    {
        *HResult = E_FAIL;
    }

    return translated;
}

HRESULT FuzzyConvert_KBps2SectorsPerSecond(const IMAPI_MEDIA_PHYSICAL_TYPE mediaType, const ULONG writeSpeedKBps, __out ULONG *writeSpeedSectorsPerSecond)
{
    // Parameter Verification: skipped, internal only
    HRESULT hr = S_OK;
    ULONG finalSpeedSPS = 0;

    if (IS_CD_MEDIA(mediaType))
    {
        // CD media ... tricky
        // cd's are harder because we need to determine which data/sector number to use to set it properly
        // 2324: CD-ROM/XA FORM 2
        // 2048: CD-ROM/XA FORM 1
        // 2352: 
        // 2336: MODE2
        // For now, I standardized on the largest and it seems to work pretty well ...
        finalSpeedSPS = (writeSpeedKBps * 1000) / 2352;
    }
    else if (IS_DVD_MEDIA(mediaType))
    {
        // DVD media ... always the same sector size
        finalSpeedSPS = (writeSpeedKBps * 1000) / 2048;        
    }
    else if (mediaType == IMAPI_MEDIA_TYPE_UNKNOWN)
    {
        // ARBITRARY DECISION: treat reported blank media speeds as CD
        finalSpeedSPS = (writeSpeedKBps * 1000) / 2352;
    }
    else if (mediaType == IMAPI_MEDIA_TYPE_DISK)
    {
        // consider sector sizes on disk media to be 2048
        finalSpeedSPS = (writeSpeedKBps * 1000) / 2048;
    }
    else
    {
        // unsupported media type
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        *writeSpeedSectorsPerSecond = finalSpeedSPS;
    }

    return hr;
}

const HRESULT CreateVariantSafeArrayFromEnums(
        __in_ecount(valueCount)   const LONG * values,
        __in                      const ULONG valueCount,
        __deref_out                     SAFEARRAY** result
        )
{
    HRESULT     hr = S_OK;
    SAFEARRAY*  tmpSafeArray = NULL;
    VARIANT*    tmpVariant = NULL;
    const LONG* tmpItem = values;

    *result = NULL;

    // allocate the safearray
    if (SUCCEEDED(hr))
    {
        tmpSafeArray = ::SafeArrayCreateVector(VT_VARIANT, 0, valueCount);
        if (tmpSafeArray == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            tmpVariant = (VARIANT*)tmpSafeArray->pvData;
        }
    }
    // fill in the array
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 0; i < valueCount; i++, tmpVariant++, tmpItem++)
        {
            ::VariantInit(tmpVariant);
            tmpVariant->vt = VT_I4;
            tmpVariant->lVal = *tmpItem;
        }
    }
    // return the result or destory the array
    if (SUCCEEDED(hr))
    {
        *result = tmpSafeArray;
    }
    else
    {
        // NOTE that this doesn't free any items, which is good.
        SafeArrayDestroyAndNull(tmpSafeArray);
    }

    return hr;
}

