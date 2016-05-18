// ComponentInfo.h : Declaration of the CComponentInfo

#pragma once
#include "resource.h"       // main symbols
#include "napcommon.h"

EXTERN_C const CLSID CLSID_ComponentInfo;

#ifdef __cplusplus
typedef class ComponentInfo ComponentInfo;
#else
typedef struct ComponentInfo ComponentInfo;
#endif /* __cplusplus */

#ifdef __cplusplus

class DECLSPEC_UUID("E19DDEC2-3FBE-4C3B-9317-679760C13AAE")
ComponentInfo;
#endif

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// The dll file name
static const WCHAR SHA_SDK_SAMPLE_DLL_FILE_NAME[] = L"SdkShaInfo.dll";

static const WORD LANG_ID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

// CComponentInfo

class ATL_NO_VTABLE CComponentInfo :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CComponentInfo, &CLSID_ComponentInfo>,
	public INapComponentInfo
{
public:
	CComponentInfo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMPONENTINFO)

BEGIN_COM_MAP(CComponentInfo)
	COM_INTERFACE_ENTRY(INapComponentInfo)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
    STDMETHOD(GetFriendlyName)(MessageId * friendlyName);
    STDMETHOD(GetDescription)(MessageId * description);
    STDMETHOD(GetVendorName)(MessageId * vendorName);
    STDMETHOD(GetVersion)(MessageId * version);
    STDMETHOD(GetIcon)(CountedString ** dllFilePath, UINT32 * iconResourceId);
    STDMETHOD(ConvertErrorCodeToMessageId)(HRESULT errorCode, MessageId * msgId);
    STDMETHOD(GetLocalizedString)(MessageId msgId, CountedString ** string);
};
OBJECT_ENTRY_AUTO(__uuidof(ComponentInfo), CComponentInfo)