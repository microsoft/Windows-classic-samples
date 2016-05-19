// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// ---------------------------------------------------------------------------
// File: CDVGraph.cpp
// 
// Desc: CDVGraph Class definition, it supports DV Graph Building
//       This is the class to build all AVC graphs using 
//       MSTape.sys
//----------------------------------------------------------------------------

#include "CDVGraph.h"

#include "dbgsup.h"

BOOL IsDeviceOutputDV(IBaseFilter * pFilter);


/*-----------------------------------------------------------------------------
|   Function:   CDVGraph::CDVGraph
|   Purpose:    Constructor for digital tv graph class.  
|   Arguments:  None 
|   Returns:    None 
|   Notes:      initializes the digital tv graph components
\----------------------------------------------------------------------------*/
CDVGraph::CDVGraph(void)
    : m_VideoFormat( DVENCODERVIDEOFORMAT_NTSC )
    , m_DVResolution( DVRESOLUTION_HALF )
    , m_SubunitMode ( VcrMode )
    , m_pGraph( NULL )
    , m_pCaptureGraphBuilder( NULL )
    , m_pMediaControl( NULL )
    , m_pMediaEvent( NULL )
    , m_pInputFileFilter (NULL )
    , m_pVideoWindow( NULL )
    , m_pDeviceFilter( NULL )
    , m_pIAMExtDevice( NULL )
    , m_pIAMExtTransport( NULL )
    , m_pIAMTCReader( NULL )
    , m_pDroppedFrames( NULL )
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
}

/*-----------------------------------------------------------------------------
|   Function:   CDVGraph::~CDVGraph
|   Purpose:    Destructor for the graph info class.  
|   Arguments:  None
|   Returns:    None 
|   Notes:      Any clean up needed should be put here.
\----------------------------------------------------------------------------*/
CDVGraph::~CDVGraph(void)
{
    if(m_pVideoWindow != NULL)
    {
        //Otherwise, a video image remains on the screen and the user cannot get rid of it. 
        m_pVideoWindow->put_Visible(OAFALSE);

        //Otherwise, messages are sent to the wrong window, likely causing errors.
        m_pVideoWindow->put_Owner(NULL);  
    }

    FreeFilters();
    CoUninitialize();
}

/*-----------------------------------------------------------------------------
|   Function:   CDVGraph::FreeFilters
|   Purpose:    Destructor for the graph info class.  
|   Arguments:  None
|   Returns:    None 
|   Notes:      Any clean up needed should be put here.
\----------------------------------------------------------------------------*/
void CDVGraph::FreeFilters()
{
    // Release the DirectShow interfaces created 
    SAFE_RELEASE(m_pGraph) 
    SAFE_RELEASE(m_pCaptureGraphBuilder) 
    SAFE_RELEASE(m_pMediaEvent);
    SAFE_RELEASE(m_pMediaControl) 
    SAFE_RELEASE(m_pInputFileFilter);
    SAFE_RELEASE(m_pDeviceFilter);
    SAFE_RELEASE(m_pIAMExtDevice);
    SAFE_RELEASE(m_pIAMExtTransport);
    SAFE_RELEASE(m_pIAMTCReader);
    SAFE_RELEASE(m_pVideoWindow);
    SAFE_RELEASE(m_pDroppedFrames);
}

/*-----------------------------------------------------------------------------
|   Function:   CAVCGraph::GraphInitialize
|   Purpose:    Initializing the graph class members.  
|   Arguments:  None
|   Returns:    Boolean TRUE if successfull or throws exception detailing the error
|   Notes:      Initializes the required DirectShow interfaces
\----------------------------------------------------------------------------*/
HRESULT CDVGraph::InitializeGraph()
{
    HRESULT hr = S_OK;
    Dump1( TEXT(" CDVGraph::InitializeGraph().hr = %x\0"), hr );

    // All DirectShow FilterGraphs need this
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
                          IID_IGraphBuilder, (LPVOID *)&m_pGraph );
    CHECK_ERROR( TEXT(" Failed to create FilterGraph."), hr);

    // Helps the building all Graphs
    hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
                          (REFIID)IID_ICaptureGraphBuilder2, 
                          (void **)&m_pCaptureGraphBuilder);
    CHECK_ERROR( TEXT(" Failed to create CaptureGraphBuilder2."), hr);

    hr = m_pCaptureGraphBuilder->SetFiltergraph(m_pGraph);
    CHECK_ERROR( TEXT(" Failed to SetFiltergraph."), hr);

    hr = m_pGraph->QueryInterface(IID_IMediaEventEx, 
                                  reinterpret_cast<PVOID *>(&m_pMediaEvent)); 
    CHECK_ERROR( TEXT(" Failed to QI IMediaEventEx."), hr);

    // DirectShow Interface for Run, Stop, Pause the flow of the streams through the filter graph
    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pMediaControl);
    CHECK_ERROR( TEXT(" Failed to QI IMediaControl."), hr);

    hr = m_pGraph->QueryInterface(IID_IVideoWindow, (void **) &m_pVideoWindow);
    CHECK_ERROR( TEXT(" Failed to  QI IVideoWindow."), hr);

    return hr;
}

/*-----------------------------------------------------------------------------
|   Function:   CDVGraph::BuildBasicGraph
|   Purpose:    Add device filter to the graph 
|   Arguments:  None 
|   Returns:    HRESULT
|   Notes:      
\----------------------------------------------------------------------------*/
HRESULT CDVGraph::BuildBasicGraph()
{
    HRESULT hr;

    hr = InitializeGraph();
    CHECK_ERROR( TEXT(" Failed to  initialize graph."), hr);

    hr = AddDeviceFilter();
    CHECK_ERROR( TEXT(" Failed to  add device filter."), hr);

    ASSERT(m_pDeviceFilter);

    hr = m_pDeviceFilter->QueryInterface(IID_IAMExtTransport, 
                                        (void **) &m_pIAMExtTransport);
    CHECK_ERROR( TEXT(" Failed to  QI IAMExtTransport."), hr);
  
    hr = m_pDeviceFilter->QueryInterface(IID_IAMExtDevice, (void **) &m_pIAMExtDevice);
    CHECK_ERROR( TEXT(" Failed to  QI IAMExtDevice."), hr);

    hr = m_pDeviceFilter->QueryInterface(IID_IAMTimecodeReader, (void **) &m_pIAMTCReader);
    CHECK_ERROR( TEXT(" Failed to  QI IAMTimecodeReader."), hr);

    hr= GetDVMode( &m_SubunitMode );
    CHECK_ERROR( TEXT(" GetMode Failed."), hr);

    return S_OK;
}


