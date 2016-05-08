//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module COMMON.H
//
//-----------------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "Headers.h"
#include "crtdbg.h"		//_ASSERTE


/////////////////////////////////////////////////////////////////////////////
// Externs
//
//////////////////////////////////////////////////////////////////////////////
extern CComPtr<IDataConvert>	g_spDataConvert;



/////////////////////////////////////////////////////////////////////////////
// Enums
//
//////////////////////////////////////////////////////////////////////////////
enum CONV_FLAGS
{
	CONV_NONE			= 0x00000000,
	CONV_VARBOOL		= 0x00000001,
	CONV_ALPHABOOL		= 0x00000002,
	CONV_HEX			= 0x00000004,
	CONV_OCTAL			= 0x00000008,
	CONV_DECIMAL		= 0x00000010,
	CONV_BINARY			= 0x00000020,

	CONV_TYPENAME		= 0x00000100,
	CONV_MSDADC_ONLY	= 0x00001000
};



/////////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////////
#define HKEY_ROWSETVIEWER		HKEY_CURRENT_USER
#define wszROWSETVIEWER_KEY		L"Software\\Microsoft\\OLE DB\\RowsetViewer"
#define wszCONFIG_KEY			wszROWSETVIEWER_KEY L"\\Config"
#define wszRECENTCONFIG_KEY		wszROWSETVIEWER_KEY L"\\RecentConfig"
#define wszRECENTFILE_KEY		wszROWSETVIEWER_KEY L"\\RecentFile"
#define wszRECENTINITSTRING_KEY	wszROWSETVIEWER_KEY L"\\RecentInitString"
#define wszOPTIONS_KEY			wszROWSETVIEWER_KEY L"\\Options"
#define wszTRACE_KEY			wszROWSETVIEWER_KEY L"\\Trace"
#define wszOBJECTS_KEY			wszROWSETVIEWER_KEY L"\\Objects"
#define wszMDICHILD_KEY			wszROWSETVIEWER_KEY L"\\MDIChild"
#define wszRECENTURL_KEY		wszROWSETVIEWER_KEY L"\\RecentURL"


///////////////////////////////////////////////////////////////
// Colors
//
///////////////////////////////////////////////////////////////
//RGB is exact opposite of COLORREF.  COLOREF = 0x00bbggrr, RGB = 0x00RRGGBB
#define RGB_FROM_COLOREF(cref)   (((cref & 0x000000FF) << 16) | ((cref & 0x0000FF00)) | ((cref & 0x00FF0000) >> 16))

const COLORREF	RGB_BLUE		= RGB(0,		0,	 0xff);
const COLORREF	RGB_DARK_BLUE	= RGB(0,		0,	 0x80);
const COLORREF	RGB_GREEN		= RGB(0,	 0xff,		0);
const COLORREF	RGB_DARK_GREEN	= RGB(0,	 0x80,		0);
const COLORREF	RGB_RED			= RGB(0xff,		0,		0);
const COLORREF	RGB_DARK_RED	= RGB(0x80,		0,		0);
const COLORREF	RGB_BROWN		= RGB(0x80,	 0x40,		0);

const COLORREF	RGB_NONE		= 0;


////////////////////////////////////////////////////////////////////////////
// Windows Defines
//
////////////////////////////////////////////////////////////////////////////
//Dialog Box procedures want to know if you handled the MSG
//or not.  If you do, thenit just returns, if not then it calls
//the default windialog procedure to try and handle it
const BOOL HANDLED_MSG	 = TRUE;
const BOOL UNHANDLED_MSG = FALSE;


#define LVM_ERR			(-1)
#define LV_ALLITEMS		(LVM_ERR)
#define LV_ALLSELITEMS	(LV_ALLITEMS-1)

#define IMAGE_NONE		(-1)
#define PARAM_NONE		(-1)
#define STATE_NONE		(-1)


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define NUMELE(rgEle) (sizeof(rgEle) / sizeof(rgEle[0]))
#define __WIDESTRING(str) L##str
#define WIDESTRING(str) __WIDESTRING(str)

//FindLeaks
#ifdef _IMALLOC_SPY
#define DISP_FILE_LINE	InternalTraceFmt(L"File '%s', Line '%lu'\n", WIDESTRING(__FILE__), __LINE__)
#else
#define DISP_FILE_LINE
#endif //_IMALLOCSPY

