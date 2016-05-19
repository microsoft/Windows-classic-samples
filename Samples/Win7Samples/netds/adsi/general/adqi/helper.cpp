#include "stdafx.h"
#include "helper.h"



extern ADSIIF  adsiIfs[];

HRESULT  VariantToStringList(  VARIANT& refvar, CStringList& refstringlist)
{
    HRESULT hr = S_OK;
    long start, end;
	USES_CONVERSION;

	if ( !(V_VT(&refvar) &  VT_ARRAY)  ) // Single Value
    {
		if ( V_VT(&refvar) == VT_DISPATCH )
		{
			// Find out which interfaces it supports
			IUnknown *pQI;
			int xx=0;
			CString s;
			while( !IsEqualIID( *adsiIfs[xx].pIID, IID_NULL ) )
			{
		           hr = V_DISPATCH(&refvar)->QueryInterface( *adsiIfs[xx].pIID, (void**) &pQI );
		           if ( SUCCEEDED(hr) )
				   {
					    s = ARROW_SYMBOL;
						s += adsiIfs[xx].szIf;
			            refstringlist.AddHead( s );
			            pQI->Release();
				   }
		           xx++;
			}
			return S_OK;


		}
		else if ( V_VT(&refvar) != VT_BSTR )
		{
			
			hr = VariantChangeType( &refvar, &refvar,0, VT_BSTR );

			if( FAILED(hr) )
			{
				return hr;
			}

		}
		

		refstringlist.AddHead( OLE2T(V_BSTR(&refvar)) );
        return hr;
    }
	

    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

    //
    // Figure out the dimensions of the array.
    //

    hr = SafeArrayGetLBound( saAttributes, 1, &start );
        if( FAILED(hr) )
                return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
        if( FAILED(hr) )
                return hr;

    VARIANT SingleResult;
    VariantInit( &SingleResult );

    //
    // Process the array elements.
    //
    long idx;
    for ( idx = start; idx <= end; idx++   ) 
	{

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
		{
            return hr;
		}

		if ( V_VT(&SingleResult) != VT_BSTR )
		{

			
			if ( V_VT(&SingleResult) == VT_NULL )
			{
				V_VT(&SingleResult ) = VT_BSTR;
				V_BSTR(&SingleResult ) = SysAllocString(L"0");
			}
			else
			{
				hr = VariantChangeType( &SingleResult, &SingleResult,0, VT_BSTR );

				if( FAILED(hr) )
				{
					return hr;
				}
			}
		}



         refstringlist.AddHead( OLE2T(V_BSTR(&SingleResult)) );
        VariantClear( &SingleResult );
    }

    return S_OK;
} // VariantToStringList()



HRESULT OctetVariantToString(  VARIANT& refvar, CString &sResult)
{
    HRESULT hr = S_OK;
	CString s;
    long start, end;
	USES_CONVERSION;

	if ( !(V_VT(&refvar) &  VT_ARRAY)  ) // Single Value
    {
		return E_UNEXPECTED;
    }
	

    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

    //
    // Figure out the dimensions of the array.
    //

    hr = SafeArrayGetLBound( saAttributes, 1, &start );
    if( FAILED(hr) )
           return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
    if( FAILED(hr) )
            return hr;

    VARIANT SingleResult;
    VariantInit( &SingleResult );

    //
    // Process the array elements.
    //
    sResult.Empty();
    long idx;
    for ( idx = start; idx <= end; idx++   ) 
	{

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
		{
            return hr;
		}

		if ( V_VT(&SingleResult) == VT_I1 )
		{
			s.Format("%02X", V_I1(&SingleResult));
			sResult += s;
			VariantClear( &SingleResult );
		}
    }

    return S_OK;
} // VariantToStringList()