/*-----------------------------------------------------------------------------
|   Function:   CDVGraph::AddDeviceFilter
|   Purpose:    Load (not add) a filter of the name "Microsoft DV Camera and VCR" 
|               which connects to the specified filter
|   Arguments:  None 
|   Returns:    HRESULT
|   Notes:      This method adds the device filter to the filtergraph
\----------------------------------------------------------------------------*/
HRESULT CDVGraph::AddDeviceFilter()
{
    HRESULT hr = S_OK;
    ICreateDevEnum * pCreateDevEnum = NULL;
    IEnumMoniker *   pEnumMoniker = NULL;
    IMoniker *       pMoniker = NULL;
    ULONG            nFetched = 0;

    // Create Device Enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, 
                          IID_ICreateDevEnum, reinterpret_cast<PVOID *>(&pCreateDevEnum));
    CHECK_ERROR( TEXT(" Failed to create SystemDeviceEnum."), hr);
    
    // Create the enumerator of the monikers for the specified Device Class & reset them 
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if(SUCCEEDED(hr) && pEnumMoniker)
        pEnumMoniker->Reset();
    else   {
        Dump( TEXT(" Failed to CreateClassEnumerator.") );
        return hr;
    }


    // Loop through to the last moniker
    while(SUCCEEDED(pEnumMoniker->Next( 1, &pMoniker, &nFetched )) && pMoniker)    
    {

        // get the device friendly name:
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void **)&pPropBag );
        CHECK_ERROR( TEXT(" Failed to BindToStorage."), hr);

        //Friendly name
        VARIANT varFriendlyName;
        varFriendlyName.vt = VT_BSTR;
        hr = pPropBag->Read( L"FriendlyName", &varFriendlyName, 0 );
        CHECK_ERROR( TEXT(" Failed to read friendlyname."), hr);

#ifdef UNICODE      
        (void)StringCchCopy( m_DeviceName, NUMELMS(m_DeviceName), varFriendlyName.bstrVal );
#else
        WideCharToMultiByte( CP_ACP, 0, varFriendlyName.bstrVal, -1, m_DeviceName, sizeof(m_DeviceName), 0, 0 );
#endif      

        VariantClear( &varFriendlyName );


        // detect DV device by search media type of its output pins for DV type
        IBaseFilter *pDeviceFilter;

        hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pDeviceFilter );
        CHECK_ERROR(TEXT("CAVCGraph::AddDeviceFilter():: BindToObject failed"), hr);
        if(pDeviceFilter== NULL)
            return E_FAIL;

        if(IsDeviceOutputDV(pDeviceFilter))
        {
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pDeviceFilter );
            CHECK_ERROR( TEXT(" Failed to BindToObject."), hr);

            hr = m_pGraph->AddFilter(m_pDeviceFilter, L"Filter");
            CHECK_ERROR( TEXT(" Failed to AddFilter."), hr);

            SAFE_RELEASE(pMoniker);
            SAFE_RELEASE(pDeviceFilter);
            break;
        }
        else
            hr = E_FAIL;
            
        SAFE_RELEASE(pMoniker);
        SAFE_RELEASE(pDeviceFilter);
    }//end of while

    SAFE_RELEASE( pEnumMoniker );
    SAFE_RELEASE( pCreateDevEnum );

    return hr; 
}

