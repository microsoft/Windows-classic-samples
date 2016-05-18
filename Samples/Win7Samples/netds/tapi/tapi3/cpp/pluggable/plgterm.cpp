// PlgTerm.cpp: implementation of the CPlgTermSample class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlgTerm.h"
#include <initguid.h>
#include "GUIDs.h"
#include "PlgTermFilter.h"
#include "PlgTermSample_i.c"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlgTermSample::CPlgTermSample()
	:m_pEventSink(NULL),
	m_bKnownSafeContext(FALSE)
{

	LOG((MSP_TRACE, "CPlgTermSample::CPlgTermSample enter"));

}


//////////////////////////////////////////////////////////////////////////////


CPlgTermSample::~CPlgTermSample()
{
	LOG((MSP_TRACE, "CPlgTermSample::~CPlgTermSample enter"));

    //
    // if we have an event sink, release it
    //
    if( NULL != m_pEventSink )
    {
		LOG((MSP_TRACE, "CPlgTermSample::~CPlgTermSample release sink"));
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

    //
	// Release free thread marshaler
	//
    if( m_pFTM )
    {
        m_pFTM->Release();
        m_pFTM = NULL;
    }

	LOG((MSP_TRACE, "CPlgTermSample::~CPlgTermSample exit"));
}


    //
    // ITPlgSampleEvent methods
    //

HRESULT CPlgTermSample::FireEvent( long lEventCode)
{
	LOG((MSP_TRACE, "CPlgTermSample::FireEvent enter"));
	//
    // we need a sink before we can fire an event
    //

    CLock lock(m_CritSec);


    if (NULL == m_pEventSink)
    {
        LOG((MSP_ERROR, "CPlgTermSample::FireEvent no Sink - exit E_FAIL"));
        return E_FAIL;
    }


    //
    // initilize the structure
    //

    MSP_EVENT_INFO mspEventInfo;

    mspEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    mspEventInfo.Event = ME_PRIVATE_EVENT;
    mspEventInfo.hCall = NULL;

    mspEventInfo.MSP_PRIVATE_EVENT_INFO.lEventCode = lEventCode;


    //
    // put the pointer to our IDispatch interface in the structure
    //

    HRESULT hr = _InternalQueryInterface(IID_IDispatch, 
                                         reinterpret_cast<void**>(&(mspEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent)));

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CPlgTermSample::FireEvent failed to get IDispatch interface - exit hr=0x%08", hr));
        return hr;
    }

    //
    // pass event to the msp
    //

    hr = m_pEventSink->FireEvent(&mspEventInfo);

    if (FAILED(hr))
    {

        //
        // release all interfaces that we are holding. 
        // fire event failed so no one else will release them for us.
        //
        mspEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent->Release();
        mspEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent=NULL;
		
        LOG((MSP_ERROR, "CPlgTermSample::FireEvent on sink failed - exit hr=0x%08", hr));
       
        return hr;
    }

    //
    // event fired
    //

    LOG((MSP_TRACE, "CPlgTermSample::FireEvent exit S_OK"));

    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// ITPluggableTerminalEventSinkRegistration - Methods implementation
//
//////////////////////////////////////////////////////////////////////

HRESULT CPlgTermSample::RegisterSink(
    IN  ITPluggableTerminalEventSink *pSink
    )
{
	LOG((MSP_TRACE, "CPlgTermSample::RegisterSink enter"));

    //
    // Critical section
    //

    CLock lock(m_CritSec);

    //
    // Validates argument
    //

    if (!pSink)	
    {
        LOG((MSP_ERROR, "CPlgTermSample::RegisterSink - "
			"ITPluggableTerminalEventSink invalid pointer - exit 0x%08x", E_POINTER));
        return E_POINTER;
    }

    //
    // Release the old event sink
    //

    if( NULL != m_pEventSink )
    {
        LOG((MSP_TRACE, "CPlgTermSample::RegisterSink - releasing sink [%p]",m_pEventSink));
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }


    //
    // Set the new event sink
    //

	LOG((MSP_TRACE, "CPlgTermSample::RegisterSink - keeping new sink [%p]",pSink));

    m_pEventSink = pSink;
    m_pEventSink->AddRef();


    LOG((MSP_TRACE, "CPlgTermSample::RegisterSink exit - S_OK"));

    return S_OK;
}


HRESULT CPlgTermSample::UnregisterSink()
{
    LOG((MSP_TRACE, "CPlgTermSample::UnregisterSink enter"));
    //
    // Critical section
    //

    CLock lock(m_CritSec);


    //
    // Release the old event sink
    //

    if( m_pEventSink )
    {
        LOG((MSP_TRACE, "CPlgTermSample::RegisterSink - releasing sink [%p]",m_pEventSink));
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

    LOG((MSP_TRACE, "CPlgTermSample::UnregisterSink - exit S_OK"));
    return S_OK;
}



//////////////////////////////////////////////////////////////////////////////


HRESULT STDMETHODCALLTYPE CPlgTermSample::InitializeDynamic(
	    IN  IID                   iidTerminalClass,
	    IN  DWORD                 dwMediaType,
	    IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        )

{

    LOG((MSP_TRACE, "CPlgTermSample::InitializeDynamic enter"));

    //
    // make sure the direction is correct - we register it with this direction
    //

    if (TD_RENDER != Direction)
    {
        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic"
				" - bad direction [%d] requested. returning 0x%08x", 
				Direction, E_INVALIDARG));
        
        return E_INVALIDARG;
    }

    
    //
    // make sure the mediatype is correct - TAPIMEDIATYPE_AUDIO - is how we set it in registry
    //


    if ( TAPIMEDIATYPE_AUDIO != dwMediaType)
    {

        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic"
				" - bad media type [%d] requested. returning 0x%08x", 
				dwMediaType, E_INVALIDARG));

        return E_INVALIDARG;
    }


    CLock lock(m_CritSec);

    //
    // Call the base class method
    //
    LOG((MSP_TRACE, "CPlgTermSample::InitializeDynamic - CBaseTerminal::Initialize with"
                "dwMediaType = 0x%08x, Direction = 0x%08x", dwMediaType, Direction));

    HRESULT hr;
    hr = CBaseTerminal::Initialize(iidTerminalClass,
                                   dwMediaType,
                                   Direction,
                                   htAddress);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic - "
                "base class method failed - 0x%08x", hr));
        return hr;
    }

    //
    // Set the terminal info
    //

    hr = SetTerminalInfo();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic - "
                "SetTerminalInfo failed - 0x%08x", hr));

        return hr;
    }

    //
    // Create the filter
    //

    hr = CreateFilter();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic - "
				"CreateFilter failed - 0x%08x", hr));
        return hr;
    }

    //
    // Get and sets the pin
    //

    hr = FindPin();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::InitializeDynamic - "
            "FindPin failed - 0x%08x", hr));
    }

    m_bKnownSafeContext = TRUE;

	LOG((MSP_TRACE, "CPlgTermSample::InitializeDynamic exit - 0x%08x", hr));
    return hr;

}


