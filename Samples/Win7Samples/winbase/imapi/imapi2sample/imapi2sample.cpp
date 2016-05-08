/*++

Copyright (c) 2005  Microsoft Corporation

Module Name:

    imapi2sample.c

Abstract:

    A user mode console app that demenstrates the IMAPIv2 interfaces.

Environment:

    User mode only

Revision History:

    01-01-05 : Created

--*/
#include "imapi2sample.h"
#include <ntverp.h>

#define BOOLEAN_STRING(_x) ((_x)?"TRUE":"FALSE")

VOID
PrintHelp(
    WCHAR * selfName
    )
{
    printf("%S: %s\n"
           "Usage:\n"           
           "%S -list\n"
           "%S -write <dir> [-multi] [-close] [-drive <#>] [-boot <file>]\n"
           "%S -audio <dir> [-close] [-drive <#>]\n"
           "%S -raw <dir> [-close] [-drive <#>]\n"
           "%S -image <file>[-close] [-drive <#>] [-bufe | -bufd]\n"
           "%S -erase [-drive <#>]\n"
           "\n"
           "\tlist      -- list the available writers and their index.\n"
           "\terase     -- quick erases the chosen recorder.\n"
           "\tfullerase -- full erases the chosen recorder.\n"
           "\twrite     -- Writes a directory to the disc.\n"
           "\t   <dir>  -- Directory to be written.\n"
           "\t   [-SAO] -- Use Cue Sheet recording.\n"
           "\t   [-close] -- Close disc (not appendable).\n"
           "\t   [-drive <#>] -- choose the given recorder index.\n"
           "\t   [-iso, -udf, -joliet] -- specify the file system to write.\n"
           "\t   [-multi] -- Add an additional session to disc.\n"
           "\t   [-boot <file>] -- Create a boot disk.  File is a boot image.\n"
           "\teject     -- Eject the CD tray\n"
           "\tinject    -- Close the CD tray\n",
           selfName, VER_PRODUCTVERSION_STR,
           selfName,selfName,selfName,selfName,selfName,selfName
           );
    return;
}

