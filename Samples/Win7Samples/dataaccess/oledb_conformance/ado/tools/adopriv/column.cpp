// Column.cpp : Implementation of CADOPrivApp and DLL registration.

#include "stdafx.h"
#include "ADOPriv.h"
#include <privlib.h>
#include "Column.h"

/////////////////////////////////////////////////////////////////////////////
//

CColumn::CColumn() 
{
	m_pCol = NULL;
}

CColumn::~CColumn() 
{
}

STDMETHODIMP CColumn::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IColumn,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CColumn::get_ProviderTypeName(BSTR * pVal)
{
	SysAllocString(m_pCol->GetProviderTypeName());
	return S_OK;
}

STDMETHODIMP CColumn::put_ProviderTypeName(BSTR newVal)
{
	m_pCol->SetProviderTypeName((WCHAR*)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Suffix(BSTR * pVal)
{
	
	*pVal = SysAllocString(m_pCol->GetSuffix());

	return S_OK;
}

STDMETHODIMP CColumn::put_Suffix(BSTR newVal)
{
	m_pCol->SetSuffix((WCHAR *)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Prefix(BSTR * pVal)
{
	*pVal = SysAllocString(m_pCol->GetPrefix());
	return S_OK;
}

STDMETHODIMP CColumn::put_Prefix(BSTR newVal)
{
	m_pCol->SetPrefix((WCHAR *)newVal);
	return S_OK;
}

STDMETHODIMP CColumn::get_Name(BSTR * pVal)
{
	*pVal = SysAllocString(m_pCol->GetColName());
	
	return S_OK;
}

STDMETHODIMP CColumn::put_Name(BSTR newVal)
{
	m_pCol->SetColName((WCHAR *)newVal);
	return S_OK;
}

STDMETHODIMP CColumn::get_CreateParams(BSTR * pVal)
{
	*pVal = SysAllocString(m_pCol->GetCreateParams());

	return S_OK;
}

STDMETHODIMP CColumn::put_CreateParams(BSTR newVal)
{
	m_pCol->SetCreateParams((WCHAR *)newVal);
	return S_OK;
}

STDMETHODIMP CColumn::get_ColNum(long * pVal)
{
	*pVal = (ULONG)m_pCol->GetColNum();

	return S_OK;
}

STDMETHODIMP CColumn::put_ColNum(long newVal)
{
	m_pCol->SetColNum((ULONG)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_ProviderType(short * pVal)
{
	*pVal = (SHORT)m_pCol->GetProviderType();

	return S_OK;
}

STDMETHODIMP CColumn::put_ProviderType(short newVal)
{
	m_pCol->SetProviderType((DBTYPE)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_SubType(short * pVal)
{

	*pVal = (SHORT)m_pCol->GetSubType();

	return S_OK;
}

STDMETHODIMP CColumn::put_SubType(short newVal)
{
	m_pCol->SetSubType((DBTYPE)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Precision(long * pVal)
{
	*pVal = (long)m_pCol->GetPrecision();

	return S_OK;
}

STDMETHODIMP CColumn::put_Precision(long newVal)
{
	m_pCol->SetPrecision((BYTE)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_ColumnSize(long * pVal)
{
	*pVal = (long)m_pCol->GetColumnSize();

	return S_OK;
}

STDMETHODIMP CColumn::put_ColumnSize(long newVal)
{
	m_pCol->SetColumnSize((ULONG)newVal);

	return S_OK;
}


STDMETHODIMP CColumn::get_IsNullable(BOOL * pVal)
{
	*pVal = m_pCol->GetNullable();

	return S_OK;
}

STDMETHODIMP CColumn::put_IsNullable(BOOL newVal)
{
	m_pCol->SetNullable(newVal);
	return S_OK;
}

STDMETHODIMP CColumn::get_Unsigned(BOOL * pVal)
{
	*pVal = m_pCol->GetUnsigned();

	return S_OK;
}

STDMETHODIMP CColumn::put_Unsigned(BOOL newVal)
{
	m_pCol->SetUnsigned(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Scale(short * pVal)
{
	*pVal = m_pCol->GetScale();

	return S_OK;
}

STDMETHODIMP CColumn::put_Scale(short newVal)
{
	m_pCol->SetScale((BYTE) newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_MinScale(short * pVal)
{
	*pVal = m_pCol->GetMinScale();

	return S_OK;
}

STDMETHODIMP CColumn::put_MinScale(short newVal)
{
	m_pCol->SetMinScale(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_MaxScale(short * pVal)
{
	*pVal = m_pCol->GetMaxScale();

	return S_OK;
}

STDMETHODIMP CColumn::put_MaxScale(short newVal)
{
	m_pCol->SetMaxScale(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_AutoInc(BOOL * pVal)
{
	*pVal = m_pCol->GetAutoInc();

	return S_OK;
}

STDMETHODIMP CColumn::put_AutoInc(BOOL newVal)
{
	m_pCol->SetAutoInc(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Unique(BOOL * pVal)
{
	*pVal = m_pCol->GetUnique();

	return S_OK;
}

STDMETHODIMP CColumn::put_Unique(BOOL newVal)
{

	m_pCol->SetUnique(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Updateable(BOOL * pVal)
{
	*pVal = m_pCol->GetUpdateable();

	return S_OK;
}

STDMETHODIMP CColumn::put_Updateable(BOOL newVal)
{
	m_pCol->SetUpdateable(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_Searchable(long * pVal)
{
	*pVal =(long) m_pCol->GetSearchable();

	return S_OK;
}

STDMETHODIMP CColumn::put_Searchable(long newVal)
{
	m_pCol->SetSearchable((ULONG)newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_UseInSQL(BOOL * pVal)
{
	*pVal = m_pCol->GetUseInSQL();

	return S_OK;
}

STDMETHODIMP CColumn::put_UseInSQL(BOOL newVal)
{
	m_pCol->SetUseInSQL(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_IsLong(BOOL * pVal)
{
	*pVal = m_pCol->GetIsLong();

	return S_OK;
}

STDMETHODIMP CColumn::put_IsLong(BOOL newVal)
{
	m_pCol->SetIsLong(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_IsFixedLength(BOOL * pVal)
{
	*pVal = m_pCol->GetIsFixedLength();

	return S_OK;
}

STDMETHODIMP CColumn::put_IsFixedLength(BOOL newVal)
{
	m_pCol->SetIsFixedLength(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_CaseSensitive(BOOL * pVal)
{
	*pVal = m_pCol->GetCaseSensitive();

	return S_OK;
}

STDMETHODIMP CColumn::put_CaseSensitive(BOOL newVal)
{
	m_pCol->SetCaseSensitive(newVal);

	return S_OK;
}

STDMETHODIMP CColumn::get_DefaultValue(VARIANT * pVal)
{
	*pVal = m_pCol->GetDefaultValue();

	return S_OK;
}

STDMETHODIMP CColumn::put_DefaultValue(VARIANT newVal)
{
	m_pCol->SetDefaultValue(newVal);
	return S_OK;
}

STDMETHODIMP CColumn::get_HasDefault(BOOL * pVal)
{
	*pVal = m_pCol->GetHasDefault();

	return S_OK;
}

STDMETHODIMP CColumn::put_HasDefault(BOOL newVal)
{
	m_pCol->SetHasDefault(newVal);

	return S_OK;
}

BOOL CColumn::SetCol(CCol & Col)
{
	if (m_pCol)
	{
		delete m_pCol;
		m_pCol = NULL;
	}

	m_pCol = new CCol(Col);

	return (m_pCol != NULL);
}
