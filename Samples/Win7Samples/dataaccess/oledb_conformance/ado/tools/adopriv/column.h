// Column.h: Definition of the CColumn class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLUMN_H__B3AD7BC7_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_)
#define AFX_COLUMN_H__B3AD7BC7_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CColumn

class CColumn : 
	public CComDualImpl<IColumn, &IID_IColumn, &LIBID_ADOPriv>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CColumn,&CLSID_Column>
{
public:
	CColumn();
	~CColumn();
BEGIN_COM_MAP(CColumn)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IColumn)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(CColumn) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_Column)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IColumn
public:
	virtual BOOL SetCol(CCol& Col);
	STDMETHOD(get_HasDefault)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_HasDefault)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_DefaultValue)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_DefaultValue)(/*[in]*/ VARIANT newVal);
	STDMETHOD(get_CaseSensitive)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_CaseSensitive)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_IsFixedLength)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_IsFixedLength)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_IsLong)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_IsLong)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_UseInSQL)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_UseInSQL)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Searchable)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Searchable)(/*[in]*/ long newVal);
	STDMETHOD(get_Updateable)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_Updateable)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Unique)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_Unique)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_AutoInc)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_AutoInc)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_MaxScale)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_MaxScale)(/*[in]*/ short newVal);
	STDMETHOD(get_MinScale)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_MinScale)(/*[in]*/ short newVal);
	STDMETHOD(get_Scale)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_Scale)(/*[in]*/ short newVal);
	STDMETHOD(get_Unsigned)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_Unsigned)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_IsNullable)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_IsNullable)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_ColumnSize)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ColumnSize)(/*[in]*/ long newVal);
	STDMETHOD(get_Precision)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Precision)(/*[in]*/ long newVal);
	STDMETHOD(get_SubType)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_SubType)(/*[in]*/ short newVal);
	STDMETHOD(get_ProviderType)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_ProviderType)(/*[in]*/ short newVal);
	STDMETHOD(get_ColNum)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_ColNum)(/*[in]*/ long newVal);
	STDMETHOD(get_CreateParams)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_CreateParams)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Prefix)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Prefix)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Suffix)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Suffix)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProviderTypeName)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProviderTypeName)(/*[in]*/ BSTR newVal);

protected: //User member variables
	CCol*	m_pCol;
};

#endif // !defined(AFX_COLUMN_H__B3AD7BC7_57D7_11D1_8AE1_00C04FB90A5E__INCLUDED_)
