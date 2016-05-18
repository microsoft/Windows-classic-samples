//------------------------------------------------------------------------------
// File: ProgProp.cpp
//
// Desc: DirectShow sample code - implementation of property page class
//       for the PSI parser filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>

#include <streams.h>


#include <bdaiface.h>
#include <bdatypes.h>

#include "resource.h"
#include "iMpeg2PsiParser.h"
#include "progprop.h"
#include "ParserUIDs.h"
#include "PsiParser.h"

#pragma warning(disable: 4702)

void GetStreamTypeDescription(BYTE stream_type, TCHAR* sz);

//
// CreateInstance
//
// This goes in the factory template table to create new filter instances
//
CUnknown * WINAPI CProgramProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

    CProgramProperties *pNewObj = new CProgramProperties(lpunk, phr);

    //doesn't need to check whether new failed -- the caller handles it
    return pNewObj;

} // CreateInstance


//
// Constructor
//
CProgramProperties::CProgramProperties(LPUNKNOWN pUnk, HRESULT *phr)
: CBasePropertyPage(NAME("Program Property Page"), pUnk,IDD_PROGDIALOG, IDS_TITLE)
, m_pProgram(NULL)
, m_stream_id(0)
, m_pat_version(0)
, m_mappedAudPid(0)
, m_mappedVidPid(0)
, m_bAvStreaming(FALSE)
, m_pDemux(NULL)
, m_pGraphBuilder(NULL)
, m_pMediaControl(NULL)
, m_pVideoOutPin(NULL)
, m_pAudioOutPin(NULL)
, m_pIVideoPIDMap(NULL)
, m_pIAudioPIDMap(NULL)
{

} // (Constructor)

/*
//
// Destructor
//
CProgramProperties::~CProgramProperties()
{


} // (Destructor)
*/

