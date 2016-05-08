//--------------------------------------------------------------------
// Microsoft OLE DB 
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module CParseStrm.CPP | This module contains the File Manipulation code
// to read column metadata and text
//
//	TextFile Format
//
//	[INFO]
//	{Provider(TableName, <DataSource>, <UserID>, <Password>)}
//	...
//	[URL] - This section is optional. It may not exist in the INI file.
//	{URLs for Object Types}
//	...
//	[QUERY]
//	{SQLEnumIdentifier(query)}
//	...
//	[COLUMN]
//	{ColName1(iOrdinal, TYPE, ulColumnSize, bPrecision, bScale, dwFlags}
//	...
//	[DATA]
//	{<DBTYPE_TYPE>("data")}
//	...
//
//	NOTES:
//	All items above in "<>" are optional
//	null data is represented as (null) 
//  empty data is represented as ""
//


//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "privstd.h"		//Pre-compiled headers
#include "CParseStrm.hpp"
#include "miscfunc.h"
#include "Strings.h"


//////////////////////////////////////////////////////////////////////////
// CParseInitFile
//
//////////////////////////////////////////////////////////////////////////
CParseInitFile::CParseInitFile()
{
    //Allocate Stream Buffer
    m_pvInput			 = (CHAR*)PROVIDER_ALLOC(MAX_INPUT_BUFFER*sizeof(char));
	if(m_pvInput) 
		m_pvInput[0]	 = '\0';
	m_pszFileName		 = NULL;
    m_cRows				 = 0;

	//Array of URLs.
	m_cURLs	= NUMELEM(m_rgURLs);
	memset(m_rgURLs, 0, sizeof(m_rgURLs));

	//Array of QueryNams
	m_cQueryNames		 = NUMELEM(m_rgQueryNames);
	memset(m_rgQueryNames, 0, sizeof(m_rgQueryNames));

	//RowOffset Array
	m_cRowOffsets		= 0;
	m_rgRowOffsets		= NULL;
}

//////////////////////////////////////////////////////////////////////////
// ~CParseInitFile
//
//////////////////////////////////////////////////////////////////////////
CParseInitFile:: ~CParseInitFile()
{
	ULONG	i=0;

    // Close file
    if(is_open())
        close();

	m_ColData.RemoveAll();
    // Delete buffers
    PROVIDER_FREE(m_pvInput)
	PROVIDER_FREE(m_pszFileName);

	//Free Array of URLs
	for(i=0; i<m_cURLs; i++)
		PROVIDER_FREE(m_rgURLs[i]);

	//Free Array of QueryNames
	for(i=0; i<m_cQueryNames; i++)
		PROVIDER_FREE(m_rgQueryNames[i]);

	//Free RowOffset array
	PROVIDER_FREE(m_rgRowOffsets);
}


//--------------------------------------------------------------------
// @mfunc Initialization routine, opens file specified and creates
// buffers
//
// @rdesc BOOL
//      @flag S_OK | Succeeded
//      @flag E_FAIL | Failed to Initialize
//
BOOL CParseInitFile::Init(WCHAR* pwszFileName)
{
	//memory allocated in the ctor
	ASSERT(m_pvInput);
	ASSERT(pwszFileName);

	TRACE_CALL(L"PRIVLIB: CParseInitFile::Init.\n");

	// convert to ANSI
	m_pszFileName = (CHAR*)PROVIDER_ALLOC(wcslen(pwszFileName) + sizeof(CHAR));
	ASSERT(m_pszFileName);
	ConvertToMBCS(pwszFileName, m_pszFileName, (int)(wcslen(pwszFileName) + sizeof(CHAR)));
	
    //Open the File (allow share_read mode)
#if (_MSC_VER >= 1330) && !defined (_NT_TOOLS)
	open(m_pszFileName, ios::in | ios::_Nocreate);
#else
	open(m_pszFileName, ios::in | ios::nocreate, 05000/*filebuf::sh_read*/);
#endif


    if(!is_open())
	{
		odtLog << L"ERROR:  Unable to open or find INI <File:" << m_pszFileName << ">" << ENDL;
        return FALSE;
	}
	
	//set mode filebuf::binary, so there is no translation of 
	//special characters, expecially line-feeds and carrigae returns...
//	setmode(0x4000/*filebuf::text*/);
	
	// Obtain the Column Names, Data Types,
    // for each of the rows
    if(!GenerateFileInfo())
        return FALSE;

	return TRUE;
}


DBLENGTH CParseInitFile::GetRowOffset(DBCOUNTITEM iRow)
{
	//NOTE:  iRow is 1-based...
	ASSERT(m_rgRowOffsets != NULL);
	ASSERT(iRow > 0 && iRow <= m_cRows && iRow <= m_cRowOffsets);
	return m_rgRowOffsets[iRow-1]; 
}

	
HRESULT CParseInitFile::GetNextLine(CHAR* pszBuffer, DBLENGTH cLength)
{
	ASSERT(pszBuffer);
    CHAR* psz = END_OF_LINE;

	do
	{
		//Clear previous error bits
		clear();

		//Get the next line of input out of the INI file,
		//and places data into m_pvInput
		pszBuffer[0] = '\0';
		getline(pszBuffer, (ULONG) cLength, psz[0]); //64bit TODO - remove the ULONG cast.

		//End of file
		if(ios::eof())
			return S_FALSE;
	
		//Error
		if(!istream::good()) 
			return E_FAIL;

		//Skip all lines that don't have our "known" starting character...
	} while(strstr(pszBuffer, START_OF_TYPE)==NULL);

	return S_OK;
}