/*-------------------------------------------------------------------------
Routine:        CDVGraph::GetTapeInfo
Purpose:        Get Frame rate and availability of dvcr tape
Arguments:      None
Returns:        HRESULT as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::GetTapeInfo(void)
{
    HRESULT hr;
    LONG    lMediaType = 0;
    LONG    lInSignalMode = 0;

    // Query Media Type of the transport
    hr = m_pIAMExtTransport->GetStatus(ED_MEDIA_TYPE, &lMediaType);
    CHECK_ERROR( TEXT(" GetStatus failed."), hr);

    if (ED_MEDIA_NOT_PRESENT == lMediaType)
    {
        // Return failure if there is no tape installed
        hr = S_FALSE;  
    } 
    else
    {
        // Tape type should always be DVC
        ASSERT(ED_MEDIA_DVC == lMediaType);

        // Now lets query for the signal mode of the tape.
        hr = m_pIAMExtTransport->GetTransportBasicParameters(ED_TRANSBASIC_INPUT_SIGNAL, 
                                                             &lInSignalMode, NULL);
        CHECK_ERROR( TEXT(" GetTransportBasicParameters failed."), hr);

        // determine whether the camcorder supports ntsc or pal
        switch (lInSignalMode)
        {
            case ED_TRANSBASIC_SIGNAL_525_60_SD :
                m_AvgTimePerFrame = 33;  // 33 milliseconds (29.97 FPS)
                m_VideoFormat = DVENCODERVIDEOFORMAT_NTSC;
                break;

            case ED_TRANSBASIC_SIGNAL_525_60_SDL :
                m_AvgTimePerFrame = 33;  // 33 milliseconds (29.97 FPS)
                m_VideoFormat = DVENCODERVIDEOFORMAT_NTSC;
                break;

            case ED_TRANSBASIC_SIGNAL_625_50_SD :
                m_AvgTimePerFrame = 40;  // 40 milliseconds (25 FPS)
                m_VideoFormat = DVENCODERVIDEOFORMAT_PAL;
                break;

            case ED_TRANSBASIC_SIGNAL_625_50_SDL :
                m_AvgTimePerFrame = 40;  // 40 milliseconds (25 FPS)
                m_VideoFormat = DVENCODERVIDEOFORMAT_PAL;
                break;

            default : 
                Dump(TEXT("Unsupported or unrecognized tape format type."));
                m_AvgTimePerFrame = 33;  // 33 milli-sec (29.97 FPS); default
                break;
        }
        Dump1(TEXT("Avg time per frame is %d FPS\0"), m_AvgTimePerFrame);
    }
 
    return hr;
}

/*-------------------------------------------------------------------------
Routine:        CDVGraph::DV_GetDVMode
Purpose:        Determines camera mode using IAMExtDevice::GetCapability()
Arguments:      None
Returns:        Subunit mode of camera device
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::GetDVMode(DV_MODE *pSubunitMode)
{
    HRESULT hr = S_OK;
    LONG    lDeviceType = 0;

    ASSERT(m_pDeviceFilter);
    ASSERT(pSubunitMode);

    if (!pSubunitMode)
        return E_POINTER;
    if (!m_pIAMExtDevice)
        return E_NOINTERFACE;

    //  Query the Device Type Capability
    hr = m_pIAMExtDevice->GetCapability(ED_DEVCAP_DEVICE_TYPE, &lDeviceType, 0);
    CHECK_ERROR( TEXT(" m_pIAMExtDevice->GetCapability() failed."), hr);
    
    switch (lDeviceType)
    {
        case 0 :
            //device type is unknown
            *pSubunitMode = UnknownMode;
            break;

        case ED_DEVTYPE_VCR :
            *pSubunitMode = VcrMode;
            break;

        case ED_DEVTYPE_CAMERA :
            *pSubunitMode = CameraMode;
            break;

        default :
            Dump(TEXT("GetCapability returned an unknown device type!"));
            break;
    } 
    
    return hr;
} 

/*-------------------------------------------------------------------------
Routine:        DV_SaveGraph
Purpose:        Save the filter graph into a *.grf file
Arguments:      FileName
Returns:        HRESULT as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::SaveGraphToFile(TCHAR* sGraphFile)
{
    IStorage *          pStorage = NULL;
    IStream *           pStream = NULL;
    IPersistStream *    pPersistStream = NULL;
    HRESULT             hr = S_OK;

    if(m_pCaptureGraphBuilder == NULL || sGraphFile == NULL)
        return E_FAIL;

    // Either Open or Create the *.GRF file
    hr = StgOpenStorage( sGraphFile, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_DENY_WRITE, 
                         NULL, NULL, &pStorage );
    if ( STG_E_FILENOTFOUND == hr )
        hr = StgCreateDocfile( sGraphFile, STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE , 
                         NULL , &pStorage);
    CHECK_ERROR( TEXT(" StgCreateDocfile failed."), hr);
    
    hr = pStorage->CreateStream( L"ActiveMovieGraph", STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 
                                 NULL, NULL, &pStream );
    CHECK_ERROR( TEXT(" CreateStream failed."), hr);

    // Persist the stream, save & commit to disk
    hr = m_pGraph->QueryInterface( IID_IPersistStream, (void **) &pPersistStream );
    CHECK_ERROR( TEXT(" QI IPersistStream failed."), hr);

    hr = pPersistStream->Save(pStream, TRUE);
    CHECK_ERROR( TEXT(" pPersistStream->Save() failed."), hr);

    hr = pStorage->Commit( STGC_DEFAULT );
    CHECK_ERROR( TEXT(" Save GRF file failed."), hr);

    SAFE_RELEASE(pStorage);
    SAFE_RELEASE(pStream);
    SAFE_RELEASE(pPersistStream);

    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        CDVGraph::MakePreviewGraph()
Purpose:        Builds the DV preview graph
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a  preview graph for DV :           
                    DV_Cam(AV Out)->DVSplitter(vid)->DVCodec->VideoWindow
                                    DVSplitter(aud)->Default DirectSound device
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakePreviewGraph()
{
    m_iGraphType = GRAPH_PREVIEW;

    HRESULT hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, 
                                                      &MEDIATYPE_Interleaved, 
                                                      m_pDeviceFilter,  
                                                      NULL, NULL);
    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        CDVGraph::MakeDvToFileGraph_Type1
Purpose:        Builds and runs the DV to File graph
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a capture & preview graph for DV Type 1 AVI files           
                    This graph is a bit more complex.  It looks like this:
                    DV_Cam(AV Out)->SmartTee(capture)->AviMux->FileWriter
                                    SmartTee(preview)->DVSplitter(vid)->DVCodec->VideoWindow
                                                       DVSplitter(aud)->Default DirectSound device
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeDvToFileGraph_Type1(TCHAR* OutputFileName)
{
    m_iGraphType = GRAPH_DV_TO_FILE;
    HRESULT hr = S_OK;

    ASSERT(OutputFileName[0]);
    
    SmartPtr<IBaseFilter>        ppf;
    SmartPtr<IFileSinkFilter>    pSink;     
    hr = m_pCaptureGraphBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, OutputFileName, &ppf, &pSink);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_Type1::m_pCaptureGraphBuilder->SetOutputFileName() failed."), hr);

    hr = SetAviOptions(ppf, INTERLEAVE_NONE);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_Type1::m_pCaptureGraphBuilder->SetAviOptions() failed."), hr);
    
    // The graph we're making is:   MSDV --> Smart Tee --> AVI Mux --> File Writer
    //                                 --> DV Splitter --> DV Decoder --> Video Renderer
    //                                 --> Audio Renderer       
    // Connect interleaved stream of MSDV to the AVI Mux/FW
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, m_pDeviceFilter, NULL, ppf);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // Build a preview graph off of it too
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, m_pDeviceFilter, NULL, NULL);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);
   
    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeDvToFileGraph_NoPre
Purpose:        Builds and runs the DV to File graph with no preview
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a capture only graph for DV Type 1 AVI files            
                    This graph is not too complex.  It looks like this:
                    DV_Cam(AV Out)->AviMux->FileWriter
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeDvToFileGraph_NoPre_Type1(TCHAR* OutputFileName)
{
    m_iGraphType = GRAPH_DV_TO_FILE_NOPRE;
    HRESULT hr = S_OK;
    ASSERT(OutputFileName[0]) ;
    
    //add the avimux, and file writer to the graph 
    SmartPtr<IBaseFilter>        ppf = NULL;
    SmartPtr<IFileSinkFilter>    pSink = NULL;

    hr = m_pCaptureGraphBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, OutputFileName, &ppf, &pSink);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->SetOutputFileName() failed."), hr);

    hr = SetAviOptions(ppf, INTERLEAVE_NONE);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // the graph we're making is:  MSDV --> AVI Mux --> File Writer
    // Set the AVI Options like interleaving mode etc...       
    // Build the graph
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, m_pDeviceFilter, 
                                              NULL, ppf);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);
         
    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeFileToDvGraph
Purpose:        Builds and runs the File to DV graph
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a transmit & playback graph for DV Type 1 AVI files         
                This graph is a bit more complex.  It looks like this:
                    FileSource->AVI_Splitter->InfPinTee->DV_Camera
                                              InfPinTee->DVSplitter(vid)->DVDecoder->VideoWIndow
                                                         DVSplitter(aud)->Default DirectSound device
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeFileToDvGraph_Type1(TCHAR* InputFileName)
{
    m_iGraphType = GRAPH_FILE_TO_DV;
    HRESULT hr = S_OK;

    // Add the file as source filter to the graph
    hr = m_pGraph->AddSourceFilter(InputFileName, InputFileName, &m_pInputFileFilter);
    CHECK_ERROR( TEXT(" CDVGraph::MakeFileToDvGraph_Type1::m_pGraph->AddSourceFilter failed."), hr);

    SmartPtr<IBaseFilter> pAviSplitter = NULL;
    SmartPtr<IBaseFilter> pInfTee      = NULL;

    hr = CoCreateInstance(CLSID_AviSplitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pAviSplitter));
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::CoCreate AviSplitter  failed."), hr);

    hr = CoCreateInstance(CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pInfTee));
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::CoCreate InfTee failed."), hr);

    // the graph we're making is:   Async Reader --> AVI Splitter --> Tee --> MSDV
    //                                           --> DV Splitter --> DV Decoder --> VidRend
    //                                                                          --> AudRend
    // Add the AVI Splitter
    hr = m_pGraph->AddFilter(pAviSplitter, L"AVI Splitter");
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pGraph->AddFilter failed."), hr);

    // Connect file source to the splitter
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, pAviSplitter);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // Add an infinite Tee
    hr = m_pGraph->AddFilter(pInfTee, L"Infinite Tee"); 
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pGraph->AddFilter() failed."), hr);

    // Connect the Splitter to the Tee
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, &MEDIATYPE_Interleaved, pAviSplitter, NULL, pInfTee);
    CHECK_ERROR( TEXT(" CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // Connect one branch of the tee to MSDV 
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pInfTee, NULL, m_pDeviceFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // Build a preview graph off the other branch
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pInfTee, NULL, NULL);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeFileToDvGraph_NoPre
Purpose:        Builds and runs the File to DV graph without preview
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a transmit only graph for DV Type 1 AVI files           
                    This graph is a bit simplex.  It looks like this:
                    FileSource->AVI_Splitter->DV_Camera
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeFileToDvGraph_NoPre_Type1(TCHAR* InputFileName)
{
    m_iGraphType = GRAPH_FILE_TO_DV_NOPRE;
    HRESULT hr = S_OK;
    ASSERT(InputFileName[0]) ;

    // Add the file as source filter to the graph
    hr = m_pGraph->AddSourceFilter(InputFileName, InputFileName, &m_pInputFileFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDVGraph_NoPre_Type1::m_pGraph->AddSourceFilter() failed."), hr);

    // the graph we're making is:    Async Reader --> AVI Splitter --> MSDV 
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, m_pDeviceFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDVGraph_NoPre_Type1::m_pCaptureGraphBuilder->RenderStream() failed."), hr);
              
    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeDvToFileGraph_Type2
Purpose:        Builds and runs the DV to File graph
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a capture & preview graph for DV Type 2 AVI files           
                    This graph is a bit more complex.  It looks like this:
                    DV_Cam(AV Out)->DVSplitter(vid)->SmartTee(capture)->AviMux->FileWriter
                                                     SmartTee(preview)->DVCodec->VideoWindow
                                    DVSplitter(aud)->InfinitePinTee->AviMux->FileWriter
                                                     InfinitePinTee->Default DirectSound device
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeDvToFileGraph_Type2(TCHAR* OutputFileName)
{
    m_iGraphType = GRAPH_DV_TO_FILE_TYPE2;
    HRESULT hr = S_OK;

	// making sure there is a output file selected
    ASSERT(OutputFileName[0]);

    SmartPtr<IBaseFilter>    pDVSplitter          = NULL;
    hr = CoCreateInstance(CLSID_DVSplitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVSplitter));
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::create DVSplitter failed."), hr);

    hr =  m_pGraph->AddFilter(pDVSplitter, L"DV Splitter");
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::m_pGraph->AddFilter() failed."), hr);


    //add the avimux, and file writer to the graph 
    SmartPtr<IBaseFilter> ppf = NULL;
    SmartPtr<IFileSinkFilter> pSink = NULL;
    hr = m_pCaptureGraphBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, OutputFileName, &ppf, &pSink);
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::m_pCaptureGraphBuilder->SetOutputFileName() failed."), hr);

    // Set the AVI Options like interleaving mode etc...
    hr = SetAviOptions(ppf, INTERLEAVE_NONE);
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::SetAviOptions() failed."), hr);
             
    // the graph we're making is:   MSDV --> Smart Tee --> DV SPLITTER --> AVI MUX --> File Writer
    //                                                 --> DV SPLITTER --> DV DEC  --> Video Renderer
    //                                                                             --> Audio Renderer 

    // Connect MSDV Interleave stream through DV Splitter to AVI MUX/FW
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, m_pDeviceFilter, pDVSplitter, ppf);
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // Connect the other DV Splitter output to the AVI MUX/FW
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pDVSplitter, NULL, ppf);
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    // Render the preview part of the graph
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, m_pDeviceFilter, NULL, NULL);
    CHECK_ERROR( TEXT("CDVGraph::MakeDVToFileGraph_Type2::m_pCaptureGraphBuilder->RenderStream() failed."), hr);

    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeDvToFileGraph_NoPre_Type2
Purpose:        Builds and runs the DV to File graph
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a capture only graph for DV Type 2 AVI files            
                    This graph is a bit simplex .  It looks like this:
                    DV_Cam(AV Out)->DVSplitter(vid)->AviMux->FileWriter
                                                DVSplitter(aud)->AviMux->FileWriter
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeDvToFileGraph_NoPre_Type2(TCHAR* OutputFileName)
{
    m_iGraphType = GRAPH_DV_TO_FILE_NOPRE_TYPE2;
    HRESULT hr = S_OK;

	// making sure there is a output file selected
    ASSERT(OutputFileName[0]);

    SmartPtr<IBaseFilter> pDVSplitter = NULL;

    hr = CoCreateInstance(CLSID_DVSplitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVSplitter));
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::create DVSplitter failed."), hr);
   
    hr = m_pGraph->AddFilter(pDVSplitter, L"DV Splitter");
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::m_pGraph->AddFilter failed."), hr);

    //add the avimux, and file writer to the graph 
    SmartPtr<IBaseFilter>        ppf = NULL;
    SmartPtr<IFileSinkFilter>    pSink = NULL;

    hr = m_pCaptureGraphBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, OutputFileName, &ppf, &pSink);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::m_pCaptureGraphBuilder->SetOutputFileName failed."), hr);
              
    // the graph we're making is:   MSDV --> Smart Tee --> DV SPLITTER --> AVI MUX --> File Writer
    //
    // connect MSDV interleaved pin through DV Splitter to AVI MUX/FW
    hr = m_pCaptureGraphBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, m_pDeviceFilter, pDVSplitter, ppf);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect the other DV Splitter output to the AVI MUX/FW
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pDVSplitter, NULL, ppf);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);
                      
    // Set the AVI Options like interleaving mode etc...
    hr = SetAviOptions(ppf, INTERLEAVE_NONE);
    CHECK_ERROR( TEXT("CDVGraph::MakeDvToFileGraph_NoPre_Type2::SetAviOptions failed."), hr);

    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeFileToDvGraph_Type2
Purpose:        Builds and runs the File to DV graph 
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a transmit & playback graph for DV Type 2 AVI files         
                    This graph is a bit complex.  It looks like this:
                     FileSource->AVI_Splitter(vid) ->DVMuxer(vid)---------->InfPinTee->DV_Camera
                                        AVI_Splitter(aud)->DVMuxer(aud)     InfPinTee->DVSplitter(vid)->DVDecoder->VideoWIndow
                                                                                       DVSplitter(aud)->DSoundDevice
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeFileToDvGraph_Type2(TCHAR* InputFileName)
{
    m_iGraphType = GRAPH_FILE_TO_DV_TYPE2;
    HRESULT hr = S_OK;

	ASSERT(InputFileName[0] );

    SmartPtr<IBaseFilter>  pDVMux   = NULL;
    SmartPtr<IBaseFilter>  pInfTee  = NULL;
    hr = CoCreateInstance(CLSID_DVMux, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVMux));
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::create DVMux failed."), hr);

    hr = CoCreateInstance(CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pInfTee));
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::CoCreate InfTeefailed."), hr); 

    // Add the file as source filter to the graph
    hr =m_pGraph->AddSourceFilter(InputFileName, InputFileName, &m_pInputFileFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pGraph->AddSourceFilter failed."), hr);

    // Add the DVMuxer to the graph
    hr = m_pGraph->AddFilter(pDVMux, L"DV Muxer");
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pGraph->AddFilter failed."), hr);

    //Add the infinite pin tee filter to the graph and connect it downstream to the dv muxer
    hr = m_pGraph->AddFilter(pInfTee, L"Infinite Tee"); 
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pGraph->AddFilter failed."), hr);

    // the graph we need to build is:    ASYNC reader --> AVI SPLITTER --> DV MUX --> TEE --> MSDV
    //                                                --> DVSP --> DV DEC --> VR
    //                                                --> AR
    // connect file source video stream to DV MUX
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, pDVMux);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect file source audio stream to DV MUX
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, pDVMux);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect DV MUX to tee
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pDVMux, NULL, pInfTee);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect one branch of tee to MSDV transmit
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pInfTee, NULL, m_pDeviceFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // render other branch of tee to audio & video preview
    SmartPtr<IPin> pOut;
    hr = m_pCaptureGraphBuilder->FindPin(pInfTee, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pOut);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pCaptureGraphBuilder->FindPin failed."), hr);

    hr = m_pGraph->Render(pOut);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_Type2::m_pGraph->Render failed."), hr);

    return hr;
}

/*---------------------------------------------------------------------------------------------------------
Routine:        DV_MakeFileToDvGraph_NoPre_Type2
Purpose:        Builds and runs the File to DV graph 
Arguments:      None
Returns:        HRESULT as apropriate
Notes:          This is a transmit only graph for DV Type 2 AVI files           
                    This graph looks like this:
                     FileSource->AVI_Splitter(vid) ->DVMuxer(vid)----->DV_Camera
                                        AVI_Splitter(aud)->DVMuxer(aud)                 
---------------------------------------------------------------------------------------------------------*/
HRESULT CDVGraph::MakeFileToDvGraph_NoPre_Type2(TCHAR* InputFileName)
{
    m_iGraphType = GRAPH_FILE_TO_DV_NOPRE_TYPE2;
    HRESULT hr = S_OK;

	ASSERT(InputFileName[0]);

    SmartPtr< IBaseFilter > pDVMux = NULL;
    hr = CoCreateInstance(CLSID_DVMux, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVMux));
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::CoCreate DVMux failed."), hr);
  
    // Add the file as source filter to the graph
    hr = m_pGraph->AddSourceFilter(InputFileName, InputFileName, &m_pInputFileFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::m_pGraph->AddSourceFilter failed."), hr);

    // Add the DVMuxer to the graph
    hr = m_pGraph->AddFilter(pDVMux, L"DV Muxer");
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::add DVMux failed."), hr);

    // the graph we need to build is:   ASYNC reader --> AVI SPLITTER --> DV MUX --> MSDV
    //
    // connect file video stream to DV MUX
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, pDVMux);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect file audio stream to DV MUX
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, m_pInputFileFilter, NULL, pDVMux);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    // connect DV MUX to DV Transmit
    hr = m_pCaptureGraphBuilder->RenderStream(NULL, NULL, pDVMux, NULL, m_pDeviceFilter);
    CHECK_ERROR( TEXT("CDVGraph::MakeFileToDvGraph_NoPre_Type2::m_pCaptureGraphBuilder->RenderStream failed."), hr);

    return hr;
}

