 
#define INC_OLE2
#define UNICODE 1
#define _WIN32_DCOM

#include <windows.h>
#include <winuser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <winldap.h>

#include <activeds.h>
#include <assert.h>

#include "CreateUserHelpers.h"
#include "UserProps.h"


//#include "AttributesList.h"
#define MAX_ATTRIBS 4096

////////////////////////////////////////////////////////////////////////////////////////////////////
// String for usage information
WCHAR * pwszUsage = L"Creates new User under the passed container.\n"
                    L"Usage:\n"
                    L"CreateUser\n"
                    L"        <*> Simple New User Information <*>\n"
                    L"            /LDAP <Path of container>\n" 
                    L"              ADsPath of the container for placing the new group\n"
                    L"           /UNAME <NT 5 User Name> \n"
                    L"              This is the name for the new group\n"
                    L"           /SAMNAME <Downlevel NT 4 Sam Account name>\n"
                    L"              Cannot exceed 20 characters and must be globally unique\n"
                    L"\n"
                    L"        <*> Detailed New User information <*>\n"
                    L"            /FILE < Info File >\n"
                    L"              Filename for file to contain new user information\n"
                    L"\n"
                    L"        <*> OPTIONAL Binding Information <*>\n"
                    L"           /USER <User name used for binding to the DC>\n"
                    L"           /PASS <Password used for binding to the DC>\n"
                    L"           (If these are passed, binding is done with ADsOpenObject())\n"
                    L"\n"         
                    L"\n";


// Sample command line:

//    /LDAP "LDAP://nttest.microsoft.com/CN=UGExercise Sample Container,dc=nttest,dc=microsoft,dc=com" /UNAME BOB /SAMNAME BOB /USER Administrator /PASS ""

////////////////////////////////////////////////////////////////////////////////////////////////////
// Module level global variables

BSTR bsLDAP     = NULL; // <Path of container> ADsPath of the container for placing the new group
    
BSTR bsUNAME    = NULL; // <NT 5 User Name> This is the name for the new group
    
BSTR bsSAMNAME  = NULL; // <Downlevel NT 4 Sam Account name> Cannot exceed 20 characters and must be globally unique

BSTR bsFILE     = NULL; // < Info File > Filename for file to contain new user information

BSTR bsUSER     = NULL; // <User name used for binding to the DC>
    
BSTR bsPASS     = NULL; // <Password used for binding to the DC>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
BOOL    ParseCommandLine( int argc, wchar_t *argv[ ]);
HRESULT CreateUser(IDirectoryObject *pDirObject, LPWSTR pwCommonName,LPWSTR pwSamAcctName,IDirectoryObject ** ppDirObjRet);
void    Trim(LPWSTR pwszData);
HRESULT CreateUserFromFile(IDirectoryObject *pDirObject, LPWSTR pwCommonName,LPWSTR pwSamAcctName,IDirectoryObject ** ppDirObjRet,LPWSTR pwszFileName);
BOOL    GetNextLine(FILE* fp,LPWSTR pwszDest,int iSize);
void    RemoveSpecialChars(LPWSTR pwszLine);

////////////////////////////////////////////////////////////////////////////////////////////////////
// main()