HRESULT CParseInitFile::FindSection(
	CHAR* pszSectionName,	//Name of section to find.
	CHAR* pszFirstReqSec	//Name of first required section if pszSectionName
							//is an optional section.
	)
{
	ASSERT(pszSectionName);
	size_t ulNameLen = strlen(pszSectionName);
	HRESULT hr = S_OK;

	//Loop over the File until we find the [sectionname] marker...
	CHAR* pszStartofType = strstr(m_pvInput, START_OF_TYPE);
	while(hr == S_OK)
	{
		//If we have a match were done!
		if(pszStartofType)
		{
			if(strncmp(pszStartofType+1, pszSectionName, ulNameLen)==0)
				return S_OK;
			else if(pszFirstReqSec)
			{
				size_t ulNameLen2 = strlen(pszFirstReqSec);

				if(strncmp(pszStartofType+1, pszFirstReqSec, ulNameLen2)==0)
					return S_FALSE;
			}
		}

		//Get the NextLine
		if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER))!=S_OK)
		{
			odtLog << "ERROR:  Unable to find " << pszSectionName <<" marker in INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return E_FAIL;
		}
		
		//Obtain the next starting element
		pszStartofType = strstr(m_pvInput, START_OF_TYPE);
	}

	return E_FAIL;
}



//--------------------------------------------------------------------
// @mfunc	Obtain the column metadata from the file
//			Obtain the offsets into the file that each row exists at.
//			
//
// @rdesc BOOL
//      @flag TRUE | Got the offsets, Column Names and Data Types
//      @flag FALSE | Could not obtain all the necessary info
//
BOOL CParseInitFile::GenerateFileInfo()
{
	TRACE_CALL(L"PRIVLIB: CParseInitFile::GenerateFileInfo.\n");

	//Obtain ProviderInfo, including TableName of generated Data...
	if(!ParseProviderInfo())
		return FALSE;

	//Obtain URLInfo
	if(!ParseURLInfo())
		return FALSE;

	//Obtain QueryInfo, (all SQL Statements)
	if(!ParseQueryInfo())
		return FALSE;

    // Generate Column Info, if FALSE is returned, a problem
    // was encountered while reading the Column Names.
    if(!ParseColumnInfo())
        return FALSE;

    // Get the Data in the file
    // FALSE is returned if error was encountered while reading the Data
    if(!ParseDataInfo())
        return FALSE;
 
    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Tokenize the Provider info
//
// @rdesc BOOL
//      @flag TRUE | Parsing yielded no Error
//
BOOL CParseInitFile::ParseProviderInfo()
{
	//move to beginning of file
    seekg(0L);
	CHAR* pszStart = NULL;
	CHAR* pszEnd = NULL;
	CHAR szVersion[100];

	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseProviderInfo.\n");

	//Skip over any lines, until the [INFO] section is reached...
	//Make sure the INI contains the required version (at least)
	if( FindSection("[INFO]")==S_OK &&
		GetNextLine(m_pvInput, MAX_INPUT_BUFFER)==S_OK  &&
		(pszStart = strstr(m_pvInput, START_OF_TYPE)) &&
		(pszStart = strstr(pszStart, "VERSION=")) &&
		(pszStart = strstr(pszStart, ",")) &&
		(pszStart = strstr(pszStart+1, ",")) &&
		(pszEnd = strstr(pszStart+1, ",")) )
	{
		//Build Version is between 2nd and 3rd comma.  (1,50,3518,00)
		pszStart++;
		strncpy(szVersion, pszStart, (size_t)(pszEnd - pszStart));
		szVersion[pszEnd - pszStart] = '\0';
		ULONG ulVersion = strtoul(szVersion, NULL, 10);
		if(ulVersion == ULONG_MAX || ulVersion < 3518)
		{
			odtLog << "ERROR:  This version of the Privlib requires a INI File generated from " << ENDL;
			odtLog << "ERROR:  from TableDump.exe 1.50.3518.00 or later." << ENDL;
			return FALSE;
		}
	}
	else
	{
		odtLog << "ERROR:  Unable to find Versioning Information in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  This version of the Privlib requires a INI with a version section" << ENDL;
		odtLog << "ERROR:  and generated using a version of TableDump.exe 1.50.3518.00 or later." << ENDL;
		return FALSE;
	}

	//Get the NextLine {(TABLE=; DEFAULTQUERY=; DATASOURCE=; USERID=; PASSWORD=; etc... )}
	if(GetNextLine(m_pvInput, MAX_INPUT_BUFFER)!=S_OK ||
		(pszStart = strstr(m_pvInput, START_OF_TYPE))==NULL ||
		(pszStart = strstr(pszStart, "TABLE="))==NULL)
	{
		odtLog << "ERROR:  Unable to find InitString containing Initialization Information in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
		return FALSE;
	}
    
	//We just need to append the InitString from the FILE to the InitString 
	//Already stored in the CModInfo from LTM.  And we will parse both together...
	GetModInfo()->AddToInitString(pszStart);
	return  TRUE;
}


//--------------------------------------------------------------------
// @mfunc Tokenize the URL info
//
// @rdesc BOOL
//      @flag TRUE | Parsing yielded no Error
//
BOOL CParseInitFile::ParseURLInfo()
{
	CHAR*	pszStart = m_pvInput;
	CHAR*	pszEnd = m_pvInput;
	CHAR*	pszEndData = m_pvInput;
	EINTERFACE	eInterface;
	HRESULT hr = S_OK;
	
	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseURLInfo.\n");

	//Skip over any lines, until the [URL] section (or the [QUERY] 
	//section) is reached...
	if(FAILED(hr = FindSection("[URL]", "[QUERY]")))
	{
		return FALSE;
	}

	if(hr==S_FALSE)
		return TRUE;

	//Get the next line 
	if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER)) != S_OK)
	{
		odtLog << "ERROR:  Unable to find URLs in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
		return FALSE;
	}
	
	while(hr==S_OK)
	{
		if(pszStart)
		{
			//If were at the next section, were done here...
			if(strncmp(pszStart+1, "[QUERY]", 6)==0)
				return TRUE;

			//Try to find the beginning of 
			pszStart = strstr(m_pvInput, START_OF_TYPE);
			pszEndData = strstr(m_pvInput, START_OF_DATA);
			pszEnd = strstr(m_pvInput, END_OF_DATA);

			//Error checking
			if(pszStart==NULL || pszEnd==NULL)
			{
				odtLog << "ERROR:  Unable to find Start or End of URLInfo in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}

			pszStart++;
			
			if(strncmp(pszStart, "URL_DSO", (size_t)(pszEndData-pszStart))==0)
				eInterface = DATASOURCE_INTERFACE;
			else if(strncmp(pszStart, "URL_SESSION", (size_t)(pszEndData-pszStart))==0)
				eInterface = SESSION_INTERFACE;
			else if(strncmp(pszStart, "URL_ROW", (size_t)(pszEndData-pszStart))==0)
				eInterface = ROW_INTERFACE;
			else if(strncmp(pszStart, "URL_ROWSET", (size_t)(pszEndData-pszStart))==0)
				eInterface = ROWSET_INTERFACE;
			else if(strncmp(pszStart, "URL_STREAM", (size_t)(pszEndData-pszStart))==0)
				eInterface = STREAM_INTERFACE;
			else if(strncmp(pszStart, "URL_COMMAND", (size_t)(pszEndData-pszStart))==0)
				eInterface = COMMAND_INTERFACE;
			else
				return FALSE;

			//Now try to find the Query value in between ()
			pszStart = strstr(pszStart, START_OF_DATA);
			pszEndData = strstr(pszStart, END_OF_DATA);

			//Error checking
			if(pszStart==NULL || pszEndData==NULL || pszEndData > pszEnd)
			{
				odtLog << "ERROR:  Unable to find URL in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}

			//Obtain the actual Query Name
			pszStart++;
			pszEndData[0] = '\0';

			ASSERT(eInterface < (LONG)m_cURLs);
			PROVIDER_FREE(m_rgURLs[eInterface]);

			//Copy  and Convert into our WCHAR buffers...
			m_rgURLs[eInterface] = NULL;
			if(pszEndData >= pszStart)
			{
				m_rgURLs[eInterface] = (WCHAR*)PROVIDER_ALLOC((strlen(pszStart)+1)*sizeof(WCHAR));
				ConvertToWCHAR(pszStart, m_rgURLs[eInterface], (int)(strlen(pszStart)+1) );
			}
		}

		//Get the NextLine
		if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER)) != S_OK)
		{
			odtLog << "ERROR:  Unable to find the [URL] section in the INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return FALSE;
		}
		
		pszStart = strstr(m_pvInput, START_OF_TYPE);
	}

	return  FALSE;
}


