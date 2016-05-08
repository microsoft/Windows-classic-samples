//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module ERROR.CPP
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "WinMain.h"
#include "Error.h"

#include <olectl.h>	  // IConnectionPoints interface


////////////////////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////////////////////

//Displays values like VALUE as   VALUE , L"VALUE"
#define VALUE_WCHAR(value) value, L#value


typedef struct _INTERFACEMAP
{
	const IID*	pIID;				// The sql type value
	WCHAR*		pwszInterface;		// Name for display
} INTERFACEMAP;


INTERFACEMAP rgInterfaceMap[] =
{
	//IUnknown
		& VALUE_WCHAR(IID_IUnknown),

	//TEnumerator
		& VALUE_WCHAR(IID_IParseDisplayName),
		& VALUE_WCHAR(IID_ISourcesRowset),

	//TDataSource
		& VALUE_WCHAR(IID_IDBInitialize),
		& VALUE_WCHAR(IID_IDBProperties),
		& VALUE_WCHAR(IID_IDBCreateSession),
		& VALUE_WCHAR(IID_IDBInfo),
		& VALUE_WCHAR(IID_IPersist),
		& VALUE_WCHAR(IID_IDBDataSourceAdmin),
		& VALUE_WCHAR(IID_IPersistFile),
		& VALUE_WCHAR(IID_ISupportErrorInfo),

	//TSession
		& VALUE_WCHAR(IID_IGetDataSource),
		& VALUE_WCHAR(IID_IOpenRowset),
		& VALUE_WCHAR(IID_ISessionProperties),
		& VALUE_WCHAR(IID_IDBCreateCommand),
		& VALUE_WCHAR(IID_IDBSchemaRowset),
		& VALUE_WCHAR(IID_IIndexDefinition),
		& VALUE_WCHAR(IID_ITableDefinition),
		& VALUE_WCHAR(IID_ITransactionJoin),
		& VALUE_WCHAR(IID_ITransactionLocal),
		& VALUE_WCHAR(IID_ITransactionObject),

	//TCommand
		& VALUE_WCHAR(IID_IAccessor),
		& VALUE_WCHAR(IID_IColumnsInfo),
		& VALUE_WCHAR(IID_ICommand),
		& VALUE_WCHAR(IID_ICommandProperties),
		& VALUE_WCHAR(IID_ICommandText),
		& VALUE_WCHAR(IID_IConvertType),
		& VALUE_WCHAR(IID_IColumnsRowset),
		& VALUE_WCHAR(IID_ICommandPrepare),
		& VALUE_WCHAR(IID_ICommandWithParameters),
	
	//TRowset
		& VALUE_WCHAR(IID_IRowset),
		& VALUE_WCHAR(IID_IRowsetInfo),
		& VALUE_WCHAR(IID_IColumnsRowset),
		& VALUE_WCHAR(IID_IConnectionPointContainer),
		& VALUE_WCHAR(IID_IRowsetChange),
		& VALUE_WCHAR(IID_IRowsetIdentity),
		& VALUE_WCHAR(IID_IRowsetLocate),
		& VALUE_WCHAR(IID_IRowsetResynch),
		& VALUE_WCHAR(IID_IRowsetScroll),
		& VALUE_WCHAR(IID_IRowsetUpdate),
	
	//TIndex
		& VALUE_WCHAR(IID_IRowsetIndex),
	
	//TError
		& VALUE_WCHAR(IID_IErrorInfo),
		& VALUE_WCHAR(IID_IErrorRecords),
		& VALUE_WCHAR(IID_ISQLErrorInfo),
};


////////////////////////////////////////////////////////////////////////
// ERRORMAP
//
////////////////////////////////////////////////////////////////////////
typedef struct _ERRORMAP
{
	HRESULT		hr;			// HRESULT
	WCHAR*		pwszError;	// Name
} ERRORMAP;

