//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DATASRC.CPP | CDataSource object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"
#include <shlobj.h>		//SHBrowseForFolder
#include <shellapi.h>	//ShellExecute

// Code ----------------------------------------------------------------------

// CDataSource::CDataSource --------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CDataSource::CDataSource
    (
    LPUNKNOWN pUnkOuter         //@parm IN | Outer Unkown Pointer
    )	// invoke ctor for base class
	: CBaseObj( BOT_DATASOURCE )
{

    //  Initialize simple member vars
    m_cRef              = 0L;
    m_pUnkOuter         = pUnkOuter ? pUnkOuter : this;
    m_wszPath[0]        = L'\0';
    m_fDSOInitialized   = FALSE;
    m_fDBSessionCreated = FALSE;
    m_pUtilProp         = NULL;

    //  Initially, NULL all contained interfaces
    m_pIDBInitialize    = NULL;
    m_pIDBProperties	= NULL;
    m_pIDBInfo			= NULL;
	m_pIDBCreateSession = NULL;
	m_pIPersist			= NULL;
	
	//Data Links optional pages
	m_pIServiceProvider	= NULL;

    // Increment global object count.
    OBJECT_CONSTRUCTED();

    return;
}


// CDataSource::~CDataSource -------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CDataSource:: ~CDataSource
    (
    void
    )
{
	DBREFCOUNT	ulRefCount;

	// Decrement the ref count on the data conversion object
	if( g_pIDataConvert )
	{
		ulRefCount = g_pIDataConvert->Release();

		// Is it gone for good?
		if( !ulRefCount )
			g_pIDataConvert = NULL;
	}

    // Free properties management object
    SAFE_DELETE( m_pUtilProp );

    //  Free contained interfaces
    SAFE_DELETE( m_pIDBInitialize );
    SAFE_DELETE( m_pIDBProperties );
    SAFE_DELETE( m_pIDBInfo );
	SAFE_DELETE( m_pIDBCreateSession );
	SAFE_DELETE( m_pIPersist );
	SAFE_DELETE( m_pIServiceProvider );

    // Decrement global object count.
    OBJECT_DESTRUCTED();

    return;
}


// CDataSource::FInit --------------------------------------------------------
//
// @mfunc Initialize the command Object
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CDataSource::FInit
    (
    void
    )
{
	// Instantiate the data conversion service object
	if( !g_pIDataConvert )
	{
		CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY,
						 NULL,
						 CLSCTX_INPROC_SERVER,
						 IID_IDataConvert,
						 (void **)&g_pIDataConvert);

		//
		// Tell the DC that we are 2.5
		//
		if( g_pIDataConvert )
		{
			DCINFO rgInfo[] = {{DCINFOTYPE_VERSION,{VT_UI4, 0, 0, 0, 0x0}}};
			IDCInfo	*pIDCInfo = NULL;

			if( g_pIDataConvert->QueryInterface(IID_IDCInfo, (void **)&pIDCInfo) == S_OK && 
				pIDCInfo )
			{
				// OLE DB Version 02.50
				V_UI4(&rgInfo->vData) = 0x250;
				pIDCInfo->SetInfo(NUMELEM(rgInfo),rgInfo);
				pIDCInfo->Release();
			}
		}
	}
	else
		// Already instantiated, increment reference count
		g_pIDataConvert->AddRef();

    // Allocate properties management object
    m_pUtilProp = new CUtilProp();

    //Allocate contained interface objects
	//[MANDATORY]
    m_pIDBInitialize    = new CImpIDBInitialize( this, m_pUnkOuter );
    m_pIDBProperties	= new CImpIDBProperties( this, m_pUnkOuter );
    m_pIDBCreateSession = new CImpIDBCreateSession( this, m_pUnkOuter );
	
	//[OPTIONAL]
	m_pIDBInfo			= new CImpIDBInfo( this, m_pUnkOuter );
    m_pIPersist			= new CImpIPersist( this, m_pUnkOuter );
    m_pIServiceProvider	= new CImpIServiceProvider( this, m_pUnkOuter );

    return (BOOL) (m_pUtilProp && m_pIDBInitialize && m_pIDBProperties &&
				   m_pIDBCreateSession && m_pIDBInfo && m_pIPersist && 
				   m_pIServiceProvider);
}