WCHAR* CParseInitFile::GetURL(EINTERFACE eInterface)
{
	ASSERT(m_rgURLs);
	ASSERT(eInterface >=0 && eInterface < INVALID_INTERFACE);

	return m_rgURLs[eInterface];
}

BOOL CParseInitFile::SetURL(EINTERFACE eInterface, WCHAR* pwszURL)
{
	ASSERT(m_rgURLs);
	if(eInterface<0 || eInterface>=INVALID_INTERFACE || !pwszURL)
		return FALSE;
	if(wcslen(pwszURL) < 2)
		return FALSE;

	//If it already exists, donot overwrite it.
	if(m_rgURLs[eInterface] && wcslen(m_rgURLs[eInterface])>1)
		return FALSE;

	PROVIDER_FREE(m_rgURLs[eInterface]);
	m_rgURLs[eInterface] = wcsDuplicate(pwszURL);

	if(m_rgURLs[eInterface] && wcslen(m_rgURLs[eInterface])>1)
		return TRUE;
	else
		return FALSE;
}

// This function was added to enable over writing of URLs once Confprov is already
// initialized. In cases where the tests don't run against ini files using conformance 
// provider - this function is used to Reinitialize the Conformance Provider with test
// constructed URLs. Added on 03/30/2001

BOOL CParseInitFile::OverwriteURL(EINTERFACE eInterface, WCHAR* pwszURL)
{
	ASSERT(m_rgURLs);
	if(eInterface<0 || eInterface>=INVALID_INTERFACE || !pwszURL)
		return FALSE;
	if(wcslen(pwszURL) < 2)
		return FALSE;

	PROVIDER_FREE(m_rgURLs[eInterface]);
	m_rgURLs[eInterface] = wcsDuplicate(pwszURL);

	if(m_rgURLs[eInterface] && wcslen(m_rgURLs[eInterface])>1)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Tokenize the Query info
//
// @rdesc BOOL
//      @flag TRUE | Parsing yielded no Error
//
BOOL CParseInitFile::ParseQueryInfo()
{
	CHAR*	pszStart = m_pvInput;
	CHAR*	pszEnd = m_pvInput;
	CHAR*	pszEndData = m_pvInput;
	LONG	lItem;
	HRESULT hr = S_OK;
	
	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseQueryInfo.\n");

	//Skip over any lines, until the [QUERY] section is reached...
	if(FAILED(hr = FindSection("[QUERY]")))
		return FALSE;

	//Get the NextLine {SELECT_(query)}
	if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER)) != S_OK)
	{
		odtLog << "ERROR:  Unable to find Queries in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
		return FALSE;
	}
	
	while(hr==S_OK)
	{
		if(pszStart)
		{
			//If were at the next section, were done here...
			if(strncmp(pszStart+1, "[COLUMN]", 7)==0)
				return TRUE;

			//Try to find the beginning of {
			pszStart = strstr(m_pvInput, START_OF_TYPE);
			pszEndData = strstr(m_pvInput, START_OF_DATA);
			pszEnd = strstr(m_pvInput, END_OF_DATA);

			//Error checking
			if(pszStart==NULL || pszEnd==NULL)
			{
				odtLog << "ERROR:  Unable to find Start or End of SQLEnumIndentifier in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}

			//Otherwise we have real data inbetween {SQLEnumIdentifier(
			//Try to find this name in our list of EnumNames
			lItem = -1;
			pszStart++;
			
			//Obtain SQLToken array
			NAMEMAP* rgSQLTokens = NULL;
			ULONG cSQLTokens = GetSQLTokenMap(&rgSQLTokens);
			
			//Loop over all SQL tokens
			for(ULONG i=0; i<cSQLTokens; i++)
			{
				//NOTE: We have to use "strncmp" since the start/end doesn't have a NULL terminator
				//but we don't want DISTINCTCOLLIST to tag on DISTINCTCOLLISTORDERBY, (a super set)
				//so we need to do more than just compare the first n-characters.
				if(strncmp(pszStart, rgSQLTokens[i].pszName, (size_t)(pszEndData-pszStart))==0
					&& strlen(rgSQLTokens[i].pszName)==(size_t)(pszEndData-pszStart))
				{
					lItem = rgSQLTokens[i].lItem;
					break;
				}
			}

			//Error checking
			if(lItem == -1)
			{
				odtLog << "ERROR:  Unknown SQLEnumIndentifier [" << pszStart << "] in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}

			//Now try to find the Query value in between ()
			pszStart = strstr(pszStart, START_OF_DATA);
			pszEndData = strstr(pszStart, END_OF_DATA);

			//Error checking
			if(pszStart==NULL || pszEndData==NULL || pszEndData > pszEnd)
			{
				odtLog << "ERROR:  Unable to find DefaultQuery in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}

			//Obtain the actual Query Name
			pszStart++;
			pszEndData[0] = '\0';

			//Add This Query to our list...
			//Bascially our list is nothing more than an array as large as the EQUERY enum
			//Each Enum Value indexes into the array.  So we can do quick lookups, such as
			//m_rgpwszQueryNames[SELECT_ALLFROMTBL] == the string for SELECT_ALLFROMTBL...
			ASSERT(lItem < (LONG)m_cQueryNames);
			ASSERT(m_rgQueryNames[lItem] == NULL);

			//Copy  and Convert into our WCHAR buffers...
			m_rgQueryNames[lItem] = NULL;
			if(pszEndData >= pszStart)
			{
				m_rgQueryNames[lItem] = (WCHAR*)PROVIDER_ALLOC((strlen(pszStart)+1)*sizeof(WCHAR));
				ConvertToWCHAR(pszStart, m_rgQueryNames[lItem], (int)(strlen(pszStart)+1));
			}
		}

		//Get the NextLine
		if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER)) != S_OK)
		{
			odtLog << "ERROR:  Unable to find the [COLUMN] section in the INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return FALSE;
		}
		
		pszStart = strstr(m_pvInput, START_OF_TYPE);
	}

	return  FALSE;
}


