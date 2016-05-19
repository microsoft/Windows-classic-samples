//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module  CExtError Implementation Module | Implementation of 
//			extended error class derived from CError.
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
// @subindex CExtError              
//
//---------------------------------------------------------------------------

#include "privstd.h"	//Private library common precompiled header
#include "cexterr.hpp"


//--------------------------------------------------------------------
// CExtError
//
// @mfunc 
//
//--------------------------------------------------------------------
CExtError::CExtError(
			CLSID	 ProviderCLSID,	//@parm [IN]	Clsid of top level component in error chain
			CError * pError,		//@parm [IN]	Current error object
			BOOL	 fLocalize		//@parm [IN]	Print to the string
)
{	
	m_ProviderClsid = ProviderCLSID;
	m_pError		= GetModInfo()->GetErrorObject();
	m_lcid			= GetSystemDefaultLCID(); 

	// Find out if provider is ODBC related -- right now we only know of
	// ODBC Provider, so use MSDASQL's CLSID to detect if its an ODBC provider
	if (IsEqualCLSID(m_ProviderClsid, Kagera_ClassID))
		m_fIsODBCProvider = TRUE;
	else if (IsEqualCLSID(m_ProviderClsid, ENUMERATOR_ClassID))
		m_fIsODBCProvider = TRUE;
	else
		m_fIsODBCProvider = FALSE;

	m_fPrint			= fLocalize;
	m_iid				= IID_IUnknown;
	m_PreErrIID			= IID_IUnknown;
	m_MethodReceivedHr	= S_OK;
	m_PreErrHr			= S_OK;
	m_pIErrorInfo		= NULL;
	m_pIErrorRecords	= NULL;
	m_pwszFileName		= NULL;
	m_ulLine			= 0;
}


//--------------------------------------------------------------------
// ~CExtError
//
// @mfunc 
//
//--------------------------------------------------------------------
CExtError::~CExtError()
{	
	PROVIDER_FREE(m_pwszFileName);
}

//--------------------------------------------------------------------
// ValidateExtended
//
// Validates the correct behavior for the given error situation
//
// @mfunc 
//
//--------------------------------------------------------------------
BOOL CExtError::ValidateExtended(
			 HRESULT MethodReceivedHr,	//@parm [IN] HRESULT received from method
			 IUnknown * pIUnknown,		//@parm [IN] Object from which the method was called
			 IID iid,					//@parm [IN] IID of interface on which the method was called
			 LPWSTR wszFile,			//@parm [IN] File name from which the call was made
			 ULONG  ulLine				//@parm [IN] Line number from which the call was made
)
{
	BOOL				fResults			= TRUE;	
	HRESULT				hr					= E_FAIL;
	HRESULT				SupportErrorInfoHr	= E_FAIL;
	HRESULT				GetErrorInfoHr		= E_FAIL;

	ISupportErrorInfo * pISupportErrorInfo	= NULL;
	
	// Record our member vars, needs for any error logging
	m_MethodReceivedHr	= MethodReceivedHr;
	m_ulLine			= ulLine;
	m_iid				= iid;
	
	PROVIDER_FREE(m_pwszFileName);
	m_pwszFileName		= wcsDuplicate(wszFile);
	
	// Find out if this object supports any extended errors by QIing	
	if(VerifyInterface(pIUnknown, IID_ISupportErrorInfo, UNKNOWN_INTERFACE, (IUnknown**)&pISupportErrorInfo))
	{
		// Find out if this interface is capable of returning extended errors
		SupportErrorInfoHr = pISupportErrorInfo->InterfaceSupportsErrorInfo(m_iid);
		
		if (SupportErrorInfoHr == S_OK)
		{
			// Try to get an error object, set pointer to bogus value so
			// we can tell if its changed from NULL
			m_pIErrorInfo	= INVALID(IErrorInfo*);
			GetErrorInfoHr	= GetErrorInfo(0, &m_pIErrorInfo);

			// GetErrorInfo can only return S_OK or S_FALSE.  If S_FALSE
			// then there was no error object to return
			switch (GetErrorInfoHr)
			{
				case S_OK:
					// If we got an object back, Validate it contains
					// the right information
					if (m_pIErrorInfo != NULL)
					{
						// No Error should be returned if the hr is S_OK or S_FALSE
						if (m_MethodReceivedHr == S_OK || m_MethodReceivedHr == S_FALSE)
						{
							odtLog << L"Warning: Error Object exists though method returned S_OK.";
							odtLog << L"         This is legal but may indicate a failure depending on the method and provider.\n";
						}

						// Make sure the error object is OK
						if (!ValidateErrorObject())
						{
							// Only increment error count once, although
							// function may have printed more than one error
							(*m_pError)++;
							fResults = FALSE;
						}
					}
					else
					{
						// If GetErrorInfo returned S_OK it should have returned an error object
						LogString(L"GetErrorInfo returned S_OK but no error object was available.", TRUE);
						(*m_pError)++;
						fResults=FALSE;
					}
					break;

				case S_FALSE:
					// The error object must be NULL
					if (m_pIErrorInfo != NULL)
					{
						LogString(L"GetErrorInfo returned S_FALSE but returned an error object anyway.", TRUE);
						(*m_pError)++;
						fResults = FALSE;
					}

					// If the method failed we probably should have posted an error
					if (FAILED(m_MethodReceivedHr))
					{
						odtLog << L"Warning: The method supports error information, but didn't return an error object for this error.\n";
						odtLog << L"         This is legal but may indicate a failure depending on the method and provider.\n";
					}
					break;

				default:
					LogString(L"GetErrorInfo returned an unexpected HRESULT.", TRUE);
					m_pError->LogExpectedHr(S_OK);
					m_pError->LogReceivedHr(GetErrorInfoHr, m_pwszFileName, m_ulLine);
					(*m_pError)++;
					fResults = FALSE;
			}

			// Make sure release of error object is clean
			if(m_pIErrorInfo != INVALID(IErrorInfo*))
				SAFE_RELEASE(m_pIErrorInfo);
		}
		else
		{
			// We have to have received S_FALSE or its an error
			if (SupportErrorInfoHr != S_FALSE)			
			{
				LogString(L"Incorrect return code received from ISupportErrorInfo::InterfaceSupportsErrorInfo.", TRUE);							
				fResults = FALSE;
			}
		}		

		SAFE_RELEASE(pISupportErrorInfo);
	}		
	else
	{
		odtLog<<L"ISupportErrorInfo is not supported.\n";
	}

	return fResults;
}