//
// SetDirty
//
// Sets m_bDirty and notifies the property page site of the change
//
void CProgramProperties::SetDirty()
{
    m_bDirty = TRUE;

    if(m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

} // SetDirty

//
// PopulateTransportStreamInfo
//
BOOL CProgramProperties::PopulateTransportStreamInfo(  )
{

    TCHAR sz[60];
    HRESULT hr =StringCchPrintf(sz, 60, TEXT("%d\0"), m_stream_id);
    Edit_SetText(GetDlgItem(m_hwndDialog, IDC_StreamIdEDIT), sz);
    hr = StringCchPrintf(sz, 60, TEXT("%d\0"), m_pat_version);
    Edit_SetText(GetDlgItem(m_hwndDialog, IDC_PatVersionEDIT), sz);

    return TRUE;
}

//
//   SetListColumns
//
//
BOOL CProgramProperties::SetListColumns()
{
    LV_COLUMN   lvColumn;
    int         i;
    TCHAR       szString[NUM_OF_PROGRAM_SUBITEM_COLUMN+1][MAX_LEN] = {TEXT("Program Number"), TEXT("PMT PID") };

    //empty the list
    ListView_DeleteAllItems(m_hwndProgList);
    for(i = 0; i < NUM_OF_PROGRAM_SUBITEM_COLUMN; i++)
    {
        ListView_DeleteColumn(m_hwndProgList, 0);
    }

    //initialize the columns
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;

    for(i = 0; i < NUM_OF_PROGRAM_SUBITEM_COLUMN; i++)
    {
        lvColumn.pszText = szString[i];
        lvColumn.iSubItem = i;
        lvColumn.cx = 180;
        if( ListView_InsertColumn(m_hwndProgList, i, &lvColumn) == -1)
            return FALSE;
    }

    return TRUE;
}

//
// AddListViewItems
//
BOOL CProgramProperties::AddListViewItems()
{

    LVITEM lvI;
    INT dIndex;
    TCHAR sz[MAX_LEN];

    for(int i = m_number_of_programs-1; i>=0;i--)
    {

        WORD wProgramNumber;
        WORD wMapPid;
        m_pProgram->GetRecordProgramNumber((DWORD) i, &wProgramNumber);
        m_pProgram->GetRecordProgramMapPid((DWORD) i, &wMapPid);

        // 1. program number
        lvI.mask = LVIF_TEXT|LVIF_IMAGE;
        lvI.iItem = 0;
        lvI.iSubItem = 0;
        HRESULT hr = StringCchPrintf(sz, MAX_LEN, TEXT("%d\0"), wProgramNumber);
        lvI.pszText = sz;
        dIndex = ListView_InsertItem(m_hwndProgList,&lvI);


        // 2. pmt pid
        lvI.mask = TVIF_TEXT;
        lvI.iItem = dIndex;
        lvI.iSubItem = 1;
        hr = StringCchPrintf(sz, MAX_LEN, TEXT("%#x\0"), wMapPid);
        lvI.pszText = sz;
        ListView_SetItem(m_hwndProgList,&lvI);

    }

    return TRUE;
}

//
//   SetListColumns
//
BOOL CProgramProperties::SetEsListColumns()
{
    LV_COLUMN   lvColumn;
    int         i;
    TCHAR       szString[NUM_OF_ES_SUBITEM_COLUMN+1][MAX_LEN] =
                    {TEXT("PID"), TEXT("Stream Type"),TEXT("Contents") };

    //empty the list
    ListView_DeleteAllItems(m_hwndEsList);

    for(i = 0; i < NUM_OF_ES_SUBITEM_COLUMN; i++)
    {
        ListView_DeleteColumn(m_hwndEsList, 0);
    }

    //initialize the columns
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;

    for(i = 0; i < NUM_OF_ES_SUBITEM_COLUMN; i++)
    {
        lvColumn.pszText = szString[i];
        lvColumn.iSubItem = i;
        if( ListView_InsertColumn(m_hwndEsList, i, &lvColumn) == -1)
            return FALSE;
    }

    ListView_SetColumnWidth(m_hwndEsList, 0, 50);
    ListView_SetColumnWidth(m_hwndEsList, 1, 80);
    ListView_SetColumnWidth(m_hwndEsList, 2, 220);

    return TRUE;
}

//
// AddEsListViewItems
//
BOOL CProgramProperties::AddEsListViewItems( WORD program_number)
{
    LVITEM lvI;
    INT dIndex;
    TCHAR sz[MAX_LEN];
    WORD wCountOfESs;

    m_pProgram->GetCountOfElementaryStreams(program_number, &wCountOfESs);

    for(int i = wCountOfESs-1; i>=0;i--)
    {
        WORD wPid;
        BYTE bType;

        m_pProgram->GetRecordElementaryPid((WORD) program_number, (DWORD) i, &wPid);
        m_pProgram->GetRecordStreamType( (WORD) program_number, (DWORD) i, &bType);


        // 1. ES PID
        lvI.mask = LVIF_TEXT|LVIF_IMAGE;
        lvI.iItem = 0;
        lvI.iSubItem = 0;
        HRESULT hr = StringCchPrintf(sz, MAX_LEN,TEXT("%d\0"), wPid);
        lvI.pszText = sz;
        dIndex = ListView_InsertItem(m_hwndEsList,&lvI);


        // 2. ES stream_type
        lvI.mask = TVIF_TEXT;
        lvI.iItem = dIndex;
        lvI.iSubItem = 1;
        hr = StringCchPrintf(sz, MAX_LEN, TEXT("%#x\0"), bType);
        lvI.pszText = sz;
        ListView_SetItem(m_hwndEsList,&lvI);


        // 3. description
        GetStreamTypeDescription(bType, sz, MAX_LEN);

        lvI.mask = TVIF_TEXT;
        lvI.iItem = dIndex;
        lvI.iSubItem = 2;
        lvI.pszText = sz;
        ListView_SetItem(m_hwndEsList,&lvI);
    }

    return TRUE;
}

//
//OnRefreshProgram
//
BOOL CProgramProperties::OnRefreshProgram()
{
    BOOL bResult = FALSE;

    if(!OnUpdate())
        return FALSE;


    if(m_pDemux != NULL && m_pGraphBuilder != NULL){

            if(m_number_of_programs != 0){
            PopulateTransportStreamInfo();
            SetListColumns();
            AddListViewItems();
            SetEsListColumns();
        }
    }

    return TRUE;

} //OnRefreshProgram


BOOL CProgramProperties::OnUpdate()
{

    // if there is no streaming, the following variables will not be initialized.
    if(m_pDemux != NULL && m_pGraphBuilder != NULL){
        // Get the initial Program value
        m_pProgram->GetTransportStreamId( &m_stream_id);
        m_pProgram->GetPatVersionNumber( &m_pat_version);
        m_pProgram->GetCountOfPrograms( &m_number_of_programs );
    }
    return TRUE;
}

//
// OnReceiveMessage
//
// Virtual method called by base class with Window messages
//
INT_PTR CProgramProperties::OnReceiveMessage(HWND hwnd,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            m_hwndDialog = hwnd;
            PopulateTransportStreamInfo( );

            m_hwndProgList = GetDlgItem (hwnd, IDC_PROGLIST) ;
            ASSERT(m_hwndProgList);

            m_hwndEsList = GetDlgItem (hwnd, IDC_ES_LIST) ;
            ASSERT(m_hwndEsList);

            // populate program list
            SetListColumns();
            AddListViewItems();

            // set elementary stream list
            SetEsListColumns();

            return TRUE;
        }

        case WM_DESTROY:
        {
            DestroyWindow(m_hwndDialog);
            return TRUE;
        }

        case WM_COMMAND:
        {

            switch (LOWORD (wParam)) {

                case IDC_VIEWBUTTON :
                    OnViewProgram () ;
                    m_bAvStreaming = TRUE;
                    break ;

                case IDC_REFRESHBUTTON :
                    OnRefreshProgram () ;
                    break ;

                case IDC_STOPBUTTON :
                    if(m_bAvStreaming == TRUE){
                        UnmapAvPIDs();
                        m_bAvStreaming = FALSE;
                    }
                    break ;
            };
            return TRUE;
        }

        case WM_NOTIFY:
        {
            switch (wParam)
            {
                 case IDC_PROGLIST:

                    LPNM_LISTVIEW   pnlv = (LPNM_LISTVIEW)lParam;
                    BOOL itemSelected = FALSE;
                    WORD program_number;
                    UINT ii;

                    switch (pnlv->hdr.code)
                    {
                        case NM_CLICK:

                            ii = ListView_GetItemCount(m_hwndProgList);
                            for(; ii; ii--)
                            {
                                if(0!= ListView_GetItemState(
                                        m_hwndProgList,
                                        ii-1,
                                        LVIS_SELECTED))
                                break;
                            }


                            TCHAR szTmp[MAX_LEN];
                            ListView_GetItemText( m_hwndProgList,
                                                  ii-1,
                                                  0, // first column
                                                  szTmp,
                                                  MAX_LEN);
                            program_number = (WORD) _wtoi(szTmp);
                            Dump1(TEXT("selected program number = %s\n"), szTmp);
                            Dump1(TEXT("selected program number = %#x\n"), program_number);

                            SetEsListColumns();
                            AddEsListViewItems( program_number);

                            break;

                        default:
                            break;
                    }

                    return TRUE;

            } // end switch (wParam)
        }

        default:
            return FALSE;
    }

    return TRUE;

} // OnReceiveMessage