ERRORMAP rgErrorMap[] =
{
	 VALUE_WCHAR(NULL),

	 //System Errors
	 VALUE_WCHAR(E_FAIL),
	 VALUE_WCHAR(E_INVALIDARG),
	 VALUE_WCHAR(E_OUTOFMEMORY),
	 VALUE_WCHAR(E_NOINTERFACE),
	 VALUE_WCHAR(REGDB_E_CLASSNOTREG),
	 VALUE_WCHAR(CLASS_E_NOAGGREGATION),
     VALUE_WCHAR(E_UNEXPECTED),
     VALUE_WCHAR(E_NOTIMPL),
     VALUE_WCHAR(E_POINTER),
     VALUE_WCHAR(E_HANDLE),
     VALUE_WCHAR(E_ABORT),
     VALUE_WCHAR(E_ACCESSDENIED),
     VALUE_WCHAR(E_PENDING),

	 //OLE DB Errors
     VALUE_WCHAR(DB_E_BADACCESSORHANDLE),
     VALUE_WCHAR(DB_E_ROWLIMITEXCEEDED),
     VALUE_WCHAR(DB_E_READONLYACCESSOR),
     VALUE_WCHAR(DB_E_SCHEMAVIOLATION),
     VALUE_WCHAR(DB_E_BADROWHANDLE),
     VALUE_WCHAR(DB_E_OBJECTOPEN),
     VALUE_WCHAR(DB_E_CANTCONVERTVALUE),
     VALUE_WCHAR(DB_E_BADBINDINFO),
     VALUE_WCHAR(DB_SEC_E_PERMISSIONDENIED),
     VALUE_WCHAR(DB_E_NOTAREFERENCECOLUMN),
     VALUE_WCHAR(DB_E_NOCOMMAND),
     VALUE_WCHAR(DB_E_BADBOOKMARK),
     VALUE_WCHAR(DB_E_BADLOCKMODE),
     VALUE_WCHAR(DB_E_PARAMNOTOPTIONAL),
     VALUE_WCHAR(DB_E_BADCOLUMNID),
     VALUE_WCHAR(DB_E_BADRATIO),
     VALUE_WCHAR(DB_E_ERRORSINCOMMAND),
     VALUE_WCHAR(DB_E_CANTCANCEL),
     VALUE_WCHAR(DB_E_DIALECTNOTSUPPORTED),
     VALUE_WCHAR(DB_E_DUPLICATEDATASOURCE),
     VALUE_WCHAR(DB_E_CANNOTRESTART),
     VALUE_WCHAR(DB_E_NOTFOUND),
     VALUE_WCHAR(DB_E_NEWLYINSERTED),
     VALUE_WCHAR(DB_E_UNSUPPORTEDCONVERSION),
     VALUE_WCHAR(DB_E_BADSTARTPOSITION),
     VALUE_WCHAR(DB_E_NOTREENTRANT),
     VALUE_WCHAR(DB_E_ERRORSOCCURRED),
     VALUE_WCHAR(DB_E_NOAGGREGATION),
     VALUE_WCHAR(DB_E_DELETEDROW),
     VALUE_WCHAR(DB_E_CANTFETCHBACKWARDS),
     VALUE_WCHAR(DB_E_ROWSNOTRELEASED),
     VALUE_WCHAR(DB_E_BADSTORAGEFLAG),
     VALUE_WCHAR(DB_E_BADSTATUSVALUE),
     VALUE_WCHAR(DB_E_CANTSCROLLBACKWARDS),
     VALUE_WCHAR(DB_E_MULTIPLESTATEMENTS),
     VALUE_WCHAR(DB_E_INTEGRITYVIOLATION),
     VALUE_WCHAR(DB_E_ABORTLIMITREACHED),
     VALUE_WCHAR(DB_E_DUPLICATEINDEXID),
     VALUE_WCHAR(DB_E_NOINDEX),
     VALUE_WCHAR(DB_E_INDEXINUSE),
     VALUE_WCHAR(DB_E_NOTABLE),
     VALUE_WCHAR(DB_E_CONCURRENCYVIOLATION),
     VALUE_WCHAR(DB_E_BADCOPY),
     VALUE_WCHAR(DB_E_BADPRECISION),
     VALUE_WCHAR(DB_E_BADSCALE),
     VALUE_WCHAR(DB_E_BADID),
     VALUE_WCHAR(DB_E_BADTYPE),
     VALUE_WCHAR(DB_E_DUPLICATECOLUMNID),
     VALUE_WCHAR(DB_E_DUPLICATETABLEID),
     VALUE_WCHAR(DB_E_TABLEINUSE),
     VALUE_WCHAR(DB_E_NOLOCALE),
     VALUE_WCHAR(DB_E_BADRECORDNUM),
     VALUE_WCHAR(DB_E_BOOKMARKSKIPPED),
     VALUE_WCHAR(DB_E_BADPROPERTYVALUE),
     VALUE_WCHAR(DB_E_INVALID),
     VALUE_WCHAR(DB_E_BADACCESSORFLAGS),
     VALUE_WCHAR(DB_E_BADSTORAGEFLAGS),
     VALUE_WCHAR(DB_E_BYREFACCESSORNOTSUPPORTED),
     VALUE_WCHAR(DB_E_NULLACCESSORNOTSUPPORTED),
     VALUE_WCHAR(DB_E_NOTPREPARED),
     VALUE_WCHAR(DB_E_BADACCESSORTYPE),
     VALUE_WCHAR(DB_E_WRITEONLYACCESSOR),
     VALUE_WCHAR(DB_SEC_E_AUTH_FAILED),
     VALUE_WCHAR(DB_E_CANCELED),
     VALUE_WCHAR(DB_E_BADSOURCEHANDLE),
     VALUE_WCHAR(DB_E_PARAMUNAVAILABLE),
     VALUE_WCHAR(DB_E_ALREADYINITIALIZED),
     VALUE_WCHAR(DB_E_NOTSUPPORTED),
     VALUE_WCHAR(DB_E_MAXPENDCHANGESEXCEEDED),
     VALUE_WCHAR(DB_E_BADORDINAL),
     VALUE_WCHAR(DB_E_PENDINGCHANGES),
     VALUE_WCHAR(DB_E_DATAOVERFLOW),
     VALUE_WCHAR(DB_E_BADHRESULT),
     VALUE_WCHAR(DB_E_BADLOOKUPID),
     VALUE_WCHAR(DB_E_BADDYNAMICERRORID),
     VALUE_WCHAR(DB_E_PENDINGINSERT),
     VALUE_WCHAR(DB_E_BADCONVERTFLAG),
     VALUE_WCHAR(DB_S_ROWLIMITEXCEEDED),
     VALUE_WCHAR(DB_S_COLUMNTYPEMISMATCH),
     VALUE_WCHAR(DB_S_TYPEINFOOVERRIDDEN),
     VALUE_WCHAR(DB_S_BOOKMARKSKIPPED),
     VALUE_WCHAR(DB_S_ENDOFROWSET),
     VALUE_WCHAR(DB_S_COMMANDREEXECUTED),
     VALUE_WCHAR(DB_S_BUFFERFULL),
     VALUE_WCHAR(DB_S_NORESULT),
     VALUE_WCHAR(DB_S_CANTRELEASE),
     VALUE_WCHAR(DB_S_DIALECTIGNORED),
     VALUE_WCHAR(DB_S_UNWANTEDPHASE),
     VALUE_WCHAR(DB_S_UNWANTEDREASON),
     VALUE_WCHAR(DB_S_COLUMNSCHANGED),
     VALUE_WCHAR(DB_S_ERRORSRETURNED),
     VALUE_WCHAR(DB_S_BADROWHANDLE),
     VALUE_WCHAR(DB_S_DELETEDROW),
     VALUE_WCHAR(DB_S_STOPLIMITREACHED),
     VALUE_WCHAR(DB_S_LOCKUPGRADED),
     VALUE_WCHAR(DB_S_PROPERTIESCHANGED),
     VALUE_WCHAR(DB_S_ERRORSOCCURRED),
     VALUE_WCHAR(DB_S_PARAMUNAVAILABLE),
     VALUE_WCHAR(DB_S_MULTIPLECHANGES),
};


