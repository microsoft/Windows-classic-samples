// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  HMMSampDlg.cpp
//
// Description:
//	This file implements the CAdvClientDlg dialog class which 
//		is the main dialog for the tutorial.
// 
// History:
//
// **************************************************************************


#include "stdafx.h"
#include "AdvClient.h"
#include "AdvClientDlg.h"
#include "OnAsync.h"
#include <strsafe.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// **************************************************************************
//
//	CAdvClientDlg::CAdvClientDlg()
//
// Description:
//		Constructor for the main dialog. Puts the default namespace
//		in the edit box and initializes variables.	
// Parameters:
//		pParent (in) - parent window.
//
// Returns:
//		nothing.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
CAdvClientDlg::CAdvClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvClientDlg::IDD, pParent),
	m_threadId(0), m_thread(0), m_ptrReady(0), m_stopThread(0)
{
	//{{AFX_DATA_INIT(CAdvClientDlg)
	m_namespace = _T("\\\\.\\root\\cimv2");
	//}}AFX_DATA_INIT

	m_pIWbemServices = NULL;
	m_pOfficeService = NULL;
	m_pQueryCallback = NULL;
	m_regPerm = TRUE;
	m_regTemp = TRUE;
	m_pStream = NULL;

    InitializeCriticalSection(&m_critSect);
	m_stopThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_ptrReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	

}

// **************************************************************************
//
//	CAdvClientDlg::~CAdvClientDlg()
//
// Description:
//		Destructor. Releases COM interfaces for the dialog class.
// Parameters:
//		None.
//
// Returns:
//		nothing.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
CAdvClientDlg::~CAdvClientDlg()
{
	
	if (m_thread != NULL)
	{
		OnTempUnregister();
	}

	// done with m_pQueryCallback sink. 
	if(m_pQueryCallback)
	{
		m_pQueryCallback->Release();
		delete m_pQueryCallback;
		m_pQueryCallback = NULL;
	}

	// done with m_pIWbemServices. 
	if (m_pIWbemServices)
	{ 
		m_pIWbemServices->Release(); 
		m_pIWbemServices = NULL;
	}
	// done with m_pIWbemServices. 
	if (m_pOfficeService)
	{ 
		m_pOfficeService->Release(); 
		m_pOfficeService = NULL;
	}

	StopThread();

    DeleteCriticalSection(&m_critSect);


}
void CAdvClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvClientDlg)
	DDX_Control(pDX, IDC_DISK_PROPS_DESCRIPTIONS, m_diskDescriptions);
	DDX_Control(pDX, IDC_EVENTLIST, m_eventList);
	DDX_Control(pDX, IDC_REGPERM, m_perm);
	DDX_Control(pDX, IDC_REGTEMP, m_temp);
	DDX_Control(pDX, IDC_ADDEQUIPMENT, m_addEquipment);
	DDX_Control(pDX, IDC_ENUMSERVICES, m_enumServices);
	DDX_Control(pDX, IDC_ENUMSERVICESASYNC, m_enumServicesAsync);
	DDX_Control(pDX, IDC_ENUMDISKS, m_enumDisks);
	DDX_Control(pDX, IDC_DISKDETAILS, m_diskDetails);
	DDX_Control(pDX, IDC_CONNECT, m_connect);
	DDX_Control(pDX, IDC_OUTPUTLIST, m_outputList);
	DDX_Text(pDX, IDC_NAMESPACE, m_namespace);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvClientDlg, CDialog)
	//{{AFX_MSG_MAP(CAdvClientDlg)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_ADDEQUIPMENT, OnAddEquipment)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_ENUMDISKS, OnEnumdisks)
	ON_BN_CLICKED(IDC_ENUMSERVICES, OnEnumservices)
	ON_BN_CLICKED(IDC_ENUMSERVICESASYNC, OnEnumservicesasync)
	ON_BN_CLICKED(IDC_DISKDETAILS, OnDiskdetails)
	ON_BN_CLICKED(IDC_REGPERM, OnRegPerm)
	ON_BN_CLICKED(IDC_REGTEMP, OnRegTemp)
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DISK_PROPS_DESCRIPTIONS, OnDiskPropsDescriptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvClientDlg message handlers

