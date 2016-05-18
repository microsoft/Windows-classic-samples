//------------------------------------------------------------------------------
// File: PsiParser.cpp
//
// Desc: DirectShow sample code -
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

// Summary
//
// The parser filter works with MPEG2 demux and parses the psi sections in the
// transport stream received from the demux. It has a property page to display
// the MPEG2 program information it parsed. To view a program, the user can
// enter audio and video pids, then the audio and video output pins of the Demux
// will be created and mapped, and a preview graph will be built and run.

// Implementation
//
// The parser filter has one input pin, which owns two objects (CPatProcessor
// and CPmtProcessor) to process pat section and pmt section.

// Note:
//
// Property page is for setting/getting local/private data/parameters and is not
// meant for graph control/modification. The property page was used in this sample
// for the purpose of convenience, to keep the sample simple, and is not a
// recommended practice.

// Demonstration instructions
//
// Preview MPEG2 transport stream from capture device (1394 Mpeg2 camcorder, D-VHS)
/*
a.  plug in 1394 Mpeg2 AV/C device;
b.  Install Mpeg2 software decoder
c.  register proppage.dll and PsiParser.ax
d.  Open graph editor and insert the capture device from Capture Source;
    insert Mpeg2 demux and PsiParser filter from DirectShow filter
e.  Connect capture filter and Mpeg2 demux
f.  Use Mpeg2 Demux property page to create one PSI output Pin with PID=0x0000:
    (under "Output Pin", enter new output pin name "PSI", select MPEG-2PSI as Media
    Type and click "Create". Then go to "PID Mappings", select PID = 0x00000 as PID
    and PSI as PIN, and select MPEG2 PSI Sections, then click "Map" button.)
g.  Refresh the graph to see the output PSI pin; connect it to PSI parser filter
h.  Play the capture device and run the graph
i.  Open the property page of PSI Parser, and click Refresh button
j.  Select a program by clicking on a program number, notice the information of
    the elementary streams contained in the selected program will be displayed below
k.  Enter audio and video PID of a program, and click "View Program" to play the
    A/V from the selected program
l.  Stop viewing by clicking "Stop Viewing" button.
*/

#include <streams.h>
#include <initguid.h>

#include <bdaiface.h>
#include <bdatypes.h>

#include "dbg.h"
#include "psiobj.h"
#include "iMpeg2PsiParser.h"
#include "PsiParser.h"
#include "ParserUIDs.h"
#include "progprop.h"
#include "resource.h"


// {78C4E641-0ED1-11d3-BC6B-00A0C90D63B5}
DEFINE_GUID(MEDIATYPE_MPEG2PSI,
0x78c4e641, 0xed1, 0x11d3, 0xbc, 0x6b, 0x0, 0xa0, 0xc9, 0xd, 0x63, 0xb5);

// Prevent "already defined" linker error problem with Visual C++ 6.0.
// Copy the Microsoft MEDIATYPE_MPEG2_SECTIONS GUID into a renamed GUID
// that will not clash with strmiids.lib.  This is a workaround for
// machines with Visual C++ 6.0 without a newer Platform SDK.
DEFINE_GUID( MY_MEDIATYPE_MPEG2_SECTIONS,
    0x455f176c, 0x4b06, 0x47ce, 0x9a, 0xef, 0x8c, 0xae, 0xf7, 0x3d, 0xf7, 0xb5);


// Setup data
const AMOVIESETUP_FILTER sudPsiParser =
{
    &CLSID_PsiParser,               // Filter CLSID
    L"PSI Parser",                  // String name
    MERIT_DO_NOT_USE,               // Filter merit
    0,
    NULL
};


