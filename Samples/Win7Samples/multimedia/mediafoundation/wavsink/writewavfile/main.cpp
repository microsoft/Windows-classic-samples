#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <stdio.h>

#include "..\\WavSink\\CreateWavSink.h"

#define USE_LOGGING
#include "common.h"
using namespace MediaFoundationSamples;


// Forward declares

HRESULT CreateWavFile(const WCHAR *sURL, const WCHAR *sOutputFile);

HRESULT CreateTopology(IMFMediaSource *pSource, IMFMediaSink *pSink, IMFTopology **ppTopology);

HRESULT CreateTopologyBranch(
    IMFTopology *pTopology,
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFMediaSink *pSink
    );

HRESULT CreateSourceNode(
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode          // Receives the node pointer.
    );

HRESULT CreateOutputNode(IMFMediaSink *pSink, DWORD iStream, IMFTopologyNode **ppNode);

HRESULT RunMediaSession(IMFTopology *pTopology);


///////////////////////////////////////////////////////////////////////
//  Name: wmain
//  Description:  Entry point to the application.
//  
//  Usage: writewavfile.exe inputfile outputfile
///////////////////////////////////////////////////////////////////////

int wmain(int argc, wchar_t *argv[ ])
{
    HRESULT hr;

    TRACE_INIT();

    if (argc != 3)
    {
        wprintf(L"Usage: WriteWavFile.exe InputFile OuputFile\n");
        return -1;
    }

    hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
        wprintf(L"MFStartup failed!\n");
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateWavFile(argv[1],argv[2]);

        if (SUCCEEDED(hr))
        {
            wprintf(L"Done!\n");
        }
        else
        {
            wprintf(L"Error: Unable to author file.\n");
        }
    }

    MFShutdown();

    TRACE_CLOSE();

    return 0;
}




///////////////////////////////////////////////////////////////////////
//  Name: CreateWavFile
//  Description:  Creates a .wav file from an input file.
///////////////////////////////////////////////////////////////////////

HRESULT CreateWavFile(const WCHAR *sURL, const WCHAR *sOutputFile)
{
    IMFByteStream *pStream = NULL;
    IMFMediaSink *pSink = NULL;
    IMFMediaSource *pSource = NULL;
    IMFTopology *pTopology = NULL;

    HRESULT hr = MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, sOutputFile, &pStream);
    if (FAILED(hr))
    {
        wprintf(L"MFCreateFile failed!\n");
    }

    // Create the WavSink object.
    if (SUCCEEDED(hr))
    {
        hr = CreateWavSink(pStream, &pSink);
    }

    // Create the media source from the URL.
    if (SUCCEEDED(hr))
    {
        hr = CreateMediaSource(sURL, &pSource);
    }

    // Create the topology.
    if (SUCCEEDED(hr))
    {
        hr = CreateTopology(pSource, pSink, &pTopology);
    }

    // Run the media session.
    if (SUCCEEDED(hr))
    {
        hr = RunMediaSession(pTopology);
        if (FAILED(hr))
        {
            wprintf(L"RunMediaSession failed!\n");
        }
    }

    if (pSource)
    {
        pSource->Shutdown();
    }

    SAFE_RELEASE(pStream);
    SAFE_RELEASE(pSink);
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pTopology);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: RunMediaSession
//  Description:  
//  Queues the specified topology on the media session and runs the
//  media session until the MESessionEnded event is received.
///////////////////////////////////////////////////////////////////////