WCHAR* CParseInitFile::GetQuery(EQUERY eQuery)
{
	ASSERT(m_rgQueryNames);
	ASSERT((ULONG)eQuery < m_cQueryNames);

	return m_rgQueryNames[eQuery];
}


//--------------------------------------------------------------------
// @mfunc Retrieve the data associated with a particular column. 
//
//
// @rdesc BOOL
//      @flag TRUE | Succeeded
//      @flag FALSE | Failed
//
BOOL CParseInitFile::ParseColumnInfo()
{
    HRESULT hr = S_OK;
	CHAR* pszStart = NULL;
	
	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseColumnInfo.\n");

	// If Column data has not been retrieved,
    if(m_ColData.IsEmpty())
	{
		//Skip over any lines, until the [COLUMN] section is reached...
		if(FAILED(hr = FindSection("[COLUMN]")))
			return FALSE;

		//Get the NextLine {ColName(iOrdinal, TYPE, ulColumnSize, bPrecision, bScale, dwFlags)} 
		if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER))!=S_OK)
		{
			odtLog << "ERROR:  Unable to find Columns in INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return FALSE;
		}

		//Now parse the Columns
		m_ColData.RemoveAll();
			    
		// Parse the records
		while(hr==S_OK)
		{	
			pszStart = strstr(m_pvInput, START_OF_TYPE);
			if(pszStart)
			{
				// if we have reached [DATA] part bail out
				pszStart++;
				if(strncmp(pszStart, szDATA, 6) ==0)
					return TRUE;
			
				// parse the column metadata info
				if(!GetColumns(pszStart))
					break;
			}
			
			//Retrieve the next row
			if((hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER))!=S_OK)
			{
				odtLog << "ERROR:  Unable to finding ColumnInfo for Column " << m_ColData.GetCount() << " in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}
		}
    }

	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc	Obtain the offsets into the file for each row