BOOL CAdvClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	//make scrollable horizontally by 5000 pixels
	m_outputList.SetHorizontalExtent(5000);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAdvClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


// **************************************************************************
//
//	ErrorString()
//
// Description:
//		Converts an HRESULT to a displayable string.
//
// Parameters:
//		hRes (in) - HRESULT to be converted.
//
// Returns:
//		ptr to displayable string.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
LPCTSTR ErrorString(HRESULT hRes)
{
    TCHAR szBuffer2[19];
	static TCHAR szBuffer[24];
	LPCTSTR psz;

   switch(hRes) 
   {
   case WBEM_NO_ERROR:
		psz = _T("WBEM_NO_ERROR");
		break;
   case WBEM_S_FALSE:
		psz = _T("WBEM_S_FALSE");
		break;
   case WBEM_S_NO_MORE_DATA:
		psz = _T("WBEM_S_NO_MORE_DATA");
		break;
	case WBEM_E_FAILED:
		psz = _T("WBEM_E_FAILED");
		break;
	case WBEM_E_NOT_FOUND:
		psz = _T("WBEM_E_NOT_FOUND");
		break;
	case WBEM_E_ACCESS_DENIED:
		psz = _T("WBEM_E_ACCESS_DENIED");
		break;
	case WBEM_E_PROVIDER_FAILURE:
		psz = _T("WBEM_E_PROVIDER_FAILURE");
		break;
	case WBEM_E_TYPE_MISMATCH:
		psz = _T("WBEM_E_TYPE_MISMATCH");
		break;
	case WBEM_E_OUT_OF_MEMORY:
		psz = _T("WBEM_E_OUT_OF_MEMORY");
		break;
	case WBEM_E_INVALID_CONTEXT:
		psz = _T("WBEM_E_INVALID_CONTEXT");
		break;
	case WBEM_E_INVALID_PARAMETER:
		psz = _T("WBEM_E_INVALID_PARAMETER");
		break;
	case WBEM_E_NOT_AVAILABLE:
		psz = _T("WBEM_E_NOT_AVAILABLE");
		break;
	case WBEM_E_CRITICAL_ERROR:
		psz = _T("WBEM_E_CRITICAL_ERROR");
		break;
	case WBEM_E_INVALID_STREAM:
		psz = _T("WBEM_E_INVALID_STREAM");
		break;
	case WBEM_E_NOT_SUPPORTED:
		psz = _T("WBEM_E_NOT_SUPPORTED");
		break;
	case WBEM_E_INVALID_SUPERCLASS:
		psz = _T("WBEM_E_INVALID_SUPERCLASS");
		break;
	case WBEM_E_INVALID_NAMESPACE:
		psz = _T("WBEM_E_INVALID_NAMESPACE");
		break;
	case WBEM_E_INVALID_OBJECT:
		psz = _T("WBEM_E_INVALID_OBJECT");
		break;
	case WBEM_E_INVALID_CLASS:
		psz = _T("WBEM_E_INVALID_CLASS");
		break;
	case WBEM_E_PROVIDER_NOT_FOUND:
		psz = _T("WBEM_E_PROVIDER_NOT_FOUND");
		break;
	case WBEM_E_INVALID_PROVIDER_REGISTRATION:
		psz = _T("WBEM_E_INVALID_PROVIDER_REGISTRATION");
		break;
	case WBEM_E_PROVIDER_LOAD_FAILURE:
		psz = _T("WBEM_E_PROVIDER_LOAD_FAILURE");
		break;
	case WBEM_E_INITIALIZATION_FAILURE:
		psz = _T("WBEM_E_INITIALIZATION_FAILURE");
		break;
	case WBEM_E_TRANSPORT_FAILURE:
		psz = _T("WBEM_E_TRANSPORT_FAILURE");
		break;
	case WBEM_E_INVALID_OPERATION:
		psz = _T("WBEM_E_INVALID_OPERATION");
		break;
	case WBEM_E_INVALID_QUERY:
		psz = _T("WBEM_E_INVALID_QUERY");
		break;
	case WBEM_E_INVALID_QUERY_TYPE:
		psz = _T("WBEM_E_INVALID_QUERY_TYPE");
		break;
	case WBEM_E_ALREADY_EXISTS:
		psz = _T("WBEM_E_ALREADY_EXISTS");
		break;
   case WBEM_S_ALREADY_EXISTS:
      psz = _T("WBEM_S_ALREADY_EXISTS");
      break;
   case WBEM_S_RESET_TO_DEFAULT:
      psz = _T("WBEM_S_RESET_TO_DEFAULT");
      break;
   case WBEM_S_DIFFERENT:
      psz = _T("WBEM_S_DIFFERENT");
      break;
   case WBEM_E_OVERRIDE_NOT_ALLOWED:
      psz = _T("WBEM_E_OVERRIDE_NOT_ALLOWED");
      break;
   case WBEM_E_PROPAGATED_QUALIFIER:
      psz = _T("WBEM_E_PROPAGATED_QUALIFIER");
      break;
   case WBEM_E_PROPAGATED_PROPERTY:
      psz = _T("WBEM_E_PROPAGATED_PROPERTY");
      break;
   case WBEM_E_UNEXPECTED:
      psz = _T("WBEM_E_UNEXPECTED");
      break;
   case WBEM_E_ILLEGAL_OPERATION:
      psz = _T("WBEM_E_ILLEGAL_OPERATION");
      break;
   case WBEM_E_CANNOT_BE_KEY:
      psz = _T("WBEM_E_CANNOT_BE_KEY");
      break;
   case WBEM_E_INCOMPLETE_CLASS:
      psz = _T("WBEM_E_INCOMPLETE_CLASS");
      break;
   case WBEM_E_INVALID_SYNTAX:
      psz = _T("WBEM_E_INVALID_SYNTAX");
      break;
   case WBEM_E_NONDECORATED_OBJECT:
      psz = _T("WBEM_E_NONDECORATED_OBJECT");
      break;
   case WBEM_E_READ_ONLY:
      psz = _T("WBEM_E_READ_ONLY");
      break;
   case WBEM_E_PROVIDER_NOT_CAPABLE:
      psz = _T("WBEM_E_PROVIDER_NOT_CAPABLE");
      break;
   case WBEM_E_CLASS_HAS_CHILDREN:
      psz = _T("WBEM_E_CLASS_HAS_CHILDREN");
      break;
   case WBEM_E_CLASS_HAS_INSTANCES:
      psz = _T("WBEM_E_CLASS_HAS_INSTANCES");
      break;
   case WBEM_E_QUERY_NOT_IMPLEMENTED:
      psz = _T("WBEM_E_QUERY_NOT_IMPLEMENTED");
      break;
   case WBEM_E_ILLEGAL_NULL:
      psz = _T("WBEM_E_ILLEGAL_NULL");
      break;
   case WBEM_E_INVALID_QUALIFIER_TYPE:
      psz = _T("WBEM_E_INVALID_QUALIFIER_TYPE");
      break;
   case WBEM_E_INVALID_PROPERTY_TYPE:
      psz = _T("WBEM_E_INVALID_PROPERTY_TYPE");
      break;
   case WBEM_E_VALUE_OUT_OF_RANGE:
      psz = _T("WBEM_E_VALUE_OUT_OF_RANGE");
      break;
   case WBEM_E_CANNOT_BE_SINGLETON:
      psz = _T("WBEM_E_CANNOT_BE_SINGLETON");
      break;
	default:
      _itot_s(hRes, szBuffer2, 16);
	  StringCbCopy(szBuffer, sizeof(szBuffer),_T("0x"));
      StringCbCat(szBuffer, sizeof(szBuffer), szBuffer2);
	  psz = szBuffer;
	}
	return psz;
}

