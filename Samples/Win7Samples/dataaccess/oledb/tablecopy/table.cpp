//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module TABLE.H
//
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "winmain.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"
#include "wizard.h"
#include "progress.h"


//////////////////////////////////////////////////////////////////////////////
// Defines / Macros
//
//////////////////////////////////////////////////////////////////////////////
#define NO_MATCH					0x0000
#define MATCH_EXACT					0x0001
#define MATCH_TYPE					0x0002
#define MATCH_SIZE					0x0004
#define MATCH_DEFAULT				0x0008



/////////////////////////////////////////////////////////////////
// CTable::CTable
//
/////////////////////////////////////////////////////////////////
CTable::CTable(CWizard* pCWizard)
{
    ASSERT(pCWizard);

    m_wszIDQuote[0]			= EOL;		
    m_wszIDSeperator[0]		= EOL;		

    //TableInfo
    memset(&m_TableInfo, 0, sizeof(TABLEINFO));
    m_wszQualTableName[0]	= EOL;

    //IndexInfo
    m_cIndexes		= 0;		// Count of indexes
    m_rgIndexInfo	= NULL;		// Index information

    //ColumnInfo
    m_cColumns		= 0;		// Count of columns
    m_rgColDesc		= NULL;		// Column information	

    //DataSource
    m_pCDataSource  = new CDataSource;
    m_pCWizard		= pCWizard;			// Back pointer to Windowing class

    //Rowset
    m_pIAccessor	= NULL;
    m_pIRowset		= NULL;
}


/////////////////////////////////////////////////////////////////
// CTable::~CTable
//
/////////////////////////////////////////////////////////////////
CTable::~CTable()
{
    delete m_pCDataSource;
    
    SAFE_FREE(m_rgIndexInfo);
    SAFE_FREE(m_rgColDesc);

    //Rowset
    SAFE_RELEASE(m_pIAccessor);
    SAFE_RELEASE(m_pIRowset);
}


/////////////////////////////////////////////////////////////////
// BOOL CTable::Connect
//
/////////////////////////////////////////////////////////////////
BOOL CTable::Connect(HWND hWnd, CDataSource* pCDataSource)
{
    ASSERT(m_pCDataSource);
    
    HRESULT hr = m_pCDataSource->Connect(hWnd, m_pCDataSource);
    if (SUCCEEDED(hr) && (REGDB_E_CLASSNOTREG != hr))
    {
        //Get LiteralInfo for this table
        GetLiteralInfo();
        return TRUE;
    }

    return FALSE;
}


/////////////////////////////////////////////////////////////////
// BOOL CTable::IsConnected
//
/////////////////////////////////////////////////////////////////
BOOL CTable::IsConnected()
{
    ASSERT(m_pCDataSource);
    return m_pCDataSource->IsConnected();
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetLiteralInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetLiteralInfo()
{
    ASSERT(m_pCDataSource);
    ASSERT(m_pCDataSource->m_pIDBInitialize);

    HRESULT hr;
    
    const static ULONG cLiterals = 2;
    const static DBLITERAL rgLiterals[cLiterals] = {DBLITERAL_QUOTE, DBLITERAL_CATALOG_SEPARATOR};
    
    IDBInfo* pIDBInfo = NULL;

    ULONG	cLiteralInfo = 0;
    DBLITERALINFO* rgLiteralInfo = NULL;
    WCHAR* pwszCharBuffer = NULL;

    //Reset Info
    m_wszIDQuote[0] = EOL;
    m_wszIDSeperator[0] = EOL;
    
    //Obtain IDBInfo interface
    //Some providers may not support IDBInfo so don't display dialog
    QTESTC(hr = m_pCDataSource->m_pIDBInitialize->QueryInterface(IID_IDBInfo, (void **)&pIDBInfo));
        
    //GetLiteralInfo
    //Can return an error for unsupported literals
    hr = pIDBInfo->GetLiteralInfo(cLiterals, rgLiterals, &cLiteralInfo, &rgLiteralInfo, &pwszCharBuffer);
                
    //DBLITERAL_QUOTE
    if(rgLiteralInfo && rgLiteralInfo[0].fSupported) 
        StringCchCopyW(m_wszIDQuote, sizeof(m_wszIDQuote)/sizeof(WCHAR), rgLiteralInfo[0].pwszLiteralValue);

    //DBLITERAL_CATALOG_SEPARATOR
    if(rgLiteralInfo && rgLiteralInfo[1].fSupported) 
        StringCchCopyW(m_wszIDSeperator, sizeof(m_wszIDSeperator)/sizeof(WCHAR), rgLiteralInfo[1].pwszLiteralValue);


CLEANUP:
    SAFE_RELEASE(pIDBInfo);
    SAFE_FREE(rgLiteralInfo);
    SAFE_FREE(pwszCharBuffer);
    return hr;
}



/////////////////////////////////////////////////////////////////
// BOOL CTable::GetQuotedID
//
/////////////////////////////////////////////////////////////////
BOOL CTable::GetQuotedID(WCHAR* pwszOutBuff, size_t cwchOutBuff, WCHAR* pwszInBuff)
{
    WCHAR  pwsz[MAX_NAME_LEN*2];
    WCHAR* pwszItr = pwsz;
    size_t cwchItr = sizeof(pwsz)/sizeof(WCHAR);

    //size_t cQuoteLen = wcslen(m_wszIDQuote);
    size_t cSepLen	 = wcslen(m_wszIDSeperator);
    //size_t cPeriodLen= wcslen(wsz_PERIOD);

    //No-op case
    //If the provider doesn't have a delimiter, then just use input name.
    if(m_wszIDQuote[0] == L'\0')
    {
        StringCchCopyW(pwszOutBuff, cwchOutBuff, pwszInBuff);
        return (TRUE);
    }

    //Put on front delimeter
    StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);

    //Copy from Source to Temp
    while(*pwszInBuff!=EOL)
    {
        //If we have a seperator char, we need to quote both pieces
        if(wcsncmp(pwszInBuff, m_wszIDSeperator, cSepLen)==0)
        {
            // "nstl@odbc.authors -> "nstl"@"odbc.authors"
            StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);		//Ending Quote
            StringCchCopyExW(pwszItr, cwchItr, m_wszIDSeperator, &pwszItr, &cwchItr, 0);	//Seperator
            StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);		//Beginning Quote
        }
        else if(*pwszInBuff == L'.')
        {
            // "nstl@odbc.authors -> "nstl@odbc"."authors"
            StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);		//Ending Quote
            StringCchCopyExW(pwszItr, cwchItr, wsz_PERIOD, &pwszItr, &cwchItr, 0);			//Period
            StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);		//Beginning Quote
        }
        else
        {
            if (cwchItr)
            {
                *pwszItr = *pwszInBuff;			//Actual Character
                pwszItr++;
                cwchItr--;
            }
        }

        pwszInBuff++;
        *pwszItr = EOL;
    }
    
    //Put on the Tail delimeter
    StringCchCopyExW(pwszItr, cwchItr, m_wszIDQuote, &pwszItr, &cwchItr, 0);

    //Give back to the user
    StringCchCopyW(pwszOutBuff, cwchOutBuff, pwsz);
    return TRUE;
}



/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetTypeNameAndParams
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetTypeNameAndParams(ULONG iCol, WCHAR* pwszName, size_t cwchName)
{
    ASSERT(iCol < m_cColumns);
    ASSERT(pwszName);

    WCHAR  wszBuffer[MAX_NAME_LEN];	 // Buffer

    // Add CreateParams precision and scale information
    wszBuffer[0] = EOL;
    if(m_rgColDesc[iCol].ulCreateParams & CP_PRECISION && m_rgColDesc[iCol].ulCreateParams & CP_SCALE)
    {
        StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), L"(%lu,%lu)", COLINFO_SIZE(m_rgColDesc[iCol]), m_rgColDesc[iCol].bScale);
    }
    else if(m_rgColDesc[iCol].ulCreateParams & CP_PRECISION || m_rgColDesc[iCol].ulCreateParams & CP_LENGTH || m_rgColDesc[iCol].ulCreateParams & CP_MAXLENGTH)
    {
        StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), L"(%lu)", COLINFO_SIZE(m_rgColDesc[iCol]));
    }

    //Add the ColumnType
    //Check for "()" in the typename, indicating creation params
    //are required at some other position besides the end, ie: "numeric() identity"
    pwszName[0] = EOL;
    if(wcsstr(m_rgColDesc[iCol].wszTypeName, L"()"))
    {
        // Add the ColumnType, upto the '()'
        StringCchCatNW(pwszName, cwchName, m_rgColDesc[iCol].wszTypeName, wcsstr(m_rgColDesc[iCol].wszTypeName, L"()") - m_rgColDesc[iCol].wszTypeName);

        // Add the precision and scale information
        StringCchCatW(pwszName, cwchName, wszBuffer);

        // Add the ColumnType, after the '()'
        StringCchCatW(pwszName, cwchName, wcsstr(m_rgColDesc[iCol].wszTypeName, L"()") + 2);
    }
    else
    {
        // Add the ColumnType
        StringCchCopyW(pwszName, cwchName, m_rgColDesc[iCol].wszTypeName);

        // If required, add the precision and scale information
        StringCchCatW(pwszName, cwchName, wszBuffer);
    }
        
    return S_OK;
}