//--------------------------------------------------------------------
// ValidateErrorObject
//
//	Verifies the current error object 
//
// @mfunc 
//
//--------------------------------------------------------------------
BOOL CExtError::ValidateErrorObject()
{
	LONG			lBeg, lEnd, lDelta, i, j;	
	ULONG			cRecords			= 0;
	BOOL			fResults			= FALSE;
	IErrorInfo *	pRecordIErrorInfo	= NULL;
	IErrorInfo **	rgRecords			= NULL;

	if(!VerifyInterface(m_pIErrorInfo, IID_IErrorRecords, ERROR_INTERFACE, (IUnknown**)&m_pIErrorRecords))
		goto DONE;

	if (!CHECK(m_pIErrorRecords->GetRecordCount(&cRecords), S_OK))
	{
		LogString(L"IErrorRecords::GetRecordCount failed.");
		goto DONE;
	}
	
	if (!cRecords)
	{
		LogString(L"Error Records is 0.");
		goto DONE;
	}

	// To mix up order in which records are retrieved, if the count is even
	// retreive them in order, if odd, retreive them in reverse order
	if (cRecords % 2)
	{
		lBeg = cRecords - 1;	//Begin at largest record element
		lEnd = -1;				//Go thru record zero, stop at -1
		lDelta = -1;			//Decrement thru records
	}
	else
	{
		lBeg = 0;				//Begin with record zero
		lEnd = cRecords;		//Go thru largest record, stop at cRecords
		lDelta = 1;				//Increment thru records

		// For even number, we will also hold onto each 
		// record til the end, so allocate space for each one
		rgRecords = (IErrorInfo **)PROVIDER_ALLOC(sizeof(IErrorInfo *) * cRecords);
		memset(rgRecords, 0, sizeof(IErrorInfo *) * cRecords);
	}

	// Validate each record
	i = lBeg;
	while(i!=lEnd)
	{				
		if (m_pIErrorRecords->GetErrorInfo(i, m_lcid, &pRecordIErrorInfo) != S_OK)
		{
			LogString(L"Error record not retreived.");
			goto DONE;
		}
		
		// Validate the individual record contains correct info
		if (!ValidateErrorRecord(i, pRecordIErrorInfo))
		{
			SAFE_RELEASE(pRecordIErrorInfo);
			goto DONE;
		}

		// Mix up the way we release/hold records, depending on odd or even record num
		if (cRecords % 2)
		{
			// Always release each record right away if odd number of records
			SAFE_RELEASE(pRecordIErrorInfo);
		}
		else
			// Save interfaces in array until we're ready to release in DONE
			rgRecords[i] = pRecordIErrorInfo;

		i = i+lDelta;
	}	

	fResults = TRUE;

DONE:

	// Release all interfaces if we've saved them due to even record count  
	if (!(cRecords % 2) && rgRecords)
		for (j=0; j<i; j++)
			SAFE_RELEASE(rgRecords[j]);

	SAFE_RELEASE(m_pIErrorRecords);
	PROVIDER_FREE(rgRecords);
	return fResults;
}

//--------------------------------------------------------------------
// LogString
//
// Logs a message and optionally increments the error count
//
// @mfunc 
//
//--------------------------------------------------------------------
void CExtError::LogString(
					LPWSTR wszError,		//@parm [IN] String to log
					BOOL fIncrementError	//@parm [IN] Whether or not to increment error count
)
{
	// Increment the error count if specified
	if (fIncrementError)
		(*m_pError)++;

	// Log the error message with file, line and dashes
	odtLog << wszError << wszNewLine;
	odtLog << wszFile << m_pwszFileName << wszLine << m_ulLine << wszNewLine;
	odtLog << wszDashes << wszNewLine;
}

//--------------------------------------------------------------------
// ValidateErrorRecord
//
// Validates on specific error record.  If its record zero,
// also verifies that same info is returned for base error object.
//
// @mfunc 
//
//--------------------------------------------------------------------
BOOL CExtError::ValidateErrorRecord(
			ULONG	ulRecordNum,			//@parm [IN] Record number to verify
			IErrorInfo * pRecordIErrorInfo	//@parm [IN] Interface to record to verify
)
{
	BOOL		fResults	= TRUE;
	IUnknown *	pIUnkCustom = NULL;
	ERRORINFO	ErrorInfo;
	DISPPARAMS  DispParam;

	// First log all info from IErrorInfo for manual verification
	if (!LogErrorInfo(ulRecordNum, pRecordIErrorInfo))
		return FALSE;

	// Now verify Basic Error Info
	if (!CHECK(m_pIErrorRecords->GetBasicErrorInfo(ulRecordNum, &ErrorInfo), S_OK))
	{
		LogString(L"GetBasicErrorInfo failed.", TRUE);
		fResults = FALSE;
	}
	else
	{
		// We know what the values should be if its record zero
		if (ulRecordNum == 0)
		{
			if (ErrorInfo.hrError != m_MethodReceivedHr)
			{
				LogString(L"Error Record 0's hrError does not match what was returned from method.", TRUE);
				if (ErrorInfo.hrError == m_PreErrHr) 
					LogString(L"Error Record 0's hrError matches what was returned from the previous error.", TRUE);
				fResults = FALSE;
			}	
			

			if (ErrorInfo.clsid != m_ProviderClsid)
			{
				//The CLSID of the Error "should" match the CLSID Provider, but
				//Doesn't have to, the provider may be using another component
				//Which is posting its own ErrorInfo objects (ie CursorEngine)
				//Just display they mismtached, and continue (FALSE = don't increment error count...)
				LogString(L"Error Record 0's clsid does not match what was returned from method.", FALSE);
			}
			
			if (ErrorInfo.iid != m_iid)
			{
				LogString(L"Error Record 0's iid does not match what was returned from method.", TRUE);		
				fResults = FALSE;
			}
		}
	}
	
	// Now verify Error parameters
	if (!CHECK(m_pIErrorRecords->GetErrorParameters(ulRecordNum, &DispParam), S_OK))
	{
		LogString(L"GetErrorParameters failed.", TRUE);
		fResults = FALSE;
	}
	else
	{
		// We don't know much about what these are, but we 
		// do know that we have to release them
		if (DispParam.rgvarg) {
			for(ULONG i=0; i < DispParam.cArgs; i++)
				VariantClear(&(DispParam.rgvarg[i]));
		}

		PROVIDER_FREE(DispParam.rgvarg);
		PROVIDER_FREE(DispParam.rgdispidNamedArgs);
	}
	
	// Now verify Custom Error object
	if (!CHECK(m_pIErrorRecords->GetCustomErrorObject(ulRecordNum, IID_IUnknown, &pIUnkCustom), S_OK))
	{
		LogString(L"GetCustomErrorObject failed.", TRUE);
		fResults = FALSE;
	}
	else
	{
		// Validate custom error object
		if (!ValidateCustomErrorObject(ulRecordNum, pIUnkCustom))
			fResults = FALSE;

		// Release custom error object
		SAFE_RELEASE(pIUnkCustom);
	}

	return fResults;
}