#define CHECK_MEMORY(p)				{ if(!(p)) { DisplayAllErrors(hr = E_OUTOFMEMORY, WIDESTRING(__FILE__), __LINE__); goto CLEANUP; }			}
#define SAFE_ALLOC(p, type, cb)		{ DISP_FILE_LINE; (p) = (type*)CoTaskMemAlloc(ULONG((cb)*sizeof(type))); CHECK_MEMORY(p);					}
#define SAFE_REALLOC(p, type, cb)	{ DISP_FILE_LINE; (p) = (type*)CoTaskMemRealloc((type*)p, ULONG((cb)*sizeof(type))); CHECK_MEMORY(p);		}
#define SAFE_BSTRALLOC(p, bstr)		{ (p) = SysAllocString(bstr); CHECK_MEMORY(p);																}		

#define SAFE_FREE(p)				if(p) { CoTaskMemFree((void*)p); (p) = NULL;	}
#define SAFE_BSTRFREE(p)			if(p) { SysFreeString(p); (p) = NULL;			}

#define SAFE_ADDREF(p)				if(p) { (p)->AddRef();							}
#define SAFE_RELEASE(p)				if(p) { (p)->Release(); (p) = NULL;				}  
#define SAFE_DELETE(p)				if(p) { delete p; p = NULL;						}

																									
//Error Checking
#define TEST(hr)					{ (hr);										}
#define TESTC(hr)					{ if(FAILED(hr)) goto CLEANUP;				}
#define TESTC_(hr, hrExpected)		{ if((hr) != (hrExpected)) goto CLEANUP;	}

//Error Checking
//These Macros are used for calling all OLE DB Methods...
#define XTEST(hr)					{ DisplayAllErrors(hr, WIDESTRING(__FILE__), __LINE__);											}
#define XTEST_(hr, hrExpected)		{ DisplayAllErrors(hr, hrExpected, WIDESTRING(__FILE__), __LINE__);								}
#define XTESTC(hr)					{ if(FAILED(DisplayAllErrors(hr, WIDESTRING(__FILE__), __LINE__))) goto CLEANUP;				}
#define XTESTC_(hr, hrExpected)		{ if(FAILED(DisplayAllErrors(hr, hrExpected, WIDESTRING(__FILE__), __LINE__))) goto CLEANUP;	}

//Windows Error Checking
#define GETLASTERROR(exp)			{ if(!(exp)) { DisplayWinError(GetLastError(), WIDESTRING(__FILE__), __LINE__); 				} }
#define GETLASTERRORC(exp)			{ if(!(exp)) { DisplayWinError(GetLastError(), WIDESTRING(__FILE__), __LINE__)); goto CLEANUP;	} }


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define MAX_QUERY_LEN			4096
#define MAX_NAME_LEN			256

#define MAX_COL_SIZE		    5000
#define MAX_BLOCK_SIZE			2000
#define MAX_OPENROWS			 256

//POINTER_DISPLAYSIZE = 0x00000000 = (1 Byte = 2 Asci Chars + "0x" + NULL Terminator)
#define POINTER_DISPLAYSIZE		(sizeof(INT_PTR)*2+2+1)		

//Displays values like VALUE as   VALUE , L"VALUE"
#define VALUE_WCHAR(value)		L#value, value
#define VALUE_CHAR(value)		#value, value
#define VALUE_PWCHAR(value)		L#value, &value
#define VALUE_CHAR_HR(hr)		#hr, HRESULT_FROM_WIN32(hr)

#define EOL		 '\0'
#define wEOL	L'\0'
#define WndEOL	"\r\n"
#define wWndEOL	L"\r\n"


///////////////////////////////////////////////////////////////////
// Accessor / Binding 
//
///////////////////////////////////////////////////////////////////
// Constant combination of two DBTYPEs
#define DBTYPE_SRC_DST(dbSrc, dbDst) (((dbSrc) << (sizeof(DBTYPE)*8)) + (dbDst))

//STATUS helpers, for locating obStatus offsets in the bindings
#define STATUS_IS_BOUND(Binding)    ( (Binding).dwPart & DBPART_STATUS )
#define BINDING_STATUS(Binding, p) (*(DBSTATUS*)((BYTE*)(p) + (Binding).obStatus))

//LENGTH helpers, for locating obLength offsets in the bindings
#define LENGTH_IS_BOUND(Binding)    ( (Binding).dwPart & DBPART_LENGTH )
#define BINDING_LENGTH(Binding, p) (*(DBLENGTH*)((BYTE*)(p) + (Binding).obLength))

//VALUE helpers, for locating obValue offsets in the bindings
#define VALUE_IS_BOUND(Binding)     ( (Binding).dwPart & DBPART_VALUE )
#define BINDING_VALUE(Binding, p)  (*(void**)((BYTE*)(p) + (Binding).obValue ))