/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetColInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetColInfo(DWORD dwInsertOpt)
{
    HRESULT hr;

    DBORDINAL i,cColumns = 0;
    DBCOLUMNINFO* rgColInfo = NULL;
    WCHAR*	rgStringBuffer = NULL;
    IColumnsInfo* pIColumnsInfo = NULL;

    //Obtain the rowset (m_pIRowset);
    QTESTC(hr = GetRowset(dwInsertOpt));
    CHECKC(m_pIRowset);

    //Now finally GetColInfo
    XTESTC(hr = m_pIRowset->QueryInterface(IID_IColumnsInfo, (void **)&pIColumnsInfo));
    XTESTC(hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgColInfo, &rgStringBuffer));

    //Alloc room for the COLDESC
    SAFE_FREE(m_rgColDesc);
    SAFE_ALLOC(m_rgColDesc, COLDESC, cColumns);
    memset(m_rgColDesc, 0, cColumns*sizeof(COLDESC));
            
    //Loop through the ColInfo and Copy to our ColDesc
    m_cColumns = 0;
    for(i=0; i<cColumns; i++)
    {
        //ignore Bookmark columns
        if(rgColInfo[i].iOrdinal == 0)
            continue;

        //DBCOLUMNINFO pwszName
        if(rgColInfo[i].pwszName==NULL)
        {
            //Although ColInfo is allowed to return NULL for unknown column names
            //TableCopy needs column names for many operations, 
            //(CREATE TABLE, INSERT, INDEXES), so just "generate" one
            StringCchPrintfW(m_rgColDesc[m_cColumns].wszColName, 
                             sizeof(m_rgColDesc[m_cColumns].wszColName)/sizeof(WCHAR),
                             L"Unknown%d", 
                             i);
        }
        else
        {
            StringCchCopyW(m_rgColDesc[m_cColumns].wszColName, 
                           sizeof(m_rgColDesc[m_cColumns].wszColName)/sizeof(WCHAR),
                           rgColInfo[i].pwszName);
        }

        //Now copy the rest of the info
        m_rgColDesc[m_cColumns].iOrdinal		= rgColInfo[i].iOrdinal;
        m_rgColDesc[m_cColumns].ulColumnSize	= rgColInfo[i].ulColumnSize;
        m_rgColDesc[m_cColumns].wType			= rgColInfo[i].wType;
        m_rgColDesc[m_cColumns].dwFlags			= rgColInfo[i].dwFlags;
        m_rgColDesc[m_cColumns].bPrecision		= rgColInfo[i].bPrecision;
        m_rgColDesc[m_cColumns].bScale			= rgColInfo[i].bScale;
        m_cColumns++;
    }

    
CLEANUP:	
    SAFE_RELEASE(pIColumnsInfo); 
    SAFE_FREE(rgColInfo);
    SAFE_FREE(rgStringBuffer);
    return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetTypeInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetTypeInfo()
{
    ASSERT(m_pCDataSource);
    HRESULT hr;
    
    IRowset*	pIRowset = NULL;
    IAccessor*	pIAccessor = NULL;
    HACCESSOR	hAccessor = DB_NULL_HACCESSOR;

    ULONG		i;
    DBCOUNTITEM	cRowsObtained = 0;
    HROW*		rghRows = NULL;
    
    //Current SchemaInfo for each type, until we find the correct match
    TYPEINFO TypeInfo;
    TYPEINFO* rgTypeInfo = NULL;
    ULONG* rgMatch = NULL;

    //Arrays to store best TypeInfo
    SAFE_ALLOC(rgTypeInfo, TYPEINFO, m_cColumns);
    SAFE_ALLOC(rgMatch, ULONG, m_cColumns);
    memset(rgMatch, NO_MATCH, m_cColumns*sizeof(ULONG));

    //Get ProviderTypes rowset IDBSchemaRowset
    QTESTC(hr = GetTypeInfoRowset(&pIAccessor, &hAccessor, &pIRowset));

    //Loop over all the Schema TypeInfo rowset
    //And match up with ColInfo
    while(TRUE)
    {
        XTESTC(hr = pIRowset->GetNextRows(NULL,0,MAX_BLOCK_SIZE,&cRowsObtained, &rghRows));
        
        //ENDOFROWSET
        if(cRowsObtained==0)
            break;
                
        //Loop over the BLOCK of rows obtained
        for(i=0; i<cRowsObtained; i++)	
        {	
            //Reset all the TypeInfo fields
            memset(&TypeInfo, 0, sizeof(TYPEINFO));
            
            //Put the data for one type into the TypeInfo Struct
            XTESTC(hr = pIRowset->GetData(rghRows[i],hAccessor, (void*)&TypeInfo));

            //Loop over all the columns and see if they match this type
            for(ULONG iCol=0; iCol<m_cColumns; iCol++)
            {	
                ASSERT(m_rgColDesc); 
                
                //Only try matching if this is the correct type and
                //the column doesn't already have a perfect match
                if(TypeInfo.wType != m_rgColDesc[iCol].wType || (rgMatch[iCol] & MATCH_EXACT))
                    continue;

                //A Nullable type cannot be mapped to a non-Nullable type
                if(m_rgColDesc[iCol].fIsNullable && !TypeInfo.fIsNullable)
                    continue;
                    
                //If never matched before, we at least know they match by type
                if(!rgMatch[iCol])
                {
                    rgMatch[iCol] |= MATCH_TYPE;
                    rgTypeInfo[iCol] = TypeInfo;
                }

                // Exact type/size matches take precedence
                if(COLINFO_SIZE(m_rgColDesc[iCol]) == TypeInfo.ulColumnSize)
                {
                    rgMatch[iCol] |= MATCH_EXACT;
                    rgTypeInfo[iCol] = TypeInfo;
                }

                // Otherwise try best fit size
                if(COLINFO_SIZE(m_rgColDesc[iCol]) < TypeInfo.ulColumnSize &&
                    TypeInfo.ulColumnSize <= rgTypeInfo[iCol].ulColumnSize)
                {
                    rgMatch[iCol] |= MATCH_SIZE;
                    rgTypeInfo[iCol] = TypeInfo;
                }
            }	
        }

        //Release this group of rows
        XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL));
        SAFE_FREE(rghRows);
    }

    //Now that we have the TypeInfo matched, fill in our ColDesc struct
    for(i=0; i<m_cColumns; i++) 
    {
        ASSERT(m_rgColDesc); 
        if(rgMatch[i])
        {
            //TYPEINFO
            StringCchCopyW(m_rgColDesc[i].wszTypeName, 
                           sizeof(m_rgColDesc[i].wszTypeName)/sizeof(WCHAR),
                           rgTypeInfo[i].wszTypeName);
            m_rgColDesc[i].ulCreateParams	= GetCreateParams(rgTypeInfo[i].wszCreateParams);
            m_rgColDesc[i].fIsNullable		= rgTypeInfo[i].fIsNullable == VARIANT_TRUE;
            m_rgColDesc[i].fIsAutoInc		= rgTypeInfo[i].fIsAutoInc == VARIANT_TRUE;
        }
        else
        {
            wMessageBox(NULL, MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OK, wsz_ERROR, 
                wsz_NO_TYPE_FOUND_, GetDBTypeName(m_rgColDesc[i].wType));
        }
    }


