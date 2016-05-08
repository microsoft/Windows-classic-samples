
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
#include <strsafe.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
// String for usage information
WCHAR * pwszUsage = L"Adds new group under the passed container.\n"
                    L"Usage:\n"
                    L"AddGroup <Path of container>\n" 
                    L"              ADsPath of the container for placing the new group\n"
                    L"         <Windows2000 Group Name> \n"
                    L"              This is the name for the new group\n"
                    L"         <Downlevel NT 4 Sam Account name>\n"
                    L"              Cannot exceed 20 characters and must be globally unique\n"
                    L"         \nOptional:\n"
                    L"         <Group Type>\n" 
                    L"              Possible values: 'global' or 'local' or 'universal'\n"
                    L"         <Security >\n"
                    L"             Possible values: 'Security' or 'NoSecurity' \n"
                    L"             If security is specified, new group will be created with \n"
                    L"             ADS_GROUP_TYPE_SECURITY_ENABLED.If parameter is not supplied,\n"
                    L"             new group will be created with ADS_GROUP_TYPE_SECURITY_ENABLED\n";


////////////////////////////////////////////////////////////////////////////////////////////////////
// Module level global variables
BSTR bsADsPathContainer;
BSTR bsNT5GroupName;
BSTR bsSamAcctName;
int iGroupType;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
BOOL ParseCommandLine( int argc, __in wchar_t *argv[ ]);
HRESULT CreateGroup(IDirectoryObject *pDirObject, __in LPWSTR pwCommonName,__in LPWSTR pwSamAcctName,
                    IDirectoryObject ** ppDirObjRet,int iGroupType);
void PrintIADSObject(IADs * pIADs);
void CheckADHRESULT(HRESULT passedhr,__in LPOLESTR pwReason);
WCHAR * CrackADError(HRESULT hResult);


////////////////////////////////////////////////////////////////////////////////////////////////////
// main()

/* Note: Using the UNICODE version of main().
   this removes the need for the sample to include
   UNICODE-ANSI conversion routines
*/
int __cdecl wmain( int argc, __in wchar_t *argv[ ])
{

    HRESULT hr;
    IDirectoryObject * pDirObjectContainer = NULL;
    IDirectoryObject * pDirObjRet = NULL;
    
    if (!ParseCommandLine(argc,argv))
        goto ret;
 
    // Initialize COM
    CoInitialize(0);

    // Bind to the container passed 
    hr = ADsGetObject(  bsADsPathContainer, IID_IDirectoryObject,(void **)&pDirObjectContainer);
    
    if (SUCCEEDED(hr))
    {
        // Call the helper funtion to create the group
        hr = CreateGroup(pDirObjectContainer, bsNT5GroupName,bsSamAcctName,
                         &pDirObjRet,iGroupType);
        
        if (SUCCEEDED(hr))
        {
            _putws(L"\n New Group created with the following properties:");
            
            IADs * pIADsNewGoup = NULL;
            
            // Group succeeded- now get an IADs interface to it 
            // and print some properties
            hr = pDirObjRet->QueryInterface(IID_IADs,(void**)&pIADsNewGoup);

            if (SUCCEEDED(hr))
            {
                PrintIADSObject(pIADsNewGoup);
                            
                pIADsNewGoup->Release();
                pIADsNewGoup = NULL;
            }
            else
                CheckADHRESULT(hr,L"QueryInterface() - New group for IADs");
            pDirObjRet->Release();
            pDirObjRet = NULL;    
        }
        else
            CheckADHRESULT(hr,L"CreateGroup()");
        
        pDirObjectContainer->Release();
        pDirObjectContainer = NULL;    
    }
    else
        CheckADHRESULT(hr,L"ADsGetObject()");

    CoUninitialize();
    ret: 
    if(bsADsPathContainer) SysFreeString(bsADsPathContainer);
    if(bsNT5GroupName) SysFreeString(bsNT5GroupName);
    if(bsSamAcctName) SysFreeString(bsSamAcctName);
    
 }


////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    ParseCommandLine()- Parses command line and sets module level globals
    
    Parameters:

        int argc            - Number of Arguments
        wchar_t *argv[ ]    - Array of arguments

    Returns:
        TRUE if Command line was successfully parsed