// CDataSource::QueryInterface -----------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use
// QueryInterface to determine which interfaces the called object
// supports.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK          | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG  | One or more arguments are invalid.
//
STDMETHODIMP CDataSource::QueryInterface
    (
    REFIID riid,    //@parm IN | Interface ID of the interface being queried for.
    LPVOID * ppv    //@parm OUT | Pointer to interface that was instantiated
    )
{
    // Is the pointer bad?
    if (ppv == NULL)
        return ResultFromScode( E_INVALIDARG );

    //  Place NULL in *ppv in case of failure
    *ppv = NULL;

    //  This is the non-delegating IUnknown implementation
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
    else if (riid == IID_IDBInitialize)
        *ppv = (LPVOID) m_pIDBInitialize;
	else if (riid == IID_IDBProperties)
        *ppv = (LPVOID) m_pIDBProperties;
    else if (riid == IID_IPersist)
        *ppv = (LPVOID) m_pIPersist;
	else if (riid == IID_IServiceProvider)
		*ppv = (LPVOID) m_pIServiceProvider;
	else
	{
		// These are not valid at Uninit state.
		if( riid == IID_IDBCreateSession )
			*ppv = (LPVOID)m_pIDBCreateSession;
		else if (riid == IID_IDBInfo)
			*ppv = (LPVOID) m_pIDBInfo;

		// Special case for uninitialized.
		if( *ppv && !m_fDSOInitialized )
		{
			*ppv = NULL;			
			return ResultFromScode( E_UNEXPECTED );
		}
	}

    //  If we're going to return an interface, AddRef it first
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
        return ResultFromScode( S_OK );
        }
    else
        return ResultFromScode( E_NOINTERFACE );
}


// CDataSource::AddRef -------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CDataSource::AddRef
     (
     void
     )
{
    return ++m_cRef;
}


// CDataSource::Release ------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CDataSource::Release
     (
     void
     )
{
    if (!--m_cRef)
        {
        delete this;
        return 0;
        }

    return m_cRef;
}


////////////////////////////////////////////////////////
// CDataSource::OpenFile
//
////////////////////////////////////////////////////////
HRESULT		CDataSource::OpenFile(WCHAR* pwszFileName, CFileIO** ppCFileIO)
{
	ASSERT(ppCFileIO);
	HRESULT hr = S_OK;
	WCHAR	wszFullPath[_MAX_PATH + _MAX_PATH + 1]=L"";
	CHAR	szFullPath[_MAX_PATH + _MAX_PATH + 1]= "";
	
	//open and initialize a file object
	CFileIO* pCFileIO = new CFileIO();
	if(!pCFileIO)
		return E_OUTOFMEMORY;

	//Do we have a base directory
	if(m_wszPath[0])
	{
		//Contruct the filename
		StringCchPrintfW(wszFullPath, 
						 sizeof(wszFullPath)/sizeof(WCHAR), 
						 L"%s\\%s",
						 m_wszPath,
						 pwszFileName);

		//Convert to MBCS (stream operations)
		ConvertToMBCS(wszFullPath, szFullPath, _MAX_PATH+_MAX_PATH);
		
		//Now try to initialize the stream, if successful we are done...
		if(SUCCEEDED(hr = pCFileIO->fInit(  szFullPath )))
			goto CLEANUP;
	}

	//Otherwise its a full filename...
	ConvertToMBCS(pwszFileName, szFullPath, _MAX_PATH+_MAX_PATH);
	
	//Now try to initialize the stream...
	hr = pCFileIO->fInit(  szFullPath );

CLEANUP:
	if(FAILED( hr ))
		SAFE_DELETE( pCFileIO );
	*ppCFileIO = pCFileIO;
	return hr;
}



////////////////////////////////////////////////////////
// CImpIServiceProvider::QueryService
//
////////////////////////////////////////////////////////
STDMETHODIMP	CImpIServiceProvider::QueryService
(
	REFGUID guidService,
	REFIID riid,
	void** ppvObject
)
{
	HRESULT hr = S_OK;
	if(!ppvObject)
		return E_INVALIDARG;
	*ppvObject = NULL;

	//DSL property Pages
	if(guidService == OLEDB_SVC_DSLPropertyPages)
	{
		CImpISpecifyPropertyPages* pCImpISpecifyPropertyPages = new CImpISpecifyPropertyPages;
		if(!pCImpISpecifyPropertyPages)
			return E_OUTOFMEMORY;

		return pCImpISpecifyPropertyPages->QueryInterface(IID_ISpecifyPropertyPages, ppvObject);
	}
	else
	{
//		TODO? hr = SVC_E_UNKNOWNSERVICE;
		hr = E_FAIL;
	}

	return hr;
}