//
//OnViewProgram
//
BOOL CProgramProperties::OnViewProgram ()
{
    WORD audPid;
    WORD vidPid;
    if(!GetSelectedPids(&audPid, &vidPid)){
        return FALSE;
    }

    if(audPid == 0 || vidPid == 0){
        MBOX(TEXT("PIDs of the selected program are not valid!"));
        return FALSE;
    }

    // if no aud or vid output pins of demux exist, create, map and render them
    if(!AvPinsExisted())
    {
        if(!CreateAndRenderAvOutPins()){
            return FALSE;
        }

        m_mappedAudPid = audPid;
        m_mappedVidPid = vidPid;

        if(MapAvPIDs()){
            m_pMediaControl->Run();
        }
        else{
            return FALSE;
        }
    }
    else{
        if(PidsChanged(audPid, vidPid)){
            // AV pins have been unmapped if "stop view" button was clicked
            if(m_bAvStreaming == TRUE){
                if(!UnmapAvPIDs())
                    return FALSE;
            }

            m_mappedAudPid = audPid;
            m_mappedVidPid = vidPid;

            if(MapAvPIDs()){
                m_pMediaControl->Run();
            }
            else
                return FALSE;
        }
        else // the same pids
        {
            MapAvPIDs();
            m_pMediaControl->Run();
        }
    }

    return TRUE;

} //OnViewProgram


//
//GetSelectedPids
//

