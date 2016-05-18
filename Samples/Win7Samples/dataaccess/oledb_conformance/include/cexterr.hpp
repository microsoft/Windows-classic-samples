//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Extended Error Class Header Module | Declaration of Extended Error Class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 03-04-96	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CExtError Elements|
//
// @subindex CExtError|
//
//---------------------------------------------------------------------------

#ifndef __CEXTERR_HPP_
#define __CEXTERR_HPP_

#include "modstandard.hpp"
#include "miscfunc.h"

//--------------------------------------------------------------------
// @Class CExtError | Extended Error Class 
//
// The class is used to validate extended error inforation.
//
//--------------------------------------------------------------------
class CExtError  
{
	// @access Protected
	protected:	
		
	// @cmember Base error object to increment when we encounter problems. <nl>
	CError	* m_pError;

	// @cmember Valid locale id. <nl>
	LCID	m_lcid;
	
	// @cmember Flag to indicate if customer error object supports ISQLErrorInfo. <nl>
	BOOL m_fIsODBCProvider;

	// @cmember Current provider's CLSID, which we use to determine m_fIsODBCProvider. <nl>
	CLSID m_ProviderClsid;

	// @cmember File name from which ValidateExtended was last called. <nl>
	WCHAR*	m_pwszFileName;

	// @cmember File line from which ValidateExtended was last called. <nl>
	ULONG	m_ulLine;

	// @cmember What ref count for error object should be, based on what we've done. <nl>
	ULONG	m_ulRefCount;

	// @cmember Interface to error object. <nl>
	IErrorInfo * m_pIErrorInfo;

	// @cmember Interface to error object. <nl>
	IErrorRecords * m_pIErrorRecords;

	// @cmember HRESULT received by method which is being verified by ValidateExtended. <nl>
	HRESULT		m_MethodReceivedHr;

	// @cmember IID for interface of method which is being verified by ValidateExtended. <nl>
	IID			m_iid;
	
	// @cmember IID for interface of method which causes the previous error. <nl>
	IID			m_PreErrIID;
		
	// @cmember HRESULT received by method which causes the previous error. <nl>
	HRESULT		m_PreErrHr;
	
	// @cmember Print the error description. <nl>
	BOOL		m_fPrint;

	// @cmember Validates the error description. <nl>
	BOOL ValidateErrorDescp(BSTR bstr);

	// @cmember Set the error message. <nl>
	void setErrorMsg(LPOLESTR *errorMsg);

	// @access Public
	public:
	
	// @cmember	CTOR. <nl>
	CExtError(CLSID ProviderClassID, CError * pError, BOOL fLocalize = FALSE);	
    virtual ~CExtError(void);
	
	// @cmember	Validates the extended error info if applicable. <nl>
	virtual BOOL ValidateExtended(HRESULT MethodReceivedHr, IUnknown * pIUnknown, IID iid, LPWSTR wszFile, ULONG ulLine);
	
	// @cmember	Validates the current error object contains correct info. <nl>
	virtual BOOL ValidateErrorObject();

	// @cmember Validates a specific error record. <nl>
	virtual BOOL ValidateErrorRecord(ULONG ulRecordNum, IErrorInfo * pRecordIErrorInfo);

	// @cmember	Verifies the error object contains correct info. <nl>
	virtual void LogString(LPWSTR wszError, BOOL fIncrementError = FALSE);

	// @cmember Validates custom error object if ISQLErrorInfo is supported, else returns TRUE. <nl>
	virtual BOOL ValidateCustomErrorObject(ULONG ulRecordNum, IUnknown * pIUnkCustom);

	// @cmember Logs all IErrorInfo for a record for manual verification. <nl>
	BOOL LogErrorInfo(ULONG ulRecordNum, IErrorInfo * pRecordIErrorInfo);

	// @cmember Generates error object, to be used in conjunction with successful. <nl>
	// method call and ValidateExtended(). <nl>
	BOOL CauseError();
	
};

//-----------------------------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------------------------
const GUID Kagera_ClassID = {0xC8B522CBL,0x5CF3,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}};
const GUID ENUMERATOR_ClassID = {0xC8B522CDL,0x5CF3,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}};
const WCHAR wszColon[] = L":  ";

//-----------------------------------------------------------------------------------------------
// Define
//-----------------------------------------------------------------------------------------------
#define ERRORMSG_BUFFER		256