//////////////////////////////////////////////////////////////////////////////


DWORD CPlgTermSample::GetSupportedMediaTypes()
{
    
    LOG((MSP_TRACE, "CPlgTermSample::GetSupportedMediaTypes enter "));

    CLock lock(m_CritSec);

    
    DWORD dwMediaType = TAPIMEDIATYPE_AUDIO; //we set this in registry


    LOG((MSP_TRACE, "CPlgTermSample::GetSupportedMediaTypes exit - S_OK "));


    return dwMediaType;
}



//////////////////////////////////////////////////////////////////////////////


HRESULT CPlgTermSample::AddFiltersToGraph()
{
    LOG((MSP_TRACE, "CPlgTermSample::AddFiltersToGraph enter "));

    USES_CONVERSION;

    //
    // Validates m_pGraph
    //

    if ( m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CPlgTermSample::AddFiltersToGraph - "
				"we have no graph - returning 0x%08x", E_UNEXPECTED));
        return E_UNEXPECTED;
    }

    //
    // Validates m_pIFilter
    //

    if ( m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CPlgTermSample::AddFiltersToGraph - "
				"we have no filter - returning 0x%08x", E_UNEXPECTED));
        return E_UNEXPECTED;
    }

    CLock lock(m_CritSec);

    //
    // AddFilter returns VFW_S_DUPLICATE_NAME if name is duplicate; still succeeds
    //

    HRESULT hr = m_pGraph->AddFilter(m_pIFilter, T2CW(m_szName));

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::AddFiltersToGraph - "
				"can not add filter "));

    }

    LOG((MSP_TRACE, "CPlgTermSample::AddFiltersToGraph - exit %08x", hr));
    return hr;
}
/*++
SetTerminalInfo

Sets the name of the terminal and the terminal type
Is called by InitializeDynamic
--*/
HRESULT CPlgTermSample::SetTerminalInfo()
{
    LOG((MSP_TRACE, "CPlgTermSample::SetTerminalInfo - enter"));

	//
    // set terminals's name for ITTerminal::get_Name 
    //

    size_t nStringMaxSize = sizeof(m_szName)/sizeof(TCHAR);

    wcsncpy_s(m_szName,nStringMaxSize , SZTERMNAME, min(_tcslen(SZTERMNAME), nStringMaxSize));


    //
    // in case string copy did not append with a zero, do it by hand
    //

    m_szName[min(_tcslen(SZTERMNAME), nStringMaxSize - 1)] = 0;

    //
    // set other properties
    //

    m_TerminalType = TT_DYNAMIC;

    m_TerminalState = TS_NOTINUSE;

    LOG((MSP_TRACE, "CPlgTermSample::SetTerminalInfo - exit S_OK"));

    return S_OK;
}


