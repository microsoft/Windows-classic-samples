
/*

Copyright (c) 1999  Microsoft Corporation


Module Name:

    TAPISend.cpp


Abstract:

    This sample illustrates use of Media Streaming Terminal for sending audio.

    The application makes a TAPI call to the address specified in the command 
    line and uses Media Streaming Terminal to send the media content of a wav 
    file, also specified in the command line, to the remote machibe.
 
*/


#include "common.h"
#include "avifilereader.h"


//
// global TAPI object
//

ITTAPI *g_pTapi = NULL;


//
// a global flag, set true to request stop streaming
//

BOOL g_bExitRequested = FALSE;


//
// possible address types strings and corresponding LINEADDRESSTYPE_ constants
//

char *g_szAddressTypes[] = 
            {"PHONENUMBER", "CONFERENCE", "EMAIL", "MACHINE", "IP"};


long g_nAddressTypeConstants[] = 
            { LINEADDRESSTYPE_PHONENUMBER,
              LINEADDRESSTYPE_SDP,
              LINEADDRESSTYPE_EMAILNAME,
              LINEADDRESSTYPE_DOMAINNAME,
              LINEADDRESSTYPE_IPADDRESS };

//
// number of different address types
//

const UINT g_nNumberOfAddressTypes = 
                    sizeof(g_szAddressTypes)/sizeof(g_szAddressTypes[0]);


///////////////////////////////////////////////////////////////////////////////
// 
// LogMessage
//
//
// log a message using printf
//
///////////////////////////////////////////////////////////////////////////////

void LogMessage(IN CHAR *pszFormat, ... )
{
    
    //
    // output buffer -- note: hardcoded limit
    //

    static int  const BUFFER_SIZE = 1280;

    char szBuffer[BUFFER_SIZE]; 


    //
    // get current time
    //

    SYSTEMTIME SystemTime;

    GetLocalTime(&SystemTime);

    
    //
    // format thread id and time
    //

    StringCbPrintf( szBuffer, BUFFER_SIZE, "[%lx]:[%02u:%02u:%02u.%03u]::",
             GetCurrentThreadId(),
             SystemTime.wHour,
             SystemTime.wMinute,
             SystemTime.wSecond,
             SystemTime.wMilliseconds);


    size_t iStringLength = 0;

    HRESULT hr = StringCbLength(szBuffer, BUFFER_SIZE, &iStringLength);

    if (FAILED(hr))
    {
        // either this code is wrong, or someone else in the process corrupted 
        // our memory.
        return;
    }


    //
    // get the actual message
    //

    va_list vaArguments;

    va_start(vaArguments, pszFormat);


    size_t iBytesLeft = BUFFER_SIZE - iStringLength;


    //
    // not checking the return code of this function. even if it fails, we will
    // have a null-terminated string that we will be able to log.
    //

    StringCbVPrintf( &(szBuffer[iStringLength]), iBytesLeft, pszFormat, vaArguments);

    va_end(vaArguments);


    //
    // how big is the string now, and how many bytes do we have left?
    //

    hr = StringCbLength(szBuffer, BUFFER_SIZE, &iStringLength);

    if (FAILED(hr))
    {
        // either this code is wrong, or someone else in the process corrupted 
        // our memory.
        return;
    }

    iBytesLeft = BUFFER_SIZE - iStringLength;

    //
    // append a carriage return to the string. ignore the result code, the 
    // result string will be null-terminated no matter what.
    //

    StringCbCat(szBuffer, iBytesLeft, "\n");


    //
    // log the buffer 
    //

    printf(szBuffer);
}


#define LogError LogMessage


///////////////////////////////////////////////////////////////////////////////
// 
// LogFormat
//
// use LogMessage to log wave format
//
///////////////////////////////////////////////////////////////////////////////

void LogFormat(IN WAVEFORMATEX *pWaveFormat)
{
    LogMessage("    Format: ");
    LogMessage("        tag: %u", pWaveFormat->wFormatTag);
    LogMessage("        channels: %u", pWaveFormat->nChannels);
    LogMessage("        samples/sec: %lu", pWaveFormat->nSamplesPerSec);
    LogMessage("        align: %u", pWaveFormat->nBlockAlign);
    LogMessage("        bits/sample: %u", pWaveFormat->wBitsPerSample);

}


///////////////////////////////////////////////////////////////////////////////
//
// AllocateMemory
//
// use win32 heap api to allocate memory on the application's heap
// and zero the allocated memory
//
///////////////////////////////////////////////////////////////////////////////

void *AllocateMemory(SIZE_T nMemorySize)
{
    

    //
    // use HeapAlloc to allocate and clear memory
    //

    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nMemorySize);
}


///////////////////////////////////////////////////////////////////////////////
//
// FreeMemory
//
// use win32 heap api to free memory previously allocated on the application's
// heap
//
///////////////////////////////////////////////////////////////////////////////

void FreeMemory(void *pMemory)
{
    
    //
    // get size of the allocated memory
    //

    SIZE_T nMemorySize = HeapSize(GetProcessHeap(), 0, pMemory);

    if (-1 == nMemorySize)
    {
        LogError("FreeMemory: failed to get size of the memory block %p",
                 pMemory);

        //
        // don't exit -- try freeing anyway
        //

    }
    else
    {
        //
        // fill memory with 0xdd's before freeing, so it is easier to debug 
        // failures caused by using pointer to deallocated memory
        //
        
        if (0 != pMemory)
        {
            FillMemory(pMemory, nMemorySize, 0xdd);
        }

    }


    //
    // use HeapFree to free memory. use return code to log the result, but
    // do not return it to the caller
    //
    
    BOOL bFreeSuccess = HeapFree(GetProcessHeap(), 0, pMemory);

    if (FALSE == bFreeSuccess)
    {
        LogError("FreeMemory: HeapFree failed");

        //
        // if this assertion fires, it is likely there is a problem with the 
        // memory we are trying to deallocate. Was it allocated using heapalloc
        // and on the same heap? Is this a valid pointer?
        //

        _ASSERTE(FALSE);
    }

}