// **************************************************************************
//
//	ValueToString()
//
// Description:
//		Converts a variant to a displayable string.
//
// Parameters:
//		pValue (in) - variant to be converted.
//		pbuf (out) - ptr to receive displayable string.
//
// Returns:
//		Same as pbuf.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
#define BLOCKSIZE (32 * sizeof(WCHAR))
#define CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop  (this size stolen from cvt.h in c runtime library) */

LPWSTR ValueToString(VARIANT *pValue, WCHAR **pbuf)
{
   DWORD iNeed = 0;
   DWORD iVSize = 0;
   DWORD iCurBufSize = 0;

   WCHAR *vbuf = NULL;
   WCHAR *buf = NULL;

   switch (pValue->vt) 
   {
   case VT_NULL: 
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
			StringCbCopyW(buf, BLOCKSIZE, L"<null>");
         break;

   case VT_BOOL: {
         VARIANT_BOOL b = pValue->boolVal;
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
		 {
			if (!b) {
				StringCbCopyW(buf, BLOCKSIZE, L"FALSE");

			} else {
				StringCbCopyW(buf, BLOCKSIZE, L"TRUE");
			}
		}
         break;
      }

   case VT_UI1: {
     	unsigned char b = pValue->bVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            if (b >= 32)
            {
                StringCbPrintfW(buf, BLOCKSIZE, L"'%c' (%hu, 0x%hX)", b, (unsigned char)b, b);
            }
            else
            {
                StringCbPrintfW(buf, BLOCKSIZE, L"%hu (0x%hX)", (unsigned char)b, b);
            }
        }
        break;
	}
   case VT_I2:  {
        USHORT i = pValue->uiVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%hu (0x%hX)", i, i);
        }
        break;
    }

   case VT_I4: {
         LONG l = pValue->lVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%d (0x%X)", l, l);
        }
        break;
      }

   case VT_R4: {
        float f = pValue->fltVal;
        buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE,  L"%10.4f", f);
        }
        break;
      }

   case VT_R8: {
		double d = pValue->dblVal;
        buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%10.4f", d);
        }
        break;
	  }

   case VT_BSTR: {
		 LPWSTR pWStr = pValue->bstrVal;
		 size_t bufSize = (wcslen(pWStr) * sizeof(WCHAR)) + sizeof(WCHAR) + (2 * sizeof(WCHAR));
		 buf = (WCHAR *)malloc(bufSize);
		 if (buf)
			StringCbPrintfW(buf, bufSize, L"\"%wS\"", pWStr);
		 break;
		}

	// the sample GUI is too simple to make it necessary to display
	// these 'complicated' types--so ignore them.
   case VT_DISPATCH:  // Currently only used for embedded objects
   case VT_BOOL|VT_ARRAY: 
   case VT_UI1|VT_ARRAY: 
   case VT_I2|VT_ARRAY: 
   case VT_I4|VT_ARRAY: 
   case VT_R4|VT_ARRAY: 
   case VT_R8|VT_ARRAY: 
   case VT_BSTR|VT_ARRAY: 
   case VT_DISPATCH | VT_ARRAY: 
         break;

   default:
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
			StringCbCopyW(buf, BLOCKSIZE, L"<conversion error>");
   }

   *pbuf = buf;   
   return buf;
}
// **************************************************************************
//
//	CAdvClientDlg::EnsureOfficeNamespace()
//
// Description:
//		Returns a pointer to root\cimv2\office namespace; creating
//		it if it doesn't already exist
// Parameters:
//		None.
//
// Returns:
//		COM interface to the namespace.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
BOOL CAdvClientDlg::EnsureOfficeNamespace(void)
{
	IWbemClassObject *pNSClass = NULL;
	IWbemClassObject *pNSInst = NULL;

	BSTR Prop = NULL;
	VARIANT v;
	HRESULT hRes;

	BSTR Namespace = SysAllocString(L"SAMPLE_Office");
	if (Namespace == NULL)
	{
		return FALSE;
	}

	// avoid getting another pointer if you already one. Potential
	// leak if you do.
	if(m_pOfficeService != NULL)
	{
		return TRUE;
	}

	// if 'Office' namespace doesnt exists...
	if((hRes = m_pIWbemServices->OpenNamespace(Namespace, 0, NULL, &m_pOfficeService, NULL)) != S_OK)
	{ // create it.

		BSTR NamespaceClass = SysAllocString(L"__Namespace");
		if (NamespaceClass == NULL)
		{
			SysFreeString(Namespace);
			return FALSE;
		}

		//-----------------------------------
		// get the namespace class
		hRes = m_pIWbemServices->GetObject(NamespaceClass,
											0L,	NULL,
											&pNSClass,
											NULL);

		SysFreeString(NamespaceClass);
		if(hRes == S_OK)
		{
			//-----------------------------------
			// spawn a new instance
			if((hRes = pNSClass->SpawnInstance(0, &pNSInst)) == S_OK)
			{
				TRACE(_T("SpawnInstance() worked\n"));

				// set the namespace's name.
				VariantInit(&v);
				V_VT(&v) = VT_BSTR;
				V_BSTR(&v) = Namespace;
				Prop = SysAllocString(L"Name");
				if (Prop == NULL)
				{
					pNSClass->Release();  // Don't need the class any more
					pNSClass = NULL;
					SysFreeString(Namespace);
					return FALSE;
				}

				pNSInst->Put(Prop, 0, &v, 0);

				SysFreeString(Prop);
				VariantClear(&v);

				// create the instance.
				hRes = m_pIWbemServices->PutInstance(pNSInst, 0, NULL, NULL);
				pNSInst->Release();
				pNSInst = NULL;

				//-----------------------------------
				// open the new namespace.
				if((hRes = m_pIWbemServices->OpenNamespace(Namespace, 0, NULL, &m_pOfficeService, NULL)) != S_OK)
				{
					TRACE(_T("OpenNamespace() still failed: %s\n"), ErrorString(hRes));
				} // endif OpenNamespace() again
			}
			else
			{
				TRACE(_T("SpawnInstance() failed: %s\n"), ErrorString(hRes));

			} //endif SpawnInstance()

			pNSClass->Release();  // Don't need the class any more
			pNSClass = NULL;
		}

	} //endif OpenNamespace()

	SysFreeString(Namespace);

	// NOTE: The caller needs to Release() it.
	return (m_pOfficeService != NULL);
}
//--------------------------------------------------------
BOOL CAdvClientDlg::CheckOfficeNamespace(void)
{
	IWbemServices *pOfficeService = NULL;
	HRESULT hRes;
	BOOL itsThere = FALSE;

	// lazy check first.
	if(m_pOfficeService != NULL) return TRUE;

	// do some real work now.
	BSTR Namespace = SysAllocString(L"SAMPLE_Office");
	if (Namespace == NULL)
		return FALSE; //out of memory

	// if 'Office' namespace doesnt exists...
	if((hRes = m_pIWbemServices->OpenNamespace(Namespace, 0, NULL, 
												&pOfficeService, NULL)) == S_OK)
	{ 
		itsThere = TRUE;
		pOfficeService->Release();
		pOfficeService = NULL;

	} //endif OpenNamespace()

	SysFreeString(Namespace);

	return itsThere;
}

//---------------------------------------------------------------
void CAdvClientDlg::OnCancel() 
{
	// make sure you unregister that sink.
	if (m_thread != NULL)
	{
		OnTempUnregister();
	}
	
	CDialog::OnCancel();
}