HRESULT StringListToVariant( VARIANT& refvar, const CStringList& refstringlist)
{
    HRESULT hr = S_OK;
    int cCount = refstringlist.GetCount();

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cCount;

    SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
    if (NULL == psa)
        return E_OUTOFMEMORY;

    VariantClear( &refvar );
    V_VT(&refvar) = VT_VARIANT|VT_ARRAY;
    V_ARRAY(&refvar) = psa;

    VARIANT SingleResult;
    VariantInit( &SingleResult );
    V_VT(&SingleResult) = VT_BSTR;
    POSITION pos = refstringlist.GetHeadPosition();
    long i;
    for (i = 0; i < cCount, pos != NULL; i++)
    {
        V_BSTR(&SingleResult) = T2BSTR((LPCTSTR)refstringlist.GetNext(pos));
        hr = SafeArrayPutElement(psa, &i, &SingleResult);
        if( FAILED(hr) )
            return hr;
    }
    if (i != cCount || pos != NULL)
        return E_UNEXPECTED;

    return hr;
} // StringListToVariant()


void StringToStringList( CString s, CStringList &sList )
{
	int idx;
	CString sAttr;

	//////////////////////////////////////////
	// Parse and put them in the string list
	//////////////////////////////////////////
	idx = s.Find(_T(","));
	while( idx != -1 )
	{
		   sAttr = s.Mid(0, idx );
		   sAttr.TrimLeft(); sAttr.TrimRight();
		   if ( !sAttr.IsEmpty() )
		   {
			 sList.AddTail( sAttr );
		   }
		   s = s.Mid( idx + 1 );
		   idx = s.Find(_T(","));
	}
	s.TrimLeft(); s.TrimRight();
	if ( !s.IsEmpty() )	  
	{
		sList.AddTail( s );
	}


}


void StringListToString( CStringList &sList, CString &s )
{
	POSITION pos;
	int idx=0;
	CString sTemp;


	pos = sList.GetHeadPosition();
	s.Empty();
	while( pos != NULL )
	{
		if ( idx )
		{
			s += _T(" #");
			s += sList.GetAt(pos);
		}
		else
		{
			s += sList.GetAt(pos);
		}
		
		sList.GetNext(pos);
		idx++;

	}

}



void ADsToStringList(ADSVALUE *pValues, DWORD dwNumber, CStringList &sList )
{
	CString sTemp;

	sList.RemoveAll();


	USES_CONVERSION;

	for (DWORD x=0; x < dwNumber; x++) 
	{
		    if ( pValues->dwType == ADSTYPE_INVALID )
			{
				continue;
			}

			
			sTemp.Empty();

			switch( pValues->dwType ) 
			{
			case ADSTYPE_DN_STRING         :
				sTemp.Format(
				"%s",
                OLE2T(pValues->DNString)
                );
				break;

			case ADSTYPE_CASE_EXACT_STRING :
				sTemp.Format(
                "%s",
                OLE2T(pValues->CaseExactString)
                );
				break;

			case ADSTYPE_CASE_IGNORE_STRING:
			
				sTemp.Format(
                "%s",
                OLE2T(pValues->CaseIgnoreString)
                );
				
            break;

			case ADSTYPE_PRINTABLE_STRING  :
				sTemp.Format(
                "%s",
                pValues->PrintableString
                );
				break;

			case ADSTYPE_NUMERIC_STRING    :
				sTemp.Format(
                "%s",
                pValues->NumericString
                );
				break;
    
			case ADSTYPE_OBJECT_CLASS    :
				sTemp.Format(
                "%s",
                pValues->ClassName
                );
				break;
    
			case ADSTYPE_BOOLEAN           :
				sTemp.Format(
                "%s",
                (DWORD) pValues->Boolean ? 
                "TRUE" : "FALSE"
                );
				break;
    
			case ADSTYPE_INTEGER           :
				sTemp.Format(
                "%d",
                (DWORD) pValues->Integer 
                );
				break;
    
			case ADSTYPE_OCTET_STRING      :
				{
					CString sOctet;
					CString sPrint;
					CString sChar;

					
			
					BYTE  b;
					DWORD idx; 
					for ( idx=0; idx<pValues->OctetString.dwLength; idx++) 
					{
						b = ((BYTE *)pValues->OctetString.lpValue)[idx];
						sOctet.Format(
					   "%02X ",
					    b
						);
						if ( idx != 0 && ((idx % 16)  == 0) )
						{
							sTemp += "\t";
							sTemp += sPrint;
							sList.AddTail( sTemp );
							sTemp.Empty();
							sPrint.Empty();
						}

						if ( !isprint(b) )
						{
							sPrint += _T(".");
						}
						else
						{
							sChar.Format("%c", b );
							sPrint += sChar;
							
						}

					
						sTemp += sOctet;

					}
					
					if ( !sPrint.IsEmpty() )
					{
						DWORD dwMod;
					   // Figure out the remaining 16 bytes aligment
					   // for display
					   dwMod = 16 - (idx % 16);
					   for( idx=0;idx < dwMod; idx++ )
					   {
						  sTemp += _T("Xx ");
					   }
					   sTemp += _T("\t");
					   sTemp += sPrint;					   
					}
					
				}
				break;
    
			case ADSTYPE_LARGE_INTEGER     :
				if 	( pValues->LargeInteger.HighPart )
				{
					sTemp.Format("%ld%ld", pValues->LargeInteger.HighPart, pValues->LargeInteger.LowPart );
				}
				else
				{
					sTemp.Format("%ld", pValues->LargeInteger.LowPart );
				}
				
				break;
    
			case ADSTYPE_UTC_TIME          :
				sTemp.Format(
					"%02d/%02d/%04d %02d:%02d:%02d", pValues->UTCTime.wMonth, pValues->UTCTime.wDay, pValues->UTCTime.wYear,
					 pValues->UTCTime.wHour, pValues->UTCTime.wMinute, pValues->UTCTime.wSecond 
					);
				break;

			case ADSTYPE_PROV_SPECIFIC     :
				sTemp.Format(
					"(provider specific value) "
					);
				break;


			case ADSTYPE_NT_SECURITY_DESCRIPTOR:
				{
					sTemp.Format(_T("%sIADsSecurityDescriptor"), ARROW_SYMBOL );
    			}
				break;
    
			}
			
			pValues++;

			
			sList.AddTail( sTemp );
		
	}


 }



