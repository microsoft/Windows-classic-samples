// PlgTerm.h: interface for the CPlgTermSample class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TERMSAMPLE_H__4FD57957_2DF1_4F78_AB2C_5E365EFD9CC8__INCLUDED_)
#define AFX_TERMSAMPLE_H__4FD57957_2DF1_4F78_AB2C_5E365EFD9CC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SZTERMNAME		_T("Pluggable Sample Terminal")

#include "PlgTermPriv.h"


class CPlgTermSample  : 
    public CComCoClass<CPlgTermSample, &CLSID_PlgTermSample>,
    public ITPluggableTerminalEventSinkRegistration,
    public ITPluggableTerminalInitialization,
    public ITPlgPrivEventSink,
    public CMSPObjectSafetyImpl,
    public CSingleFilterTerminal
{

public:

    DECLARE_REGISTRY_RESOURCEID(IDR_PLG_TEST)


BEGIN_COM_MAP(CPlgTermSample)
    COM_INTERFACE_ENTRY(ITPluggableTerminalEventSinkRegistration)
    COM_INTERFACE_ENTRY(ITPluggableTerminalInitialization)
    COM_INTERFACE_ENTRY(ITPlgPrivEventSink)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY2(IDispatch, ITTerminal)
    COM_INTERFACE_ENTRY_CHAIN(CSingleFilterTerminal)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

    CPlgTermSample();
	virtual ~CPlgTermSample();

    DECLARE_NOT_AGGREGATABLE(CPlgTermSample) 
    DECLARE_GET_CONTROLLING_UNKNOWN()

    virtual HRESULT FinalConstruct(void);

    //
    // ITTerminal methods - is implemented in base class
    //

//    STDMETHOD(get_TerminalClass)(OUT  BSTR *pbstrTerminalClass);
//    STDMETHOD(get_TerminalType) (OUT  TERMINAL_TYPE *pTerminalType);
//    STDMETHOD(get_State)        (OUT  TERMINAL_STATE *pTerminalState);
//    STDMETHOD(get_Name)         (OUT  BSTR *pVal);
//    STDMETHOD(get_MediaType)    (OUT  long * plMediaType);
//    STDMETHOD(get_Direction)    (OUT  TERMINAL_DIRECTION *pDirection);


    //
    // implementations of CBaseTerminal methods
    //

    virtual HRESULT AddFiltersToGraph();

    virtual DWORD GetSupportedMediaTypes();

    //
    // implementations of CSingleFilterTerminal methods - is implemented in base class
    //

//	virtual HRESULT GetNumExposedPins(
//        IN   IGraphBuilder * pGraph,
//        OUT  DWORD         * pdwNumPins); 
 
	
	//
    // ITPluggableTerminalInitialization method
    //

    virtual HRESULT STDMETHODCALLTYPE InitializeDynamic(
        IN  IID                   iidTerminalClass,
        IN  DWORD                 dwMediaType,
        IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        );

    //
    // overriding IObjectSafety methods. we are only safe if properly 
    // initialized by terminal manager, so these methods will fail if this
    // is not the case.
    //

    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD dwOptionSetMask, 
                                         DWORD dwEnabledOptions);

    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD *pdwSupportedOptions, 
                                         DWORD *pdwEnabledOptions);


	//
    // ITPlgPrivEventSink methods
    //

    STDMETHOD (FireEvent)(long lEventCode);
    
    //
    // ITPluggableTerminalEventSinkRegistration methods
    //

    STDMETHOD(RegisterSink)(
        IN  ITPluggableTerminalEventSink *pSink
        );

    STDMETHOD(UnregisterSink)();


private:
    // --- Helper functions ---
    HRESULT SetTerminalInfo();

    HRESULT CreateFilter();

    HRESULT FindPin();


private:
    //
    // sink for firing terminal events
    //

    ITPluggableTerminalEventSink* m_pEventSink; 

	//
	// pointer to the free threaded marshaler
	//

	IUnknown*            m_pFTM;                
    
	//
    // this terminal should only be instantiated in the context of terminal 
    // manager. the object will only be safe for scripting if it has been 
    // InitializeDynamic'ed. 
    //
    // this flag will be set when InitializeDynamic succeeds
    //

    BOOL m_bKnownSafeContext;

};

#endif // !defined(AFX_TERMSAMPLE_H__4FD57957_2DF1_4F78_AB2C_5E365EFD9CC8__INCLUDED_)