//--------------------------------------------------------------------
// ValidateCustomErrorObject
//
// Verifies a specific error record's custom error object can be asked
// for, and if the object exists and suports ISQLErrorInfo, that interface
// is used to retreive the SQL State and log it.
//
// @mfunc 
//
//--------------------------------------------------------------------
BOOL CExtError::ValidateCustomErrorObject(
			ULONG	ulRecordNum,
			IUnknown * pIUnkCustom	//@parm [IN] Interface to record to verify
)
{	
	BOOL			fResults		= TRUE;
	ISQLErrorInfo * pISQLErrorInfo	= NULL;
	BSTR			bstrSQLState	= NULL;
	LONG __RPC_FAR  nativeError;

	// SQLState is only five characters, put null terminator at end of those five
	// No custom error object is OK, just return
	if (!pIUnkCustom)
		return TRUE;

	// If it doesn't support ISQLErrorInfo, we don't have anything to verify
	if(!VerifyInterface(pIUnkCustom, IID_ISQLErrorInfo, CUSTOMERROR_INTERFACE, (IUnknown**)&pISQLErrorInfo))
		return TRUE;

	if (!CHECK(pISQLErrorInfo->GetSQLInfo(&bstrSQLState, &nativeError), S_OK))
	{
		LogString(L"GetSQLState failed", TRUE);
		fResults = FALSE;
	}
	else
	{
		odtLog << L"SQL State for record " << ulRecordNum << wszColon;
		if(bstrSQLState)
			odtLog << bstrSQLState << wszNewLine;

		SysFreeString(bstrSQLState);
	}

	// Release ISQLErrorInfo interface 
	SAFE_RELEASE(pISQLErrorInfo);
	return fResults;
}


//--------------------------------------------------------------------
// LogErrorInfo
//
// Retreives all IErrorInfo info from a record and logs it
//
// @mfunc 
//
//--------------------------------------------------------------------
BOOL CExtError::LogErrorInfo(ULONG ulRecordNum, IErrorInfo * pRecordIErrorInfo)
{
	BOOL	fResult = TRUE;
	BSTR	bstr	= NULL;
	BSTR	bstr2	= NULL;
	
	DWORD	dwHelpContext;
	DWORD	dwHelpContext2;
	
	GUID	guid;
	GUID	guid2;

	// Log GetDescription results
	if (!CHECK(pRecordIErrorInfo->GetDescription(&bstr), S_OK) || 
		bstr == NULL)
	{
		LogString(L"GetDescription failed.");
		fResult = FALSE;
	}
	else
	{
		// Make sure base record returns the same thing if this is the zeroth record
		if (ulRecordNum == 0)
		{
			if (!CHECK(m_pIErrorInfo->GetDescription(&bstr2), S_OK) ||
				bstr2 == NULL)
			{
				LogString(L"GetDescription on base error object failed.");
				fResult = FALSE;
			}
			else
			{
				//Compare BSTR's, (DONT FREE MEMORY)
				if (!CompareDBTypeData(&bstr, &bstr2, DBTYPE_BSTR, 0, NULL, NULL, NULL, FALSE))

				{
					LogString(L"GetDescription for base error object did not match that of record zero.", TRUE);
					fResult = FALSE;
				}	
			}
			
		}
		
		//Try to match the ErrorDescription with the "Standard" Description
		//That we know about.  If it does not match its not an error, just that
		//we cannot verify thats it looks correct, we will dump it to the screen
		//for visual verfication...
		ValidateErrorDescp(bstr);
	}

	SYSSTRING_FREE(bstr);
	SYSSTRING_FREE(bstr2);

	if (!CHECK(pRecordIErrorInfo->GetHelpFile(&bstr), S_OK))
	{
		LogString(L"GetHelpFile failed.");
		fResult = FALSE;
	}
	else
	{
		// Make sure base record returns the same thing if this is the zeroth record
		if (ulRecordNum == 0)
		{
			if (!CHECK(m_pIErrorInfo->GetHelpFile(&bstr2), S_OK))
			{
				LogString(L"GetHelpFile on base error object failed.");
				fResult = FALSE;
			}
			else if ((bstr!=NULL) && (bstr2!=NULL))
			{
				//Compare BSTR's, (DONT FREE MEMORY)
				if (!CompareDBTypeData(&bstr, &bstr2, DBTYPE_BSTR, 0, NULL, NULL, NULL, FALSE))
				{
					LogString(L"GetHelpFile for base error object did not match that of record zero.", TRUE);
					fResult = FALSE;
				}
			}

			// Either bstr or bstr2 is not NULL
			else if ((bstr != NULL) || (bstr2 != NULL))
			{
				// Since bstr != NULL
				LogString(L"GetHelpFile for base error object did not match that of record zero.", TRUE);
				fResult = FALSE;
			}

		}
	}
	
	SYSSTRING_FREE(bstr);
	SYSSTRING_FREE(bstr2);

	// Log GetSource results
	if (!CHECK(pRecordIErrorInfo->GetSource(&bstr), S_OK) || (bstr == NULL))
	{
		LogString(L"GetSource failed.");
		fResult = FALSE;
	}
	else
	{
		// Make sure base record returns the same thing if this is the zeroth record
		if (ulRecordNum == 0)
		{
			if (!CHECK(m_pIErrorInfo->GetSource(&bstr2), S_OK))
			{
				LogString(L"GetSource on base error object failed.");
				fResult = FALSE;
			}
			else
			{
				//Compare BSTR's, (DONT FREE MEMORY)
				if (!CompareDBTypeData(&bstr, &bstr2, DBTYPE_BSTR, 0, NULL, NULL, NULL, FALSE))
				{
					LogString(L"GetSource for base error object did not match that of record zero.", TRUE);
					fResult = FALSE;
				}
			}
		}	
	}

	// Log GetHelpContext results
	if (!CHECK(pRecordIErrorInfo->GetHelpContext(&dwHelpContext), S_OK)) 
	{
		LogString(L"GetHelpContext failed.");
		fResult = FALSE;
	}
	else
	{		
		// Make sure base record returns the same thing if this is the zeroth record
		if (ulRecordNum == 0)
		{
			if (!CHECK(m_pIErrorInfo->GetHelpContext(&dwHelpContext2), S_OK))
			{
				LogString(L"GetHelpContext on base error object failed.");
				fResult = FALSE;
			}
			else
			{				
				if (dwHelpContext != dwHelpContext2)				
				{
					LogString(L"GetHelpContext for base error object did not match that of record zero.", TRUE);
					fResult = FALSE;
				}
			}
		}
	}


	// Also make note of whether or not GetGUID returns same interface
	// as returned the error for record zero (the only one we're guaranteed
	// to know).  If they don't match, it may just mean that the interface
	// is returning an error it did not define.
	if (!CHECK(pRecordIErrorInfo->GetGUID(&guid), S_OK))
	{
		LogString(L"GetGUID failed. ");
		fResult = FALSE;
	}
	else
	{		
		// We don't want to print the GUID, so only verify it when we 
		// know what it might be -- ie, for record zero it should match
		// the iid returned with GetBasicErrorInfo
		if (ulRecordNum == 0)
		{
			// remember guid and will be compared with the iid member of the ERRORINFO structure returned by
			// IErrorRecords::GetBasicErrorInfo
			m_iid = guid;
		
			// Now try getting it on the base error object
			if (!CHECK(m_pIErrorInfo->GetGUID(&guid2), S_OK))
			{
				LogString(L"GetGUID on base error object failed.");
				fResult = FALSE;
			}
			else
			{				
				if (guid != guid2)				
				{		
					LogString(L"GetGUID for base error object did not match that of record zero.", TRUE);				
					fResult = FALSE;
				}
			}
		}
	}

	SYSSTRING_FREE(bstr);
	SYSSTRING_FREE(bstr2);
	return fResult;
}