////////////////////////////////////////////////////////////////////////
// WCHAR* GetErrorName
//
////////////////////////////////////////////////////////////////////////
WCHAR* GetErrorName(HRESULT hr)
{
	for(ULONG i=0; i<NUMELE(rgErrorMap); i++)	
	{
		if(hr == rgErrorMap[i].hr) 
			return rgErrorMap[i].pwszError;
	}

	//Otherwise just return Unknown
	return rgErrorMap[0].pwszError;
}


////////////////////////////////////////////////////////////////////////
// WCHAR* GetInterfaceName
//
////////////////////////////////////////////////////////////////////////
WCHAR* GetInterfaceName(REFIID riid)
{
	for(ULONG i=0; i<NUMELE(rgInterfaceMap); i++)	
	{
		if(riid == *(rgInterfaceMap[i].pIID)) 
			return rgInterfaceMap[i].pwszInterface;
	}

	//Otherwise just return IUnknown
	return rgInterfaceMap[0].pwszInterface;
}


////////////////////////////////////////////////////////////////////////
// HRESULT GetErrorRecords
//
// Get the error message generated by an OLE DB object
/////////////////////////////////////////////////////////////////////////////
HRESULT GetErrorRecords(ULONG* pcRecords, IErrorRecords** ppIErrorRecords)
{
	ASSERT(pcRecords && ppIErrorRecords);
	HRESULT hr;

	//NULL output params
	*pcRecords = 0;
	*ppIErrorRecords = NULL;
	
	ISupportErrorInfo* pISupportErrorInfo = NULL;
	IErrorInfo* pIErrorInfo = NULL;

	//See if this interface supports ErrorInfo
	//If not there is no reason to display any error
	if((hr = GetErrorInfo(0, &pIErrorInfo))==S_OK && pIErrorInfo)
	{
		//IErrorRecords may not be supported on the existing error object.
		//Some other things could have posted an error object (VB) for example...
		QTESTC(hr = pIErrorInfo->QueryInterface(IID_IErrorRecords, (void**)ppIErrorRecords));
		XTESTC(hr = (*ppIErrorRecords)->GetRecordCount(pcRecords));
	}
		
CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo);
	SAFE_RELEASE(pIErrorInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT GetSqlErrorInfo
//
// Get the error message generated by an OLE DB object
/////////////////////////////////////////////////////////////////////////////
HRESULT GetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr)
{
	ASSERT(pBstr);
	HRESULT hr = S_OK;

	ISQLErrorInfo* pISQLErrorInfo = NULL;
	LONG lNativeError = 0;

	//Get the Error Records
	if(pIErrorRecords)
	{
		//If there is ErrorInfo, GetSQLInfo for the desired record
		//ISQLErrorInfo is not mandatory
		QTESTC(hr = pIErrorRecords->GetCustomErrorObject(iRecord, IID_ISQLErrorInfo, (IUnknown**)&pISQLErrorInfo));
		
		//If there was a CustomErrorObject
		if(pISQLErrorInfo)
			hr = pISQLErrorInfo->GetSQLInfo(pBstr, &lNativeError);
	}

CLEANUP:
	SAFE_RELEASE(pISQLErrorInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayAllErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayAllErrors(HWND hWnd, HRESULT Actualhr, WCHAR* pwszFile, ULONG ulLine)
{
	HRESULT hr = S_OK;

	ULONG cRecords = 0;
	IErrorRecords* pIErrorRecords = NULL;

	//Try to display Extened ErrorInfo
	if((hr = GetErrorRecords(&cRecords, &pIErrorRecords))==S_OK) 
	{
		hr = DisplayErrorRecords(hWnd, cRecords, pIErrorRecords, pwszFile, ulLine);
	}
	//If not available, display MSG Box with info
	else
	{
		//display the Error
		wMessageBox(hWnd, (hWnd ? MB_APPLMODAL : MB_TASKMODAL) | MB_ICONEXCLAMATION | MB_OK, wsz_ERRORINFO, 
			L"Interface: %s\nResult: %x = %s\n\nIErrorInfo: %s\n\nFile: %s\nLine: %d", L"Unknown", Actualhr, GetErrorName(Actualhr), L"Error Object not available", pwszFile, ulLine);
	}

	
	SAFE_RELEASE(pIErrorRecords);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayErrorRecords
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayErrorRecords(HWND hWnd, ULONG cRecords, IErrorRecords* pIErrorRecords, WCHAR* pwszFile, ULONG ulLine)
{
	HRESULT hr = S_OK;

	IErrorInfo* pIErrorInfo = NULL;
	BSTR bstrErrorInfo = NULL;
	BSTR bstrSQLInfo = NULL;

	LCID lcid = GetSystemDefaultLCID(); 

	//Get the Error Records
	if(cRecords && pIErrorRecords)
	{
		LONG lNativeError = 0;
		ERRORINFO ErrorInfo;

		//Loop through the records
		for(ULONG i=0; i<cRecords; i++)
		{
			//GetErrorInfo
			XTESTC(hr = pIErrorRecords->GetErrorInfo(i,lcid,&pIErrorInfo));
				
			//Get the Description
			XTESTC(hr = pIErrorInfo->GetDescription(&bstrErrorInfo));
				
			//Get the Basic ErrorInfo
			XTESTC(hr = pIErrorRecords->GetBasicErrorInfo(i,&ErrorInfo));
			
			//Get the SQL Info
			GetSqlErrorInfo(i, pIErrorRecords, &bstrSQLInfo);

			//Display the Error
			if(bstrSQLInfo)
				wMessageBox(hWnd, (hWnd ? MB_APPLMODAL : MB_TASKMODAL) | MB_ICONEXCLAMATION | MB_OK, wsz_ERRORINFO, L"Interface: %s\nResult: %x = %s\n\nIErrorInfo: [%s] %s\n\nFile: %s\nLine: %d", GetInterfaceName(ErrorInfo.iid), ErrorInfo.hrError, GetErrorName(ErrorInfo.hrError), bstrSQLInfo, bstrErrorInfo, pwszFile, ulLine);
			else
				wMessageBox(hWnd, (hWnd ? MB_APPLMODAL : MB_TASKMODAL) | MB_ICONEXCLAMATION | MB_OK, wsz_ERRORINFO, L"Interface: %s\nResult: %x = %s\n\nIErrorInfo: %s\n\nFile: %s\nLine: %d", GetInterfaceName(ErrorInfo.iid), ErrorInfo.hrError, GetErrorName(ErrorInfo.hrError), bstrErrorInfo, pwszFile, ulLine);

			SAFE_RELEASE(pIErrorInfo);
			SAFE_SYSFREE(bstrErrorInfo);
			SAFE_SYSFREE(bstrSQLInfo);
		}
	}
	

CLEANUP:
	SAFE_RELEASE(pIErrorInfo);
	SAFE_SYSFREE(bstrErrorInfo);
	SAFE_SYSFREE(bstrSQLInfo);
	return hr;
}
