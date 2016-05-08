//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DATASRC.H | CDataSource base object and contained interface
// definitions
//
//
#ifndef _DATASRC_H_
#define _DATASRC_H_

#include "fileio.h"
#include "baseobj.h"


// Forward declarations ------------------------------------------------------

class	CDataSource;
class   CImpIDBInitialize;
class   CImpIDBProperties;
class   CImpIDBCreateSession;
class	CImpIDBInfo;
class   CImpIPersist;
class	CImpIServiceProvider;

typedef CDataSource*			PCDATASOURCE;
typedef CImpIDBInitialize*      PIMPIDBINITIALIZE;
typedef CImpIDBProperties*      PIMPIDBProperties;
typedef CImpIDBCreateSession*   PIMPIDBCREATESESSION;
typedef CImpIDBInfo*			PIMPIDBINFO;
typedef CImpIPersist*			PIMPIPERSIST;


// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CDataSource | Containing class for all interfaces on the Datasource 
// CoType Object
//
class CDataSource : public CBaseObj					//@base public | CBaseObj
{
	//	Contained interfaces are friends
	friend class CImpIDBInitialize;
    friend class CImpIDBInfo;
	friend class CImpIDBProperties;
    friend class CImpIDBCreateSession;
    friend class CImpIPersist;
    friend class CImpIServiceProvider;
    friend class CDBSession;

	protected: //@access protected
		//@cmember Reference count
		DBREFCOUNT					m_cRef;						
		//@cmember Path Name
		WCHAR						m_wszPath[MAX_PATH];
        //@member flag == TRUE if this Data Source object is an an initialized state
        BOOL                        m_fDSOInitialized;    
        //@member flag == TRUE if DBSession object has been created
        BOOL                        m_fDBSessionCreated;
        //@member Utility object to manage properties
        PCUTILPROP                  m_pUtilProp;
		//@cmember Contained IDBInitialize
		PIMPIDBINITIALIZE			m_pIDBInitialize;			
        //@cmember Contained IDBProperties
        PIMPIDBProperties           m_pIDBProperties;
		//@cmember Contained IDBInfo
		PIMPIDBINFO					m_pIDBInfo;
        //@member contained IDBCreateSession
        PIMPIDBCREATESESSION        m_pIDBCreateSession;
        //@member contained IPersist
        CImpIPersist*				m_pIPersist;
        //@member contained IServiceProvider
        CImpIServiceProvider*		m_pIServiceProvider;

	public: //@access public
		//@cmember Constructor		 
		 CDataSource(LPUNKNOWN);
		//@cmember Destructor
		~CDataSource(void);

		//@cmember Intitialization Routine
		BOOL FInit(void);

		//@cmember Controlling IUnknown
		LPUNKNOWN					m_pUnkOuter;				

		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);
		
		//@cmember Set the DBSessionCreated flag to FALSE
		inline VOID				RemoveSession(void)		{ m_fDBSessionCreated = FALSE; }
		//@cmember Return the DataSource path
		inline WCHAR *			GetFilePath(void)		{ return m_wszPath; };
		//@cmember OpenFile
		STDMETHODIMP			OpenFile(WCHAR* pwszFileName, CFileIO** ppCFileIO);
};

typedef CDataSource *PCDATASOURCE;


//----------------------------------------------------------------------------
// @class CImpIDBInitialize | Contained IDBInitialize class
//
class CImpIDBInitialize : public IDBInitialize		//@base public | IDBInitialize
{
	private:  //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:  //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIDBInitialize);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBInitialize members
		//@cmember Initialize Method
	    STDMETHODIMP			Initialize(void);
		//@cmember Uninitialize Method
        STDMETHODIMP            Uninitialize(void);    
};



//----------------------------------------------------------------------------------------
// @class CImpIDBCreateSession   | contained IDBCreateSession class