//--------------------------------------------------------------------
// ValidateErrorDescp
//
// Validate the error description.
// Returns TRUE if the error description matches the error code returned,
// else FALSE.
//
// @mfunc
//
//--------------------------------------------------------------------
BOOL CExtError::ValidateErrorDescp(BSTR bstr)
{
	BOOL fResult	 = FALSE;
	WCHAR * pwszExpDesc = NULL;
	
	LPOLESTR errorMsg[EC_BADCONVERTFLAG +1];

	// Check the BSTR
	if (!bstr)
		return FALSE;

	// set error msg first
	setErrorMsg(errorMsg);

	// Compare the error description with the standard msg
	// of each error return code
	// some of HR codes are commented out right now, need to uncomment
	// or remove when 2.0 headers are available.
	switch (m_MethodReceivedHr)
	{
		case (E_UNEXPECTED):
			pwszExpDesc=errorMsg[EC_UNEXPECTED];	
			break;
		case (E_NOINTERFACE):
			pwszExpDesc=errorMsg[EC_NOINTERFACE];	
			break;
		case (E_INVALIDARG):
			pwszExpDesc=errorMsg[EC_INVALIDARG];	
			break;
		case (DB_E_BADACCESSORHANDLE):
			pwszExpDesc=errorMsg[EC_BADACCESSORHANDLE];
			break;
		case (DB_E_ROWLIMITEXCEEDED):
			pwszExpDesc=errorMsg[EC_ROWLIMITEXCEEDED];
			break;
		case (DB_E_READONLYACCESSOR):
			pwszExpDesc=errorMsg[EC_READONLYACCESSOR];
			break;
		case (DB_E_SCHEMAVIOLATION):
			pwszExpDesc=errorMsg[EC_SCHEMAVIOLATION];
			break;
		case (DB_E_BADROWHANDLE):
			pwszExpDesc=errorMsg[EC_BADROWHANDLE];
			break;
		case (DB_E_OBJECTOPEN):
			pwszExpDesc=errorMsg[EC_OBJECTOPEN];
			break;
		case (DB_E_BADCHAPTER):
			pwszExpDesc=errorMsg[EC_BADCHAPTER];
			break;
		case (DB_E_CANTCONVERTVALUE):
			pwszExpDesc=errorMsg[EC_CANTCONVERTVALUE];
			break;
		case (DB_E_BADBINDINFO):
			pwszExpDesc=errorMsg[EC_BADBINDINFO];
			break;
		case (DB_SEC_E_PERMISSIONDENIED):
			pwszExpDesc=errorMsg[EC_PERMISSIONDENIED];
			break;
		case (DB_E_NOTAREFERENCECOLUMN):
			pwszExpDesc=errorMsg[EC_NOTAREFERENCECOLUMN];
			break;
	//		case (DB_E_LIMITREJECTED):
	//			pwszExpDesc=errorMsg[EC_LIMITREJECTED];
	//			break;
		case (DB_E_NOCOMMAND):
			pwszExpDesc=errorMsg[EC_NOCOMMAND];
			break;
	//		case (DB_E_COSTLIMIT):
	//			pwszExpDesc=errorMsg[EC_COSTLIMIT];
	//			break;
		case (DB_E_BADBOOKMARK):
			pwszExpDesc=errorMsg[EC_BADBOOKMARK];
			break;
		case (DB_E_BADLOCKMODE):
			pwszExpDesc=errorMsg[EC_BADLOCKMODE];
			break;
		case (DB_E_PARAMNOTOPTIONAL):
			pwszExpDesc=errorMsg[EC_PARAMNOTOPTIONAL];
			break;
		case (DB_E_BADCOLUMNID):
			pwszExpDesc=errorMsg[EC_BADCOLUMNID];
			break;
		case (DB_E_BADRATIO):
			pwszExpDesc=errorMsg[EC_BADRATIO];
			break;
	//		case (DB_E_BADVALUES):
	//			pwszExpDesc=errorMsg[EC_BADVALUES];
	//			break;
		case (DB_E_CANTCANCEL):
			pwszExpDesc=errorMsg[EC_CANTCANCEL];
			break;
		case (DB_E_DIALECTNOTSUPPORTED):
			pwszExpDesc=errorMsg[EC_DIALECTNOTSUPPORTED];
			break;
		case (DB_E_DUPLICATEDATASOURCE):
			pwszExpDesc=errorMsg[EC_DUPLICATEDATASOURCE];
			break;
		case (DB_E_CANNOTRESTART):
			pwszExpDesc=errorMsg[EC_CANNOTRESTART];
			break;
		case (DB_E_NOTFOUND):
			pwszExpDesc=errorMsg[EC_NOTFOUND];
			break;
	//		case (DB_E_CANNOTFREE):
	//			pwszExpDesc=errorMsg[EC_CANNOTFREE];
	//			break;
		case (DB_E_NEWLYINSERTED):
			pwszExpDesc=errorMsg[EC_NEWLYINSERTED];
			break;
	//		case (DB_E_GOALREJECTED):
	//			pwszExpDesc=errorMsg[EC_GOALREJECTED];
	//			break;
		case (DB_E_UNSUPPORTEDCONVERSION):
			pwszExpDesc=errorMsg[EC_UNSUPPORTEDCONVERSION];
			break;
		case (DB_E_BADSTARTPOSITION):
			pwszExpDesc=errorMsg[EC_BADSTARTPOSITION];
			break;
	//		case (DB_E_NOQUERY):
	//			pwszExpDesc=errorMsg[EC_NOQUERY];
	//			break;
		case (DB_E_NOTREENTRANT):
			pwszExpDesc=errorMsg[EC_NOTREENTRANT];
			break;
		case (DB_E_ERRORSOCCURRED):
			pwszExpDesc=errorMsg[EC_ERRORSOCCURRED];
			break;
		case (DB_E_NOAGGREGATION):
			pwszExpDesc=errorMsg[EC_NOAGGREGATION];
			break;
		case (DB_E_DELETEDROW):
			pwszExpDesc=errorMsg[EC_DELETEDROW];
			break;
		case (DB_E_CANTFETCHBACKWARDS):
			pwszExpDesc=errorMsg[EC_CANTFETCHBACKWARDS];
			break;
		case (DB_E_ROWSNOTRELEASED):
			pwszExpDesc=errorMsg[EC_ROWSNOTRELEASED];
			break;
		case (DB_E_BADSTORAGEFLAG):
			pwszExpDesc=errorMsg[EC_BADSTORAGEFLAG];
			break;
		case (DB_E_BADCOMPAREOP):
			pwszExpDesc=errorMsg[EC_BADCOMPAREOP];
			break;
		case (DB_E_BADSTATUSVALUE):
			pwszExpDesc=errorMsg[EC_BADSTATUSVALUE];
			break;
		case (DB_E_CANTSCROLLBACKWARDS):
			pwszExpDesc=errorMsg[EC_CANTSCROLLBACKWARDS];
			break;
	//		case (DB_E_BADREGIONHANDLE):
	//			pwszExpDesc=errorMsg[EC_BADREGIONHANDLE];
	//			break;
	//		case (DB_E_NONCONTIGUOUSRANGE):
	//			pwszExpDesc=errorMsg[EC_NONCONTIGUOUSRANGE];
	//			break;
	//		case (DB_E_INVALIDTRANSITION):
	//			pwszExpDesc=errorMsg[EC_INVALIDTRANSITION];
	//			break;
	//		case (DB_E_NOTASUBREGION):
	//			pwszExpDesc=errorMsg[EC_NOTASUBREGION];
	//			break;
		case (DB_E_MULTIPLESTATEMENTS):
			pwszExpDesc=errorMsg[EC_MULTIPLESTATEMENTS];
			break;
		case (DB_E_INTEGRITYVIOLATION):
			pwszExpDesc=errorMsg[EC_INTEGRITYVIOLATION];
			break;
		case (DB_E_BADTYPENAME):
			pwszExpDesc=errorMsg[EC_BADTYPENAME];
			break;
		case (DB_E_ABORTLIMITREACHED):
			pwszExpDesc=errorMsg[EC_ABORTLIMITREACHED];
			break;
	//		case (DB_E_ROWSETINCOMMAND):
	//			pwszExpDesc=errorMsg[EC_ROWSETINCOMMAND];
	//			break;
	//		case (DB_E_CANTTRANSLATE):
	//			pwszExpDesc=errorMsg[EC_CANTTRANSLATE];
	//			break;
		case (DB_E_DUPLICATEINDEXID):
			pwszExpDesc=errorMsg[EC_DUPLICATEINDEXID];
			break;
		case (DB_E_NOINDEX):
			pwszExpDesc=errorMsg[EC_NOINDEX];
			break;
		case (DB_E_INDEXINUSE):
			pwszExpDesc=errorMsg[EC_INDEXINUSE];
			break;
		case (DB_E_NOTABLE):
			pwszExpDesc=errorMsg[EC_NOTABLE];
			break;
		case (DB_E_CONCURRENCYVIOLATION):
			pwszExpDesc=errorMsg[EC_CONCURRENCYVIOLATION];
			break;
		case (DB_E_BADCOPY):
			pwszExpDesc=errorMsg[EC_BADCOPY];
			break;
		case (DB_E_BADPRECISION):
			pwszExpDesc=errorMsg[EC_BADPRECISION];
			break;
		case (DB_E_BADSCALE):
			pwszExpDesc=errorMsg[EC_BADSCALE];
			break;
		case (DB_E_BADTABLEID):
			pwszExpDesc=errorMsg[EC_BADTABLEID];
			break;
		case (DB_E_BADTYPE):
			pwszExpDesc=errorMsg[EC_BADTYPE];
			break;
		case (DB_E_DUPLICATECOLUMNID):
			pwszExpDesc=errorMsg[EC_DUPLICATECOLUMNID];
			break;
		case (DB_E_DUPLICATETABLEID):
			pwszExpDesc=errorMsg[EC_DUPLICATETABLEID];
			break;
		case (DB_E_TABLEINUSE):
			pwszExpDesc=errorMsg[EC_TABLEINUSE];
			break;
		case (DB_E_NOLOCALE):
			pwszExpDesc=errorMsg[EC_NOLOCALE];
			break;
		case (DB_E_BADRECORDNUM):
			pwszExpDesc=errorMsg[EC_BADRECORDNUM];
			break;
		case (DB_E_BOOKMARKSKIPPED):
			pwszExpDesc=errorMsg[EC_BOOKMARKSKIPPED];
			break;
		case (DB_E_BADPROPERTYVALUE):
			pwszExpDesc=errorMsg[EC_BADPROPERTYVALUE];
			break;
		case (DB_E_INVALID):
			pwszExpDesc=errorMsg[EC_INVALID];
			break;
		case (DB_E_BADACCESSORFLAGS):
			pwszExpDesc=errorMsg[EC_BADACCESSORFLAGS];
			break;
		case (DB_E_BADSTORAGEFLAGS):
			pwszExpDesc=errorMsg[EC_BADSTORAGEFLAGS];
			break;
		case (DB_E_BYREFACCESSORNOTSUPPORTED):
			pwszExpDesc=errorMsg[EC_BYREFACCESSORNOTSUPPORTED];
			break;
		case (DB_E_NULLACCESSORNOTSUPPORTED):
			pwszExpDesc=errorMsg[EC_NULLACCESSORNOTSUPPORTED];
			break;
		case (DB_E_NOTPREPARED):
			pwszExpDesc=errorMsg[EC_NOTPREPARED];
			break;
		case (DB_E_BADACCESSORTYPE):
			pwszExpDesc=errorMsg[EC_BADACCESSORTYPE];
			break;
		case (DB_E_WRITEONLYACCESSOR):
			pwszExpDesc=errorMsg[EC_WRITEONLYACCESSOR];
			break;
		case (DB_SEC_E_AUTH_FAILED):
			pwszExpDesc=errorMsg[EC_AUTH_FAILED];
			break;
		case (DB_E_CANCELED):
			pwszExpDesc=errorMsg[EC_CANCELED];
			break;
	//		case (DB_E_CHAPTERNOTRELEASED):
	//			pwszExpDesc=errorMsg[EC_CHAPTERNOTRELEASED];
	//			break;
		case (DB_E_BADSOURCEHANDLE):
			pwszExpDesc=errorMsg[EC_BADSOURCEHANDLE];
			break;
		case (DB_E_PARAMUNAVAILABLE):
			pwszExpDesc=errorMsg[EC_PARAMUNAVAILABLE];
			break;
		case (DB_E_ALREADYINITIALIZED):
			pwszExpDesc=errorMsg[EC_ALREADYINITIALIZED];
			break;
		case (DB_E_NOTSUPPORTED):
			pwszExpDesc=errorMsg[EC_NOTSUPPORTED];
			break;
		case (DB_E_MAXPENDCHANGESEXCEEDED):
			pwszExpDesc=errorMsg[EC_MAXPENDCHANGESEXCEEDED];
			break;
		case (DB_E_BADORDINAL):
			pwszExpDesc=errorMsg[EC_BADORDINAL];
			break;
		case (DB_E_PENDINGCHANGES):
			pwszExpDesc=errorMsg[EC_PENDINGCHANGES];
			break;
		case (DB_E_DATAOVERFLOW):
			pwszExpDesc=errorMsg[EC_DATAOVERFLOW];
			break;
		case (DB_E_BADHRESULT):
			pwszExpDesc=errorMsg[EC_BADHRESULT];
			break;
		case (DB_E_BADLOOKUPID):
			pwszExpDesc=errorMsg[EC_BADLOOKUPID];
			break;
		case (DB_E_BADDYNAMICERRORID):
			pwszExpDesc=errorMsg[EC_BADDYNAMICERRORID];
			break;
		case (DB_E_PENDINGINSERT):
			pwszExpDesc=errorMsg[EC_PENDINGINSERT];
			break;
		case (DB_E_BADCONVERTFLAG):
			pwszExpDesc=errorMsg[EC_BADCONVERTFLAG];
			break;
		
		// the error msg of this error may contain a dynamically assigned command obj ID
		case (E_FAIL):
			pwszExpDesc=errorMsg[EC_FAIL];
			break;
		case (DB_E_ERRORSINCOMMAND):
			pwszExpDesc=errorMsg[EC_ERRORSINCOMMAND];	
			break;
		
		// Not able to check the error msg in transaction right now,
		// so better print it out and check it manually
		case (XACT_E_CONNECTION_DENIED):
	//		case (XACT_E_CONNECTION_REQUEST_DENIED):
		case (XACT_E_CONNECTION_DOWN):
		case (XACT_E_ISOLATIONLEVEL):
		case (XACT_E_INDOUBT):
		case (XACT_E_LOGFULL):
		case (XACT_E_ABORTED):
		case (XACT_E_ALREADYINPROGRESS):
		case (XACT_E_NOISORETAIN):
		case (XACT_E_CANTRETAIN):
		case (XACT_E_COMMITFAILED):
		case (XACT_E_NOTSUPPORTED):
		case (XACT_E_NOTRANSACTION):
		case (XACT_E_NOTIMEOUT):
		case (XACT_E_NOENLIST):
		case (XACT_E_XTIONEXISTS):
		case (XACT_E_TMNOTAVAILABLE):
		// For all successful returned code but with an error object,
		// the error description should be any informational message
		// has to check manuelly
		default:
			fResult = FALSE;
	};

	if (pwszExpDesc)
		fResult = CompareStrings(bstr, pwszExpDesc,m_fPrint);	

	//Display the error if it didn't match... (if we haven't already)
	if(fResult==FALSE && m_fPrint==FALSE)
	{
		odtLog << L"Warning: Description does not match expected description" << wszNewLine;
		odtLog << L"Description = \"" << bstr << L"\"" << wszNewLine;
		if (pwszExpDesc)
			odtLog << L"Expected Desc = \"" << pwszExpDesc << L"\"" << wszNewLine;
		else
			odtLog << L"No expected description found for this error.\n";
	}
	
	return fResult;
}