/*++
CreateFilter

Create the internal filter
Is called by InitializeDynamic
--*/
HRESULT CPlgTermSample::CreateFilter()
{
    LOG((MSP_TRACE, "CPlgTermSample::CreateFilter - enter"));

    // Create the filter
    CPlgFilter* pFilter = new CPlgFilter();

    if( NULL == pFilter )
    {
        LOG((MSP_ERROR, "CPlgTermSample::CreateFilter - "
                "create filter failed - returning 0x%08x", E_OUTOFMEMORY));
        return E_OUTOFMEMORY;
    }

    // COM Rule
    pFilter->AddRef();

	//
	// Get the pin
	//

	HRESULT hr = pFilter->Initialize( );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::CreateFilter - "
                "Initialize private failed - returning 0x%08x", hr));
		pFilter->Release();
        return hr;
    }


    //
    // Get the IBaseFilter interface
    //

    hr = pFilter->QueryInterface(
        IID_IBaseFilter,
        (void**)&m_pIFilter
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPlgTermSample::CreateFilter - "
            "QI for IBaseFilter failed"));
    }    
    else
    {
        pFilter->InitializePrivate(this);
    }

    pFilter->Release();

    LOG((MSP_TRACE, "CPlgTermSample::CreateFilter - exit 0x%08x", hr));
    return hr;
}

/*++
FindPin

Get the pin from the filter and set the m_pIPin member
Is called by InitializeDynamic
--*/
HRESULT CPlgTermSample::FindPin()
{
    LOG((MSP_TRACE, "CPlgTermSample::FindPin - enter"));

    //
    // Validates the filter object (smart pointer)
    //

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CPlgTermSample::FindPin - "
				"filter object is NULL - returning 0x%08x", E_UNEXPECTED));
        return E_UNEXPECTED;
    }

    //
    // Make sure the IPin object is not initialized
    //

    if (m_pIPin != NULL)
    {
        LOG((MSP_ERROR, "CPlgTermSample::FindPin - "
				"already got a pin - returning 0x%08x", E_UNEXPECTED));
        return E_UNEXPECTED;
    }

    HRESULT hr;
    IEnumPins* pIEnumPins;
    ULONG cFetched;

    //
    // Get the pins collection
    //

    if (FAILED(hr = m_pIFilter->EnumPins(&pIEnumPins)))
    {
        LOG((MSP_ERROR, "CPlgTermSample::FindPin - "
				"cannot enum pins - returning 0x%08x", hr));
        return hr;
    }

    //
    // Get the out pin of the Pluggable Filter
    //

    if (S_OK != (hr = pIEnumPins->Next(1, &m_pIPin, &cFetched)))
    {
        LOG((MSP_ERROR, "CPlgTermSample::FindPin - "
				"cannot get a pin - returning 0x%08x", hr));

        hr = (hr == S_FALSE) ? E_FAIL : hr;
    }

    LOG((MSP_TRACE, "CPlgTermSample::FindPin - exit 0x%08x", hr));
    return S_OK;
}



//////////////////////////////////////////////////////////////////////////////
//
//  SetInterfaceSafetyOptions
//
//  this is a safeguard to prevent using this terminal in scripting outside 
//  terminal manager context.
//
//  if we detect that InitializeDynamic has not been called, this method will 
//  fail thus marking the object as unsafe for scripting
//
//////////////////////////////////////////////////////////////////////////////

HRESULT CPlgTermSample::SetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD dwOptionSetMask, 
                                                    DWORD dwEnabledOptions)
{

    CLock lock(m_CritSec);


    //
    // check if we are running in safe context
    //

    if (!m_bKnownSafeContext) 
    {

        //
        // we have not been initialized properly... someone evil is trying to 
        // use this terminal. NO!
        //

        return E_FAIL;
    }


    //
    // we are known to safe, so simply delegate request to the base class
    //

    return CMSPObjectSafetyImpl::SetInterfaceSafetyOptions(riid, 
                                                           dwOptionSetMask, 
                                                           dwEnabledOptions);
}


//////////////////////////////////////////////////////////////////////////////
//
//  GetInterfaceSafetyOptions
//
//  this is a safeguard to prevent using this terminal in scripting outside 
//  terminal manager context.
//
//  if we detect that InitializeDynamic has not been called, this method will 
//  fail thus marking the object as unsafe for scripting
//
//////////////////////////////////////////////////////////////////////////////

HRESULT CPlgTermSample::GetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD *pdwSupportedOptions, 
                                                    DWORD *pdwEnabledOptions)
{

    CLock lock(m_CritSec);


    //
    // check if we are running in safe context
    //

    if (!m_bKnownSafeContext) 
    {

        //
        // we have not been initialized properly... someone evil is trying to 
        // use this terminal. NO!
        //

        return E_FAIL;
    }


    //
    // we are known to safe, so simply delegate request to the base class
    //

    return CMSPObjectSafetyImpl::GetInterfaceSafetyOptions(riid, 
                                                           pdwSupportedOptions,
                                                           pdwEnabledOptions);
}



HRESULT CPlgTermSample::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CPlgTermSample::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPlgTermSample::FinalConstruct - exit S_OK"));

    return S_OK;

}

// eof