class CImpIDBCreateSession : public IDBCreateSession    //@base public | IDBCreateSession
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIDBCreateSession);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

        // IDBCreateSession method
        //@cmember CreateSession method
        STDMETHODIMP    CreateSession( IUnknown*, REFIID, IUnknown** );
};



//----------------------------------------------------------------------------
// @class CImpIDBProperties | Contained IDBProperties class
//
class CImpIDBProperties : public IDBProperties		//@base public | IDBProperties
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIDBProperties);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBProperties member functions

        //@cmember GetProperties method
        STDMETHODIMP GetProperties
					(
						ULONG				cPropertySets,		
				      	const DBPROPIDSET	rgPropertySets[], 	
			        	ULONG*              pcProperties, 	
					 	DBPROPSET**			prgProperties 	    
		        	);

        //@cmember GetPropertyInfo method
        STDMETHODIMP    GetPropertyInfo
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
};

//----------------------------------------------------------------------------
// @class CImpIDBInfo | Contained IDBInfo class
//
class CImpIDBInfo : public IDBInfo		//@base public | IDBInfo
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIDBInfo);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBProperties member functions
        
        //@cmember GetKeywords method
        STDMETHODIMP    GetKeywords
                        (
	                        LPWSTR*			ppwsKeywords
                        );
        
        //@cmember GetLiteralInfo method
        STDMETHODIMP    GetLiteralInfo
                        (
	                        ULONG           cLiterals,
							const DBLITERAL rgLiterals[ ],
							ULONG*          pcLiteralInfo,
							DBLITERALINFO** prgLiteralInfo,
							WCHAR**         ppCharBuffer
                        );
};


//----------------------------------------------------------------------------------------
// @class CImpIPersist   | contained IPersist class


class CImpIPersist : public IPersist    //@base public | IPersist
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIPersist);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

        // IPersist method
        //@cmember GetClassID method
        STDMETHODIMP    GetClassID( CLSID *pClassID );
};



////////////////////////////////////////////////////////
// CImpIServiceProvider
//
////////////////////////////////////////////////////////
class CImpIServiceProvider : public IServiceProvider		//@base public | IServiceProvider
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDataSource)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDataSource, CImpIServiceProvider);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

        // IServiceProvider method
        //@cmember QueryService method
        STDMETHODIMP    QueryService( REFGUID guidService, REFIID riid, void** ppvObject);
};


////////////////////////////////////////////////////////
// CImpISpecifyPropertyPages
//
////////////////////////////////////////////////////////
class CImpISpecifyPropertyPages : public ISpecifyPropertyPages		//@base public | ISpecifyPropertyPages
{
	private:        //@access private
		ULONG			m_cRef;

	public:         //@access public
 		CImpISpecifyPropertyPages()
		{
			m_cRef		= 0;
		}
 		virtual ~CImpISpecifyPropertyPages()
		{
		}
		
		DEFINE_ADDREF_RELEASE
		STDMETHODIMP	QueryInterface(REFIID riid, LPVOID *ppv)
		{
			if(!ppv)
				return E_INVALIDARG;

			if(riid == IID_IUnknown)
				*ppv = (IUnknown*)this;
			else if(riid == IID_ISpecifyPropertyPages)
				*ppv = (ISpecifyPropertyPages*)this;
			else
			{
				*ppv = NULL;
				return E_NOINTERFACE;
			}

			SAFE_ADDREF(*ppv);
			return S_OK;
		}

		//ISpecifyPropertyPages member functions
        STDMETHODIMP	GetPages(CAUUID* pPages)
		{
			//Outer array is consumer allocated.
			HRESULT hr = E_OUTOFMEMORY;
			if(!pPages)
				return E_INVALIDARG;
		
			//Inner sets are provider allocaed
			pPages->cElems = 2;
			SAFE_ALLOC(pPages->pElems, GUID, pPages->cElems);

			//Fill in the array
			pPages->pElems[0] = CLSID_SampProvConnectionPage;
			pPages->pElems[1] = CLSID_SampProvAdvancedPage;
			hr = S_OK;

		CLEANUP:
			return hr;
		}
};


