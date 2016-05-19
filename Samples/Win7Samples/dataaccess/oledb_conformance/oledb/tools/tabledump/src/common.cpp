//--------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// File name: COMMON.CPP
//
//      Common\output routines for the Test TableDump.
//
//


////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "TableDump.h"
#include "TableDumpVer.h"


////////////////////////////////////////////////////////////////////////
// DumpProviderInfo
//
////////////////////////////////////////////////////////////////////////
void DumpProviderInfo(WCHAR* pwszProvider, WCHAR* pwszTableName, CHAR* pszCmdLine)
{
	ASSERT(pwszProvider);
	ASSERT(pwszTableName);
	ASSERT(pszCmdLine);

    //Get Coordinated universal time:             
    time_t ltime = 0;
	time(&ltime);

	//Dump the Heading Information...
	//////////////////////////////////////////////////////////////////////
	// OLE DB Conformance INI File
	//
	// File created by TableDump.exe generator version 1.50.3518
	// on Thu Jul 24 17:19:57 1998
	//
	//////////////////////////////////////////////////////////////////////
	OutputFile("//////////////////////////////////////////////////////////////////////\n");
	OutputFile("// OLE DB Conformance INI File\n");
	OutputFile("//\n");
	OutputFile("// File created by TableDump.exe generator version %s\n", VER_FILEVERSION_STR);
	OutputFile("// on %s", asctime(gmtime(&ltime)));
	OutputFile("//\n");
	OutputFile("//////////////////////////////////////////////////////////////////////\n");
	DumpLineEnd();

	//[INFO]
	OutputFile("%s[INFO] PROVIDER=; TABLENAME=; DEFAULTQUERY=;", START_OF_TYPE);
	DumpLineEnd();

	//VERSION
	OutputFile("%sVERSION=%d,%d,%d,%d;", START_OF_TYPE, VER_FILEVERSION);
	DumpLineEnd();

	//This line is needed.  Although we are passing the entire CmdLine to
	//The INI File, TABLE= may not have been used, (CREATETABLE= case...)
	OutputFile("%sTABLE=%S; ", START_OF_TYPE, pwszTableName);

	//The first Format Specifier in the DefualtQuery signifies the TableName
	//Since our TableName is Unicode, it needs to be a capable %S not %s
	ASSERT(pszCmdLine);
	CHAR* pszFormat = strstr(pszCmdLine, "%s");
	if(pszFormat)
		pszFormat[1] = 'S';

	//Dump the entire InitString passed to TableDump
	//The Privlib.dll can filter which ones it wants and doesn't...
	//NOTE:  pszCmdLine may contain a %S for the tablename, if it does
	//it will get filled in with the tablename also passed...
	OutputFile(pszCmdLine, pwszTableName);

	//END
	DumpLineEnd();
	DumpLineEnd();
	DumpLineEnd();
}