///////////////////////////////////////////////////////////////////////////////
// 
// InitializeTAPI()
//
// create and initialize the TAPI object
//
///////////////////////////////////////////////////////////////////////////////

HRESULT InitializeTAPI()
{
    
    HRESULT hr = E_FAIL;

    LogMessage("InitializeTAPI: started");
    
    
    //
    // create the TAPI object
    //

    hr = CoCreateInstance(
                          CLSID_TAPI,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTAPI,
                          (LPVOID *)&g_pTapi
                         );

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: CoCreateInstance on TAPI failed");

        return hr;
    }

    
    //
    // must initialize tapi object before using it
    //
    
    LogMessage("InitializeTAPI: calling ITTAPI::Initialize()");

    hr = g_pTapi->Initialize();

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: TAPI failed to initialize");

        g_pTapi->Release();
        g_pTapi = NULL;
        
        return hr;
    }

    
    //
    // setting event filtering -- this is required for synchronous 
    // ITBasicCallControl->Connect to work
    //
    
    LogMessage("InitializeTAPI: calling ITTAPI::put_EventFilter()");

    hr = g_pTapi->put_EventFilter(TE_CALLSTATE);

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: TAPI failed to put event filtering");

        g_pTapi->Shutdown();

        g_pTapi->Release();
        g_pTapi = NULL;
        
        return hr;
    }


    
    LogMessage("InitializeTAPI: succeeded");
    
    return S_OK;
}



///////////////////////////////////////////////////////////////////////////////
// 
// ShutdownTAPI
//
// shutdown and release the tapi object
// 
///////////////////////////////////////////////////////////////////////////////

void ShutdownTAPI()
{
    
    LogMessage("ShutdownTAPI: started");


    if (NULL != g_pTapi)
    {
        
        g_pTapi->Shutdown();
        
        g_pTapi->Release();
        g_pTapi = NULL;
    }

    
    LogMessage("ShutdownTAPI: completed");

}


///////////////////////////////////////////////////////////////////////////////
//
//  IsValidAudioFile
//
// returns TRUE if the file specified by pszFileName 
// exists and is a valid audio file
//
///////////////////////////////////////////////////////////////////////////////

BOOL IsValidAudioFile(IN char *pszFileName)
{

    //
    // open the file
    //
    
    CAVIFileReader FileReader;

    HRESULT hr = FileReader.Initialize(pszFileName);


    //
    // see if it is a valid audio file
    //

    if ((FAILED(hr)) || !FileReader.IsValidAudioFile())
    {
        LogError("IsValidAudioFile: file [%s] does not exist or is not "
                 "a valid wav file", pszFileName);

        return FALSE;
    }
    else
    {
        LogMessage("IsValidAudioFile: file [%s] is a valid audio file", 
                    pszFileName);

        return TRUE;
    }

}


///////////////////////////////////////////////////////////////////////////////
//
// FindAddress
//
// find an address of the requested type that supports audio. returns S_OK 
// if address found, failure otherwise
//
///////////////////////////////////////////////////////////////////////////////