*/    

BOOL ParseCommandLine( int argc, __in wchar_t *argv[ ])
{
	if (argc < 4 || argc >6)
	{
        _putws(pwszUsage);
        return FALSE;
    }
    // if we have at least 3 arguments (the first is the EXE name)
    // Save them
    if (argc >= 4)
    {
        bsADsPathContainer  = SysAllocString(argv[1]);
        if(!bsADsPathContainer)
            return FALSE;
        bsNT5GroupName      = SysAllocString(argv[2]);
        if(!bsNT5GroupName)
            return FALSE;
        if(wcslen(bsNT5GroupName)>64) return FALSE;
        bsSamAcctName       = SysAllocString(argv[3]);
        if(!bsSamAcctName)
            return FALSE;
    }
    
    // was the group type passed in?
    if (argc >=5)
    {
        if (_wcsicmp(L"global",argv[4])==0)
            iGroupType = ADS_GROUP_TYPE_GLOBAL_GROUP;
        
        else if (_wcsicmp(L"local",argv[4])==0)
            iGroupType = ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP;

        else if (_wcsicmp(L"universal",argv[4])==0)
            iGroupType = ADS_GROUP_TYPE_UNIVERSAL_GROUP;

        else
	    {
            _putws(L"!! Incorrect Group Type Parameter !!");                
            _putws(pwszUsage);
            return FALSE;
        }
    }
     else   // Default to Global 
        iGroupType = ADS_GROUP_TYPE_GLOBAL_GROUP;

    // was security enabled passed in?
    if (argc ==6)
    {
        if (_wcsicmp(L"security",argv[5])==0)
            iGroupType |= ADS_GROUP_TYPE_SECURITY_ENABLED;
        
        // else if the parameter there is NOT nosecurity- error
        else if (!(_wcsicmp(L"nosecurity",argv[5])==0))
        {
            _putws(L"!! Incorrect security Parameter !!");                
            _putws(pwszUsage);
            return FALSE;
        }
    
    }
    else    // Default to security 
        iGroupType |= ADS_GROUP_TYPE_SECURITY_ENABLED;
        
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*  CreateGroup()   - Function for creating a basic group
    
    Parameters

        IDirectoryObject *pDirObject    -   Parent Directory Object for the new group
        LPWSTR pwCommonName             -   Common Name for the new group
        IDirectoryObject ** ppDirObjRet -   Pointer to the Pointer which will receive the new Group
        int iGroupType                  -   Bitflags for new group:
                                                                    ADS_GROUP_TYPE_GLOBAL_GROUP, 
                                                                    ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP, 
                                                                    ADS_GROUP_TYPE_UNIVERSAL_GROUP, 
                                                                    ADS_GROUP_TYPE_SECURITY_ENABLED 
*/
HRESULT CreateGroup(IDirectoryObject *pDirObject, __in LPWSTR pwCommonName,__in LPWSTR pwSamAcctName,IDirectoryObject ** ppDirObjRet,int iGroupType)
{
    assert(pDirObject);
    if (wcslen(pwSamAcctName) >256)
    {
        MessageBox(NULL,L"SamAccountName CANNOT be bigger than 256 characters",L"Error: CreateSimpleGroup()",MB_ICONSTOP);
        assert(0);
        return E_FAIL;
    }

    HRESULT    hr;
    ADSVALUE   sAMValue;
    ADSVALUE   classValue;
    ADSVALUE   groupType;

    LPDISPATCH pDisp;
    WCHAR       pwCommonNameFull[1024];
    
    ADS_ATTR_INFO  attrInfo[] = 
    {  
       { L"objectClass", ADS_ATTR_UPDATE, 
                           ADSTYPE_CASE_IGNORE_STRING, &classValue, 1 },
       {L"sAMAccountName", ADS_ATTR_UPDATE, 
                           ADSTYPE_CASE_IGNORE_STRING, &sAMValue, 1},
       {L"groupType", ADS_ATTR_UPDATE, 
                           ADSTYPE_INTEGER, &groupType, 1}
    };

    DWORD dwAttrs = sizeof(attrInfo)/sizeof(ADS_ATTR_INFO); 
 
    classValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    classValue.CaseIgnoreString = L"group";
   
    sAMValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
    sAMValue.CaseIgnoreString = pwSamAcctName;
 
    groupType.dwType=ADSTYPE_INTEGER;
    groupType.Integer =  iGroupType;
    
    swprintf_s(pwCommonNameFull,1024,L"CN=%s",pwCommonName);
     
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


////////////////////////////////////////////////////////////////////////////////////////////////////
/*  
    PrintIADSObject()       -   Calls all the methods on a IADs object 
                                and prints them the console window
    Parameters
    
        IADs * pIADs        - IADs Interface from which to retrieve attribute values
*/
void PrintIADSObject(IADs * pIADs)
{
	assert(pIADs);

	BSTR bsResult;
	HRESULT hr = S_OK;
	
	hr = pIADs->get_Name(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get name\n"); return;}
	wprintf(L" NAME: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	hr = pIADs->get_Class(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get class\n"); return;}
	wprintf(L" CLASS: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	hr = pIADs->get_GUID(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get GUID\n"); return;}	
	wprintf(L" GUID: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	hr = pIADs->get_ADsPath(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get ADsPath\n"); return;}
	wprintf(L" ADSPATH: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	hr = pIADs->get_Parent(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get parent\n"); return;}
	wprintf(L" PARENT: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	hr = pIADs->get_Schema(&bsResult); 
	if(FAILED(hr)) {wprintf(L" Failed to get schema\n"); return;}
	wprintf(L" SCHEMA: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Error Handling routines

// Will display ADSI or COM error messages
// when passed a BAD Hresult
void CheckADHRESULT(HRESULT passedhr,__in LPOLESTR pwReason)
{
    if (SUCCEEDED(passedhr))
        return;

    HRESULT hr;
    DWORD lError;
    WCHAR pwErrBuff[2048];
    WCHAR pwADErrBuff[512];

    WCHAR * pADErrCodeStr;
    
    pwErrBuff[0] = NULL;
    pADErrCodeStr = CrackADError( passedhr);

    // If an ADSI error code was returned, display it
    if (pADErrCodeStr[0] != NULL)
    {
        wcscat_s(pwErrBuff,2048,L"<*> ADSI Error HRESULT Returns: <*> \n");
        wcscat_s(pwErrBuff,2048,pADErrCodeStr);
        wcscat_s(pwErrBuff,2048,L"\n");
    }           
    
    // Call ADsGetLastError() to retrieve the error text from ADSI
    wcscat_s(pwErrBuff,2048,L"<*> ADSI Error: <*>\n");
    
    hr = ADsGetLastError(
        &lError,
        pwADErrBuff,
        511,
        L"",
        0
        );


    if (FAILED(hr))
        wcscat_s(pwErrBuff,2048,L"<Unable to retrieve ADs Error>");
    else
        wcscat_s(pwErrBuff,2048,pwADErrBuff);

    wcscat_s(pwErrBuff,2048,L"\n");
    
    WCHAR sz[1024];
    
    wcscat_s(pwErrBuff,2048,L"<*> System Error: <*> \n");

    // Check the standard system to crack the bad HRESULT into a string
    if (!FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, passedhr, 0, sz, 1024, 0))
        wcscat_s(sz, 1024,L"Unknown HRESULT error");
    else
        wcscat_s(pwErrBuff, 2048,sz);
        
    // Display
    OutputDebugStringW(pwErrBuff);
    OutputDebugStringW(L"\r\n");
    _putws(L"\n**********");
    wprintf(L"\nERROR on %s\n\n",pwReason);
    _putws(pwErrBuff);
    _putws(L"\n**********");
    
}

WCHAR * CrackADError(HRESULT hResult)
{
   static WCHAR szwText[ 128 ];
   szwText[0] = 0l;

   switch( hResult )
   {
      case  E_ADS_BAD_PATHNAME:
         return L"E_ADS_BAD_PATHNAME";
         break;

      case  E_ADS_INVALID_DOMAIN_OBJECT:
         return L"E_ADS_INVALID_DOMAIN_OBJECT";
         break;

      case  E_ADS_INVALID_USER_OBJECT:
         return L"E_ADS_INVALID_USER_OBJECT";
         break;

      case  E_ADS_INVALID_COMPUTER_OBJECT:
         return L"E_ADS_INVALID_COMPUTER_OBJECT";
         break;

      case  E_ADS_UNKNOWN_OBJECT:
         return L"E_ADS_UNKNOWN_OBJECT";
         break;

      case  E_ADS_PROPERTY_NOT_SET:
         return L"E_ADS_PROPERTY_NOT_SET";
         break;

      case  E_ADS_PROPERTY_NOT_SUPPORTED:
         return L"E_ADS_PROPERTY_NOT_SUPPORTED";
         break;

      case  E_ADS_PROPERTY_INVALID:
         return L"E_ADS_PROPERTY_INVALID";
         break;

      case  E_ADS_BAD_PARAMETER:
         return L"E_ADS_BAD_PARAMETER";
         break;

      case  E_ADS_OBJECT_UNBOUND:
         return L"E_ADS_OBJECT_UNBOUND";
         break;

      case  E_ADS_PROPERTY_NOT_MODIFIED:
         return L"E_ADS_PROPERTY_NOT_MODIFIED";
         break;

      case  E_ADS_PROPERTY_MODIFIED:
         return L"E_ADS_PROPERTY_MODIFIED";
         break;

      case  E_ADS_CANT_CONVERT_DATATYPE:
         return L"E_ADS_CANT_CONVERT_DATATYPE";
         break;

      case  E_ADS_PROPERTY_NOT_FOUND:
         return L"E_ADS_PROPERTY_NOTFOUND";
         break;

      case  E_ADS_OBJECT_EXISTS:
         return L"E_ADS_OBJECT_EXISTS";
         break;

      case  E_ADS_SCHEMA_VIOLATION:
         return L"E_ADS_SCHEMA_VIOLATION";
         break;

      case  E_ADS_COLUMN_NOT_SET:
         return L"E_ADS_COLUMN_NOT_SET";
         break;

      case  E_ADS_INVALID_FILTER:
         return L"E_ADS_INVALID_FILTER";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_OPERATIONS_ERROR):
         return L"ERROR_DS_OPERATIONS_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_PROTOCOL_ERROR):
         return L"ERROR_DS_PROTOCOL_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_TIMELIMIT_EXCEEDED):
         return L"ERROR_DS_TIMELIMIT_EXCEEDED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_SIZELIMIT_EXCEEDED):
         return L"ERROR_DS_SIZELIMIT_EXCEEDED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_COMPARE_FALSE):
         return L"ERROR_DS_COMPARE_FALSE";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_DS_COMPARE_TRUE):
         return L"ERROR_DS_COMPARE_TRUE";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_DS_AUTH_METHOD_NOT_SUPPORTED):
         return L"ERROR_DS_AUTH_METHOD_NOT_SUPPORTED";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_DS_STRONG_AUTH_REQUIRED):
         return L"ERROR_DS_STRONG_AUTH_REQUIRED";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_MORE_DATA):
         return L"ERROR_MORE_DATA";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_REFERRAL):
         return L"(ERROR_DS_REFERRAL";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_ADMIN_LIMIT_EXCEEDED):
         return L"ERROR_DS_ADMIN_LIMIT_EXCEEDED";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_DS_UNAVAILABLE_CRIT_EXTENSION):
         return L"(ERROR_DS_UNAVAILABLE_CRIT_EXTENSION";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CONFIDENTIALITY_REQUIRED):
         return L"__HRESULT_FROM_WIN32(ERROR_DS_CONFIDENTIALITY_REQUIRED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_NO_ATTRIBUTE_OR_VALUE):
         return L"ERROR_DS_NO_ATTRIBUTE_OR_VALUE";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED):
         return L"(ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_INAPPROPRIATE_MATCHING):
         return L"(ERROR_DS_INAPPROPRIATE_MATCHING";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION):
         return L"ERROR_DS_CONSTRAINT_VIOLATION";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS):
         return L"ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_INVALID_ATTRIBUTE_SYNTAX):
         return L"ERROR_DS_INVALID_ATTRIBUTE_SYNTAX";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT):
         return L"ERROR_DS_NO_SUCH_OBJECT";
         break;


      case  __HRESULT_FROM_WIN32(ERROR_DS_INVALID_DN_SYNTAX):
         return L"ERROR_DS_INVALID_DN_SYNTAX";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_IS_LEAF):
         return L"ERROR_DS_IS_LEAF";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_ALIAS_DEREF_PROBLEM):
         return L"ERROR_DS_ALIAS_DEREF_PROBLEM";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_INAPPROPRIATE_AUTH):
         return L"ERROR_DS_INAPPROPRIATE_AUTH";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD):
         return L"ERROR_INVALID_PASSWORD";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
         return L"ERROR_ACCESS_DENIED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_BUSY):
         return L"ERROR_DS_BUSY";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_UNAVAILABLE):
         return L"ERROR_DS_UNAVAILABLE";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM):
         return L"ERROR_DS_UNWILLING_TO_PERFORM";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_LOOP_DETECT):
         return L"ERROR_DS_LOOP_DETECT";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_NAMING_VIOLATION):
         return L"ERROR_DS_NAMING_VIOLATION";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_OBJ_CLASS_VIOLATION):
         return L"ERROR_DS_OBJ_CLASS_VIOLATION";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_NON_LEAF):
         return L"ERROR_DS_CANT_ON_NONLEAF";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_RDN):
         return L"ERROR_DS_CANT_ON_RDN";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):
         return L"ERROR_ALREADY_EXISTS";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CANT_MOD_OBJ_CLASS):
         return L"ERROR_DS_CANT_MOD_OBJ_CLASS";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_OBJECT_RESULTS_TOO_LARGE):
         return L"ERROR_DS_OBJECT_RESULTS_TOO_LARGE";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_AFFECTS_MULTIPLE_DSAS):
         return L"ERROR_DS_AFFECTS_MULTIPLE_DSAS";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_GEN_FAILURE):
         return L"(ERROR_GEN_FAILURE";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN):
         return L"ERROR_DS_SERVER_DOWN";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_LOCAL_ERROR):
         return L"ERROR_DS_LOCAL_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_ENCODING_ERROR):
         return L"ERROR_DS_ENCODING_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_DECODING_ERROR):
         return L"(ERROR_DS_DECODING_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_TIMEOUT):
         return L"(ERROR_TIMEOUT";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_AUTH_UNKNOWN):
         return L"ERROR_DS_AUTH_UNKNOWN";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_FILTER_UNKNOWN):
         return L"(ERROR_DS_FILTER_UNKNOWN";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_CANCELLED):
         return L"(ERROR_CANCELLED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_PARAM_ERROR):
         return L"ERROR_DS_PARAM_ERROR";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
         return L"ERROR_NOT_ENOUGH_MEMORY";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_CONNECTION_REFUSED):
         return L"ERROR_CONNECTION_REFUSED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_NOT_SUPPORTED):
         return L"ERROR_DS_NOT_SUPPORTED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_NO_RESULTS_RETURNED):
         return L"ERROR_DS_NO_RESULTS_RETURNED";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CONTROL_NOT_FOUND):
         return L"ERROR_DS_CONTROL_NOT_FOUND";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CLIENT_LOOP):
         return L"(ERROR_DS_CLIENT_LOOP";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_REFERRAL_LIMIT_EXCEEDED):
         return L"ERROR_DS_REFERRAL_LIMIT_EXCEEDED";
         break;

      case  E_FAIL:
         return L"E_FAIL";
         break;

      case  E_NOTIMPL:
         return L"E_NOIMPL";
         break;

      case  S_OK:
         return L"OK";
         break;

      case  0x800704b8:
      // we have an extended error
      {
         WCHAR szErrorBuff[ 1024 ];
         WCHAR szNameBuff[ 1024 ];
         DWORD dwError;

         ADsGetLastError( &dwError,
                          szErrorBuff,
                          1023,
                          szNameBuff,
                          1023 );

         int sizeNameBuff = wcslen(szNameBuff);	  

	  if(sizeNameBuff<1024-3)
	  {
         	wcsncat_s( szNameBuff, 1024, L" : ",3);
		if(1023-sizeNameBuff-3 >=0) //-3 for previous wcsncat
         	wcsncat_s( szNameBuff, 1024, szErrorBuff,1023-sizeNameBuff-3);
	  }
         wcsncat_s( szwText,128, szNameBuff,127);

         return szwText;
      }

   }

    return szwText; 
}