BOOLEAN
ParseCommandLine(
    IN DWORD Count,
    IN WCHAR * Arguments[],
    OUT PPROGRAM_OPTIONS Options
    )
{
    BOOLEAN goodEnough = FALSE;
    // initialize with defaults
    RtlZeroMemory( Options, sizeof(PROGRAM_OPTIONS) );

    for ( DWORD i = 0; i < Count; i++ )
    {
        if ( (Arguments[i][0] == '/') || (Arguments[i][0] == '-') )
        {
            BOOLEAN validArgument = FALSE;
            Arguments[i][0] = '-'; // minimize checks below

            //
            // If the first character of the argument is a - or a / then
            // treat it as an option.
            //
            if ( _wcsnicmp(Arguments[i], (L"-write"), strlen("-write")) == 0 )
            {
                Options->Write = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-image"), strlen("-image")) == 0 )
            {
                Options->Image = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-audio"), strlen("-audio")) == 0 )
            {
                Options->Audio = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-raw"), strlen("-raw")) == 0 )
            {
                Options->Raw = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-close"), strlen("-close")) == 0 )
            {
                Options->CloseDisc = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-multi"), strlen("-multi")) == 0 )
            {
                Options->Multi = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-iso"), strlen("-iso")) == 0 )
            {
                Options->Iso = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-joliet"), strlen("-joliet")) == 0 )
            {
                Options->Joliet = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-udf"), strlen("-udf")) == 0 )
            {
                Options->UDF = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-free"), strlen("-free")) == 0 )
            {
                Options->FreeSpace = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-inject"), strlen("-inject")) == 0 )
            {
                Options->Close = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-eject"), strlen("-eject")) == 0 )
            {
                Options->Eject = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-drive"), strlen("-drive")) == 0 )
            {                
                // requires second argument
                if ( i+1 < Count )
                {
                    i++; // advance argument index
                    ULONG tmp = 666;
                    if ( (swscanf_s( Arguments[i], (L"%d"), &tmp ) == 1) &&
                         (tmp != 666) )
                    {
                        // Let's do this zero based for now
                        Options->WriterIndex = tmp;
                        validArgument = TRUE;
                    }
                }

                if ( !validArgument )
                {
                    printf("Need a second argument after drive,"
                           " which is the one-based index to the\n"
                           "writer to use in decimal format.  To"
                           "get a list of available drives and"
                           "their indexes, use \"-list\" option\n"
                           );
                }
            }
            else if ( _wcsnicmp(Arguments[i], (L"-boot"), strlen("-boot")) == 0 )
            {                
                // requires second argument
                if ( i+1 < Count )
                {
                    i++; // advance argument index
                    ULONG tmp = 666;
                    if ( Arguments[i] != NULL )
                    {
                        // Let's do this zero based for now
                        Options->BootFileName = Arguments[i];
                        validArgument = TRUE;
                    }
                }

                if ( !validArgument )
                {
                    printf("Need a second argument after boot,"
                           " which is the boot file the\n"
                           "writer will use\n"
                           );
                }
            }
            else if ( _wcsnicmp(Arguments[i], (L"-vol"), strlen("-vol")) == 0 )
            {                
                // requires second argument
                if ( i+1 < Count )
                {
                    i++; // advance argument index
                    ULONG tmp = 666;
                    if ( Arguments[i] != NULL )
                    {
                        // Let's do this zero based for now
                        Options->VolumeName = Arguments[i];
                        validArgument = TRUE;
                    }
                }

                if ( !validArgument )
                {
                    printf("Need a second argument after vol,"
                           " which is the volume name for\n"
                           "the disc\n"
                           );
                }
            }
            else if ( _wcsnicmp(Arguments[i], (L"-list"), strlen("-list")) == 0 )
            {
                Options->ListWriters = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-erase"), strlen("-erase")) == 0 )
            {
                Options->Erase = TRUE;
                Options->FullErase = FALSE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-fullerase"), strlen("-fullerase")) == 0 )
            {
                Options->Erase = TRUE;
                Options->FullErase = TRUE;
                validArgument = TRUE;
            }
            else if ( _wcsnicmp(Arguments[i], (L"-?"), strlen("-?")) == 0 )
            {
                printf("Requesting help\n");
            }
            else
            {
                printf("Unknown option -- %S\n", Arguments[i]);
            }

            if(!validArgument)
            {
                return FALSE;
            }
        }
        else if ( Options->FileName == NULL )
        {
            //
            // The first non-flag argument is the ISO to write name.
            //

            Options->FileName = Arguments[i];

        }
        else
        {

            //
            // Too many non-flag arguments provided.  This must be an error.
            //

            printf("Error: extra argument %S not expected\n", Arguments[i]);
            return FALSE;
        }
    }

    //
    // Validate the command-line arguments.
    //
    if ( Options->ListWriters )
    {
        // listing the Writers stands alone
        if ( !(Options->Write || Options->Erase ) )
        {
            goodEnough = TRUE;
        }
        else
        {
            printf("Error: Listing writers must be used alone\n");
        }
    }
    else if ( Options->Write )
    {
        // Write allows erase, but not self-test
        // Write requires at least a filename
        if ( Options->FileName != NULL )
        {
            goodEnough = TRUE;
        }
        else
        {
            printf("Error: Write requires directory\n");
        }

        // validate erase options?
    }
    else if ( Options->Image )
    {
        // Write allows erase, but not self-test
        // Write requires at least a filename
        if ( Options->FileName != NULL )
        {
            goodEnough = TRUE;
        }
        else
        {
            printf("Error: Image requires filename\n");
        }

    }
    else if ( Options->Audio )
    {
        // Write allows erase, but not self-test
        // Write requires at least a filename
        if ( Options->FileName != NULL )
        {
            goodEnough = TRUE;
        }
        else
        {
            printf("Error: Audio requires directory\n");
        }

    }
    
    else if ( Options->Raw )
    {
        // Write allows erase, but not self-test
        // Write requires at least a filename
        if ( Options->FileName != NULL )
        {
            goodEnough = TRUE;
        }
        else
        {
            printf("Error: Raw requires directory\n");
        }

    }

    else if ( Options->Erase )
    {
        // validate erase options?
        goodEnough = TRUE;
    }


    // These are not stand alone options.
    //if ( Options->CloseDisc )
    //{
    //    //printf("Option 'DiscOpen' is not yet implemented\n");
    //    goodEnough = TRUE;
    //}
    //if ( Options->Multi )
    //{
    //    goodEnough = TRUE;
    //}

    if ( Options->FreeSpace )
    {
        goodEnough = TRUE;
    }
    if ( Options->Eject )
    {
        goodEnough = TRUE;
    }
    if ( Options->Close )
    {
        goodEnough = TRUE;
    }

    if ( !goodEnough )
    {
        RtlZeroMemory( Options, sizeof(PROGRAM_OPTIONS) );
        return FALSE;
    }

    return TRUE;
}

//*********************************************************************
//* FUNCTION: GetSecondsElapsed
//*          
//* PURPOSE: 
//*********************************************************************
 DWORD
 GetSecondsElapsed(
    SYSTEMTIME * StartTime,
    SYSTEMTIME * EndTime)
    {
    FILETIME Start,End;
    unsigned __int64 Start64=0, End64=0, Elapsed64=0; 
    
    
    //
    //--- Convert System time
    //
    SystemTimeToFileTime(StartTime,&Start);
    SystemTimeToFileTime(EndTime,&End);

     
    //
    //---- Convert start and end file 
    //---- time to 2  64 bit usigned integers
    //
    ((LPDWORD)(&Start64))[1] = Start.dwHighDateTime;
    ((LPDWORD)(&Start64))[0] = Start.dwLowDateTime;
  
    ((LPDWORD)(&End64))[1] = End.dwHighDateTime;
    ((LPDWORD)(&End64))[0] = End.dwLowDateTime;

   
    //
    //--- Calc elpased time
    //
    Elapsed64 = End64 - Start64;
    
    //
    //---- Get micro seconds elpased
    //
    Elapsed64 /= 10;

    //
    //--- Get milly seconds elpased
    //
    Elapsed64 /= 1000;

    //
    //--- Get Secconds elpased
    //
    Elapsed64 /= 1000;

    //
    //--- Return the LowDateTime of seconds elapsed
    //--- This will be good enough for ~136 years elapsed
    //
    return(((LPDWORD)(&Elapsed64))[0]);
    }

//*********************************************************************
//* FUNCTION:CalcElapsedTime
//*          
//* PURPOSE: 
//*********************************************************************
#define SECONDS_IN_A_DAY     ((DWORD)(SECONDS_IN_A_HOUR*24))
#define SECONDS_IN_A_HOUR    ((DWORD)(SECONDS_IN_A_MINUTE*60))
#define SECONDS_IN_A_MINUTE  ((DWORD)(60))
void 
CalcElapsedTime(
   SYSTEMTIME * StartTime,
   SYSTEMTIME * FinishTime,
   SYSTEMTIME * ElapsedTime)
   {
   DWORD SecondsElapsed;
   
   memset(ElapsedTime,0,sizeof(SYSTEMTIME));
   
   SecondsElapsed = GetSecondsElapsed(
         StartTime, FinishTime);


   if(SecondsElapsed >= SECONDS_IN_A_DAY)
      {
      ElapsedTime->wDay = (WORD) (SecondsElapsed / SECONDS_IN_A_DAY);
      SecondsElapsed -= (ElapsedTime->wDay*SECONDS_IN_A_DAY);
      }
   
   
   if(SecondsElapsed >= SECONDS_IN_A_HOUR)
      {
      ElapsedTime->wHour  = (WORD) (SecondsElapsed / SECONDS_IN_A_HOUR);
      SecondsElapsed -= (ElapsedTime->wHour * SECONDS_IN_A_HOUR);
      }
   
   if(SecondsElapsed >= SECONDS_IN_A_MINUTE)
      {
      ElapsedTime->wMinute = (WORD) (SecondsElapsed / SECONDS_IN_A_MINUTE);
      SecondsElapsed -= (ElapsedTime->wMinute * SECONDS_IN_A_MINUTE);
      }
   
   
   ElapsedTime->wSecond = (WORD) SecondsElapsed;

   }


// using a simple array due to consecutive zero-based values in this enum
CHAR * g_MediaTypeStrings[] = {
    "IMAPI_MEDIA_TYPE_UNKNOWN",
    "IMAPI_MEDIA_TYPE_CDROM",
    "IMAPI_MEDIA_TYPE_CDR",
    "IMAPI_MEDIA_TYPE_CDRW",
    "IMAPI_MEDIA_TYPE_DVDROM",
    "IMAPI_MEDIA_TYPE_DVDRAM",
    "IMAPI_MEDIA_TYPE_DVDPLUSR",
    "IMAPI_MEDIA_TYPE_DVDPLUSRW",
    "IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER",
    "IMAPI_MEDIA_TYPE_DVDDASHR",
    "IMAPI_MEDIA_TYPE_DVDDASHRW",
    "IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER",
    "IMAPI_MEDIA_TYPE_DISK",
    "IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER",
    "IMAPI_MEDIA_TYPE_HDDVDROM",
    "IMAPI_MEDIA_TYPE_HDDVDR",
    "IMAPI_MEDIA_TYPE_HDDVDRAM",
    "IMAPI_MEDIA_TYPE_BDROM",
    "IMAPI_MEDIA_TYPE_BDR",
    "IMAPI_MEDIA_TYPE_BDRE",
    "IMAPI_MEDIA_TYPE_MAX"
};
//typedef enum _STREAM_DATA_SOURCE {
//    DevZeroStream = 0,
//    SmallImageStream = 1,
//    EightGigStream = 2,
//} STREAM_DATA_SOURCE, *PSTREAM_DATA_SOURCE;


// Get a disc recorder given a disc index
HRESULT GetDiscRecorder(__in ULONG index, __out IDiscRecorder2 ** recorder)
{
    HRESULT hr = S_OK;
    IDiscMaster2* tmpDiscMaster = NULL;
    BSTR tmpUniqueId;
    IDiscRecorder2* tmpRecorder = NULL;
    
    *recorder = NULL;

    // create the disc master object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscMaster2,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&tmpDiscMaster)
                              );
        if (FAILED(hr))
        {
            printf("Failed CoCreateInstance\n");
            PrintHR(hr);
        }
    }

    // get the unique id string
    if (SUCCEEDED(hr))
    {        
        hr = tmpDiscMaster->get_Item(index, &tmpUniqueId);
        if (FAILED(hr))
        {
            printf("Failed tmpDiscMaster->get_Item\n");
            PrintHR(hr);
        }
    }

    // Create a new IDiscRecorder2
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscRecorder2,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&tmpRecorder)
                              );
        if (FAILED(hr))
        {
            printf("Failed CoCreateInstance\n");
            PrintHR(hr);
        }
    }
    // Initialize it with the provided BSTR
    if (SUCCEEDED(hr))
    {
        hr = tmpRecorder->InitializeDiscRecorder(tmpUniqueId);
        if (FAILED(hr))
        {
            printf("Failed to init disc recorder\n");
            PrintHR(hr);
        }
    }
    // copy to caller or release recorder
    if (SUCCEEDED(hr))
    {
        *recorder = tmpRecorder;
    }
    else
    {
        ReleaseAndNull(tmpRecorder);
    }
    // all other cleanup
    ReleaseAndNull(tmpDiscMaster);
    FreeSysStringAndNull(tmpUniqueId);
    return hr;
}

HRESULT ListAllRecorders()
{
    HRESULT hr = S_OK;
    LONG          index = 0;
    IDiscMaster2* tmpDiscMaster = NULL;

            // create a disc master object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscMaster2,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&tmpDiscMaster)
                              );
        if (FAILED(hr))
        {
            printf("Failed CoCreateInstance\n");
            PrintHR(hr);
        }
    }
    // Get number of recorders
    if (SUCCEEDED(hr))
    {
        hr = tmpDiscMaster->get_Count(&index);

        if (FAILED(hr))
        {
            printf("Failed to get count\n");
            PrintHR(hr);
        }
    }

    // Print each recorder's ID
    for (LONG i = 0; i < index; i++)
    {
        IDiscRecorder2* discRecorder = NULL;

        hr = GetDiscRecorder(i, &discRecorder);

        if (SUCCEEDED(hr))
        {
            
            BSTR discId;
            BSTR venId;
  
            // Get the device strings
            if (SUCCEEDED(hr)) { hr = discRecorder->get_VendorId(&venId); }
            if (SUCCEEDED(hr)) { hr = discRecorder->get_ProductId(&discId); }
            if (FAILED(hr))
            {
                printf("Failed to get ID's\n");
                PrintHR(hr);
            }                
        
            if (SUCCEEDED(hr))
            {
                printf("Recorder %d: %ws %ws", i, venId, discId); 
            }
            // Get the mount point
            if (SUCCEEDED(hr))
            {
                SAFEARRAY * mountPoints = NULL;
                hr = discRecorder->get_VolumePathNames(&mountPoints);
                if (FAILED(hr))
                {
                    printf("Unable to get mount points, failed\n");
                    PrintHR(hr);
                }
                else if (mountPoints->rgsabound[0].cElements == 0)
                {
                    printf(" (*** NO MOUNT POINTS ***)");
                }
                else
                {
                    VARIANT* tmp = (VARIANT*)(mountPoints->pvData);
                    printf(" (");
                    for (ULONG j = 0; j < mountPoints->rgsabound[0].cElements; j++)
                    {
                        printf(" %ws ", tmp[j].bstrVal);
                    }
                    printf(")");
                }
                SafeArrayDestroyDataAndNull(mountPoints);
            }
            // Get the media type
            if (SUCCEEDED(hr))
            {
                IDiscFormat2Data* dataWriter = NULL;

                // create a DiscFormat2Data object
                if (SUCCEEDED(hr))
                {
                    hr = CoCreateInstance(CLSID_MsftDiscFormat2Data,
                                        NULL, CLSCTX_ALL,
                                        IID_PPV_ARGS(&dataWriter)
                                        );
                    if (FAILED(hr))
                    {
                        printf("Failed CoCreateInstance on dataWriter\n");
                        PrintHR(hr);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = dataWriter->put_Recorder(discRecorder);
                    if (FAILED(hr))
                    {
                        printf("Failed dataWriter->put_Recorder\n");
                        PrintHR(hr);
                    }
                }
                // get the current media in the recorder
                if (SUCCEEDED(hr))
                {
                    IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
                    hr = dataWriter->get_CurrentPhysicalMediaType(&mediaType);
                    if (SUCCEEDED(hr))
                    {            
                        printf(" (%s)", g_MediaTypeStrings[mediaType]);
                    }
                }
                ReleaseAndNull(dataWriter);
            }

            printf("\n");
            FreeSysStringAndNull(venId);
            FreeSysStringAndNull(discId);
 
        }
        else
        {            
            printf("Failed to get drive %d\n", i);
        }

        ReleaseAndNull(discRecorder);

    }

    return hr;
}


HRESULT GetIsoStreamForDataWriting(__out IStream** result, __out ULONG* sectors2, WCHAR * shortStreamFilename)
{
    HRESULT hr = S_OK;
    *result = NULL;
    *sectors2 = 0;
    
    IStream* data = NULL;
    ULONG tmpSectors = 0;

    {
        STATSTG stat; RtlZeroMemory(&stat, sizeof(STATSTG));
        // open an ISO image for the stream
        if (SUCCEEDED(hr))
        {
            hr = SHCreateStreamOnFileW(shortStreamFilename,
                                        STGM_READ | STGM_SHARE_DENY_WRITE,
                                        &data
                                        );
            if (FAILED(hr))
            {
                printf("Failed to open file %S\n",
                            shortStreamFilename);
                PrintHR(hr);
            }
        }
        // validate size and save # of blocks for use later
        if (SUCCEEDED(hr))
        {
            hr = data->Stat(&stat, STATFLAG_NONAME);
            if (FAILED(hr))
            {
                printf("Failed to get stats for file\n");
                PrintHR(hr);
            }
        }
        // validate size and save # of blocks for use later
        if (SUCCEEDED(hr))
        {
            if (stat.cbSize.QuadPart % 2048 != 0)
            {
                printf("File is not multiple of 2048 bytes.  File size is %I64d (%I64x)\n",
                            stat.cbSize.QuadPart, stat.cbSize.QuadPart);
                hr = E_FAIL;
            }
            else if (stat.cbSize.QuadPart / 2048 > 0x7FFFFFFF)
            {
                printf("File is too large, # of sectors won't fit a LONG.  File size is %I64d (%I64x)\n",
                            stat.cbSize.QuadPart, stat.cbSize.QuadPart);
                hr = E_FAIL;
            }
            else
            {
                tmpSectors = (ULONG)(stat.cbSize.QuadPart / 2048);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *result = data;
        *sectors2 = tmpSectors;
    }
    else
    {
        ReleaseAndNull(data);
    }
    return hr;
}





HRESULT ImageWriter(PROGRAM_OPTIONS options)
{
    HRESULT hr = S_OK;
    IDiscRecorder2* recorder = NULL;
    IDiscFormat2Data* dataWriter = NULL;
    IStream * dataStream = NULL;
    ULONG sectorsInDataStream = 0;
    BOOLEAN dualLayerDvdMedia = FALSE;
    ULONG index = options.WriterIndex;
    CComObject<CTestDataWriter2Event>* eventSink = NULL;

    // create a DiscFormat2Data object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscFormat2Data,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&dataWriter)
                              );
        if (FAILED(hr))
        {
            printf("CoCreateInstance failed\n");
            PrintHR(hr);
        }
    }
    // create a DiscRecorder object
    if (SUCCEEDED(hr))
    {
        hr = GetDiscRecorder(index, &recorder);
        if (FAILED(hr))
        {
            printf("CoCreateInstance failed\n");
            PrintHR(hr);
        }
    }

    // Set the recorder as the recorder for the data writer
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->put_Recorder(recorder);
        if (FAILED(hr))
        {
            printf("Failed put_Recorder()\n");
            PrintHR(hr);
        }
    }
    // release the recorder early...
    ReleaseAndNull(recorder);

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"Imapi2Sample");

        hr = dataWriter->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for ISO write!\n");
            PrintHR(hr);
        }
        FreeSysStringAndNull(appName);
    }
   
    // verify the Current media write speed property gets
    //if (SUCCEEDED(hr))
    //{
    //    LONG value = 0;
    //    hr = dataWriter->get_CurrentMediaWriteSpeed(&value);
    //    if (FAILED(hr))
    //    {
    //        DoTrace((TRACE_LEVEL_INFORMATION, DebugGeneral,
    //                 "Failed get_CurrentMediaWriteSpeed() %!HRESULT!",
    //                 hr
    //                 ));
    //       printf("Ignoring failed get_CurrentWriteSpeed() for now\n");
    //        hr = S_OK;
    //    }
    //}
 
 
    // verify the SupportedMediaTypes property gets
    if (SUCCEEDED(hr))
    {
        SAFEARRAY * value = NULL;
        hr = dataWriter->get_SupportedMediaTypes(&value);
        if (FAILED(hr))
        {
            printf("Failed get_SupportedMediaTypes()\n");
            PrintHR(hr);
        }
        SafeArrayDestroyDataAndNull(value);
    }
  
    // get a stream to write to the disc
    if (SUCCEEDED(hr))
    {
        hr = GetIsoStreamForDataWriting(&dataStream, &sectorsInDataStream, options.FileName);
        if (FAILED(hr))
        {
            printf("Failed to create data stream for writing\n");
            PrintHR(hr);
        }
    }

    // Create the event sink
    if (SUCCEEDED(hr))
    {
        hr = CComObject<CTestDataWriter2Event>::CreateInstance(&eventSink);
        if (FAILED(hr))
        {
            printf("Failed to create event sink\n");
            PrintHR(hr);
        }
    }

    // need to keep a reference to this eventSink
    if (SUCCEEDED(hr))
    {
        hr = eventSink->AddRef();
        if (FAILED(hr))
        {
            printf("FAILED to addref\n");
            PrintHR(hr);
        }
    }

    // Hookup the event sink
    if (SUCCEEDED(hr))
    {
        hr = eventSink->DispEventAdvise(dataWriter);
        if (FAILED(hr))
        {
            printf("Failed to hookup event sink\n");
            PrintHR(hr);
        }
    }

    // write the stream
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->Write(dataStream);
        if (FAILED(hr))
        {
            printf("Failed to write stream\n");
            PrintHR(hr);
        }
    }

    // unhook events
        // unhook events
    if (NULL != eventSink)
    {
        eventSink->DispEventUnadvise(dataWriter);
    }

    // verify that clearing the disc recorder works
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->put_Recorder(NULL);
        if (FAILED(hr))
        {
            printf("Failed put_Recorder(NULL)\n");
            PrintHR(hr);
        }
    }

    ReleaseAndNull(eventSink);
    ReleaseAndNull(dataWriter);
    ReleaseAndNull(dataStream);

    if (SUCCEEDED(hr))
    {
        printf("ImageWriter succeeded for drive index %d\n",
               index
               );
    }
    else
    {
        printf("ImageWriter FAILED for drive index %d\n", index);
        PrintHR(hr);
    }
    return hr;
}



HRESULT EjectClose(PROGRAM_OPTIONS options, BOOLEAN close)
{
    HRESULT hr = S_OK;
    IDiscRecorder2* recorder = NULL;
    ULONG index = options.WriterIndex;
    BOOL disableMCN = FALSE;

    // create a DiscRecorder object
    if (SUCCEEDED(hr))
    {
        hr = GetDiscRecorder(index, &recorder);
        if (FAILED(hr))
        {
            printf("FAILED to get disc recorder for eject/close\n");
            PrintHR(hr);
        }
    }

    // Try and prevent shell pop-ups
    if (SUCCEEDED(hr))
    {
        hr = recorder->DisableMcn();
        if (FAILED(hr))
        {
            printf("FAILED to enable MCN after Eject/Close\n");
            PrintHR(hr);
        }
        else
        {
            // Will use this flag instead of HR, in case there is a failure later on
            // we still want to enable MCN
            disableMCN = TRUE;
        }
    }

    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL canLoad = VARIANT_FALSE;
        hr = recorder->get_DeviceCanLoadMedia(&canLoad);
        if (FAILED(hr))
        {
            printf("FAILED recorder->get_DeviceCanLoadMedia\n");
            PrintHR(hr);
        }
        if (canLoad && close)
        {
            hr = recorder->CloseTray();
            if (FAILED(hr))
            {
                printf("FAILED recorder->CloseTray()\n");
                PrintHR(hr);
            }
        }
        else if (canLoad)
        {
            hr = recorder->EjectMedia();
            if (FAILED(hr))
            {
                printf("FAILED recorder->EjectMedia()\n");
                PrintHR(hr);
            }
        }
    }

    // re-enable MCN
    if (disableMCN)
    {
        hr = recorder->EnableMcn();
        if (FAILED(hr))
        {
            printf("FAILED to enable MCN after Eject/Close\n");
            PrintHR(hr);
        }
    }
 
    ReleaseAndNull(recorder);

    if (SUCCEEDED(hr))
    {

    }
    else
    {    
        printf("EjectClose FAILED for drive index %d\n",
               index
               );
        PrintHR(hr);
    }
    return hr;
}

// Write wav files to CD
HRESULT AudioWriter(PROGRAM_OPTIONS options)
{
    HRESULT hr = S_OK;
    IDiscRecorder2* recorder = NULL;
    IDiscFormat2TrackAtOnce * audioWriter = NULL;
    CComObject<CAudioEvent>* eventSink = NULL;

    BOOLEAN bReturn = TRUE;
    ULONG index = options.WriterIndex;

    BSTR dir = ::SysAllocString( options.FileName );

    // create a IDiscFormat2TrackAtOnce object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscFormat2TrackAtOnce,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&audioWriter)
                              );
        if (FAILED(hr))
        {
            printf("Failed CoCreateInstance on dataWriter\n");
            PrintHR(hr);
        }
    }

    // create a DiscRecorder object
    if (SUCCEEDED(hr))
    {
        hr = GetDiscRecorder(index, &recorder);
        if (FAILED(hr))
        {
            printf("Failed GetDiscRecorder\n");
            PrintHR(hr);
        }
    }

    // Set the recorder as the recorder for the audio writer
    if (SUCCEEDED(hr))
    {
        hr = audioWriter->put_Recorder(recorder);
        if (FAILED(hr))
        {
            printf("Failed audioWriter->put_Recorder\n");
            PrintHR(hr);
        }
    }

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"Imapi2Sample");

        hr = audioWriter->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for Audio!\n");
            PrintHR(hr);
        }
        FreeSysStringAndNull(appName);
    } 

    // get the current media in the recorder
    if (SUCCEEDED(hr))
    {
        IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
        hr = audioWriter->get_CurrentPhysicalMediaType(&mediaType);
        if (FAILED(hr))
        {
            printf("FAILED audioWriter->get_CurrentPhysicalMediaType\n");
            PrintHR(hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL b;
        hr = audioWriter->IsCurrentMediaSupported(recorder, &b);
        if (FAILED(hr))
        {
            hr = S_OK;
        }
    }

    // NOW PREPARE THE MEDIA, it will be ready to use
    if (SUCCEEDED(hr))
    {
        hr = audioWriter->PrepareMedia();
        if (FAILED(hr))
        {
            printf("Failed audioWriter->PrepareMedia()\n");
            PrintHR(hr);
        }
    }

    //if (SUCCEEDED(hr) && !options.Close)
    //{
    //    hr = audioWriter->put_DoNotFinalizeDisc(VARIANT_FALSE);
    //    if (FAILED(hr))
    //    {
    //        printf("FAILED audioWriter->get_NumberOfExistingTracks\n");
    //        PrintHR(hr);
    //    }
    //}

    // hookup events
    // create an event object for the write engine
    if (SUCCEEDED(hr))
    {
        hr = CComObject<CAudioEvent>::CreateInstance(&eventSink);
        if (FAILED(hr))
        {
            printf("Failed to create event sink\n");
            PrintHR(hr);
        }
        else
        {
            eventSink->AddRef();
        }
    }

    // hookup the event
    if (SUCCEEDED(hr))
    {
        hr = eventSink->DispEventAdvise(audioWriter);
        if (FAILED(hr))
        {
            printf("Failed to hookup event sink\n");
            PrintHR(hr);
        }
    }

    // Add a track
    if (SUCCEEDED(hr))
    {
	    WCHAR             AppendPath[MAX_PATH];
        WCHAR             FullPath[MAX_PATH];
	    DWORD             ReturnCode;
//	    DWORD             FileAttributes;
	    HANDLE            Files;
	    WIN32_FIND_DATAW  FileData;
	    memset(&FileData, 0, sizeof(WIN32_FIND_DATA));


    	StringCchPrintfW(AppendPath, (sizeof(AppendPath))/(sizeof(AppendPath[0])), (L"%s\\*"), options.FileName);
	    Files = FindFirstFileW(AppendPath, &FileData);

		if (INVALID_HANDLE_VALUE != Files) {
			//We have the search handle for the first file.
			ReturnCode = 1;
			while (ReturnCode) {
				if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
						//This is a real directory that we should deal with.
						printf("Skipping directory %ws\n", FileData.cFileName);
				} 
                else 
                {
                    // We have a file, let's add it
                    IStream * testStream = NULL;
                    STATSTG stat;
                    
                    StringCchPrintfW(FullPath, (sizeof(FullPath))/(sizeof(FullPath[0])), 
                                                (L"%s\\%s"), options.FileName, FileData.cFileName);
                    printf("Attempting to add %ws\n", FullPath);

                    // get a stream to write to the disc
                    hr = SHCreateStreamOnFileW(FullPath,
                                        STGM_READWRITE,
                                        &testStream
                                        );

                    if (FAILED(hr))
                    {
                        printf("FAILED to get file stream\n");
                        PrintHR(hr);
                    }
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = testStream->Stat(&stat, STATFLAG_DEFAULT);
                        if (FAILED(hr))
                        {
                            printf("FAILED to get testStream->Stat\n");
                            PrintHR(hr);
                        }
                    }

                    // Mod the size so that it is 2352 byte aligned
                    if (SUCCEEDED(hr))
                    {
                        ULARGE_INTEGER newSize;

                        newSize.HighPart = stat.cbSize.HighPart;
                        newSize.LowPart = stat.cbSize.LowPart - (stat.cbSize.LowPart%2352);

                        hr = testStream->SetSize(newSize);
                        if (FAILED(hr))
                        {
                            printf("FAILED testStream->SetSize\n");
                            PrintHR(hr);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        //write the stream
                        printf("adding track now...\n");
                        hr = audioWriter->AddAudioTrack(testStream);
                        if (FAILED(hr))
                        {
                            printf("FAILED audioWriter->AddAudioTrack(testStream)\n");
                            PrintHR(hr);
                        }
                        else
                        {
                            printf("\n");
                        }
                    }

                    ReleaseAndNull(testStream);

				}
				memset(&FileData, 0, sizeof(WIN32_FIND_DATA));
				ReturnCode = FindNextFileW(Files, &FileData);
			}

			if (!ReturnCode) 
            {
				ReturnCode = GetLastError();
				if (ReturnCode != ERROR_NO_MORE_FILES) 
                {
					printf("Error in attempting to get the next file in %s\n.",
							AppendPath);
				} 
                else
                {
					printf("No More Files\n");
				}
			}
			FindClose(Files);
		}
        else
        {
            ReturnCode = GetLastError();
			printf("Could not open a search handle on %ws.\n", options.FileName);
            printf("return = %d\n", ReturnCode);
		}
    }

    // unhook events
    if (NULL != eventSink)
    {
        eventSink->DispEventUnadvise(audioWriter);
    }  

    // Release the media now that we are done
    if (SUCCEEDED(hr))
    {
        hr = audioWriter->ReleaseMedia();
        if (FAILED(hr))
        {
            printf("FAILED audioWriter->ReleaseMedia()\n");
            PrintHR(hr);
        }
    } 

    // Let's clear the recorder also
    if (SUCCEEDED(hr))
    {
        hr = audioWriter->put_Recorder(NULL);
        if (FAILED(hr))
        {
            printf("FAILED audioWriter->put_Recorder(NULL)\n");
            PrintHR(hr);
        }
    }

    ReleaseAndNull(eventSink);
    ReleaseAndNull(audioWriter);
    ReleaseAndNull(recorder);

    if (SUCCEEDED(hr))
    {
        printf("AudioWriter succeeded for drive index %d\n",
               index
               );
    }
    else
    {
        printf("AudioWriter FAILED for drive index %d\n", index);
        PrintHR(hr);
    }
    return hr;
}

