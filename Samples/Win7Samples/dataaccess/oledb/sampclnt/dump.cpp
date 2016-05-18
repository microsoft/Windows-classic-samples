//--------------------------------------------------------------------
// Microsoft OLE DB Sample Consumer
// (C) Copyright 1991 - 2000 Microsoft Corporation. All Rights Reserved.
//
// File name: DUMP.CPP
//
//      Dump\output routines for the SAMPCLNT sample OLE DB consumer.
//
//      See README.TXT for more information on the SAMPCLNT sample.
//
// Functions:
//
//      See SAMPCLNT.H for function prototypes
//



#include "sampclnt.h"




void DumpErrorMsg
(
 const char* format,
 ...
 )
{
	va_list argptr;						    

	assert(format != NULL);

	// log this message to stderr and to our log file
	va_start( argptr, format );
	tvfprintf( stderr, format, argptr);
	tvfprintf( g_fpLogFile, format, argptr);
	va_end( argptr );
}    



void DumpStatusMsg
(
 const char* format,
 ...
 )
{
	va_list argptr;

	assert(format != NULL);

	// log this message to stdout and to our log file
	va_start( argptr, format );
	tvfprintf( stdout, format, argptr );
	tvfprintf( g_fpLogFile, format, argptr );
	va_end( argptr );
}    




