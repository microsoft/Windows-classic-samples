//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CCol Implementation Module | This module contains header information for CCol.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 09-01-95	Microsoft	Code review update <nl>
//	[03] 10-01-95	Microsoft	Change to WCHAR * <nl>
//	[04] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CCol Elements|
//
// @subindex CCol
//
//---------------------------------------------------------------------------

#include "privstd.h"	// Private library common precompiled header
#include "CCol.hpp"
#include "miscfunc.h"   // wcsDuplicate


//---------------------------------------------------------------------------
// CCol::CCol
// This function does not set m_fUseInSQL or m_wszColName. It's purpose is to make
// the caller have this information before the object is conwszucted. Should
// be called from CTable::CreateTable. 
//	
// @mfunc CCol
//
//---------------------------------------------------------------------------
CCol::CCol(										
	IMalloc *		pIMalloc,				// @parm [IN] IMalloc pointer to alloc memory with (Default = NULL).
											// Null is a valid default because the majority of usage is
											// as a temp variable to hold a member of CTable.m_ColList,
											// which already has a malloc point associated with it.
	WCHAR *			pwszColName,			// @parm [IN] Column Name
	DBORDINAL		iOrdinal,				// @parm [IN] Column Ordinal
	DBTYPE			wType,					// @parm [IN] Datatype (Default = DBTYPE_STR). 
	WCHAR *			pwszProviderTypeName,	// @parm [IN] Provider datatype name (Default = NULL).
	BYTE			bPrecision,				// @parm [IN] Maximum precision allowed for this column (Default = 0xFF).
	BYTE			bScale,					// @parm [IN] Scale (Default = 0xFF).
	WCHAR *			pwszPrefix,				// @parm [IN] Literal Prefix (Default = "").
	WCHAR *			pwszSuffix,				// @parm [IN] Literal Suffix (Default = "").
	BOOL			fNullable,				// @parm [IN] Nullable (Default = FALSE).
	BOOL			fUnsigned/* =-1 */,		// @parm [IN] Unsigned (Default = -1).
	BOOL			fAutoInc,				// @parm [IN] Auto Increment (Default = FALSE).
	WCHAR *			pwszCreateParams,		// @parm [IN] CreateParam info (Default = "").
	BOOL			fUpdateable,			// @parm [IN] Updateable (Default = FALSE).
	ULONG			ulSearchable,			// @parm [IN] Searchable (Default = DB_SEARCHABLE).
	BOOL			fIsLong,				// @parm [IN] Very long data (Default = FALSE).
	GUID			gTypeGuid,
	BOOL			fIsFixedLength,			// @parm [IN] Is column fixed len (Default = FALSE)
	BOOL			fCaseSensitive,			// @parm [IN] Is column case sensitive (Default = FALSE)
	BOOL			fUnique,				// @parm [IN] Does column contain unique values (Default = FALSE)
	BOOL			fHasDefault,			// @parm [IN] Has column a default value (Default = FALSE)
	CSchema *		pSchema					// @parm [IN] Containing Schema object (Default = NULL)
)
{
	InitCCol(
		pIMalloc,				
		pwszColName,			
		iOrdinal,				
		wType,					
		pwszProviderTypeName,	
		bPrecision,				
		bScale,					
		pwszPrefix,				
		pwszSuffix,				
		fNullable,				
		fUnsigned,
		fAutoInc,				
		pwszCreateParams,		
		fUpdateable,			
		ulSearchable,			
		fIsLong,				
		gTypeGuid,
		fIsFixedLength,			
		fCaseSensitive,			
		fUnique,				
		fHasDefault,			
		pSchema					
	);
};


//---------------------------------------------------------------------------
// CCol::CCol
//		Allow convenient construction of CCol object with setting of back schema
//
//---------------------------------------------------------------------------
CCol::CCol(
	CSchema *		pSchema						// [IN] Containing Schema object (Default = NULL)
) 	
{
	// Initialize CCol with default values except for pSchema
	InitCCol(
		NULL,
		NULL,
		0,
		DBTYPE_STR,
		NULL,
		UCHAR_MAX,
		UCHAR_MAX,
		NULL,
		NULL,
		0,
		-1,
		FALSE,
		NULL,
		FALSE,
		DB_UNSEARCHABLE,
		FALSE,
		GUID_NULL,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		pSchema
	);
}

//---------------------------------------------------------------------------
// CCol::CCol
//		Perform initialization of CCol member vars
//
//---------------------------------------------------------------------------
void CCol::InitCCol(										
	IMalloc *		pIMalloc,				// @parm [IN] IMalloc pointer to alloc memory with (Default = NULL).
											// Null is a valid default because the majority of usage is
											// as a temp variable to hold a member of CTable.m_ColList,
											// which already has a malloc point associated with it.
	WCHAR *			pwszColName,			// @parm [IN] Column Name
	DBORDINAL		iOrdinal,				// @parm [IN] Column Ordinal
	DBTYPE			wType,					// @parm [IN] Datatype (Default = DBTYPE_STR). 
	WCHAR *			pwszProviderTypeName,	// @parm [IN] Provider datatype name (Default = NULL).
	BYTE			bPrecision,				// @parm [IN] Maximum precision allowed for this column (Default = 0xFF).
	BYTE			bScale,					// @parm [IN] Scale (Default = 0xFF).
	WCHAR *			pwszPrefix,				// @parm [IN] Literal Prefix (Default = "").
	WCHAR *			pwszSuffix,				// @parm [IN] Literal Suffix (Default = "").
	BOOL			fNullable,				// @parm [IN] Nullable (Default = FALSE).
	BOOL			fUnsigned/* =-1 */,		// @parm [IN] Unsigned (Default = -1).
	BOOL			fAutoInc,				// @parm [IN] Auto Increment (Default = FALSE).
	WCHAR *			pwszCreateParams,		// @parm [IN] CreateParam info (Default = "").
	BOOL			fUpdateable,			// @parm [IN] Updateable (Default = FALSE).
	ULONG			ulSearchable,			// @parm [IN] Searchable (Default = DB_SEARCHABLE).
	BOOL			fIsLong,				// @parm [IN] Very long data (Default = FALSE).
	GUID			gTypeGuid,
	BOOL			fIsFixedLength,			// @parm [IN] Is column fixed len (Default = FALSE)
	BOOL			fCaseSensitive,			// @parm [IN] Is column case sensitive (Default = FALSE)
	BOOL			fUnique,				// @parm [IN] Does column contain unique values (Default = FALSE)
	BOOL			fHasDefault,			// @parm [IN] Has column a default value (Default = FALSE)
	CSchema *		pSchema					// @parm [IN] Containing Schema object (Default = NULL)
)
{
	//DBCOLUMNINFO
	memset(&m_ColInfo, 0, sizeof(DBCOLUMNINFO));
	m_ColInfo.pwszName		= wcsDuplicate(pwszColName);
	m_ColInfo.iOrdinal		= iOrdinal;
	m_ColInfo.dwFlags		= 0;
	m_ColInfo.wType			= wType;
	m_ColInfo.ulColumnSize	= (ULONG_PTR)~0;
	m_ColInfo.bPrecision	= bPrecision;
	m_ColInfo.bScale		= bScale;
	m_ColInfo.pTypeInfo		= NULL;
	SetColName(pwszColName);

	//Init Strings
	m_pwszProviderTypeName	= wcsDuplicate(pwszProviderTypeName);
	m_pwszPrefix			= wcsDuplicate(pwszPrefix);
	m_pwszSuffix			= wcsDuplicate(pwszSuffix);
	m_pwszCreateParams		= wcsDuplicate(pwszCreateParams);

	//Init values
	m_wSubType				= wType;
	m_fNullable				= fNullable;
	m_fUnsigned				= fUnsigned;
	m_fAutoInc				= fAutoInc;
	m_fCanAutoInc			= fAutoInc;
	m_fUpdateable			= fUpdateable;
	m_ulSearchable			= ulSearchable;
	m_fUseInSQL				= TRUE;
	m_fIsLong				= fIsLong;
	m_gTypeGuid				= GUID_NULL;
	m_fIsFixedLength		= fIsFixedLength;
	m_fCaseSensitive		= fCaseSensitive;
	m_fUnique				= fUnique;
	m_fHasDefault			= fHasDefault;
	m_sMinScale				= 0;
	m_sMaxScale				= 0;
	VariantInit(&m_DefaultValue);
	m_cReqParams			= ULONG_MAX;
	m_pwszColDescription	= NULL;
	m_ulLCID				= 0;

	// Variant subtypes use default list unless otherwise over-ridden
	m_cVariantSubTypes		= NUMELEM(g_rgDefaultSubTypes);
	m_prgVariantSubTypes	= (DBTYPE *)g_rgDefaultSubTypes;

	m_pSchema				= pSchema;
	m_fIsNewLongType		= FALSE;
}

