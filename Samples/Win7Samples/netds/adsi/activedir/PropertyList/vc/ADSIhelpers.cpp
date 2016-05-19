
#define INC_OLE2
#define UNICODE 1
#define _WIN32_DCOM


#include "ADSIhelpers.h"


#include <stdio.h>
#include <assert.h>
#include <iads.h>
#include <adshlp.h>
#include <adserr.h>
#include <wchar.h>
#include <objbase.h>
#include <activeds.h>
#include <stdio.h>
//Make sure you define UNICODE
//Need to define version 5 for Windows 2000
#define _WIN32_WINNT 0x0500


#include <sddl.h>

//////////////////////////////////////////////////////////////////////
//
// PLEASE READ: You MUST run on Windows 2000 to run this sample
//
////////////////////////////////////////////////////////////////////////


// Calls all the methods on a IADs object
// and prints them
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


// Will display a message box for a BAD Hresult.
// Will do nothing for a S_OK HRESULT
// Pass in the HRESULT to be tested and a char * describing the
// area in the code 
void CheckHRESULT(HRESULT hr, const char *pszCause)
{
    if (FAILED(hr))
    {
        char sz[1024];
        char szCaption[1024];
        strcpy_s(szCaption, 1024, pszCause);
        strcat_s(szCaption, 1024, " failed!");
        if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, hr, 0, sz, 1024, 0))
            strcpy_s(sz, 1024, "Unknown error");
        MessageBoxA(0, sz, szCaption, MB_SETFOREGROUND);
       // DebugBreak();
    }
}


// When passed a VARIANT vt member, will return a string describing
// all the bitmask settings...
const char * GetVariantStyle(VARTYPE vt)
{
	static const unsigned int buffSize = 200;
	static char pRet[buffSize]; 

	pRet[0] = NULL;
	
	if (vt == VT_EMPTY) strcat_s(pRet,buffSize,"VT_EMPTY ");

	if (HAS_BIT_STYLE(vt, VT_BYREF)) strcat_s(pRet,buffSize,"VT_BYREF ");
	if (HAS_BIT_STYLE(vt, VT_UI1)) strcat_s(pRet,buffSize,"VT_UI1 ");
	if (HAS_BIT_STYLE(vt, VT_I2)) strcat_s(pRet,buffSize,"VT_I2 ");
	if (HAS_BIT_STYLE(vt, VT_I4)) strcat_s(pRet,buffSize,"VT_I4 ");
	if (HAS_BIT_STYLE(vt, VT_R4)) strcat_s(pRet,buffSize,"VT_R4 ");
	if (HAS_BIT_STYLE(vt, VT_R8)) strcat_s(pRet,buffSize,"VT_R8 ");
	if (HAS_BIT_STYLE(vt, VT_CY)) strcat_s(pRet,buffSize,"VT_CY ");
	if (HAS_BIT_STYLE(vt, VT_BSTR)) strcat_s(pRet,buffSize,"VT_BSTR ");
	if (HAS_BIT_STYLE(vt, VT_NULL)) strcat_s(pRet,buffSize,"VT_NULL ");
	if (HAS_BIT_STYLE(vt, VT_ERROR)) strcat_s(pRet,buffSize,"VT_ERROR ");
	if (HAS_BIT_STYLE(vt, VT_BOOL)) strcat_s(pRet,buffSize,"VT_BOOL ");
	if (HAS_BIT_STYLE(vt, VT_DATE)) strcat_s(pRet,buffSize,"VT_DATE ");
	if (HAS_BIT_STYLE(vt, VT_DISPATCH)) strcat_s(pRet,buffSize,"VT_DISPATCH ");
	if (HAS_BIT_STYLE(vt, VT_VARIANT)) strcat_s(pRet,buffSize,"VT_VARIANT ");
	if (HAS_BIT_STYLE(vt, VT_UNKNOWN)) strcat_s(pRet,buffSize,"VT_UNKNOWN ");
	if (HAS_BIT_STYLE(vt, VT_ARRAY)) strcat_s(pRet,buffSize,"VT_ARRAY ");

	return pRet;  	    
}



// Pass in the interface ptr to the property value
// will return a BSTR value of the data.
// The IADsPropertyValue::get_ADsType()  is called to retrieve the 
// ADSTYPE valued enum 
// This enum is then used to determine which IADsPropertyValue method
// to call to receive the actual data