HRESULT DumpErrorHResult
(
 HRESULT      hr_return,
 const char  *format,			// can be NULL
 ... 
 )
{
	char     buff[100];
	int      cBytesWritten;
	va_list  argptr;

	//
	// Dump an error message.
	// Print the text of the HRESULT,
	// Return the HRESULT we were passed.

	// these result codes were generated from the oledberr.h 
	static Note ResultCodes[] = {
		// oledberr.h error codes
		NOTE(DB_E_BADACCESSORHANDLE),
		NOTE(DB_E_BADACCESSORHANDLE),
		NOTE(DB_E_ROWLIMITEXCEEDED),
		NOTE(DB_E_READONLYACCESSOR),
		NOTE(DB_E_SCHEMAVIOLATION),
		NOTE(DB_E_BADROWHANDLE),
		NOTE(DB_E_OBJECTOPEN),
		NOTE(DB_E_BADBINDINFO),
		NOTE(DB_SEC_E_PERMISSIONDENIED),
		NOTE(DB_E_NOTAREFERENCECOLUMN),
		NOTE(DB_E_NOCOMMAND),
		NOTE(DB_E_BADBOOKMARK),
		NOTE(DB_E_BADLOCKMODE),
		NOTE(DB_E_PARAMNOTOPTIONAL),
		NOTE(DB_E_BADRATIO),
		NOTE(DB_E_ERRORSINCOMMAND),
		NOTE(DB_E_BADSTARTPOSITION),
		NOTE(DB_E_NOTREENTRANT),
		NOTE(DB_E_NOAGGREGATION),
		NOTE(DB_E_DELETEDROW),
		NOTE(DB_E_CANTFETCHBACKWARDS),
		NOTE(DB_E_ROWSNOTRELEASED),
		NOTE(DB_E_BADSTORAGEFLAG),
		NOTE(DB_E_BADSTATUSVALUE),
		NOTE(DB_E_CANTSCROLLBACKWARDS),
		NOTE(DB_E_INTEGRITYVIOLATION),
		NOTE(DB_E_ABORTLIMITREACHED),
		NOTE(DB_E_DUPLICATEINDEXID),
		NOTE(DB_E_NOINDEX),
		NOTE(DB_E_INDEXINUSE),
		NOTE(DB_E_NOTABLE),
		NOTE(DB_E_CONCURRENCYVIOLATION),
		NOTE(DB_E_BADCOPY),
		NOTE(DB_E_BADPRECISION),
		NOTE(DB_E_BADSCALE),
		NOTE(DB_E_BADID),
		NOTE(DB_E_BADTYPE),
		NOTE(DB_E_DUPLICATECOLUMNID),
		NOTE(DB_E_DUPLICATETABLEID),
		NOTE(DB_E_TABLEINUSE),
		NOTE(DB_E_NOLOCALE),
		NOTE(DB_E_BADRECORDNUM),
		NOTE(DB_E_BOOKMARKSKIPPED),
		NOTE(DB_E_BADPROPERTYVALUE),
		NOTE(DB_E_INVALID),
		NOTE(DB_E_BADACCESSORFLAGS),
		NOTE(DB_E_BADSTORAGEFLAGS),
		NOTE(DB_E_BYREFACCESSORNOTSUPPORTED),
		NOTE(DB_E_NULLACCESSORNOTSUPPORTED),
		NOTE(DB_E_NOTPREPARED),
		NOTE(DB_E_BADACCESSORTYPE),
		NOTE(DB_E_WRITEONLYACCESSOR),
		NOTE(DB_SEC_E_AUTH_FAILED),
		NOTE(DB_E_CANCELED),
		NOTE(DB_E_BADSOURCEHANDLE),
		NOTE(DB_S_ROWLIMITEXCEEDED),
		NOTE(DB_S_COLUMNTYPEMISMATCH),
		NOTE(DB_S_TYPEINFOOVERRIDDEN),
		NOTE(DB_S_BOOKMARKSKIPPED),
		NOTE(DB_S_ENDOFROWSET),
		NOTE(DB_S_BUFFERFULL),
		NOTE(DB_S_CANTRELEASE),
		NOTE(DB_S_DIALECTIGNORED),
		NOTE(DB_S_UNWANTEDPHASE),
		NOTE(DB_S_COLUMNSCHANGED),
		NOTE(DB_S_ERRORSRETURNED),
		NOTE(DB_S_BADROWHANDLE),
		NOTE(DB_S_DELETEDROW),
		NOTE(DB_S_STOPLIMITREACHED),
		NOTE(DB_S_LOCKUPGRADED),
		NOTE(DB_S_PROPERTIESCHANGED),
		NOTE(DB_S_ERRORSOCCURRED),
		NOTE(DB_S_PARAMUNAVAILABLE),
		NOTE(DB_S_MULTIPLECHANGES),

		// winerr.h
		NOTE(E_UNEXPECTED),
		NOTE(E_NOTIMPL),
		NOTE(E_OUTOFMEMORY),
		NOTE(E_INVALIDARG),
		NOTE(E_NOINTERFACE),
		NOTE(E_POINTER),
		NOTE(E_HANDLE),
		NOTE(E_ABORT),
		NOTE(E_FAIL),
		NOTE(E_ACCESSDENIED),
		NOTE(S_OK),
		NOTE(S_FALSE),
		NOTE(E_UNEXPECTED),
		NOTE(E_NOTIMPL),
		NOTE(E_OUTOFMEMORY),
		NOTE(E_INVALIDARG),
		NOTE(E_NOINTERFACE),
		NOTE(E_POINTER),
		NOTE(E_HANDLE),
		NOTE(E_ABORT),
		NOTE(E_FAIL),
		NOTE(E_ACCESSDENIED),
		// BindMoniker Errors
		NOTE(MK_E_NOOBJECT),
		NOTE(MK_E_EXCEEDEDDEADLINE),
		NOTE(MK_E_CONNECTMANUALLY),
		NOTE(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED),
		NOTE(STG_E_ACCESSDENIED),
		NOTE(MK_E_SYNTAX),
		NOTE(MK_E_CANTOPENFILE),
	};


	// Format the message.
	// Print name of hresult code.

	if (format)
	{
		va_start( argptr, format );
		cBytesWritten = _vsnprintf_s( buff, NUMELEM(buff), _TRUNCATE, format, argptr );
		va_end( argptr );
	}
	else
		strcpy_s( buff, NUMELEM(buff),  "" );

	// log to stderr and also to our log file
	tfprintf( stderr, "%.*s: Returned %.30s\n", 
		sizeof(buff), buff, 
		GetNoteString( ResultCodes, NUMELEM(ResultCodes), GetScode(hr_return)) );

	tfprintf( g_fpLogFile, "%.*s: Returned %.30s\n", 
		sizeof(buff), buff, 
		GetNoteString( ResultCodes, NUMELEM(ResultCodes), GetScode(hr_return)) );

	return ResultFromScode( hr_return );
}