//---------------------------------------------------------------------------
// CCol::~CCol
//	
// @mfunc ~CCol
// 	
//---------------------------------------------------------------------------
CCol::~CCol()
{
	//DBCOLUMNINFO
	SAFE_FREE(m_ColInfo.pwszName);
	SAFE_RELEASE(m_ColInfo.pTypeInfo);
	ReleaseDBID(&m_ColInfo.columnid, FALSE);
	SAFE_FREE(m_pwszColDescription);

	VariantClear(&m_DefaultValue);
	PROVIDER_FREE(m_pwszProviderTypeName);
	PROVIDER_FREE(m_pwszCreateParams)
	PROVIDER_FREE(m_pwszPrefix)
	PROVIDER_FREE(m_pwszSuffix)
}


//---------------------------------------------------------------------------
//	Overloaded = Operator
// 
// 	@mfunc CCol&|CCol|operator=|
//---------------------------------------------------------------------------
CCol& CCol::operator=(
	CCol& refCCol	 //  @parm CCol&|refCol|[IN] Reference to CCol object.
)
{
	//DBCOLUMNINFO
	SetColID(refCCol.GetColID());
	SetColName(refCCol.m_ColInfo.pwszName);
	m_ColInfo.iOrdinal		= refCCol.m_ColInfo.iOrdinal;
	m_ColInfo.dwFlags		= refCCol.m_ColInfo.dwFlags;
	m_ColInfo.wType			= refCCol.m_ColInfo.wType;
	m_ColInfo.ulColumnSize	= refCCol.m_ColInfo.ulColumnSize;
	m_ColInfo.bPrecision	= refCCol.m_ColInfo.bPrecision;
	m_ColInfo.bScale		= refCCol.m_ColInfo.bScale;
	m_ColInfo.pTypeInfo		= refCCol.m_ColInfo.pTypeInfo;
	SAFE_ADDREF(m_ColInfo.pTypeInfo);

	//Copy Strings
	SetProviderTypeName(refCCol.m_pwszProviderTypeName);
	SetPrefix(refCCol.m_pwszPrefix);
	SetSuffix(refCCol.m_pwszSuffix);
	SetColDescription(refCCol.GetColDescription());
	SetCreateParams(refCCol.m_pwszCreateParams);
	
	//Copy values
	m_fUnique				= refCCol.m_fUnique;
	m_fHasDefault			= refCCol.m_fHasDefault;
	VariantCopy(&m_DefaultValue, &refCCol.m_DefaultValue);
	
	m_wSubType				= refCCol.m_wSubType;
	m_fNullable				= refCCol.m_fNullable;
	m_fUnsigned				= refCCol.m_fUnsigned;
	m_sMinScale				= refCCol.m_sMinScale;
	m_sMaxScale				= refCCol.m_sMaxScale;
	m_fAutoInc				= refCCol.m_fAutoInc;
	m_fCanAutoInc			= refCCol.m_fCanAutoInc;
	m_fUpdateable			= refCCol.m_fUpdateable;
	m_ulSearchable			= refCCol.m_ulSearchable;
	m_fUseInSQL				= refCCol.m_fUseInSQL;
	m_fIsLong				= refCCol.m_fIsLong;
	m_gTypeGuid				= refCCol.m_gTypeGuid;
	m_fIsFixedLength		= refCCol.m_fIsFixedLength;
	m_fCaseSensitive		= refCCol.m_fCaseSensitive;
	m_cReqParams			= refCCol.m_cReqParams;
	m_ulLCID				= refCCol.m_ulLCID;
	m_cVariantSubTypes		= refCCol.m_cVariantSubTypes;
	m_prgVariantSubTypes	= refCCol.m_prgVariantSubTypes;
	m_pSchema				= refCCol.m_pSchema;
	m_fIsNewLongType		= refCCol.m_fIsNewLongType;
	return *this;
}