// List of class IDs and creator functions for class factory
CFactoryTemplate g_Templates [2]  = {
    { L"PSI Parser"
    , &CLSID_PsiParser
    , CPsiParserFilter::CreateInstance
    , NULL
    , &sudPsiParser }
,
    { L"Program Property Page"
    , &CLSID_ProgramPropertyPage
    , CProgramProperties::CreateInstance
    , NULL
    , NULL }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// CreateInstance
//
// This goes in the factory template table to create new instances
//
CUnknown * WINAPI CPsiParserFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CPsiParserFilter(pUnk, phr);

} // CreateInstance

//
// Constructor
//
// Create the filter, and input pin
//

CPsiParserFilter::CPsiParserFilter(LPUNKNOWN pUnk,
                           HRESULT *phr)
: CBaseFilter(NAME("CPSIParser"), pUnk, &m_Lock, CLSID_PsiParser)

{
    ASSERT(phr);

    // Create the single input pin
    m_pInputPin = new CPsiParserInputPin(   this,
                                        GetOwner(),
                                        &m_Lock,
                                        &m_ReceiveLock,
                                        phr);
    if(m_pInputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

} // (Constructor)


//
// Destructor
//
CPsiParserFilter::~CPsiParserFilter()
{
    ASSERT(m_pInputPin);
    delete m_pInputPin;
    m_pInputPin = NULL;

} // (Destructor)

//
// NonDelegatingQueryInterface
//
// Reveals IMpeg2PsiParser and ISpecifyPropertyPages
//
STDMETHODIMP CPsiParserFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IMpeg2PsiParser) {
        return GetInterface((IMpeg2PsiParser *) this, ppv);

    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);

    }

    else {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface

//
// GetPages
//
// This is the sole member of ISpecifyPropertyPages
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CPsiParserFilter::GetPages(CAUUID *pPages)
{
    if(pPages == NULL)
        return E_POINTER;

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if(pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_ProgramPropertyPage;
    return NOERROR;

} // GetPages
//
// GetPinCount
//
// Return the number of input pins we support
//
int CPsiParserFilter::GetPinCount()
{
    return 1;

} // GetPinCount


//
// GetPin
//
// Return our single input pin - not addrefed
//
CBasePin *CPsiParserFilter::GetPin(int n)
{
    // We only support one input pin and it is numbered zero

    ASSERT(n == 0);
    if(n != 0)
    {
        return NULL;
    }

    return m_pInputPin;

} // GetPin

//
// GetTransportStreamId
//
STDMETHODIMP CPsiParserFilter::GetTransportStreamId(WORD *pstreamId)
{
    if(!pstreamId)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);

    CPATProcessor*   pPatProcessor;
    HRESULT hr = m_pInputPin->GetPatProcessor(&pPatProcessor);
    if(hr == NOERROR)
        *pstreamId = (WORD) pPatProcessor->m_current_transport_stream_id;

    return NOERROR;
} // GetTransportStreamId

//
// GetPatVersionNumber
//
STDMETHODIMP CPsiParserFilter::GetPatVersionNumber(BYTE *pPatVersion)
{
    if(!pPatVersion)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);


    CPATProcessor*   pPatProcessor;
    HRESULT hr = m_pInputPin->GetPatProcessor(&pPatProcessor);
    if(hr == NOERROR)
        *pPatVersion = (BYTE) pPatProcessor->m_current_pat_version_number;

    return NOERROR;
}// GetPatVersionNumber

//
// GetCountOfPrograms
//
STDMETHODIMP CPsiParserFilter::GetCountOfPrograms(int *pNumOfPrograms)
{
    if(!pNumOfPrograms)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);

    *pNumOfPrograms = (int) m_pInputPin->m_Programs.m_ProgramCount;

    return NOERROR;

} // GetCountOfPrograms

//
// GetRecordProgramNumber
//
STDMETHODIMP CPsiParserFilter::GetRecordProgramNumber(DWORD dwIndex, WORD * pwVal)
{
    if(pwVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);


    *pwVal = (WORD) m_pInputPin->m_Programs.m_programs[dwIndex]->program_number;

    return NOERROR;
}// GetRecordProgramNumber

