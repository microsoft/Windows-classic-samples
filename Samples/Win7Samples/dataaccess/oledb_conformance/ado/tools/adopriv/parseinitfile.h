// ParseInitFile.h: Definition of the CParseFile class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSEINITFILE_H__D9824553_5964_11D1_8AE1_00C04FB90A5E__INCLUDED_)
#define AFX_PARSEINITFILE_H__D9824553_5964_11D1_8AE1_00C04FB90A5E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CParseFile


class CParseFile : 
	public CComDualImpl<IParseInitFile, &IID_IParseInitFile, &LIBID_ADOPriv>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CParseFile,&CLSID_ParseInitFile>,
	public fstream
{
public:
	CParseFile();
	~CParseFile();
BEGIN_COM_MAP(CParseFile)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IParseInitFile)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(CParseFile) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_ParseInitFile)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IParseInitFile
public:
	STDMETHOD(GetColumn)(/*[in]*/ SHORT Index, /*[out, retval]*/ IColumn** ppVal);
	STDMETHOD(putref_ModInfo)(/*[in]*/ IModInfo* newVal);
	STDMETHOD(GetRowOffset)(/*[in]*/ LONG Row, /*[out, retval]*/ LONG* pVal);
	STDMETHOD(GetQuery)(/*[in]*/ LONG eQuery, /*[out, retval]*/ BSTR* pVal);
	STDMETHOD(GetData)(/*[in]*/ LONG Row, /*[in]*/ LONG Column, /*[in, out]*/ BSTR* Data, /*[out, retval]*/ SHORT* pVal);
	STDMETHOD(FetchRow)(/*[in]*/ LONG Row, /*[out, retval]*/ BOOL* pVal);
	STDMETHOD(FindSection)(/*[in]*/ BSTR Section, /*[out,retval]*/ BOOL* pVal);
	STDMETHOD(GetNextLine)(/*[in, out]*/ LONG* plLength);
	STDMETHOD(Init)(/*[in]*/ BSTR FileName, /*[out, retval]*/ BOOL* pVal);
	STDMETHOD(get_RowCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ColumnCount)(/*[out, retval]*/ long *pVal);
protected:

protected: //User Variables
	virtual HRESULT CreateColumn(IColumn** ppVal);
	IModInfo*			m_pModuleInfo;
	CParseInitFile*		m_pParser;
	POSITION			m_posHead;
	POSITION			m_posTail;
	POSITION*			m_pPosition;
	LONG				m_cColumn;
	LONG				m_cRow; //1 based
};

#endif // !defined(AFX_PARSEINITFILE_H__D9824553_5964_11D1_8AE1_00C04FB90A5E__INCLUDED_)