//---------------------------------------------------------------------------
//	Copy Constructor
// 
// 	@mfunc CCol&|CCol|operator=|
//
//---------------------------------------------------------------------------
CCol::CCol(CCol& refCCol)
{
	//NOTE:  The CopyConstrcutor is called to not only construct a new object
	//but also copy data into from an existing object.

	//DBCOLUMNINFO
	memset(&m_ColInfo, 0, sizeof(DBCOLUMNINFO));
	m_pwszColDescription = NULL;

	//Init Strings
	m_pwszProviderTypeName		= NULL;
	m_pwszPrefix				= NULL;
	m_pwszSuffix				= NULL;
	m_pwszCreateParams			= NULL;
	VariantInit(&m_DefaultValue);

	//Just delegate out to the (operator =) method
	operator=(refCCol);
}


	
//---------------------------------------------------------------------------
// CCol::SetColInfo
// Client is responsible for releasing memory 
//	
// @mfunc SetColInfo
//	
//---------------------------------------------------------------------------
void CCol::SetColInfo(
	DBCOLUMNINFO* pColInfo	  // @parm [IN] Column Info. 
)	
{
	ASSERT(pColInfo);

	//DBCOLUMNINFO.columnid
	SetColID(&pColInfo->columnid);

	//DBCOLUMNINFO.pwszName
	SetColName(pColInfo->pwszName);

	//DBCOLUMNINFO.iOrdinal
	SetColNum(pColInfo->iOrdinal);

	//DBCOLUMNINFO.wType
	SetProviderType(pColInfo->wType);

	//DBCOLUMNINFO.dwFlags & DBCOLUMNFLAGS_ISLONG
	SetIsLong(pColInfo->dwFlags & DBCOLUMNFLAGS_ISLONG ? 1 : 0);

	//DBCOLUMNINFO.dwFlags & DBCOLUMNFLAGS_ISNULLABLE
	SetNullable((pColInfo->dwFlags & DBCOLUMNFLAGS_ISNULLABLE)? 1 : 0);

	//DBCOLUMNINFO.dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH
	SetIsFixedLength((SHORT)(pColInfo->dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH) ? 1 : 0);

	//DBCOLUMNINFO.dwFlags & DBCOLUMNFLAGS_WRITE | WRITEUNKNOWN
	SetUpdateable((pColInfo->dwFlags & DBCOLUMNFLAGS_WRITE) || (pColInfo->dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN) ? 1 : 0);

	//DBCOLUMNINFO.ulColumnSize
	SetColumnSize(pColInfo->ulColumnSize);

	//DBCOLUMNINFO.bPrecision
	SetPrecision(pColInfo->bPrecision);

	//DBCOLUMNINFO.bScale
	SetScale(pColInfo->bScale);
}



