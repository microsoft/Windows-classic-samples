
//
// ------------------------------------------
//

EXTERN_C const CLSID    CLSID_WirelessHelperExtension;
EXTERN_C const IID      LIBID_WirelessHelperExtensionLib;

#define CHECK_HR(hr) if (FAILED(hr)) goto error;

//
// ------------------------------------------
//

class CWirelessHelperExtension :
    public INetDiagHelperInfo,
    public INetDiagHelper,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CWirelessHelperExtension, &CLSID_WirelessHelperExtension>
{
public:
    CWirelessHelperExtension(){
        m_IfType=0; m_initialized=0; m_LowHealthAttributeType=0; m_ReturnRepair=0;
    }

    BEGIN_COM_MAP(CWirelessHelperExtension)
        COM_INTERFACE_ENTRY(INetDiagHelperInfo)
        COM_INTERFACE_ENTRY(INetDiagHelper)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CWirelessHelperExtension)
    DECLARE_REGISTRY_RESOURCEID(IDR_HELPER_EXTENSION)

    // INetDiagHelperInfo
    HRESULT STDMETHODCALLTYPE GetAttributeInfo( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HelperAttributeInfo **pprgAttributeInfos);

    // INetDiagHelper
    HRESULT STDMETHODCALLTYPE Initialize( 
        ULONG celt,
        __RPC__in_ecount_full(celt) HELPER_ATTRIBUTE rgAttributes[  ]);
   
    HRESULT STDMETHODCALLTYPE GetDiagnosticsInfo( 
        __RPC__deref_out_opt DiagnosticsInfo **ppInfo);

    HRESULT STDMETHODCALLTYPE GetKeyAttributes( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HELPER_ATTRIBUTE **pprgAttributes);

    HRESULT STDMETHODCALLTYPE LowHealth( 
        __RPC__in_opt LPCWSTR pwszInstanceDescription,        
        __RPC__deref_out_opt_string LPWSTR *ppwszDescription,
        __RPC__out long *pDeferredTime,
        __RPC__out DIAGNOSIS_STATUS *pStatus);

    HRESULT STDMETHODCALLTYPE HighUtilization( 
        __RPC__in_opt LPCWSTR pwszInstanceDescription,        
        __RPC__deref_out_opt_string LPWSTR *ppwszDescription,
        __RPC__out long *pDeferredTime,
        __RPC__out DIAGNOSIS_STATUS *pStatus);

    HRESULT STDMETHODCALLTYPE GetLowerHypotheses( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses);

    HRESULT STDMETHODCALLTYPE GetDownStreamHypotheses( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses) ;

    HRESULT STDMETHODCALLTYPE GetHigherHypotheses( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses);

    HRESULT STDMETHODCALLTYPE GetUpStreamHypotheses( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses);

    HRESULT STDMETHODCALLTYPE Repair( 
        __RPC__in RepairInfo *pInfo,
        __RPC__out long *pDeferredTime,
        __RPC__out REPAIR_STATUS *pStatus);


    HRESULT STDMETHODCALLTYPE Validate(
        PROBLEM_TYPE problem,
        __RPC__out long *pDeferredTime,
        __RPC__out REPAIR_STATUS *pStatus);

    HRESULT STDMETHODCALLTYPE GetRepairInfo( 
        PROBLEM_TYPE problem,
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) RepairInfo **ppInfo);

    HRESULT STDMETHODCALLTYPE GetLifeTime( 
        __RPC__out LIFE_TIME *pLifeTime);

    HRESULT STDMETHODCALLTYPE SetLifeTime( 
        LIFE_TIME lifeTime);

    HRESULT STDMETHODCALLTYPE GetCacheTime( 
        __RPC__out FILETIME *pCacheTime);

    HRESULT STDMETHODCALLTYPE GetAttributes( 
        __RPC__out ULONG *pcelt,
        __RPC__deref_out_ecount_full_opt(*pcelt) HELPER_ATTRIBUTE **pprgAttributes);

    HRESULT STDMETHODCALLTYPE Cancel();

    HRESULT STDMETHODCALLTYPE Cleanup();

    DWORD m_IfType;
    LONG m_initialized;
    UINT32 m_LowHealthAttributeType;
    BOOL m_ReturnRepair;
};