////////////////////////////////////////////////////////
// CPropertyPage
//
////////////////////////////////////////////////////////
CPropertyPage::CPropertyPage(REFCLSID clsid, INT iDialogID)
{
	m_cRef		= 0;
	m_clsid		= clsid;
	m_iDialogID	= iDialogID;
	m_hWnd		= NULL;

	m_pIPropertyPageSite = NULL;
}

////////////////////////////////////////////////////////
// ~CPropertyPage
//
////////////////////////////////////////////////////////
CPropertyPage::~CPropertyPage()
{
	SAFE_RELEASE(m_pIPropertyPageSite);
}

////////////////////////////////////////////////////////
// CPropertyPage::QueryInterface
//
////////////////////////////////////////////////////////
STDMETHODIMP	CPropertyPage::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(!ppv)
		return E_INVALIDARG;

	if(riid == IID_IUnknown)
		*ppv = (IUnknown*)(IPropertyPage*)this;
	else if(riid == IID_IPropertyPage)
		*ppv = (IPropertyPage*)this;
	else if(riid == IID_IPersistPropertyBag)
		*ppv = (IPersistPropertyBag*)this;
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	SAFE_ADDREF(*ppv);
	return S_OK;
}

////////////////////////////////////////////////////////
// CPropertyPage::SetPageSite
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::SetPageSite(IPropertyPageSite* pPageSite)
{
	//SetPageSite can call with NULL to indicate to release interface.
	if(!pPageSite)
	{
		SAFE_RELEASE(m_pIPropertyPageSite);
		return S_OK;
	}
	
	//Otherwise...
	ASSERT(m_pIPropertyPageSite == NULL);
	return pPageSite->QueryInterface(IID_IPropertyPageSite, (void**)&m_pIPropertyPageSite);
}

		
////////////////////////////////////////////////////////
// CPropertyPage::Activate
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::Activate
( 
	HWND hWndParent,
	LPCRECT pRect,
	BOOL bModal
)
{
	//Create a modeless dialog...
	m_hWnd = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(m_iDialogID), hWndParent, DialogProc, (LPARAM)this);
	if(m_hWnd)
	{
		return Move(pRect);
	}

	return E_FAIL;
}

		
////////////////////////////////////////////////////////
// CPropertyPage::Deactivate
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::Deactivate()
{
	if(m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	return S_OK;
}

	
////////////////////////////////////////////////////////
// CPropertyPage::TranslateAccelerator
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::TranslateAccelerator(MSG* pMsg)
{
	ASSERT(pMsg);	

	if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return S_FALSE;

	return IsDialogMessage(m_hWnd, pMsg) ? S_OK : S_FALSE;
}


////////////////////////////////////////////////////////
// CPropertyPage::IsPageDirty
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::IsPageDirty()
{
	//Out page is always dirty since we don't want to go through the overhead of 
	//determining if the user used any of the controls on the page, S_OK = Dirty...
	return S_OK;
}

		
////////////////////////////////////////////////////////
// CPropertyPage::Apply
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::Apply()
{
	return S_OK;
}


////////////////////////////////////////////////////////
// CPropertyPage::Show
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::Show(UINT nCmdShow)
{
	ShowWindow(m_hWnd, nCmdShow);
	return S_OK;
}


////////////////////////////////////////////////////////
// CPropertyPage::Move
//
////////////////////////////////////////////////////////
STDMETHODIMP CPropertyPage::Move(LPCRECT pRect)
{
	ASSERT(pRect);
	MoveWindow(m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left,	 pRect->bottom - pRect->top, TRUE);
	return S_OK;
}

	
////////////////////////////////////////////////////////
// CPropertyPage::DialogProc
//
////////////////////////////////////////////////////////
INT_PTR CALLBACK CPropertyPage::DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			//Save the "this" pointer
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			return 0;
		}

		case WM_COMMAND:
		{
			//Get the "this" pointer
			CPropertyPage* pThis = (CPropertyPage*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			return pThis->HandleMessage(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}

////////////////////////////////////////////////////////
// CPropertyPage::HandleMessage
//
////////////////////////////////////////////////////////
BOOL CPropertyPage::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}



////////////////////////////////////////////////////////
// CDSLConnectionPage
//
////////////////////////////////////////////////////////
CDSLConnectionPage::CDSLConnectionPage()	
	: CPropertyPage(CLSID_SampProvConnectionPage, IDD_DSL_CONNECTION)
{
	VariantInit(&m_vDataSource);
}

////////////////////////////////////////////////////////
// ~CDSLConnectionPage
//
////////////////////////////////////////////////////////
CDSLConnectionPage::~CDSLConnectionPage()
{
	VariantClear(&m_vDataSource);
}


////////////////////////////////////////////////////////
// CDSLConnectionPage::HandleMessage
//
////////////////////////////////////////////////////////
BOOL CDSLConnectionPage::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				break;
			}

			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_BROWSE_DIRECTORY:
				{
					//Obtain the current value for the default
					CHAR szDirectory[MAX_QUERY_LEN] = {0};
					SendMessage(GetDlgItem(m_hWnd, IDE_DATASOURCE), WM_GETTEXT, MAX_QUERY_LEN, (LPARAM)szDirectory);
					
					//Display Common Dialog to obtain Directory...
					BROWSEINFO browseInfo;
					memset( &browseInfo, 0, sizeof(browseInfo));
					browseInfo.hwndOwner		= m_hWnd; 
					browseInfo.pidlRoot			= NULL; 
					browseInfo.pszDisplayName	= szDirectory; 
					browseInfo.lpszTitle		= "Browse for default directory";; 
					browseInfo.ulFlags			= 0; 
					browseInfo.lpfn				= NULL; 
					browseInfo.lParam			= 0; 
					browseInfo.iImage			= 0; 

					//Display Common Dialog to obtain File To Load...
					LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
					if(pidl)
					{
						//Obtain the file path from the ITemID...
						if(SHGetPathFromIDList(pidl, szDirectory))
						{
							//Update the Property value on the Dialog...
							SendMessage(GetDlgItem(m_hWnd, IDE_DATASOURCE), WM_SETTEXT, 0, (LPARAM)szDirectory);
						}

						SAFE_FREE(pidl);
					}
					break;
				}
			};
			break;
		}
	};

	return FALSE;
}