//---------------------------------------------------------------------------
// CCol::MakeData 
//
// CCol			|
// MakeData		|
// Generates consistent data of the appropriate type, and returns
// a pointer to that data.
//
// @mfunc	MakeData
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function succeeded
//  @flag E_FAIL    | Function failed
//
//	NOTE:	64 bit casting is used to preserve signs between Scale and Precision comparisons
//			(Precision is a very large 32bit value for (MAX) data types and some providers may have -ve scale)
//---------------------------------------------------------------------------
HRESULT CCol::MakeData
(
	WCHAR* 		wszData,		// @parm [OUT] Return data
	DBCOUNTITEM	ulRowNum,		// @parm [IN]  Row number which is one based
	EVALUE 		eValue,			// @parm [IN]  Primary or secondary data 
	ENULL		eNulls,			// @parm [IN]  Allow nulls for diagonal
	DBTYPE*		pwSubType,		// @parm [IN/OUT]  Column SubType (if Variant)
	DBORDINAL	ulIndexCol		// @parm [IN]  Unique column, (possible index)
)
{
	DBCOUNTITEM	ulRowHold=0;		// Calculate seed value
	DBCOUNTITEM	ulRemainder=0;		// Remainder of %
	DBLENGTH	lPrecision=0;		// Precision
	DBLENGTH	lPrecUsed=0;		// Precision used 

	SHORT		sScale=0;			// Scale
	DBLENGTH	iColLen=0;			// Length of the value seed
	DBLENGTH	iLenLeft=0;			// Scale - length left in strDecimal to fill
	INT_PTR		ivalue=0;			// int value
	long		lvalue=0;			// long value
									// Also value returned from ReverseFind
	DBLENGTH	lenwszData=0;		// length of wszData for first column	
	DBTYPE		wType;				// DBTYPE data type
	WCHAR 		wszCol[MAX_COL_SIZE+1];	// Seed value
	WCHAR		wszBuffer[MAX_COL_SIZE+1];	// Copy of the data
	WCHAR *		pwszSingleChar=NULL;// Single character
	HRESULT		hr = S_OK;

	//If we need to choose a subtype of a variant, we will do
	//so by using the row seed.
	DBCOUNTITEM	VarTypeSeed = ulRowNum;

	//Decrement VarTypeSeed to make it zero-based instead of 1-based.
	if(VarTypeSeed)
		VarTypeSeed--; 

	ASSERT(wszData);

	// Initialize strings
	wszData[0]=L'\0';
	wszCol[0]=L'\0';
	wszBuffer[0]=L'\0';

	//if we were passed in a file name, we  have the data in the ini file
	//The INI file is only able to obtain data for existing rows.
	//MakeData maybe called to generate data for new rows (FillInputBindings)
	if(GetModInfo()->GetFileName() && ulRowNum <= GetModInfo()->GetParseObject()->GetRowCount())
	{
		//Obtain data from the INI FIle...
		hr = GetModInfo()->GetParseObject()->GetData(ulRowNum, GetColNum(), wszData, &wType);
		if(FAILED(hr) || hr==S_FALSE)
			return hr;

		if(pwSubType)
		{
			// need to change the VT type to a valid variant type
			if( wType == DBTYPE_WSTR || wType == DBTYPE_STR )
				*pwSubType = DBTYPE_BSTR;
			else
				*pwSubType = wType;
		}

		// If the type for the column can not be updated then skip the column
		// Also check to see if the test explicitly requests that Read Only Columns
		// not be compared.
		// For example, a test may be using .ini file and running variations
		// that delete rows; in such a case, it's not possible to restore the
		// values that existed in the non-updateable columns.
		if (!GetUpdateable() && (!GetModInfo()->GetFileName() || !GetModInfo()->GetCompReadOnlyCols()) )
			return DB_E_BADTYPE;

		return hr;
	}
	

	// If the type for the column can not be updated then skip the column
	if (!GetUpdateable())
		return DB_E_BADTYPE;

	// If user specified nulls on the CTable constructor and the datatype is
	// nullable then set the value to null when the row number is equal to
	// the column number and not equal to the column with a unique index
	// Note:  bNoNullValues by default is FALSE.  so unless bNoNullValues is TRUE
	// The working of MakeData doesn't change.  in case where bNoNullValeus is passed
	// as TRUE the function will always create the value instead of returning NULL for the
	// following case.
	// Only makes one NULL per column because the spirit of MakeData is to try,
	// whenever possible to guarantee uniqueness up to 1000 rows.
	if((eNulls == USENULLS) && (GetNullable() == 1)
		&& (GetColNum()==ulRowNum)
		&& (GetColNum() != 1))	
		return S_FALSE;
	
	// Get the column data type
	wType = GetProviderType();


	// If the type is DBTYPE_VARIANT, we want to check the type underneath
	if(wType == DBTYPE_VARIANT)
	{
		//Create data for the variant
		CCol col;

		wType = GetVariantType(ulRowNum, col);

		if (pwSubType)
			*pwSubType = wType;

		// If we are basing the variant data based on the provider's schema rowset
		// Otherwise, just continue since we have set the "wType" variable
		if( col.GetProviderType() != DBTYPE_EMPTY )
			return col.MakeData(wszData, ulRowNum, eValue, eNulls);
	}

	if(wType & (DBTYPE_ARRAY|DBTYPE_VECTOR) )
	{
		DBTYPE		wBaseType	= (DBTYPE)(wType & ~(DBTYPE_ARRAY|DBTYPE_VECTOR));
		DBCOUNTITEM	cElements	= ulRowNum%5+1;
		WCHAR*		pwsz		= wszData;

		//Add SafeArray Dimension indicator
		if(wType & DBTYPE_ARRAY)
			wcscpy(pwsz++, L"[");	

		for(ULONG iEle=0; iEle<cElements; iEle++)
		{
			//Create data for this sub-elements
			CCol col(*this);
			
			col.SetProviderType(wBaseType);
			if(IsNumericType(wBaseType))
			{
				// Set a safe precision/scale values
				col.SetPrecision(2); 
				col.SetScale(0); 
			}

			col.MakeData( pwsz, ulRowNum, eValue, eNulls);
			
			//Elements seperator
			if(iEle+1 < cElements)
				wcscat(pwsz++, L",");
		}
		//SafeArray Dimension indicator
		if(wType & DBTYPE_ARRAY)
			wcscat(pwsz++, L"]");	

		return S_OK;
	}

	// Special case data types where part of the value needs to be hard coded.
	switch(wType) 
	{
		// Data type bit
		// bits should be 0 or 1, mod will make ulRowNum either 0 or 1
		case DBTYPE_BOOL:
		{
			swprintf(wszData,L"%u", ulRowNum % 2);
			return (S_OK);
		}

		// Data type date
		case DBTYPE_DBDATE:
		{
			// let the year range be between 1997-2035
			swprintf(wszData,L"%04i-%02u-%02u", 1997+(ulRowNum%39), 12, ulRowNum % 30 + 1);
			return (S_OK);
		}

		// Data type filetime
		case DBTYPE_FILETIME:
		{
			UWORD day;
			UWORD hour = 0;
			UWORD minute = 0;
			UWORD second = 0;
			UDWORD fraction = 0;
			UWORD	month = 0;
			UWORD	year = 0;

			// Keep the fraction in two digits.
			fraction	= ( (UWORD)ulRowNum + 1)  % 100;
			second		= (UWORD)(ulRowNum % 60);
			minute		= (UWORD)(ulRowNum % 60);
			hour		= (UWORD)(ulRowNum % 24);

			// let the year range be between 1997-2035
			year = (UWORD)(1997+(ulRowNum%39));
			month = (UWORD)(ulRowNum % 12);
			day = (UWORD)(1 + ulRowNum % 28);
			swprintf(wszData,L"%05hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
					year, month, day, hour, minute, second, 
					fraction);
			return (S_OK);
		}

		// Data type timestamp
		case DBTYPE_DBTIMESTAMP:
		{
			UWORD day = 1;
			UWORD hour = 0;
			UWORD minute = 0;
			UWORD second = 0;
			UDWORD fraction = 0;

			switch(GetPrecision())
			{
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
					// Keep the fraction in two digits.
					fraction = ( (UWORD)ulRowNum + 1)  % 100;
				case 19:
					second = (UWORD)(ulRowNum % 60);
				case 16:
					minute = (UWORD)(ulRowNum % 60);
				case 13:
					hour = (UWORD)(ulRowNum % 24);
				case 10:
					day = (UWORD)(ulRowNum % 30 + 1);
					break;
				default:
					TERROR(L"The Precision for DBTYPE_DBTIMESTAMP is incorrect.");
				}

			// let the year range be between 1997-2035
			if(fraction) 
			{
				//NOTE:  SQLServer has an issue with reporting they support Scale=3,
				//but only in increments of 3.  Due to this, just work arround this issue
				//by only requiring 1 less than the indicated scale...
				ULONG ulFractionalWidth = GetScale();
				if(ulFractionalWidth)
					ulFractionalWidth--;

				swprintf(wszData,L"%04i-%02u-%02d %02d:%02d:%02d.%0*d",
					1997+(ulRowNum%39), 1, day, hour, minute, second, 
					ulFractionalWidth,	fraction);
			}
			else 
			{
				swprintf(wszData,L"%04i-%02u-%02d %02d:%02d:%02d", 1997+(ulRowNum%39),
					12, day, hour, minute, second);
			}
			return (S_OK);
		}

		// Data type time
		case DBTYPE_DBTIME:
		{
			swprintf(wszData,L"%02u:%02u:%02u", ulRowNum % 24, 
				ulRowNum % 60, ulRowNum % 60);
			return (S_OK);
		}

		// Data type date (OLE Automation)
		case DBTYPE_DATE:
		{ 
			// let the year range be between 1997-2035
			swprintf(wszData,L"%04i-%02u-%02u %02u:%02u:%02u", 
				1997+(ulRowNum%39), 12, ulRowNum % 30 + 1,ulRowNum % 24, 0, 0);
			return (S_OK);
		}

		// Data type guid {C8B522CF-5CF3-11CE-ADE5-00AA0044773D}
		case DBTYPE_GUID:
		{ 
			swprintf(wszData,L"{%08i-%04i-%04i-%04i-%012i}", 
				ulRowNum%10, ulRowNum%10, ulRowNum%10, ulRowNum%10, ulRowNum%1000);
			return (S_OK);
		}
	}

	// wszCol holds the seed which is or made partially from the row #.
	// If the row number is less than 100 use the row number unless
	// it is a multiple of 11, then add two zeros's to the row number.
	// Only zero will guarantee no dupe's because any other digits, say "4"
	// will cause row 4 to match row 44 for string data.
	if (ulRowNum < 100)
	{
		swprintf(wszCol,L"%u", ulRowNum);

		if (ulRowNum%11 == 0)
			wcscat(wszCol,L"00");
	}
	else
	{
		// For a row number greater than 99 use the row number unless
		// the row number is made up of the same digit (Ex=333), then
		// add one five to the end of the row number for the seed.
		ulRowHold = ulRowNum;
		ulRemainder = ulRowHold%10;
		ulRowHold = ulRowHold / 10;

		while((ulRowHold%10)==ulRemainder)
			ulRowHold=ulRowHold/10;

		swprintf(wszCol,L"%u", ulRowNum);

		if(!ulRowHold)
			wcscat(wszCol,L"5");
	}

	// Make a secondary seed. This will make a secondary value. The table can
	// be created with primary or secondary values.
	if (eValue == SECONDARY)
	{
		swprintf(wszBuffer, L"%u", ulRowNum+1);
		wcscat(wszBuffer,wszCol);
		wcscpy(wszCol,wszBuffer);
	}

	// Precision
	lPrecUsed = GetMaxSize();

	// Scale
	// Since some servers store float, double, and real with a scale of one we
	// will give them a scale with a minumum of one.
	switch(wType)
	{
		// Data types float and double
		case DBTYPE_R8:
			sScale = (SHORT)(ulRowNum > MAX_SCALE ? MAX_SCALE : ulRowNum);
			break;

		// Data type real
		case DBTYPE_R4:
			sScale = 1;
			break;

		// Data type currency
		case DBTYPE_CY:
			sScale = 4;
			break;

		case DBTYPE_VARNUMERIC:
			if ( ulRowNum % 3 == 0 )
				sScale = GetMaxScale();
			else if ( ulRowNum % 3 == 1 )
				sScale = GetMinScale();
			else if ( ulRowNum % 3 == 2 )
				sScale = GetScale();
			break;

		case DBTYPE_DECIMAL:
		case DBTYPE_NUMERIC:
		case DBTYPE_DBTIMESTAMP:
			sScale = GetScale();
			break;
			
		default:
			sScale = 0;
			break;
	}

	//HACK for Oracle FLOAT columns...
	//This is needed since DecribeParams returns WSTR for all types.  DataConvert
	//will then convert all params to WSTR, which will truncate FLOAT to the MAX
	//precision of SQL_C_DOUBLE==15.  To get arround this problem for now, just
	//make sure that all FLOAT types on Oracle are no larger than 15.
	WCHAR *providerTypeName = GetProviderTypeName();
	
	if(providerTypeName)
		if (wType == DBTYPE_R8 && wcscmp(providerTypeName, L"FLOAT")==0)
			lPrecUsed = min(lPrecUsed, 15);
	//HACK end
	
	if ((sScale >= 0) && ((unsigned __int64)sScale > (unsigned __int64)lPrecUsed ))	//See function comments for __int64 use
	{
		// only TRUE for VARNUMERIC
		ASSERT(wType==DBTYPE_VARNUMERIC);
		lPrecision = lPrecUsed;
	}
	else
		lPrecision = lPrecUsed  - sScale;

	// We are converting from char so we need to double the length
	// Data types binary, varbinary, and longvarbinary
	if (wType == DBTYPE_BYTES)
		lPrecision = lPrecision * 2;

	// We set the precision to 255 right now
	if (wType == DBTYPE_BSTR)
		lPrecision = 255;
	
	// Length of the seed
	iColLen = wcslen(wszCol);

	// We want to make sure that the index column has unique numbers so use
	// the row number as the only part of the left side of the decimal
	if (GetColNum() != ulIndexCol)
	{
		if ( ((unsigned __int64)lPrecUsed > (unsigned __int64)sScale) || (sScale < 0) )	//See function comments for __int64 use
		{
			// Fill left side of the decimal point with the seed
			DBLENGTH lDataSize = ( MAXDATALEN <= (size_t)lPrecision) ? (MAXDATALEN/2) - 100 : lPrecision;
			while(wcslen(wszData) < lDataSize)
			{
				iLenLeft = lDataSize - wcslen(wszData);
			
				if (iColLen <= iLenLeft)
					wcscat(wszData,wszCol);
				else
				{
					wcsncat(wszData,wszCol,(size_t)iLenLeft);
					wszData[lDataSize] = L'\0';
				}
			}
		}
		else
		{
			// This path should only be hit in this case
			ASSERT( ((sScale >= 0) && (unsigned __int64)sScale >= (unsigned __int64)lPrecUsed) );	//See function comments for __int64 use

			// Fill right side of the decimal point with the see
			wszData[0] = L'0';
			wszData[1] = L'.';

			// pad out with zeros at the beginning			
			for(ivalue=0; (SHORT)ivalue<sScale-(SHORT)lPrecUsed;ivalue++)
				wszData[2+ivalue] = L'0';

			wszData[sScale-(SHORT)lPrecUsed+2] = L'\0'; //+2 is for the decimal point char and leading zero

			while(wcslen(wszData+(sScale-(SHORT)lPrecUsed)+2) < (size_t)lPrecUsed)
			{
				iLenLeft = lPrecUsed - wcslen(wszData+(abs(sScale-(INT)lPrecUsed)+2));
			
				if (iColLen <= iLenLeft)
					wcscat(wszData,wszCol);
				else
				{
					wcsncat(wszData,wszCol,(size_t)iLenLeft);
					wszData[sScale+2] = L'\0';	//+2 is for the decimal point char and leading zero
				}
			}
		}
	}
	else
	{
		// If the first column happens to by DBTYPE_BYTES, then we have to make
		// sure there are two chars since it takes two chars to make one binary
		// digit.
		if (wType == DBTYPE_BYTES)
			// Print number left padded with 0's to precision
			swprintf(wszData,L"%0*u",lPrecision, ulRowNum);
		else
			swprintf(wszData,L"%u", ulRowNum);

		lenwszData = wcslen(wszData);
		if(lPrecision < lenwszData)
			wszData[lPrecision] = L'\0';

		//THIS CODE IS REQUIRED FOR CORRECT DATA COMPARISION OF NUMERICS!
		//Since we insert a numeric of a particular scale, we also need to be
		//able to create data with a particular scale.  The only way to represent
		//scale in a string, is to use a "." with following "0000"...
		//IE:  1.0000 represent a numeric of precision=5, sign=1, and scale=4!

		// first column needs to know what the scale is in
		// order to compare with CompareData
        if (sScale > 0 && (unsigned __int64)sScale <= (unsigned __int64)lPrecision ) //See function comments for __int64 use
		{
			wszData[lenwszData] = L'.';
			wszData[lenwszData + 1] = L'\0';
			lenwszData = wcslen(wszData);

			for(ivalue=0;ivalue<sScale;ivalue++)
				wszData[lenwszData+ivalue]=L'0';

			wszData[lenwszData+ivalue]=L'\0';
		}

		return S_OK;
	}

	// Right side from decimal point
	// Fill right side of the decimal point with the seed
    if ( (unsigned __int64)lPrecUsed > (unsigned __int64)sScale && sScale > 0)	//See function comments for __int64 use
	{		
		wszData[lPrecUsed - sScale] = L'.';
		wszData[(lPrecUsed - sScale) + 1] = L'\0';

		while(wcslen(wszData) < (size_t)(sScale + 1 + lPrecision))
		{
			iLenLeft = (sScale + 1 + lPrecision) - wcslen(wszData);
			if (iColLen <= iLenLeft)
			{
				wcscat(wszData,wszCol);
			}
			else
			{
				wszCol[iLenLeft]=L'\0';
				wcsncat(wszData,wszCol,(size_t)iLenLeft);
			}
		}				
	}
	else if (sScale < 0)
	{
		// pad out with zeros at the end
		wszData[lPrecUsed] = L'\0';

		for(ivalue=0; ivalue<abs(sScale);ivalue++)
			wszData[lPrecUsed + ivalue] = L'0';

		wszData[(SHORT)lPrecUsed + ivalue] = L'\0';
	}
	

	// Remove extra zero's that are not needed after the decimal point
	// Data types float and double
	if ((ulRowNum == 1) && (wType == DBTYPE_R8))
	{
		while(wcslen(wszData) > (size_t)(lPrecision + 2))
		{
			pwszSingleChar=wcsrchr(wszData,L'0');
			iLenLeft = pwszSingleChar - wszData;
			wcsncat(wszData,wszData,(size_t)(iLenLeft-1));
			wszData[iLenLeft-1] = L'\0';
		}
	}

	// Every third row, starting with row two, will have negative numeric values
	if ((ulRowNum % 3 == 1) && 0 ==GetUnsigned())
	{
		if (IsNumericType(wType))
		{
			wcscpy(wszBuffer,L"-");
			wcscat(wszBuffer,wszData);
			wcscpy(wszData,wszBuffer);
		}
		else
			wszData[0]=L'-';
	}

	// Special cases
	switch(wType) 
	{
		// Data types binary, varbinary, longvarbinary
		case DBTYPE_BYTES:
			if (ulRowNum > 0)
				wszData[0]=L'F';
			break;

		// Add a H to the front of all char types
		// Data types character, varchar, longvarchar
		// If International Data flag is set, create the data differently using
		// locale-specific characters.
		case DBTYPE_BSTR:		
		case DBTYPE_STR:	
			if ( GetModInfo()->GetLocaleInfo() ) 
			{
				void *pvAnsiString = NULL;              

				GetModInfo()->GetLocaleInfo()->SetAnsiSeed((INT)(ulRowNum + GetColNum()));
				if (GetIsNewLongType())
				{
					DBLENGTH iDSSize = (DBLENGTH) MAX_LONG_COL_SIZE;
					pvAnsiString = PROVIDER_ALLOC(iDSSize+sizeof(char));		
					if (pvAnsiString == NULL)
					{
						return E_OUTOFMEMORY;						 
					}
					GetModInfo()->GetLocaleInfo()->MakeAnsiIntlString((char *)pvAnsiString, (INT)(iDSSize+1));
					MultiByteToWideChar(CP_ACP, 0, (char *)pvAnsiString, -1, wszData, (INT)(iDSSize+1));
				}
				else
				{
					pvAnsiString = PROVIDER_ALLOC(lPrecision+sizeof(char));	
					if (pvAnsiString == NULL)
					{
						return E_OUTOFMEMORY;						 
					}

					GetModInfo()->GetLocaleInfo()->MakeAnsiIntlString((char *)pvAnsiString, (INT)(lPrecision+1));
					MultiByteToWideChar(CP_ACP, 0, (char *)pvAnsiString, -1, wszData, (INT)(lPrecision+1));
				}

				PROVIDER_FREE(pvAnsiString);
			} 
			else 
			{
				if(ulRowNum > 0)
					wszData[0]=L'H';
			}
			break;
		// If the international flag is set, generate International data, regardless of
		// the client's locale
		case DBTYPE_WSTR:
			if ( GetModInfo()->GetLocaleInfo() ) 
			{
				GetModInfo()->GetLocaleInfo()->SetUnicodeSeed((INT)(ulRowNum + GetColNum()));

				if (GetIsNewLongType())
				{
					DBLENGTH iDSSize = (DBLENGTH) MAX_LONG_COL_SIZE;
					GetModInfo()->GetLocaleInfo()->MakeUnicodeIntlData((WCHAR *)wszData, (INT)(iDSSize+1));
				}
				else 
				{
					GetModInfo()->GetLocaleInfo()->MakeUnicodeIntlData((WCHAR *)wszData, (INT)(lPrecision+1));
				}
			} 
			else 
			{
				if(ulRowNum > 0)
					wszData[0]=L'H';
			}
			break;

		// Data type bigint
		// Max bigint (DBTYPE_UI8) value is 18446744073709551616 (0xffffffffffffffff)
		case DBTYPE_UI8:
		{
			// If value is greater than max precision for UI64 truncate
			if (wcslen(wszData) >= 20)
			{
				WCHAR wszMax[20];
		
				wszData[20] = L'\0';//Maximum Precision

				// Make a maximum string value
				_ui64tow(_UI64_MAX, (LPWSTR)wszMax, 10);

				// If the value is greater than the max string value then remove last char
				if (wcscmp(wszData, wszMax) > 0)
					wszData[(wcslen(wszData))-1]=L'\0';
			}

			break;
		}
		// Max bigint (DBTYPE_I8) value is  9223372036854775807
		// Min bigint (DBTYPE_I8) value is -9223372036854775807 - 1
		case DBTYPE_I8:
		{
			WCHAR wszMax[21];
			LPWSTR pwszData = wszData;
			BOOL fNegative = FALSE;

			// Skip over any minus sign
			if (*pwszData == '-')
			{
				pwszData++;
				fNegative = TRUE;
			}

			// If value is greater than max precision for I64 truncate
			if (wcslen(pwszData) >= 19)
			{
		
				pwszData[19] = L'\0';//Maximum Precision

				if (!fNegative)
					// Make a maximum string value
					_i64tow(_I64_MAX, (LPWSTR)wszMax, 10);

				else
					// Make a minimum string value
					_i64tow(_I64_MIN, (LPWSTR)wszMax, 10);

				// If the value is greater than the appropriate max string value then remove last char
				if (wcscmp(wszData, wszMax) > 0)
					wszData[(wcslen(wszData))-1]=L'\0';

			}

			break;
		}


		// Data type integer
		// For "int" max number is 2147483647
		case DBTYPE_UI4:
		case DBTYPE_I4:
			wszData[10] = L'\0';//Maximum Precision
			if (ulRowNum > 1)
				wszData[(wcslen(wszData))-1]=L'\0';
			break;

		// Data type tinyint
		// For "tinyint" max number is 255
		case DBTYPE_UI1:
		case DBTYPE_I1:
			wszData[3] = L'\0';//Maximum Precision
			ivalue = _wtoi(wszData);
			
			if (wType == DBTYPE_UI1) 
				swprintf(wszData,L"%u", ivalue % 0xFF);
			else
				swprintf(wszData,L"%i", -1 * (ivalue % 0x7F));
			break;

		// Data type smallint
		// For "smallint" max num in 32,767 or -32,768
		case DBTYPE_UI2:
		case DBTYPE_I2:
			wszData[5] = L'\0';//Maximum Precision
			lvalue = _wtol(wszData);
			
			if (lvalue > 32767)
				swprintf(wszData,L"%lu", lvalue % 32767);
			
			if(lvalue < -32768)
				swprintf(wszData,L"%li", -((-lvalue) % 32768));
			break;

		// Data type decimal
		// There are three decimal data types with different precisions.
		// For "smallmoney" max number is 214748.3647
		case DBTYPE_CY:
		case DBTYPE_NUMERIC:
			if ((ulRowNum > 1) && (lPrecision < 11) && ((DB_LORDINAL)lPrecUsed != sScale))
			{
				// If negative number
				if ((ulRowNum % 3 == 1) && !(GetUnsigned()))
					wszData[1]=L'1';
				else
					wszData[0]=L'1';
				break;
			}

			// For "money" max number is 922337203685477.5807
			if (lPrecision == 15)
			{
				// For a negative value take the first three characters, which
				// includes the negative sign. For positive numbers take the
				// first two char's.
				if ((ulRowNum % 3 == 1) && !(GetUnsigned()))
				{
					wcsncpy(wszCol,wszData,3);
					wszCol[3] = L'\0';
				}
				else
				{
					wcsncpy(wszCol,wszData,2);
					wszCol[2] = L'\0';
				}

				lvalue = _wtol(wszCol);

				// If greater than 91 or less than -91 then the number is not
				// valid for money datatype.
				if ((lvalue > 91) || (ulRowNum == 9) || (lvalue < -91))
				{
					// If negative number
					if ((ulRowNum % 3 == 1) && !(GetUnsigned()))
						wszData[2]=L'1';
					else
						wszData[1]=L'1';
				}
			}
			break;
	}

	return S_OK;
}