HRESULT FindAddress(IN  long nAddressType,
                    OUT ITAddress **ppAddress)
{

    HRESULT hr = E_FAIL;


    //
    // don't return garbage even if we fail
    //
    
    *ppAddress = NULL;


    //
    // enumerate all available addresses
    //

    IEnumAddress *pEnumAddress = NULL;
    
    hr = g_pTapi->EnumerateAddresses(&pEnumAddress);
    
    if (FAILED(hr))
    {
        LogError("FindAddress: Failed to enumerate addresses");

        return hr;
    }


    //
    // walk through the enumeration of addresses and look for those which are
    // of requested type and supports audio 
    //

    while (TRUE)
    {
        //        
        // get the next address from the enumeration
        // 

        ITAddress *pAddress = NULL;

        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {

            //
            // no more addresses
            //

            LogMessage("FindAddress: no more addresses");

            break;
        }
        

        //
        // we got an address. check its capabilities
        //
        
        ITAddressCapabilities *pAddressCaps = NULL;

        hr = pAddress->QueryInterface( IID_ITAddressCapabilities, 
                                       (void**)&pAddressCaps );

        if (FAILED(hr))
        {

            LogError("FindAddress: "
                     "Failed to QI address for address capabilities");

            //
            // just continue to the next address
            // 

            pAddress->Release();
            pAddress = NULL;

            continue;
        }


        //
        // is this the right address type?
        //
    
        long nType = 0;

        hr = pAddressCaps->get_AddressCapability(AC_ADDRESSTYPES, &nType);
 

        pAddressCaps->Release();
        pAddressCaps = NULL;


        if (FAILED(hr))
        {
            LogError("FindAddress: Failed to get_AddressCapability");

            pAddress->Release();
            pAddress = NULL;

            continue;
        }


        if (nType & nAddressType)
        {
            
            //
            // this address is of the right type. does it support audio?
            //

            ITMediaSupport *pMediaSupport = NULL;

            hr = pAddress->QueryInterface(IID_ITMediaSupport,
                                           (void **)&pMediaSupport);

            if (FAILED(hr))
            {

                //
                // continue to the next address
                //

                LogError("FindAddress: "
                         "failed to qi address for ITMediaSupport");

                pAddress->Release();
                pAddress = NULL;

                continue;
            }

            
            VARIANT_BOOL bAudioSupported = VARIANT_FALSE;

            hr = pMediaSupport->QueryMediaType(TAPIMEDIATYPE_AUDIO, 
                                               &bAudioSupported);

            pMediaSupport->Release();
            pMediaSupport = NULL;
            
            if (SUCCEEDED(hr) && (VARIANT_TRUE == bAudioSupported))
            {

                LogMessage("FindAddress: address found");


                //
                // log the name of this address
                //
                
                BSTR bstrAddressName = NULL;
                
                hr = pAddress->get_AddressName(&bstrAddressName);

                if (FAILED(hr))
                {
                    LogError("FindAddress: failed to get address name");
                }
                else
                {
                    LogMessage("   %S\n", bstrAddressName);
                }

                SysFreeString(bstrAddressName);


                //
                // will use this address
                //

                *ppAddress = pAddress;

                break;;
            }
        }


        //
        // can't use this address. release it
        //

        pAddress->Release();
        pAddress = NULL;

    }

    
    //
    // done with the enumeration. release.
    //

    pEnumAddress->Release();
    pEnumAddress = NULL;


    //
    // log a message if no address was found
    //

    if (NULL == *ppAddress)
    {
        LogError("FindAddress: no address found");

        return E_FAIL;

    }


    LogMessage("FindAddress: completed");

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetAddressType
//
// convert a address type specified by pszRequestedAddressType
// to tapi LINEADDRESSTYPE_ constants (LINEADDRESSTYPE_PHONENUMBER,
// LINEADDRESSTYPE_IPADDRESS, etc).
//
// returns E_FAIL if the string does not correspond to a valid address type
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetAddressType(IN char *pszRequestedAddressType, 
                       IN OUT long *pnAddressType)
{
    
    //
    // match address type specified by the user to one of the known
    // address types
    //

    *pnAddressType = 0;


    for (int i = 0; i < g_nNumberOfAddressTypes; i++)
    {

        if (0 == _stricmp(g_szAddressTypes[i], pszRequestedAddressType))
        {
            
            //
            // get the address type constant corresponding to the string
            //

            *pnAddressType =  g_nAddressTypeConstants[i];

            LogMessage("GetAddressType: "
                       "matched address type [%s] to address type [%d]", 
                       pszRequestedAddressType, *pnAddressType);

            return S_OK;
        }


    }

    LogError("GetAddressType: unrecognized address type [%s]", 
              pszRequestedAddressType);

    return E_FAIL;
}



///////////////////////////////////////////////////////////////////////////////
//
// CreateBSTRfromString
// 
// create a bstr from a string supplied. the caller is responsible for
// freeng the returned string by calling SysFreeString. 
//
// returns the allocated string or NULL if failed.
//
///////////////////////////////////////////////////////////////////////////////

BSTR CreateBSTRfromString(IN char *pszString)
{

    //
    // convert to wchar so we can create bstr
    //
    // allocate buffer for resulting string of wchars
    //
    
    size_t nStringLength = strlen(pszString) + 1;

    WCHAR *pwsString = (WCHAR *)AllocateMemory(sizeof(WCHAR) *  nStringLength);

    if (NULL == pwsString)
    {
        LogError("CreateBSTRfromString: "
                 "failed to allocate memory for address string.");

        return NULL;
    }


    //
    // convert to wchar
    //

    int rc = MultiByteToWideChar(CP_ACP, 
                                 0, 
                                 pszString, 
                                 -1, 
                                 pwsString, 
                                 (int)nStringLength);

    if (0 == rc)
    {
        LogError("CreateBSTRfromString: Failed to convert char to wchar");

        FreeMemory(pwsString);
        pwsString = NULL;

        return NULL;
    }


    //
    // create bstr
    //

    BSTR bstr = SysAllocString(pwsString);


    //
    // no longer needed, deallocate
    //

    FreeMemory(pwsString);
    pwsString = NULL;

    return bstr;

}


///////////////////////////////////////////////////////////////////////////////
//
// CreateAndConnectCall
//
// make a call using the address object specified, to the address specified
//
// if successful, returns S_OK and connected call, error otherwise
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CreateAndConnectCall(IN  ITAddress *pAddress,
                             IN  char *pszDestinationAddress,
                             IN  long nAddressType,
                             OUT ITBasicCallControl **ppCall)
{

    HRESULT hr = E_FAIL;

    
    //
    // don't return garbage
    //

    *ppCall = NULL;


    //
    // create a call on the address
    //

    BSTR bstrDestinationAddress = CreateBSTRfromString(pszDestinationAddress);
    
    ITBasicCallControl *pCall = NULL;

    hr = pAddress->CreateCall(bstrDestinationAddress,
                              nAddressType,
                              TAPIMEDIATYPE_AUDIO,
                              &pCall);

    SysFreeString (bstrDestinationAddress);


    if (FAILED(hr))
    {
        LogError("CreateAndConnectCall: Failed to create a call");

        return hr;
    }


    //
    // call created. attempt to connect synchronously
    //

    LogMessage("CreateAndConnectCall: attempting a synchronous connect");

    hr = pCall->Connect(VARIANT_TRUE);

    if (S_OK != hr)
    {

        LogError("CreateAndConnectCall: failed to connect, hr = 0x%lx", hr);


        pCall->Disconnect(DC_NORMAL);


        //
        // we don't need the call object if it failed to connect
        //

        pCall->Release();
        pCall = NULL;

        hr = E_FAIL;

    }
    else
    {
        //
        // call is successful. return the call object
        //

        LogMessage("CreateAndConnectCall: call connected successfully");

        *ppCall = pCall;
    }


    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// Call
//
// make a call to the specified address on the first address object
// of the requested type that supports audio
//
// if successful, returns S_OK and connected call, error otherwise
// 
///////////////////////////////////////////////////////////////////////////////

HRESULT Call(IN char *szDestinationAddress,
             IN char *szAddressType,
             OUT ITBasicCallControl **ppCall)
{
    HRESULT hr = E_FAIL;

    
    //
    // we don't want to return garbage even if we fail
    //

    *ppCall = NULL;


    //
    // find address type
    //

    long nAddressType = 0;
    
    hr = GetAddressType(szAddressType, &nAddressType);

    if (FAILED(hr))
    {
        LogError("Call: failed to recognize address type %s", szAddressType);

        return hr;
    }

    
    //
    // find an address for this address type that supports audio
    //
    
    ITAddress *pAddress = NULL;

    hr = FindAddress(nAddressType, &pAddress);

    if (FAILED(hr))
    {
        LogError("Call: failed to find an address with audio for type %s", 
                 szAddressType);

        return hr;
    }


    //
    // have the address. create and connect call
    //

    ITBasicCallControl *pCall = NULL;

    hr = CreateAndConnectCall(pAddress,
                              szDestinationAddress,
                              nAddressType,
                              &pCall);

    pAddress->Release();
    pAddress = NULL;
    
    if (FAILED(hr))
    {
        LogError("Call: Failed to create and connect call");

        return hr;
    }


    //
    // we have a connected call. return it.
    //

    *ppCall = pCall;

    LogMessage("Call: succeeded.");

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// FindAudioStream
// 
// given a call, return the first outgoing audio stream.
//
// returns S_OK if successful, error otherwise
//
///////////////////////////////////////////////////////////////////////////////

HRESULT FindAudioStream( IN  ITBasicCallControl *pCall, 
                         OUT ITStream **ppStream)
{
    
    HRESULT hr = E_FAIL;
    
    LogMessage("FindAudioStream: started");


    //
    // don't return garbage
    //

    *ppStream = NULL;


    //
    // enumerate streams on the call
    //



    //
    // get the ITStreamControl interface for this call
    //

    ITStreamControl *pStreamControl = NULL;

    hr = pCall->QueryInterface(IID_ITStreamControl,
                                (void **) &pStreamControl);

    if (FAILED(hr))
    {
        LogError("FindAudioStream: failed to QI call for ITStreamControl");

        return hr;
    }


    //
    // enumerate the streams
    //

    IEnumStream *pEnumStreams = NULL;
    
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);
    
    pStreamControl->Release();
    pStreamControl = NULL;

    if (FAILED(hr))
    {
        LogError("CreateAndSelectMST: failed to enumerate streams on call");
        
        return hr;
    }

    
    //
    // walk through the streams on the call
    // return the first outgoing audio stream
    //
  
    while (TRUE)
    {
        ITStream *pStream = NULL;

        hr = pEnumStreams->Next(1, &pStream, NULL);

        if (S_OK != hr)
        {
            //
            // no more streams
            //

            break;
        }


        //
        // check the stream's direction
        //

        TERMINAL_DIRECTION td;

        hr = pStream->get_Direction(&td);

        if (FAILED(hr))
        {
            LogError("CreateAndSelectMST: Failed to get stream direction");
            
            pStream->Release();
            pStream = NULL;
            

            //
            // proceed to the next stream, if any
            //

            continue;
        }


        //
        // is the stream of the right direction?
        //

        if (TD_CAPTURE != td)
        {

            //
            // incoming stream. we need outgoing. 
            // release the stream and continue
            // 

            pStream->Release();
            pStream = NULL;

            continue;
        }


        //
        // check the stream's media type
        //

        long lMediaType = 0;

        hr = pStream->get_MediaType(&lMediaType);

        if (FAILED(hr))
        {
            LogError("CreateAndSelectMST: Failed to get media type");

            pStream->Release();
            pStream = NULL;
            
            continue;
        }

        //
        // Does this stream have the right media type?
        // Streams are defined as having exactly one media type
        // (not a bitmask).
        //

        if ( lMediaType == TAPIMEDIATYPE_AUDIO )
        {

            LogMessage("FindAudioStream: stream found");


            //
            // this is what we need, so stop looking
            //

            *ppStream = pStream;

            break;
        }


        pStream->Release();
        pStream = NULL;

    } // while (walking through the call's streams)



    //
    // release stream enumeration
    //

    pEnumStreams->Release();
    pEnumStreams = NULL;


    //
    // return the error code, depending on whether we found a stream or not
    //

    if (NULL == *ppStream)
    {
        LogMessage("FindAudioStream: didn't find an audio stream");

        return E_FAIL;
    }

    LogMessage("FindAudioStream: succeded");

    return S_OK;

}


///////////////////////////////////////////////////////////////////////////////
//
// CreateCaptureMediaStreamingTerminal
//
// create media streaming terminal for outgoing data
//
// returns the created terminal or NULL if failed
//
///////////////////////////////////////////////////////////////////////////////

ITTerminal *CreateCaptureMediaStreamingTerminal(IN ITBasicCallControl *pCall)
{
    
    HRESULT hr = E_FAIL;


    //
    // get the call's call info so we can get the call's address 
    //

    ITCallInfo *pCallInfo = NULL;

    hr = pCall->QueryInterface(IID_ITCallInfo, (void**)&pCallInfo);

    if (FAILED(hr))
    {
        LogError("CreateCaptureMediaStreamingTerminal: "
                 "failed to qi call for ITCallInfo");

        return NULL;
    }


    //
    // now we can get the address
    //

    ITAddress *pAddress = NULL;

    hr = pCallInfo->get_Address(&pAddress);
    
    pCallInfo->Release();
    pCallInfo = NULL;

    if (FAILED(hr))
    {
        LogError("CreateCaptureMediaStreamingTerminal: "
                 "failed to get call's address");
        return NULL;
    }

    
    //
    // get the terminal support interface
    //

    ITTerminalSupport *pTerminalSupport = NULL;

    hr = pAddress->QueryInterface( IID_ITTerminalSupport, 
                                   (void **)&pTerminalSupport );

    
    pAddress->Release();
    pAddress = NULL;


    if (FAILED(hr))
    {
        LogError("CreateCaptureMediaStreamingTerminal: "
                 "failed to QI pAddress for ITTerminalSupport");

        return NULL;
    }

    
    //
    // get string for the terminal's class id
    //

    LPOLESTR psTerminalClass = NULL;

    hr = StringFromIID(CLSID_MediaStreamTerminal, &psTerminalClass);

    if (FAILED(hr))
    {
        LogError("CreateCaptureMediaStreamingTerminal: "
                 "Failed to generate string from terminal's class id");

        pTerminalSupport->Release();
        pTerminalSupport = NULL;

        return NULL;
    }


    //
    // make bstr out of the class id
    //

    BSTR bstrTerminalClass = SysAllocString (psTerminalClass);


    //
    // free the string returned by StringFromIID
    //

    CoTaskMemFree(psTerminalClass);
    psTerminalClass = NULL;


    if (NULL == bstrTerminalClass)
    {

        LogError("CreateCaptureMediaStreamingTerminal: "
                 "Failed to allocate BSTR for terminal class");

        pTerminalSupport->Release();
        pTerminalSupport = NULL;

        return NULL;
    }

    

    //
    // create media streaming terminal
    //
    
    ITTerminal *pTerminal = NULL;

    hr = pTerminalSupport->CreateTerminal(bstrTerminalClass,
                                          TAPIMEDIATYPE_AUDIO,
                                          TD_CAPTURE,
                                          &pTerminal);

    
    //
    // release resources no longer needed
    //

    SysFreeString(bstrTerminalClass);

    pTerminalSupport->Release();
    pTerminalSupport = NULL;


    if (FAILED(hr))
    {
        LogError("CreateCaptureMediaStreamingTerminal: "
                 "failed to create media streaming terminal hr = 0x%lx", hr);

        return NULL;
    }


    //
    // successfully created media streaming terminal. return.
    //

    LogMessage("CreateCaptureMediaStreamingTerminal: "
               "Terminal created successfully");

    return pTerminal;

}



///////////////////////////////////////////////////////////////////////////////
//
// SetTerminalFormat
//
// tell media streaming terminal the format of the data we are going to provide
//
///////////////////////////////////////////////////////////////////////////////

HRESULT SetTerminalFormat(IN ITTerminal *pTerminal, 
                          IN WAVEFORMATEX *pWaveFormat)
{

    HRESULT hr = E_FAIL;


    //
    // log format requested
    //

    LogMessage("SetTerminalFormat: starting.");
    LogFormat(pWaveFormat);

    
    ITAMMediaFormat *pIMediaFormat = NULL;
    
    hr = pTerminal->QueryInterface(IID_ITAMMediaFormat, 
                                   (void **)&pIMediaFormat);

    if (FAILED(hr)) 
    { 
    
        LogError("SetTerminalFormat: Failed to set terminal format");

        return hr; 
    }


    //
    // fill the media format structure 
    //

    AM_MEDIA_TYPE MediaType;

    ZeroMemory(&MediaType, sizeof(AM_MEDIA_TYPE));

    MediaType.majortype            = MEDIATYPE_Audio;
    MediaType.subtype              = MEDIASUBTYPE_PCM;
    MediaType.bFixedSizeSamples    = TRUE;
    MediaType.bTemporalCompression = FALSE;
    MediaType.lSampleSize          = pWaveFormat->nBlockAlign;
    MediaType.formattype           = FORMAT_WaveFormatEx;
    MediaType.pUnk                 = NULL;

    MediaType.cbFormat             = sizeof(WAVEFORMATEX) + 
                                     pWaveFormat->cbSize;

    MediaType.pbFormat             = (BYTE*)pWaveFormat;

    
    //
    // set the requested format 
    //

    hr = pIMediaFormat->put_MediaFormat(&MediaType);
    
    if (FAILED(hr))
    {

        //
        // try to see what format the terminal wanted
        //

        LogError("SetTerminalFormat: failed to set format");

        AM_MEDIA_TYPE *pMediaFormat = NULL;

        HRESULT hr2 = pIMediaFormat->get_MediaFormat(&pMediaFormat);


        if (SUCCEEDED(hr2))
        {

            if (pMediaFormat->formattype == FORMAT_WaveFormatEx)
            {

                //
                // log the terminal's format
                //

                LogError("SetTerminalFormat: terminal's format is");
                LogFormat((WAVEFORMATEX*) pMediaFormat->pbFormat);

            }
            else 
            {

                LogError("SetTerminalFormat: "
                         "terminal's format is not WAVEFORMATEX");
            }


            //
            // note: we are responsible for deallocating the format returned by
            // get_MediaFormat
            //

            DeleteMediaType(pMediaFormat);

        } //  succeeded to get terminal's format
        else
        {

            LogError("SetTerminalFormat: failed to get terminal's format");

        }

    } // failed to set format


    pIMediaFormat->Release();
    pIMediaFormat = NULL;

    LogMessage("SetTerminalFormat: completed");
    
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// SetAllocatorProperties
//
// sets allocator properties to the terminal
//
///////////////////////////////////////////////////////////////////////////////

HRESULT SetAllocatorProperties(IN ITTerminal *pTerminal)
{

    //
    // different buffer sizes may produce different sound quality, depending
    // on the underlying transport that is being used.
    // 
    // this function illustrates how an app can control the number and size of
    // buffers. A multiple of 30 ms (480 bytes at 16-bit 8 KHz PCM) is the most
    // appropriate sample size for IP (especailly G.723.1).
    //
    // However, small buffers can cause poor audio quality on some voice boards.
    //
    // If this method is not called, the allocator properties suggested by the 
    // connecting filter will be used.
    //
    // Note: do not set allocator properties in the applications unless you are
    // sure that sound quality will not degrade as a result. Some MSPs can have
    // their own preferred allocator properties, and will not be able to 
    // provide the best quality if the app sets its own properties, different 
    // from what is preferred by the msp.
    //
    // Also note that ITAllocatorProperties::SetBufferSize allows the app to
    // specify preferred size of the buffer allocated to the application 
    // without affecting terminal's allocator properties.
    //
    
    LogMessage("SetAllocatorProperties: starting.");


    HRESULT hr = E_FAIL;


    //
    // get ITAllocator properties interface on the terminal
    //

    ITAllocatorProperties *pITAllocatorProperties = NULL;


    hr = pTerminal->QueryInterface(IID_ITAllocatorProperties, 
                                   (void **)&pITAllocatorProperties);


    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: "
                 "failed to QI terminal for ITAllocatorProperties");

        return hr;
    }

    
    //
    // configure allocator properties
    //

    ALLOCATOR_PROPERTIES AllocProps;
    
    AllocProps.cbBuffer   = 4800;
    AllocProps.cBuffers   = 5;
    AllocProps.cbAlign    = 1;
    AllocProps.cbPrefix   = 0;

    
    hr = pITAllocatorProperties->SetAllocatorProperties(&AllocProps);

    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: failed to set allocator properties. "
                 "hr = 0x%lx", hr);

        pITAllocatorProperties->Release();
        pITAllocatorProperties = NULL;

        return hr;
    }

    
    //
    // ask media streaming terminal to allocate buffers for us. 
    // TRUE is the default, so strictly speaking, we didn't have to call 
    // this method.
    //

    hr = pITAllocatorProperties->SetAllocateBuffers(TRUE);


    pITAllocatorProperties->Release();
    pITAllocatorProperties = NULL;

    
    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: failed to SetAllocateBuffers, "
                 "hr = 0x%lx", hr);

        return hr;
    }


    //
    // succeeded setting allocator properties
    //

    LogMessage("SetAllocatorProperties: completed");

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// ReadFileIntoTerminal
//
// read data from the file and submit it to media streaming terminal
// 
// exit when finished reading the file, user requested exit by pressing 
// ctrl+break, or the connection broke
//
// returns S_FALSE when finished streaming the file, S_OK if user requested 
// exit, failure otherwise
//
///////////////////////////////////////////////////////////////////////////////