////////////////////////////////////////////////////////
// CDSLConnectionPage::Load
//
////////////////////////////////////////////////////////
STDMETHODIMP	CDSLConnectionPage::Load
( 
	IPropertyBag*	pPropBag,
    IErrorLog*	pErrorLog
)
{
	if(!pPropBag)
		return E_INVALIDARG;

	//Delegate
	HRESULT hr = pPropBag->Read(L"Data Source", &m_vDataSource, pErrorLog);

	//Update our dialog (maybe empty...)
	if(SUCCEEDED(hr) && V_VT(&m_vDataSource)==VT_BSTR)
	{
		//Update the Property value on the Dialog...
		wSendMessage(GetDlgItem(m_hWnd, IDE_DATASOURCE), WM_SETTEXT, 0, V_BSTR(&m_vDataSource));
	}

	return hr;
}


////////////////////////////////////////////////////////
// CDSLConnectionPage::Save
//
////////////////////////////////////////////////////////
STDMETHODIMP	CDSLConnectionPage::Save
( 
	IPropertyBag __RPC_FAR *pPropBag,
	BOOL fClearDirty,
	BOOL fSaveAllProperties
)
{
	if(!pPropBag)
		return E_INVALIDARG;

	//Obtain the Property value from the Dialog...
	WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
	wSendMessage(GetDlgItem(m_hWnd, IDE_DATASOURCE), WM_GETTEXT, MAX_QUERY_LEN, wszBuffer);

	//Indicate this page is dirty...
	ASSERT(m_pIPropertyPageSite);
	m_pIPropertyPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

	//Store it into our variant...
	VariantClear(&m_vDataSource);
	V_VT(&m_vDataSource)	= VT_BSTR;
	V_BSTR(&m_vDataSource)	= SysAllocString(wszBuffer);
	
	//Delegate
	return pPropBag->Write(L"Data Source", &m_vDataSource);
}



////////////////////////////////////////////////////////
// CDSLAdvancedPage
//
////////////////////////////////////////////////////////
CDSLAdvancedPage::CDSLAdvancedPage()	
	: CPropertyPage(CLSID_SampProvAdvancedPage, IDD_DSL_ADVANCED)
{
}

////////////////////////////////////////////////////////
// ~CDSLAdvancedPage
//
////////////////////////////////////////////////////////
CDSLAdvancedPage::~CDSLAdvancedPage()
{
}



////////////////////////////////////////////////////////
// CDSLAdvancedPage::HandleMessage
//
////////////////////////////////////////////////////////
BOOL CDSLAdvancedPage::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}