/*-------------------------------------------------------------------------
Routine:        DV_SetAviOptions
Purpose:        Routine for changing AVI Mux properties.  In this sample, 
                we just set a few options.  
                These options could be set through the Avi Mux property sheet, 
                or through a separate dialog.
Arguments:      Pointer to the AVI renderer (from SetOutputFileName())
Returns:        HRESULT as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::SetAviOptions(IBaseFilter *ppf, InterleavingMode INTERLEAVE_MODE)
{
    HRESULT hr;
    SmartPtr<IConfigAviMux>        pMux           = NULL;
    SmartPtr<IConfigInterleaving>  pInterleaving  = NULL;

    ASSERT(ppf);
    if (!ppf)
        return E_POINTER;

    // QI for interface AVI Muxer
    hr = ppf->QueryInterface(IID_IConfigAviMux, reinterpret_cast<PVOID *>(&pMux));
    CHECK_ERROR( TEXT("CDVGraph::SetAviOptions::QI IConfigAviMux failed."), hr);

    hr = pMux->SetOutputCompatibilityIndex(TRUE);
    CHECK_ERROR( TEXT("CDVGraph::SetAviOptions::pMux->SetOutputCompatibilityIndex failed."), hr);

    // QI for interface Interleaving
    hr = ppf->QueryInterface(IID_IConfigInterleaving, reinterpret_cast<PVOID *>(&pInterleaving));
    CHECK_ERROR( TEXT("CDVGraph::SetAviOptions::QI IConfigInterleaving failed."), hr);

    // put the interleaving mode (full, none, half)
    hr = pInterleaving->put_Mode(INTERLEAVE_MODE);
    CHECK_ERROR( TEXT("CDVGraph::SetAviOptions::pInterleaving->put_Mode failed."), hr);
   
    return hr;
} 

/*-----------------------------------------------------------------------------
|   Function:   CAVCGraph::RemoveFilters
|   Purpose:    Tears downs the graph from the specified filter onwards.  
|   Arguments:  Filter to be removed downstream from
|   Returns:    None 
|   Notes:      The filter specified in the argument is not removed from the filtergraph; This is a recursively calling function
\----------------------------------------------------------------------------*/
HRESULT CDVGraph::RemoveFilters(IBaseFilter *pFilter, BOOL bRemoveDownStream)
{
    HRESULT hr = S_OK;
    IPin *pPin = NULL, *pToPin = NULL;
    IEnumPins *pEnumPins = NULL;

    ULONG uFetched = 0;
    PIN_INFO PinInfo;

    ASSERT(m_pGraph);
    
    // Validating the the pointer to the Filter is not null
    if(!pFilter)
    {
        Dump( TEXT("CAVCGraph::RemoveFilters():: Invalid Argument:: Invalid filter to remove from") );
        return E_FAIL;
    }

    // enumerate all the pins on this filter
    // reset the enumerator to the first pin
    hr = pFilter->EnumPins(&pEnumPins);
    CHECK_ERROR( TEXT("CAVCGraph::RemoveFilters():: Could not enumerate pins on the filter to be removed.hr=%#x"), hr);
    pEnumPins->Reset(); 
   
    // Loop through all the pins of the filter
    while( SUCCEEDED(pEnumPins->Next(1, &pPin, &uFetched)) && pPin )
    {
        // Get the pin & its pin_info struct that this filter's pin is connected to 
        hr = pPin->ConnectedTo(&pToPin);
        if(SUCCEEDED(hr) &&pToPin )
        {
            hr = pToPin->QueryPinInfo(&PinInfo);
            CHECK_ERROR( TEXT("pToPin->QueryPinInfo failed.hr=%#x"), hr);
                   
            // Check that this ConnectedTo Pin is a input pin thus validating that our filter's pin is an output pin
            if(PinInfo.dir == PINDIR_INPUT && bRemoveDownStream)
            {
                // thus we have a pin on the downstream filter so remove everything downstream of that filter recursively
                RemoveFilters(PinInfo.pFilter, bRemoveDownStream);
                // Disconnect the two pins and remove the downstream filter
                m_pGraph->Disconnect(pToPin);
                m_pGraph->Disconnect(pPin);

                //always leave the Camera filter in the graph
                if (PinInfo.pFilter != m_pDeviceFilter)
                {           
                    hr = m_pGraph->RemoveFilter(PinInfo.pFilter);
                }
            }

            SAFE_RELEASE(PinInfo.pFilter);                 
            SAFE_RELEASE(pToPin);
        }
        SAFE_RELEASE(pPin);       
    }

    SAFE_RELEASE(pEnumPins);
    return S_OK;
}