BOOL CProgramProperties::GetSelectedPids(WORD* pulAudPID, WORD* pulVidPID)
{
    // get the audio PID
    BOOL bTranslated;
    *pulAudPID = (WORD) GetDlgItemInt (m_hwndDialog, IDC_EDIT_AUDIO, &bTranslated, TRUE);
    if (!bTranslated)
    {
        MBOX (TEXT("Select audio PID from Elementary Stream List and type it in the audio PID edit box"));
        return FALSE;
    }

    // get the video PID
    bTranslated;
    *pulVidPID = (WORD) GetDlgItemInt (m_hwndDialog, IDC_EDIT_VIDEO, &bTranslated, TRUE);
    if (!bTranslated)
    {
        MBOX (TEXT("Select video PID from Elementary Stream List and type it in the video PID edit box"));
        return FALSE;
    }

    return TRUE;

} //GetSelectedPids

//
// AvPinsExisted()
//
BOOL CProgramProperties::AvPinsExisted()
{
    if(m_pVideoOutPin  != NULL && m_pAudioOutPin != NULL)
        return TRUE;

    return FALSE;

} //AvPinsExisted

//
//CreatAvOutPins
//
BOOL CProgramProperties::CreateAndRenderAvOutPins()
{
    HRESULT hr = S_OK;
    RETURN_FALSE_IF_BADPTR( TEXT("CProgramProperties::SetupMPEGDeMux : pDemuxDev is null "), m_pDemux);

    // Query the interface to create pins on the demux
    IMpeg2Demultiplexer * pIMpeg2Demux ;
    hr = m_pDemux->QueryInterface(IID_IMpeg2Demultiplexer, (void **) &pIMpeg2Demux) ;
    RETURN_FALSE_IF_FAILED(TEXT("CProgramProperties::SetupMPEGDeMux : create pIMpeg2Demux failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CProgramProperties::SetupMPEGDeMux :pIMpeg2Demux is NULL"), pIMpeg2Demux);

    //video:
    // Setting the media type for the pins to be created
    AM_MEDIA_TYPE amTypeVideo;
    amTypeVideo.majortype = MEDIATYPE_Video;
    amTypeVideo.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
    amTypeVideo.bFixedSizeSamples = TRUE;
    amTypeVideo.bTemporalCompression = 0;
    amTypeVideo.formattype = FORMAT_MPEG2Video;
    amTypeVideo.pUnk = NULL;
    amTypeVideo.cbFormat = sizeof(g_Mpeg2ProgramVideo);
    amTypeVideo.pbFormat = g_Mpeg2ProgramVideo;

    // Create the pins
    hr = pIMpeg2Demux->CreateOutputPin(&amTypeVideo, L"MpegVideo", &m_pVideoOutPin);
    RETURN_FALSE_IF_FAILED(TEXT(" CProgramProperties::SetupMPEGDeMux():: Creating the Video Output Pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CProgramProperties::SetupMPEGDeMux()::pVideoOutpin is null "), m_pVideoOutPin) ;


    // for audio: could be Mpeg1, Mpeg2, AC3:
    AM_MEDIA_TYPE amTypeAudio;
    amTypeAudio.majortype = MEDIATYPE_Audio;
    amTypeAudio.bFixedSizeSamples = TRUE;
    amTypeAudio.bTemporalCompression = 0;
    amTypeAudio.formattype = FORMAT_WaveFormatEx;
    amTypeAudio.pUnk = NULL;
    amTypeAudio.cbFormat = sizeof g_MPEG1AudioFormat;
    amTypeAudio.pbFormat = g_MPEG1AudioFormat;

    // put the graph to stop state, in order to render the pins
    hr = m_pMediaControl->Stop();

    hr = m_pGraphBuilder->Render(m_pVideoOutPin);
    if(FAILED(hr))
    {
        hr = pIMpeg2Demux->DeleteOutputPin(L"MpegAudio");
        RETURN_FALSE_IF_FAILED(TEXT(" CProgramProperties::SetupMPEGDeMux():: render the video Output Pin failed %X"), hr);
    }

    //create audio output pin ( must use L"MpegAudio", will be used in pid finder filter to find this pin
    BOOL bTryAgain = TRUE;
    const GUID* guidStr[] = {   & MEDIASUBTYPE_MPEG1Payload, &MEDIASUBTYPE_MPEG2_AUDIO, &MEDIASUBTYPE_DOLBY_AC3 };
    int i;

    for (i = 0; i<3; i++ )
    {
        amTypeAudio.subtype = *(guidStr[i]);//can be Mpeg1, MPeg2 or AC3

        hr = pIMpeg2Demux->CreateOutputPin(&amTypeAudio, L"MpegAudio", &m_pAudioOutPin);
        RETURN_FALSE_IF_FAILED(TEXT(" CProgramProperties::SetupMPEGDeMux():: Creating the Audio Output Pin failed %X"), hr);
        RETURN_FALSE_IF_BADPTR(TEXT("CProgramProperties::SetupMPEGDeMux()::pAudioOutpin is null "), m_pAudioOutPin) ;

        hr = m_pGraphBuilder->Render(m_pAudioOutPin);
        if(SUCCEEDED(hr))
            break;
        else
        {
            hr = pIMpeg2Demux->DeleteOutputPin(L"MpegAudio");
            RETURN_FALSE_IF_FAILED(TEXT(" CProgramProperties::SetupMPEGDeMux():: Delete Audio Output Pin on the Demux Filter Failed %X"), hr);
        }
    }
    // after rendering the pins, switch to run state:
    hr = m_pMediaControl->Run();

    hr = m_pVideoOutPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &m_pIVideoPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: QI the IMPEG2PIDMap inf on video out pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::SetupMPEGDeMux()::pIVideoPIDMap is null "), m_pIVideoPIDMap) ;

    hr = m_pAudioOutPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &m_pIAudioPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: QI the IMPEG2PIDMap inf on audio put pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::SetupMPEGDeMux()::pIAudioPIDMap is null "), m_pIAudioPIDMap) ;

    pIMpeg2Demux->Release();

    return TRUE;

} //CreatAvOutPins