// Write audio to disc using disc at once
HRESULT RawWriter(PROGRAM_OPTIONS options)
{
    HRESULT hr = S_OK;

    ::ATL::CComPtr<IRawCDImageCreator> raw;
    ::ATL::CComPtr<IStream> resultStream;
    ATL::CComPtr<IDiscRecorder2> iDiscRecorder;
    ATL::CComPtr<IDiscFormat2RawCD> iDiscFormatRaw;
    ATL::CComObject<CTestRawWriter2Event> *events = NULL;
    ULONG index = options.WriterIndex;
    
    // cocreate all burning classes

    // create a DiscRecorder object
    if (SUCCEEDED(hr))
    {
        hr = GetDiscRecorder(index, &iDiscRecorder);
        if (FAILED(hr))
        {
            printf("Failed GetDiscRecorder\n");
            PrintHR(hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw.CoCreateInstance(CLSID_MsftDiscFormat2RawCD);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw.CoCreateInstance\n");
            PrintHR(hr);
        }
    }

    // attach disc recorder and disc format
    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->put_Recorder(iDiscRecorder);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->put_Recorder\n");
            PrintHR(hr);
        }
    }

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"Imapi2Sample");

        hr = iDiscFormatRaw->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for Raw!\n");
            PrintHR(hr);
        }
        FreeSysStringAndNull(appName);
    } 

    // check if the current recorder and media support burning
    VARIANT_BOOL recorderSupported = VARIANT_FALSE;
    VARIANT_BOOL mediaSupported = VARIANT_FALSE;

    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->IsRecorderSupported(iDiscRecorder, &recorderSupported);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->IsRecorderSupported!\n");
            PrintHR(hr);
        }

        if (recorderSupported != VARIANT_TRUE)
        {
            printf("ERROR: recorder reports it doesn't support burning DAO RAW capabilities!\n");
            hr = E_FAIL;
        }
    } 

    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->IsCurrentMediaSupported(iDiscRecorder, &mediaSupported);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->IsCurrentMediaSupported!\n");
            PrintHR(hr);
        }

        if (mediaSupported != VARIANT_TRUE)
        {
            printf("ERROR: recorder reports the current media doesn't support burning DAO RAW!\n");
            hr = E_FAIL;
        }
    }
    
    // create a raw image creator
    if (SUCCEEDED(hr))
    {
        hr = raw.CoCreateInstance(CLSID_MsftRawCDImageCreator);

        if (FAILED(hr))
        {
            printf("FAILED raw.CoCreateInstance\n");
            PrintHR(hr);
        }
    }
    
    // set the image type
    if (SUCCEEDED(hr))
    {
        hr = raw->put_ResultingImageType(IMAPI_FORMAT2_RAW_CD_SUBCODE_IS_RAW); //IMAPI_FORMAT2_RAW_CD_SUBCODE_PQ_ONLY);

        if (FAILED(hr))
        {
            printf("FAILED raw->put_ResultingImageType\n");
            PrintHR(hr);
        }
    }

    // Add tracks
    if (SUCCEEDED(hr))
    {
        WCHAR             AppendPath[MAX_PATH];
        WCHAR             FullPath[MAX_PATH];
        DWORD             ReturnCode;
        HANDLE            Files;
        WIN32_FIND_DATAW  FileData;
        memset(&FileData, 0, sizeof(WIN32_FIND_DATA));
        LONG index = 0; 


    	StringCchPrintfW(AppendPath, (sizeof(AppendPath))/(sizeof(AppendPath[0])), (L"%s\\*"), options.FileName);
	    Files = FindFirstFileW(AppendPath, &FileData);

		if (INVALID_HANDLE_VALUE != Files) {
			//We have the search handle for the first file.
			ReturnCode = 1;
			while (ReturnCode) {
				if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
						//This is a real directory that we should deal with.
						printf("Skipping directory %ws\n", FileData.cFileName);
				} 
                else 
                {
                    // We have a file, let's add it
                    IStream * testStream = NULL;
                    STATSTG stat;
                    
                    StringCchPrintfW(FullPath, (sizeof(FullPath))/(sizeof(FullPath[0])), 
                                                (L"%s\\%s"), options.FileName, FileData.cFileName);
                    printf("Attempting to add %ws\n", FullPath);

                    // get a stream to write to the disc
                    hr = SHCreateStreamOnFileW(FullPath,
                                        STGM_READWRITE,
                                        &testStream
                                        );

                    if (FAILED(hr))
                    {
                        printf("FAILED to get file stream\n");
                        PrintHR(hr);
                    }
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = testStream->Stat(&stat, STATFLAG_DEFAULT);
                        if (FAILED(hr))
                        {
                            printf("FAILED to get testStream->Stat\n");
                            PrintHR(hr);
                        }
                    }

                    // Mod the size so that it is 2352 byte aligned
                    if (SUCCEEDED(hr))
                    {
                        ULARGE_INTEGER newSize;

                        newSize.HighPart = stat.cbSize.HighPart;
                        newSize.LowPart = stat.cbSize.LowPart - (stat.cbSize.LowPart%2352);

                        hr = testStream->SetSize(newSize);
                        if (FAILED(hr))
                        {
                            printf("FAILED testStream->SetSize\n");
                            PrintHR(hr);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        //Add the track to the stream
                        printf("adding track now...\n");
                        hr = raw->AddTrack(IMAPI_CD_SECTOR_AUDIO, testStream, &index);
                        if (FAILED(hr))
                        {
                            printf("FAILED raw->AddTrack(testStream)\n");
                            PrintHR(hr);
                        }
                        else
                        {
                            printf("\n");
                        }
                    }

                    ReleaseAndNull(testStream);

				}
				memset(&FileData, 0, sizeof(WIN32_FIND_DATA));
				ReturnCode = FindNextFileW(Files, &FileData);
			}

			if (!ReturnCode) 
            {
				ReturnCode = GetLastError();
				if (ReturnCode != ERROR_NO_MORE_FILES) 
                {
					printf("Error in attempting to get the next file in %s\n.",
							AppendPath);
				} 
                else
                {
					printf("No More Files\n");
				}
			}
			FindClose(Files);
		}
        else
        {
            ReturnCode = GetLastError();
			printf("Could not open a search handle on %ws.\n", options.FileName);
            printf("return = %d\n", ReturnCode);
		}
    }

    // create the disc image    
    if (SUCCEEDED(hr))
    {
        raw->CreateResultImage(&resultStream);

        if (FAILED(hr))
        {
            printf("FAILED raw->CreateResultImage\n");
            PrintHR(hr);
        }
    }

    // prepare media
    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->PrepareMedia();

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->PrepareMedia\n");
            PrintHR(hr);
        }
    }

    // set up options on disc format
    // NOTE: this will change later to put a different mode when it's fully implemented
    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->put_RequestedSectorType(IMAPI_FORMAT2_RAW_CD_SUBCODE_IS_RAW); //IMAPI_FORMAT2_RAW_CD_SUBCODE_PQ_ONLY);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->put_RequestedSectorType\n");
            PrintHR(hr);
        }
    }

    // connect events
    //if (SUCCEEDED(hr))
    //{
    //    hr = events->DispEventAdvise(iDiscFormatRaw);

    //    if (FAILED(hr))
    //    {
    //        printf("FAILED events->DispEventAdvise\n");
    //        PrintHR(hr);
    //    }
    //}

    // burn stream
    if (SUCCEEDED(hr))
    {
        hr = iDiscFormatRaw->WriteMedia(resultStream);

        if (FAILED(hr))
        {
            printf("FAILED iDiscFormatRaw->WriteMedia\n");
            PrintHR(hr);
        }
    }

    // unadvise events
    //if (events != NULL)
    //{
    //    events->DispEventUnadvise(iDiscFormatRaw);
    //}

    // release media (even if the burn failed)
    hr = iDiscFormatRaw->ReleaseMedia();

    return hr;
}