/*-------------------------------------------------------------------------
Routine:        DV_StartGraph
Purpose:        Starts the Filter Graph 
Arguments:      None
Returns:        HResult as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::StartGraph(void)
{
    HRESULT hr;

    // start the graph
    hr = m_pMediaControl->Run();
    if ( FAILED(hr))
    {
        Dump(TEXT("CDVGraph::StartGraph::m_pMediaControl->Run() Failed!"));
        // stop parts that ran
        m_pMediaControl->Stop();
    }

    return hr;
}

/*-------------------------------------------------------------------------
Routine:        DV_PauseGraph
Purpose:        Starts the Filter Graph 
Arguments:      None
Returns:        HResult as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::PauseGraph(void)
{
    HRESULT hr;

    // Pause the graph
    hr = m_pMediaControl->Pause();
    if ( FAILED(hr))
    {
        Dump(TEXT("CDVGraph::StartGraph::m_pMediaControl->Pause() Failed!"));
         // stop parts that ran
        m_pMediaControl->Stop();
    }

    return hr;
}


/*-------------------------------------------------------------------------
Routine:        DV_StopGraph
Purpose:        Starts the Filter Graph 
Arguments:      None
Returns:        HResult as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT CDVGraph::StopGraph(void)
{
    HRESULT hr = m_pMediaControl->Stop();
    return hr;
}

/*-------------------------------------------------------------------------
Routine:        CDVGraph::getDroppedFrameNum
Purpose:        Callback proc to display dropped frame info
Arguments:      [in]DWORD is the graph in transmit mode
Arguments:      [out]long* number of dropped frames
Arguments:      [out]long* number of not dropped frames
Returns:        None
Notes:          For both Capture & Transmit graphs
------------------------------------------------------------------------*/
HRESULT CDVGraph::getDroppedFrameNum( BOOL *bIsModeTransmit, long* pDropped, long* pNotdropped)
{
    HRESULT hr;
    SmartPtr<IPin> pAVIn = NULL;

    SAFE_RELEASE(m_pDroppedFrames);
    ASSERT(bIsModeTransmit != NULL);
    
    if (!bIsModeTransmit)
        return E_POINTER;

    // capture
    if (GRAPH_DV_TO_FILE == m_iGraphType || GRAPH_DV_TO_FILE_NOPRE == m_iGraphType ||
        GRAPH_DV_TO_FILE_TYPE2 == m_iGraphType || GRAPH_DV_TO_FILE_NOPRE_TYPE2 == m_iGraphType)
        *bIsModeTransmit = FALSE;

    // transmit
    else if (GRAPH_FILE_TO_DV == m_iGraphType || GRAPH_FILE_TO_DV_NOPRE == m_iGraphType ||
        GRAPH_FILE_TO_DV_TYPE2 == m_iGraphType || GRAPH_FILE_TO_DV_NOPRE_TYPE2 == m_iGraphType)
        *bIsModeTransmit = TRUE;

    if( *bIsModeTransmit)
    {
        hr = m_pCaptureGraphBuilder->FindPin(m_pDeviceFilter, PINDIR_INPUT, NULL, NULL, FALSE, 0, &pAVIn);
        if(FAILED(hr) || pAVIn == NULL)
        {
            Dump(TEXT("CDVGraph::getDroppedFrameNum::m_pCaptureGraphBuilder->FindPin Failed!"));
            return hr;
        }
        
        hr = pAVIn->QueryInterface(IID_IAMDroppedFrames, reinterpret_cast<PVOID *>(&m_pDroppedFrames));
        CHECK_ERROR( TEXT("CDVGraph::getDroppedFrameNum::QI IAMDroppedFrames failed."), hr);        
    }
    else
    {
        hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, 
                                                   &MEDIATYPE_Interleaved, 
                                                   m_pDeviceFilter, IID_IAMDroppedFrames, 
                                                   reinterpret_cast<PVOID *>(&m_pDroppedFrames));
        CHECK_ERROR( TEXT("CDVGraph::getDroppedFrameNum::m_pCaptureGraphBuilder->FindInterface failed."), hr);        
    }
     
    hr = m_pDroppedFrames->GetNumDropped(pDropped);
    CHECK_ERROR( TEXT("CDVGraph::getDroppedFrameNum::m_pDroppedFrames->GetNumDropped failed."), hr);        

    hr = m_pDroppedFrames->GetNumNotDropped(pNotdropped);
    CHECK_ERROR( TEXT("CDVGraph::getDroppedFrameNum::m_pDroppedFrames->GetNumDropped failed."), hr);        

    return hr;
}