void DumpColumnsInfo
(
 DBCOLUMNINFO*	pColInfo,
 DBORDINAL		cCol
 )
{
	DBORDINAL j;

	assert(pColInfo != NULL);

	tfprintf( g_fpLogFile, "\nColumn Information:\n\n"); 

	for (j=0; j < cCol; j++)
	{
		WriteColumnInfo( g_fpLogFile, &pColInfo[j] );
	}
}



void WriteColumnInfo
(
 FILE*			fp,
 DBCOLUMNINFO*	p 
 )
{
	DBID 	    *pCol;
	DBKIND      eKind;
	wchar_t 	wszGuidBuff[MAX_GUID_STRING];
	wchar_t     wszNameBuff[MAX_GUID_STRING];    

	static char *szDbcolkind[] = { "Guid+Name", "Guid+PropID", "Name", 
		"Guid+Name", "Guid+PropID", "PropID", "Guid" };

	assert(p != NULL);

	// For DBTYPEENUM.  Doesn't need to be in order.
	// Below we mask off the high bits.
	static Note typenotes[] = 
	{
		NOTE(DBTYPE_EMPTY),
		NOTE(DBTYPE_NULL),
		NOTE(DBTYPE_I2),
		NOTE(DBTYPE_I4),
		NOTE(DBTYPE_R4),
		NOTE(DBTYPE_R8),
		NOTE(DBTYPE_CY),
		NOTE(DBTYPE_DATE),
		NOTE(DBTYPE_BSTR),
		NOTE(DBTYPE_IDISPATCH),
		NOTE(DBTYPE_ERROR),
		NOTE(DBTYPE_BOOL),
		NOTE(DBTYPE_VARIANT),
		NOTE(DBTYPE_IUNKNOWN),
		NOTE(DBTYPE_DECIMAL),
		NOTE(DBTYPE_UI1),
		NOTE(DBTYPE_ARRAY),
		NOTE(DBTYPE_BYREF),
		NOTE(DBTYPE_I1),
		NOTE(DBTYPE_UI2),
		NOTE(DBTYPE_UI4),
		NOTE(DBTYPE_I8),
		NOTE(DBTYPE_UI8),
		NOTE(DBTYPE_GUID),
		NOTE(DBTYPE_VECTOR),
		NOTE(DBTYPE_RESERVED),
		NOTE(DBTYPE_BYTES),
		NOTE(DBTYPE_STR),
		NOTE(DBTYPE_WSTR),
		NOTE(DBTYPE_NUMERIC),
		NOTE(DBTYPE_UDT),
		NOTE(DBTYPE_DBDATE),
		NOTE(DBTYPE_DBTIME),
		NOTE(DBTYPE_DBTIMESTAMP),
	};

	static Note flagnotes[] = 
	{
		NOTE(DBCOLUMNFLAGS_ISBOOKMARK),
		NOTE(DBCOLUMNFLAGS_MAYDEFER),
		NOTE(DBCOLUMNFLAGS_WRITE),
		NOTE(DBCOLUMNFLAGS_WRITEUNKNOWN),
		NOTE(DBCOLUMNFLAGS_ISFIXEDLENGTH),
		NOTE(DBCOLUMNFLAGS_ISNULLABLE),
		NOTE(DBCOLUMNFLAGS_MAYBENULL),
		NOTE(DBCOLUMNFLAGS_ISLONG),
		NOTE(DBCOLUMNFLAGS_ISROWID),
		NOTE(DBCOLUMNFLAGS_ISROWVER),
		NOTE(DBCOLUMNFLAGS_CACHEDEFERRED),
	};

	pCol = & p->columnid;
	eKind = pCol->eKind;

	// stringize GUID for pretty printing
	switch (eKind)
	{
	case DBKIND_GUID_NAME:
	case DBKIND_GUID_PROPID:
	case DBKIND_GUID:
		StringFromGUID2( pCol->uGuid.guid, wszGuidBuff, sizeof(wszGuidBuff) );
		break;
	case DBKIND_PGUID_NAME:
	case DBKIND_PGUID_PROPID:
		StringFromGUID2( *(pCol->uGuid.pguid), wszGuidBuff, sizeof(wszGuidBuff) );
		break;
	default:
		wcscpy_s( wszGuidBuff, NUMELEM(wszGuidBuff), L"<none>" );
		break;    
	}

	// stringize name or propID for pretty printing   
	switch (eKind)
	{
	case DBKIND_GUID_NAME:
	case DBKIND_NAME:
	case DBKIND_PGUID_NAME:
		swprintf_s( wszNameBuff, NUMELEM(wszNameBuff), L"[name=%.50S]", pCol->uName.pwszName ? pCol->uName.pwszName : L"(unknown)" );
		break;
	case DBKIND_GUID_PROPID:
	case DBKIND_PGUID_PROPID:
	case DBKIND_PROPID:
		swprintf_s( wszNameBuff, NUMELEM(wszNameBuff), L"[propid=%lu]", pCol->uName.ulPropid );
		break;
	default:
		wcscpy_s( wszNameBuff, NUMELEM(wszNameBuff), L"" );
		break;    
	}   

	// pretty print column info
	tfprintf( fp, "ColumnId [kind=%.40s] [guid=%.40S] %.60S\n", 
		szDbcolkind[eKind], wszGuidBuff, wszNameBuff );


	// Now move on to other stuff...
	// Name in DBCOLUMNINFO different than name in DBCOLUMNID (maybe).
	tfprintf(fp, "  Name          = '%.50S'\n", p->pwszName );
	tfprintf(fp, "  iOrdinal      = %Id\n", p->iOrdinal);
	tfprintf(fp, "  wType         = %.100s\n", 
		GetNoteString( typenotes, NUMELEM(typenotes),
		p->wType & (~DBTYPE_BYREF) & (~DBTYPE_ARRAY) & (~DBTYPE_VECTOR) ) );
	if (p->wType & DBTYPE_BYREF)
		tfprintf(fp, "      (BYREF)\n");
	if (p->wType & DBTYPE_ARRAY)
		tfprintf(fp, "      (ARRAY)\n");
	if (p->wType & DBTYPE_VECTOR)
		tfprintf(fp, "      (VECTOR)\n");
	tfprintf(fp, "  ulColumnSize  = %Id\n", p->ulColumnSize );
	tfprintf(fp, "  bPrecision    = %d\n",  p->bPrecision );
	tfprintf(fp, "  bScale        = %d\n",  p->bScale );
	tfprintf(fp, "  dwFlags       = %s\n\n",
		GetNoteStringBitvals( flagnotes, NUMELEM(flagnotes), p->dwFlags ) );


}