HRESULT ReadFileIntoTerminal(IN CAVIFileReader *pFileReader, 
                             IN ITTerminal *pPlaybackTerminal)
{

    HRESULT hr = E_FAIL;

    LogMessage("ReadFileIntoTerminal: started.");


    Sleep(20000);


    //
    // get terminal's IMediaStream interface
    //

    IMediaStream *pTerminalMediaStream = NULL;

    hr = pPlaybackTerminal->QueryInterface(IID_IMediaStream, 
                                           (void**)&pTerminalMediaStream);

    if (FAILED(hr))
    {
        LogError("ReadFileIntoTerminal: "
                 "failed to QI terminal for IMediaStream");

        return hr;
    }

    
    //
    // create a queue (STL deque) that will hold all the samples that we ever 
    // submitted to media streaming terminal
    //
    // we need this so when we are finished reading the file, we can go through 
    // the list of all the samples that we have submitted and make sure mst 
    // is finished processing them
    //
    // note that samples get reused, the same samples will be put in the queue 
    // more than once. so the size of the queue will be proportional to the 
    // size of the file being played. this might cause problems if
    // the file is big or the source of the samples is unlimited (live audio
    // feed). In this case, the logic can be modified to only enqueue each
    // sample once, by comparing against existing queue entries.
    // 
    
    std::deque<IStreamSample*> SampleQ;


    //
    // count the number of samples that have been submitted
    //

    ULONG nSampleCount = 0;

    
    //
    // keep reading samples from file and sending them.
    // (until user requests exit, there is no more data, or failure)
    //

    while (!g_bExitRequested)
    {
       
        //
        // allocate a sample on the terminal's media stream
        //
        // Note: the call to AllocateSample() will block if we filled all the 
        // samples with data, and there are no more samples for us to fill 
        // (waiting for media streaming terminal to process samples we have
        // submitted). When MST is is done with at least one sample, the call 
        // will return. This logic will ensure that MST always has work and is
        // never starved for samples.
        //

        IStreamSample *pStreamSample = NULL;

        hr = pTerminalMediaStream->AllocateSample(0, &pStreamSample);

        if (FAILED(hr))
        {
            LogError("ReadFileIntoTerminal: "
                     "failed to allocate a sample on terminal's stream");

            break;
        }


        //
        // get IMemoryData on the sample so we can get to the sample's memory 
        // data
        //

        IMemoryData *pSampleMemoryData = NULL;

        hr = pStreamSample->QueryInterface(IID_IMemoryData, 
                                           (void**)&pSampleMemoryData);

        if (FAILED(hr))
        {
            LogError("ReadFileIntoTerminal: failed to qi sample for IMemoryData");

            pStreamSample->Release();
            pStreamSample = NULL;

            break;

        }


        //
        // get to the sample's memory buffer
        //

        DWORD nBufferSize = 0;

        BYTE *pBuffer = NULL;

        hr = pSampleMemoryData->GetInfo(&nBufferSize, &pBuffer, NULL);

        if (FAILED(hr))
        {

            LogError("ReadFileIntoTerminal: "
                     "failed to get info on sample's memory");

            pStreamSample->Release();
            pStreamSample = NULL;

            pSampleMemoryData->Release();
            pSampleMemoryData = NULL;

            break;

        }


        //
        // read file into memory buffer provided by the sample
        //

        LONG nBytesRead = 0;
        
        hr = pFileReader->Read(pBuffer, nBufferSize, &nBytesRead);

        if (FAILED(hr))
        {
            LogError("ReadFileIntoTerminal: failed to read data from file");

            pStreamSample->Release();
            pStreamSample = NULL;

            pSampleMemoryData->Release();
            pSampleMemoryData = NULL;

            break;

        }
        else if (S_FALSE == hr)
        {
            //
            // no more data
            //

            LogMessage("ReadFileIntoTerminal: finished reading file");

            pStreamSample->Release();
            pStreamSample = NULL;

            pSampleMemoryData->Release();
            pSampleMemoryData = NULL;

            break;
        }


        
        //
        // tell the sample how many useful bytes are in the sample's buffer
        //
        
        hr = pSampleMemoryData->SetActual(nBytesRead);

        pSampleMemoryData->Release();
        pSampleMemoryData = NULL;

        if (FAILED(hr))
        {

            LogError("ReadFileIntoTerminal: failed to SetActual (%ld bytes) "
                     "on the sample.", nBytesRead);

            pStreamSample->Release();
            pStreamSample = NULL;

            break;

        }

        
        //
        // we are done with the sample. now let media streaming terminal 
        // process it asynchronously. when the terminal is finished with 
        // the sample, this sample will be returned to us from the call 
        // to AllocateSample()
        //

        hr = pStreamSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);


        //
        // with some MSPs, starting the stream can be done asynchronously, so
        // there may be a delay between the time when terminal is selected
        // (or call connected) and the time when the stream becomes usable.
        //
        // attempting to use the stream before the stream is active would
        // result in the Update() returning error VFW_E_NOT_COMMITTED.
        //
        // Usually an application would not start using the stream until 
        // it gets media event CME_STREAM_ACTIVE. This requires the app
        // to register a callback interface by calling 
        // ITTAPI::RegisterCallNotifications. Refer to documentation and other 
        // samples for more details on how this is done.
        //
        // To keep things simple, this sample doesn't do event processing.
        // To deal with the problem of using the stream before it becomes 
        // active, we retry Update() until we succeed.
        //
        // Note that there is still a danger that the stream becomes 
        // disconnected before we process the first sample, in which case
        // we will be stuck in a loop, which can be exited when the user
        // presses ctrl+break
        //

        while ( (hr == VFW_E_NOT_COMMITTED)
                && (0 == nSampleCount) 
                && !g_bExitRequested )
        {
            LogMessage("ReadFileIntoTerminal: "
                       "Update returned VFW_E_NOT_COMMITTED. "
                       "Likely cause: stream not yet started. Retrying.");

            Sleep( 1000 );

            hr = pStreamSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);
        }

        if (FAILED(hr))
        {
            LogError("ReadFileIntoTerminal: failed to Update the sample");

            pStreamSample->Release();
            pStreamSample = NULL;

            break;

        }

        
        //
        // the sample was submitted successfully. update count
        //

        nSampleCount++;

        if (nSampleCount == 300)
        {
            
            LogError("ReadFileIntoTerminal: sleeping 10 seconds");

            Sleep(30000);

            LogError("ReadFileIntoTerminal: woke up");
        }

        //
        // keep the sample we have just submitted. on exit, we will wait 
        // for it to be processed by mst
        //

        SampleQ.push_back(pStreamSample);

    } // file reading/sample-filling loop


    LogMessage("ReadFileIntoTerminal: processed %lu samples", nSampleCount);

    
    //
    // walk through the list of all the samples we have submitted and wait for 
    // each sample to be done
    //
    
    while (!SampleQ.empty())
    {

        //
        // get and remove a sample from the queue 
        //

        IStreamSample *pStreamSample = SampleQ.front();

        SampleQ.pop_front();


        //
        // wait for the Media Streaming Terminal to finish 
        // processing the sample
        //

        pStreamSample->CompletionStatus(COMPSTAT_WAIT, INFINITE);


        //
        // ignore the error code -- release the sample in any case
        //

        pStreamSample->Release();
        pStreamSample = NULL;

    }


    LogMessage("ReadFileIntoTerminal: released all submitted samples");

    
    //
    // tell media streaming terminal's stream that there is no more data
    //

    pTerminalMediaStream->SendEndOfStream(0);


    //
    // ignore the error code
    //

    pTerminalMediaStream->Release();
    pTerminalMediaStream = NULL;


    //
    // if we disconnect the call right away, the call may be dropped before
    // receiver gets all the samples. An application should wait for 
    // STREAM_INACTIVE media event before disconnecting the call.
    //
    // Since, for simplicity, we are not processing events in this sample,
    // wait several seconds to give the receiver a little time to complete
    // processing.
    //

    LogMessage("ReadFileIntoTerminal: Sleeping to give the receiver time "
               "to process everything we have sent.");

    Sleep(7500);


    LogMessage("ReadFileIntoTerminal: completed");
   
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// CreateAndSelectTerminal
//
// creates a media streaming terminal for capture, sets requested format,
// sets allocator properties, and selects the terminal on the call's first 
// outgoing audio stream.
//
// returns S_OK and terminal if success
// error if failure
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CreateAndSelectTerminal(IN ITBasicCallControl *pCall,
                                IN WAVEFORMATEX *pWaveFormat,
                                OUT ITTerminal **ppTerminal)
{

    HRESULT hr = E_FAIL;

    
    //
    // don't return garbage
    //

    *ppTerminal = NULL;



    //
    // find an outgoing audio stream on the call
    //

    ITStream *pStream = NULL;

    hr = FindAudioStream(pCall, &pStream);

    if (FAILED(hr))
    {
        LogError("CreateAndSelectTerminal: failed to find an outgoing audio stream");

        return hr;
    }


    //
    // create media streaming terminal
    //
    
    ITTerminal *pTerminal = NULL;

    pTerminal = CreateCaptureMediaStreamingTerminal(pCall);


    if (NULL == pTerminal)
    {

        LogError("CreateAndSelectTerminal: Failed to create media streaming terminal");

        pStream->Release();
        pStream = NULL;

        return hr;

    }


    //
    // tell media streaming terminal format of the data 
    // we are going to send. If the terminal cannot handle this format
    // return an error
    //

    hr = SetTerminalFormat(pTerminal, pWaveFormat);

    if (FAILED(hr))
    {
        LogMessage("CreateAndSelectTerminal: "
                   "terminal does not support requested format");
    
        pStream->Release();
        pStream = NULL;

        pTerminal->Release();
        pTerminal = NULL;

        return hr;
    }


    //
    // set allocator properties. 
    //
    // calling ITAllocatorProperties::SetAllocatorProperties with the 
    // properties that are not optimal for the MSP in use can result
    // in loss of sound quality.
    //
    // So make sure that you only call this function if you know you
    // need it.
    // 
    // Do not use ITAllocatorProperties::SetAllocatorProperties to set
    // the size of the buffer you want to get when you fill samples, 
    // ITAllocatorProperties::SetBufferSize will accomplish that without
    // affecting terminal's allocator properties.
    //

    // hr = SetAllocatorProperties(pTerminal);

    if (FAILED(hr))
    {

        //
        // not fatal -- we are still likely to successfully stream data
        //

        LogMessage("CreateAndSelectTerminal: "
                   "failed to set allocator properties. continuing.");
    }


    //
    // select the terminal on the stream
    //

    hr = pStream->SelectTerminal(pTerminal);


    //
    // don't need the stream anymore
    //

    pStream->Release();
    pStream = NULL;
    

    if (FAILED(hr))
    {
        LogError("CreateAndSelectTerminal: Failed to select terminal on the stream");

        pTerminal->Release();
        pTerminal = NULL;

    }

    
    //
    // if everything went smoothly pTerminal has a pointer to configured 
    // and selected terminal. otherwise pTerminal is null and all resources 
    // have been released
    //

    *ppTerminal = pTerminal;

    
    return hr;

}