/*-------------------------------------------------------------------------
Routine:        ChangeFrameRate
Purpose:        Controls discard or not half of the frames in the video stream
Arguments:      None
Returns:        HRESULT
Notes:          For NTSC, the frame rate is reduced from 30 frames per second (fps) to 15 fps. 
                For PAL, the frame rate is reduced from 25 fps to 12.5 fps.
------------------------------------------------------------------------*/
HRESULT CDVGraph::ChangeFrameRate(BOOL bHalfFrameRate)
{
    HRESULT hr = S_OK;
    SmartPtr<IBaseFilter>    pDVSplitter;
    SmartPtr<IDVSplitter>    pIDVSplitter = NULL;

    /* Obtain the dv docoder's IBaseFilter interface. */
    hr = m_pGraph->FindFilterByName(L"DV Splitter", &pDVSplitter) ;
    CHECK_ERROR( TEXT("CDVGraph::ChangeFrameRate()::m_pGraph->FindFilterByName failed."), hr);        

    hr = pDVSplitter->QueryInterface(IID_IDVSplitter, reinterpret_cast<PVOID *>(&pIDVSplitter));
    CHECK_ERROR( TEXT("CDVGraph::ChangeFrameRate()::QI IDVSplitter failed."), hr);        
    
    if(bHalfFrameRate)
        hr = pIDVSplitter->DiscardAlternateVideoFrames(1);  // if the value is non-zero, discards alternate frames. 
    else 
        hr = pIDVSplitter->DiscardAlternateVideoFrames(0);  // If the value is zero, the filter delivers every frame.
    CHECK_ERROR( TEXT("CDVGraph::ChangeFrameRate()::pIDVSplitter->DiscardAlternateVideoFrames failed."), hr);        
    
    return hr;
}

