// ParseInitFile.cpp : Implementation of CADOPrivApp and DLL registration.

#include "stdafx.h"
#include "ADOPriv.h"
#include <privlib.h> //privlib
#include "Column.h"
#include "ModInfo.h"
#include <fstream>
#include "ParseInitFile.h"


//////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//

CParseFile::CParseFile() 
{
	m_pModuleInfo = NULL;
	m_pParser = NULL;
	m_posHead = m_posTail = 0;
	m_pPosition = NULL;
	m_cColumn = 0;
	m_cRow = 0;

}

CParseFile::~CParseFile() 
{
	ATLTRACE(_T("CParseFile::~CParseFile()"));
	if (m_pPosition)
		delete [] m_pPosition;
}	

STDMETHODIMP CParseFile::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IParseInitFile,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CParseFile::get_ColumnCount(long * pVal)
{
	ASSERT(m_pParser != NULL);
	LONG	i = 0L;
	CCol	Coll;
	LONG	lCount = 0L;
	POSITION	pos;
	
	m_posHead = m_pParser->GetColList().GetHeadPosition();
	m_posTail = m_pParser->GetColList().GetTailPosition();

	*pVal = lCount = (LONG) m_pParser->GetColumnCount();
	
	if (*pVal)
	{
		if (m_pPosition == NULL)
		{
			m_pPosition = new POSITION[lCount+1];
			
			m_pPosition[i++] = pos = m_posHead;
			while (i < lCount )
			{
				Coll = m_pParser->GetColList().GetNext(pos);
				m_pPosition[i++] = pos;
			}

		}
	}
	m_cColumn = lCount; //save

	
	return S_OK;
}

STDMETHODIMP CParseFile::get_RowCount(long * pVal)
{
	ASSERT(m_pParser != NULL);

	*pVal = m_cRow = (LONG) m_pParser->GetRowCount();

	return S_OK;
}

STDMETHODIMP CParseFile::Init(BSTR FileName, BOOL * pVal)
{
	ASSERT(m_pParser != NULL);

	WCHAR*	pwszFileName = (LPWSTR)FileName;
	*pVal = m_pParser->Init(pwszFileName);
	
	return S_OK;
}

STDMETHODIMP CParseFile::GetNextLine(LONG * plLength)
{
	HRESULT	hr = S_OK;

	return hr;
}

STDMETHODIMP CParseFile::FindSection(BSTR Section, BOOL * pVal)
{
	//don't seem that we need it

	return S_OK;
}


STDMETHODIMP CParseFile::FetchRow(LONG Row, BOOL * pVal)
{
	HRESULT	hr = S_OK;
	
	//Don't need this either, it just seems like a helper
	return(hr);
}


STDMETHODIMP CParseFile::GetData(LONG Row, LONG Column, BSTR * Data, SHORT * pVal)
{
	ASSERT(m_pParser != NULL);
	ASSERT((Row > 0) && (Row <= m_cRow));
	
	HRESULT	hr = S_OK;
	WCHAR*	pwszData = new WCHAR[1024];

	hr = m_pParser->GetData((ULONG)Row, (ULONG)Column, pwszData, (USHORT *)pVal);
	
	if (!FAILED(hr))
	{
		SysFreeString(*Data);
		*Data = SysAllocString(pwszData);
	}
	delete [] pwszData;

	return hr;
}

STDMETHODIMP CParseFile::GetQuery(LONG eQuery, BSTR * pVal)
{
	ASSERT(m_pParser != NULL);

	WCHAR*	pwsz = m_pParser->GetQuery((EQUERY)eQuery);

	SysFreeString(*pVal);
	*pVal = SysAllocString(pwsz);
	
	if (pwsz)
		return(S_FALSE);

	return S_OK;
}

STDMETHODIMP CParseFile::GetRowOffset(LONG Row, LONG * pVal)
{
	//No need
	return S_OK;
}




HRESULT CParseFile::CreateColumn(IColumn * * ppVal)
{
	HRESULT	hr = S_OK;
	
	hr = CoCreateInstance(CLSID_Column, NULL, CLSCTX_INPROC_SERVER, IID_IColumn,(void **)ppVal); 
	
	if (*ppVal)
	{
		ATLTRACE(_T("Hello world"));
	}
	return(hr);
}


STDMETHODIMP CParseFile::putref_ModInfo(IModInfo * newVal)
{
	LONG	lCount = 0;
	CModuleInfo*	pModuleInfo = (CModuleInfo *)newVal;

	m_pModuleInfo = newVal;
	m_pParser  = pModuleInfo->GetModInfo()->GetParseObject();
	return S_OK;
}

STDMETHODIMP CParseFile::GetColumn(SHORT Index, IColumn * * ppVal)
{
	ASSERT((Index > -1) && (Index <= m_cColumn));
	CColumn*	pColumn = NULL;
	HRESULT		hr = S_OK;
	BOOL		fResult = TRUE;
	LONG		lCount = 0L;

	if (m_pPosition == NULL)
		this->get_ColumnCount(&lCount);
	//check boundary here
	hr = this->CreateColumn(ppVal);

	if (FAILED(hr))
		return(hr);

	pColumn = (CColumn *)(*ppVal);

	if (pColumn)
	{
		fResult = pColumn->SetCol(m_pParser->GetColList().GetAt(m_pPosition[Index])); // + Index);
		
		if (!fResult)
			return(S_FALSE);
	}

	return S_OK;
}