void PopulateListBoxFromStringList( CListBox &list, CStringList &sList )
{
	POSITION pos;



	pos = sList.GetHeadPosition();
	while( pos != NULL )
	{
		list.AddString( sList.GetAt(pos));
		sList.GetNext(pos);
	}

}


void PopulateComboBoxFromStringList( CComboBox &list, CStringList &sList )
{
	POSITION pos;



	pos = sList.GetHeadPosition();
	while( pos != NULL )
	{
		list.AddString( sList.GetAt(pos));
		sList.GetNext(pos);
	}

}


HRESULT ReportError( HRESULT hr )
{
	AfxMessageBox( GetErrorMessage(hr) );
	return hr;
}



HRESULT  VariantToPtrList(  VARIANT& refvar, CPtrList & refptrlist)
{
    HRESULT hr = S_OK;
    long start, end;

	if ( !(V_VT(&refvar) &  VT_ARRAY)  )
    {
                
		if ( V_VT(&refvar) != VT_DISPATCH )
		{
			return E_FAIL;
		}
		else
		{
		   refptrlist.AddHead( V_DISPATCH(&refvar) );

		}

    }

    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

    //
    // Figure out the dimensions of the array.
    //

    hr = SafeArrayGetLBound( saAttributes, 1, &start );
        if( FAILED(hr) )
                return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
        if( FAILED(hr) )
                return hr;

    VARIANT SingleResult;
    VariantInit( &SingleResult );

    //
    // Process the array elements.
    //
    long idx;
    for ( idx = start; idx <= end; idx++   ) 
	{

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
		{
            return hr;
		}

		if ( V_VT(&SingleResult) != VT_DISPATCH )
		{
			return E_UNEXPECTED;
		}


        refptrlist.AddHead( V_DISPATCH(&SingleResult) );
    }

    return S_OK;
} // VariantToStringList()