/*-------------------------------------------------------------------------------
Routine:        CDVGraph::GetResolutionFromDVDecoderPropertyPage
Purpose:        controls discard or not half of the frames in the video stream
Arguments:      None
Returns:        HRESULT
Notes:          For NTSC, the frame rate is reduced from 30 frames per second (fps) to 15 fps. 
                For PAL, the frame rate is reduced from 25 fps to 12.5 fps.
---------------------------------------------------------------------------------*/
HRESULT CDVGraph::GetResolutionFromDVDecoderPropertyPage( HWND hwndApp, BOOL bChangeResolution)
{
    HRESULT hr;
    IBaseFilter *pDVDecoder;
    IIPDVDec    *pIPDVDec;

    /* Obtain the dv docoder's IBaseFilter interface. */
    hr = m_pGraph->FindFilterByName(L"DV Video Decoder", &pDVDecoder) ;
    CHECK_ERROR( TEXT("CDVGraph::GetResolutionFromDVDecoderPropertyPage()::m_pGraph->FindFilterByName failed."), hr);        

    SmartPtr<ISpecifyPropertyPages> pProp;
    hr = pDVDecoder->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
    CHECK_ERROR( TEXT("CDVGraph::GetResolutionFromDVDecoderPropertyPage()::QI ISpecifyPropertyPages failed."), hr);        
    
    // Get the filter's name and IUnknown pointer.
    FILTER_INFO FilterInfo;
    hr = pDVDecoder->QueryFilterInfo(&FilterInfo); 
    CHECK_ERROR( TEXT("CDVGraph::GetResolutionFromDVDecoderPropertyPage()::pDVDecoder->QueryFilterInfo failed."), hr);        

    // Show the page. 
    if(bChangeResolution)
    {
        CAUUID caGUID;
        pProp->GetPages(&caGUID);

        OleCreatePropertyFrame(
            hwndApp,                // Parent window
            0, 0,                   // (Reserved)
            FilterInfo.achName,     // Caption for the dialog box
            1,                      // Number of objects (just the filter)
            (IUnknown **)&pDVDecoder,            // Array of object pointers. 
            caGUID.cElems,          // Number of property pages
            caGUID.pElems,          // Array of property page CLSIDs
            0,                      // Locale identifier
            0, NULL                 // Reserved
        );
        CoTaskMemFree(caGUID.pElems);
    }

    // Clean up.
    FilterInfo.pGraph->Release(); 
   
    hr = pDVDecoder->QueryInterface(IID_IIPDVDec, reinterpret_cast<PVOID *>(&pIPDVDec));
    CHECK_ERROR( TEXT("CDVGraph::GetResolutionFromDVDecoderPropertyPage()::QI IIPDVDec failed."), hr);        

    hr = pIPDVDec->get_IPDisplay(reinterpret_cast <int *>(&m_DVResolution));
    CHECK_ERROR( TEXT("CDVGraph::GetResolutionFromDVDecoderPropertyPage()::pIPDVDec->get_IPDisplay failed."), hr);        
  
    SAFE_RELEASE( pDVDecoder );
    SAFE_RELEASE( pIPDVDec );

    return hr;
}

