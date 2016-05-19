/*--

Copyright (C) Microsoft Corporation, 2006

Main test routintes.

--*/

#include "stdafx.h"
#include "EraseTest.h"
#include "EraseEventTest.h"


VOID PrintHelp(WCHAR * selfName)
{
    printf("Usage:\n"           
           "%S DriveLetter [-fullerase]\n"
           "Example:\n"
           "%S E: -fullerase\n",
           selfName, selfName
           );
    return;
}

BOOLEAN
GetParameters(
    IN  DWORD           Count,
    IN  WCHAR*          Arguments[],
    OUT IDiscRecorder2** Recorder,
    OUT VARIANT_BOOL*   FullErase
    )
{
    BOOLEAN validArgument = TRUE;

    if ((Count < 1) || (Count > 2))
    {
        validArgument = FALSE;
    }

    if (validArgument && (Arguments[0][1] != ':'))
    {
        validArgument = FALSE;
    }

    if (validArgument && (Count == 2))
    {
        if ( _wcsnicmp(Arguments[1], (L"-fullerase"), strlen("-fullerase")) == 0 )
        {
            *FullErase = VARIANT_TRUE;
        }
        else
        {
            validArgument = FALSE;
        }
    }

    //Try to retrieve the recorder.
    if (validArgument)
    {
        HRESULT         hr = S_OK;
        IDiscMaster2*   tmpDiscMaster = NULL;
        IEnumVARIANT*   tmpRecorderEnumerator = NULL;
        IDiscRecorder2* tmpRecorder = NULL;
        VARIANT         tmpUniqueId;
        BOOLEAN         recorderFound = FALSE;

        // create the disc master object
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_MsftDiscMaster2,
                                  NULL, 
                                  CLSCTX_ALL,
                                  IID_PPV_ARGS(&tmpDiscMaster)
                                  );
            if (FAILED(hr))
            {
                printf("Failed to Create DiscMaster. Error: 0x%X.\n", hr);
            }
        }

        // get a new enumerator for the disc recorders
        if (SUCCEEDED(hr))
        {        
            hr = tmpDiscMaster->get__NewEnum(&tmpRecorderEnumerator);
            if (FAILED(hr))
            {
                printf("Failed get__NewEnum. Error: 0x%X.\n", hr);
            }
        }

        // Reset the enumerator to the beginning
        if (SUCCEEDED(hr))
        {
            hr = tmpRecorderEnumerator->Reset();
            if (FAILED(hr))
            {
                printf("Failed to reset enumerator. Error: 0x%X.\n", hr);
            }
        }

        // request just that one recorder's BSTR
        if (SUCCEEDED(hr))
        {
            while (SUCCEEDED(hr) && !recorderFound)
            {
                hr = tmpRecorderEnumerator->Next(1, &tmpUniqueId, NULL);
                if (SUCCEEDED(hr))
                {
                    // Create a new IDiscRecorder2
                    hr = CoCreateInstance(CLSID_MsftDiscRecorder2,
                                          NULL, 
                                          CLSCTX_ALL,
                                          IID_PPV_ARGS(&tmpRecorder)
                                          );
                    if (FAILED(hr))
                    {
                        printf("Failed CoCreateInstance for DiscRecorder. Error: 0x%X.\n", hr);
                    }

                    // Initialize it with the provided BSTR
                    if (SUCCEEDED(hr))
                    {
                        hr = tmpRecorder->InitializeDiscRecorder(tmpUniqueId.bstrVal);
                        if (FAILED(hr))
                        {
                            printf("Failed to init disc recorder. Error: 0x%X.\n", hr);
                        }
                    }

                    //Check if it's the right recorder.
                    if (SUCCEEDED(hr))
                    {
                        SAFEARRAY * mountPoints = NULL;
                        hr = tmpRecorder->get_VolumePathNames(&mountPoints);
                        if (FAILED(hr))
                        {
                            printf("Unable to get mount points. Error: 0x%X.\n", hr);
                        }
                        else if (mountPoints->rgsabound[0].cElements == 0)
                        {
                            printf("\tMount Point: *** NO MOUNT POINTS FOR RECORDER ***\n");
                        }
                        else
                        {
                            VARIANT* tmp = (VARIANT*)(mountPoints->pvData);
                            for (ULONG i = 0; i < mountPoints->rgsabound[0].cElements; i++)
                            {
                                if ( _wcsnicmp(Arguments[0], tmp[i].bstrVal, 2) == 0 )
                                {
                                    recorderFound = TRUE;
                                    break;
                                }
                            }
                        }
                        SafeArrayDestroyDataAndNull(mountPoints);
                    }
                }
            }
        }

        // copy to caller or release recorder
        if (SUCCEEDED(hr) && recorderFound)
        {
            *Recorder = tmpRecorder;
        }
        else
        {
            validArgument = FALSE;
            ReleaseAndNull(tmpRecorder);
        }
        // all other cleanup
        ReleaseAndNull(tmpDiscMaster);
    }

    return validArgument;
}


class CConsoleModule : public ::ATL::CAtlExeModuleT<CConsoleModule>
{
};
CConsoleModule _AtlModule;

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT         hr = S_OK;
    IDiscRecorder2* recorder = NULL;
    VARIANT_BOOL    fullErase = VARIANT_FALSE;

    if ( !GetParameters(argc-1, argv+1, &recorder, &fullErase) )
    {
        PrintHelp(argv[0]);
        hr = E_INVALIDARG;
    }
    else
    {
        hr = TestErase(recorder, fullErase);
    }
    
    return hr;
}