//
// GetRecordProgramMapPid
//
STDMETHODIMP CPsiParserFilter::GetRecordProgramMapPid(DWORD dwIndex, WORD * pwVal)
{
    if(pwVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);

    *pwVal = (WORD) m_pInputPin->m_Programs.m_programs[dwIndex]->network_or_program_map_PID;

    return NOERROR;
} // GetRecordProgramMapPid

//
// FindRecordProgramMapPid
//
STDMETHODIMP CPsiParserFilter::FindRecordProgramMapPid(WORD wProgramNumber, WORD * pwVal)
{
    if(pwVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);
    int i;


    for (i = 0; i<(int) m_pInputPin->m_Programs.m_ProgramCount; i++){
        if(m_pInputPin->m_Programs.m_programs[i]->program_number == wProgramNumber){
            *pwVal = (WORD) m_pInputPin->m_Programs.m_programs[i]->network_or_program_map_PID;
            return NOERROR;
        }
    }


    return E_INVALIDARG;

}//FindRecordProgramMapPid

//
// GetPmtVersionNumber
//
STDMETHODIMP CPsiParserFilter::GetPmtVersionNumber(WORD wProgramNumber, BYTE *pVal)
{
    if(pVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);

    for (int i = 0; i<(int) m_pInputPin->m_Programs.m_ProgramCount; i++){
        if(m_pInputPin->m_Programs.m_programs[i]->program_number == wProgramNumber){
            *pVal = (BYTE) m_pInputPin->m_Programs.m_programs[i]->mpeg2_pmt_section.version_number;
            return NOERROR;

        }
    }


    return E_INVALIDARG;
}// GetPmtVersionNumber

//
// GetCountOfElementaryStreams
//
STDMETHODIMP CPsiParserFilter::GetCountOfElementaryStreams(WORD wProgramNumber, WORD *pwVal)
{
    if(pwVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);

    for (int i = 0; i<(int) m_pInputPin->m_Programs.m_ProgramCount; i++){
        if(m_pInputPin->m_Programs.m_programs[i]->program_number == wProgramNumber){
            *pwVal = (BYTE) m_pInputPin->m_Programs.m_programs[i]->mpeg2_pmt_section.number_of_elementary_streams;
            return NOERROR;

        }
    }


    return E_INVALIDARG;
}// GetCountOfElementaryStreams

//
// GetRecordStreamType
//
STDMETHODIMP CPsiParserFilter::GetRecordStreamType(WORD wProgramNumber,DWORD dwRecordIndex, BYTE *pbVal)
{
    if(pbVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);
    int i;

    for (i = 0; i<m_pInputPin->m_Programs.m_ProgramCount; i++){
        if(m_pInputPin->m_Programs.m_programs[i]->program_number == wProgramNumber){
            *pbVal = (BYTE) m_pInputPin->m_Programs.m_programs[i]->mpeg2_pmt_section.elementary_stream_info[dwRecordIndex].stream_type;
            return NOERROR;
        }
    }
    return E_INVALIDARG;
}// GetRecordStreamType

//
// GetRecordElementaryPid
//
STDMETHODIMP CPsiParserFilter::GetRecordElementaryPid(WORD wProgramNumber,DWORD dwRecordIndex, WORD *pwVal)
{
    if(pwVal == NULL)
        return E_INVALIDARG;

    CAutoLock cAutoLock(&m_ParserLock);
    int i;

    for (i = 0; i<m_pInputPin->m_Programs.m_ProgramCount; i++){
        if(m_pInputPin->m_Programs.m_programs[i]->program_number == wProgramNumber){
            *pwVal = (WORD) m_pInputPin->m_Programs.m_programs[i]->mpeg2_pmt_section.elementary_stream_info[dwRecordIndex].elementary_PID;
            return NOERROR;
        }
    }
    return E_INVALIDARG;
}// GetRecordElementaryPid