//---------------------------------------------------------------------------
// CCol::CreateColDef
// Create column definition. Returns a string,
// that includes column name, date type, precision to be used, and null, 
// should look like:"colname CHAR (10) NULL" or "colname DECIMAL (10,5) NULL".
// Client is responsible for releasing memory of pwszColumnDefinition.
//	
// @mfunc void|CCol|CreateColDef|
//	
//---------------------------------------------------------------------------
void CCol::CreateColDef(
	WCHAR ** wszColDef		// @parm WCHAR **|wszColDef|[OUT] SQL column definition,
							// *wszColDef should be = NULL.
)
{
	size_t	ulSize	= 0;
	WCHAR * pwch;
	
	// Get Parts of Col Def
	ulSize = wcslen(wszLEFTPAREN) + (2*MAX_ULONG) + wcslen(wszCOMMA) +
			 (2*MAX_ULONG) + wcslen(wszRIGHTPAREN);
	ulSize *= sizeof(WCHAR);
	ulSize += sizeof(WCHAR);

	WCHAR * pwszPrecision = (WCHAR *)PROVIDER_ALLOC(ulSize);
	pwszPrecision[0] = L'\0';

	WCHAR wszPrec[MAX_ULONG];
	wszPrec[0] = L'\0';
	
	WCHAR wszScale[MAX_ULONG];
	wszScale[0] = L'\0';

	//Make sure there is a column name
	ASSERT(wcslen(GetColName()));

	// Build Precision wszPrecision should look like: "(x)" or "(x,y)"
	if (m_pwszCreateParams)
	{											
		// Left paren
		wcscpy(pwszPrecision, wszLEFTPAREN);
		swprintf(wszPrec, L"%u", GetMaxSize());
		wcscat(pwszPrecision, wszPrec);
		
		// If the create params retrieved from the server has a comma
		// in it then there must be two parameters
		if (wcsstr(m_pwszCreateParams, L","))
		{
			swprintf(wszScale, L"%u", GetScale());
			wcscat(pwszPrecision, wszCOMMA);
			wcscat(pwszPrecision, wszScale);
		}

		wcscat(pwszPrecision,wszRIGHTPAREN);
	}

	// Allocate memory for string
	ulSize = wcslen(GetColName()) + wcslen(wszSPACE) + 
			 wcslen(m_pwszProviderTypeName) + wcslen(pwszPrecision) + 
			 wcslen(wszSPACE) +	wcslen(wszNOTNULL);
	
	ulSize *= sizeof(WCHAR);
	ulSize += sizeof(WCHAR);

	WCHAR * pwszColumnDefinition = (WCHAR *)PROVIDER_ALLOC(ulSize);

	// Fill string
	wcscpy(pwszColumnDefinition, GetColName());
	wcscat(pwszColumnDefinition, wszSPACE);
	wcscat(pwszColumnDefinition, m_pwszProviderTypeName);	

	if (m_pwszCreateParams)
	{											
		// Check for paren's in the type name like "numeric() identity"
		if (pwch = wcschr(pwszColumnDefinition, ')')) 
		{
			pwch--;
			*pwch = '\0';
			wcscat(pwszColumnDefinition, pwszPrecision);
			wcscat(pwszColumnDefinition, (wcschr(m_pwszProviderTypeName, ')') + 1));
		} 
		else // Insert precision/scale after type name			
			wcscat(pwszColumnDefinition,pwszPrecision);
	}

	PRVTRACE(L"**privlib.dll (CCol::CreateColDef):'%s'\n",pwszColumnDefinition);
	*wszColDef = pwszColumnDefinition;

	PROVIDER_FREE(pwszPrecision);
}