////////////////////////////////////////////////////////////////////
DWORD GetSecondsElapsed(SYSTEMTIME* StartTime, SYSTEMTIME* EndTime)
{
    FILETIME         Start;
    FILETIME         End;
    unsigned __int64 Start64 = 0;
    unsigned __int64 End64 = 0;
    unsigned __int64 Elapsed64 = 0; 
        
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
    //--- Calc elapsed time
    //
    Elapsed64 = End64 - Start64;
    
    //
    //---- Get micro seconds elapsed
    //
    Elapsed64 /= 10;

    //
    //--- Get milly seconds elapsed
    //
    Elapsed64 /= 1000;

    //
    //--- Get Seconds elapsed
    //
    Elapsed64 /= 1000;

    //
    //--- Return the LowDateTime of seconds elapsed
    //--- This will be good enough for ~136 years elapsed
    //
    return(((LPDWORD)(&Elapsed64))[0]);
}

#define SECONDS_IN_A_DAY     ((DWORD)(SECONDS_IN_A_HOUR*24))
#define SECONDS_IN_A_HOUR    ((DWORD)(SECONDS_IN_A_MINUTE*60))
#define SECONDS_IN_A_MINUTE  ((DWORD)(60))
void CalcElapsedTime(SYSTEMTIME* StartTime, SYSTEMTIME* FinishTime, SYSTEMTIME* ElapsedTime)
{
    DWORD SecondsElapsed = 0;

    memset(ElapsedTime,0,sizeof(SYSTEMTIME));

    SecondsElapsed = GetSecondsElapsed(StartTime, FinishTime);

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

HRESULT TestErase(IDiscRecorder2* Recorder, VARIANT_BOOL FullErase)
{
    HRESULT         hr = S_OK;
    IEraseSample*   eraser = NULL;
    IStream *       dataStream = NULL;
    ULONG           sectorsInDataStream = 0;
    BOOLEAN         dualLayerDvdMedia = FALSE;
    SYSTEMTIME      startTime;
    SYSTEMTIME      endTime;
    SYSTEMTIME      elapsedTime;

    ::ATL::CComObject<CTestEraseEvent>* eventSink = NULL;

    // create a EraseSample object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(__uuidof(MsftEraseSample),
                              NULL, 
                              CLSCTX_ALL,
                              IID_PPV_ARGS(&eraser)
                              );
        if (FAILED(hr))
        {
            printf("CoCreateInstance of CLSID_MsftEraseSample failed. Error: 0x%X.\n", hr);
        }
    }

    // Set the recorder as the recorder for erase object
    if (SUCCEEDED(hr))
    {
        hr = eraser->put_Recorder(Recorder);
        if (FAILED(hr))
        {
            printf("Failed put_Recorder(). Error: 0x%X.\n", hr);
        }
    }

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"EraseTest");

        hr = eraser->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for erase!. Error: 0x%X.\n", hr);
        }
        FreeSysStringAndNull(appName);
    }

    // get the current media in the recorder
    if (SUCCEEDED(hr))
    {
        IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
        hr = eraser->get_CurrentPhysicalMediaType(&mediaType);
        if (FAILED(hr))
        {
            printf("FAILED to eraser->get_CurrentPhysicalMediaType. Error: 0x%X.\n", hr);
        }
    }

    //// Set full erase flag
    if (SUCCEEDED(hr))
    {
        hr = eraser->put_FullErase(FullErase);
        if (FAILED(hr))
        {
            printf("Failed put_FullErase(). Error: 0x%X.\n", hr);
        }            
    }

    // release the recorder early...
    ReleaseAndNull(Recorder);
   
    // create event
    if (SUCCEEDED(hr))
    {
        hr = ::ATL::CComObject<CTestEraseEvent>::CreateInstance(&eventSink);
        if (FAILED(hr))
        {
            printf("Failed to create event sink. Error: 0x%X.\n", hr);
        }
        else
        {
            eventSink->AddRef();
        }
    }

    // hookup events
    if (SUCCEEDED(hr))
    {
        hr = eventSink->DispEventAdvise(eraser);
        if (FAILED(hr))
        {
            printf("Failed to hookup event sink. Error: 0x%X.\n", hr);
        }
    }

    // erase the disc
    if (SUCCEEDED(hr))
    {
        GetSystemTime(&startTime);
        hr = eraser->EraseMedia();
        GetSystemTime(&endTime);
        if (FAILED(hr))
        {
            printf("FAILED to erase!. Error: 0x%X.\n", hr);
        }
        else
        {
            printf("\n");

            CalcElapsedTime(&startTime, &endTime, &elapsedTime);
            printf(" - Time to erase: %02d:%02d:%02d\n", elapsedTime.wHour,
                                                         elapsedTime.wMinute,
                                                         elapsedTime.wSecond);
        }
    }
    
    // unhook events
    if (NULL != eventSink)
    {
        eventSink->DispEventUnadvise(eraser);
    }

    ReleaseAndNull(eventSink);
    ReleaseAndNull(eraser);

    if (SUCCEEDED(hr))
    {
        printf("EraseMedia succeeded.\n");
    }
    else
    {
        printf("EraseMedia FAILED. Error: 0x%X.\n", hr);
    }

    return hr;
}
