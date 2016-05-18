//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module BINDER.H | BINDER base object and contained interface
// definitions
//
//

#ifndef _BINDER_H_
#define _BINDER_H_


#include "baseobj.h"

// Forward declarations ------------------------------------------------------

class CImpIBindResource;
class CImpIDBBinderProperties;
class CImpICreateRow;

typedef CImpIBindResource *			PIMPIBINDRESOURCE;
typedef CImpIDBBinderProperties *	PIMPIDBBINDERPROPERTIES;
typedef CImpICreateRow *			PIMPICREATEROW;


// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CBinder | Binder object. Containing class for all Binder interfaces
// Object
//
class CBinder : public CBaseObj	//@base public | IUnknown
{
	//	Contained interfaces are friends
	friend class CImpIBindResource;
	friend class CImpIDBBinderProperties;
	friend class CImpICreateRow;

	protected: //@access protected

		//@member Utility object to manage properties
		PCUTILPROP					m_pUtilProp;

		// Interface and OLE Variables

		//@cmember Reference count
		DBREFCOUNT					m_cRef;		

		//@cmember Contained IBindResource
		PIMPIBINDRESOURCE			m_pIBindResource;
		//@cmember Contained IBinderProperties
		PIMPIDBBINDERPROPERTIES		m_pIDBBinderProperties;
		//@cmember Contained ICreateRow
		PIMPICREATEROW				m_pICreateRow;

	public: //@access public
		//@cmember Constructor
		CBinder(LPUNKNOWN);
		//@cmember Destructor
		virtual ~CBinder(void);
		
		
		//@cmember Intitialization Routine
		BOOL FInit();

		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);
};

typedef CBinder* PCBINDER;


//----------------------------------------------------------------------------
// @class CImpIBindResource | Contained IBindResource class
//
class CImpIBindResource : public IBindResource			//@base public | IBindResource
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBaseObj)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBaseObj, CImpIBindResource);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IBindResource members
		STDMETHODIMP Bind
					(
						IUnknown *			pUnkOuter, 
						LPCOLESTR			pwszURL,
						DWORD				dwBindFlags,
						REFGUID				rguid,
						REFIID				riid,
						IAuthenticate *		pAuthenticate,
						DBIMPLICITSESSION *	pImplSession,
						DWORD *				pdwBindStatus,
						IUnknown **			ppUnk
					);

		// non members
		HRESULT		BindDSO
					(
						IUnknown *			pUnkOuter,
						REFIID				riid,	
						BOOL				fWaitForInit,
						WCHAR *				pwszDataSource,
						IUnknown **			ppUnk	
					);

		HRESULT		BindSession
					(
						IUnknown *			pUnkOuter,
						REFIID				riid,	
						WCHAR *				pwszDataSource,
						IUnknown **			ppUnk	
					);

		HRESULT		BindSession
					(
						DBIMPLICITSESSION *	pImplSession,
						WCHAR *				pwszDataSource
					);

		HRESULT		BindRowset
					(
						IUnknown *			pUnkOuter,
						REFIID				riid,	
						DBIMPLICITSESSION *	pImplSession,
						WCHAR *				pwszDataSource,
						WCHAR *				pwszFile,
						IUnknown **			ppUnk	
					);

		HRESULT		BindRow
					(
						IUnknown *			pUnkOuter,	
						REFIID				riid,	
						DBIMPLICITSESSION * pImplSession,
						WCHAR *				pwszDataSource,
						WCHAR *				pwszFile,	
						ULONG				ulRowNum,	
						IUnknown **			ppUnk		
					);

		HRESULT		BindStream
					(
						IUnknown *			pUnkOuter,	
						REFIID				riid,	
						DBIMPLICITSESSION * pImplSession,
						WCHAR *				pwszDataSource,
						WCHAR *				pwszFile,	
						ULONG				ulRowNum,	
						IUnknown **			ppUnk		
					);

		HRESULT		ValidateBindArgs
					(
						IUnknown *			pUnkOuter,
						LPCOLESTR			pwszURL,
						DBBINDURLFLAG		dwBindFlags,
						REFGUID				rguid,
						REFIID				riid,
						DBIMPLICITSESSION * pImplSession,
						IAuthenticate *		pAuthenticate,
						DWORD *				pdwBindStatus,
						IUnknown **			ppUnk
					);

		HRESULT		ParseURL
					(
						LPCOLESTR			pwszURL,
						WCHAR **			ppwszDataSource,
						WCHAR **			ppwszTableName,
						ULONG *				pulRowNum
					);

		BOOL		FindKeyword
					(
						LPCOLESTR			pwszURL,
						LPCOLESTR			pwszKeyword,
						WCHAR **			ppwszToken
					);
};


//----------------------------------------------------------------------------
// @class CImpICreateRow | Contained ICreateRow class
//
class CImpICreateRow : public ICreateRow					//@base public | ICreateRow
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBinder)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBinder, CImpICreateRow);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	ICreateRow members
		//@cmember CreateRow method
		STDMETHODIMP CreateRow
					(
						IUnknown *			pUnkOuter, 
						LPCOLESTR			pwszURL,
						DWORD				dwBindFlags,
						REFGUID				rguid,
						REFIID				riid,
						IAuthenticate *		pAuthenticate,
						DBIMPLICITSESSION *	pImplSession,
						DWORD *				pdwBindStatus,
						LPOLESTR *			ppwszNewURL,
						IUnknown **			ppUnk
					);
};


//----------------------------------------------------------------------------
// @class CImpIBinderProperties | Contained IBinderProperties class
//
class CImpIDBBinderProperties : public IDBBinderProperties	//@base public | IDBBinderProperties
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBinder)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBinder, CImpIDBBinderProperties);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBProperties member functions
		//@cmember GetProperties method
		STDMETHODIMP GetProperties
		        	(
						ULONG				cPropertySets,		
						const DBPROPIDSET	rgPropertySets[], 	
			        	ULONG*              pcPropertySets, 	
						DBPROPSET**			prgPropertySets 	    
		        	);

		//@cmember GetPropertyInfo method
        STDMETHODIMP	GetPropertyInfo
					( 
						ULONG				cPropertySets, 
						const DBPROPIDSET	rgPropertySets[],
						ULONG*				pcPropertyInfoSets, 
						DBPROPINFOSET**		prgPropertyInfoSets,
						WCHAR**				ppDescBuffer
					);

        //@cmember SetProperties method
        STDMETHODIMP	SetProperties
				 	(
						ULONG				cPropertySets,		
					 	DBPROPSET			rgPropertySets[] 	    
					);

		//@cmember Reset method
        STDMETHODIMP	Reset();
};



#endif //_BINDER_H_