// Function for writing a dir to disc
HRESULT DataWriter(PROGRAM_OPTIONS options)
{
    HRESULT           hr = S_OK;
    IDiscRecorder2*   discRecorder = NULL;
    IDiscFormat2Data* dataWriter = NULL;
    IStream*          dataStream = NULL;
    BOOLEAN           dualLayerDvdMedia = FALSE;
    ULONG             index = options.WriterIndex;
    VARIANT_BOOL      isBlank = FALSE;
    
    SYSTEMTIME startTime;
    SYSTEMTIME endTime;
    SYSTEMTIME elapsedTime;

    IFileSystemImage *       image = NULL;
    IFileSystemImageResult * fsiresult = NULL;
    IFsiDirectoryItem *      root = NULL;
    BSTR                     dir = ::SysAllocString( options.FileName );
    IBootOptions *           pBootOptions = NULL;
    IStream *                bootStream = NULL;

    CComObject<CTestDataWriter2Event>* eventSink = NULL;

    // create a DiscFormat2Data object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscFormat2Data,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&dataWriter)
                              );
        if (FAILED(hr))
        {
            printf("FAILED CoCreateInstance\n");
            PrintHR(hr);
        }
    }
    // create a DiscRecorder object
    if (SUCCEEDED(hr))
    {
        hr = GetDiscRecorder(index, &discRecorder);
        if (FAILED(hr))
        {
            printf("FAILED GetDiscRecorder\n");
            PrintHR(hr);
        }
    }

    // Set the recorder as the recorder for the data writer
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->put_Recorder(discRecorder);
        if (FAILED(hr))
        {
            printf("FAILED to put_Recorder\n");
            PrintHR(hr);
        }
    }

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"Imapi2Sample");

        hr = dataWriter->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for erase!\n");
            PrintHR(hr);
        }
        FreeSysStringAndNull(appName);
    }

    // verify the SupportedMediaTypes property gets
    if (SUCCEEDED(hr))
    {
        SAFEARRAY * value = NULL;
        hr = dataWriter->get_SupportedMediaTypes(&value);
        if (FAILED(hr))
        {
            printf("FAILED to get_SupportedMediaTypes\n");
            PrintHR(hr);
        }
        SafeArrayDestroyDataAndNull(value);
    }
    
    // Close the disc if specified
    if (SUCCEEDED(hr) && options.CloseDisc)
    {
        printf("Disc will be closed\n");
        hr = dataWriter->put_ForceMediaToBeClosed(VARIANT_TRUE);
        if (FAILED(hr))
        {
            printf("FAILED to put_ForceMediaToBeClosed\n");
            PrintHR(hr);
        }
    }

    // verify the StartAddressOfPreviousSession property 
    // ALSO -- for DVD+R DL, if from sector zero, set to finalize media
    //if (SUCCEEDED(hr))
    //{
    //    LONG value = 0;
    //    hr = dataWriter->get_StartAddressOfPreviousSession(&value);
    //    if (FAILED(hr))
    //    {
    //        printf("FAILED to get_StartAddressOfPreviousSession\n");
    //        PrintHR(hr);
    //    }
    //    else if (value == ((ULONG)-1))
    //    {
    //        hr = dataWriter->put_ForceMediaToBeClosed(VARIANT_TRUE);
    //        if (FAILED(hr))
    //        {
    //            printf("FAILED to put_ForceMediaToBeClosed\n");
    //            PrintHR(hr);
    //        }
    //    }
    //}   
    //   
    // get a stream to write to the disc
    // create a ID_IFileSystemImage object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftFileSystemImage,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&image)
                              );
        if (FAILED(hr))
        {
            printf("Failed CoCreate for filesystem\n");
            PrintHR(hr);
        }
    }

    // Set the filesystems to use if specified
    if (SUCCEEDED(hr) && (options.Iso || options.Joliet || options.UDF))
    {
        FsiFileSystems fileSystem = FsiFileSystemNone;
        if (options.Iso)
        {
            fileSystem = (FsiFileSystems)(fileSystem | FsiFileSystemISO9660);
        }
        if (options.Joliet)
        {
            fileSystem = (FsiFileSystems)(fileSystem | FsiFileSystemJoliet);
            fileSystem = (FsiFileSystems)(fileSystem | FsiFileSystemISO9660);
        }
        if (options.UDF)
        {
            fileSystem = (FsiFileSystems)(fileSystem | FsiFileSystemUDF);
        }

        hr = image->put_FileSystemsToCreate(fileSystem);
        if (FAILED(hr))
        {
            printf("Failed to put PileSystemsToCreate\n");
            PrintHR(hr);
        }
    }

    // Get the root dir
    if (SUCCEEDED(hr))
    {       
        hr = image->get_Root(&root);
        if (FAILED(hr))
        {
            printf("Failed to get root directory\n");
            PrintHR(hr);
        }
    }
    // create the BootImageOptions object
    if (SUCCEEDED(hr) && (NULL != options.BootFileName))
    {
        hr = CoCreateInstance(CLSID_BootOptions,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&pBootOptions)
                              );
        if (FAILED(hr))
        {
            printf("FAILED cocreate bootoptions\n");
            PrintHR(hr);
        }

        if (SUCCEEDED(hr))
        {
            hr = SHCreateStreamOnFileW(options.BootFileName,
                    STGM_READ | STGM_SHARE_DENY_WRITE,
                    &bootStream
                    );

            if (FAILED(hr))
            {
                printf("Failed SHCreateStreamOnFileW for BootImage\n");
                PrintHR(hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pBootOptions->AssignBootImage(bootStream);
            if (FAILED(hr))
            {
                printf("Failed BootImage put_BootImage\n");
                PrintHR(hr);
            }
        }            

        if (SUCCEEDED(hr))
        {
            hr = image->put_BootImageOptions(pBootOptions);
            if (FAILED(hr))
            {
                printf("Failed BootImage put_BootImageOptions\n");
                PrintHR(hr);
            }
        }                        
    }

    // Check if media is blank
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->get_MediaHeuristicallyBlank(&isBlank);
        if (FAILED(hr))
        {
            printf("Failed to get_MediaHeuristicallyBlank\n");
            PrintHR(hr);
        }
    }

    if (SUCCEEDED(hr) && !options.Multi && !isBlank)
    {
        printf("*** WRITING TO NON-BLANK MEDIA WITHOUT IMPORT! ***\n");
    }

        // ImportFileSystem - Import file data from disc
    if (SUCCEEDED(hr) && options.Multi)
    {       
        FsiFileSystems fileSystems;
        SAFEARRAY* multiSession = NULL;

        // Get mutlisession interface to set in image
        if (SUCCEEDED(hr))
        {        
            hr = dataWriter->get_MultisessionInterfaces(&multiSession);

            if (FAILED(hr))
            {
                printf("Failed dataWriter->MultisessionInterfaces\n");
                PrintHR(hr);
            }
        }

        // Set the multisession interface in the image
        if (SUCCEEDED(hr))
        {
            hr = image->put_MultisessionInterfaces(multiSession);

            if (FAILED(hr))
            {
                printf("Failed image->put_MultisessionInterfaces\n");
                PrintHR(hr);
                if (multiSession != NULL)
                {
                    SafeArrayDestroy(multiSession);
                }
            }
            multiSession = NULL;
        }

        if (SUCCEEDED(hr))
        {
            hr = image->ImportFileSystem(&fileSystems);
            if (FAILED(hr))
            {
                if (hr == IMAPI_E_EMPTY_DISC)
                {
                    printf("Empty Disc\n");
                    hr = S_OK;
                }
                else
                {
                    printf("Failed to ImportFileSystem\n");
                    PrintHR(hr);
                }
            }
        }
    }

    // Get free media blocks
    if (SUCCEEDED(hr))
    {
        LONG freeBlocks;

        hr = dataWriter->get_FreeSectorsOnMedia(&freeBlocks);        
        if (FAILED(hr))
        {
            printf("Failed to get Free Media Blocks\n");
            PrintHR(hr);
        }
        else
        {
            hr = image->put_FreeMediaBlocks(freeBlocks);
            if (FAILED(hr))
            {
                printf("Failed to put Free Media Blocks\n");
                PrintHR(hr);
            }
        }
    }
        
    // Add a dir to the image
    if (SUCCEEDED(hr))
    {
        printf("Adding %ws", dir);
        GetSystemTime(&startTime);       // gets current time
        hr = root->AddTree(dir, false);
        GetSystemTime(&endTime);       // gets current time
        if (FAILED(hr))
        {
            printf("\nFailed to add %ws to root dir using AddTree\n", dir);
            PrintHR(hr);
        }
        else
        {
            CalcElapsedTime(&startTime, &endTime, &elapsedTime);
            printf(" - Time: %02d:%02d:%02d\n", elapsedTime.wHour,
                                          elapsedTime.wMinute,
                                          elapsedTime.wSecond);
        }
    }

    // Free our bstr
    FreeSysStringAndNull( dir );

    // Check what file systems are being used
    if (SUCCEEDED(hr))
    {
        FsiFileSystems fileSystems;
        hr = image->get_FileSystemsToCreate(&fileSystems);
        if (FAILED(hr))
        {
            printf("Failed image->get_FileSystemsToCreate\n");
            PrintHR(hr);
        }
        else
        {
            printf("Supported file systems: ");
            if (fileSystems & FsiFileSystemISO9660)
            {
                printf("ISO9660 ");
            }
            if (fileSystems & FsiFileSystemJoliet)
            {
                printf("Joliet ");
            }
            if (fileSystems & FsiFileSystemUDF)
            {
                printf("UDF ");
            }
            printf("\n");
        }
    }

    // Get count
    if (SUCCEEDED(hr))
    {
        LONG numFiles = 0;
        LONG numDirs = 0;
        if (SUCCEEDED(hr)) { hr = image->get_FileCount(&numFiles); }
        if (SUCCEEDED(hr)) { hr = image->get_DirectoryCount(&numDirs); }
        if (FAILED(hr))
        {
            printf("Failed image->get_FileCount\n");
            PrintHR(hr);
        }
        else
        {
            printf("Number of Files: %d\n", numFiles);
            printf("Number of Directories: %d\n", numDirs);
        }
    }
    
    //Set the volume name
    if (SUCCEEDED(hr) && (NULL != options.VolumeName))
    {
        BSTR volName = options.VolumeName;
        hr = image->put_VolumeName(volName);
        if (FAILED(hr))
        {
            printf("Failed setting Volume Name\n");
            PrintHR(hr);
        }

        FreeSysStringAndNull(volName);
    }

    // Create the result image
    if (SUCCEEDED(hr))
    {
        hr = image->CreateResultImage(&fsiresult);
        if (FAILED(hr))
        {
            printf("Failed to get result image, returned %08x\n", hr);
        }
    }

    // Get the stream
    if (SUCCEEDED(hr))
    {
        hr = fsiresult->get_ImageStream(&dataStream);
        if (FAILED(hr))
        {
            printf("Failed to get stream, returned %08x\n", hr);
        }
        else
        {
            printf("Image ready to write\n");
        }
    }

    // Create event sink
    if (SUCCEEDED(hr))
    {
        hr = CComObject<CTestDataWriter2Event>::CreateInstance(&eventSink);
        if (FAILED(hr))
        {
            printf("Failed to create event sink\n");
            PrintHR(hr);
        }
        else
        {
            eventSink->AddRef();
        }
    }

    // hookup event sink
    if (SUCCEEDED(hr))
    {
        hr = eventSink->DispEventAdvise(dataWriter);
        if (FAILED(hr))
        {
            printf("Failed to hookup event sink\n");
            PrintHR(hr);
        }
    }

    // write the stream
    if (SUCCEEDED(hr))
    {        
        GetSystemTime(&startTime);
        hr = dataWriter->Write(dataStream);
        GetSystemTime(&endTime);
        if (FAILED(hr))
        {
            printf("Failed to write stream\n");
            PrintHR(hr);
        }
                else
        {
            CalcElapsedTime(&startTime, &endTime, &elapsedTime);
            printf(" - Time to write: %02d:%02d:%02d\n", elapsedTime.wHour,
                                          elapsedTime.wMinute,
                                          elapsedTime.wSecond);
        }
    }
    // unhook events
    if (NULL != eventSink)
    {
        eventSink->DispEventUnadvise(dataWriter);
    }
    
    // verify the WriteProtectStatus property gets
    if (SUCCEEDED(hr))
    {
        IMAPI_MEDIA_WRITE_PROTECT_STATE value = (IMAPI_MEDIA_WRITE_PROTECT_STATE)0;
        hr = dataWriter->get_WriteProtectStatus(&value);
        if (FAILED(hr))
        {
            printf("Failed get_WriteProtectStatus()\n");
            PrintHR(hr);
        }
    }
    
    // verify that clearing the disc recorder works
    if (SUCCEEDED(hr))
    {
        hr = dataWriter->put_Recorder(NULL);
        if (FAILED(hr))
        {
            printf("Failed put_Recorder(NULL)\n");
            PrintHR(hr);
        }

    }
          
    ReleaseAndNull(eventSink);
    ReleaseAndNull(image);
    ReleaseAndNull(fsiresult);
    ReleaseAndNull(dataWriter);
    ReleaseAndNull(dataStream);
    ReleaseAndNull(discRecorder);
    ReleaseAndNull(pBootOptions);
    ReleaseAndNull(bootStream);

    if (SUCCEEDED(hr))
    {
        printf("DataWriter succeeded for drive index %d\n",
               index
               );
    }
    else
    {    
        printf("DataWriter FAILED for drive index %d\n",
               index
               );
        PrintHR(hr);
    }
    return hr;
}