///////////////////////////////////////////////////////////////////////////////
//
// StreamFile
//
// use TAPI to connect to the remote machine to stream file
//
///////////////////////////////////////////////////////////////////////////////

HRESULT StreamFile(IN char *szFileName, 
                   IN char *szAddressString, 
                   IN char *szAddressType)
{

    HRESULT hr = E_FAIL;


    LogMessage("StreamFile: file [%s] address [%s] address type [%s]", 
                szFileName, szAddressString, szAddressType);


    //
    // check if the file is valid
    //

    if (!IsValidAudioFile(szFileName))
    {

        LogError("StreamFile: file not valid [%s]", szFileName);

        return E_FAIL;
    }


    //
    // initialize COM libraries -- used by TAPI
    //

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if ( FAILED(hr))
    {
        LogError("StreamFile: Failed to CoInitialize");

        return hr;
    }


    //
    // create and initialize TAPI
    //

    hr = InitializeTAPI();

    if (SUCCEEDED(hr))
    {

        //
        // try to make a call
        //

        ITBasicCallControl *pCall = NULL;

        hr = Call(szAddressString, szAddressType, &pCall);

        if (SUCCEEDED(hr))
        {

            //
            // construct the file reader object
            // to be used to read file's data
            //
            // note: If you want to play more than one file in one call, reuse 
            // the same terminal for the duration of the call. Under 
            // Windows 2000, all data submitted via the Media Streaming Terminal
            // during the call must have the same format. Changing a Media 
            // Streaming Terminal's format after it is initially configured will 
            // always return an error code, regardless of the OS version. 
            // Unselecting a Media Streaming Terminal and creating and selecting
            // a new terminal on the same stream from the same call with a 
            // different format is not supported on Windows 2000. This may, 
            // however, be supported on other versions of Windows. For the latest 
            // information on which versions of Windows support this, please 
            // refer to the latest Platform SDK documentation.
            //

            CAVIFileReader FileReader;

            hr =  FileReader.Initialize(szFileName);


            //
            // get the file's format. remember to deallocate when done.
            //

            WAVEFORMATEX *pWaveFormat = NULL;

            if (SUCCEEDED(hr) && 
                SUCCEEDED(hr = FileReader.GetFormat(&pWaveFormat)))
            {

                //
                // create and configure a media streaming terminal and select it on this call
                //
                //

                ITTerminal *pPlaybackTerminal = NULL;

                hr = CreateAndSelectTerminal(pCall, pWaveFormat, &pPlaybackTerminal);


                if (SUCCEEDED(hr))
                {

                    //
                    // use the terminal to send the file
                    //

                    hr = ReadFileIntoTerminal(&FileReader, pPlaybackTerminal);

                    if (FAILED(hr))
                    {
                        LogError("StreamFile: failed to ReadFileIntoTerminal");
                    }
                    else
                    {
                        LogError("StreamFile: succeeded");
                    }



                    //
                    // release the terminal, we no longer need it
                    //

                    pPlaybackTerminal->Release();
                    pPlaybackTerminal = NULL;

                }
                else
                {
                    LogError("StreamFile: failed to create and select terminal");

                }


                //
                // no longer need wave format. free memory.
                //

                FreeMemory(pWaveFormat);
                pWaveFormat = NULL;

            }  // got file format
            else
            {
                LogError("StreamFile: failed to get file's format");

            }


            //
            // there is not much we can do if disconnect fails, 
            // so ignore its return code
            //

            pCall->Disconnect(DC_NORMAL);

            pCall->Release();
            pCall = NULL;

        }   // call connected
        else
        {
        
            LogError("StreamFile: failed to connect to %s", szAddressString);

        }


        //
        // tapi has been initialized. shutdown now.
        //

        ShutdownTAPI();

    } // initialized tapi
    else
    {

        LogError("StreamFile: Failed to initialize TAPI");

    }


    CoUninitialize();
        
    return hr;
}



