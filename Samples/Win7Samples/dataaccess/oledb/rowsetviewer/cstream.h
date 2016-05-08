//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CSTREAM.H
//
//-----------------------------------------------------------------------------------

#ifndef _CSTREAM_H_
#define _CSTREAM_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Class CStream
// 
///////////////////////////////////////////////////////////////////////////////
class CStream : public CAsynchBase
{
public:
	//Constructors
	CStream(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CStream();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);


	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Stream";				} 
	virtual UINT			GetObjectMenu()			{ return IDM_STREAMMENU;		}
	virtual LONG			GetObjectImage()		{ return IMAGE_STREAM;			}
	virtual REFIID			GetDefaultInterface()	{ return IID_ISequentialStream;	}

	virtual	void			OnDefOperation();
	virtual HRESULT			DisplayObject();

	//Helpers
	virtual HRESULT			ReadBytes(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, DBLENGTH cbMaxSize, void* pBuffer, DBLENGTH* pcbRead = NULL);
	virtual HRESULT			WriteBytes(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, void* pBuffer, DBLENGTH* pcbWritten = NULL);

	//Helpers
	virtual HRESULT			ReadString(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, DBLENGTH ulMaxSize, WCHAR* pwszBuffer, DBLENGTH* pcbRead = NULL);
	virtual HRESULT			WriteString(REFIID riid, DBLENGTH ulOffset, DBLENGTH cBytes, WCHAR* pwszBuffer, DBLENGTH* pcbWritten = NULL);

	virtual HRESULT			Stat(STATSTG* pSTATSTG);
	virtual HRESULT			Clone(IStream** ppStream);
	virtual HRESULT			Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition);

	//[MANDATORY]
	ISequentialStream*		m_pISequentialStream;

	//[OPTIONAL]
	IStream*				m_pIStream;
	IStorage*				m_pIStorage;
	ILockBytes*				m_pILockBytes;

	//Data
	DBTYPE					m_wType;
};




///////////////////////////////////////////////////////////////////////////////
// Class CStorageBuffer
// 
///////////////////////////////////////////////////////////////////////////////
class CStorageBuffer : public IStream, /*ISequentialStream*/ public ILockBytes
{
public:
	//Constructors
	CStorageBuffer();
	virtual ~CStorageBuffer();
	
	//Helpers
	STDMETHODIMP			Seek(LONG lOffset, DWORD dwOrigin = STREAM_SEEK_SET, ULONG* pulNewPosition = NULL);
	STDMETHODIMP			ReadBuffer(ULONG ulOffset, void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHODIMP			WriteBuffer(ULONG ulOffset, const void* pv, ULONG cb, ULONG* pcbRead);

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
	//Data

private:
	ULONG			m_cRef;			// reference count

	void*			m_pBuffer;		// buffer
	ULONG			m_cMaxSize;     // Maximum allocated buffer size
	ULONG			m_iPos;			// current index position in the buffer
};


///////////////////////////////////////////////////////////////////////////////
// Class CFileStream
// 
// Implementation of stream interface on top of a file
///////////////////////////////////////////////////////////////////////////////
class CFileStream :	public IStream
{
public:
	//constructors
	CFileStream();
	virtual ~CFileStream();

	//Helpers
	HRESULT	OpenFile(WCHAR* pwszFile, WCHAR* pwszMode);
	HRESULT	CloseFile();

	//IUnknown
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
	
	//IStream
	STDMETHODIMP Read( 
            /* [out] */		void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbRead);
        
    STDMETHODIMP Write( 
            /* [in] */		const void* pv,
            /* [in] */		ULONG cb,
            /* [out] */		ULONG* pcbWritten);

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
 
	
protected:
	FILE*			m_pFile;

private:
	ULONG			m_cRef;
};

	
#endif //_CSTREAM_H_