/////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

//
// Constructor
//
CPsiParserInputPin::CPsiParserInputPin( CPsiParserFilter *pFilter,
                                        LPUNKNOWN pUnk,
                                        CCritSec *pLock,
                                        CCritSec *pReceiveLock,
                                        HRESULT *phr)
: CRenderedInputPin(NAME("CPSIParserInputPin"),
                    (CBaseFilter *) pFilter,    // Filter
                    pLock,                      // Locking
                    phr,                        // Return code
                    L"Input")                   // Pin name
, m_pFilter(pFilter)
, m_pReceiveLock(pReceiveLock)
, m_pPatProcessor(NULL)
, m_pPmtProcessor(NULL)
, m_bCreated(false)

{

} // (Constructor)


//
// Destructor
//
CPsiParserInputPin::~CPsiParserInputPin()
{

} // (Destructor)


//
// GetPatProcessor
//
HRESULT CPsiParserInputPin::GetPatProcessor(CPATProcessor ** ppPatProcessor)
{
    if(ppPatProcessor == NULL)
        return E_INVALIDARG;

    CAutoLock lock(m_pReceiveLock);

    if(m_pPatProcessor != NULL)
        *ppPatProcessor = m_pPatProcessor;
    else
        return E_INVALIDARG;

    return NOERROR;
} //GetPatProcessor

//
// CheckMediaType
//
// Check that we can support a given proposed type
//
HRESULT
CPsiParserInputPin::CheckMediaType(const CMediaType *pMediaType)
{

    if( *(pMediaType->Type()) == MY_MEDIATYPE_MPEG2_SECTIONS ||
        *(pMediaType->Type()) == MEDIATYPE_MPEG2PSI) {
        return S_OK;
    }

    return VFW_E_INVALIDMEDIATYPE;
} // CheckMediaType


//
// Receive
//
// Here's the next block of data from the stream
//
HRESULT CPsiParserInputPin::Receive(IMediaSample * pSample)
{

    // Lock this with the receiver lock
    CAutoLock lock(m_pReceiveLock);

    // Check all is well with the base class, if not, drop it.
    HRESULT hr = CBaseInputPin::Receive(pSample);
    if(hr != S_OK)
    {
        return hr;
    }


    // Send the sample to the table processor object for processing

    CHECK_BADPTR( TEXT("invalid sample"), pSample);


    BYTE* pData = NULL;
    hr = pSample->GetPointer(&pData);
    CHECK_ERROR( TEXT("pSample->GetPointer() failed"), hr);
    CHECK_BADPTR( TEXT("invalid sample"), pData);

    long lDataLen;
    lDataLen =  pSample->GetActualDataLength();


    unsigned char tableID;
    tableID = *pData;

    // the two processors are created here instead of in the constructor,
    // because the demux and the psi filter should be connected first before
    // the demux is retrieved from psi filter in the processor's constructor

    if (m_bCreated == false) {

        // initialize data shared by two processors:
        m_Programs.init_programs();

        // create two processors once
        m_pPatProcessor = new CPATProcessor(m_pFilter,
                                        &m_Programs,
                                        &hr);
        if(m_pPatProcessor == NULL)
        {
              hr = E_OUTOFMEMORY;
        }

        m_pPmtProcessor = new CPMTProcessor(m_pFilter,
                                            &m_Programs,
                                            &hr);

        if(m_pPmtProcessor == NULL)
        {
                hr = E_OUTOFMEMORY;
        }

        m_bCreated = true;
    }
    // PAT section
    if(*pData == PAT_TABLE_ID && VALID_PSI_HEADER(pData) ){
        m_pPatProcessor->process(pData, lDataLen);
    }
    //PMT section
    else if(*pData == PMT_TABLE_ID && VALID_PSI_HEADER(pData)){
        m_pPmtProcessor->process(pData, lDataLen);
    }

    return hr;

} // Receive

