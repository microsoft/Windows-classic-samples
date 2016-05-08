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





int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy=1);
bool SAFE_APPEND(int targetSize, LPWSTR szTarget, LPCWSTR szSource);








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

	pIADs->get_Name(&bsResult); 
	wprintf(L" NAME: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	pIADs->get_Class(&bsResult); 
	wprintf(L" CLASS: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	pIADs->get_GUID(&bsResult); 
	wprintf(L" GUID: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	pIADs->get_ADsPath(&bsResult); 
	wprintf(L" ADSPATH: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	pIADs->get_Parent(&bsResult); 
	wprintf(L" PARENT: %s\n",(LPOLESTR) bsResult);
	SysFreeString(bsResult);
	
	pIADs->get_Schema(&bsResult); 
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

void CheckADHRESULT(HRESULT passedhr,LPOLESTR pwReason)
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

	int maxAlloc = sizeof(pwErrBuff)/sizeof(WCHAR);

    // If an ADSI error code was returned, display it
    if (pADErrCodeStr[0] != NULL)
    {
		SAFE_APPEND(maxAlloc, pwErrBuff, L"<*> ADSI Error HRESULT Returns: <*> \n"); 
		SAFE_APPEND(maxAlloc, pwErrBuff, pADErrCodeStr); 
		SAFE_APPEND(maxAlloc, pwErrBuff, L"\n"); 
    }           
    
    // Call ADsGetLastError() to retrieve the error text from ADSI
	SAFE_APPEND(maxAlloc, pwErrBuff, L"<*> ADSI Error: <*>\n"); 
    
    hr = ADsGetLastError(
        &lError,
        pwADErrBuff,
        2047,
        L"",
        0
        );


    if (FAILED(hr))
		SAFE_APPEND(maxAlloc, pwErrBuff, L"<Unable to retrieve ADs Error>"); 
    else
		SAFE_APPEND(maxAlloc, pwErrBuff, pwADErrBuff); 
        

    
	SAFE_APPEND(maxAlloc, pwErrBuff, L"\n"); 
    
    WCHAR sz[1024];
    
	SAFE_APPEND(maxAlloc, pwErrBuff, L"<*> System Error: <*> \n"); 

    // Check the standard system to crack the bad HRESULT into a string
    if (!FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, passedhr, 0, sz, 1024, 0))
        wcscat_s(sz, 1024, L"Unknown HRESULT error");
    else
		SAFE_APPEND(maxAlloc, pwErrBuff, sz); 
        
        
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

      //case  E_ADS_LDAP_BASE:
      //   return L"E_ADS_LDAP_BASE";
      //   break;

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

      // case  __HRESULT_FROM_WIN32(E_ADS_LDAP_ALIAS_PROBLEM:
         // return L"__HRESULT_FROM_WIN32(E_ADS_LDAP_ALIAS_PROBLEM";
         // break;

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

      // case  __HRESULT_FROM_WIN32(E_ADS_LDAP_MORE_RESULTS_TO_RETURN:
         // return L"__HRESULT_FROM_WIN32(E_ADS_LDAP_MORE_RESULTS_TO_RETURN";
         // break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_CLIENT_LOOP):
         return L"(ERROR_DS_CLIENT_LOOP";
         break;

      case  __HRESULT_FROM_WIN32(ERROR_DS_REFERRAL_LIMIT_EXCEEDED):
         return L"ERROR_DS_REFERRAL_LIMIT_EXCEEDED";
         break;

      // case  __HRESULT_FROM_WIN32(E_ADS_LDAP_LAST:
         // return L"__HRESULT_FROM_WIN32(E_ADS_LDAP_LAST";
         // break;

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
		 int errSize=sizeof(szErrorBuff)/sizeof(WCHAR);
		 SAFE_APPEND(errSize, szNameBuff, L" : " ); 
         SAFE_APPEND(errSize, szNameBuff, szErrorBuff); 
         
		 szwText[0]=0l;
		 int textSize= sizeof(szwText)/sizeof(WCHAR);
		 SAFE_APPEND(textSize,szwText, szNameBuff);


         return szwText;
      }

   }

    return szwText; 
}




int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy)
{
	     
         if (toCopy == -1)

          {
               toCopy = wcslen(pszSource);
          }

		  return maxAlloc - (wcslen(pszTarget) + toCopy + 1); 
}



bool SAFE_APPEND(int targetSize, LPWSTR szTarget, LPCWSTR szSource) 
{
	if ( IS_BUFFER_ENOUGH( targetSize, szTarget, szSource) > 0 )
	{
	  wcscpy_s(szTarget, targetSize, szSource);
	  return true;
	}

	return false;
}
 