/* Note: Using the UNICODE version of main().
   this removes the need for the sample to include
   UNICODE-ANSI conversion routines
*/
void wmain( int argc, wchar_t *argv[ ])
{
    WCHAR pwszTemp[4096];
    
    // We have now scanned PAST whitespace- so copy the string:
    wcscpy_s(pwszTemp,4096,L" A String");

    Trim(pwszTemp);

    HRESULT hr;
    IDirectoryObject * pDirObjectContainer = NULL;
    IDirectoryObject * pDirObjRet = NULL;

    if (!ParseCommandLine(argc,argv))
        return;

    // Initialize COM
    CoInitialize(0);
 
    // Bind to the container passed 
    // If USER and PASS passed in, use ADsOpenObject()
    if (bsUSER)
        hr = ADsOpenObject(bsLDAP, bsUSER, bsPASS, 
                                   ADS_SECURE_AUTHENTICATION,IID_IDirectoryObject, (void**) &pDirObjectContainer);
    else
        hr = ADsGetObject(  bsLDAP, IID_IDirectoryObject,(void **)&pDirObjectContainer);
    
    if (SUCCEEDED(hr))
    {
        // if a file is NOT passed in- Do the simple version
        if (!bsFILE)
        {
            // Call the helper funtion to create the User
            hr = CreateUser(pDirObjectContainer, bsUNAME,bsSAMNAME,
                             &pDirObjRet);
        }
        else // file was passed in
        {
            // Call the helper funtion to create the User
            hr = CreateUserFromFile(pDirObjectContainer, bsUNAME,bsSAMNAME,
                                    &pDirObjRet,bsFILE);
        }        
        if (SUCCEEDED(hr))
        {
            _putws(L"\n\n New User created with the following properties:\n");
        
            IADs * pIADsNewGoup = NULL;
        
            // User succeeded- now get an IADs interface to it 
            // and print some properties
            hr = pDirObjRet->QueryInterface(IID_IADs,(void**)&pIADsNewGoup);

            if (SUCCEEDED(hr))
            {
                PrintIADSObject(pIADsNewGoup);
                        
                pIADsNewGoup->Release();
                pIADsNewGoup = NULL;
            }
            else
                CheckADHRESULT(hr,L"QueryInterface() - New User for IADs");
            pDirObjRet->Release();
            pDirObjRet = NULL;    
        }
        else
            CheckADHRESULT(hr,L"CreateUser()");

        pDirObjectContainer->Release();
        pDirObjectContainer = NULL;    
    }
    else
        if (bsUSER)
            CheckADHRESULT(hr,L"ADsOpenObject()");
        else
            CheckADHRESULT(hr,L"ADsGetObject()");


	if ( bsLDAP )
		::SysFreeString(bsLDAP);
	if ( bsUNAME )
		::SysFreeString(bsUNAME);
	if ( bsSAMNAME )
		::SysFreeString(bsSAMNAME);
	if ( bsFILE )
		::SysFreeString(bsFILE);
	if ( bsUSER )
		::SysFreeString(bsUSER);
	if ( bsPASS )
		::SysFreeString(bsPASS);

    CoUninitialize();
 }


////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    ParseCommandLine()- Parses command line and sets module level globals
    
    Parameters:

        int argc            - Number of Arguments
        wchar_t *argv[ ]    - Array of arguments

    Returns:
        TRUE if Command line was successfully parsed

    Affectets global variables:

        BSTR bsLDAP    
        BSTR bsUNAME   
        BSTR bsSAMNAME 
        BSTR bsFILE    
        BSTR bsUSER    
        BSTR bsPASS    
*/    