CLEANUP:
    if(hAccessor && pIAccessor)
        XTEST(pIAccessor->ReleaseAccessor(hAccessor,NULL));

    SAFE_RELEASE(pIAccessor);
    SAFE_RELEASE(pIRowset);

    SAFE_FREE(rgMatch);
    SAFE_FREE(rgTypeInfo);

    SAFE_FREE(rghRows);
    return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::MapTableInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::MapTableInfo(CTable* pCSourceTable)
{
    ASSERT(pCSourceTable);
    ASSERT(m_pCDataSource);
    HRESULT hr;

    IAccessor* pIAccessor = NULL;
    HACCESSOR  hAccessor = DB_NULL_HACCESSOR;

    IRowset*	pIRowset = NULL;

    DBCOUNTITEM	cRowsObtained = 0;
    HROW*		rghRows = NULL;

    //Match bitmask, indicating what type of match was found
    BOOL fMatchedAll = FALSE;

    TYPEINFO TypeInfo;
    TYPEINFO* rgTypeInfo = NULL;
    ULONG* rgMatch = NULL;

    //ColumnInfo
    m_cColumns	= pCSourceTable->m_cColumns;
    SAFE_FREE(m_rgColDesc);
    SAFE_ALLOC(m_rgColDesc, COLDESC, m_cColumns);
    memcpy(m_rgColDesc,	pCSourceTable->m_rgColDesc, m_cColumns * sizeof(COLDESC));	

    //IndexInfo
    m_cIndexes	= pCSourceTable->m_cIndexes;
    SAFE_FREE(m_rgIndexInfo);
    SAFE_ALLOC(m_rgIndexInfo, INDEXINFO, m_cIndexes);
    memcpy(m_rgIndexInfo, pCSourceTable->m_rgIndexInfo, m_cIndexes * sizeof(INDEXINFO));	

    //Arrays to store best TypeInfo
    SAFE_ALLOC(rgTypeInfo, TYPEINFO, m_cColumns);
    SAFE_ALLOC(rgMatch, ULONG, m_cColumns);
    memset(rgMatch, NO_MATCH, m_cColumns*sizeof(ULONG));
    
    //Get ProviderTypes rowset IDBSchemaRowset
    QTESTC(hr = GetTypeInfoRowset(&pIAccessor, &hAccessor, &pIRowset));
    
    //Loop until all types are matched.
    //We may not find a match, which promotes the type to the next higher
    //type, which will require another cycle to match that type...
    while(!fMatchedAll) 
    {
        //Get data for each row in rowset
        XTESTC(hr = pIRowset->RestartPosition(NULL));
                                
        //Loops over the entire SchemaRowset
        while(TRUE)
        {
            XTESTC(hr = pIRowset->GetNextRows(NULL, 0, MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));
            
            //ENDOFROWSET
            if(cRowsObtained == 0)
                break;
        
            //Loop over the BLOCK of rows obtained
            for(ULONG i=0; i<cRowsObtained; i++)	
            {	
                ASSERT(m_rgColDesc); 
        
                //Reset all the TypeInfo fields
                memset(&TypeInfo, 0, sizeof(TYPEINFO));

                //Put the data for one type into the TypeInfo Struct
                XTESTC(hr = pIRowset->GetData(rghRows[i], hAccessor, (void *)&TypeInfo));

                //Loop over the columns and get TypeInfo	
                for(ULONG iCol=0; iCol<m_cColumns; iCol++)
                {
                    //Only try matching if this is the correct type and
                    //the column doesn't already have a perfect match
                    if(TypeInfo.wType != m_rgColDesc[iCol].wType || (rgMatch[iCol] & MATCH_EXACT)) 
                        continue;

                    //An NonAutoInc Column cannot be placed into an AutoInc type
                    if(TypeInfo.fIsAutoInc && !m_rgColDesc[iCol].fIsNullable)
                        continue;

                    //A Nullable type cannot be mapped to a non-Nullable type
                    if(m_rgColDesc[iCol].fIsNullable && !TypeInfo.fIsNullable)
                        continue;
                    
                    //If never matched before, we at least know they match by type
                    if(!rgMatch[iCol])
                    {
                        rgMatch[iCol] |= MATCH_TYPE;
                        rgTypeInfo[iCol] = TypeInfo;
                    }

                    // Exact type/size matches take precedence
                    if(COLINFO_SIZE(m_rgColDesc[iCol]) == TypeInfo.ulColumnSize)
                    {
                        rgMatch[iCol] |= MATCH_EXACT;
                        rgTypeInfo[iCol] = TypeInfo;
                    }
                    
                    // Otherwise try best fit size
                    if(COLINFO_SIZE(m_rgColDesc[iCol]) < TypeInfo.ulColumnSize &&
                        TypeInfo.ulColumnSize <= rgTypeInfo[iCol].ulColumnSize)
                    {
                            rgMatch[iCol] |= MATCH_SIZE;
                            rgTypeInfo[iCol] = TypeInfo;
                    }
                }
            }

            //Release this group of rows
            XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
            SAFE_FREE(rghRows);
        }
            
        // See if every type has a match
        fMatchedAll = TRUE;
        for(ULONG i=0; i<m_cColumns; i++) 
        {
            ASSERT(m_rgColDesc); 

            // If not we will have to promote a type and try again
            if(rgMatch[i])
            {
                //If found a match fill in the TypeInfo fileds of our ColDesc
                StringCchCopyW(m_rgColDesc[i].wszTypeName,
                               sizeof(m_rgColDesc[i].wszTypeName)/sizeof(WCHAR),
                               rgTypeInfo[i].wszTypeName);
                m_rgColDesc[i].ulCreateParams	= GetCreateParams(rgTypeInfo[i].wszCreateParams);
                m_rgColDesc[i].fIsNullable		= rgTypeInfo[i].fIsNullable == VARIANT_TRUE;
                m_rgColDesc[i].fIsAutoInc		= rgTypeInfo[i].fIsAutoInc == VARIANT_TRUE;

                //TODO: why is this here?
                if (m_rgColDesc[i].ulColumnSize > rgTypeInfo[i].ulColumnSize)
                    m_rgColDesc[i].ulColumnSize = rgTypeInfo[i].ulColumnSize;
            }
            else
            {
                fMatchedAll = FALSE;

                //Try to promote it to the next largest type
                if(!GetPromotedType(&m_rgColDesc[i].wType)) 
                {
                    //If unable to promote, we are out of luck
                    wMessageBox(NULL, MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OK, wsz_ERROR, 
                        wsz_NO_TYPE_MATCH_, GetDBTypeName(m_rgColDesc[i].wType));
                    goto CLEANUP;
                }
            }
        }
    }


CLEANUP:
    if(hAccessor && pIAccessor)
        XTEST(pIAccessor->ReleaseAccessor(hAccessor, NULL));
    
    SAFE_RELEASE(pIAccessor);
    SAFE_RELEASE(pIRowset);

    SAFE_FREE(rgMatch);
    SAFE_FREE(rgTypeInfo);
    
    SAFE_FREE(rghRows);
    return hr;
}




/////////////////////////////////////////////////////////////////
// HRESULT CTable::CreateTable
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::CreateTable()
{
    ASSERT(m_pCDataSource);
    ASSERT(m_pCDataSource->m_pICommandText || m_pCDataSource->m_pIOpenRowset);
    HRESULT hr;

    BSTR			bstrSqlState = NULL;
    WCHAR			wszSqlStmt[MAX_QUERY_LEN];	// Create table statement

    ULONG cRecords = 0;
    IErrorRecords* pIErrorRecords = NULL;
    ICommandText* pICommandText = NULL;
    DBCOLUMNDESC* rgColumnDesc = NULL;

    //Release any existing rowsets
    SAFE_RELEASE(m_pIRowset);
    SAFE_RELEASE(m_pIAccessor);

    //If ITableDefinition is supported by the provider, use it by default.
    if(m_pCDataSource->m_pITableDefinition)
    {
        DBID			TableID;
        
        //Get DBCOLUMNDESC info
        QTESTC(hr = GetColumnDesc(&rgColumnDesc));

        //Create TableID
        TableID.eKind = DBKIND_NAME;
        TableID.uName.pwszName = wszSqlStmt;
        GetQuotedID(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), m_wszQualTableName);

        //ITableDefinition::CreateTable
        hr = m_pCDataSource->m_pITableDefinition->CreateTable(NULL, &TableID, m_cColumns, rgColumnDesc, IID_NULL, NULL, NULL, NULL, NULL);
        
        //If table already exists, offer to drop it
        if(hr == DB_E_DUPLICATETABLEID)
        {
            //If the user doesn't wants to drop it, exit
            if(IDNO == wMessageBox(NULL, MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO, wsz_ERROR, 
                wsz_ASK_DROP_TABLE_, m_pCDataSource->m_pwszTableTerm, m_wszQualTableName)) 
                goto CLEANUP;

            //Otherwise drop that table and continue
            XTESTC(hr = m_pCDataSource->m_pITableDefinition->DropTable(&TableID));
            XTESTC(hr = m_pCDataSource->m_pITableDefinition->CreateTable(NULL, &TableID, m_cColumns, rgColumnDesc, IID_NULL, NULL, NULL, NULL, NULL));
        }
        //If succeeded offer warning and continue
        if(hr == DB_S_ERRORSOCCURRED)
        {
            //TODO:display dialog
        }
        else
        {
            //Some other failure
            //Display Extended ErrorInfo
            XTESTC(hr);
        }
    }
    // Otherwise use SQL to create the table.
    else if(m_pCDataSource->m_pICommandText)
    {
        // Setup the initialize CREATE TABLE '<CTableName>'
        CreateSQLStmt(ESQL_CREATE_TABLE, wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR));

        // Set the Command Text
        pICommandText = m_pCDataSource->m_pICommandText;
        XTESTC(hr = pICommandText->SetCommandText(DBGUID_DBSQL, wszSqlStmt));

        // Execute the command
        hr = pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL);

        // If this didn't work, then we need to display messages, and if the
        // error was a duplicate table, offer to drop.
        if(FAILED(hr)) 
        {
            //Get the Error Records, need to save them, since every call
            //cleans the previous error objects
            QTESTC(GetErrorRecords(&cRecords, &pIErrorRecords));

            //If Error was due to an existing table, just ask to drop it
            if(GetSqlErrorInfo(0, pIErrorRecords, &bstrSqlState)==S_OK && 
                bstrSqlState && wcscmp(bstrSqlState, L"S0001")==0) 
            {
                WCHAR 	wszBuffer[MAX_QUERY_LEN];

                //If the user doesn't wants to drop it, exit
                if(IDNO == wMessageBox(NULL, MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO, wsz_ERROR, 
                                    wsz_ASK_DROP_TABLE_, m_pCDataSource->m_pwszTableTerm, m_wszQualTableName)) 
                    goto CLEANUP;
                
                //Otherwise drop that table and continue
                CreateSQLStmt(ESQL_DROP_TABLE, wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR));
                    
                //Drop the existing Table
                XTESTC(hr = pICommandText->SetCommandText(DBGUID_DBSQL, wszBuffer));
                XTESTC(hr = pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL));
                    
                //Now reset the CreateTable text to the SqlStmt and Execute
                XTESTC(hr = pICommandText->SetCommandText(DBGUID_DBSQL, wszSqlStmt));
                XTESTC(hr = pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL));
            }	
            else
            {
                //Otherwsie unknown error, just display it to the user
                DisplayErrorRecords(NULL, cRecords, pIErrorRecords);
            }
        }
    }
    