//ROUNDUP on all platforms pointers must be aligned properly
#define ROUNDUP_AMOUNT	8
#define ROUNDUP_(size,amount)		(((DBBYTEOFFSET)(size)+((amount)-1))&~((DBBYTEOFFSET)(amount)-1))
#define ROUNDUP(size)				ROUNDUP_(size, ROUNDUP_AMOUNT)

#define BIT_SET(dwValue, dwMask)				(((dwValue) & (dwMask)) == (dwMask))	//All bits set
#define BIT_CLEAR(dwValue, dwMask)				(!((dwValue) & (dwMask)))				//All bits clear
#define ENABLE_BIT(dwValue, dwMask, fEnable)	((fEnable) ? (dwValue) |= (dwMask) : (dwValue) &= ~(dwMask))
#define IS_EVEN(dwValue)						(((dwValue) & 1) == 0)
#define ABS(value)								((value) >= 0 ? (value) : -(value))

HRESULT	FreeBindings(DBCOUNTITEM* pcBindings, DBBINDING** prgBindings);
HRESULT	CopyBinding(DBBINDING* pDest, DBBINDING* pSource);

HRESULT FreeData(DBTYPE wType, DBLENGTH cData, void* rgData);
HRESULT	FreeBindingData(DBCOUNTITEM cBindings, const DBBINDING* rgBindings, void* pData, BOOL fSetData = FALSE);
HRESULT	FreeColAccess(ULONG* pcColAccess, DBCOLUMNACCESS** prgColAccess, void** ppData);
HRESULT FreeConstraintDesc(DBORDINAL *pcConsDesc, DBCONSTRAINTDESC **prgConsDesc, BOOL fFree = TRUE);


////////////////////////////////////////////////////////////////////////////
// Window functions
//
////////////////////////////////////////////////////////////////////////////
BOOL	IsUnicodeOS();


////////////////////////////////////////////////////////////////////////////
// DBTYPE functions
//
////////////////////////////////////////////////////////////////////////////
BOOL IsFixedType(DBTYPE wType);
BOOL IsVariableType(DBTYPE wType);
BOOL IsNumericType(DBTYPE wType);

WCHAR*			GetDBTypeName(DBTYPE wType);
DBTYPE			GetDBType(WCHAR* pwszName);
HRESULT			GetDBTypeMaxSize(DBTYPE wType, DBLENGTH* pulMaxSize = NULL, BYTE* pbPrecision = NULL, BYTE* pbScale = NULL);

WCHAR*			GetVariantTypeName(DBTYPE wType);
DBTYPE			GetVariantType(WCHAR* pwszName);
			  
WCHAR*			GetColName(const DBCOLUMNINFO* pColInfo);
DBLENGTH		GetMaxDisplaySize(DBTYPE wBindingType, DBTYPE wBackendType, DBLENGTH ulColumnSize, DBLENGTH ulMaxVarSize);





////////////////////////////////////////////////////////////////////////////
// OLE DB General Helper functions
//
////////////////////////////////////////////////////////////////////////////
INT			ConvertToMBCS(LPCWSTR pwsz, CHAR* psz, INT chDst);
INT			ConvertToMBCS(LPCWSTR pwsz, INT cchSrc, CHAR* psz, INT cchDst);
INT			ConvertToWCHAR(LPCSTR psz, WCHAR* pwsz, INT cbDst);
INT			ConvertToWCHAR(LPCSTR psz, INT cchSrc, WCHAR* pwsz, INT ccbDst);

CHAR*		ConvertToMBCS(LPCWSTR pwsz);
WCHAR*		ConvertToWCHAR(LPCSTR psz);
CHAR*		strDuplicate(LPCSTR psz);
WCHAR*		wcsDuplicate(LPCWSTR pwsz);

WCHAR*		StackFormat(LPCWSTR pwszFmt, ...);
WCHAR*		StringFormat(WCHAR* pwszDest, DBLENGTH ulStrLen, LPCWSTR pwszFmt, ...);
WCHAR*		StringCopy(WCHAR* pwszDest, LPCWSTR pwszSource, DBLENGTH cMaxLen);
CHAR*		StringCopy(CHAR* pszDest, LPCSTR pszSource, DBLENGTH cMaxLen);
BOOL		StringCompare(LPCWSTR pwsz1, LPCWSTR pwsz2);
BOOL		StringCompareI(LPCWSTR pwsz1, LPCWSTR pwsz2);