////////////////////////////////////////////////////////////////////////
// DumpURLInfo
//
////////////////////////////////////////////////////////////////////////
void DumpURLInfo(WCHAR* pwszDefURL, WCHAR* pwszRowURL, WCHAR* pwszTableName)
{
	if(!pwszDefURL)
		return;

	WCHAR* pwszFormat = wcsstr(pwszDefURL, L"%s");
	if(pwszFormat)
		pwszFormat[1] = 'S';

	pwszFormat = wcsstr(pwszRowURL, L"%s");
	if(pwszFormat)
		pwszFormat[1] = 'S';

	CHAR*	pszDefURL = ConvertToMBCS(pwszDefURL);
	CHAR*	pszRowURL = ConvertToMBCS(pwszRowURL);

	//[URL]
	OutputFile("%s[URL] URLInfo", START_OF_TYPE);
	DumpLineEnd();

	OutputFile("%s%s%s", START_OF_TYPE, "URL_DSO", START_OF_DATA);
	OutputFile(pszDefURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();
	OutputFile("%s%s%s", START_OF_TYPE, "URL_SESSION", START_OF_DATA);
	OutputFile(pszDefURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();
	OutputFile("%s%s%s", START_OF_TYPE, "URL_ROWSET", START_OF_DATA);
	OutputFile(pszDefURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();
	OutputFile("%s%s%s", START_OF_TYPE, "URL_ROW", START_OF_DATA);
	OutputFile(pszRowURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();
	OutputFile("%s%s%s", START_OF_TYPE, "URL_STREAM", START_OF_DATA);
	OutputFile(pszRowURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();
	OutputFile("%s%s%s", START_OF_TYPE, "URL_COMMAND", START_OF_DATA);
	//TableDump will not fill in this field. Leaving it blank.
	//OutputFile(pszDefURL, pwszTableName);
	OutputFile("%s", END_OF_DATA);
	DumpLineEnd();

	DumpLineEnd();
	DumpLineEnd();

	SAFE_FREE(pszDefURL);
	SAFE_FREE(pszRowURL);
}


////////////////////////////////////////////////////////////////////////
// DumpQueryInfo
//
////////////////////////////////////////////////////////////////////////
HRESULT DumpQueryInfo(CTable* pCTable, WCHAR* pwszDefaultQuery, WCHAR* pwszTableName)
{
	HRESULT hr = S_OK;
	
	//[QUERY]
	OutputFile("%s[QUERY] SQLEnumIdentifier Query", START_OF_TYPE);
	DumpLineEnd();
	
	//Obtain the SQLToken array...
	NAMEMAP* rgSQLTokens = NULL;
	ULONG cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Determine which query to output (DefaultQuery or just TableName)
	WCHAR* pwszQuery = pwszDefaultQuery ? pwszDefaultQuery : pwszTableName;
	WCHAR* pwszSQL	 = NULL;

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		//If we have access to the table (ie: CreateTable) actually dump the SQL Statement.
		if(pCTable && pCTable->GetCommandSupOnCTable())
		{
			//NOTE: We have to pass in the second table name (as as our tablename) since some
			//of the statements require a second table...
			if(FAILED(pCTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, pwszTableName, &pwszSQL, NULL, NULL)))
			{
				pwszSQL = NULL;
				pwszQuery = pwszDefaultQuery;
			}
		}
		
		//Output the query for the identifier... {SQLEnumIdentifier(Query)}
		if ( pwszQuery || pwszSQL )
		{
			OutputFile("%s%s%s%S%s", START_OF_TYPE, rgSQLTokens[i].pszName, START_OF_DATA, pwszSQL ? pwszSQL : pwszQuery, END_OF_DATA);
			DumpLineEnd();
		}

		SAFE_FREE(pwszSQL);
	}

	DumpLineEnd();
	DumpLineEnd();

//CLEANUP:
	SAFE_FREE(pwszSQL);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// DumpColumnsInfo
//
////////////////////////////////////////////////////////////////////////
void DumpColumnsInfo
(
    DBORDINAL		cColumns,
    DBCOLUMNINFO*	rgColInfo,
	CTable*			pCTable
)
{
	WCHAR*		pwszGuid = NULL;
	WCHAR		pwszNum[10];
	WCHAR*		pwszPropID = NULL;
	HRESULT		hr = S_OK;

	ASSERT(rgColInfo != NULL);
	 
	OutputFile("%s[COLUMN] ColName iOrdinal, TYPE, ulColumnSize, bPrecision, bScale, dwFlags, Prefix, Suffix, ColIDKind, ColIDGuid, ColIDName", START_OF_TYPE);
	DumpLineEnd();

	for(DBORDINAL i=0; i<cColumns; i++)
	{
		//Skip bookmark columns
		if(rgColInfo[i].iOrdinal == 0)
			continue;
		
		//Display ColumnInfo
		const static NAMEMAP rgColumnFlags[] = 
		{
			VALUE_CHAR(DBCOLUMNFLAGS_ISBOOKMARK),
			VALUE_CHAR(DBCOLUMNFLAGS_MAYDEFER),
			VALUE_CHAR(DBCOLUMNFLAGS_WRITE),
			VALUE_CHAR(DBCOLUMNFLAGS_WRITEUNKNOWN),
			VALUE_CHAR(DBCOLUMNFLAGS_ISFIXEDLENGTH),
			VALUE_CHAR(DBCOLUMNFLAGS_ISNULLABLE),
			VALUE_CHAR(DBCOLUMNFLAGS_MAYBENULL),
			VALUE_CHAR(DBCOLUMNFLAGS_ISLONG),
			VALUE_CHAR(DBCOLUMNFLAGS_ISROWID),
			VALUE_CHAR(DBCOLUMNFLAGS_ISROWVER),
			VALUE_CHAR(DBCOLUMNFLAGS_CACHEDEFERRED),
			
			VALUE_CHAR(DBCOLUMNFLAGS_SCALEISNEGATIVE),
			VALUE_CHAR(DBCOLUMNFLAGS_ISCHAPTER),
			VALUE_CHAR(DBCOLUMNFLAGS_ISROWURL),
			VALUE_CHAR(DBCOLUMNFLAGS_ISDEFAULTSTREAM),
			VALUE_CHAR(DBCOLUMNFLAGS_ISCOLLECTION),
		};

		//Format: {ColName(iOrdinal, TYPE, ulColumnSize, bPrecision, bScale, dwFlags}
		
		//ColumnName
		//fprintf replaces NULL with "(NULL)" which we don't want interpreted as the 
		//column name, so just output an empty string for this case...
		OutputFile("%s%S%,", START_OF_TYPE, rgColInfo[i].pwszName ? rgColInfo[i].pwszName : L"");
		
		//iOrdinal
		OutputFile("%d,", rgColInfo[i].iOrdinal);

		//TYPE
		OutputFile("%S ", GetDBTypeName(rgColInfo[i].wType & ~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR)) );
		if (rgColInfo[i].wType & DBTYPE_BYREF)
			OutputFile(" | DBTYPE_BYREF ");
		if (rgColInfo[i].wType & DBTYPE_ARRAY)
			OutputFile(" | DBTYPE_ARRAY ");
		if (rgColInfo[i].wType & DBTYPE_VECTOR)
			OutputFile(" | DBTYPE_VECTOR ");
		OutputFile(",");
		
		//ulColumnSize
		OutputFile("%ld,", rgColInfo[i].ulColumnSize );
		
		//bPrecision
		OutputFile("%d,",  rgColInfo[i].bPrecision );
		
		//bScale
		OutputFile("%d,",  rgColInfo[i].bScale );
		
		//dwFlags
		OutputFile("%s,",
			GetNoteStringBitvals(NUMELEM(rgColumnFlags), rgColumnFlags, rgColInfo[i].dwFlags));

		//ColIDKind
		OutputFile("%d,",  rgColInfo[i].columnid.eKind);

		if (rgColInfo[i].columnid.eKind != DBKIND_NAME &&
			rgColInfo[i].columnid.eKind != DBKIND_PROPID)
		{
			if(rgColInfo[i].columnid.eKind == DBKIND_GUID_NAME ||
				rgColInfo[i].columnid.eKind == DBKIND_GUID_PROPID ||
				rgColInfo[i].columnid.eKind == DBKIND_GUID)
				hr = StringFromCLSID(rgColInfo[i].columnid.uGuid.guid, &pwszGuid);
			else if(rgColInfo[i].columnid.uGuid.pguid)
				//It is a PGUID type.
				hr = StringFromCLSID(*(rgColInfo[i].columnid.uGuid.pguid), &pwszGuid);
		}

		switch (rgColInfo[i].columnid.eKind)
		{
		case DBKIND_GUID_NAME:
			OutputFile("%S,", pwszGuid);
			OutputFile("%S", rgColInfo[i].columnid.uName.pwszName);
			break;
		case DBKIND_GUID_PROPID:
			OutputFile("%S,",  pwszGuid);
			_ultow(rgColInfo[i].columnid.uName.ulPropid, pwszNum, 10);
			OutputFile("%S", pwszNum);
			break;
		case DBKIND_NAME:
			OutputFile("%s,", "");
			OutputFile("%S", rgColInfo[i].columnid.uName.pwszName);
			break;
		case DBKIND_PGUID_NAME:
			OutputFile("%S,", pwszGuid);
			OutputFile("%S", rgColInfo[i].columnid.uName.pwszName);
			break;
		case DBKIND_PGUID_PROPID:
			OutputFile("%S,", pwszGuid);
			_ultow(rgColInfo[i].columnid.uName.ulPropid, pwszNum, 10);
			OutputFile("%S", pwszNum);
			break;
		case DBKIND_PROPID:
			OutputFile("%s,", "");
			_ultow(rgColInfo[i].columnid.uName.ulPropid, pwszNum, 10);
			OutputFile("%S", pwszNum);
			break;
		case DBKIND_GUID:
			OutputFile("%S,", pwszGuid);
			OutputFile("%s", "");
			break;
		default:
			break;
		} //switch

		//Prefix Suffix.
		//Note: The prefix and suffix are not part of the ColumnInfo.  So we have to look them
		//up in the CTable object, hopefully we created the table so we have this info left
		//around from the Schema rowset.  
		WCHAR* pwszPrefix = NULL;
		WCHAR* pwszSuffix = NULL;
		CCol rCol;
		
		//We also use ColumnID matching, since DefaultQuery may be  a superset of the OpenRowset TableCreation.
		if(SUCCEEDED(pCTable->GetColInfo(&rgColInfo[i].columnid, rCol)))
		{
			pwszPrefix = rCol.GetPrefix();
			pwszSuffix = rCol.GetSuffix();
		}

		//Prefix
		OutputFile(",%ls,", pwszPrefix);
				
		//Suffix
		OutputFile("%ls", pwszSuffix);

		//ENDL
		DumpLineEnd();
	}

	SAFE_FREE(pwszGuid);

	DumpLineEnd();
	DumpLineEnd();
	OutputFile("%s[DATA] Type data", START_OF_TYPE);
	DumpLineEnd();
}



////////////////////////////////////////////////////////////////////////
// GetNoteStringBitvals
//
////////////////////////////////////////////////////////////////////////
CHAR* GetNoteStringBitvals
(
	ULONG			cNameMap,
	const NAMEMAP*	rgNameMap, 
	DWORD			dwValue 
)
{
	static CHAR szBuffer[MAX_QUERY_LEN];

	szBuffer[0] = '\0';

	// Make a string that combines all the bits ORed together.
	for(ULONG i=0; i<cNameMap; i++) 
	{
		if(rgNameMap[i].lItem & dwValue) 
		{
			if(szBuffer[0])
				strcat(szBuffer, " | " );
			strcat(szBuffer, rgNameMap[i].pszName);
		}
	}

	return szBuffer;
}




////////////////////////////////////////////////////////////////////////
// DumpRow
//
////////////////////////////////////////////////////////////////////////
void DumpRow
	(
    DBCOUNTITEM	cBindings,
    DBBINDING* 	rgBindings,
    void* 		pData
    )
{
	// Print each column we're bound to.
	for(DBCOUNTITEM iBind=0; iBind < cBindings; iBind++)
	{
		DumpColumn(&rgBindings[iBind], pData);
	}

	//End of Line
	DumpLineEnd();
}    


////////////////////////////////////////////////////////////////////////
// DumpColumn
//
////////////////////////////////////////////////////////////////////////
void DumpColumn
	(
	DBBINDING*		pBinding,
	void*			pData
	)
{
    ASSERT(pBinding && pData);

	void*	p = NULL;
	BOOL    fIsUnicode = TRUE;
	WCHAR	wszBuffer[MAX_QUERY_LEN];
	
	HRESULT hr = S_OK;
	VARIANT* pvVariant = NULL;
	IUnknown* pIUnknown = NULL;
	void* pByRef = NULL;
    
	// Pretty print a column.
	// May have different type of binding.

	DBSTATUS dwStatus = STATUS_BINDING(*pBinding, pData);
	DBLENGTH dwLength = LENGTH_BINDING(*pBinding, pData);

	DBTYPE wType = pBinding->wType;

	switch(dwStatus)
	{
		case DBSTATUS_S_ISNULL:
			dwLength = 0;
			break;
	
		case DBSTATUS_S_OK:
		case DBSTATUS_S_TRUNCATED:
			switch(wType)
			{
				case DBTYPE_BSTR:
				case DBTYPE_WSTR:
					// We have a string in our buffer, so use it.
					p = &VALUE_BINDING(*pBinding, pData);
					dwLength = wcslen((WCHAR*)p);
					break;

				case DBTYPE_STR:
					// We have a string in our buffer, so use it.
					p = &VALUE_BINDING(*pBinding, pData);
					dwLength = strlen((CHAR*)p);
					fIsUnicode = FALSE;
					break;

				case DBTYPE_VARIANT:
					// We have a variant in our buffer, so convert to string.
					pvVariant = (VARIANT*)&VALUE_BINDING(*pBinding, pData);
					wType = V_VT(pvVariant);

					//VariantChangeTypeEx seems to handle most types,
					TESTC(VariantToString(pvVariant, wszBuffer, MAX_QUERY_LEN, FALSE));
					p = wszBuffer;
					break;

				case DBTYPE_HCHAPTER:
				{
					HCHAPTER hChapter = (HCHAPTER)VALUE_BINDING(*pBinding, pData);
					swprintf(wszBuffer, L"%lu", hChapter);
					p = wszBuffer;	
					break;
				}

				case DBTYPE_IUNKNOWN:
				case DBTYPE_IUNKNOWN | DBTYPE_BYREF:
				{
					//It's possible that this object supports a storage object
					//If so, read all the data from the Storage Object and dump it
					//Otherwise just output the actual ptr					
					static WCHAR s_rgchByteHex[] = L"0123456789ABCDEF";
					
					ULONG cBytesRead = 0;
					ULONG ulMaxSize = MAX_QUERY_LEN / 2;
					BYTE* pbBuffer = (BYTE*)PROVIDER_ALLOC(ulMaxSize);
					WCHAR* pch = wszBuffer;

					p = wszBuffer;
					pIUnknown = (IUnknown*)VALUE_BINDING(*pBinding, pData);
					if(wType & DBTYPE_BYREF)
					{
						pByRef = (void*)VALUE_BINDING(*pBinding, pData);
						pIUnknown = *(IUnknown**)pByRef;
					}

					hr = StorageRead(IID_IUnknown, pIUnknown, pbBuffer, ulMaxSize, &cBytesRead);
						
					if (hr == S_OK || hr == S_FALSE)
					{
						// Convert the byte buffer to string.
						for (BYTE *pb = pbBuffer; cBytesRead ;cBytesRead--, pb++)
						{
							*pch++ = s_rgchByteHex[*pb >> 4];
							*pch++ = s_rgchByteHex[*pb & 0x0F];							
						}
						*pch = L'\0';											
					}
					else
					{
						swprintf(wszBuffer, L"%p", pIUnknown);
					}

					SAFE_FREE(pbBuffer);
					break;
				}
		
				default:
					if(wType & DBTYPE_VECTOR)
					{
						DBVECTOR* pVector = (DBVECTOR*)&VALUE_BINDING(*pBinding, pData);
						hr = VectorToString(pVector, wType, wszBuffer, MAX_QUERY_LEN);
						p = wszBuffer;
					}
					else if(wType & DBTYPE_ARRAY)
					{
						SAFEARRAY* pSafeArray = *(SAFEARRAY**)&VALUE_BINDING(*pBinding, pData);
						hr = SafeArrayToString(pSafeArray, wType, wszBuffer, MAX_QUERY_LEN);
						p = wszBuffer;
					}
					else
					{
						p = L"??? Unknown Type ???";
						dwLength = 0;
					}
					break;
			}
			break;

		default:
	        p = L"??? Unknown Status ???";
			dwLength = 0;
			break;
    };


	//Output column data
	if(dwStatus == DBSTATUS_S_ISNULL)
	{
		//Don't put "" arround "null"
		//Format:  {(null)}
		OutputFile("%s%s%s%s%s", START_OF_TYPE, "", START_OF_DATA, "", END_OF_DATA);
	}
	else
	{
		CHAR szTypeName[MAX_QUERY_LEN];
		sprintf(szTypeName, "%S", GetDBTypeName(wType & ~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR)));
		if (wType & DBTYPE_BYREF)
			strcat(szTypeName, " | DBTYPE_BYREF ");
		if (wType & DBTYPE_ARRAY)
			strcat(szTypeName, " | DBTYPE_ARRAY ");
		if (wType & DBTYPE_VECTOR)
			strcat(szTypeName, " | DBTYPE_VECTOR ");

		//Format:  {(DBTYPE_TYPE("data"))}
		if(fIsUnicode)
			OutputFile("%s%s%s%S%s", START_OF_TYPE, szTypeName, START_OF_DATA, p, END_OF_DATA);
		else
			OutputFile("%s%s%s%s%s", START_OF_TYPE, szTypeName, START_OF_DATA, p, END_OF_DATA);
	}

CLEANUP:
	// Free memory used by the variant.
	if(pvVariant)
		VariantClear(pvVariant);
	SAFE_RELEASE(pIUnknown);
//	SAFE_FREE(pByRef); By-Refs are freed by caller (ReleaseInputBindingsMemory)
	return;
}



////////////////////////////////////////////////////////////////////////
// DumpLineEnd
//
////////////////////////////////////////////////////////////////////////
void DumpLineEnd()
{
	if (g_fpLogFile)
	{	
		fputs( END_OF_LINE, g_fpLogFile );
		fputs( "\n"  , g_fpLogFile );
	}
}


////////////////////////////////////////////////////////////////////////
// OutputFile
//
////////////////////////////////////////////////////////////////////////
void OutputFile
	(
	const char* format,
	... 
	)
{
	if(g_fpLogFile)
	{
		va_list argptr;

		// log this message to a file
		va_start( argptr, format );
		vfprintf( g_fpLogFile, format, argptr);
		va_end( argptr );
	}
}



////////////////////////////////////////////////////////////////////////
// OutputText
//
////////////////////////////////////////////////////////////////////////
void OutputText(const CHAR* format, ...)
{
	va_list argptr;

	// log this message to stdout
	va_start( argptr, format );
	vprintf(format, argptr);
	va_end( argptr );
}    


////////////////////////////////////////////////////////////////////////
// OutputText
//
////////////////////////////////////////////////////////////////////////
void OutputText(const WCHAR* format, ...)
{
	va_list argptr;

	// log this message to stdout
	va_start( argptr, format );
	vwprintf(format, argptr);
	va_end( argptr );
}    



////////////////////////////////////////////////////////////////////////
// ParseCmdLine
//
////////////////////////////////////////////////////////////////////////
void ParseCmdLine(short* argc, WCHAR* argv[], ULONG* pcPropSets, DBPROPSET** prgPropSets)
{
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);

	//Provider
	//NOTE: Privlib will potentially need the provider passed in if not already in the InitString
	if(GetModInfo()->GetInitStringValue(L"PROVIDER", &argv[ARG_PROVIDER]))
	{
		CLSID clsidProvider;
		if(SUCCEEDED(CLSIDFromString(argv[ARG_PROVIDER], &clsidProvider)))
			GetModInfo()->SetProviderCLSID(clsidProvider);
	}
	
	//Binding Type
	GetModInfo()->GetInitStringValue(L"BINDINGTYPE", &argv[ARG_BINDINGTYPE]);

	//CreateTable
	GetModInfo()->GetInitStringValue(L"CREATETABLE", &argv[ARG_CREATETABLE]);

	//CreateIndex
	if(!GetModInfo()->GetInitStringValue(L"CREATEINDEX", &argv[ARG_CREATEINDEX]) && argv[ARG_CREATETABLE])
	{
		//Default is Create an index on the First Column
		//To not have a index use CREATEINDEX=0;
		argv[ARG_CREATEINDEX] = wcsDuplicate(L"1");
	}

	//Log File
	GetModInfo()->GetInitStringValue(L"OUTPUT", &argv[ARG_OUTPUT]);

	//GetInitProps
	GetModInfo()->GetInitProps(pcPropSets, prgPropSets);
}