CLEANUP:
    SAFE_SYSFREE(bstrSqlState);
    SAFE_RELEASE(pIErrorRecords);

    //Free DBCOLUMNDESC
    if(rgColumnDesc)
    {
        for(ULONG i=0; i<m_cColumns && rgColumnDesc; i++)
            FreeProperties(rgColumnDesc[i].cPropertySets, rgColumnDesc[i].rgPropertySets);
        SAFE_FREE(rgColumnDesc);
    }
    return hr;
}




/////////////////////////////////////////////////////////////////
// HRESULT CTable::CopyIndexes
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::CopyIndexes(CTable* pCTable)
{
    ASSERT(pCTable);
    HRESULT hr;
    
    WCHAR			wszBuffer[MAX_NAME_LEN];
    WCHAR			wszSqlStmt[MAX_QUERY_LEN];
    ULONG i;

    // Don't waste time if there aren't any indexes.
    if (m_cIndexes == 0)
        return (S_OK);
    
    //Copy Index Info from Source table
    m_cIndexes	= pCTable->m_cIndexes;
    ULONG cIndexColumnDescs = 0;
    DBINDEXCOLUMNDESC* rgIndexColumnDescs = NULL;
    DBID* rgDBIDs = NULL;

    //Array to indicate which index/columns we have used
    ULONG* rgIndexUsed = NULL;
    SAFE_ALLOC(rgIndexUsed, ULONG, m_cIndexes);
    memset(rgIndexUsed, 0, m_cIndexes * sizeof(ULONG));

    SAFE_FREE(m_rgIndexInfo);
    SAFE_ALLOC(m_rgIndexInfo, INDEXINFO, m_cIndexes);
    memcpy(m_rgIndexInfo, pCTable->m_rgIndexInfo, m_cIndexes * sizeof(INDEXINFO));

    //Alloc DBINDEXCOLUMNDESC
    SAFE_ALLOC(rgIndexColumnDescs, DBINDEXCOLUMNDESC, m_cColumns);
    SAFE_ALLOC(rgDBIDs, DBID, m_cColumns);

    //Use IIndexDefinition::CreateIndex if supported
    if(m_pCDataSource->m_pIIndexDefinition)
    {
        //Create TableID
        DBID TableID;
        TableID.eKind = DBKIND_NAME;
        GetQuotedID(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), m_wszQualTableName);
        TableID.uName.pwszName = wszSqlStmt;

        // Loop around each index that is valid.  See if any are to be created
        for(i=0; i<m_cIndexes; i++) 
        {
            //If this index has already been created, skip
            //might have been used in another index creation
            if(rgIndexUsed[i])
                continue;
            
            //If this index is used as a primary key, skip
            //PrimaryKeys are taken care of differently
            if(m_rgIndexInfo[i].fIsPrimaryKey)
                continue;
            
            //Create IndexID
            DBID IndexID;
            IndexID.eKind = DBKIND_NAME;
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgIndexInfo[i].wszIndexName);
            IndexID.uName.pwszName = wszBuffer;
        
            // Now loop through all columns that belong to this index
            cIndexColumnDescs = 0;
            for(ULONG iCol=i; iCol<m_cIndexes; iCol++) 
            {
                //If not the same index skip
                if(wcscmp(m_rgIndexInfo[i].wszIndexName, m_rgIndexInfo[iCol].wszIndexName)!=0)
                    continue;
                    
                //mark this Index as used
                rgIndexUsed[iCol] = TRUE;
            
                //DBINDEXCOLUMNDESC info
                rgIndexColumnDescs[cIndexColumnDescs].pColumnID = &rgDBIDs[cIndexColumnDescs];
                rgIndexColumnDescs[cIndexColumnDescs].pColumnID->eKind = DBKIND_NAME;
                rgIndexColumnDescs[cIndexColumnDescs].pColumnID->uName.pwszName = m_rgIndexInfo[iCol].wszColName;
                
                //Indicate column order
                rgIndexColumnDescs[cIndexColumnDescs].eIndexColOrder = (m_rgIndexInfo[iCol].dwCollation == DB_COLLATION_DESC) ? DBINDEX_COL_ORDER_DESC : DBINDEX_COL_ORDER_ASC;
                cIndexColumnDescs++;
            }
            
            //Now Setup Index Properties
            ULONG cPropSets = 0;
            DBPROPSET* rgPropSets = NULL;

            //DBPROP_INDEX_AUTOUPDATE
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fAutoUpdate);
            
            //DBPROP_INDEX_CLUSTERED
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fClustered);

            //DBPROP_INDEX_FILLFACTOR
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_I4, m_rgIndexInfo[i].dwFillFactor);

            //DBPROP_INDEX_INITIALSIZE
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_I4, m_rgIndexInfo[i].dwInitialSize);
            
            //DBPROP_INDEX_NULLCOLLATION
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_I4, m_rgIndexInfo[i].dwNullCollation);

            //DBPROP_INDEX_NULLS
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_NULLS, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_I4, m_rgIndexInfo[i].dwNulls);
            
            //DBPROP_INDEX_PRIMARYKEY
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fIsPrimaryKey);

            //DBPROP_INDEX_SORTBOOKMARKS
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fSortBookmarks);
            
            //DBPROP_INDEX_TEMPINDEX
//			if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_TEMPINDEX, DBPROPSET_INDEX))
//				SetProperty(DBPROP_INDEX_TEMPINDEX, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fTempIndex);

            //DBPROP_INDEX_TYPE
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_TYPE, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_TYPE, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_I4, m_rgIndexInfo[i].wType);

            //DBPROP_INDEX_UNIQUE
            if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX))
                SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &cPropSets, &rgPropSets, DBTYPE_BOOL, m_rgIndexInfo[i].fUnique);
            
            //IIndexDefinition::CreateIndex
            //Don't exit yet, the user might want to continue even though this index failed
            XTEST(hr = m_pCDataSource->m_pIIndexDefinition->CreateIndex(&TableID, &IndexID, cIndexColumnDescs, rgIndexColumnDescs, cPropSets, rgPropSets, NULL));

            //Free Properties
            FreeProperties(cPropSets, rgPropSets);
            
            //If INDEX Failed
            if(FAILED(hr))
            {
                //Index Failed, Do you want to Continue?
                if(IDNO == wMessageBox(NULL, MB_TASKMODAL | MB_ICONINFORMATION | MB_YESNO, wsz_ERROR, 
                        wsz_INDEX_FAILED_, m_rgIndexInfo[i].wszIndexName))
                    goto CLEANUP;	
            }
            
            //Since the User didn't exit, continue as normal
            hr = S_OK;
        }
    }
    //Using SQL Commands to insert the index
    else if(m_pCDataSource->m_pICommandText)
    {

        // Loop around each index that is valid.  See if any are to be created
        for(i=0; i<m_cIndexes; i++) 
        {
            //If this index has already been created, skip
            //might have been used in another index creation
            if(rgIndexUsed[i])
                continue;
            
            //If this index is used as a primary key, skip
            //PrimaryKeys are taken care of differently
            if(m_rgIndexInfo[i].fIsPrimaryKey)
                continue;
            
            //"CREATE <UNIQUE> INDEX "
            QTESTC(hr = StringCchPrintfW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wsz_CREATE_INDEX_, (m_rgIndexInfo[i].fUnique == VARIANT_TRUE) ? wsz_UNIQUE_INDEX : wsz_SPACE));
                
            //Add IndexName to the list (quoted)
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgIndexInfo[i].wszIndexName);
            QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wszBuffer)); 

            //Add TableName
            QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), L" ON "));
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);
            QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wszBuffer)); 
            QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wsz_LPAREN));
                
            // Now loop through find all columns that belong to this index
            for(ULONG iCol=i; iCol<m_cIndexes; iCol++) 
            {
                //If not the same index skip
                if(wcscmp(m_rgIndexInfo[i].wszIndexName, m_rgIndexInfo[iCol].wszIndexName)!=0)
                    continue;
                    
                //mark this Index as used
                rgIndexUsed[iCol] = TRUE;
            
                //Add Column Name to the list (quoted)
                GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgIndexInfo[iCol].wszColName);
                QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wszBuffer));

                // Indicate Asending or Decending
                if(m_rgIndexInfo[iCol].dwCollation == DB_COLLATION_DESC)
                    QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wsz_INDEX_DESC)); 

                //Add trailing "," between col names
                QTESTC(hr = StringCchCatW(wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), wsz_COMMA));
            }
                
            //Replace last trailing "," with a ")"
            QTESTC(hr = StringCchCopyW(&wszSqlStmt[wcslen(wszSqlStmt)-wcslen(wsz_COMMA)],
                           sizeof(wszSqlStmt)/sizeof(WCHAR) - wcslen(wszSqlStmt)+wcslen(wsz_COMMA),
                           wsz_RPAREN));
            
            // If user wants to see the statement, show it to them
            if(m_pCWizard->m_pCTableCopy->m_fShowQuery)
                wMessageBox(NULL, MB_TASKMODAL | MB_OK | MB_ICONINFORMATION, wsz_OLEDB, 
                    wsz_SHOW_SQL_, m_pCDataSource->m_pwszDataSource, wszSqlStmt);
                
            //Set the command text
            XTESTC(hr = m_pCDataSource->m_pICommandText->SetCommandText(DBGUID_DBSQL, wszSqlStmt));
                    
            //Execute the command
            //Don't exit yet, the user might want to continue even though this index failed
            XTEST(hr = m_pCDataSource->m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL));

            //If INDEX Failed
            if(FAILED(hr))
            {
                //Index Failed, Do you want to Continue?
                if(IDNO == wMessageBox(NULL, MB_TASKMODAL | MB_ICONINFORMATION | MB_YESNO, wsz_ERROR, 
                        wsz_INDEX_FAILED_, m_rgIndexInfo[i].wszIndexName))
                    goto CLEANUP;	
            }
            
            //Since the User didn't exit, continue as normal
            hr = S_OK;
        }
    }