WCHAR*		GetProgID(REFCLSID clsid);
HRESULT		CLSIDFromStringEx(WCHAR* pwszString, CLSID* pCLSID);
HRESULT		CLSIDFromProgIDEx(WCHAR* pwszString, CLSID* pCLSID);
LONG		RunSysCommand(WCHAR* pwszCommand);
HINSTANCE	LoadDll(WCHAR* pwszDll);

HRESULT		DBIDToString(const DBID* pDBID, WCHAR* pwsz, ULONG ulMaxLen);
void		DBIDFree(DBID* pDBID);
HRESULT		DBIDCopy(DBID* pDst, const DBID* pSrc);
BOOL		DBIDEqual(const DBID* pDst, const DBID* pSrc);

HRESULT		PostProcessString(WCHAR* pwsz, DBLENGTH ulMaxSize, DWORD dwFlags);

//Conversions
HRESULT DataConvert
(
	DBSTATUS		 dbSrcStatus,		// @parm IN | source data status
	DBLENGTH		 cbSrcLength,		// @parm IN | size of source data
	DBLENGTH		 cbSrcMaxLength,	// @parm IN | max size of input buffer
	DBTYPE			 wSrcType,			// @parm IN | type of source data
	void*			 pSrcValue,			// @parm IN | ptr to source data
	BYTE			 bPrecision,		// @parm IN | precision to use for dst
	BYTE			 bScale,			// @parm IN | scale to use for dst

	DBTYPE			 wDstType,			// @parm IN | type of output data
	DBSTATUS*		 pdbDstStatus,		// @parm OUT | ptr to output data status
	DBLENGTH*		 pcbDstLength,		// @parm OUT | ptr to size of output data
	void*			 pDstValue,			// @parm OUT | ptr to output data
	DBLENGTH		 cbDstMaxLength,	// @parm IN | max size of output buffer
	DWORD			 dwConvFlags		// @parm IN | Conversion flags
);



///////////////////////////////////////////////////////////////
// CVariant
//
///////////////////////////////////////////////////////////////
class CVariant : public tagVARIANT
{
public:
	//constructors
	CVariant();
	~CVariant();

	HRESULT Init();
	HRESULT Clear();
	HRESULT Attach(VARIANT* pSrc);
	HRESULT Detach(VARIANT* pDest);

	HRESULT Copy(VARIANT* pSrc);
	HRESULT ChangeType(VARTYPE vt, VARIANT* pSrc = NULL);

protected:
};


HRESULT		VariantInitFast(VARIANT* pVariant);
HRESULT		VariantCopyFast(VARIANT* pVarDest, VARIANT* pVarSrc);
HRESULT		VariantClearFast(VARIANT* pVariant);
HRESULT		VariantChangeFast(VARIANT* pVarDest, VARIANT* pVarSrc, LCID lcid, USHORT wFlags, VARTYPE vt);

HRESULT		VariantToString(VARIANT* pVariant, WCHAR* pwsz, DBLENGTH ulMaxSize, DWORD dwFlags);
HRESULT		StringToVariant(WCHAR* pwsz, VARTYPE vt, VARIANT* pVariant, DWORD dwFlags = CONV_ALPHABOOL | CONV_VARBOOL);

HRESULT		SafeArrayToString(SAFEARRAY* pSafeArray, DBTYPE wType, WCHAR* pwsz, DBLENGTH ulMaxSize);
HRESULT		StringToSafeArray(WCHAR* pwsz, DBTYPE wType, SAFEARRAY** ppSafeArray);

HRESULT		StringToVector(WCHAR* pwszBuffer, DBTYPE wType, DBVECTOR* pVector);
HRESULT		VectorToString(DBVECTOR* pVector, DBTYPE wType, WCHAR* pwszBuffer, DBLENGTH ulMaxSize);



////////////////////////////////////////////////////////////////////////////
// Windows functions
//
////////////////////////////////////////////////////////////////////////////
void*		SetThis(HWND hWnd, void* pThis);
void*		GetThis(HWND hWnd);

INT			wMessageBox(HWND hDlg, UINT uiStyle, WCHAR* pwszTitle, WCHAR* pwszFmt, ...);
LRESULT		wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszText);
LRESULT		wSendMessageFmt(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszFmt, ...);
WCHAR*		wGetWindowText(HWND hWnd);