/*-------------------------------------------------------------------------
Routine:        CDVGraph::GetResolutionFromDVDecoderPropertyPage
Purpose:        controls discard or not half of the frames in the video stream
Arguments:      None
Returns:        HRESULT
Notes:          For NTSC, the frame rate is reduced from 30 frames per second (fps) to 15 fps. 
                For PAL, the frame rate is reduced from 25 fps to 12.5 fps.
------------------------------------------------------------------------*/
HRESULT CDVGraph::GetVideoWindowDimensions (int* pWidth, int *pHeight, BOOL bChangeResolution,HWND hwndApp)
{
    HRESULT hr;

    ASSERT(pWidth != NULL);
    ASSERT(pHeight != NULL);
    if (!pWidth || !pHeight)
        return E_POINTER;

    hr = GetResolutionFromDVDecoderPropertyPage( hwndApp, bChangeResolution );
    CHECK_ERROR( TEXT("CDVGraph::GetVideoWindowDimensions()::GetResolutionFromDVDecoderPropertyPage() failed."), hr); 

    switch (m_DVResolution)
    {
        case DVRESOLUTION_FULL:
            *pWidth = DVENCODER_WIDTH;
            if (DVENCODERVIDEOFORMAT_PAL == m_VideoFormat)
                *pHeight = PAL_DVENCODER_HEIGHT;
            else if (DVENCODERVIDEOFORMAT_NTSC == m_VideoFormat)
                *pHeight = NTSC_DVENCODER_HEIGHT;
            break;

        case DVRESOLUTION_HALF:
            *pWidth = DVENCODER_WIDTH/2;
            if (DVENCODERVIDEOFORMAT_PAL == m_VideoFormat)
                *pHeight = PAL_DVENCODER_HEIGHT/2;
            else if (DVENCODERVIDEOFORMAT_NTSC == m_VideoFormat)
                *pHeight = NTSC_DVENCODER_HEIGHT/2;
            break;

        case DVRESOLUTION_QUARTER:
            *pWidth = DVENCODER_WIDTH/4;
            if (DVENCODERVIDEOFORMAT_PAL == m_VideoFormat)
                *pHeight = PAL_DVENCODER_HEIGHT/4;
            else if (DVENCODERVIDEOFORMAT_NTSC == m_VideoFormat)
                *pHeight = NTSC_DVENCODER_HEIGHT/4;
            break;

        case DVRESOLUTION_DC:
            *pWidth = 88;
            if (DVENCODERVIDEOFORMAT_PAL == m_VideoFormat)
               *pHeight = PAL_DVENCODER_HEIGHT/8;
            else if (DVENCODERVIDEOFORMAT_NTSC == m_VideoFormat)
               *pHeight = NTSC_DVENCODER_HEIGHT/8;
            break;
    }

    return hr;
}

/*-------------------------------------------------------------------------
Routine:      DV_SeekATN
Purpose:      ATN Seek function - uses GetTransportBasicParameters to send RAW AVC command 
Arguments:    None
Returns:      TRUE if successful
Notes:        This is Absolute Track Number Seek not TimeCode Seek but 
              uses the timecode display as input
------------------------------------------------------------------------*/
HRESULT CDVGraph::SeekATN(int iHr, int iMn, int iSc, int iFr)
{
    BOOL bStatus = FALSE;
    HRESULT hr = S_OK;
    ULONG ulTrackNumToSearch;
    long iCnt = 8;

    if (DVENCODERVIDEOFORMAT_PAL == m_VideoFormat && (iFr > 25) )
    {
        Dump(TEXT("Invalid Parameter - Frame should be less than 25 for PAL"));
        return E_FAIL;
    }
    
    if (DVENCODERVIDEOFORMAT_NTSC == m_VideoFormat && (iFr > 30) )
    {
        Dump(TEXT("Invalid Parameter - Frame should be less than 30 for NTSC"));
        return E_FAIL;
    }   
    

    // ATN Seek Raw AVC Command 
    BYTE RawAVCPkt[8] = {0x00, 0x20, 0x52, 0x20, 0xff, 0xff, 0xff, 0xff};

    if ((iHr < 24) && (iHr >= 0) && (iMn < 60) && (iMn >= 0) && (iSc < 60) && (iSc >= 0))
    {
        //Calculate the ATN
        if (m_AvgTimePerFrame == 40) 
        {
            ulTrackNumToSearch = ((iMn * 60 + iSc) * 25 + iFr) * 12 * 2;
        } 
        else 
        {
            // Drop two frame every minutes
            ulTrackNumToSearch = ((iMn * 60 + iSc) * 30 + iFr - ((iMn - (iMn / 10)) * 2)) * 10 * 2;
        }
        // Update the Raw AVC Command query
        RawAVCPkt[4] = (BYTE)  (ulTrackNumToSearch & 0x000000ff);
        RawAVCPkt[5] = (BYTE) ((ulTrackNumToSearch & 0x0000ff00) >> 8);
        RawAVCPkt[6] = (BYTE) ((ulTrackNumToSearch & 0x00ff0000) >> 16);
        
        // RAW AVC Call
        hr = m_pIAMExtTransport->GetTransportBasicParameters(ED_RAW_EXT_DEV_CMD, &iCnt, (LPOLESTR *)RawAVCPkt);     
        if ((HRESULT) ERROR_TIMEOUT == hr)
            OutputDebugString(TEXT(" ATN Seek returns ERROR_TIMEOUT"));
        else if ((HRESULT) ERROR_REQ_NOT_ACCEP == hr)
            OutputDebugString(TEXT(" ATN Seek returns ERROR_REQ_NOT_ACCEP"));
        else if ((HRESULT) ERROR_NOT_SUPPORTED == hr)
            OutputDebugString(TEXT(" ATN Seek returns ERROR_NOT_SUPPORTED"));
        else if ((HRESULT) ERROR_REQUEST_ABORTED == hr)
            OutputDebugString(TEXT(" ATN Seek returns ERROR_REQUEST_ABORTED "));       
    }
    else
    {
        Dump(TEXT("Invalid Parameter - Time entered should be:\nHour:Minute:Second:Frame"));
        hr = E_FAIL;
    } 

    return hr;
} 

// helper function
BOOL IsDeviceOutputDV(IBaseFilter * pFilter) 
{
    if (!pFilter) return FALSE;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    ULONG            ul ;
    BOOL bFound = FALSE;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) return FALSE;

    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))        {
            SAFE_RELEASE(pPin);
            break;
        }

        if (ThisPinDir == PINDIR_OUTPUT)
        {
            IEnumMediaTypes* pTypeEnum;
            hr = pPin->EnumMediaTypes (&pTypeEnum);

            AM_MEDIA_TYPE* pMediaType;
            // Loop thru' media type list for a match
            do {
                hr = pTypeEnum->Next(1, &pMediaType, &ul) ;
                if (FAILED(hr) || 0 == ul) {
                    SAFE_RELEASE(pPin);
                    break ;
                }
        
                if (pMediaType->subtype == MEDIASUBTYPE_dvsd  ||
                    pMediaType->subtype == MEDIASUBTYPE_DVSD) {  
                    bFound = TRUE;
                    SAFE_RELEASE(pPin);
                    DeleteMediaType( pMediaType );
                
                    pTypeEnum->Release();
                    SAFE_RELEASE(pEnum);

                    return TRUE;
                }
                
                DeleteMediaType( pMediaType );

            } while (!bFound) ;  // until the reqd one is found

            pTypeEnum->Release();                      
        } 
    }

    SAFE_RELEASE(pPin);
    SAFE_RELEASE(pEnum);

    // Did not find a matching filter.
    return FALSE;
}