//-----------------------------------------------------------------------------------------------
// Enum
//-----------------------------------------------------------------------------------------------
enum ERRORCODE
{	EC_UNEXPECTED = 0,
	EC_FAIL =1,
	EC_NOINTERFACE =2,
	EC_INVALIDARG =3,
	EC_BADACCESSORHANDLE,
	EC_ROWLIMITEXCEEDED,
	EC_READONLYACCESSOR,
	EC_SCHEMAVIOLATION,
	EC_BADROWHANDLE,
	EC_OBJECTOPEN,
	EC_BADCHAPTER,
	EC_CANTCONVERTVALUE,
	EC_BADBINDINFO,
	EC_PERMISSIONDENIED,
	EC_NOTAREFERENCECOLUMN,
	EC_LIMITREJECTED,
	EC_NOCOMMAND,
	EC_COSTLIMIT,
	EC_BADBOOKMARK,
	EC_BADLOCKMODE,
	EC_PARAMNOTOPTIONAL,
	EC_BADCOLUMNID,
	EC_BADRATIO,
	EC_BADVALUES,
	EC_ERRORSINCOMMAND,
	EC_CANTCANCEL,
	EC_DIALECTNOTSUPPORTED,
	EC_DUPLICATEDATASOURCE,
	EC_CANNOTRESTART,
	EC_NOTFOUND,
	EC_CANNOTFREE,
	EC_NEWLYINSERTED,
	EC_GOALREJECTED,
	EC_UNSUPPORTEDCONVERSION,
	EC_BADSTARTPOSITION,
	EC_NOQUERY,
	EC_NOTREENTRANT,
	EC_ERRORSOCCURRED,
	EC_NOAGGREGATION,
	EC_DELETEDROW,
	EC_CANTFETCHBACKWARDS,
	EC_ROWSNOTRELEASED,
	EC_BADSTORAGEFLAG,
	EC_BADCOMPAREOP,
	EC_BADSTATUSVALUE,
	EC_CANTSCROLLBACKWARDS,
	EC_BADREGIONHANDLE,
	EC_NONCONTIGUOUSRANGE,
	EC_INVALIDTRANSITION,
	EC_NOTASUBREGION,
	EC_MULTIPLESTATEMENTS,
	EC_INTEGRITYVIOLATION,
	EC_BADTYPENAME,
	EC_ABORTLIMITREACHED,
	EC_ROWSETINCOMMAND,
	EC_CANTTRANSLATE,
	EC_DUPLICATEINDEXID,
	EC_NOINDEX,
	EC_INDEXINUSE,
	EC_NOTABLE,
	EC_CONCURRENCYVIOLATION,
	EC_BADCOPY,
	EC_BADPRECISION,
	EC_BADSCALE,
	EC_BADTABLEID,
	EC_BADTYPE,
	EC_DUPLICATECOLUMNID,
	EC_DUPLICATETABLEID,
	EC_TABLEINUSE,
	EC_NOLOCALE,
	EC_BADRECORDNUM,
	EC_BOOKMARKSKIPPED,
	EC_BADPROPERTYVALUE,
	EC_INVALID,
	EC_BADACCESSORFLAGS,
	EC_BADSTORAGEFLAGS,
	EC_BYREFACCESSORNOTSUPPORTED,
	EC_NULLACCESSORNOTSUPPORTED,
	EC_NOTPREPARED,
	EC_BADACCESSORTYPE,
	EC_WRITEONLYACCESSOR,
	EC_AUTH_FAILED,
	EC_CANCELED,
	EC_CHAPTERNOTRELEASED,
	EC_BADSOURCEHANDLE,
	EC_PARAMUNAVAILABLE,
	EC_ALREADYINITIALIZED,
	EC_NOTSUPPORTED,
	EC_MAXPENDCHANGESEXCEEDED,
	EC_BADORDINAL,
	EC_PENDINGCHANGES,
	EC_DATAOVERFLOW,
	EC_BADHRESULT,
	EC_BADLOOKUPID,
	EC_BADDYNAMICERRORID,
	EC_PENDINGINSERT,
	EC_BADCONVERTFLAG
};


//-----------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------
#define XCHECK(pIUnknown, iid, hr) m_pExtError->ValidateExtended(hr, pIUnknown, iid,  \
	LONGSTRING(__FILE__), __LINE__)

#endif	// __CEXTERR_HPP_