//--------------------------------------------------------------------
// setErrorMsg
//
// Fill in the standard error message array. 
//
// @mfunc
//
//--------------------------------------------------------------------
void CExtError::setErrorMsg(LPOLESTR *errorMsg)
{
	errorMsg[EC_UNEXPECTED] = L"Catastrophic failure";
	errorMsg[EC_FAIL] = L"Login Failed";
	errorMsg[EC_NOINTERFACE] = L"No such interface supported";
	errorMsg[EC_INVALIDARG] = L"The parameter is incorrect.";
	errorMsg[EC_BADACCESSORHANDLE] = L"Accessor is invalid.";
	errorMsg[EC_ROWLIMITEXCEEDED] = L"Row could not be inserted into the rowset without exceeding provider's maximum number of active rows.";
	errorMsg[EC_READONLYACCESSOR] = L"Accessor is read-only. Operation failed.";
	errorMsg[EC_SCHEMAVIOLATION] = L"Given values violate the database schema.";
	errorMsg[EC_BADROWHANDLE] = L"Row handle is invalid.";
	errorMsg[EC_OBJECTOPEN] = L"Object was open.";
	errorMsg[EC_BADCHAPTER] = L"Chapter is invalid.";
	errorMsg[EC_CANTCONVERTVALUE] = L"Data or literal value could not be converted to the type of the column in the data source, and the provider was unable to determine which columns could not be converted.  Data overflow or sign mismatch was not the cause.";
	errorMsg[EC_BADBINDINFO] = L"Binding information is invalid.";
	errorMsg[EC_PERMISSIONDENIED] = L"Permission denied.";
	errorMsg[EC_NOTAREFERENCECOLUMN] = L"Specified column does not contain bookmarks or chapters.";
	errorMsg[EC_LIMITREJECTED] = L"Cost limits were rejected.";
	errorMsg[EC_NOCOMMAND] = L"Command text was not set for the command object.";
	errorMsg[EC_COSTLIMIT] = L"Query plan within the cost limit cannot be found.";
	errorMsg[EC_BADBOOKMARK] = L"Bookmark is invalid.";
	errorMsg[EC_BADLOCKMODE] = L"Lock mode is invalid.";
	errorMsg[EC_PARAMNOTOPTIONAL] = L"No value given for one or more required parameters.";
	errorMsg[EC_BADCOLUMNID] = L"Column ID is invalid.";
	errorMsg[EC_BADRATIO] = L"Numerator was greater than denominator. Values must express ratio between zero and 1.";
	errorMsg[EC_BADVALUES] = L"Value is invalid.";
	errorMsg[EC_ERRORSINCOMMAND] = L"One or more errors occurred during processing of command.";
	errorMsg[EC_CANTCANCEL] = L"Command cannot be canceled.";
	errorMsg[EC_DIALECTNOTSUPPORTED] = L"Command dialect is not supported by this provider.";
	errorMsg[EC_DUPLICATEDATASOURCE] = L"Data source object could not be created because the named data source already exists.";
	errorMsg[EC_CANNOTRESTART] = L"The rowset was built over a live data feed and cannot be restarted.";
	errorMsg[EC_NOTFOUND] = L"No key matching the described characteristics could be found within the current range.";
	errorMsg[EC_NEWLYINSERTED] = L"Identity cannot be determined for newly inserted rows.";
	errorMsg[EC_CANNOTFREE] = L"Provider has ownership of this tree.";
	errorMsg[EC_GOALREJECTED] = L"Goal was rejected because no nonzero weights were specified for any goals supported. Current goal was not changed.";
	errorMsg[EC_UNSUPPORTEDCONVERSION] = L"Requested conversion is not supported.";
	errorMsg[EC_BADSTARTPOSITION] = L"Goal was rejected because no nonzero weights were specified for any goals supported. Current goal was not changed.";
	errorMsg[EC_NOQUERY] = L"Information was requested for a query, and the query was not set.";
	errorMsg[EC_NOTREENTRANT] = L"Consumer's event handler called a non-reentrant method in the provider.";
	errorMsg[EC_ERRORSOCCURRED] = L"Multiple-step OLE DB operation generated errors. Check each OLE DB status value, if available. No work was done.";
	errorMsg[EC_NOAGGREGATION] = L"Non-NULL controlling IUnknown was specified, and either the requested interface was not IUnknown, or the provider does not support COM aggregation.";
	errorMsg[EC_DELETEDROW] = L"Row handle referred to a deleted row or a row marked for deletion.";
	errorMsg[EC_CANTFETCHBACKWARDS] = L"The rowset does not support fetching backwards.";
	errorMsg[EC_ROWSNOTRELEASED] = L"Row handles must all be released before new ones can be obtained.";
	errorMsg[EC_BADSTORAGEFLAG] = L"One or more storage flags are not supported.";
	errorMsg[EC_BADCOMPAREOP] = L"Comparison operator is invalid.";
	errorMsg[EC_BADSTATUSVALUE] = L"The specified status flag was neither DBCOLUMNSTATUS_OK nor DBCOLUMNSTATUS_ISNULL.";
	errorMsg[EC_CANTSCROLLBACKWARDS] = L"Rowset does not support scrolling backward.";
	errorMsg[EC_BADREGIONHANDLE] = L"Region handle is invalid.";
	errorMsg[EC_NONCONTIGUOUSRANGE] = L"Set of rows is not contiguous to, or does not overlap, the rows in the watch region.";
	errorMsg[EC_INVALIDTRANSITION] = L"A transition from ALL* to MOVE* or EXTEND* was specified.";
	errorMsg[EC_NOTASUBREGION] = L"The specified region is not a proper subregion of the region identified by the given watch region handle.";
	errorMsg[EC_MULTIPLESTATEMENTS] = L"Multiple-statement commands are not supported by this provider.";
	errorMsg[EC_INTEGRITYVIOLATION] = L"A specified value violated the integrity constraints for a column or table.";
	errorMsg[EC_BADTYPENAME] = L"Type name is invalid.";
	errorMsg[EC_ABORTLIMITREACHED] = L"Execution stopped because a resource limit was reached. No results were returned.";
	errorMsg[EC_ROWSETINCOMMAND] = L"Command object whose command tree contains a rowset or rowsets cannot be cloned.";
	errorMsg[EC_CANTTRANSLATE] = L"Current tree cannot be represented as text.";
	errorMsg[EC_DUPLICATEINDEXID] = L"The specified index already exists.";
	errorMsg[EC_NOINDEX] = L"The specified index does not exist.";
	errorMsg[EC_INDEXINUSE] = L"Index is in use.";
	errorMsg[EC_NOTABLE] = L"The specified table does not exist.";
	errorMsg[EC_CONCURRENCYVIOLATION] = L"The rowset was using optimistic concurrency and the value of a column has been changed since it was last read.";
	errorMsg[EC_BADCOPY] = L"Errors were detected during the copy.";
	errorMsg[EC_BADPRECISION] = L"Precision is invalid.";
	errorMsg[EC_BADSCALE] = L"Scale is invalid.";
	errorMsg[EC_BADTABLEID] = L"Table ID is invalid.";
	errorMsg[EC_BADTYPE] = L"Type is invalid.";
	errorMsg[EC_DUPLICATECOLUMNID] = L"Column ID already exists or occurred more than once in the array of columns.";
	errorMsg[EC_DUPLICATETABLEID] = L"The specified table already exists.";
	errorMsg[EC_TABLEINUSE] = L"Table is in use.";
	errorMsg[EC_NOLOCALE] = L"The specified locale ID was not supported.";
	errorMsg[EC_BADRECORDNUM] = L"Record number is invalid.";
	errorMsg[EC_BOOKMARKSKIPPED] = L"Although the bookmark was validly formed, no row could be found to match it.";
	errorMsg[EC_BADPROPERTYVALUE] = L"Property value is invalid.";
	errorMsg[EC_INVALID] = L"The rowset was not chaptered.";
	errorMsg[EC_BADACCESSORFLAGS] = L"One or more accessor flags were invalid.";
	errorMsg[EC_BADSTORAGEFLAGS] = L"One or more storage flags are invalid.";
	errorMsg[EC_BYREFACCESSORNOTSUPPORTED] = L"Reference accessors are not supported by this provider.";
	errorMsg[EC_NULLACCESSORNOTSUPPORTED] = L"Null accessors are not supported by this provider.";
	errorMsg[EC_NOTPREPARED] = L"The command was not prepared.";
	errorMsg[EC_BADACCESSORTYPE] = L"The specified accessor was not a parameter accessor.";
	errorMsg[EC_WRITEONLYACCESSOR] = L"The given accessor was write-only.";
	errorMsg[EC_AUTH_FAILED] = L"Authentication failed.";
	errorMsg[EC_CANCELED] = L"Operation was canceled.";
	errorMsg[EC_CHAPTERNOTRELEASED] = L"The rowset was single-chaptered and the chapter was not released.";
	errorMsg[EC_BADSOURCEHANDLE] = L"Source handle is invalid.";
	errorMsg[EC_PARAMUNAVAILABLE] = L"Provider cannot derive parameter information and SetParameterInfo has not been called.";
	errorMsg[EC_ALREADYINITIALIZED] = L"The data source object is already initialized.";
	errorMsg[EC_NOTSUPPORTED] = L"Method is not supported by this provider.";
	errorMsg[EC_MAXPENDCHANGESEXCEEDED] = L"Number of rows with pending changes exceeded the limit.";
	errorMsg[EC_BADORDINAL] = L"The specified column did not exist.";
	errorMsg[EC_PENDINGCHANGES] = L"There are pending changes on a row with a reference count of zero.";
	errorMsg[EC_DATAOVERFLOW] = L"Literal value in the command exceeded the range of the type of the associated column.";
	errorMsg[EC_BADHRESULT] = L"HRESULT is invalid.";
	errorMsg[EC_BADLOOKUPID] = L"Lookup ID is invalid.";
	errorMsg[EC_BADDYNAMICERRORID] = L"DynamicError ID is invalid.";
	errorMsg[EC_PENDINGINSERT] = L"Most recent data for a newly inserted row could not be retrieved because the insert is pending.";
	errorMsg[EC_BADCONVERTFLAG] = L"Conversion flag is invalid.";
}

//--------------------------------------------------------------------
// CauseError
//
// Creates an error on the current thread which causes an 
// error object to be created.  This function should be used before
// a successful call to a method on which we want to validate extended
// error information.  Generating an error object will allow us
// to test in ValidateExtended that the successful call gets rid
// of any existing error objects.
// Returns TRUE if the error was generated without problems, else FALSE.
//
// @mfunc
//
//--------------------------------------------------------------------
BOOL CExtError::CauseError()
{
	IDBProperties*		pIDBProperties = NULL;
	HRESULT				hr = S_OK;
	BOOL				fResults = FALSE;	

	// Get our initial connection to the provider	
	if(!CHECK(hr = GetModInfo()->CreateProvider(NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties), S_OK))
		goto CLEANUP;
	
	m_PreErrIID = IID_IDBProperties;

	//Cause an error to occur (E_INVALIDARG)
	if(!CHECK(hr = m_PreErrHr = pIDBProperties->SetProperties(1, NULL),E_INVALIDARG))
		goto CLEANUP;
	
	fResults = TRUE;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	return fResults;
}