//---------------------------------------------------------------------------
// CCol::SetColName
// Client is responsible for releasing memory 
//	
// @mfunc SetColName 
//	
//---------------------------------------------------------------------------
void CCol::SetColName(
	WCHAR * pwszColName	  // @parm [IN] Column name. 
)	
{
	PROVIDER_FREE(m_ColInfo.pwszName);
	m_ColInfo.pwszName = wcsDuplicate(pwszColName);
}

//---------------------------------------------------------------------------
// CCol::SetColID
// Client is responsible for releasing memory 
//	
// @mfunc SetColID
//	
//---------------------------------------------------------------------------
void CCol::SetColID(
	DBID* pColumnID	  // @parm [IN] Column ID
)	
{
	// try to copy whatever there is
	ReleaseDBID(GetColID(), FALSE);
	DuplicateDBID(*pColumnID, GetColID());
}

//---------------------------------------------------------------------------
// CCol::SetProviderTypeName,| Sets provider type name
// Client is responsible for releasing memory 
//	
// @mfunc SetProviderTypeName 
//	
//---------------------------------------------------------------------------
void CCol::SetProviderTypeName(
	WCHAR * pwszProviderTypeName	// @parm [IN] Provider type name.
)
{
	PROVIDER_FREE(m_pwszProviderTypeName);
	m_pwszProviderTypeName = wcsDuplicate(pwszProviderTypeName);
}