//
// EndOfStream
//
STDMETHODIMP CPsiParserInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();

} // EndOfStream


/////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////



CPATProcessor::CPATProcessor(CPsiParserFilter *pParser,
                             CPrograms * pPrograms,
                             HRESULT *phr)
: m_pParser(pParser)
, m_pPrograms(pPrograms)
, m_current_transport_stream_id( 0xff)
, m_pat_section_count(0)
, m_mapped_pmt_pid_count(0)
, m_pDemuxPsiOutputPin(NULL)
{
    // follow these steps to get demux PSI output pin:

    // get the graph builder
    IFilterGraph *     pGraph;
    pGraph = m_pParser-> GetFilterGraph();

    // get demux filter
    IBaseFilter * pDemux = NULL;
    *phr = GetDemuxFilter(pGraph, &pDemux);
    if (*phr == S_OK)
    {
        // get demux psi output pin:
        *phr = GetDemuxPsiOutputPin(pDemux, &m_pDemuxPsiOutputPin);

        pDemux->Release();
    }

}

CPATProcessor::~CPATProcessor()
{
    ASSERT(m_pDemuxPsiOutputPin);
    m_pDemuxPsiOutputPin->Release();
    m_pDemuxPsiOutputPin = NULL;
}


BOOL CPATProcessor::IsNewPATSection(DWORD dwSectionNumber)
{

    BOOL bIsNewSection = TRUE;

    for(int i = 0; i<(int)m_pat_section_count; i++) {
        if(m_pat_section_number_array[i] == dwSectionNumber) {
            return FALSE;
        }
    }

    return bIsNewSection;

}

//
// MapPmtPid
//
BOOL CPATProcessor::MapPmtPid(DWORD dwPmtPid)
{
    BOOL bResult = TRUE;


    if(m_pDemuxPsiOutputPin == NULL)
        return FALSE;


    IMPEG2PIDMap * pIPmtPIDMap;
    HRESULT hr = m_pDemuxPsiOutputPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &pIPmtPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::MapPmtPid():: QI the IMPEG2PIDMap inf on PSI out pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::MapPmtPid()::pIPmtPIDMap is null "), pIPmtPIDMap) ;

    ULONG ulPmtPID[1] = {(ULONG) dwPmtPid};

    hr = pIPmtPIDMap->MapPID(1, ulPmtPID, MEDIA_MPEG2_PSI);
    if(FAILED(hr)){
        pIPmtPIDMap->Release();
        return FALSE;
    }

    pIPmtPIDMap->Release();
    return bResult;
}// MapPmtPid


//
// UnmapPmtPid
//
// unmap all pmt pids from demux psi output pin
BOOL CPATProcessor::UnmapPmtPid()
{
    BOOL bResult = TRUE;

    if(m_pDemuxPsiOutputPin == NULL)
        return FALSE;

    IMPEG2PIDMap * pIPmtPIDMap;
    HRESULT hr = m_pDemuxPsiOutputPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &pIPmtPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: QI the IMPEG2PIDMap inf on PSI out pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::SetupMPEGDeMux()::pIPmtPIDMap is null "), pIPmtPIDMap) ;


    for(int i = 0; i<(int)m_mapped_pmt_pid_count;i++){

        ULONG ulPmtPID[1] = {(ULONG) m_mapped_pmt_pid_array[i]};

        hr = pIPmtPIDMap->UnmapPID(1, ulPmtPID);
        if(FAILED(hr)){
            pIPmtPIDMap->Release();
            return FALSE;
        }
    }
    pIPmtPIDMap->Release();
    return bResult;
} // UnmapPmtPid