BOOL ParseCommandLine( int argc, wchar_t *argv[ ])
{
/*
     ** PARSE the FOLLOWING ARGUMENTS:
    /LDAP <Path of container>
        ADsPath of the container for placing the new group
    
    /UNAME <NT 5 User Name>
        This is the name for the new group
    
    /SAMNAME <Downlevel NT 4 Sam Account name>
        Cannot exceed 20 characters and must be globally unique

    <*> Detailed New User information <*>
    /FILE < Info File >
    
        Filename for file to contain new user information

    <*> OPTIONAL Binding Information <*>
    /USER <User name used for binding to the DC>
    
    /PASS <Password used for binding to the DC>
    (If these are passed, binding is done with ADsOpenObject())
*/        
	if (argc == 1)
	{
        _putws(pwszUsage);
        return FALSE;
    }
    for (int x= 1; x < argc; x++)
    {
        if (_wcsicmp(argv[x],L"/LDAP")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsLDAP = SysAllocString(argv[x]);                
        }
        else if (_wcsicmp(argv[x],L"/UNAME")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsUNAME = SysAllocString(argv[x]);                
        }
        else if (_wcsicmp(argv[x],L"/SAMNAME")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsSAMNAME = SysAllocString(argv[x]);                
        }
        else if (_wcsicmp(argv[x],L"/FILE")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsFILE = SysAllocString(argv[x]);                
        }
        else if (_wcsicmp(argv[x],L"/USER")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsUSER = SysAllocString(argv[x]);                
        }
        else if (_wcsicmp(argv[x],L"/PASS")==0)
        {
            if (argc == x) // Make sure the parameter was passed
            {
                wprintf(L"\nERROR: %s Missing parameter!!!!\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
            }
            // Go to the next argument and save it in module level variable
            x++;
            bsPASS = SysAllocString(argv[x]);                
        }
        else
        {
                wprintf(L"\nERROR: %s UNknown Argument\n\n",argv[x]);
                _putws(pwszUsage);
                return FALSE;
        }
    }
    // Check if User is poassed, then password is required:
    if (bsUSER || bsPASS)
        if (!bsUSER || !bsPASS) // if either one is missing complain
        {
            _putws(L"\nERROR: If /USER is specified /PASS is required!");
            return FALSE;
        }                    
      
    return TRUE;
}


HRESULT ConvertUTCFromStringToUTCTime( LPWSTR pwszTime, SYSTEMTIME *pst)
{
    FILETIME ft;
    TCHAR sz[3];
    LPWSTR pszSrc = pwszTime;
    SYSTEMTIME st;

    // Year
    sz[0] = pszSrc[0];
    sz[1] = pszSrc[1];
    sz[2] = TEXT('\0');
    st.wYear = (WORD)_wtoi(sz);
    if (st.wYear < 50)
    {
        st.wYear += 2000;
    }
    else
    {
        st.wYear += 1900;
    }
    // Month
    sz[0] = pszSrc[2];
    sz[1] = pszSrc[3];
    st.wMonth = (WORD)_wtoi(sz);
    
    // Day
    sz[0] = pszSrc[4];
    sz[1] = pszSrc[5];
    st.wDay = (WORD)_wtoi(sz);

    // Hour
    sz[0] = pszSrc[6];
    sz[1] = pszSrc[7];
    st.wHour = (WORD)_wtoi(sz);

    // Minute
    sz[0] = pszSrc[8];
    sz[1] = pszSrc[9];
    st.wMinute = (WORD)_wtoi(sz);

    // Second
    sz[0] = pszSrc[10];
    sz[1] = pszSrc[11];
    st.wSecond = (WORD)_wtoi(sz);
    st.wMilliseconds = 0;

    // This gets us the day of week
    SystemTimeToFileTime(&st, &ft);
    FileTimeToSystemTime(&ft, &st);
    *pst = st;
    return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/*  GetNextLine()   - Retrives a line from the passed file pointer
    
    Parameters

        FILE* fp        - File Pointer
        LPWSTR pwszDest - Destination Buffer
        int iSize       - Size of Destination Buffer 
*/

BOOL GetNextLine(FILE* fp,LPWSTR pwszDest,int iSize)
{
	//BOOL bRet;
	WCHAR c =0 ;
	int x = 0;
	do 
	{
		if (x < iSize )
        {
            // Get a character
            c = getwc(fp);
		    // Handle the end
            if (x ==0 && ((char)c) == EOF )
			    return FALSE;
                	
		    // If it is NOT a carriage return - save the character
            // (note the cast to (char): as EOF is a char -1.
            // with a WIDE character this becomes 65535)
            // so we cast DOWN the char to test for the potential -1
            if (c!= '\r' && ((char)c) != EOF && c != '\n')
			    pwszDest[x++] = c;
	        else
                break;
        }			
	} while (c != '\r' && c != EOF && c != '\n' && c !=65535);

	pwszDest[x] = NULL;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/*  Trim()   - VERY simple function for removing left and right whitespace from a unicode string
    
    Parameters

        LPWSTR pwszData                 -   Data passed for trimming- Function will modify
*/
void Trim(LPWSTR pwszData)
{
    int iscan=0;
    int iput=0;

    int iLenData = wcslen(pwszData);
    LPWSTR pwszTemp = new WCHAR[iLenData+1];
    int iLenTemp;
        
    // Trim left:
    while (pwszData[iscan] != NULL && (pwszData[iscan] == ' ' || pwszData[iscan] == '\t' ))
        iscan++;
    
    // We have now scanned PAST whitespace- so copy the string:
    wcscpy_s(pwszTemp,iLenData+1,&pwszData[iscan]);
    
    // Now we need to do the TrimRight
    iLenTemp = wcslen(pwszTemp);
    
    for (iscan = iLenTemp-1; iscan; iscan--)
    {
        if (!(pwszTemp[iscan] == ' ' || pwszTemp[iscan] == '\t' ) )
            break;
    }
    pwszTemp[iscan+1] =NULL;
    wcscpy_s(pwszData,iLenData,pwszTemp);            
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  RemoveSpecialChars()   - Removes Tabs from passed string
    
    Parameters

    LPWSTR pwszLine         - Target String   
*/
void RemoveSpecialChars(LPWSTR pwszLine)
{
    int iLen = wcslen(pwszLine);
    for (int x = 0; x < iLen; x++)
    {
        switch (pwszLine[x])
        {
            case '\t':
                pwszLine[x] = ' '; // convert tabs to spaces..
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  ReadDataFromLine()   - Reads the Attribute and the Value from a passed line of text
    
    Parameters

        LPWSTR pwszLine         - Line to read
        BSTR * pbsAttrib        - Attribute NAME to return
        BSTR * pbsValue         - Value of the attribute
*/
HRESULT ReadDataFromLine(LPWSTR pwszLine,BSTR * pbsAttrib, BSTR * pbsValue)
{
    int x;
    int iLen = wcslen(pwszLine);    

    *pbsAttrib = NULL;
    *pbsValue  = NULL;

    // Remove any tabs still in the line:
    RemoveSpecialChars(pwszLine);
    
    // If this line is a comment or blank, bail..
    if (pwszLine[0] == L'/' || iLen == 0)
        return S_FALSE;
        
    // Make a temp string to save our string 
    LPWSTR pwszTempAttrib = new WCHAR [iLen +1];
	if ( !pwszTempAttrib )
		return E_FAIL;

    pwszTempAttrib[0]= NULL;

    for (x = 0; x< iLen; x++)
        switch(pwszLine[x])
        {
             case L' ':
             case L'\t':
             {
                // NULL terminate the copy we are building
                pwszTempAttrib[x] = L'\0';
                x++;
                goto DoneReadDataFromLine;  
                break;
             }
             case NULL:    // if we hit the end of the string- there is NOT a value there
             {
                wprintf(L"\n!!!Attribute missing value [%s]\n",pwszLine);
				delete [] pwszTempAttrib;
                return E_FAIL;
             }
             default:
             {
                pwszTempAttrib[x] = pwszLine[x];   
             }
        }

DoneReadDataFromLine:
    // Trim off any preceding spaces..
    Trim((LPWSTR)pwszLine +x);

    // Grab the END of the passed line, and put it in the 
    // Value BSTR
    *pbsValue = SysAllocString((LPWSTR)pwszLine +x);
    
    // Alloc a new BSTR to return for this attribute
    *pbsAttrib= SysAllocString(pwszTempAttrib);
    if (iLen)
        delete [] pwszTempAttrib ;   
    return S_OK;         
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  MakeOctetADSVALUE()   - Reads the passed octet string into the passed ADSVALUE,
                            Sample String:
                                    x00 x00 x00 x00 xe0 xff x03 xe0 xff x03 xe0 xff x03 xe0 xff x03 xe0 xff x03 x00 x00
                            The "x"'s are required and the values must be in HEX format
    
    Parameters

    ADSVALUE *  pAdsValue    - Returned ADSVALUE of Type OctetString
    LPWSTR      pwszOctet    - String representing OCTET

*/
HRESULT MakeOctetADSVALUE(ADSVALUE * pAdsValue,LPWSTR pwszOctet)
{
    int iNumBytes = 0;
    int iStrLen = wcslen(pwszOctet);
    
    // Count the number of BYTES in the String
    for (int x =0;x <iStrLen;x++)
        if (pwszOctet[x] == L'x' || pwszOctet[x] == L'X')
            iNumBytes ++;

    if (iNumBytes)
    {
        int iCurrByte =0;

        // Allocate the bytes
        pAdsValue->OctetString.lpValue = new BYTE [iNumBytes+1];
        pAdsValue->OctetString.dwLength = iNumBytes;

        for (int x =0;x <iStrLen;x++)
            if (pwszOctet[x] == L'x' || pwszOctet[x] == L'X')
            {
                x++;
                int iVal;
                // found an x so grab the data
                swscanf_s(  &pwszOctet[x], L"%x",&iVal);
                pAdsValue->OctetString.lpValue[iCurrByte++]= iVal;                      
            
                // scan past the rest of the string
                for (x =x;x<iStrLen && pwszOctet[x] != (WCHAR)' ' ;x++)
                    ;
            }
    }
    else 
        return E_FAIL;
    
    return S_OK;             
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  PopulateADSAttrInfo()   - Populates the passed ADS_ATTR_INFO from the passed attribute and Value
    
    Parameters

    ADS_ATTR_INFO * pattrInfo   - Pointer to Element to be added
    LPWSTR pwszAttrib           - Name of the Attribute
    LPWSTR pwszValue            - Value of the Attribute

*/
BOOL PopulateADSAttrInfo(ADS_ATTR_INFO * pattrInfo,LPWSTR pwszAttrib,LPWSTR pwszValue)
{
    // Look up the attribute name and get the TYPE string for this attribute
    LPWSTR pwszType;
    LPWSTR  pwszSingleMulti;
    BOOL bAttribIsSingle = TRUE;

    BOOL bret;
    
    bret = MapUserAttribToType(pwszAttrib,&pwszType,&pwszSingleMulti);
    if (!bret)
    {
        wprintf(L"\n!!!Error: unable to map %s\n",pwszAttrib);
        return FALSE;
    }

    if (_wcsicmp(pwszSingleMulti,L"MV")==0)
        bAttribIsSingle = FALSE;
    else
        bAttribIsSingle = TRUE;        

    // Look up the string and get the corrosponding ADSVALUE...
    ADSTYPE adType = MapTypeToADSTYPE(pwszType);

    if (adType == ADSTYPE_UNKNOWN)
    {
        wprintf(L"\n!!!Error: unable to map %s ADS_TYPE unknown\n",pwszAttrib);
        return FALSE;
    }
    // FIll in the Attribute Name
	unsigned int attrNameLen = wcslen(pwszAttrib)+1;
    pattrInfo->pszAttrName = new WCHAR [attrNameLen];
    wcscpy_s(pattrInfo->pszAttrName,attrNameLen,pwszAttrib);
    pattrInfo->dwControlCode  = ADS_ATTR_UPDATE;
    pattrInfo->dwADsType = ADSTYPE_CASE_IGNORE_STRING;

    pattrInfo->pADsValues = new ADSVALUE;
    pattrInfo->pADsValues->dwType = adType;
    pattrInfo->dwNumValues = 1;
    
    // Fill in the VALUE:
    switch (adType )
    {

        case ADSTYPE_DN_STRING:	            
        case ADSTYPE_CASE_EXACT_STRING:	    
        case ADSTYPE_CASE_IGNORE_STRING:	    
        case ADSTYPE_PRINTABLE_STRING:	    
        case ADSTYPE_NUMERIC_STRING:	        
        case ADSTYPE_TYPEDNAME:	            
        case ADSTYPE_FAXNUMBER:	            
        case ADSTYPE_PATH:	                
        case ADSTYPE_OBJECT_CLASS:
        {
            pattrInfo->pADsValues->CaseIgnoreString = new WCHAR [wcslen(pwszValue)+1];
            wcscpy_s(pattrInfo->pADsValues->CaseIgnoreString,wcslen(pwszValue)+1,pwszValue);

            break;
        }
        case ADSTYPE_BOOLEAN:
            if (_wcsicmp(pwszValue,L"TRUE")==0)
                pattrInfo->pADsValues->Boolean = 1;
            else if (_wcsicmp(pwszValue,L"FALSE")==0)
                pattrInfo->pADsValues->Boolean = 0;
            else if (_wtoi(pwszValue)>0)
                pattrInfo->pADsValues->Boolean = 1;
            else
                pattrInfo->pADsValues->Boolean = 0;
            break;
        case ADSTYPE_INTEGER:
        {
            pattrInfo->pADsValues->Integer = _wtoi(pwszValue);
            break;
        }
        case ADSTYPE_OCTET_STRING:	        
        {
            // Read the string into an actualy binary blob
            HRESULT hr = MakeOctetADSVALUE(pattrInfo->pADsValues ,pwszValue);    
            if (FAILED(hr))
                return FALSE;                    
            break;
        }                                                                        
        case ADSTYPE_UTC_TIME:	            
        {
            SYSTEMTIME pst;
            
            HRESULT hr = ConvertUTCFromStringToUTCTime( pwszValue,&pst );
            
            if (FAILED(hr))
                return FALSE;                    

            pattrInfo->pADsValues->UTCTime = pst;

            break;
        }
        case ADSTYPE_LARGE_INTEGER:	        
        {
            pattrInfo->pADsValues->dwType = ADSTYPE_LARGE_INTEGER;
            swscanf_s (pwszValue, L"%I64d", &pattrInfo->pADsValues->LargeInteger);
            break;
        }

        default:
        {
            wprintf(L"\n\n!!!ERROR Unrecognized Type for atrib: %s",pwszAttrib);
            return FALSE;
        }
    }
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/*  PopulateAttrInfoFromFile()   - Populates the passed ADS_ATTR_INFO from the passed filename
    
    Parameters

    LPWSTR pwszFileName         - file from which info is read
    ADS_ATTR_INFO * pattrInfo   - Pointer to Elements to be added
    DWORD &rdwNumPopulated      - Returned number populated
*/

BOOL PopulateAttrInfoFromFile(LPWSTR pwszFileName,ADS_ATTR_INFO * pattrInfo,DWORD &rdwNumPopulated )
{
    HRESULT hr = S_OK;
    BOOL    bRet = FALSE;
	FILE * fpInput = NULL;
    WCHAR  pwszLine[4096];
    rdwNumPopulated = 0;
	errno_t status = 0;

	if( (status = _wfopen_s( &fpInput, pwszFileName, L"r" )) == NULL )
	{
		wprintf(L"\nError Opening Input File:%s",pwszFileName);
		return FALSE;
	}

    while (GetNextLine(fpInput,pwszLine,4096) && SUCCEEDED(hr))
    {
        Trim(pwszLine);
        
        BSTR bsAttrib;
        BSTR bsValue;

        // Read the line into our BSTRS
        // if S_FALSE is returned, then we do not have a line
        // if E_FAIL is returned, then we have an error condition
        hr = ReadDataFromLine(pwszLine,&bsAttrib,&bsValue);

        if (hr == S_OK) 
        {
            wprintf(L" attrib:%s value:%s\n",bsAttrib,bsValue);
            
            // Take the attribute and value and add them to the Array
            if (!PopulateADSAttrInfo(&pattrInfo[rdwNumPopulated],bsAttrib,bsValue))
            {
                hr = E_FAIL;
                break;
            }

            SysFreeString(bsAttrib);
            SysFreeString(bsValue);

            rdwNumPopulated++;
        }
    }
	fclose(fpInput);
	return (SUCCEEDED(hr));
}




////////////////////////////////////////////////////////////////////////////////////////////////////
/*  CreateUserFromFile()   - Function for creating a basic User
    
    Parameters

        IDirectoryObject *pDirObject    -   Parent Directory Object for the new User
        LPWSTR pwCommonName             -   Common Name for the new User
        IDirectoryObject ** ppDirObjRet -   Pointer to the Pointer which will receive the new User
        int iUserType                  -   Bitflags for new User:
                                                                    ADS_User_TYPE_GLOBAL_User, 
                                                                    ADS_User_TYPE_DOMAIN_LOCAL_User, 
                                                                    ADS_User_TYPE_UNIVERSAL_User, 
                                                                    ADS_User_TYPE_SECURITY_ENABLED 
*/
HRESULT CreateUserFromFile(IDirectoryObject *pDirObject, LPWSTR pwCommonName,LPWSTR pwSamAcctName,IDirectoryObject ** ppDirObjRet,LPWSTR pwszFileName)
{
    // To simply things, allocate a fixed array:
    ADS_ATTR_INFO   attrInfo [MAX_ATTRIBS+1];

    HRESULT         hr = S_OK;
    DWORD           dwNumPopulated;
    LPDISPATCH      pDisp;
    *ppDirObjRet     = NULL;
    WCHAR           pwCommonNameFull[MAX_PATH*2];
    BOOL    bRet = FALSE;
    FILE * fpInput = NULL;
    WCHAR  pwszLine[4096];
    dwNumPopulated = 0;
	errno_t status = 0;

	if( (status = _wfopen_s( &fpInput, pwszFileName, L"r" )) == NULL )
	{
		wprintf(L"\nError Opening Input File:%s",pwszFileName);
		return FALSE;
	}

    while (GetNextLine(fpInput,pwszLine,4096) && SUCCEEDED(hr))
    {
        Trim(pwszLine);
        
        BSTR bsAttrib;
        BSTR bsValue;

        // Read the line into our BSTRS
        // if S_FALSE is returned, then we do not have a line
        // if E_FAIL is returned, then we have an error condition
        hr = ReadDataFromLine(pwszLine,&bsAttrib,&bsValue);

        if (hr == S_OK) 
        {
            wprintf(L" attrib:%s value:%s\n",bsAttrib,bsValue);
            
            // Take the attribute and value and add them to the Array
            if (!PopulateADSAttrInfo(&attrInfo[dwNumPopulated],bsAttrib,bsValue))
            {
                hr = E_FAIL;
                break;
            }

            SysFreeString(bsAttrib);
            SysFreeString(bsValue);
            dwNumPopulated++;
        }
    }
	fclose(fpInput);

    wsprintfW(pwCommonNameFull,L"CN=%s",pwCommonName);

    hr = pDirObject->CreateDSObject( pwCommonNameFull,  attrInfo, 
    dwNumPopulated, &pDisp );

    if (SUCCEEDED(hr))
    {
        hr = pDisp->QueryInterface(IID_IDirectoryObject,(void**) ppDirObjRet);

        pDisp->Release();
        pDisp = NULL;
    }

    //FreeAttrInfo(attrInfo);

	return hr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  CreateUser()   - Function for creating a basic User
    
    Parameters

        IDirectoryObject *pDirObject    -   Parent Directory Object for the new User
        LPWSTR pwCommonName             -   Common Name for the new User
        IDirectoryObject ** ppDirObjRet -   Pointer to the Pointer which will receive the new User
        int iUserType                  -   Bitflags for new User:
                                                                    ADS_User_TYPE_GLOBAL_User, 
                                                                    ADS_User_TYPE_DOMAIN_LOCAL_User, 
                                                                    ADS_User_TYPE_UNIVERSAL_User, 
                                                                    ADS_User_TYPE_SECURITY_ENABLED 
*/
HRESULT CreateUser(IDirectoryObject *pDirObject, LPWSTR pwCommonName,LPWSTR pwSamAcctName,IDirectoryObject ** ppDirObjRet)
{
    assert(pDirObject);
    if (wcslen(pwSamAcctName) >20)
    {
        MessageBox(NULL,L"SamAccountName CANNOT be bigger than 20 characters",L"Error: CreateSimpleUser()",MB_ICONSTOP);
        assert(0);
        return E_FAIL;
    }

	HRESULT    hr;
    ADSVALUE   sAMValue;
    ADSVALUE   classValue;
    LPDISPATCH pDisp;
    WCHAR       pwCommonNameFull[1024];
    
    ADS_ATTR_INFO  attrInfo[] = 
    {  
       { L"objectClass", ADS_ATTR_UPDATE, 
                           ADSTYPE_CASE_IGNORE_STRING, &classValue, 1 },
       {L"sAMAccountName", ADS_ATTR_UPDATE, 
                           ADSTYPE_CASE_IGNORE_STRING, &sAMValue, 1},
    };

    DWORD dwAttrs = sizeof(attrInfo)/sizeof(ADS_ATTR_INFO); 
    classValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    classValue.CaseIgnoreString = L"User";
   
    sAMValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
    sAMValue.CaseIgnoreString = pwSamAcctName;
 
    wsprintfW(pwCommonNameFull,L"CN=%s",pwCommonName);
     
    hr = pDirObject->CreateDSObject( pwCommonNameFull,  attrInfo, 
                                dwAttrs, &pDisp );
    if (SUCCEEDED(hr))
    {
        hr = pDisp->QueryInterface(IID_IDirectoryObject,(void**) ppDirObjRet);

        pDisp->Release();
        pDisp = NULL;
    }
    return hr;
}