//
//MapRenderAvOutPins
//  maps the PIDs to the pins
//
BOOL CProgramProperties::MapAvPIDs()
{
    HRESULT hr;
    ULONG ulVideoPID[1] = {(ULONG) m_mappedVidPid};

    hr = m_pIVideoPIDMap->MapPID(1, ulVideoPID, MEDIA_ELEMENTARY_STREAM);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: Mapping the PID to the Video Output Pin on the Demux Filter Failed %X"), hr);

    ULONG ulAudioPID[1] = {(ULONG) m_mappedAudPid};
    hr = m_pIAudioPIDMap->MapPID(1, ulAudioPID, MEDIA_ELEMENTARY_STREAM);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: Mapping the PID to the Audio Output Pin on the Demux Filter Failed %X"), hr);

    return TRUE;

} //MapRenderAvOutPins

//
//PidsChanged
//
BOOL CProgramProperties::PidsChanged(WORD audPid, WORD vidPid)
{
    if( audPid == m_mappedAudPid && vidPid == m_mappedVidPid)
        return FALSE;

    return TRUE;

} //PidsChanged


//
//MapRenderAvOutPins
//

BOOL CProgramProperties::UnmapAvPIDs()
{
    HRESULT hr;

    ULONG ulAudioPID[1] = {(ULONG) m_mappedAudPid};
    hr = m_pIAudioPIDMap->UnmapPID(1, ulAudioPID);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: Unmapping the PID of the Audio Output Pin on the Demux Filter Failed %X"), hr);

    ULONG ulVideoPID[1] = {(ULONG) m_mappedVidPid};
    hr = m_pIVideoPIDMap->UnmapPID(1, ulVideoPID);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: Unmapping the PID of the Video Output Pin on the Demux Filter Failed %X"), hr);

    return TRUE;

} //MapRenderAvOutPins


