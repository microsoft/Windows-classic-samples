//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTable Header Module | This module contains header information for CStorage
//
// @normal (C) Copyright 1995-1998 Microsoft Corporation.  All Rights Reserved.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CStorage Elements|
//
// @subindex CStorage|
//
//---------------------------------------------------------------------------

#ifndef _CSTORAGE_HPP_
#define _CSTORAGE_HPP_


/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////


//64bit TODO - Change the variables representing offset, number of bytes, etc. after
//it's done in 64 bit COM.

///////////////////////////////////////////////////////////////////////////////
// Class CStorage
// 
///////////////////////////////////////////////////////////////////////////////
class CStorage : public IStream, public ILockBytes
{
public:
	//Constructors
	CStorage();
	virtual ~CStorage();
	
	//Helpers
	virtual BOOL		Clear();
	virtual BOOL		Compare(ULONG cBytes, void* pBuffer);
	STDMETHODIMP		Seek(LONG lOffset);

	//Interface
	virtual IUnknown*	pUnknown()			{ return (IStream*)this;	}
	virtual ULONG		GetTotalRead()		{ return m_cbRead;			}
	virtual ULONG		GetEndReached()		{ return m_cEndReached;		}
	virtual ULONG		GetRefCount()		{ return m_cRef;			}

	virtual void	SetS_OKonEOF(BOOL f)	{ m_fS_OKonEOF = f; return;}
	virtual void	SetQISeqStream(BOOL f)	{ m_fQISeqStream = f; return;}
	virtual void	SetQIStream(BOOL f)		{ m_fQIStream = f; return;}
	virtual void	SetQILockBytes(BOOL f)	{ m_fQILockBytes = f; return;}

	//IUnknown
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
	
 	//ISequentialStream interfaces
	STDMETHODIMP Read( 
            /* [out] */		void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbRead);
        
    STDMETHODIMP Write( 
            /* [in] */		const void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbWritten);

	//IStream interfaces
	STDMETHODIMP Seek(  
							LARGE_INTEGER dlibMove,			//Offset relative to dwOrigin
							DWORD dwOrigin,					//Specifies the origin for the offset
							ULARGE_INTEGER * plibNewPosition);

	STDMETHODIMP SetSize(	ULARGE_INTEGER libNewSize);		//Specifies the new size of the stream object

	STDMETHODIMP CopyTo(	IStream * pstm,					//Points to the destination stream
							ULARGE_INTEGER cb,				//Specifies the number of bytes to copy
							ULARGE_INTEGER * pcbRead,		//Pointer to the actual number of bytes read from the source
							ULARGE_INTEGER * pcbWritten);	//Pointer to the actual number of bytes written to the destination

	STDMETHODIMP Commit(	DWORD grfCommitFlags);			//Specifies how changes are committed

	STDMETHODIMP Revert(	);

	STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset,		//Specifies the byte offset for the beginning of the range
							ULARGE_INTEGER cb,				//Specifies the length of the range in bytes
							DWORD dwLockType);				//Specifies the restriction on accessing the specified range

	STDMETHODIMP UnlockRegion(  ULARGE_INTEGER libOffset,	//Specifies the byte offset for the beginning of the range
								ULARGE_INTEGER cb,			//Specifies the length of the range in bytes
								DWORD dwLockType);			//Specifies the access restriction previously placed on the range);
	STDMETHODIMP Stat(
							STATSTG * pstatstg,				//Location for STATSTG structure
							DWORD grfStatFlag);				//Values taken from the STATFLAG enumeration

	STDMETHODIMP Clone(		IStream ** ppstm );				//Points to location for pointer to the new stream object
 

 	//ILockBytes interfaces
	STDMETHODIMP ReadAt( 
							ULARGE_INTEGER ulOffset,
            /* [out] */		void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbRead);
        
    STDMETHODIMP WriteAt( 
							ULARGE_INTEGER ulOffset,
            /* [in] */		const void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbWritten);

	STDMETHODIMP Flush();

protected:
	BOOL		m_fS_OKonEOF;		//Default - FALSE
	BOOL		m_fQISeqStream;		//Default - TRUE
	BOOL		m_fQIStream;		//Default - TRUE
	BOOL		m_fQILockBytes;		//Default - TRUE

private:

	ULONG		m_cRef;			// reference count

	void*       m_pBuffer;		// Buffer
	ULONG       m_cSize;	    // Stream Size
	ULONG		m_iPos;			// current index position in the buffer

	//Debugging
	ULONG		m_cbRead;		// How many bytes the user has read
	BOOL		m_cEndReached;	// How may times the end was reached
};


///////////////////////////////////////////////////////////////////////////////
// Class CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
extern const GUID IID_IAggregate;

class CAggregate : public IUnknown
{
public:
	CAggregate(IUnknown* pIUnkParent = NULL);
	virtual ~CAggregate();

	//IUnknown
	virtual STDMETHODIMP_(ULONG)	AddRef(void);
	virtual STDMETHODIMP_(ULONG)	Release(void);
	virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);

	//Helpers
	virtual ULONG	GetRefCount();
	virtual HRESULT	SetUnkInner(IUnknown* pIUnkInner);
	virtual HRESULT	ReleaseInner();
	virtual BOOL	VerifyAggregationQI(HRESULT hrReturned, REFIID riidInner, IUnknown** ppIUnknown = NULL, BOOL fInitialized = TRUE);

	//Data
	IUnknown*	m_pIUnkInner;

protected:

	//Data
	IUnknown*	m_pIUnkCheckRefCount;
	ULONG		m_cRef;
	ULONG		m_ulRefParent;
};


///////////////////////////////////////////////////////////////////////////////
// Class CDispatch
// 
///////////////////////////////////////////////////////////////////////////////
class CDispatch : public IDispatch
{
public:
	//Constructors
	CDispatch();
	~CDispatch();
	
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
	STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
	STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, 
		DISPID *rgDispId);
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
		VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

	//Data
	ULONG		m_cRef;
};



#endif //_CSTORAGE_HPP_
