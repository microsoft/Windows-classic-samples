#ifndef __UTIL__H__
#define __UTIL__H__

#ifdef __USEPRIVLIB

typedef BOOL (*fCreateModInfo)(CThisTestModule* pCThisTestModule);

#else

HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cStrLen);
CHAR* ConvertToMBCS(WCHAR* pwsz);
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cStrLen);
WCHAR* ConvertToWCHAR(CHAR* psz);

BOOL	IsFixedLength(DBTYPE dbtype);
BOOL	IsNumericType(DBTYPE dbtype);
BOOL	AddCharToNumericVal(WCHAR wLetter,DB_NUMERIC * pNumeric);



/////////////// CLIST ///////////////////////////

/////////////////////////////////////////////////////////////////////////////


template<class TYPE>
class CListPriv 
{
public:
	CListPriv(INT nBlockSize = 10);
	~CListPriv();

	INT		GetCount() const;
	BOOL	IsEmpty() const;

	TYPE*	Item(SHORT iIndex);
	BOOL	Append(TYPE* pItem);
	BOOL	RemoveAll();
protected:
	INT		m_nBlockSize;
	INT		m_cCount;
	INT		m_cTotal;
	ULONG*	m_plData;
};

template<class TYPE>
CListPriv<TYPE>::CListPriv(INT nBlockSize) 
{
	m_nBlockSize = nBlockSize;
	m_cCount = 0;
	m_cTotal = 0;
	m_plData = NULL;
}

template<class TYPE>
CListPriv<TYPE>::~CListPriv() 
{
	//this->RemoveAll();

	if (m_plData)
		delete [] m_plData;
}

template<class TYPE>
INT		CListPriv<TYPE>::GetCount() const
{
	return(m_cCount);
}


template<class TYPE>
BOOL	CListPriv<TYPE>::IsEmpty() const
{
	return(m_cCount == 0);
}

template<class TYPE>
TYPE*	CListPriv<TYPE>::Item(SHORT iIndex)
{
	if (iIndex < m_cCount)
		return((TYPE *)m_plData[iIndex]);
	else
		return(NULL);
}

template<class TYPE>
BOOL	CListPriv<TYPE>::Append(TYPE* pItem)
{
	if (m_cCount == m_cTotal)
	{
		ULONG*	plNewData = new ULONG[m_cTotal + m_nBlockSize];

		if (!plNewData)
			return(FALSE);

		memcpy(plNewData, m_plData, sizeof(ULONG) * m_cTotal);
		m_cTotal += m_nBlockSize;
		delete [] m_plData;
		m_plData = plNewData;

	}

	if (m_cCount < m_cTotal)
	{
		m_plData[m_cCount++] = (ULONG)pItem;
	}
	return(TRUE);
}

template<class TYPE>
BOOL	CListPriv<TYPE>::RemoveAll()
{	
	
 // 	for(INT i=0; i < m_cCount; i++)
 // 	{
 // 		delete (this->Item(i));
 // 	}

	m_cCount = 0;

	return(TRUE);
}

#endif //__USEPRIVLIB

#endif //__UTIL__H__