class CConsoleModule :
    public ::ATL::CAtlExeModuleT<CConsoleModule>
{
};
CConsoleModule _AtlModule;

int __cdecl wmain(int argc, WCHAR *argv[])
{
    HRESULT coInitHr = S_OK;
    HRESULT hr = S_OK;
    PROGRAM_OPTIONS options;

    SYSTEMTIME startTime;
    SYSTEMTIME endTime;
    SYSTEMTIME elapsedTime;

    if ( !ParseCommandLine( argc-1, argv+1, &options ) )
    {
        PrintHelp(argv[0]);
        hr = E_INVALIDARG;
        return hr;
    }
    else
    {
        //PrintOptions(&options);
    }
    
    // Get start time for total time
    GetSystemTime(&startTime);

     if (CAtlBaseModule::m_bInitFailed)
     {
         printf("AtlBaseInit failed...\n");
         coInitHr = E_FAIL;
     }
     else
     {
         // printf("AtlBaseInit passed...\n");
         coInitHr = S_OK;
     }

    if ( SUCCEEDED(coInitHr) && options.ListWriters )
    {
        hr = ListAllRecorders();
    }
    if ( SUCCEEDED(coInitHr) && options.Erase )
    {
        hr = EraseMedia( options.WriterIndex, options.FullErase );
    }
    if ( SUCCEEDED(coInitHr) && options.Write )
    {
        hr = DataWriter(options);
    }
    if ( SUCCEEDED(coInitHr) && options.Image )
    {
        hr = ImageWriter(options);
    }
    if ( SUCCEEDED(coInitHr) && options.Audio )
    {
        hr = AudioWriter(options);
    }
    if ( SUCCEEDED(coInitHr) && options.Raw )
    {
        hr = RawWriter(options);
    }
    if ( SUCCEEDED(coInitHr) && options.Eject )
    {
        hr = EjectClose(options, FALSE);
    }
    if ( SUCCEEDED(coInitHr) && options.Close )
    {
        hr = EjectClose(options, TRUE);
    }

    if (SUCCEEDED(coInitHr))
    {
        CoUninitialize();
    }

    GetSystemTime(&endTime);

    CalcElapsedTime(&startTime, &endTime, &elapsedTime);
    printf(" - Total Time: %02d:%02d:%02d\n", elapsedTime.wHour,
                                    elapsedTime.wMinute,
                                    elapsedTime.wSecond);

    if (SUCCEEDED(hr))
        return 0;
    else
    {
        PrintHR(hr);
        return 1;
    }
}

