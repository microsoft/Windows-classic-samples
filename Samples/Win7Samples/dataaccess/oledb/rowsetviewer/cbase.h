//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CBASE.H
//
//-----------------------------------------------------------------------------------

#ifndef _CBASE_H_
#define _CBASE_H_


///////////////////////////////////////////////////////////////
// Forwards
//
///////////////////////////////////////////////////////////////
class CBase;
class CMainWindow;
class CMDIChild;
class CIntTrace;
class COptionsSheet;


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
enum SOURCE
{
	eInvalid				= 0,

	//Object Source
	eCUnknown				= 1,
	eCDataSource			= 2,
	eCSession				= 3,
	eCCommand				= 4,
	eCMultipleResults		= 5,
	eCRowset				= 6,
	eCRow					= 7,
	eCStream				= 8,	
	eCEnumerator			= 9,		
	eCBinder				=10,
	eCServiceComp			=11,
	eCDataLinks				=12,
	eCDataset				=13,	
	eCTransaction			=14,
	eCTransactionOptions	=15,
	eCError					=16,
	eCCustomError			=17,
	eCRowPosition			=18, 
	eCConnectionPoint		=19,
};


enum BASE_CLASS
{
	eCBase				= 0x0001000,
	eCContainerBase		= 0x0002000,
	eCAsynchBase		= 0x0004000,
	eCPropertiesBase	= 0x0008000,
	eCDataAccess		= 0x0010000
};