// CALLER assumes responsibility for freeing returned BSTR
HRESULT	GetIADsPropertyValueAsBSTR(BSTR * pbsRet,IADsPropertyEntry *pAdsEntry, IADsPropertyValue * pAdsPV)
{
	HRESULT hr = S_OK;

	long lAdsType;
	hr = pAdsPV->get_ADsType(&lAdsType);
	
	if (FAILED(hr))
		return hr;

	switch (lAdsType)
	{
		case ADSTYPE_INVALID :
		{
			*pbsRet = SysAllocString(L"<ADSTYPE_INVALID>");
		}
		break;

		case ADSTYPE_DN_STRING :
		{
			hr = pAdsPV->get_DNString(pbsRet);
		}
		break;
		case ADSTYPE_CASE_EXACT_STRING :
		{
			hr = pAdsPV->get_CaseExactString(pbsRet);
		}
		break;
		case ADSTYPE_CASE_IGNORE_STRING :
		{
			hr = pAdsPV->get_CaseIgnoreString(pbsRet);
		}
		break;
		case ADSTYPE_PRINTABLE_STRING :
		{
			hr = pAdsPV->get_PrintableString(pbsRet);
		}
		break;
		case ADSTYPE_NUMERIC_STRING :
		{
			hr = pAdsPV->get_NumericString(pbsRet);
		}
		break;
		case ADSTYPE_BOOLEAN :
		{
			long b;
			hr = pAdsPV->get_Boolean(&b);
			if (SUCCEEDED(hr))
			{
				if (b)
					*pbsRet = SysAllocString(L"<TRUE>");
				else
					*pbsRet = SysAllocString(L"<FALSE>");
			}
		}
		break;
		case ADSTYPE_INTEGER :
		{
			long lInt;
			hr = pAdsPV->get_Integer(&lInt);
			if (SUCCEEDED(hr))
			{
				WCHAR wOut[100];
				swprintf_s(wOut,ARRAYSIZE(wOut),L"%d",lInt);
				*pbsRet = SysAllocString(wOut);
			}
		}
		break;
		case ADSTYPE_OCTET_STRING :
		{
			*pbsRet = SysAllocString(L"<ADSTYPE_OCTET_STRING>");
            BSTR bsName= NULL;
            VARIANT vOctet;
            DWORD dwSLBound;
            DWORD dwSUBound;
            void HUGEP *pArray = NULL;
            VariantInit(&vOctet);
	
				//Get the name of the property to handle
				//the properties we're interested in.
				pAdsEntry->get_Name(&bsName);
				hr = pAdsPV->get_OctetString(&vOctet);
				
                //Get a pointer to the bytes in the octet string.
				if (SUCCEEDED(hr))
				{
					hr = SafeArrayGetLBound( V_ARRAY(&vOctet),
											  1,
											  (long FAR *) &dwSLBound );
					hr = SafeArrayGetUBound( V_ARRAY(&vOctet),
											  1,
											  (long FAR *) &dwSUBound );
					if (SUCCEEDED(hr))
					{
						hr = SafeArrayAccessData( V_ARRAY(&vOctet),
												  &pArray );
					}
					/* Since an Octet String has a specific meaning 
                       depending on the attribute name, handle two 
                       common ones here
                    */
					if ( pArray )
					{
						if (0==wcscmp(L"objectGUID", bsName))
						{
							LPOLESTR szDSGUID = new WCHAR [39];

							//Cast to LPGUID
							LPGUID pObjectGUID = (LPGUID)pArray;
							//Convert GUID to string.
							::StringFromGUID2(*pObjectGUID, szDSGUID, 39); 
							*pbsRet = SysAllocString(szDSGUID);

						}
						else if (0==wcscmp(L"objectSid", bsName))
						{
							PSID pObjectSID = (PSID)pArray;

							//Convert SID to string.
							LPOLESTR szSID = NULL;
							ConvertSidToStringSid(pObjectSID, &szSID);
							*pbsRet = SysAllocString(szSID);
							LocalFree(szSID);
						}
						else
						{
							*pbsRet = SysAllocString(L"<Value of type Octet String. No Conversion>");

						}
					}
					else
					{
						*pbsRet = SysAllocString(L"<Unable to Access variant data>");
					}

					SafeArrayUnaccessData( V_ARRAY(&vOctet) );
					VariantClear(&vOctet);
				}

				SysFreeString(bsName);
				

		}
		break;
		case ADSTYPE_LARGE_INTEGER :
		{
			*pbsRet = SysAllocString(L"<ADSTYPE_LARGE_INTEGER>");
		}
		break;
		case ADSTYPE_PROV_SPECIFIC :
		{
			*pbsRet = SysAllocString(L"<ADSTYPE_PROV_SPECIFIC>");
		}
		break;
		case ADSTYPE_OBJECT_CLASS :
		{
			hr = pAdsPV->get_CaseIgnoreString(pbsRet);
		}
		break;
		case ADSTYPE_PATH :
		{
			hr = pAdsPV->get_CaseIgnoreString(pbsRet);
		}
		break;
		case ADSTYPE_NT_SECURITY_DESCRIPTOR :
		{
			*pbsRet = SysAllocString(L"<ADSTYPE_NT_SECURITY_DESCRIPTOR>");
		}
		break;
    
		default: 
			*pbsRet = SysAllocString(L"<UNRECOGNIZED>");
		break;
			
	}	
	return hr;
}