char* GetNoteString
( 
 Note * rgNote, 
 int    cNote,
 DWORD  dwValue 
 )
{
	int j;

	assert(rgNote != NULL);

	// Scan a table of value/string,
	// return ptr to string found.

	for (j=0; j < cNote; j++) {
		if (rgNote[j].dwFlag == dwValue)
			return rgNote[j].szText;
	}
	return "<unknown>";
}







char*	GetNoteStringBitvals
(
 Note* 	rgNote,
 int     cNote,
 DWORD   dwValue 
 )
{
	static char buff[400];
	int j;

	assert(rgNote != NULL);

	// Make a string that combines all the bits ORed together.

	strcpy_s(buff, NUMELEM(buff), "");
	for (j=0; j < cNote; j++) {
		if (rgNote[j].dwFlag & dwValue) {
			if (buff[0])
				strcat_s( buff, NUMELEM(buff), " | " );
			strcat_s( buff, NUMELEM(buff), rgNote[j].szText );
		}
	}
	assert(strlen(buff) < sizeof(buff));
	return buff;
}




DBLENGTH CalcPrettyPrintMaxColWidth
(
 DBBINDING* rgBind,
 DBCOUNTITEM cBind
 )
{
	DBLENGTH	cMaxWidth;
	DBLENGTH	cTotalWidth;
	DBCOUNTITEM	iBind;

	assert(rgBind != NULL);

	cMaxWidth = DEFAULT_CBMAXLENGTH;
	while (1)
	{
		cTotalWidth = 0;

		for (iBind=0; iBind < cBind; iBind++)
			cTotalWidth += min( cMaxWidth, rgBind[iBind].cbMaxLen ) + 1;

		if (cTotalWidth < PRETTYPRINT_MAXTOTALWIDTH || cMaxWidth < PRETTYPRINT_MINCOLWIDTH)
			break;

		cMaxWidth--;
	}

	return cMaxWidth;
}