//			
//			
//
// @rdesc BOOL
//      @flag TRUE | Got the offsets, 
//      @flag FALSE | Could not obtain all the necessary info
//
BOOL CParseInitFile::ParseDataInfo()
{
	HRESULT hr = S_OK;
	m_cRows = 0;
	
	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseDataInfo.\n");

	//Skip over any lines, until the [DATA] section is reached...
	if(FAILED(hr = FindSection("[DATA]")))
		return FALSE;
    
	while(hr==S_OK)
    {
        //Check if we are provided with more rows than 
		//our current array can hold...
		if(m_cRows >= m_cRowOffsets || m_rgRowOffsets == NULL)
		{
			//Realloc the array
			m_cRowOffsets += MAX_ROW;
			m_rgRowOffsets = (DBLENGTH*)PROVIDER_REALLOC(m_rgRowOffsets, m_cRowOffsets*sizeof(DBLENGTH));
			if(m_rgRowOffsets == NULL)
				return FALSE;
		}
		
		//Save the Current Row Offset, so we can get this row data again
		//(ignoring null data)
        if(m_pvInput[0])
			m_rgRowOffsets[m_cRows] = tellg();
						
		//Get the NextLine of input
		//Note we allow S_FALSE to be returned for end of file...
		if(FAILED(hr = GetNextLine(m_pvInput, MAX_INPUT_BUFFER)))
		{
			odtLog << "ERROR:  Unable to find RowData for Row " << m_cRows+1 << " in INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return FALSE;
		}

		//We successfully retrieved this row...
		if(hr == S_OK)
			m_cRows++;
    }

	return TRUE;
}



//--------------------------------------------------------------------
// @mfunc Tokenize the column info
//
// @rdesc BOOL
//      @flag TRUE | Parsing yielded no Error
//
BOOL CParseInitFile::GetColumns
(
	CHAR*  pszInput
)
{
	ASSERT(pszInput);
	WCHAR*	pwszColName = NULL;
	WCHAR*	pwszColIDName = NULL;
	WCHAR*	pwszTempGuid = NULL;
	WCHAR*	pwszPrefix = NULL;
	WCHAR*	pwszSuffix = NULL;
	CCol	NewCol;
	DBID	dbidCol;
	GUID*	pGuid = NULL;
	GUID	guid;
	HRESULT hr = S_OK;
	
	CHAR*	pszStart	= pszInput;
	CHAR*	pszEndData	= strchr(pszStart, ',');
	CHAR*	pszEnd		= pszStart + strlen(pszStart);

	TRACE_CALL(L"PRIVLIB: CParseInitFile::GetColumns.\n");

	memset(&dbidCol, 0, sizeof(DBID));

	//Initialize NewCol to default values
	InitializeNewCol(NewCol);
	
	//Error checking
	if(pszStart==NULL || pszEndData==NULL || pszEndData > pszEnd)
	{
		odtLog << "ERROR:  Unable to find Column Name, Seperators, or End Markers in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
		return FALSE;
	}

	//Loop over all column info		
	for(ULONG i=0; i<=COL_SUFFIX; i++)
	{
		//Place the NULL Terminator
		pszEndData[0] = '\0';
		
		//Fill in Appropiate column info
		if(i==COL_NAME)
		{
			//Column Names can be NULL
			PROVIDER_FREE(pwszColName);
			if(pszStart != pszEndData)
			{
				//Convert to WCHAR
				pwszColName = ConvertToWCHAR(pszStart);
			}
			
			//Store the column name
			NewCol.SetColName(pwszColName);
		}
		else if(i==COL_NUMBER)
		{
			// column num
			NewCol.SetColNum(strtoul(pszStart, NULL, 10));
		}
		else if(i==COL_DBTYPE)
		{
			// column DBTYPE
			NewCol.SetProviderType(ConvertToDBTYPE(pszStart));
		}
		else if(i==COL_SIZE)
		{
			DBLENGTH ulValue = strtoul(pszStart, NULL, 10);
			
			//set the column size
			NewCol.SetColumnSize(ulValue);
		}
		else if(i==COL_PRECISION)
		{
			ULONG ulValue = strtoul(pszStart, NULL, 10);

			// set the actual column precision
			NewCol.SetPrecision((BYTE)ulValue);
		}			
		else if(i==COL_SCALE)
		{
			// column scale
			NewCol.SetScale((BYTE)strtol(pszStart, NULL, 10));
		}
		else if(i==COL_FLAGS)
		{
			// column Flags
			ParseFlags(pszStart, NewCol);
		}
		else if(i==COL_PREFIX)
		{
			//if the ini has null here (which is marked as '(null)' then set 
			//it the way it should be
			if (strcmp("(null)",pszStart))
			{
				// column Flags
				PROVIDER_FREE(pwszPrefix)
				pwszPrefix = ConvertToWCHAR(pszStart);
				NewCol.SetPrefix(pwszPrefix);
			}
			else
			{
				// column Flags
				PROVIDER_FREE(pwszPrefix)
				pwszPrefix = ConvertToWCHAR("");
				NewCol.SetPrefix(pwszPrefix);
			}
		}
		else if(i==COL_SUFFIX)
		{
			//if the ini has null here (which is marked as '(null)' then set 
			//it the way it should be
			if (strcmp("(null)",pszStart))
			{
				// column Flags
				PROVIDER_FREE(pwszSuffix)
				pwszSuffix = ConvertToWCHAR(pszStart);
				NewCol.SetSuffix(pwszSuffix);
			}
			else
			{
				// column Flags
				PROVIDER_FREE(pwszSuffix)
				pwszSuffix = ConvertToWCHAR("");
				NewCol.SetSuffix(pwszSuffix);
			}
		}
		else if((i==COL_IDKIND) && (pszStart != pszEndData))
		{
			dbidCol.eKind = strtoul(pszStart, NULL, 10);
		}
		else if((i==COL_IDGUID) && (pszStart != pszEndData))
		{
			if(dbidCol.eKind == DBKIND_PGUID_NAME ||
				dbidCol.eKind == DBKIND_PGUID_PROPID)
			{
				SAFE_ALLOC(pGuid, GUID, 1);
				PROVIDER_FREE(pwszTempGuid);
				pwszTempGuid = ConvertToWCHAR(pszStart);
				hr = CLSIDFromString(pwszTempGuid, pGuid);
				dbidCol.uGuid.pguid = pGuid;
			}
			else
			{
				PROVIDER_FREE(pwszTempGuid);
				pwszTempGuid = ConvertToWCHAR(pszStart);
				hr = CLSIDFromString(pwszTempGuid, &guid);
				dbidCol.uGuid.guid = guid;
			}
		}
		else if((i==COL_IDNAME) && (pszStart != pszEndData))
		{
			if(dbidCol.eKind == DBKIND_PGUID_NAME ||
				dbidCol.eKind == DBKIND_GUID_NAME ||
				dbidCol.eKind == DBKIND_NAME )
			{
				PROVIDER_FREE(pwszColIDName);
				pwszColIDName = ConvertToWCHAR(pszStart);
				dbidCol.uName.pwszName = pwszColIDName;
			}
			else
			{
				dbidCol.uName.ulPropid = strtoul(pszStart, NULL, 10);
			}
		}

		//Find the next Info Piece
		if(i<COL_SUFFIX)
		{
			pszStart	= pszEndData+1;
			pszEndData	= strchr(pszStart, ',');
			if((i==COL_FLAGS) && (!pszEndData))
				goto END;
			if(pszEndData == NULL)
				pszEndData = pszEnd;

			//Error checking
			if(pszStart==NULL || pszEndData==NULL || pszEndData > pszEnd)
			{
				odtLog << "ERROR:  Unable to find Column Name, Seperators, or End Markers in INI <File:" << m_pszFileName << ">" << ENDL;
				odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
				return FALSE;
			}
		}
	}

END:
	//Add the new columnInfo at the end fo the list
	NewCol.SetColID(&dbidCol);
	m_ColData.AddTail(NewCol);

CLEANUP:
	PROVIDER_FREE(pwszColName);
	PROVIDER_FREE(pGuid);
	PROVIDER_FREE(pwszColIDName);
	PROVIDER_FREE(pwszTempGuid);
	PROVIDER_FREE(pwszPrefix);
	PROVIDER_FREE(pwszSuffix);
	return  TRUE;
}