//Use to quickly implement GetInterfaceAddress
#define HANDLE_GETINTERFACE(interface)			\
	if(riid == IID_##interface)					\
		return (IUnknown**)&m_p##interface

#define OBTAIN_INTERFACE(interface)				\
	if(!m_p##interface)							\
		TRACE_QI(m_pIUnknown, IID_##interface, (IUnknown**)&m_p##interface, GetObjectName())

#define RELEASE_INTERFACE(interface)			\
	if(m_p##interface)							\
		TRACE_RELEASE(m_p##interface,	WIDESTRING(#interface))

#define SOURCE_GETINTERFACE(pObject, type)		\
	((pObject) ? (type*)(pObject)->GetInterface(IID_##type) : NULL)

#define SOURCE_GETOBJECT(pObject, source)		\
	(((pObject) && (((pObject)->GetObjectType() == e##source) || ((pObject)->GetBaseType() & e##source))) ? (source*)(pObject) : NULL)

#define SOURCE_GETPARENT(pObject, source)		\
	((pObject) ? (source*)(pObject)->GetParent(e##source) : NULL)


///////////////////////////////////////////////////////////////
// Functions
//
///////////////////////////////////////////////////////////////
SOURCE		GuidToSourceType(REFGUID guidType);
SOURCE		DetermineObjectType(IUnknown* pIUnknown, SOURCE eSource);


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// Interface
//
///////////////////////////////////////////////////////////////
extern const IID IID_IAggregate;
interface IAggregate : public IUnknown
{
};


/////////////////////////////////////////////////////////////////
// CBase class
//
/////////////////////////////////////////////////////////////////
class CBase : public IUnknown
{
public:
	//Constructors
	CBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CBase();

	//IUnknown
	//So we can get referening counting on parent objects...
	STDMETHODIMP			QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)	AddRef();
    STDMETHODIMP_(ULONG)	Release();

	//Base impemented helpers
	virtual ULONG			ObjectAddRef();
	virtual ULONG			ObjectRelease();
	virtual HRESULT			ObjectQI(REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			CreateObject(CBase* pCSource, REFIID riid, IUnknown* pIUnknown, DWORD dwCreateOpts = -1 /*Default*/);
	virtual HRESULT			ReleaseObject(ULONG ulExpectedRefCount = 0);
	virtual HRESULT			ReleaseChildren();

	virtual HRESULT			SetInterface(REFIID riid, IUnknown* pIUnknown);
	virtual IUnknown*		GetInterface(REFIID riid);

	virtual BOOL			IsSameObject(IUnknown* pIUnkObject);
	virtual CBase*			GetParent(SOURCE eSource);

	//Derived Object helpers (Devired Class implements this)
	virtual HRESULT			AutoQI(DWORD dwCreateOpts)			= 0;
	virtual HRESULT			AutoRelease()						= 0;
	virtual IUnknown**		GetInterfaceAddress(REFIID riid)	= 0;

	virtual WCHAR*			GetObjectName()			= 0;
	virtual UINT			GetObjectMenu()			= 0;
	virtual LONG			GetObjectImage()		= 0;
	virtual REFIID			GetDefaultInterface()	= 0;
	virtual	void			OnDefOperation();

	//UI - Helpers
	virtual HRESULT			DisplayObject();
	virtual WCHAR*			GetObjectDesc()			{ return m_strObjectDesc;				}
	virtual void			SetObjectDesc(WCHAR* pwszDescription, BOOL fCopy = TRUE);
	
	//Inlines
	inline	SOURCE			GetObjectType()			{ return m_eObjectType;					}
	inline	BASE_CLASS		GetBaseType()			{ return m_eBaseClass;					}
	
	//Interface
	virtual COptionsSheet*	GetOptions();

	//Common OLE DB Interfaces
	IUnknown*				m_pIUnknown;
	ISupportErrorInfo*		m_pISupportErrorInfo;
	IAggregate*				m_pIAggregate;
	IService*				m_pIService;

	//Data
	HTREEITEM				m_hTreeItem;
	DWORD					m_dwCLSCTX;

	//Parent Info
	CBase*					m_pCParent;
	GUID					m_guidSource;

	//BackPointers
	CMainWindow*			m_pCMainWindow;
	CMDIChild*				m_pCMDIChild;

protected:
	//Type
	SOURCE					m_eObjectType;
	BASE_CLASS				m_eBaseClass;
	CComWSTR				m_strObjectDesc;

	//IUnknown
	ULONG					m_cRef;
};



/////////////////////////////////////////////////////////////////
// CUnknown class
//
/////////////////////////////////////////////////////////////////
class CUnknown : public CBase
{
public:
	//Constructors
	CUnknown(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL)
		: CBase(eCUnknown, pCMainWindow, pCMDIChild)
	{
	}

	virtual ~CUnknown()
	{
		ReleaseObject(0);
	}

	//Derived Object helpers
	//Devired Class implements this...
	virtual HRESULT			AutoQI(DWORD dwCreateOpts)						{ return CBase::AutoQI(dwCreateOpts);	}
	virtual HRESULT			AutoRelease()									{ return CBase::AutoRelease();			}
	virtual IUnknown**		GetInterfaceAddress(REFIID riid)						{ return CBase::GetInterfaceAddress(riid);		}

	virtual WCHAR*			GetObjectName()									{ return L"Unknown";					} 
	virtual UINT			GetObjectMenu()									{ return IDM_UNKNOWNMENU;				}
	virtual LONG			GetObjectImage()								{ return IMAGE_QUESTION;				}
	virtual REFIID			GetDefaultInterface()							{ return IID_IUnknown;					}
};


/////////////////////////////////////////////////////////////////
// CContainerBase class
//
/////////////////////////////////////////////////////////////////
class CContainerBase : public CBase
{
public:
	//Constructors
	CContainerBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CContainerBase();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Members
	virtual HRESULT			FindConnectionPoint(REFIID riid, IConnectionPoint** ppIConnectionPoint);
	virtual HRESULT			AdviseListener(REFIID riid, DWORD* pdwCookie);
	virtual HRESULT			UnadviseListener(REFIID riid, DWORD* pdwCookie);

	//OLE DB Interfaces
	//[MANADATORY]

	//[OPTIONAL]
	IConnectionPointContainer*	m_pIConnectionPointContainer;

//protected:
	//Data
};


/////////////////////////////////////////////////////////////////
// CConnectionPoint class
//
/////////////////////////////////////////////////////////////////
class CConnectionPoint : public CBase
{
public:
	//Constructors
	CConnectionPoint(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CConnectionPoint();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Derived Class
	virtual WCHAR*			GetObjectName()					{ return L"ConnectionPoint";			} 
	virtual UINT			GetObjectMenu()					{ return IDM_CONNECTIONPOINTMENU;		}
	virtual LONG			GetObjectImage()				{ return IMAGE_FORM;					}
	virtual REFIID			GetDefaultInterface()			{ return IID_IConnectionPoint;			}
	virtual WCHAR*			GetObjectDesc();

	//Members
	virtual HRESULT			GetConnectionInterface(IID* pIID);

	//OLE DB Interfaces
	//[MANADATORY]
	IConnectionPoint*		m_pIConnectionPoint;

	//[OPTIONAL]

//protected:
	//Data
	DWORD					m_dwCookie;
};


/////////////////////////////////////////////////////////////////
// CAsynchBase class
//
/////////////////////////////////////////////////////////////////
class CAsynchBase : public CContainerBase
{
public:
	//Constructors
	CAsynchBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CAsynchBase();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Members
	virtual BOOL			IsInitialized()		{ return m_fInitialized;			}
	virtual HRESULT			Initialize();
	virtual HRESULT			Uninitialize();

	virtual HRESULT			Abort(HCHAPTER hChapter, DBASYNCHOP eOperation);
	virtual HRESULT			GetStatus(HCHAPTER hChapter, DBASYNCHOP eOperation, DBCOUNTITEM* pulProgress, DBCOUNTITEM* pulProgressMax, DBASYNCHPHASE* peAsynchPhase, LPOLESTR* ppwszStatusText);

	//OLE DB Interfaces
	//[MANADATORY]

	//[OPTIONAL]
	IDBInitialize*			m_pIDBInitialize;				//OLE DB interface
	IDBAsynchStatus*		m_pIDBAsynchStatus;				//OLE DB interface

	//Extra interfaces

//protected:
	//Data
	DWORD					m_dwCookieAsynchNotify;
	BOOL					m_fInitialized;
};


/////////////////////////////////////////////////////////////////
// CPropertiesBase class
//
/////////////////////////////////////////////////////////////////
class CPropertiesBase : public CAsynchBase
{
public:
	//Constructors
	CPropertiesBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CPropertiesBase();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Members
	virtual HRESULT			SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets);

	//OLE DB Interfaces
	//[MANADATORY]
	IDBProperties*			m_pIDBProperties;				//OLE DB interface

	//[OPTIONAL]

protected:
	//Data
};


///////////////////////////////////////////////////////////////////////////////
// Class CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
class CAggregate : public IAggregate
{
public:
	CAggregate();
	virtual ~CAggregate();

	//IUnknown
	virtual STDMETHODIMP_(ULONG)	AddRef(void);
	virtual STDMETHODIMP_(ULONG)	Release(void);
	virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);

	//Helpers
	virtual HRESULT		HandleAggregation(REFIID riid, IUnknown** ppIUnknown);
			HRESULT		SetInner(IUnknown* pIUnkInner);
			HRESULT		ReleaseInner();

protected:
	//Data
	ULONG				m_cRef;
	CComPtr<IUnknown>	m_spUnkInner;
};


#endif	//_CBASE_H_