//
// OnConnect
//
// Called when the property page connects to a filter
//
HRESULT CProgramProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pProgram == NULL);
    CheckPointer(pUnknown,E_POINTER);

    HRESULT hr = pUnknown->QueryInterface(IID_IMpeg2PsiParser, (void **) &m_pProgram);
    if(FAILED(hr))
    {
        return E_NOINTERFACE;
    }
    ASSERT(m_pProgram);

    IBaseFilter * pParserFilter ;
    hr = m_pProgram->QueryInterface(IID_IBaseFilter, (void **) &pParserFilter);
    RETURN_FALSE_IF_FAILED(TEXT("CProgramProperties::OnUpdate() QueryInterface() failed. %X"), hr);

    FILTER_INFO Info;
    IFilterGraph * pGraph;
    hr = pParserFilter->QueryFilterInfo(&Info);

    RETURN_FALSE_IF_FAILED(TEXT("CProgramProperties::OnUpdate() QueryFilterInfo() failed. %X"), hr);
    pGraph = Info.pGraph;
    pParserFilter->Release();

    hr = pGraph->QueryInterface(IID_IGraphBuilder, (void **) & m_pGraphBuilder);
    RETURN_FALSE_IF_FAILED(TEXT("CProgramProperties::OnUpdate() QueryInterface() failed. %X"), hr);

    // get demux filter
    hr = GetDemuxFilter(pGraph, &m_pDemux);
    pGraph->Release();

    // if there is no streaming, the following variables will not be initialized.
    if(m_pDemux != NULL && m_pGraphBuilder != NULL){
        hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **) & m_pMediaControl);
        RETURN_FALSE_IF_FAILED( TEXT(" CProgramProperties::OnUpdate():Failed to QI IMediaControl. %X"), hr);

        // Get the initial Program value
        m_pProgram->GetTransportStreamId( &m_stream_id);
        m_pProgram->GetPatVersionNumber( &m_pat_version);
        m_pProgram->GetCountOfPrograms( &m_number_of_programs );

    }

    if(!OnUpdate())
        return FALSE;

    return NOERROR;
}


//
HRESULT CProgramProperties::OnDisconnect()
{
    // Release of Interface after setting the appropriate Program value
    if (!m_pProgram)
        return E_UNEXPECTED;

    SAFE_RELEASE(m_pDemux);
    SAFE_RELEASE(m_pGraphBuilder);
    SAFE_RELEASE(m_pMediaControl);
    SAFE_RELEASE(m_pVideoOutPin);
    SAFE_RELEASE(m_pAudioOutPin);
    SAFE_RELEASE(m_pIVideoPIDMap);
    SAFE_RELEASE(m_pIAudioPIDMap);

    SAFE_RELEASE(m_pProgram);

    return NOERROR;

} // OnDisconnect


//
// OnDeactivate
//
// We are being deactivated
//
HRESULT CProgramProperties::OnDeactivate(void)
{
    // Remember the present values for the next activate

    m_pProgram->GetTransportStreamId(&m_stream_id);
    m_pProgram->GetPatVersionNumber(&m_pat_version);
    m_pProgram->GetCountOfPrograms( &m_number_of_programs );

    return NOERROR;

} // OnDeactivate


//
// OnApplyChanges
//
//
HRESULT CProgramProperties::OnApplyChanges()
{
    m_pProgram->GetTransportStreamId(&m_stream_id);
    m_pProgram->GetPatVersionNumber(&m_pat_version);
    m_pProgram->GetCountOfPrograms( &m_number_of_programs );

    m_bDirty = FALSE;
    return(NOERROR);

} // OnApplyChanges

void CProgramProperties::GetStreamTypeDescription(BYTE stream_type, TCHAR* sz, size_t len)
{
    HRESULT hr;
    switch(stream_type)
    {
        case 0x00:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ITU-T|ISO/IEC reserved"));
            break;

        case 0x01:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 11172-2 Video"));
            break;

        case 0x02:
            hr = StringCchPrintf(sz, len, TEXT("ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream"));
            break;

        case 0x03:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 11172-3 Audio"));
            break;

        case 0x04:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-3 Audio"));
            break;

        case 0x05:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ITU-T Rec. H.222.0|ISO/IEC 13818-1 private_sections"));
            break;

        case 0x06:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ITU-T Rec. H.222.0|ISO/IEC 13818-1 PES packets containing private data"));
            break;

        case 0x07:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13522 MHEG"));
            break;

        case 0x08:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("Annex A-DSM CC"));
            break;

        case 0x09:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ITU-T Rec. H.222.1"));
            break;

        case 0x0A:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-6 type A"));
            break;

        case 0x0B:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-6 type B"));
            break;

        case 0x0C:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-6 type C"));
            break;

        case 0x0D:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-6 type D"));
            break;

        case 0x0E:
            hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ISO/IEC 13818-1 auxiliary"));
            break;

        default:

            if((stream_type >= 0x0F && stream_type <= 0x7F) )
                hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("ITU-T Rec. H.222.0|ISO/IEC 13818-1 reserved"));
            else if(stream_type >= 0x80 && stream_type <= 0xFF)
                hr = StringCchPrintf(sz, len, TEXT("%s"), TEXT("User private"));
    }

    return;
}