//---------------------------------------------------------------------------
// CCol::SetPrefix
// Client is responsible for releasing memory of pwszColumnDefinition.
//	
// @mfunc SetPrefix
//	
//---------------------------------------------------------------------------
void CCol::SetPrefix(
	WCHAR * pwszPrefix	// @parm [IN] Prefix.
)
{
	PROVIDER_FREE(m_pwszPrefix);
	m_pwszPrefix = wcsDuplicate(pwszPrefix);
}

//---------------------------------------------------------------------------
// CCol::void CCol::SetSuffix(const WCHAR * pwszSuffix,BOOL fReset=TRUE)
// Client is responsible for releasing memory of pwszColumnDefinition.
// Set data source's literal suffix
//	
// @mfunc SetSuffix
//	
//---------------------------------------------------------------------------
void CCol::SetSuffix(	
	WCHAR * pwszSuffix	   // @parm [IN] Suffix.
)
{
	PROVIDER_FREE(m_pwszSuffix);
	m_pwszSuffix = wcsDuplicate(pwszSuffix);
}

//---------------------------------------------------------------------------
// CCol::SetCreateParams
// Client is responsible for releasing memory of pwszColumnDefinition.
//	
// @mfunc SetCreateParams
//	
//---------------------------------------------------------------------------
void CCol::SetCreateParams(
	WCHAR * pwszCreateParams   // @parm [IN] See OLE DB schema rowset TYPES for explanation.
)
{
	PROVIDER_FREE(m_pwszCreateParams);
	m_pwszCreateParams = wcsDuplicate(pwszCreateParams);
}


