// ModInfo.h: Definition of the CModuleInfo class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODINFO_H__B3AD7BC4_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_)
#define AFX_MODINFO_H__B3AD7BC4_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CModuleInfo

class CModuleInfo : 
	public CComDualImpl<IModInfo, &IID_IModInfo, &LIBID_ADOPriv>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CModuleInfo,&CLSID_ModInfo>
{
public:
	CModuleInfo();
	~CModuleInfo();
BEGIN_COM_MAP(CModuleInfo)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IModInfo)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(CModuleInfo) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_ModInfo)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IModInfo
public:
	virtual CModInfo* GetModInfo();
	HRESULT fInit();
	STDMETHOD(get_ParseObject)(/*[out, retval]*/ IParseInitFile* *pVal);
	STDMETHOD(Init)(/*[out, retval]*/ BOOL* pVal);
	STDMETHOD(IsStrictLeveling)(/*[out, retval]*/ VARIANT_BOOL* pfResult);
	STDMETHOD(get_Insert)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Insert)(/*[in]*/ long newVal);
	STDMETHOD(get_InitString)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_InitString)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_FileName)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_ProviderLevel)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_DefaultQuery)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_DefaultQuery)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_TableName)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_TableName)(/*[in]*/ BSTR newVal);
	STDMETHOD(GetInitStringValue)(/*[in]*/ BSTR Keyword, /*[in, out]*/ BSTR* Value, /*[out, retval]*/ VARIANT_BOOL* pfResult);
	STDMETHOD(ParseInitFile)(/*[out, retval]*/ VARIANT_BOOL* pfResult);
	STDMETHOD(ParseInitString)(/*[out, retval]*/ VARIANT_BOOL* pfResult);

protected: //member variables
	virtual IParseInitFile* CreateParseInitFile();

	CModInfo*		m_pModInfo;
	IParseInitFile*	m_pParseFile;
	BSTR			m_bstrInitString;
};

#endif // !defined(AFX_MODINFO_H__B3AD7BC4_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_)
