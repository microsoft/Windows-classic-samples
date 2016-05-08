// attributes.cpp : Defines the entry point for the console application.
//
#include <objbase.h>
#include <wchar.h>
#include <activeds.h>
//Make sure you define UNICODE
//Need to define version 5 for Windows 2000
#define _WIN32_WINNT 0x0500

int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy=-1);

///////////////////////////////////////////////////////////
// PLEASE READ: You must run on Windows 2000 machine to run this sample
////////////////////////////////////////////////////////////
#include <sddl.h>

HRESULT FindAttributesOrClasses(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
		 LPOLESTR szFilter, //Filter for finding specific attributes.
                            //NULL returns all attributeSchema objects.
         LPOLESTR *pszPropertiesToReturn, //Properties to return for attributeSchema objects found
					                    //NULL returns all set properties.
		 BOOL bIsAttributeQuery, //TRUE queries for attributes;FALSE for classes
		 BOOL bIsVerbose //TRUE means all properties for the found objects are displayed.
		                 //FALSE means only the ldapDisplayName with cn in parentheses:
						 //example: l (Locality-Name)
							);




void wmain( int argc, wchar_t *argv[ ])
{

BOOL bIsAttributeQuery = TRUE;
BOOL bReturnVerbose = FALSE;
LPOLESTR szType = L"attribute";
if (1==argc||(_wcsicmp(argv[1],L"/?") == 0))
{
	wprintf(L"This program queries the schema for the specified classes or attributes.\n");
	wprintf(L"Syntax: getschemainfo [/C|/A][/V][querystring]\n");
	wprintf(L"where /C specifies to query for classes.\n");
	wprintf(L"      /A specifies to query for attributes.\n");
	wprintf(L"      /V specifies that all properties for the found classes or attributes should be returned.\n");
	wprintf(L"      querystring is the query criteria in ldap query format.\n");
	wprintf(L"Defaults: If neither /A or /C is specified, the query is against both.\n");
	wprintf(L"          If no /V is specified, the query returns only the ldapDisplayName and cn of the items found.\n");
	wprintf(L"          If no querystring is specified, the query returns all classes and/or attributes.\n");
	wprintf(L"Example: getschemainfo /A (IsSingleValued=TRUE)\n");
	wprintf(L"Returns all single-valued attributes in the schema.\n");
	wprintf(L"Common querystrings:\n");
	wprintf(L"For attributes:\n");
	wprintf(L"(cn=Street-Address) to find the attribute with CN of Street-Address.\n");
	wprintf(L"(ldapdisplayname=street) to find the attribute with ldapdisplayname of street.\n");
	wprintf(L"(IsSingleValued=TRUE) for single-valued attributes.\n");
	wprintf(L"(IsSingleValued=FALSE) for mulit-valued attributes.\n");
	wprintf(L"(systemFlags:1.2.840.113556.1.4.804:=00000001) for non-replicated attributes\n");
	wprintf(L"(systemFlags:1.2.840.113556.1.4.804:=00000004) for constructed attributes\n");
	wprintf(L"(searchFlags=1) for indexed attributes.\n");
	wprintf(L"(isMemberOfPartialAttributeSet=TRUE) for attributes included in the global catalog\n");
	return;
}

//Handle the command line arguments
int maxAlloc=MAX_PATH*2;
LPOLESTR pszBuffer = new OLECHAR[maxAlloc];
if ( !pszBuffer )
{
    wprintf(L"Alloc Failed ");
    return;
}
wcscpy_s(pszBuffer, maxAlloc, L"");

for (int i = 1;i<argc;i++)
{
	if (_wcsicmp(argv[i],L"/C") == 0)
	{
		bIsAttributeQuery = FALSE;
		szType = L"class";
	}
	else if (_wcsicmp(argv[i],L"/A") == 0)
	{
		bIsAttributeQuery = TRUE;
		szType = L"attribute";
	}
	else if (_wcsicmp(argv[i],L"/V") == 0)
	{
		bReturnVerbose = TRUE;
	}
	else
	{
		if ( IS_BUFFER_ENOUGH(maxAlloc,pszBuffer, argv[i]) > 0 )
		{
		   wcscpy_s(pszBuffer,maxAlloc,argv[i]);
		}
		else
		{

			wprintf(L"The argument is too large ");
            if ( pszBuffer )
                delete [] pszBuffer;
			return;
		}

	}
}
if (_wcsicmp(pszBuffer,L"") == 0)
  wprintf(L"\nFinding all %sSchema objects in the schema...\n\n",szType);
else
  wprintf(L"\nFinding %sSchema objects based on query: %s...\n\n",szType, pszBuffer);
	
HRESULT hr = S_OK;
//Get rootDSE and the domain container's DN.
IADs *pObject = NULL;
IDirectorySearch *pSchemaNC = NULL;
const unsigned int pathLen = MAX_PATH;
LPOLESTR szPath = new OLECHAR[pathLen];
if ( !szPath )
{
    wprintf(L"Alloc Failed ");
    delete [] pszBuffer;
    return;
}

//Intialize COM
CoInitialize(NULL);

VARIANT var;
hr = ADsOpenObject(L"LDAP://rootDSE",
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADs,
				 (void**)&pObject);
if (FAILED(hr))
{
   wprintf(L"Could not execute query. Could not bind to LDAP://rootDSE.\n");
   if (pObject)
     pObject->Release();
   delete [] pszBuffer;
   delete [] szPath;
   CoUninitialize();
     
   return;
}
if (SUCCEEDED(hr))
{
	hr = pObject->Get(L"schemaNamingContext",&var);
	if (SUCCEEDED(hr))
	{
        wcscpy_s(szPath,pathLen,L"LDAP://");

		if ( IS_BUFFER_ENOUGH(MAX_PATH,szPath, var.bstrVal, SysStringLen(var.bstrVal)) > 0 )
		{
            wcscat_s(szPath,pathLen,var.bstrVal);
		}
		else
		{
			wprintf(L"The Schema's DN is too large");
			pObject->Release();
            delete [] pszBuffer;	    
            delete [] szPath;
	        CoUninitialize();
			return;
		}

        hr = ADsOpenObject(szPath,
						 NULL,
						 NULL,
						 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
						 IID_IDirectorySearch,
						 (void**)&pSchemaNC);

		if (SUCCEEDED(hr))
		{
			hr = FindAttributesOrClasses(pSchemaNC, //IDirectorySearch pointer to schema naming context.
					 pszBuffer, 
					 NULL,
					 bIsAttributeQuery,
					 bReturnVerbose
				 );
			if (SUCCEEDED(hr))
			{
				if (S_FALSE==hr)
				   wprintf(L"No %sSchema object could be found based on the query: %s\n",szType,pszBuffer);
			}
			else if (0x8007203e==hr)
				wprintf(L"Could not execute query. An invalid filter was specified.\n");
			else
				wprintf(L"Query failed to run. HRESULT: %x\n",hr);
		}
		else
		{
		   wprintf(L"Could not execute query. Could not bind to the schema container.\n");
		}
		if (pSchemaNC)
		   pSchemaNC->Release();
	}
    VariantClear(&var);
}
if (pObject)
    pObject->Release();
delete [] pszBuffer;
delete [] szPath;    

// Uninitialize COM
CoUninitialize();
return;
}