BOOL		CenterDialog(HWND hDlg);
BOOL		MoveWindow(HWND hWnd, ULONG x, ULONG y);
SIZE		GetClientSize(HWND hWnd);
SIZE		GetWindowSize(HWND hWnd);
BOOL		GetWindowPos(HWND hWnd, POINTS* pPTS);
RECT		GetClientCoords(HWND hWndParent, HWND hWnd);
void		SyncSibling(HWND hwndLstChg,HWND hwndLstSrc);
INT_PTR		WINAPI SynchSubProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL		ConvertToLONG(LPCWSTR pwszText, LONG* plValue, LONG lMin = LONG_MIN, LONG lMax = LONG_MAX, INT iBase = 10);
BOOL		GetEditBoxValue(HWND hEditWnd, LONG* plValue, LONG lMin = LONG_MIN, LONG lMax = LONG_MAX, BOOL fAllowEmpty = FALSE);
ULONG_PTR	GetMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition);
BOOL		SetMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition, ULONG_PTR dwItemData);
BOOL		SetSubMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition, ULONG_PTR dwItemData);

HRESULT		BrowseOpenFileName(HINSTANCE hInstance, HWND hWnd, WCHAR* pwszTitle, WCHAR* pwszFileName, ULONG ulMaxSize, WCHAR* pwszExtension = NULL, WCHAR* pwszFilter = L"All Files (*.*)\0*.*\0\0", DWORD* pdwFileOffset = NULL, DWORD dwFlags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT	| OFN_SHAREAWARE);
HRESULT		BrowseSaveFileName(HINSTANCE hInstance, HWND hWnd, WCHAR* pwszTitle, WCHAR* pwszFileName, ULONG ulMaxSize, WCHAR* pwszExtension = NULL, WCHAR* pwszFilter = L"All Files (*.*)\0*.*\0\0", DWORD* pdwFileOffset = NULL, DWORD dwFlags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_CREATEPROMPT);

HRESULT		CreateDefFileName(LPCWSTR pwszFileName, WCHAR* pwszFullPath, ULONG ulMaxSize, BOOL fCurrentDir = TRUE);


///////////////////////////////////////////////////////////////
// Registry
//
///////////////////////////////////////////////////////////////
HRESULT		CreateRegKey(HKEY hRootKey, WCHAR* pwszKeyName, HKEY* phKey, REGSAM samDesired = KEY_READ | KEY_WRITE);
HRESULT		OpenRegKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD ulOptions, REGSAM samDesired, HKEY* phKey);

HRESULT		GetRegEnumKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszSubKeyName, ULONG cBytes);
HRESULT		GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR** ppwszValueName);
HRESULT		GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszValueName, ULONG* pcBytes);

HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue, ULONG cBytes);
HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR** ppwszValue);
HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG* pulValue);
HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cbSize, ULONG* pcbActualSize, ULONG dwType = REG_BINARY);

HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue);
HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG ulValue);
HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cbSize, ULONG dwType = REG_BINARY);

HRESULT		DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName);
HRESULT		DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, BOOL fSubKeys);
	
HRESULT		CloseRegKey(HKEY hKey);


/////////////////////////////////////////////////////////////////////
// Memory debugging code
//
/////////////////////////////////////////////////////////////////////
void InternalTrace(LPCWSTR pwszText);
void InternalTraceFmt(LPCWSTR pwszFmt, ...);

#define ASSERT(exp)		_ASSERTE(exp)
#define ASSERTC(exp)	{ if(!(exp)) { ASSERT(!#exp); goto CLEANUP;		} }


#ifdef _DEBUG
#define TRACE			InternalTrace
#define TRACEFMT		InternalTraceFmt
#else  //_DEBUG
#define TRACE			if(0) InternalTrace
#define TRACEFMT		if(0) InternalTraceFmt
#endif //_DEBUG

//DebugBreak
//NOTE:  We define this ourselves, instead of just using _CrtDebugBreak, since
//we want this feature to be available for both debug and retail builds...
#ifdef _M_IX86
#define BREAKINTO() __asm { int 3 }
#else
#define BREAKINTO() DebugBreak()
#endif //_M_IX86



///////////////////////////////////////////////////////////////
// Static Strings Messages
//
///////////////////////////////////////////////////////////////

//General Status
extern WCHAR wsz_SUCCESS[];				
extern WCHAR wsz_WARNING[];				
extern WCHAR wsz_INFO[];

extern WCHAR wsz_ERROR[];				
extern WCHAR wsz_EXCEPTION[];
extern WCHAR wsz_ERRORINFO[];			
								 
//General String Values
extern WCHAR wsz_INVALID_VALUE_[];	


#endif //_COMMON_H_