void DumpColumnHeadings
(
 DBBINDING*		rgBind, 
 DBCOUNTITEM	cBind, 
 DBCOLUMNINFO* 	pColInfo, 
 DBORDINAL		cCol,
 DBLENGTH		cMaxColWidth
 )
{
	DBCOUNTITEM iBind;

	assert(rgBind != NULL);
	assert(pColInfo != NULL);

	for (iBind=0; iBind < cBind; iBind++)
	{
		tfprintf( g_fpLogFile, "%-*.*S ",
			min( cMaxColWidth, rgBind[iBind].cbMaxLen ),
			min( cMaxColWidth, rgBind[iBind].cbMaxLen ),
			LookupColumnName( pColInfo, cCol, rgBind[iBind].iOrdinal ) );
	}
	tfprintf( g_fpLogFile, "\n" );

	for (iBind=0; iBind < cBind; iBind++)
	{
		tfprintf( g_fpLogFile, "%-*.*s ",
			min( cMaxColWidth, rgBind[iBind].cbMaxLen ),
			min( cMaxColWidth, rgBind[iBind].cbMaxLen ),
			"------------------------------" );
	}
	tfprintf( g_fpLogFile, "\n" );
}



WCHAR* LookupColumnName
(
 DBCOLUMNINFO*	rgColInfo,
 DBORDINAL 		cCol,
 DBORDINAL 		iCol 
 )
{
	DBORDINAL j;

	assert(rgColInfo != NULL);

	// A really slow way to get the column name, given the ordinal.
	// The problem is that result-set ordinals do not necessarily match
	// the index into the ColumnInfo array.
	// (May have bookmark, which is always column 0.)

	for (j=0; j < cCol; j++)
		if (rgColInfo[j].iOrdinal == iCol)
			return rgColInfo[j].pwszName;

	return L"Error";
}




void DumpRow
(
 DBBINDING*		rgBind,
 DBCOUNTITEM	cBind,
 DBLENGTH		cMaxColWidth,
 BYTE* 			pData
 )
{
	DBCOUNTITEM	iBind;
	COLUMNDATA*	pColumn;

	assert(rgBind);
	assert(offsetof(COLUMNDATA, uLength) == 0);	

	// Print each column we're bound to.
	for (iBind=0; iBind < cBind; iBind++)
	{
		// Columns are bound differently; not so easy.
		// Print out to at least DEFAULT_CBMAXLENGTH width (pretty),
		// Limit to first dwLength characters.

		pColumn = (COLUMNDATA *) (pData + rgBind[iBind].obLength);
		PrintColumn( pColumn, rgBind, iBind, cMaxColWidth );
	}
	tfprintf( g_fpLogFile, "\n" );
}    