//--------------------------------------------------------------------
// @mfunc	Convert String to the DBTYPE
//			
//			
//
// @rdesc DBTYPE
//      @wType | 
//
DBTYPE CParseInitFile::ConvertToDBTYPE(LPSTR pszDBTYPE)
{
	ASSERT(pszDBTYPE);
	DBTYPE wType = 0;

	//NOTE:
	//We have to be a little Careful about using "strstr" 
	//since DBTYPE_DBTIME and DBTYPE_DBTIMESTAMP collide.
	//The trick is that our dump utility outputs a space after each type...
	
	if(strstr(pszDBTYPE,"DBTYPE_EMPTY "))
		wType = DBTYPE_EMPTY;
	else if(strstr(pszDBTYPE,"DBTYPE_NULL "))
		wType = DBTYPE_NULL;
	else if(strstr(pszDBTYPE,"DBTYPE_I2 "))
		wType = DBTYPE_I2;
	else if(strstr(pszDBTYPE,"DBTYPE_I4 "))
		wType = DBTYPE_I4;
	else if(strstr(pszDBTYPE,"DBTYPE_R4 "))
		wType = DBTYPE_R4;
	else if(strstr(pszDBTYPE,"DBTYPE_R8 "))
		wType = DBTYPE_R8;
	else if(strstr(pszDBTYPE,"DBTYPE_CY "))
		wType = DBTYPE_CY;
	else if(strstr(pszDBTYPE,"DBTYPE_DATE "))
		wType = DBTYPE_DATE;
	else if(strstr(pszDBTYPE,"DBTYPE_BSTR "))
		wType = DBTYPE_BSTR;
	else if(strstr(pszDBTYPE,"DBTYPE_IDISPATCH "))
		wType = DBTYPE_IDISPATCH;
	else if(strstr(pszDBTYPE,"DBTYPE_ERROR "))
		wType = DBTYPE_ERROR;
	else if(strstr(pszDBTYPE,"DBTYPE_BOOL "))
		wType = DBTYPE_BOOL;
	else if(strstr(pszDBTYPE,"DBTYPE_VARIANT "))
		wType = DBTYPE_VARIANT;
	else if(strstr(pszDBTYPE,"DBTYPE_IUNKNOWN "))
		wType = DBTYPE_IUNKNOWN;
	else if(strstr(pszDBTYPE,"DBTYPE_DECIMAL "))
		wType = DBTYPE_DECIMAL;
	else if(strstr(pszDBTYPE,"DBTYPE_UI1 "))
		wType = DBTYPE_UI1;
	else if(strstr(pszDBTYPE,"DBTYPE_I1 "))
		wType = DBTYPE_I1;
	else if(strstr(pszDBTYPE,"DBTYPE_UI2 "))
		wType = DBTYPE_UI2;
	else if(strstr(pszDBTYPE,"DBTYPE_UI4 "))
		wType = DBTYPE_UI4;
	else if(strstr(pszDBTYPE,"DBTYPE_I8 "))
		wType = DBTYPE_I8;
	else if(strstr(pszDBTYPE,"DBTYPE_UI8 "))
		wType = DBTYPE_UI8;
	else if(strstr(pszDBTYPE,"DBTYPE_GUID "))
		wType = DBTYPE_GUID;
	else if(strstr(pszDBTYPE,"DBTYPE_BYTES "))
		wType = DBTYPE_BYTES;
	else if(strstr(pszDBTYPE,"DBTYPE_STR "))
		wType = DBTYPE_STR;
	else if(strstr(pszDBTYPE,"DBTYPE_WSTR "))
		wType = DBTYPE_WSTR;
	else if(strstr(pszDBTYPE,"DBTYPE_NUMERIC "))
		wType = DBTYPE_NUMERIC;
	else if(strstr(pszDBTYPE,"DBTYPE_DBDATE "))
		wType = DBTYPE_DBDATE;
	else if(strstr(pszDBTYPE,"DBTYPE_DBTIME "))
		wType = DBTYPE_DBTIME;
	else if(strstr(pszDBTYPE,"DBTYPE_DBTIMESTAMP "))
		wType = DBTYPE_DBTIMESTAMP;
	else if(strstr(pszDBTYPE,"DBTYPE_HCHAPTER "))
		wType = DBTYPE_HCHAPTER;
	else if(strstr(pszDBTYPE,"DBTYPE_VARNUMERIC "))
		wType = DBTYPE_VARNUMERIC;
	else if(strstr(pszDBTYPE,"DBTYPE_FILETIME "))
		wType = DBTYPE_FILETIME;
	else if(strstr(pszDBTYPE,"DBTYPE_PROPVARIANT "))
		wType = DBTYPE_PROPVARIANT;
	
	if(strstr(pszDBTYPE,"DBTYPE_BYREF "))
		wType |= DBTYPE_BYREF;
	else if(strstr(pszDBTYPE,"DBTYPE_ARRAY "))
		wType |= DBTYPE_ARRAY;
	else if(strstr(pszDBTYPE,"DBTYPE_VECTOR "))
		wType |= DBTYPE_VECTOR;
	
	return wType;
}