CLEANUP:
    SAFE_FREE(rgIndexUsed);
    SAFE_FREE(rgIndexColumnDescs);
    SAFE_FREE(rgDBIDs);
    return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::CreateSQLStmt
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::CreateSQLStmt(ESQL_STMT eSqlStmt, WCHAR* pwszSqlStmt, size_t cwchSqlStmt, BOOL fShowSql)
{
    ASSERT(pwszSqlStmt);
    HRESULT hr = S_OK;
    WCHAR     wszBuffer[MAX_NAME_LEN*2];	 // Buffer

    switch(eSqlStmt)
    {
        //SELECT <ColumnList> FROM <QualifiedTableName>
        case ESQL_SELECT:
        {
            //Create the SELECT statment
            StringCchCopyW(pwszSqlStmt, cwchSqlStmt, wsz_SELECT);
        
            // Loop through each column
            for(ULONG i=0; i<m_cColumns; i++) 
            {
                // Add the column to the list (quoted)
                GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgColDesc[i].wszColName);
                StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);

                if(i<m_cColumns-1)
                    StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_COMMA);
            }     	
    
            // Add the Table Name
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_FROM);
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);
        }
        break;

        case ESQL_INSERT:
        {
            StringCchCopyW(pwszSqlStmt, cwchSqlStmt, wsz_INSERT_INTO);
    
            //Add the Table Name
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_LPAREN);
            ULONG cColumns = 0;

            ULONG i;
            // Add the column list
            for(i=0; i<m_cColumns; i++) 
            {
                //Only Bind updatable columns
                if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITE || m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
                {
                    //Only add a leading comma if there are preceding columns 
                    if(cColumns++)
                        StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_COMMA);

                    // Add the column to the list (quoted)
                    GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgColDesc[i].wszColName);
                    StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);
                }
            } 

            //Add VALUES clause
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_VALUES_CLAUSE);
    
            // Loop through each column
            cColumns = 0;
            for(i=0; i<m_cColumns; i++) 
            {
                //Only Bind those columns that are updatable
                if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITE || m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
                {
                    //Only add a leading comma if there are preceding columns 
                    if(cColumns++)
                        StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_COMMA);
                    StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_PARAM);
                }
            } 		

            // Finish off the string
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_RPAREN);
        }
        break;

        case ESQL_CREATE_TABLE:
        {

            // Setup the initialize CREATE TABLE '<CTableName>'
            StringCchCopyW(pwszSqlStmt, cwchSqlStmt, wsz_CREATE_TABLE);
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);

            // Setup (<ColList>)
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_LPAREN);
    
            // Loop through each column and format the column name, type, and precision
            for(ULONG i=0; i<m_cColumns; i++) 
            {
                // Add the column to the list (quoted)
                GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_rgColDesc[i].wszColName);
                StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);
                StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_SPACE);

                // Add ColumnType (formatted with CreateParams)
                GetTypeNameAndParams(i, wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR));
                StringCchCatW(pwszSqlStmt, cwchSqlStmt, wszBuffer);
                
                //Add PRIMARY KEY if Supported and a PrimaryKey Column
                if(m_rgColDesc[i].fIsPrimaryKey && m_pCDataSource->m_fPrimaryKeysSupported && m_pCWizard->m_pCTableCopy->m_fCopyPrimaryKeys)
                    StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_PRIMARY_KEY);

                //Add Comma
                if(i<m_cColumns-1)
                    StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_COMMA);
            }

            //Add trailing ")"
            StringCchCatW(pwszSqlStmt, cwchSqlStmt, wsz_RPAREN);
        }
        break;		

        case ESQL_DROP_TABLE:
        {
            GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);
            StringCchPrintfW(pwszSqlStmt, cwchSqlStmt, wsz_DROP_TABLE_, wszBuffer);
        }
        break;

        default:
            ASSERT(!"Unhandled Case!");
            break;
    };
    
    // If user wants to see the statement, show it to them
    if(m_pCWizard->m_pCTableCopy->m_fShowQuery && fShowSql)
        wMessageBox(NULL, MB_TASKMODAL | MB_OK | MB_ICONINFORMATION, wsz_OLEDB, 
            wsz_SHOW_SQL_, m_pCDataSource->m_pwszDataSource, pwszSqlStmt);

    return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::AdjustBindings
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::AdjustBindings(ULONG cBindings, DBBINDING* rgBindings, void* pData)
{
    ASSERT(pData);
    HRESULT hr = S_OK;

    //Adjust all Storage Objects
    //We have 2 problems.  First some providers (MSDASQL) may require the LENGTH
    //of the storage object bound.  We have no clue what the LENGTH is unless 
    //we read the entire stream.  Second some providers may only allow 1 storage
    //object open at any one time.  The simplest soltuion would be just to buffer
    //the provider storage objects into our own and release the providers...

    for(ULONG i=0; i<cBindings; i++)
    {
        //DBSTATUS_S_ISNULL - nothing to adjust
        if(BINDING_STATUS(rgBindings[i],pData) == DBSTATUS_S_ISNULL)
            continue;

        //DB_S_TRUNCATED
        //Need to Adjust the LENGTH binding to be cbMaxLen
        if(BINDING_STATUS(rgBindings[i],pData) == DBSTATUS_S_TRUNCATED)
            BINDING_LENGTH(rgBindings[i], pData) = (ULONG)rgBindings[i].cbMaxLen;

        //DBTYPE_IUNKNOWN
        if(rgBindings[i].wType == DBTYPE_IUNKNOWN)
        {
             //Obtain the providers ISeqStream object
            ISequentialStream* pISequentialStream = (ISequentialStream*)(*(LONG_PTR*)((BYTE*)pData + rgBindings[i].obValue));
            
            //Copy the providers stream into our own CISeqStream object
            CISeqStream* pCISeqStream = new CISeqStream();
            pCISeqStream->Write(pISequentialStream, NULL);
            *((LONG_PTR*)((BYTE*)pData + rgBindings[i].obValue)) = (LONG_PTR)pCISeqStream;
            //BINDING_VALUE(rgBindings[i], pData) = pCISeqStream;
            SAFE_RELEASE(pISequentialStream);

            //Indicate the LENGTH Binding
            BINDING_LENGTH(rgBindings[i], pData) = pCISeqStream->Length();
        }

        //DBSTATUS_S_OK
        BINDING_STATUS(rgBindings[i],pData) = DBSTATUS_S_OK;
    }

    return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::CreateAccessors
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::CreateAccessors(ULONG* pcBindingInfo, BINDINGINFO** prgBindingInfo, ULONG* pcRowSize, ULONG ulBlobSize, BOOL* pbOutofLine)
{
    ASSERT(m_pIRowset);
    ASSERT(m_pIAccessor);
    ASSERT(pcBindingInfo);
    ASSERT(prgBindingInfo);
    ASSERT(pbOutofLine);

    HRESULT hr = S_OK;

    DBBYTEOFFSET    ulOffset = 0;
    ULONG           i,cBindings = 0;
    DBBINDING*      rgBindings = NULL;
    ULONG           cStorageObjects = 0;
    
    //Alloc the space for BindingInfo
    ULONG           cBindingInfo = 0;
    BINDINGINFO*    rgBindingInfo = NULL;
    SAFE_ALLOC(rgBindingInfo, BINDINGINFO, m_cColumns);

    //Alloc the space to hold the Bindings and Accessors
    SAFE_ALLOC(rgBindings, DBBINDING, m_cColumns);

    cBindings = 0;
    for(i=0; i<m_cColumns; i++) 
    {
        //SetUp the Bindings
        rgBindings[cBindings].iOrdinal	= m_rgColDesc[i].iOrdinal;
        rgBindings[cBindings].obStatus	= ulOffset;
        rgBindings[cBindings].obLength	= ulOffset + sizeof(DBSTATUS);
        rgBindings[cBindings].obValue	= ulOffset + sizeof(DBSTATUS) + sizeof(ULONG);
        
        rgBindings[cBindings].pTypeInfo = NULL;
        rgBindings[cBindings].pBindExt  = NULL;

        rgBindings[cBindings].dwPart	= DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS;			
        rgBindings[cBindings].dwMemOwner= DBMEMOWNER_CLIENTOWNED;
        rgBindings[cBindings].eParamIO	= DBPARAMIO_NOTPARAM;
        
        rgBindings[cBindings].dwFlags	= 0;
        rgBindings[cBindings].bPrecision= m_rgColDesc[i].bPrecision;
        rgBindings[cBindings].bScale	= m_rgColDesc[i].bScale;

        rgBindings[cBindings].pObject	= NULL;
        rgBindings[cBindings].wType		= m_rgColDesc[i].wType;
        rgBindings[cBindings].cbMaxLen	= m_rgColDesc[i].ulColumnSize;

        //Account for the NULL terminator
        if(rgBindings[cBindings].wType == DBTYPE_STR)
            rgBindings[cBindings].cbMaxLen	+= sizeof(CHAR);
        if(rgBindings[cBindings].wType == DBTYPE_WSTR)
            rgBindings[cBindings].cbMaxLen	+= sizeof(WCHAR);
        
        //Adjust ISLONG Columns if not bound as ISeqStream
        if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_ISLONG)
            ADJUST_SIZE(rgBindings[cBindings].cbMaxLen, ulBlobSize);

        //ISeqStream
        //Since some providers may only allow 1 storage object bound at a time
        //we will create a seperate accessor for each storage object bound.
        if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_ISLONG && ulBlobSize == ULONG_MAX)
        {
            cStorageObjects++;
            //Setup wType
            rgBindings[cBindings].wType		= DBTYPE_IUNKNOWN;
            rgBindings[cBindings].cbMaxLen	= sizeof(IUnknown*);
            
            //Setup pObject structure
            SAFE_ALLOC(rgBindings[cBindings].pObject, DBOBJECT, 1);
            rgBindings[cBindings].pObject->iid = IID_ISequentialStream;
            rgBindings[cBindings].pObject->dwFlags = STGM_READ;
        
            //Only need a seperate Accessor if there is more than 1 Storage column
            if(cStorageObjects > 1)
            {
                //Setup BindingInfo
                rgBindingInfo[cBindingInfo].cBindings = 1;
                SAFE_ALLOC(rgBindingInfo[cBindingInfo].rgBindings, DBBINDING, 1);
                memcpy(rgBindingInfo[cBindingInfo].rgBindings, &rgBindings[cBindings], sizeof(DBBINDING));

                //Create the accessor for the Storage column
                XTESTC(hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &rgBindings[cBindings], 0, &rgBindingInfo[cBindingInfo].hAccessor, NULL));
                cBindingInfo++;
            }
        }

        //Determine if there is out of line data...
        switch(rgBindings[cBindings].wType)
        {
            case DBTYPE_VARIANT:
                *pbOutofLine = TRUE;
                break;
        }

        ulOffset = ROUNDUP(rgBindings[cBindings].obValue + rgBindings[cBindings].cbMaxLen);

        //Already handled ISeqStream columns
        if(!(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_ISLONG && ulBlobSize == ULONG_MAX && cStorageObjects>1))
            cBindings++;
    }

    //Create the accessor for the entire row, (excluding Storage columns)
    if(cBindings)
    {
        //Setup BindingInfo
        rgBindingInfo[cBindingInfo].cBindings = cBindings;
        rgBindingInfo[cBindingInfo].rgBindings = rgBindings;

        XTESTC(hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &rgBindingInfo[cBindingInfo].hAccessor, NULL));
        cBindingInfo++;
    }

    //Size for pData
    if(pcRowSize)
        *pcRowSize = (ULONG)ulOffset;

    //Accessors
    *pcBindingInfo = cBindingInfo;
    *prgBindingInfo = rgBindingInfo;
    