HRESULT PropertyValueToString( LONG lADsType, IADsPropertyValue *pValue, CString &sValue )
{
	BSTR bstr = NULL;
	LONG l;
	VARIANT var;
	HRESULT hr = S_OK;
	sValue.Empty();

	VariantInit(&var);
	switch( lADsType )
	{
	case ADSTYPE_DN_STRING:
		 hr = pValue->get_DNString( &bstr );
		 sValue = bstr;
		break;

	case ADSTYPE_CASE_EXACT_STRING:
		hr = pValue->get_CaseExactString( &bstr );
		sValue = bstr;
		break;

	case ADSTYPE_CASE_IGNORE_STRING:
		 hr = pValue->get_CaseIgnoreString( &bstr );
		 sValue = bstr;
		break;

	case ADSTYPE_PRINTABLE_STRING:
		 hr = pValue->get_PrintableString( &bstr );
		 sValue = bstr;
		break;

	case ADSTYPE_NUMERIC_STRING:
		hr = pValue->get_NumericString( &bstr );
		sValue = bstr;
		break;

	case ADSTYPE_BOOLEAN:
		 hr = pValue->get_Boolean( &l );
		 sValue = l ? _T("TRUE") : _T("FALSE");
		break;

	case ADSTYPE_INTEGER:
 		 hr = pValue->get_Integer( &l );
		 sValue.Format("%ld", l );
		break;


	case ADSTYPE_OCTET_STRING: 
		 hr = pValue->get_OctetString( &var );
		 if ( SUCCEEDED(hr) )
		 {
			CString s;
			hr =  OctetVariantToString( var, s);
			if ( SUCCEEDED(hr) )
			{
				sValue = s;
			}
		 }

		break;


	case ADSTYPE_UTC_TIME:
		{
			DATE date;
			hr = pValue->get_UTCTime(&date);


		}
		break;

	case ADSTYPE_LARGE_INTEGER:
		sValue.Format(_T("%sIADsLargeInteger"), ARROW_SYMBOL );
		break;

	case ADSTYPE_PROV_SPECIFIC:
		sValue = _T("Provider Specific");
		break;

	case ADSTYPE_CASEIGNORE_LIST:
		hr = pValue->get_CaseIgnoreString(&bstr);
		sValue = bstr;
		break;

	case ADSTYPE_OCTET_LIST:
		sValue.Format(_T("%sIADsOctetString"), ARROW_SYMBOL );
		break;

	case ADSTYPE_PATH:
		sValue.Format(_T("%sIADsPath"), ARROW_SYMBOL );
		break;

	case ADSTYPE_POSTALADDRESS:
		sValue.Format(_T("%sIADsPostalAddress"), ARROW_SYMBOL );
		break;

	case ADSTYPE_TIMESTAMP:
		sValue.Format(_T("%sIADsTimestamp"), ARROW_SYMBOL );
		break;

	case ADSTYPE_BACKLINK:
		sValue.Format(_T("%sIADsBackLink"), ARROW_SYMBOL );
		break;

	case ADSTYPE_TYPEDNAME:
		sValue.Format(_T("%sIADsTypedName"), ARROW_SYMBOL );
		break;

	case ADSTYPE_HOLD:
		sValue.Format(_T("%sIADsHold"), ARROW_SYMBOL );
		break;

	case ADSTYPE_NETADDRESS:
		sValue.Format(_T("%sIADsNetAddress"), ARROW_SYMBOL );
		break;

	case ADSTYPE_REPLICAPOINTER:
		sValue.Format(_T("%sIADsReplicaPointer"), ARROW_SYMBOL );
		break;

	case ADSTYPE_FAXNUMBER:
		sValue.Format(_T("%sIADsFaxNumber"), ARROW_SYMBOL );
		break;

	case ADSTYPE_EMAIL:
		sValue.Format(_T("%sIADsEmail"), ARROW_SYMBOL );
		break;

	case ADSTYPE_NT_SECURITY_DESCRIPTOR:
		sValue.Format(_T("%sIADsSecurityDescriptor"), ARROW_SYMBOL );
		break;

	case ADSTYPE_UNKNOWN:
		 sValue = _T("Unknown");
		 break;

	}

	if ( bstr )
	{
		SysFreeString( bstr );
	}

	VariantClear(&var);
	pValue->Release();
	return hr;
}