//
// process
//
// process the pat section received (see data flow chart in SDK document)
BOOL CPATProcessor::process(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //Is this section currently applicable ?
    // "0" indicates that the table sent is not yet applicable
    // and shall be the next table to become valid
    BOOL current_next_indicator = PAT_CURRENT_NEXT_INDICATOR_BIT(pbBuffer);
    if( current_next_indicator == 0 ){
        return FALSE; // discard and do nothing
    }

    // new transport stream
    DWORD transport_stream_id = PAT_TRANSPORT_STREAM_ID_VALUE(pbBuffer);
    if( transport_stream_id != m_current_transport_stream_id ) {
        // flush all programs
        if(!flush())
            return FALSE;
        bResult = store(pbBuffer, lDataLen);
    }
    else {

        // new section
        DWORD section_number = PAT_SECTION_NUMBER_VALUE(pbBuffer);
        if( IsNewPATSection(section_number) ){
            bResult = store(pbBuffer, lDataLen);
        }
        else {
            // new PAT version, i.e. transport stream changed (such as adding or deleting programs)
            DWORD version_number = PAT_VERSION_NUMBER_VALUE(pbBuffer);
            if( m_current_pat_version_number != version_number ){
                if(!flush())
                    return FALSE;
                bResult = store(pbBuffer, lDataLen);
            }
            // else discard the section and do nothing;
        }
    }
    return bResult;
} //process

//
//store
//
// add new program or update existing program
BOOL CPATProcessor::store(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult =TRUE;
    //if it is not valid pat section, ignore it
    if(!VALID_PAT_SECTION(pbBuffer))
        return FALSE;

    if(!ConfirmMpeg2PSICRC_32 (pbBuffer,lDataLen) )
        return FALSE;

    MPEG2_PAT_SECTION mpeg2_pat_section;
    // if we can't parse it, then ignore it
    if(!mpeg2_pat_section.Parse(pbBuffer))
        return FALSE;


    // update m_current_transport_stream_id
    m_current_transport_stream_id = mpeg2_pat_section.transport_stream_id;

    // update m_current_pat_version_number
    m_current_pat_version_number = mpeg2_pat_section.version_number;

    // update m_pat_section_number_list
    m_pat_section_number_array[m_pat_section_count] = mpeg2_pat_section.section_number;
    m_pat_section_count ++;

    // update s_mpeg2_programs, for each program:
    for( int i=0;i<(int) (mpeg2_pat_section.number_of_programs); i++)// For each program contained in this pat section
    {

        //  add new program or update existing program
        Dump(TEXT("add new program\n"));
        m_pPrograms->add_program_from_pat(&mpeg2_pat_section, i);

        // if the pmt PID has not been mapped to demux output pin, map it
        if( !HasPmtPidMappedToPin(mpeg2_pat_section.program_descriptor[i].program_number)) {
            Dump1(TEXT("map new pmt pid %d to demux output pin\n"), mpeg2_pat_section.program_descriptor[i].network_or_program_map_PID);
            if(!MapPmtPid(mpeg2_pat_section.program_descriptor[i].network_or_program_map_PID))
                return FALSE;
        }
    }

    m_pParser->NotifyEvent(EC_PROGRAM_CHANGED,0,(LONG_PTR)(IBaseFilter*)m_pParser);

    return bResult;

} // store

//
//HasPmtPidMappedToPin
//
// if the pmt pid has been mapped to demux's psi output pin, return TRUE;
BOOL CPATProcessor::HasPmtPidMappedToPin(DWORD dwPid)
{
    BOOL bMapped = FALSE;
    for(int i = 0; i<(int)m_mapped_pmt_pid_count;i++){
        if(m_mapped_pmt_pid_array[i] == dwPid)
            return TRUE;
    }
    return bMapped;
}//HasPmtPidMappedToPin

//
// flush
//
// flush an array of struct: m_mpeg2_program[];
// and unmap all PMT_PIDs pids, except one: PAT
BOOL CPATProcessor::flush()
{
    BOOL bResult = TRUE;
    bResult = m_pPrograms->free_programs();
    if(bResult == FALSE)
        return bResult;
    bResult = UnmapPmtPid();
    return bResult;
}// flush

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
CPMTProcessor::CPMTProcessor(CPsiParserFilter *pParser,CPrograms * pPrograms, HRESULT *phr)
: m_pParser(pParser)
, m_pPrograms(pPrograms)
, m_pmt_section_count(0)
{
} // Constructor