CLEANUP:
    return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetRowset(DWORD dwInsertOpt)
{
    ASSERT(m_pCDataSource);
    WCHAR		wszBuffer[MAX_NAME_LEN];
    HRESULT hr;

    ULONG cPropSets = 0;
    DBPROPSET* rgPropSets = NULL;

    //Release the current rowset
    SAFE_RELEASE(m_pIRowset);
    SAFE_RELEASE(m_pIAccessor);
    
    //Kagera's Implementation requires IID_RowsetLocate for BLOB Support
    if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_IRowsetLocate, DBPROPSET_ROWSET))
        SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, TRUE);

    //DBPROP_UPDATABILITY
    if(dwInsertOpt != IDR_PARAM_SETS && IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_UPDATABILITY, DBPROPSET_ROWSET))
        SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_I4, DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT);

    //DBPROP_IRowsetChange
    if(dwInsertOpt == IDR_INSERTROW_IMMEDIATE && IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_IRowsetChange, DBPROPSET_ROWSET))
        SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, TRUE);

    //DBPROP_IRowsetUpdate
    if(dwInsertOpt == IDR_INSERTROW_BUFFERED && IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_IRowsetUpdate, DBPROPSET_ROWSET))
    {
        //DBPROP_CANHOLDROWS
        //In order to insert more rows while there are pending changes
        SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, TRUE);
        SetProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, TRUE);
    }

    //Setup TableID
    DBID TableID;
    TableID.eKind = DBKIND_NAME;
    
    //Quote the TableName
    TableID.uName.pwszName = wszBuffer;
    GetQuotedID(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), m_wszQualTableName);

    //IOpenRowset
    XTESTC(hr = m_pCDataSource->m_pIOpenRowset->OpenRowset(NULL, &TableID, NULL, IID_IRowset, cPropSets, rgPropSets, (IUnknown**)&m_pIRowset));
    CHECKC(m_pIRowset);

    //Obtain the Accessor
    XTESTC(hr = m_pIRowset->QueryInterface(IID_IAccessor, (void**)&m_pIAccessor));
    
