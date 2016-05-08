//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module STREAM.H | CStream base object and contained interfaces
// definitions
//
//

#ifndef _STREAM_H_
#define _STREAM_H_


#include "baseobj.h"

// Forward declarations ------------------------------------------------------

class CImpIGetSourceRow;
class CImpIStream;

// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CStream | Stream object. Containing class for all interfaces on the 
// Stream Object
//
class CStream : public CBaseObj				//@base public | CBaseObj
{	
	//	Contained interfaces are friends
	friend class CImpIStream;
	friend class CImpIGetSourceRow;

	protected: //@access protected
		//@cmember Object that created this stream
		CRow *					m_pParentObj;
		//@cmember data buffer
		void *					m_pBuffer;
		//@cmember data buffer size
		DBLENGTH				m_cMaxSize;
		//@cmember  current index position in the buffer
		ULONG					m_iPos;			
		
		// Interface and OLE Variables

		//@cmember Reference count
		ULONG					m_cRef;	
		//@cmember Contained IGetSourceRow
		CImpIGetSourceRow *		m_pIGetSourceRow;
		//@cmember Contained IStream
		CImpIStream *			m_pIStream;

	public: //@access public
		//@cmember Constructor
		 CStream(LPUNKNOWN);
		//@cmember Destructor
		~CStream(void);

		//@cmember Intitialization Routine
		BOOL FInit(CRow*, ROWBUFF*);

		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP			QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(ULONG)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(ULONG)	Release(void);
};


//----------------------------------------------------------------------------
// @class CImpIGetSourceRow | Contained IGetSourceRow class
//
class CImpIGetSourceRow : public IGetSourceRow	//@base public | IGetSourceRow
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CStream)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CStream, CImpIGetSourceRow);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IGetSourceRow members
		//@cmember GetSourceRow member
        STDMETHODIMP	GetSourceRow(REFIID riid, IUnknown** ppRow);
};


//----------------------------------------------------------------------------
// @class CImpIStream | Contained IStream class
//
class CImpIStream : public IStream				//@base public | IStream
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CStream)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CStream, CImpIStream);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

 		//  ISequentialStream interfaces
		//@cmember Read member
		STDMETHODIMP Read( 
				/* [out] */		void* pv,
				/* [in] */		ULONG cb,
				/* [out] */		ULONG* pcbRead);
        //@cmember Write member
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
};


#endif //_STREAM_H_