///////////////////////////////////////////////////////////////////////////////
//
// HelpScreen
//
// this function displays usage information
//
///////////////////////////////////////////////////////////////////////////////

void HelpScreen()
{

    printf("Usage:\n\n"
               "  TAPISend filename address addresstype\n\n"
               "  where addresstype is [ ");

    int i=0;
    for ( i = 0; i < g_nNumberOfAddressTypes - 1; ++i)
    {
        printf("%s | ", g_szAddressTypes[i]);
    }

    printf("%s ]\n", g_szAddressTypes[i]);

}


///////////////////////////////////////////////////////////////////////////////
// 
// CtrlHandler
//
// handler for ctrl+break, close, logoff and shutdown. 
//
// sets g_bExitRequested flag signaling shutdown. this ensures graceful exit
// 
///////////////////////////////////////////////////////////////////////////////

BOOL CtrlHandler(DWORD nEventType) 
{

    //
    // are we in the middle of shutting down?
    //

    if (TRUE == g_bExitRequested)
    {
        LogMessage("CtrlHandler: shutdown is already in progress");

        return TRUE;
    }


    //
    // any exit event (close, ctrl+break/C, logoff, shutdown)
    // is a signal for the application to exit.
    //

    switch (nEventType) 
    { 
 
        case CTRL_C_EVENT: 
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:

            LogMessage("CtrlHandler: Initiating shutdown.");


            //
            // signal shutdown
            //

            g_bExitRequested = TRUE;


    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
// main 
//
// usage: TAPISend filename address addresstype
// 
// returns 0 if success, 1 if failure
//
///////////////////////////////////////////////////////////////////////////////

int __cdecl main(int argc, char* argv[])
{

    LogMessage("main: started");


    //
    // validate arguments
    //

    if (argc != 4)
    {
        HelpScreen();

        LogMessage("main: invalid arguments, exiting.");

        return 1;
    }


    //
    // we want to handle ctrl+c and ctrl+break events so we can cleanup on exit
    // proceed even in case of failure
    //

    SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE);


    //
    // open the file, connect to the remote machine and stream the file over
    //

    HRESULT hr = StreamFile(argv[1], argv[2], argv[3]);


    //
    // exiting... we no longer want to handle ctrl+c and ctrl+break
    //
    
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, FALSE);


    //
    // was file streaming successful?
    //

    if (FAILED(hr))
    {
        LogError("main: Failed to stream file");

        return 1;
    }


    LogMessage("main: completed");

    return 0;

}

