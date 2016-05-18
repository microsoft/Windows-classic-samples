#ifndef _IPERSISTSTREAM_H
#define _IPERSISTSTREAM_H

//////////////////////////////////////////////////////////////////////////////
// IMyPersistStream
// Implements IPersistStream as a template class. 
//
// ToDo: Derive your own class form this class and implement the methods
//				IMyPersistStreamImpl_Load
//				IMyPersistStreamImpl_Save
//				IMyPersistStreamImpl_GetSizeMax
//		 and the member variable
//				m_bRequiresSave
////////////////////////////////////////////////////////////////////////////// 
template <class T>
class ATL_NO_VTABLE IMyPersistStreamImpl
{
public:
	// IUnknown
	//
	STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject) = 0;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IMyPersistStreamImpl)

	// IPersist
	STDMETHOD(GetClassID)(CLSID *pClassID)
	{
		ATLTRACE(_T("IMyPersistStreamImpl::GetClassID\n"));
		T* pT = static_cast<T*>(this);
		*pClassID = pT->GetObjectCLSID();
		return S_OK;
	}

	// IPersistStream
	STDMETHOD(IsDirty)()
	{
		ATLTRACE(_T("IMyPersistStreamImpl::IsDirty\n"));
		T* pT = static_cast<T*>(this);
		return (pT->m_bRequiresSave) ? S_OK : S_FALSE;
	}
	STDMETHOD(Load)(LPSTREAM pStm)
	{
		ATLTRACE(_T("IMyPersistStreamImpl::Load\n"));
		T* pT = static_cast<T*>(this);
		return pT->IMyPersistStreamImpl_Load(pStm);
	}
	STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE(_T("IMyPersistStreamImpl::Save\n"));
		return pT->IMyPersistStreamImpl_Save(pStm, fClearDirty);
	}
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize )
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE(_T("IMyPersistStreamImpl::GetSizeMax\n"));
		return pT->IMyPersistStreamImpl_GetSizeMax(pcbSize);
	}

};

#endif
