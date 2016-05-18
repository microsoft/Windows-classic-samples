
#include "imapi2sample.h"

HRESULT EraseMedia(ULONG index, BOOL full)
{
    HRESULT hr = S_OK;
    IDiscRecorder2* recorder = NULL;
    IDiscFormat2Erase* eraser = NULL;
    IStream * dataStream = NULL;
    ULONG sectorsInDataStream = 0;
    DWORD cookie = 0xBAADF00D;
    BOOLEAN cookieValid = FALSE;
    BOOLEAN dualLayerDvdMedia = FALSE;
    ::ATL::CComObject<CTestErase2Event> *eventSink = NULL;

    SYSTEMTIME startTime;
    SYSTEMTIME endTime;
    SYSTEMTIME elapsedTime;

    // create a DiscFormat2Erase object
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_MsftDiscFormat2Erase,
                              NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&eraser)
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
        hr = eraser->put_Recorder(recorder);
        if (FAILED(hr))
        {

            printf("Failed put_Recorder()");
            PrintHR(hr);
        }
    }

    // Set the app name for use with exclusive access
    // THIS IS REQUIRED
    if (SUCCEEDED(hr))
    {
        BSTR appName = ::SysAllocString(L"Imapi2Sample");

        hr = eraser->put_ClientName(appName);
        if (FAILED(hr))
        {
            printf("FAILED to set client name for erase!\n");
            PrintHR(hr);
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
            printf("FAILED to eraser->get_CurrentPhysicalMediaType\n");
            PrintHR(hr);
        }
    }

    //// Set full
    if (SUCCEEDED(hr))
    {
        if (full)
        {
            hr = eraser->put_FullErase(VARIANT_TRUE);
            if (FAILED(hr))
            {
                printf("Failed put_FullErase()");
                PrintHR(hr);
            }            
        }
    }

    // release the recorder early...
    ReleaseAndNull(recorder);
   
    // create event
    if (SUCCEEDED(hr))
    {
        hr = ::ATL::CComObject<CTestErase2Event>::CreateInstance(&eventSink);
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

    // hookup events?
    if (SUCCEEDED(hr))
    {
        hr = eventSink->DispEventAdvise(eraser);
        if (FAILED(hr))
        {
            printf("Failed to hookup event sink\n");
            PrintHR(hr);
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
            printf("FAILED to erase!\n");
            PrintHR(hr);
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
        printf("EraseMedia succeeded for drive index %d\n",
               index
               );
    }
    else
    {
        printf("EraseMedia FAILED for drive index %d\n", index);
        PrintHR(hr);
    }
    return hr;
}