CLEANUP:
    FreeProperties(cPropSets, rgPropSets);
    return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CTable::CopyData
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::CopyData(CTable* pCSourceTable, DBCOUNTITEM* pcRowsCopied)
{
    ASSERT(pCSourceTable && pcRowsCopied);
    HRESULT hr;

    WCHAR   wszSqlStmt[MAX_QUERY_LEN];	// Format the select statement
    WCHAR   wszBuffer[MAX_NAME_LEN];

    ULONG           i,j;
    DBBYTEOFFSET    ulOffset = 0;
    ULONG           cBindings = 0;
    DBBINDING*      rgBindings = NULL;
    HACCESSOR       hAccessor = DB_NULL_HACCESSOR;
    IAccessor*      pIAccessor = NULL;

    ULONG           cRowSize = 0;
    IRowset*        pISourceRowset = pCSourceTable->m_pIRowset;
    IRowsetChange*  pIRowsetChange = NULL;
    IRowsetUpdate*  pIRowsetUpdate = NULL;

    DBCOUNTITEM     cRowsObtained = 0;
    HROW*           rghRows = NULL;
    DBPARAMS        DBParams;

    void*           pData = NULL;
    void*           pRowData = NULL;
    DBCOUNTITEM     cRows = 0;

    CTableCopy* pCTableCopy = m_pCWizard->m_pCTableCopy;
    ULONG ulParamSets = pCTableCopy->m_dwInsertOpt == IDR_PARAM_SETS ? pCTableCopy->m_ulParamSets : 0;
    ULONG ulBlobSize  = pCTableCopy->m_dwBlobOpt == IDR_BLOB_SIZE ? pCTableCopy->m_ulBlobSize : ULONG_MAX;
    ULONG ulMaxRows   = pCTableCopy->m_dwRowOpt == IDR_ROW_COUNT ? pCTableCopy->m_ulMaxRows : ULONG_MAX;

    BOOL 		bOutofLine = FALSE;
    ULONG 		cBindingInfo = 0;
    BINDINGINFO* 	rgBindingInfo = NULL;
    CProgress* 		pCProgress = m_pCWizard->m_pCProgress;
    
    //Get the Rowset from the SourceTable
    QTESTC(hr = pCSourceTable->CreateAccessors(&cBindingInfo, &rgBindingInfo, &cRowSize, ulBlobSize, &bOutofLine));

    //Obtain the Accessor
    SAFE_ALLOC(rgBindings, DBBINDING, m_cColumns);

    cBindings = 0; 
    for(i=0; i<m_cColumns; i++) 
    {
        rgBindings[cBindings].iOrdinal	= ulParamSets ? cBindings+1 : m_rgColDesc[i].iOrdinal;
        rgBindings[cBindings].obStatus  = ulOffset;
        rgBindings[cBindings].obLength  = ulOffset + sizeof(DBSTATUS);
        rgBindings[cBindings].obValue   = ulOffset + sizeof(ULONG) + sizeof(DBSTATUS);
        
        rgBindings[cBindings].pTypeInfo = NULL;
        rgBindings[cBindings].pBindExt  = NULL;

        rgBindings[cBindings].dwPart	= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;			
        rgBindings[cBindings].dwMemOwner= DBMEMOWNER_CLIENTOWNED;
        rgBindings[cBindings].eParamIO	= ulParamSets ? DBPARAMIO_INPUT : DBPARAMIO_NOTPARAM;
        rgBindings[cBindings].dwFlags	= 0;
    
        rgBindings[cBindings].bPrecision= pCSourceTable->m_rgColDesc[i].bPrecision;
        rgBindings[cBindings].bScale	= pCSourceTable->m_rgColDesc[i].bScale;
            
        rgBindings[cBindings].pObject	= NULL;
        rgBindings[cBindings].wType		= pCSourceTable->m_rgColDesc[i].wType;
        rgBindings[cBindings].cbMaxLen	= pCSourceTable->m_rgColDesc[i].ulColumnSize;

        //Account for the NULL terminator
        if(rgBindings[cBindings].wType == DBTYPE_STR)
            rgBindings[cBindings].cbMaxLen	+= sizeof(CHAR);
        if(rgBindings[cBindings].wType == DBTYPE_WSTR)
            rgBindings[cBindings].cbMaxLen	+= sizeof(WCHAR);
        
        //Adjust ISLONG Columns if not bound as ISeqStream
        if(pCSourceTable->m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_ISLONG) 
            ADJUST_SIZE(rgBindings[cBindings].cbMaxLen, ulBlobSize);

        //ISeqStream
        if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_ISLONG && 
            pCTableCopy->m_dwBlobOpt == IDR_ISEQ_STREAM)
        {
            //Setup wType
            rgBindings[cBindings].wType		= DBTYPE_IUNKNOWN;
            rgBindings[cBindings].cbMaxLen	= sizeof(IUnknown*);
            
            //Setup pObject structure
            SAFE_ALLOC(rgBindings[cBindings].pObject, DBOBJECT, 1);
            rgBindings[cBindings].pObject->iid = IID_ISequentialStream;
            rgBindings[cBindings].pObject->dwFlags = STGM_READ;
        }

        ulOffset = ROUNDUP(rgBindings[cBindings].obValue + rgBindings[cBindings].cbMaxLen);
        
        //Only Bind Updatable columns
        //Note, we are not using the Source info here, since in the process
        //of adjusting columns to a different DSN, the columns may have become writeable
        if(m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITE || m_rgColDesc[i].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
            cBindings++;
    }

    //If using Parameters to INSERT the Data
    if(pCTableCopy->m_dwInsertOpt == IDR_PARAM_SETS)
    {
        // Now create the INSERT INTO statment
        CreateSQLStmt(ESQL_INSERT, wszSqlStmt, sizeof(wszSqlStmt)/sizeof(WCHAR), ulParamSets);
    
        //Set the command text
        XTESTC(hr = m_pCDataSource->m_pICommandText->SetCommandText(DBGUID_DBSQL, wszSqlStmt));

        //Create the Target Accessor
        XTESTC(hr = m_pCDataSource->m_pICommandText->QueryInterface(IID_IAccessor, (void**)&pIAccessor));
        XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cBindings, rgBindings, ulOffset, &hAccessor, NULL));
    }
    //were using InsertRow 
    else
    {
        //Obtain IRowsetChange interface from the TargetRowset 
        XTESTC(hr = m_pIRowset->QueryInterface(IID_IRowsetChange, (void**)&pIRowsetChange));
        
        //Create the Target Accessor
        XTESTC(hr = m_pIRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor));
        XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, ulOffset, &hAccessor, NULL));

        if(pCTableCopy->m_dwInsertOpt == IDR_INSERTROW_BUFFERED)
            XTESTC(hr = m_pIRowset->QueryInterface(IID_IRowsetUpdate, (void**)&pIRowsetUpdate));
    }
        
    // Display the progress dialog
    pCProgress->Display();
    pCProgress->SetHeading(wsz_COPYING);
        
    //Alloc room for pData
    SAFE_ALLOC(pData, BYTE, max(ulParamSets, 1) * cRowSize);
    memset(pData, 0, max(ulParamSets, 1) * cRowSize);
    
    //Setup DBPARAMS Struct
    DBParams.cParamSets = 1;			//Numer of Parameter sets
    DBParams.hAccessor	= hAccessor;	//Target Param Accessor
    DBParams.pData		= pData;		//Source Data

    while(cRows < ulMaxRows)
    {
        XTESTC(hr = pISourceRowset->GetNextRows(NULL, 0, (ulParamSets > 1) ? ulParamSets : MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));

        //ENDOFROWSET
        if(cRowsObtained==0)
            break;
    
        //Determine the number of rows that are actually needed to retrieve
        //The user might have specfified the number of rows to retrieve which could 
        //be smaller than the number in the block size
        DBCOUNTITEM cRowsNeeded = min(cRowsObtained, ulMaxRows-cRows);

        //Use Parameters to INSERT the data, use MultipleParamSets
        if(ulParamSets > 1)
        {
            //GetData
            for(i=0; i<cRowsNeeded; i++) 
            {
                pRowData = (BYTE*)pData + (i*cRowSize);
                for(j=0; j<cBindingInfo; j++)
                {
                    XTESTC(hr = pISourceRowset->GetData(rghRows[i], rgBindingInfo[j].hAccessor, pRowData));

                    //AdjustBindings
                    QTESTC(hr = AdjustBindings(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pRowData));
                }
            }
            
            //Adjust the paramerer sets
            DBParams.cParamSets = cRowsNeeded;
            
            //Execute the INSERT (multiple param sets)
            XTESTC(hr = m_pCDataSource->m_pICommandText->Execute(NULL, IID_NULL, &DBParams, NULL, NULL));

            //FreeBindingData - outofline memory
            for(i=0; i<cRowsNeeded && bOutofLine; i++)
            {
                pRowData = (BYTE*)pData + (i*cRowSize);
                for(j=0; j<cBindingInfo; j++)
                    QTESTC(FreeBindingData(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pData));
            }

            // Update insert progress
            StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), wsz_COPIED_RECORDS, (cRows += cRowsNeeded));
            if(!pCProgress->Update(wszBuffer))
                goto CLEANUP;
        }
        //Use Paramseters to INSERT the data, but only 1 ParamSet (not multiple)
        else if(ulParamSets == 1)
        {
            for(i=0; i<cRowsNeeded; i++) 
            {
                for(j=0; j<cBindingInfo; j++)
                {
                    //GetData
                    XTESTC(hr = pISourceRowset->GetData(rghRows[i], rgBindingInfo[j].hAccessor, pData));
            
                    //AdjustBindings
                    QTESTC(hr = AdjustBindings(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pData));
                }

                //Execute the INSERT
                XTESTC(hr = m_pCDataSource->m_pICommandText->Execute(NULL, IID_NULL, &DBParams, NULL, NULL));
            
                //FreeBindingData - outofline memory
                for(j=0; j<cBindingInfo && bOutofLine; j++)
                    QTESTC(FreeBindingData(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pData));

                // Update insert progress
                StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), wsz_COPIED_RECORDS, cRows++);
                if(!pCProgress->Update(wszBuffer))
                    goto CLEANUP;
            }
        }
        //Use InsertRow to INSERT the Data
        else
        {
            for(i=0; i<cRowsNeeded; i++) 
            {
                for(j=0; j<cBindingInfo; j++)
                {
                    //GetData from the Source
                    XTESTC(hr = pISourceRowset->GetData(rghRows[i], rgBindingInfo[j].hAccessor, pData));
            
                    //AdjustBindings
                    QTESTC(hr = AdjustBindings(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pData));
                }

                //InsertRow to the Target
                XTESTC(hr = pIRowsetChange->InsertRow(NULL, hAccessor, pData, NULL));

                // Update insert progress
                if(pCTableCopy->m_dwInsertOpt == IDR_INSERTROW_IMMEDIATE)
                {
                    // Update insert progress
                    StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), wsz_COPIED_RECORDS, cRows++);
                    if(!pCProgress->Update(wszBuffer))
                        goto CLEANUP;
                }

                //FreeBindingData - outofline memory
                for(j=0; j<cBindingInfo && bOutofLine; j++)
                    QTESTC(FreeBindingData(rgBindingInfo[j].cBindings, rgBindingInfo[j].rgBindings, pData));
            }

            //Use IRowsetUpdate::Update if in Bufferred mode
            //Update all, since we don't have the inserted row handles...
            if(pCTableCopy->m_dwInsertOpt == IDR_INSERTROW_BUFFERED)
            {
                XTESTC(hr = pIRowsetUpdate->Update(NULL, 0, NULL, NULL, NULL, NULL));

                // Update insert progress
                StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), wsz_COPIED_RECORDS, cRows += cRowsNeeded);
                if(!pCProgress->Update(wszBuffer))
                    goto CLEANUP;
            }
        }

        //Release the group of rows
        XTESTC(hr = pISourceRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
        SAFE_FREE(rghRows);
    }
    
CLEANUP:
    //Stop the propgress
    pCProgress->Destroy();
    *pcRowsCopied = cRows;

    //Release Accessors
    if(hAccessor)
        XTEST(pIAccessor->ReleaseAccessor(hAccessor, NULL));
    SAFE_RELEASE(pIAccessor);

    //Free any outofbound data, (error case)
    for(i=0; i<cBindingInfo; i++)
        FreeBindingData(rgBindingInfo[i].cBindings, rgBindingInfo[i].rgBindings, pData);

    //Release Accessors
    for(i=0; i<cBindingInfo; i++)
    {
        XTEST(pCSourceTable->m_pIAccessor->ReleaseAccessor(rgBindingInfo[i].hAccessor, NULL));
        FreeBindings(rgBindingInfo[i].cBindings, rgBindingInfo[i].rgBindings);
    }
    
    SAFE_FREE(rgBindingInfo);
    FreeBindings(cBindings, rgBindings);
    SAFE_FREE(pData);

    SAFE_RELEASE(pIRowsetChange);
    SAFE_RELEASE(pIRowsetUpdate);
    SAFE_FREE(rghRows);
    return hr;
}