HRESULT FindAttributesOrClasses(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
		 LPOLESTR szFilter, //Filter for finding specific attributes.
                            //NULL returns all attributeSchema objects.
         LPOLESTR *pszPropertiesToReturn, //Properties to return for attributeSchema objects found
					                    //NULL returns all set properties unless bIsVerbose is FALSE.
		 BOOL bIsAttributeQuery, //TRUE queries for attributes;FALSE for classes
		 BOOL bIsVerbose //TRUE means all properties for the found objects are displayed.
		                 //FALSE means only the ldapDisplayName with cn in parentheses:
						 //example: l (Locality-Name)
							)
{
	if (!pSchemaNC)
		return E_POINTER;
    //Create search filter
	int allocFilter = MAX_PATH*2;
	LPOLESTR pszSearchFilter = new OLECHAR[allocFilter];
	
    if ( !pszSearchFilter )
    {
        delete [] pszSearchFilter;
        return E_OUTOFMEMORY;
    }
        
	LPOLESTR szCategory = NULL;
	if (bIsAttributeQuery)
		szCategory = L"attributeSchema";
	else
		szCategory = L"classSchema";


	wcscpy_s(pszSearchFilter, allocFilter, L"(&(objectCategory=attributeSchema)%s)");
	BOOL bBufferOK=false;
	// Check for buffer overrun...
	if (szFilter)
	{
		if ( IS_BUFFER_ENOUGH(allocFilter,pszSearchFilter, szCategory) > 0 )
		{
	       swprintf_s(pszSearchFilter, allocFilter, L"(&(objectCategory=%s)%s)",szCategory,szFilter);
		   bBufferOK=true;
		}
	}
	else
	{
		if ( IS_BUFFER_ENOUGH(allocFilter,pszSearchFilter, szCategory) > 0 )
		{
 	       swprintf_s(pszSearchFilter, allocFilter,L"(objectCategory=%s)",szCategory);
		   bBufferOK=true;
		}
	}

	
	if( !bBufferOK)
	{
            delete [] pszSearchFilter;
			wprintf(L"Filter is too large - aborting");
			return E_FAIL;
	}

    //Attributes are one-level deep in the Schema container so only need to search one level.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = ADS_SCOPE_ONELEVEL;
    DWORD dwNumPrefs = 1;

	// COL for iterations
	LPOLESTR pszColumn = NULL;    
	ADS_SEARCH_COLUMN col;
    HRESULT hr;
    
    // Interface Pointers
    IADs    *pObj = NULL;
    IADs	* pIADs = NULL;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch = NULL;
	
	// Set the search preference
    hr = pSchemaNC->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

	LPOLESTR pszBool = NULL;
	DWORD dwBool;
	PSID pObjectSID = NULL;
	LPOLESTR szSID = NULL;
	LPGUID pObjectGUID = NULL;
	FILETIME filetime;
	SYSTEMTIME systemtime;
	DATE date;
	VARIANT varDate;
	LARGE_INTEGER liValue;
	LPOLESTR *pszPropertyList = NULL;
	LPOLESTR pszNonVerboseList[] = {L"lDAPDisplayName",L"cn"};

	LPOLESTR szCNValue = new OLECHAR[MAX_PATH];
	LPOLESTR szLDAPDispleyNameValue = new OLECHAR[MAX_PATH];
	LPOLESTR szDSGUID = new WCHAR [39];
	
	if ( !szCNValue || 	!szLDAPDispleyNameValue || !szDSGUID )
	{
        if ( szDSGUID )
            delete [] szDSGUID;
        if ( szCNValue )
            delete [] szCNValue;
        if ( szLDAPDispleyNameValue )
            delete [] szLDAPDispleyNameValue;
        if ( pszSearchFilter )
            delete [] pszSearchFilter;	
        return E_OUTOFMEMORY;
    }

	int iCount = 0;
	DWORD x = 0L;



	if (!bIsVerbose)
	{
		 //Return non-verbose list properties only
         hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
		                          pszNonVerboseList,
								  sizeof(pszNonVerboseList)/sizeof(LPOLESTR),
								  &hSearch
								  );
	}
	else
	{
		if (!pszPropertiesToReturn)
		{
			//Return all properties.
			hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
		                          NULL,
								  0L,
								  &hSearch
								  );
		}
		else
		{
			//specified subset.
		    pszPropertyList = pszPropertiesToReturn;
		   //Return specified properties
		   hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
		                          pszPropertyList,
								  sizeof(pszPropertyList)/sizeof(LPOLESTR),
								  &hSearch
								  );
		}
	}
 	if ( SUCCEEDED(hr) )
	{    
    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
	  hr = pSchemaNC->GetFirstRow( hSearch);
	  if (SUCCEEDED(hr))
	  {
        while( hr != S_ADS_NOMORE_ROWS )
		{
			//Keep track of count.
			iCount++;
			if (bIsVerbose)
			  wprintf(L"----------------------------------\n");
            // loop through the array of passed column names,
            // print the data for each column

			while( pSchemaNC->GetNextColumnName( hSearch, &pszColumn ) != S_ADS_NOMORE_COLUMNS )
            {
                hr = pSchemaNC->GetColumn( hSearch, pszColumn, &col );
			    if ( SUCCEEDED(hr) )
			    {
		            // Print the data for the column and free the column
				  if(bIsVerbose)
				  {
			        // Get the data for this column
				    wprintf(L"%s\n",col.pszAttrName);
					switch (col.dwADsType)
					{
						case ADSTYPE_DN_STRING:
                          for (x = 0; x< col.dwNumValues; x++)
						  {
							  wprintf(L"  %s\r\n",col.pADsValues[x].DNString);
						  }
						  break;
						case ADSTYPE_CASE_EXACT_STRING:	    
						case ADSTYPE_CASE_IGNORE_STRING:	    
						case ADSTYPE_PRINTABLE_STRING:	    
						case ADSTYPE_NUMERIC_STRING:	        
						case ADSTYPE_TYPEDNAME:	            
						case ADSTYPE_FAXNUMBER:	            
						case ADSTYPE_PATH:	                
						case ADSTYPE_OBJECT_CLASS:
						  for (x = 0; x< col.dwNumValues; x++)
						  {
							  wprintf(L"  %s\r\n",col.pADsValues[x].CaseIgnoreString);
						  }
						  break;
						case ADSTYPE_BOOLEAN:
						  for (x = 0; x< col.dwNumValues; x++)
						  {
							  dwBool = col.pADsValues[x].Boolean;
							  pszBool = dwBool ? L"TRUE" : L"FALSE";
							  wprintf(L"  %s\r\n",pszBool);
						  }
						  break;
						case ADSTYPE_INTEGER:
					      for (x = 0; x< col.dwNumValues; x++)
						  {
							  wprintf(L"  %d\r\n",col.pADsValues[x].Integer);
						  }
						  break;
						case ADSTYPE_OCTET_STRING:
						    if ( _wcsicmp(col.pszAttrName,L"objectSID") == 0 )
							{
						      for (x = 0; x< col.dwNumValues; x++)
							  {
								  pObjectSID = (PSID)(col.pADsValues[x].OctetString.lpValue);
								  //Convert SID to string.
								  ConvertSidToStringSid(pObjectSID, &szSID);
								  wprintf(L"  %s\r\n",szSID);
								  LocalFree(szSID);
							  }
							}
						    else if ( (_wcsicmp(col.pszAttrName,L"objectGUID") == 0)
								|| (_wcsicmp(col.pszAttrName,L"schemaIDGUID") == 0) 
								|| (_wcsicmp(col.pszAttrName,L"attributeSecurityGUID") == 0) )
							{
						      for (x = 0; x< col.dwNumValues; x++)
							  {
								//Cast to LPGUID
								pObjectGUID = (LPGUID)(col.pADsValues[x].OctetString.lpValue);
								//Convert GUID to string.
								::StringFromGUID2(*pObjectGUID, szDSGUID, 39); 
								//Print the GUID
								wprintf(L"  %s\r\n",szDSGUID);
							  }
							}
						    else if ( _wcsicmp(col.pszAttrName,L"oMObjectClass") == 0 )
							{
							  //TODO: 
							  wprintf(L"  TODO:No conversion for this.");
							}
							else
							  wprintf(L"  Value of type Octet String. No Conversion.");
						    break;
						case ADSTYPE_UTC_TIME:
						  for (x = 0; x< col.dwNumValues; x++)
						  {
							systemtime = col.pADsValues[x].UTCTime;
							if (SystemTimeToVariantTime(&systemtime,
														&date) != 0) 
							{
								//Pack in variant.vt
								varDate.vt = VT_DATE;
								varDate.date = date;
								VariantChangeType(&varDate,&varDate,VARIANT_NOVALUEPROP,VT_BSTR);
							    wprintf(L"  %s\r\n",varDate.bstrVal);
								VariantClear(&varDate);
							}
							else
								wprintf(L"  Could not convert UTC-Time.\n");
						  }
						  break;
						case ADSTYPE_LARGE_INTEGER:
						  for (x = 0; x< col.dwNumValues; x++)
						  {
						    liValue = col.pADsValues[x].LargeInteger;
							filetime.dwLowDateTime = liValue.LowPart;
							filetime.dwHighDateTime = liValue.HighPart;
							if((filetime.dwHighDateTime==0) && (filetime.dwLowDateTime==0))
							{
								wprintf(L"  No value set.\n");
							}
							else
							{
								//Check for properties of type LargeInteger that represent time
								//if TRUE, then convert to variant time.
								if ((0==wcscmp(L"accountExpires", col.pszAttrName))|
									(0==wcscmp(L"badPasswordTime", col.pszAttrName))||
									(0==wcscmp(L"lastLogon", col.pszAttrName))||
									(0==wcscmp(L"lastLogoff", col.pszAttrName))||
									(0==wcscmp(L"lockoutTime", col.pszAttrName))||
									(0==wcscmp(L"pwdLastSet", col.pszAttrName))
								   )
								{
									//Handle special case for Never Expires where low part is -1
									if (filetime.dwLowDateTime==-1)
									{
										wprintf(L"  Never Expires.\n");
									}
									else
									{
										if (FileTimeToLocalFileTime(&filetime, &filetime) != 0) 
										{
											if (FileTimeToSystemTime(&filetime,
																 &systemtime) != 0)
											{
												if (SystemTimeToVariantTime(&systemtime,
																			&date) != 0) 
												{
													//Pack in variant.vt
													varDate.vt = VT_DATE;
													varDate.date = date;
													VariantChangeType(&varDate,&varDate,VARIANT_NOVALUEPROP,VT_BSTR);
													wprintf(L"  %s\r\n",varDate.bstrVal);
													VariantClear(&varDate);
												}
												else
												{
													wprintf(L"  FileTimeToVariantTime failed\n");
												}
											}
											else
											{
												wprintf(L"  FileTimeToSystemTime failed\n");
											}

										}
										else
										{
											wprintf(L"  FileTimeToLocalFileTime failed\n");
										}
									}
								}
								else
								{
									//Print the LargeInteger.
									wprintf(L"  high: %d low: %d\r\n",filetime.dwHighDateTime, filetime.dwLowDateTime);
								}
							}
						  }
						  break;
						case ADSTYPE_NT_SECURITY_DESCRIPTOR:
						  for (x = 0; x< col.dwNumValues; x++)
						  {
							  wprintf(L"  Security descriptor.\n");
						  }
						  break;
						default:
				          wprintf(L"Unknown type %d.\n",col.dwADsType);
					}
				  }
			      else
				  {
					//Verbose handles only the two single-valued attributes: cn and ldapdisplayname
					//so this is a special case.
					if (0==wcscmp(L"cn", pszColumn))
					{
					  wcscpy_s(szCNValue,MAX_PATH,col.pADsValues->CaseIgnoreString);
					}
					if (0==wcscmp(L"lDAPDisplayName", pszColumn))
					{
					  wcscpy_s(szLDAPDispleyNameValue,MAX_PATH,col.pADsValues->CaseIgnoreString);
					}
				  }
			      pSchemaNC->FreeColumn( &col );
			    }
				FreeADsMem( pszColumn );
            }
		   if (!bIsVerbose)
			   wprintf(L"%s (%s)\n",szLDAPDispleyNameValue,szCNValue);
 		   //Get the next row
		   hr = pSchemaNC->GetNextRow( hSearch);
		}

	  }
	  // Close the search handle to clean up
      pSchemaNC->CloseSearchHandle(hSearch);
	} 
	if (SUCCEEDED(hr) && 0==iCount)
		hr = S_FALSE;

    delete [] szDSGUID;
    delete [] szCNValue;
    delete [] szLDAPDispleyNameValue;
    delete [] pszSearchFilter;
        
    return hr;
}




int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy)

{
	     
         if (toCopy == -1)

          {
               toCopy = wcslen(pszSource);
          }

		  return maxAlloc - (wcslen(pszTarget) + toCopy + 1); 
}