////////////////////////////////////////////////////////
// CPropertyPage
//
////////////////////////////////////////////////////////
class CPropertyPage : public IPropertyPage, public IPersistPropertyBag		//@base public | IPropertyPage
{
	protected:        //@access 
		ULONG			m_cRef;
		CLSID			m_clsid;
		INT				m_iDialogID;
		HWND			m_hWnd;

		IPropertyPageSite* m_pIPropertyPageSite;

	public:         //@access public
 		CPropertyPage(REFCLSID clsid, INT iDialogID);
 		virtual ~CPropertyPage();

		//Dialog Proc
		static INT_PTR WINAPI DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		virtual BOOL		  HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		//IUnknown
		DEFINE_ADDREF_RELEASE;
		STDMETHODIMP	QueryInterface(REFIID riid, LPVOID *ppv);

		//IPropertyPage
        STDMETHODIMP SetPageSite(IPropertyPageSite* pPageSite);
        STDMETHODIMP Activate(HWND hWndParent, LPCRECT pRect, BOOL bModal);
        STDMETHODIMP Deactivate();
        
        virtual STDMETHODIMP GetPageInfo( 
            /* [out] */ PROPPAGEINFO __RPC_FAR *pPageInfo)
		{
			ASSERT(pPageInfo);
			ASSERT(pPageInfo->cb == sizeof(PROPPAGEINFO));
			memset(pPageInfo, 0, sizeof(PROPPAGEINFO));
			return S_OK;
		}

        STDMETHODIMP SetObjects( 
            /* [in] */ ULONG cObjects,
            /* [size_is][in] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk)
		{
			return E_NOTIMPL;
		}
        
        STDMETHODIMP Show(UINT nCmdShow);
        STDMETHODIMP Move(LPCRECT pRect);
        STDMETHODIMP IsPageDirty();
        STDMETHODIMP Apply();
        
        STDMETHODIMP Help( 
            /* [in] */ LPCOLESTR pszHelpDir)
		{
			return E_NOTIMPL;
		}
        
        STDMETHODIMP TranslateAccelerator(MSG* pMsg);

		//IPersistPropertyBag
        STDMETHODIMP InitNew( void)
		{
			return E_NOTIMPL;
		}
        
        STDMETHODIMP Load( 
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog)
		{
			return S_OK;
		}
        
        STDMETHODIMP Save( 
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ BOOL fClearDirty,
            /* [in] */ BOOL fSaveAllProperties)
		{
			return E_NOTIMPL;
		}
        
        STDMETHODIMP GetClassID( 
            /* [out] */ CLSID __RPC_FAR *pClassID)
		{
			if(!pClassID)
				return E_INVALIDARG;

			*pClassID = m_clsid;
			return S_OK;
		}
};


////////////////////////////////////////////////////////
// CDSLConnectionPage
//
////////////////////////////////////////////////////////
class CDSLConnectionPage : public CPropertyPage
{
	public:         //@access public
		CDSLConnectionPage();
		virtual ~CDSLConnectionPage();

		//Dialog
		virtual BOOL	HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		//Overloaded methods
        STDMETHODIMP Load( 
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog);

		STDMETHODIMP Save( 
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ BOOL fClearDirty,
            /* [in] */ BOOL fSaveAllProperties);

	protected:
		//data
		VARIANT		m_vDataSource;
};


////////////////////////////////////////////////////////
// CDSLAdvancedPage
//
////////////////////////////////////////////////////////
class CDSLAdvancedPage : public CPropertyPage
{
	public:         //@access public
		CDSLAdvancedPage();
		virtual ~CDSLAdvancedPage();

		//Dialog
		virtual BOOL	HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		//data
};


#endif

