// Start of the RNCP.h

#ifndef _RNCP_104C2CC1_6D0F_11d3_AC96_00C04F8DB3D5_H_
#define _RNCP_104C2CC1_6D0F_11d3_AC96_00C04F8DB3D5_H_

template <class T>
class CProxyIRowsetNotify : public IConnectionPointImpl<T, &IID_IRowsetNotify, CComDynamicUnkArray>
{
public:
	HRESULT Fire_OnFieldChangeMy(IRowset * pRowset, HROW hRow, DBORDINAL cColumns, DBORDINAL * rgColumns, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
	{
		HRESULT ret = S_OK;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IRowsetNotify* pIRowsetNotify = reinterpret_cast<IRowsetNotify*>(sp.p);
			if (pIRowsetNotify != NULL)
				ret = pIRowsetNotify->OnFieldChange(pRowset, hRow, cColumns, rgColumns, eReason, ePhase, fCantDeny);
		}	return ret;
	
	}
	
	HRESULT Fire_OnRowChangeMy(IRowset * pRowset, DBCOUNTITEM cRows, const HROW* rghRows, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
	{
		HRESULT ret = S_OK;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IRowsetNotify* pIRowsetNotify = reinterpret_cast<IRowsetNotify*>(sp.p);
			if (pIRowsetNotify != NULL)
				ret = pIRowsetNotify->OnRowChange(pRowset, cRows, rghRows, eReason, ePhase, fCantDeny);
		}	return ret;
	
	}
	
	HRESULT Fire_OnRowsetChangeMy(IRowset * pRowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
	{
		HRESULT ret = S_OK;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IRowsetNotify* pIRowsetNotify = reinterpret_cast<IRowsetNotify*>(sp.p);
			if (pIRowsetNotify != NULL)
				ret = pIRowsetNotify->OnRowsetChange(pRowset, eReason, ePhase, fCantDeny);
		}	return ret;
	
	}
	
};
#endif

// Start of the RNCP.h