//
// Destructor
//
CPMTProcessor::~CPMTProcessor()
{
}// Destructor

//
// HasPMTVersionOfThisProgramChanged
//
// for a pmt section of a given program_number, if the version is the same as recorded before,
// return TRUE; otherwise, return false;
BOOL CPMTProcessor::HasPMTVersionOfThisProgramChanged(DWORD dwProgramNumber,
                                                      DWORD dwSectionVersion)
{
    if(m_pmt_section_count != 0){
        for(int i = 0; i<(int)m_pmt_section_count; i++)
            if( dwProgramNumber == m_pmt_program_number_version_array[i].pmt_program_number &&
                dwSectionVersion == m_pmt_program_number_version_array[i].pmt_section_version )
                return FALSE;
    }
    return TRUE;
}// HasPMTVersionOfThisProgramChanged


//
// IsNewPMTSection
//
// if the section number has been received before, return TRUE; else, return FALSE
BOOL CPMTProcessor::IsNewPMTSection(DWORD dwProgramNumber)
{
    if(m_pmt_section_count != 0){
        // in each pmt section, the section number field shall be set to zero.
        // Sections are identified by the program_number field (ISO/IEC 13818-1:1996(E))
        for(int i = 0; i<(int)m_pmt_section_count; i++)
            if(dwProgramNumber == m_pmt_program_number_version_array[i].pmt_program_number)
                return FALSE;

    }
    return TRUE;
}// IsNewPMTSection

//
// process
//
// process a pmt section received(see data flow chart in SDK document)
BOOL CPMTProcessor::process(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //Is this section currently applicable ?
    // "0" indicates that the table sent is not yet applicable
    // and shall be the next table to become valid
    BOOL current_next_indicator = PMT_CURRENT_NEXT_INDICATOR_BIT(pbBuffer);
    if( current_next_indicator == 0 ){
        return FALSE; // discard and do nothing
    }

    // new section
    DWORD program_number = PMT_PROGRAM_NUMBER_VALUE(pbBuffer);
    if( IsNewPMTSection(program_number)) {
        bResult = store(pbBuffer, lDataLen);
    }
    else{
        // new pmt version, just for this program
        DWORD version_number = PMT_VERSION_NUMBER_VALUE(pbBuffer);
        if( HasPMTVersionOfThisProgramChanged(program_number, version_number )){
            bResult = store(pbBuffer, lDataLen);
        }
        //else discard and do nothing
    }
    return bResult;
}// process

//
// store
//
// store new program info in the CProgram object
BOOL CPMTProcessor::store(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //if it is not valid pmt, ignore it
    if(!VALID_PMT_SECTION(pbBuffer))
        return FALSE;

    if(!ConfirmMpeg2PSICRC_32 (pbBuffer,lDataLen) )
        return FALSE;

    MPEG2_PMT_SECTION mpeg2_pmt_section;
    // if we can't parse it, then ignore it
    if(!mpeg2_pmt_section.Parse(pbBuffer))
        return FALSE;

    // keep track of received pmt sections with their program_number and version_number
    m_pmt_program_number_version_array[m_pmt_section_count].pmt_program_number =
        mpeg2_pmt_section.program_number;
    m_pmt_program_number_version_array[m_pmt_section_count].pmt_section_version =
        mpeg2_pmt_section.version_number;

    // keep track of number of received pmt sections
    m_pmt_section_count++;
    ASSERT(m_pmt_section_count <= MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM);

    //  add new program or update existing program
    Dump(TEXT("add/ update pmt section to programs\n"));
    m_pPrograms->update_program_from_pmt(&mpeg2_pmt_section);

    m_pParser->NotifyEvent(EC_PROGRAM_CHANGED,0,(LONG_PTR)(IBaseFilter*)m_pParser);


    return bResult;

}// store
//----------------------------------------------------------------------------
//
// Given a graph, find the filter with specific ClassID
//
//----------------------------------------------------------------------------
// GetDemuxFilter
HRESULT GetDemuxFilter(IFilterGraph    * pGraph,IBaseFilter ** ppDemux )
{
    IEnumFilters * pef;
    HRESULT hr= pGraph->EnumFilters(&pef);
    if(FAILED(hr))
        return hr;

    IBaseFilter * pf;
    ULONG c;
    CLSID clsid;
    hr = E_FAIL;
    while(pef->Next(1, &pf, &c) == S_OK)
    {

        ASSERT(c == 1);
        pf->GetClassID(&clsid);
        if(clsid==CLSID_MPEG2Demultiplexer)
        {
            *ppDemux = pf;
            hr = S_OK;
            break;
        }
        pf->Release();
    }
    pef->Release();
    return hr;
} // GetDemuxFilter