//---------------------------------------------------------------------------
// CCol::GetMaxSize
// Used to determine memory allocation to use when creating bindings
//	
// @mfunc GetMaxSize
//	
//---------------------------------------------------------------------------
DBLENGTH CCol::GetMaxSize()
{
	if( IsNumericType(GetProviderType()) || GetProviderType()==DBTYPE_DBTIMESTAMP )
		return GetPrecision();
	else
		return GetMaxColumnSize();	
}


//---------------------------------------------------------------------------
// CCol::GetMaxColumnSize
// Gets data source's ColumnSize for this datatype
//	
// @mfunc GetMaxColumnSize
//	
//---------------------------------------------------------------------------
DBLENGTH CCol::GetMaxColumnSize(void)
{
	// If a test specificies a required parameter field,
	// then don't override it
	if( m_cReqParams > 0 && m_cReqParams != ULONG_MAX )
		return GetColumnSize();
	else
		return min(GetColumnSize(), MAX_COL_SIZE);
}


//---------------------------------------------------------------------------
// CCol::SetColDescription
// Sets the description of a column
//	
// @mfunc SetColDescription
//	
//---------------------------------------------------------------------------
void CCol::SetColDescription(WCHAR *pwszColDescription)
{
	SAFE_FREE(m_pwszColDescription);
	m_pwszColDescription = wcsDuplicate(pwszColDescription);
} //CCol::SetColDescription


//---------------------------------------------------------------------------
// CCol::SetLCID
// Sets the collating sequence LCID of a column
//	
// @mfunc SetLCID
//	
//---------------------------------------------------------------------------
void CCol::SetLCID(LONG lLCID)
{
	ASSERT(lLCID >= 0);

	m_ulLCID = ULONG(lLCID);
}


//---------------------------------------------------------------------------
// CCol::GetVariantType
// Returns the variant type data that will be used for a particular row seed
//	
// @mfunc GetVariantType
//	
//---------------------------------------------------------------------------
DBTYPE CCol::GetVariantType
(
	DBORDINAL	ulRowSeed,
	CCol &	rCol		
)
{
	CCol		col;
	BOOL		fFound	= FALSE;
	DBORDINAL	ulCur	= 0;

	rCol.SetProviderType(DBTYPE_EMPTY);

	if( m_pSchema )
	{
		DBORDINAL	cCols = m_pSchema->CountColumnsOnSchema();

		// Look through the list for valid variant candidates
		ulCur = (ulRowSeed % cCols)+1;

		do
		{			
			TESTC_(m_pSchema->GetColInfo(ulCur, col), S_OK);
					
			if( IsVariantCompatible(col.GetProviderType(), &col) )
			{
				fFound = TRUE;
				break;
			}
			
			if( ++ulCur > cCols )
				ulCur = 1;			
				
		} while ( ulCur != ulRowSeed );
	}
	
CLEANUP:

	if( fFound )
	{
		rCol = col;
		return col.GetProviderType();
	}
	else
		return g_rgDefaultSubTypes[ulRowSeed%NUMELEM(g_rgDefaultSubTypes)];
}


//---------------------------------------------------------------------------
// CCol::GetVariantPrefix
// Returns the variant prefix corresponding to a particular row seed
//	
// @mfunc GetVariantPrefix
//	
//---------------------------------------------------------------------------
WCHAR * CCol::GetVariantPrefix
(
	DBCOUNTITEM ulRowSeed
)
{
	DBTYPE	wVariantSubType;
	CCol	col;

	wVariantSubType = GetVariantType(ulRowSeed, col);

	if( col.GetProviderType() != DBTYPE_EMPTY )
	{
		CCol &NewCol = m_pSchema->GetColInfoForUpdate(col.GetColNum());
		
		ASSERT( NewCol.GetProviderType() == wVariantSubType ) ;
		
		return NewCol.GetPrefix();		
	}

	return NULL;
}


//---------------------------------------------------------------------------
// CCol::GetVariantSuffix
// Returns the variant suffix corresponding to a particular row seed
//	
// @mfunc GetVariantSuffix
//	
//---------------------------------------------------------------------------
WCHAR * CCol::GetVariantSuffix
(
	DBCOUNTITEM ulRowSeed
)
{
	CCol	col;
	DBTYPE	wVariantSubType;

	wVariantSubType = GetVariantType(ulRowSeed, col);

	if( col.GetProviderType() != DBTYPE_EMPTY )
	{	
		CCol &NewCol = m_pSchema->GetColInfoForUpdate(col.GetColNum());

		ASSERT( NewCol.GetProviderType() == wVariantSubType );
				
		return NewCol.GetSuffix();		
	}
		
	return NULL;
}


//---------------------------------------------------------------------------
// IsCColNullable
// Gets the nullability of a column
//	
// @mfunc IsCColNullable
//	
//---------------------------------------------------------------------------
BOOL IsCColNullable(CCol *pCol)
{
	return 0 != pCol->GetNullable();
}


//---------------------------------------------------------------------------
// IsCColNotNullable
// Gets the non-nullability of a column
//	
// @mfunc IsCColNotNullable
//	
//---------------------------------------------------------------------------
BOOL IsCColNotNullable(CCol *pCol)
{
	return 0 == pCol->GetNullable();
}