CString GetErrorMessage( HRESULT hr )
{
	BOOL bRet;
	CString s;
	LPTSTR lpBuffer=NULL;

	if ( SUCCEEDED(hr) )
	{
		return _T("Success");
	}

	
	if ( hr & 0x00005000) // standard ADSI Errors 
	{
		s = GetADSIError(hr);
	}
	else if ( HRESULT_FACILITY(hr)==FACILITY_WIN32 )
	{


		/////////////////////////////////////////////
		//
		// Retrieve the Win32 Error message
		//
		/////////////////////////////////////////////////

		bRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							 NULL,  hr,
							 MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
							 (LPTSTR) &lpBuffer, 0, NULL);

		if ( !bRet )
		{
			s.Format(_T("Error %X"), hr );
		}

		if ( lpBuffer )
		{
			s = lpBuffer;
			LocalFree( lpBuffer );
		}
	}
	else // Non Win32 Error
	{
		s.Format("%X", hr );
	}

	//////////////////////////////////////////////////////////////////
	//
	// Extended error message may be occured. 
	//
	// IADs, IADsContainer, IDirectoryObject or IDirectorySearch may return
	// this extended error message
	//
	////////////////////////////////////////////////////////////////////

	if ( (hr & 0x00005000) ||  //adsi
		 (hr & 0x00007000) )   // and win32
	{
		WCHAR szBuffer[MAX_PATH];
		WCHAR szName[MAX_PATH];
		DWORD dwError;
	

		hr = ADsGetLastError( &dwError, szBuffer, (sizeof(szBuffer)/sizeof(WCHAR))-1,
			                  szName, (sizeof(szName)/sizeof(WCHAR))-1 );
	
	
		if ( SUCCEEDED(hr) && dwError != ERROR_INVALID_DATA  && wcslen(szBuffer))
		{
			USES_CONVERSION;
			s += _T("  -- Extended Error:");
			s += OLE2T(szName);
			s += _T(" : ");
			s += OLE2T( szBuffer );
		}
	}


	return s;
}

ADSERRMSG adsErr[] = 
{
	ADDADSERROR(E_ADS_BAD_PATHNAME),
	ADDADSERROR(E_ADS_INVALID_DOMAIN_OBJECT),
	ADDADSERROR(E_ADS_INVALID_USER_OBJECT),
	ADDADSERROR(E_ADS_INVALID_COMPUTER_OBJECT),
	ADDADSERROR(E_ADS_UNKNOWN_OBJECT),
	ADDADSERROR(E_ADS_PROPERTY_NOT_SET),
	ADDADSERROR(E_ADS_PROPERTY_NOT_SUPPORTED),
	ADDADSERROR(E_ADS_PROPERTY_INVALID),
	ADDADSERROR(E_ADS_BAD_PARAMETER),
	ADDADSERROR(E_ADS_OBJECT_UNBOUND),
	ADDADSERROR(E_ADS_PROPERTY_NOT_MODIFIED),
	ADDADSERROR(E_ADS_PROPERTY_MODIFIED),
	ADDADSERROR(E_ADS_CANT_CONVERT_DATATYPE),
	ADDADSERROR(E_ADS_PROPERTY_NOT_FOUND),
	ADDADSERROR(E_ADS_OBJECT_EXISTS),
	ADDADSERROR(E_ADS_SCHEMA_VIOLATION),
	ADDADSERROR(E_ADS_COLUMN_NOT_SET),
	ADDADSERROR(E_ADS_INVALID_FILTER),
	ADDADSERROR(0),
};


/////////////////////////////////////////////
//
// Error message specific to ADSI 
//
////////////////////////////////////////////
CString GetADSIError( HRESULT hr )
{
	CString s;


	if ( hr & 0x00005000 )
	{
		int idx=0;
		while (adsErr[idx].hr != 0 )
		{
			if ( adsErr[idx].hr == hr )
			{
				return adsErr[idx].pszError;
			}
			idx++;
		}
	}

	return _T("");
  

}






#if _ATL_VER <= 0x0200 

LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	_ASSERTE(lpa != NULL);
	_ASSERTE(lpw != NULL);
	// verify that no illegal character present
	// since lpw was allocated based on the size of lpa
	// don't worry about the number of chars
	lpw[0] = '\0';
	MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	return lpw;
}

LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	_ASSERTE(lpw != NULL);
	_ASSERTE(lpa != NULL);
	// verify that no illegal character present
	// since lpa was allocated based on the size of lpw
	// don't worry about the number of chars
	lpa[0] = '\0';
	WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

#endif 
  