//----------------------------------------------------------------------------
//
// Given a filter, find the output pin which output pecific media type
//
//----------------------------------------------------------------------------
 // GetDemuxPsiOutputPin
HRESULT GetDemuxPsiOutputPin(IBaseFilter * pDemux, IPin ** ppOutPin)
{
    if (!pDemux) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    BOOL bFound = FALSE;
    ULONG            ul ;

    HRESULT hr = pDemux->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }

        if (ThisPinDir == PINDIR_OUTPUT)
        {
            // Get the filter that owns that pin.
            IEnumMediaTypes* pTypeEnum;
            hr = pPin->EnumMediaTypes (&pTypeEnum);

            AM_MEDIA_TYPE* pMediaTypes;
            // Loop thru' preferred media type list for a match
            do {
                hr = pTypeEnum->Next(1, &pMediaTypes, &ul) ;
                if (FAILED(hr) || 0 == ul) {
                    pPin->Release();
                    Dump1(TEXT("IEnumMediaTypes::Next() failed (Error 0x%lx)"),hr) ;  // should be out
                    break ;
                }

                if (pMediaTypes->majortype == MY_MEDIATYPE_MPEG2_SECTIONS ||
                    pMediaTypes->majortype == MEDIATYPE_MPEG2PSI)
                {
                    *ppOutPin = pPin;
                    bFound = TRUE ;
                    pPin->Release();
                    DeleteMediaType( pMediaTypes );

                    Dump(TEXT("AM_MEDIA_TYPE.majortype matches")) ;

                    pTypeEnum->Release();
                    pEnum->Release();
                    return S_OK;
                }
            } while (!bFound) ;  // until the reqd one is found

            pTypeEnum->Release();
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching filter.
    return E_FAIL;
} // GetDemuxPsiOutputPin

//
// ConfirmMpeg2PSICRC_32
//
static
BOOL
ConfirmMpeg2PSICRC_32 (
    IN  BYTE *  pb,
    IN  ULONG   ulLen
    )
{
    ULONG   ulCRCAccum ;
    ULONG   i, j ;

    ulCRCAccum = 0xffffffff ;
    for (j = 0; j < ulLen; j++, pb++) {
        i = (( ulCRCAccum >> 24) ^ (* pb)) & 0xff ;
        ulCRCAccum = ((ulCRCAccum << 8 ) ^ g_MPEG2_PSI_CRC_32_Lookup [i]) ;
    }

    return ulCRCAccum == 0x00000000 ;
} // ConfirmMpeg2PSICRC_32
///-------------------------------------------------------------------
//
// Exported entry points for registration and unregistration
// (in this case they only call through to default implementations).
//
//---------------------------------------------------------------------

//
// DllRegisterServer
//
// Handles DLL registry
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  dwReason,
                      LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