/////////////////////////////////////////////////////////////////////////////
// HRESULT CTable::GetTypeInfoRowset
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTable::GetTypeInfoRowset(IAccessor** ppIAccessor, HACCESSOR* phAccessor, IRowset** ppIRowset)
{
    ASSERT(ppIAccessor && phAccessor && ppIRowset);
    ASSERT(m_pCDataSource && m_pCDataSource->m_pIOpenRowset);
    HRESULT hr;
    
    IDBSchemaRowset* pIDBSchemaRowset = m_pCDataSource->m_pIDBSchemaRowset;

    //Provider doesn't have to support IDBSchemaRowset
    if(pIDBSchemaRowset == NULL)
        return E_FAIL;
    
    //Bind all the columns from types rowset: 
    const static ULONG cBindings = 6;
    const static DBBINDING rgBindings[cBindings] = 
    {
        //TYPE_NAME
        1,	 			
        offsetof(TYPEINFO, wszTypeName),	// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale

        //DATA_TYPE
        2,	 			
        offsetof(TYPEINFO, wType),			// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        sizeof(USHORT),						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_UI2, 						// data type indicator
        0,									// precision
        0, 									// scale

        //COLUMN_SIZE
        3,	 			
        offsetof(TYPEINFO, ulColumnSize),	// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        sizeof(ULONG),						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_UI4, 						// data type indicator
        0,									// precision
        0, 									// scale

        //CREATE_PARAMS
        6,	 			
        offsetof(TYPEINFO, wszCreateParams),// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale
        
        //IS_NULLABLE
        7,	 			
        offsetof(TYPEINFO, fIsNullable),	// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        sizeof(VARIANT_BOOL),				// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_BOOL, 						// data type indicator
        0,									// precision
        0, 									// scale

        //AUTO_UNIQUE_VALUE
        12,	 			
        offsetof(TYPEINFO, fIsAutoInc),		// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        sizeof(VARIANT_BOOL),				// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_BOOL, 						// data type indicator
        0,									// precision
        0, 									// scale
    };

    //GetRowset
    //DBSCHEMA_PROVIDER_TYPES is required a SCHEMA
    XTESTC(hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_PROVIDER_TYPES, 0, NULL, IID_IRowset,0, NULL, (IUnknown**)ppIRowset));

    //Create the the Accessor
    XTESTC(hr = (*ppIRowset)->QueryInterface(IID_IAccessor, (void **)ppIAccessor));
    XTESTC(hr = (*ppIAccessor)->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, phAccessor, NULL));
        

CLEANUP:
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CTable::GetColumnDesc
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTable::GetColumnDesc(DBCOLUMNDESC** prgColumnDesc)
{
    //Init buffer
    ULONG i=0;
    HRESULT hr = S_OK;
    DBCOLUMNDESC* rgColumnDesc = NULL;
    SAFE_ALLOC(rgColumnDesc, DBCOLUMNDESC, m_cColumns);

    // For each column, fill out DBCOLUMNDESC info
    for(i=0; i<m_cColumns; i++)
    {
        //TypeName
        rgColumnDesc[i].pwszTypeName = m_rgColDesc[i].wszTypeName;
        
        rgColumnDesc[i].pTypeInfo	 = NULL;
        rgColumnDesc[i].pclsid		 = NULL;
        
        //ColumnID
        rgColumnDesc[i].dbcid.eKind = DBKIND_NAME;
        rgColumnDesc[i].dbcid.uName.pwszName = m_rgColDesc[i].wszColName;

        //Typeinfo
        rgColumnDesc[i].wType			=	m_rgColDesc[i].wType;
        rgColumnDesc[i].ulColumnSize	=	m_rgColDesc[i].ulColumnSize;
        rgColumnDesc[i].bPrecision		=	m_rgColDesc[i].bPrecision;
        rgColumnDesc[i].bScale			=	m_rgColDesc[i].bScale;

        //Properties
        rgColumnDesc[i].cPropertySets = 0;
        rgColumnDesc[i].rgPropertySets = NULL;
        
        //DBPPROP_COL_AUTOINCREMENT
//why make a column auto inc just cause it can be?
//		if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN))
//			SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, &rgColumnDesc[i].cPropertySets, &rgColumnDesc[i].rgPropertySets, DBTYPE_BOOL, m_rgColDesc[i].fIsAutoInc,DBPROPOPTIONS_OPTIONAL);

        //TODO DBPROP_COL_DEFAULT

        //TODO DBPROP_COL_DESCRIPTION

        //DBPROP_COL_FIXEDLENGTH
        if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_COL_FIXEDLENGTH, DBPROPSET_COLUMN))
            SetProperty(DBPROP_COL_FIXEDLENGTH, DBPROPSET_COLUMN, &rgColumnDesc[i].cPropertySets, &rgColumnDesc[i].rgPropertySets, DBTYPE_BOOL, IsNumericType(m_rgColDesc[i].wType),DBPROPOPTIONS_OPTIONAL);
        
        //DBPROP_COL_NULLABLE
        if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_COL_NULLABLE, DBPROPSET_COLUMN))
            SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, &rgColumnDesc[i].cPropertySets, &rgColumnDesc[i].rgPropertySets, DBTYPE_BOOL, m_rgColDesc[i].fIsNullable,DBPROPOPTIONS_OPTIONAL);

        //DBPROP_COL_PRIMARYKEY
        if(IsSettableProperty(m_pCDataSource->m_pIDBInitialize, DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN))
            SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN, &rgColumnDesc[i].cPropertySets, &rgColumnDesc[i].rgPropertySets, DBTYPE_BOOL, m_rgColDesc[i].fIsPrimaryKey && m_pCWizard->m_pCTableCopy->m_fCopyPrimaryKeys,DBPROPOPTIONS_OPTIONAL);
        
        //TODO DBPROP_COL_UNIQUE
    }

CLEANUP:
    *prgColumnDesc = rgColumnDesc;
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// Class CISeqStream
// 
// My implementation of ISeqStream interface
///////////////////////////////////////////////////////////////////////////////

CISeqStream::CISeqStream()
{
    m_iPos         = 0;
    m_cRef		   = 0;
    m_pBuffer      = NULL;
    m_cBufSize     = 0;

    //The constructor AddRef's
    AddRef();
}


CISeqStream::~CISeqStream()
{
    //Shouldn't have any references left
    ASSERT(m_cRef == 0);
    SAFE_FREE(m_pBuffer);
}

ULONG	CISeqStream::AddRef(void)
{
    return ++m_cRef;
}

ULONG	CISeqStream::Release(void)
{
    ASSERT(m_cRef);

    if(--m_cRef)
        return m_cRef;
    
    delete this;
    return 0;
}

HRESULT CISeqStream::QueryInterface(REFIID riid, void** ppv)
{
    ASSERT(ppv);
    *ppv = NULL;

    if (riid == IID_IUnknown)
        *ppv = this;
    if (riid == IID_ISequentialStream)
        *ppv = this;
    
    if(*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

BOOL CISeqStream::Seek(ULONG iPos)
{
    //Make sure the desired position is within the buffer
    ASSERT(iPos == 0 || iPos < m_cBufSize);

    //Reset the current buffer position
    m_iPos = iPos;
    return TRUE;
}

BOOL CISeqStream::Clear()
{
    //Frees the buffer
    m_iPos		   = 0;
    m_cBufSize     = 0;

    SAFE_FREE(m_pBuffer);
    return TRUE;
}

BOOL CISeqStream::CompareData(void* pBuffer)
{
    ASSERT(pBuffer);

    //Quick and easy way to compare user buffer with the stream
    return memcmp(pBuffer, m_pBuffer, m_cBufSize)==0;
}


HRESULT CISeqStream::Read(void *pv,	ULONG cb, ULONG* pcbRead)
{
    //Parameter checking
    if(pcbRead)
        *pcbRead = 0;

    if(!pv)
        return STG_E_INVALIDPOINTER;

    if(cb == 0)
        return S_OK;

    //Actual code
    ULONG cBytesLeft = m_cBufSize - m_iPos;
    ULONG cBytesRead = cb > cBytesLeft ? cBytesLeft : cb;

    //if no more bytes to retrive return 
    if(cBytesLeft == 0)
        return S_FALSE; 

    //Copy to users buffer the number of bytes requested or remaining
    memcpy(pv, (void*)((BYTE*)m_pBuffer + m_iPos), cBytesRead);
    m_iPos += cBytesRead;

    if(pcbRead)
        *pcbRead = cBytesRead;

    if(cb != cBytesRead)
        return S_FALSE; 

    return S_OK;
}
        
HRESULT CISeqStream::Write(const void *pv, ULONG cb, ULONG* pcbWritten)
{
    //Parameter checking
    if(!pv)
        return STG_E_INVALIDPOINTER;

    if(pcbWritten)
        *pcbWritten = 0;

    if(cb == 0)
        return S_OK;

    //Enlarge the current buffer
    m_cBufSize += cb;

    //Need to append to the end of the stream
    SAFE_REALLOC(m_pBuffer, BYTE, m_cBufSize);
    memcpy((void*)((BYTE*)m_pBuffer + m_iPos), pv, cb);

    if(pcbWritten)
        *pcbWritten = cb;

CLEANUP:
    return S_OK;
}


HRESULT CISeqStream::Write(ISequentialStream* pISeqStream, ULONG* pcbWritten)
{
    //Parameter checking
    if(!pISeqStream)
        return STG_E_INVALIDPOINTER;

    if(pcbWritten)
        *pcbWritten = 0;

    HRESULT hr = S_OK;
    ULONG cbRead = MAX_STREAM_BLOCK_SIZE;

    //Need to read (in chunks) from the Stream passed in and sotre in our CISeqStream object
    while(hr==S_OK && cbRead==MAX_STREAM_BLOCK_SIZE)
    {
        //Keep appending the data to the end of our buffer
        SAFE_REALLOC(m_pBuffer, BYTE, m_cBufSize + MAX_STREAM_BLOCK_SIZE);
        hr = pISeqStream->Read((BYTE*)m_pBuffer + m_cBufSize, MAX_STREAM_BLOCK_SIZE, &cbRead);
        m_cBufSize += cbRead;

        if(pcbWritten)
            *pcbWritten += cbRead;
    }

CLEANUP:
    return S_OK;
}