HRESULT RunMediaSession(IMFTopology *pTopology)
{
    IMFMediaSession *pSession = NULL;

    HRESULT hr = S_OK;
    BOOL bGetAnotherEvent = TRUE;
    PROPVARIANT varStartPosition;

    PropVariantInit(&varStartPosition);

    hr = MFCreateMediaSession(NULL, &pSession);

    if (SUCCEEDED(hr))
    {
        hr = pSession->SetTopology(0, pTopology);
    }

    while (bGetAnotherEvent)
    {
        HRESULT hrStatus = S_OK;
        IMFMediaEvent *pEvent = NULL;
        MediaEventType meType = MEUnknown;

        MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    
    
        hr = pSession->GetEvent(0, &pEvent);

        if (SUCCEEDED(hr))
        {
            hr = pEvent->GetStatus(&hrStatus);
        }

        if (SUCCEEDED(hr))
        {
            hr = pEvent->GetType(&meType);
        }

        if (SUCCEEDED(hr) && SUCCEEDED(hrStatus))
        {
            switch (meType)
            {

            case MESessionTopologySet:
                wprintf(L"MESessionTopologySet\n");
                break;


        case MESessionTopologyStatus:
                // Get the status code.
                hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus);
                if (SUCCEEDED(hr))
                {
                    switch (TopoStatus)
                    {
                    case MF_TOPOSTATUS_READY:
                        wprintf(L"MESessionTopologyStatus: MF_TOPOSTATUS_READY\n");
                        hr = pSession->Start(&GUID_NULL, &varStartPosition);
                        break;

                    case MF_TOPOSTATUS_ENDED:
                        wprintf(L"MESessionTopologyStatus: MF_TOPOSTATUS_ENDED\n");
                        break;

                    }
                }
                break;

            case MESessionStarted:
                wprintf(L"MESessionStarted\n");
                break;

            case MESessionEnded:
                wprintf(L"MESessionEnded\n");
                hr = pSession->Stop();
                break;

            case MESessionStopped:
                wprintf(L"MESessionStopped.\n");
                hr = pSession->Close();
                break;

            case MESessionClosed:
                wprintf(L"MESessionClosed\n");
                bGetAnotherEvent = FALSE;
                break;

            default:
                wprintf(L"Media session event: %d\n", meType);
                break;
            }
        }

        SAFE_RELEASE(pEvent);

        if (FAILED(hr) || FAILED(hrStatus))
        {
            bGetAnotherEvent = FALSE;
        }

    }

    wprintf(L"Shutting down the media session.\n");
    pSession->Shutdown();

    PropVariantClear(&varStartPosition);

    SAFE_RELEASE(pSession);
    return hr;

}




///////////////////////////////////////////////////////////////////////
//  Name: CreateTopology
//  Description:  Creates the topology.
// 
//  Note: The first audio stream is conntected to the media sink.
//        Other streams are deselected.
///////////////////////////////////////////////////////////////////////