//--------------------------------------------------------------------
// @mfunc	Convert String to the BYTE
//			
//			
//
// @rdesc BYTE
//      @BYTE | 
//
BYTE CParseInitFile::ConvertToByte(LPSTR pszPrecision)
{
	ASSERT(pszPrecision);
	LONG lprec =0;

	lprec = atol(pszPrecision);
	ASSERT(lprec >= 0 && lprec < 256);

	return BYTE(lprec);
}


//--------------------------------------------------------------------
// @mfunc	Convert string user passed in to initilaize the col class
//			
//			
//
// @rdesc ParseFlags
//      
//      @void |
//
void CParseInitFile::ParseFlags(LPSTR pszColumnFlags, CCol &NewCol)
{
	ASSERT(pszColumnFlags);

	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseFlags.\n");
	
	//TODO - all of these flags are not needed - just here for the time being until 
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_MAYBENULL"))
		NewCol.SetNullable(FALSE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_ISNULLABLE"))
		NewCol.SetNullable(TRUE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_ISLONG"))
		NewCol.SetIsLong(TRUE);
	if(strstr(pszColumnFlags,"IS_AUTOINC"))
		NewCol.SetAutoInc(TRUE);
	if(strstr(pszColumnFlags,"IS_UNSIGNED"))
		NewCol.SetUnsigned(TRUE);
	if(strstr(pszColumnFlags,"IS_SIGNED"))
		NewCol.SetUnsigned(FALSE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_WRITE"))
		NewCol.SetUpdateable(TRUE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_WRITEUNKNOWN"))
		NewCol.SetUpdateable(TRUE);
	if(strstr(pszColumnFlags,"IS_CASESENSITIVE"))
		NewCol.SetCaseSensitive(TRUE);
	if(strstr(pszColumnFlags,"IS_NOTCASESENSITIVE"))
		NewCol.SetCaseSensitive(FALSE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_ISFIXEDLENGTH"))
		NewCol.SetIsFixedLength(TRUE);
	if(strstr(pszColumnFlags,"DBCOLUMNFLAGS_ISCHAPTER"))
		NewCol.SetIsChapter(TRUE);
}
//--------------------------------------------------------------------
// @mfunc	Initializes col to default values
//			
//			
//
// @rdesc InitializeNewCol
//      
//      @void | 
//
void CParseInitFile::InitializeNewCol(CCol &NewCol)
{
	TRACE_CALL(L"PRIVLIB: CParseInitFile::InitializeNewCol.\n");

	//Initialize to default values
	NewCol.SetNullable(UNKNOWN);
	NewCol.SetAutoInc(FALSE);
	NewCol.SetUnsigned(TRUE);
	NewCol.SetCaseSensitive(UNKNOWN);
	NewCol.SetIsLong(FALSE);
	NewCol.SetUpdateable(FALSE);
	NewCol.SetIsFixedLength(FALSE);

	// we do not prompt the user for this information
	NewCol.SetPrefix(NULL);
	NewCol.SetSuffix(NULL);
	NewCol.SetProviderTypeName(NULL);
	NewCol.SetCreateParams(NULL);
	
}

//--------------------------------------------------------------------
// @mfunc Fetch row data into the internal data buffer
//
// @rdesc HRESULT
//      @flag S_OK    | Row Retrieved successfully
//      @flag S_FALSE | End of Result Set
//      @flag E_FAIL  | Row could not be retrieved
//
HRESULT CParseInitFile::FetchRow(DBCOUNTITEM iRow)
{
	TRACE_CALL(L"PRIVLIB: CParseInitFile::FetchRow.\n");

	//NOTE:  iRow is 1-based.
    //Validate arguments
    if(iRow == 0 || iRow > m_cRows)
        return E_FAIL;

	//Make sure we have the file open!
	if(!is_open())
		return E_FAIL;

    //Seek to the current Row offset
    seekg((LONG)GetRowOffset(iRow)); //64bit TODO - 

    // Retrieve the column names record
	return GetNextLine(m_pvInput, MAX_INPUT_BUFFER);
}


//--------------------------------------------------------------------
// @mfunc Tokenize the Data values and retursn the column record
//
// @rdesc HRESULT
//      @flag S_OK | Parsing yielded no Error
//      @flag E_FAIL | Data value could not be parsed or stored
//
HRESULT CParseInitFile::ParseRowValues(DBORDINAL ulColumn, CHAR** ppszOffset, DBLENGTH* pdwLength, DBTYPE *pwType)
{
    ASSERT( m_pvInput );
	ASSERT( ppszOffset);
	ASSERT( pdwLength);

	CHAR*	pszStartType = strstr(m_pvInput, START_OF_TYPE);
	CHAR*	pszStartData = m_pvInput;
	CHAR*	pszEndData	= m_pvInput;
	CHAR	pszDBTYPE[100];
	DBTYPE	wType;

	*ppszOffset = NULL;
	*pdwLength = 0;

	TRACE_CALL(L"PRIVLIB: CParseInitFile::ParseRowValues.\n");

	//Format:
	//START_OF_TYPE type START_OF_DATA data END_OF_DATA...

	//NULL column	:  START_OF_TYPE START_OF_DATA END_OF_DATA- return S_FALSE
	//VARIANT column:  START_OF_TYPE type START_OF_DATA data END_OF_DATA- return S_OK
	//Normal column :  START_OF_TYPE START_OF_DATA data END_OF_DATA- return S_OK

	//We currently don't store the data for the bookmark column.
	if(ulColumn == 0)
		return DB_E_NOTFOUND;

	//Just Find the start of the data (as quickly as possible...)
	for(DBORDINAL i=1; i<ulColumn && pszStartType; i++)
	{
		if(i == ulColumn)
			break;

		//Move the next column
		pszStartType = strstr(pszStartType+1, START_OF_TYPE);
	}

	if(pszStartType == NULL)
	{
		odtLog << "ERROR:  Unable to find the Start of the Data for Column " << ulColumn << " in INI <File:" << m_pszFileName << ">" << ENDL;
		odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
		return E_FAIL;
	}
	else
	{
		//We now have the string starting at the correct column
		pszStartData = strstr(pszStartType, START_OF_DATA);
		pszEndData = strstr(pszStartType, END_OF_DATA);

		//Error checking
		if(pszStartType==NULL || pszStartData==NULL || pszEndData==NULL)
		{
			odtLog << "ERROR:  Missing Start or End of Data for Column " << ulColumn << " in INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return E_FAIL;
		}

		//More Error checking
		if(pszStartType > pszStartData || pszStartData > pszEndData)
		{
			odtLog << "ERROR:  Mismatched Start or End of Data for Column " << ulColumn << " in INI <File:" << m_pszFileName << ">" << ENDL;
			odtLog << "ERROR:  Make sure your using a correctly generated INI File from TableDump.exe" << ENDL;
			return E_FAIL;
		}

		DWORD dwTypeLen = (DWORD)(pszStartData - pszStartType - 1);
		if(dwTypeLen)
		{
			strncpy(pszDBTYPE, pszStartType + 1, dwTypeLen);
			pszDBTYPE[dwTypeLen] = '\0';
			strcat(pszDBTYPE, " ");
		}

		//Check for (null)
		//NULL is defined as START_OF_TYPE START_OF_DATA END_OF_DATA
		//There is no subtype and no data...
		if(dwTypeLen==0 && pszStartData+1 == pszEndData)
			return S_FALSE;

		//Otherwise we have real data in between quotes
		//Increment to the actual data (past the ")
		*ppszOffset = ++pszStartData;
		*pdwLength = pszEndData - pszStartData;
		
		if(dwTypeLen)
		{
			wType = ConvertToDBTYPE(pszDBTYPE);
			if (pwType)	
				*pwType = wType;

			// Handle VT_NULL variants
			if (wType == DBTYPE_NULL)
				return S_FALSE;
		}
	}
	
	//Were done!
	return S_OK;
}

//--------------------------------------------------------------------
// @mfunc Tokenize the Data values and retursn the column record
//
// @rdesc HRESULT
//      @flag S_OK | Parsing yielded no Error
//      @flag E_FAIL | Data value could not be parsed or stored
//
HRESULT CParseInitFile::GetData(DBCOUNTITEM ulRow,DBORDINAL ulColumn, WCHAR* pwszData, DBTYPE *pwType)
{
	ASSERT(pwszData);

	//NOTE: Since we are moving the file pointer here, we need to block so that
	//our data access to the low level file is atomic.  Otherwise threading variations
	//comparing data will get bogus data since the starting location is not correct...
	CAutoBlock autoBlock(m_csFile.GetCS());

	DBLENGTH dwLength = 0;
	CHAR* pszOffset = NULL;
	HRESULT hr = S_OK;

	TRACE_CALL(L"PRIVLIB: CParseInitFile::GetData.\n");

	if(FAILED(hr = FetchRow(ulRow)))
		goto CLEANUP;

	if(FAILED(hr = ParseRowValues(ulColumn, &pszOffset, &dwLength, pwType)))
		goto CLEANUP;

	//Check for (null) data
	if(hr == S_FALSE)
		return hr; //Indicate NULL

	if(dwLength > MAXDATALEN)
		dwLength = MAXDATALEN;

	if(dwLength)
	{
		//Convert to WCHAR.
		//NOTE: ParseRowValues returns the actual number of bytes obtained.  We need to 
		//know the actual number of characters which may be different for MBCS
		dwLength = MultiByteToWideChar(CP_ACP, 0, pszOffset, (INT)dwLength, pwszData, MAXDATALEN);
		if(!dwLength)
			TESTC_(hr = E_FAIL, S_OK);
	}

	//Add NULL terminator
	pwszData[dwLength] = L'\0';

CLEANUP:
	return hr;
}