void PrintColumn
(
 COLUMNDATA		*pColumn,
 DBBINDING		*rgBind,
 DBCOUNTITEM	iBind,
 DBLENGTH		cMaxColWidth 
 )
{
	void*		p;
	DBLENGTH	uPrintWidth;
	DBLENGTH	uPrintPrecision;
	DWORD		dwStatus;
	DBLENGTH	uLength;
	BOOL		fDidVariant;
	BOOL		fIsUnicode;
	char*		sFormat;
	HRESULT		hr;

	assert(pColumn != NULL);
	assert(rgBind != NULL);

	// Pretty print a column.
	// May have different type of binding.

	fDidVariant = FALSE;
	fIsUnicode  = FALSE;
	dwStatus = pColumn->dwStatus;
	uLength = pColumn->uLength;

	if (dwStatus == DBSTATUS_S_ISNULL)
	{
		p = "<null>";
		uLength = strlen( (char *) p);
	}
	else if (dwStatus == DBBINDSTATUS_UNSUPPORTEDCONVERSION)
	{
		p = "<unsupportedconversion>";
		uLength = strlen( (char *) p);
	}    
	else
	{
		switch (rgBind[iBind].wType) 
		{
		case DBTYPE_STR:
			// We have a string in our buffer, so use it.
			p = (void *) &pColumn->bData;
			break;
		case DBTYPE_VARIANT:
			// We have a variant in our buffer, so convert to string.
			p = (void *) &pColumn->bData;
			hr = VariantChangeTypeEx(
				(VARIANT *) p,			// Destination (convert in place)
				(VARIANT *) p,			// Source
				LOCALE_SYSTEM_DEFAULT,	// LCID
				0,						// dwFlags
				VT_BSTR );
			if (FAILED(hr))
			{
				DumpErrorHResult( hr, "VariantChangeTypeEx, field %Id", iBind );
				return;
			}
			p = (wchar_t *) (((VARIANT *)p)->bstrVal) ;
			uLength = ((DWORD *)p)[-1] / sizeof(wchar_t);
			fDidVariant = TRUE;
			fIsUnicode  = TRUE;
			break;
		default:
			p = "??? unknown type ???";
			break;
		}
	}

	// Print the column.
	// If it has been truncated or rounded, print a '#' in
	// the far right-hand column.
	uPrintWidth     = min( cMaxColWidth, rgBind[iBind].cbMaxLen );
	uPrintPrecision = min( cMaxColWidth, uLength );
	if (dwStatus == DBSTATUS_S_TRUNCATED ||  cMaxColWidth < uLength)
	{
		uPrintWidth--;
		uPrintPrecision--;
	}

	sFormat = fIsUnicode ? "%-*.*S" : "%-*.*s";

	tfprintf( g_fpLogFile, sFormat, uPrintWidth, uPrintPrecision, p );

	if (dwStatus == DBSTATUS_S_TRUNCATED ||  cMaxColWidth < uLength)
		tfprintf( g_fpLogFile, "#" );
	tfprintf( g_fpLogFile, " " );

	// Free memory used by the variant.
	if (fDidVariant)
		VariantClear( (VARIANT *) &pColumn->bData );

	return;
}




void tfprintf
(
 FILE*		fp,
 const char* format,
 ... 
 )
{
	int 	cBytesWritten;
	char 	buff[400];
	va_list argptr;

	assert(format != NULL);

	// Dump a formatted string.
	// _vsnprintf prevents overflowing our buffer.
	va_start( argptr, format );
	cBytesWritten = _vsnprintf_s( buff, NUMELEM(buff), _TRUNCATE, format, argptr );
	va_end( argptr );
	buff[sizeof(buff)-1] = '\0';

	// Can't use fprintf, because string could contain '%'.
	if (fp)
		fputs( buff, fp );
}



void tvfprintf
(
 FILE*			fp,
 const char*	format,
 va_list		argptr 
 )
{
	int 	cBytesWritten;
	char 	buff[400];


	assert(format != NULL);

	// Dump a formatted string.
	// _vsnprintf prevents overflowing our buffer.
	cBytesWritten = _vsnprintf_s( buff, NUMELEM(buff), _TRUNCATE, format, argptr );
	buff[sizeof(buff)-1] = '\0';

	// Can't use fprintf, because string could contain '%'.
	if (fp)
		fputs( buff, fp );
}