HRESULT CreateTopology(IMFMediaSource *pSource, IMFMediaSink *pSink, IMFTopology **ppTopology)
{
    IMFTopology *pTopology = NULL;
    IMFPresentationDescriptor *pPD = NULL;
    IMFStreamDescriptor *pSD = NULL;

    HRESULT hr = S_OK;
    DWORD cStreams = 0;

    hr = MFCreateTopology(&pTopology);

    if (SUCCEEDED(hr))
    {
        hr = pSource->CreatePresentationDescriptor(&pPD);
    }
    if (SUCCEEDED(hr))
    {
        hr = pPD->GetStreamDescriptorCount(&cStreams);
    }

    BOOL fConnected = FALSE;

    if (SUCCEEDED(hr))
    {
        GUID majorType = GUID_NULL;
        BOOL fSelected = FALSE;

        for (DWORD iStream = 0; iStream < cStreams; iStream++)
        {
            hr = pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD);

            if (FAILED(hr))
            {
                break;
            }

            // If the stream is not selected by default, ignore it.
            if (!fSelected)
            {
                continue;
            }

            // Get the major media type.
            hr = GetStreamMajorType(pSD, &majorType);
            if (FAILED(hr))
            {
                break;
            }

            // If it's not audio, deselect it and continue.
            if (majorType != MFMediaType_Audio)
            {
                // Deselect this stream
                hr = pPD->DeselectStream(iStream);

                if (FAILED(hr))
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            
            // It's an audio stream, so try to create the topology branch.
            hr = CreateTopologyBranch(pTopology, pSource, pPD, pSD, pSink);

            // Set our status flag. 
            if (SUCCEEDED(hr))
            {
                fConnected = TRUE;
            }

            // At this point we have reached the first audio stream in the
            // source, so we can stop looking (whether we succeeded or failed).
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        // Even if we succeeded, if we didn't connect any streams, it's a failure.
        // (For example, it might be a video-only source.
        if (!fConnected)
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppTopology = pTopology;
        (*ppTopology)->AddRef();
    }

    SAFE_RELEASE(pTopology);
    SAFE_RELEASE(pPD);
    SAFE_RELEASE(pSD);

    return hr;

}


//////////////////////////////////////////////////////////////////////
//  Name: CreateSourceNode
//  Creates a source node for a media stream. 
//
//  pSource:   Pointer to the media source.
//  pSourcePD: Pointer to the source's presentation descriptor.
//  pSourceSD: Pointer to the stream descriptor.
//  ppNode:    Receives the IMFTopologyNode pointer.
///////////////////////////////////////////////////////////////////////

HRESULT CreateSourceNode(
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode          // Receives the node pointer.
    )
{
    IMFTopologyNode *pNode = NULL;
    HRESULT hr = S_OK;
    
    // Create the node.
    hr = MFCreateTopologyNode(
        MF_TOPOLOGY_SOURCESTREAM_NODE, 
        &pNode);

    // Set the attributes.
    if (SUCCEEDED(hr))
    {
        hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
    }

    if (SUCCEEDED(hr))
    {
        hr = pNode->SetUnknown(
            MF_TOPONODE_PRESENTATION_DESCRIPTOR, 
            pPD);
    }

    if (SUCCEEDED(hr))
    {
        hr = pNode->SetUnknown(
            MF_TOPONODE_STREAM_DESCRIPTOR, 
            pSD);
    }

    // Return the pointer to the caller.
    if (SUCCEEDED(hr))
    {
        *ppNode = pNode;
        (*ppNode)->AddRef();
    }
    SAFE_RELEASE(pNode);
    return hr;
}



///////////////////////////////////////////////////////////////////////
//  Name: CreateTopologyBranch
//  Description:  Adds a source and sink to the topology and
//                connects them.
//
//  pTopology: The topology.
//  pSource:   The media source.
//  pPD:       The source's presentation descriptor.
//  pSD:       The stream descriptor for the stream.
//  pSink:     The media sink.
//
///////////////////////////////////////////////////////////////////////

HRESULT CreateTopologyBranch(
    IMFTopology *pTopology,
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFMediaSink *pSink
    )
{

    IMFTopologyNode *pSourceNode = NULL;
    IMFTopologyNode *pOutputNode = NULL;
    
    HRESULT hr = S_OK;

    hr = CreateSourceNode(pSource, pPD, pSD, &pSourceNode);

    if (SUCCEEDED(hr))
    {
        hr = CreateOutputNode(pSink, 0, &pOutputNode);
    }

    if (SUCCEEDED(hr))
    {
        hr = pTopology->AddNode(pSourceNode);
    }

    if (SUCCEEDED(hr))
    {
        hr = pTopology->AddNode(pOutputNode);
    }

    if (SUCCEEDED(hr))
    {
        hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
    }


    SAFE_RELEASE(pSourceNode);
    SAFE_RELEASE(pOutputNode);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: CreateOutputNode
//  Description:  Creates an output node for a stream sink.
//
//  pSink:     The media sink.
//  iStream:   Index of the stream sink on the media sink.
//  ppNode:    Receives a pointer to the topology node.
///////////////////////////////////////////////////////////////////////

HRESULT CreateOutputNode(IMFMediaSink *pSink, DWORD iStream, IMFTopologyNode **ppNode)
{
    IMFTopologyNode *pNode = NULL;
    IMFStreamSink *pStream = NULL;

    HRESULT hr = S_OK;

    hr = pSink->GetStreamSinkByIndex(iStream, &pStream);

    if (SUCCEEDED(hr))
    {
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = pNode->SetObject(pStream);
    }

    if (SUCCEEDED(hr))
    {
        *ppNode = pNode;
        (*ppNode)->AddRef();
    }

    SAFE_RELEASE(pNode);
    SAFE_RELEASE(pStream);
    
    return hr;
}
