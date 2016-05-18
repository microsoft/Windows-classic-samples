// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  Consumer.cpp
//
// Description:
//		  Event consumer implementation
//
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "Consumer.h"
#include <objbase.h>
#include <strsafe.h>

CConsumer::CConsumer(CListBox	*pOutputList)
{
	m_cRef = 0L;
	m_pOutputList = pOutputList;
}

CConsumer::~CConsumer()
{
}

STDMETHODIMP CConsumer::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv=NULL;

    if (riid == IID_IUnknown || riid == IID_IWbemUnboundObjectSink)
        *ppv=this;

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CConsumer::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CConsumer::Release(void)
{
    if (--m_cRef != 0L)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CConsumer::IndicateToConsumer(IWbemClassObject *pLogicalConsumer,
											long lNumObjects,
											IWbemClassObject **ppObjects)
{

	// NOTE: If this routine returns a failure code, including GPFs from called routines,
	// CIMOM will recreate the object and call here again. If you see this routine being
	// called twice for every indication, it means this routine is returning a failure code
	// somehow. Especially watch the AddRef()/Release() semantics for the embedded object.
	// If they're too low, you'll return a GPF.
	HRESULT  hRes;
	CString clMyBuff;
	BSTR objName = NULL;
	BSTR propName = NULL;
	VARIANT pVal, vUnk;
	IUnknown *pUnk = NULL;
	IWbemClassObject *tgtInst = NULL;

	VariantInit(&pVal);
	VariantInit(&vUnk);

	TRACE(_T("Indicate() called\n"));

	objName = SysAllocString(L"TargetInstance");
	if (!objName)
	{
		TRACE(_T("SysAllocString failed\n"));
		return E_OUTOFMEMORY;
	}
	propName = SysAllocString(L"Item");
	if (!propName)
	{
		TRACE(_T("SysAllocString failed\n"));
		SysFreeString(objName);
		return E_OUTOFMEMORY;
	}

	// walk though the classObjects...
	for (int i = 0; i < lNumObjects; i++)
	{
		// clear my output buffer.
		clMyBuff.Empty();

		// get what was added. This will be an embedded object (VT_UNKNOWN). All
		// WMI interfaces are derived from IUnknown.
		if ((hRes = ppObjects[i]->Get(objName, 0L, 
										&vUnk, NULL, NULL)) == S_OK) 
		{
			//--------------------------------
			// pull the IUnknown out of the various. Dont cast directly to to IWbemClassObject.
			// it MIGHT work now but a suptle change later will break your code. The PROPER 
			// way is to go through QueryInterface and do all the right Release()'s.
			pUnk = (IUnknown *)V_UNKNOWN(&vUnk);

			//--------------------------------
			// Even though you didn't use QI(), you just made a copy of a COM ptr so you 
			// better do the AddRef() that QI() would otherwise do for you.
			pUnk->AddRef();

			//--------------------------------
			// ask for the IWbemClassObject which is the embedded object. This will be the
			// instance that was created.
			if(SUCCEEDED(pUnk->QueryInterface(IID_IWbemClassObject, (void **)&tgtInst)))
			{
				//--------------------------------
				// done with it.
				pUnk->Release();

				//--------------------------------
				// get the 'Item' property out of the embedded object.
				if ((hRes = tgtInst->Get(propName, 0L, 
										&pVal, NULL, NULL)) == S_OK) 
				{
					//--------------------------------
					// done with it.
					tgtInst->Release();

					// compose a string for the listbox.
					clMyBuff = _T("OfficeEquipment Instance added for: ");
					clMyBuff += V_BSTR(&pVal);
				
					// output the buffer.
					m_pOutputList->AddString(clMyBuff);
				}
				else
				{
					TRACE(_T("Get() Item failed %s\n"), ErrorString(hRes));
				}
			}
			else
			{
				TRACE(_T("QI() failed \n"));
			}
		}
		else
		{
			TRACE(_T("Get() targetInst failed %s\n"), ErrorString(hRes));
			m_pOutputList->AddString(_T("programming error"));
		} //endif Get()

	} // endfor

	SysFreeString(propName);
	SysFreeString(objName);
	VariantClear(&pVal);
	VariantClear(&vUnk);

	TRACE(_T("walked indication list\n"));

	return S_OK;
}

// **************************************************************************
//
//	ErrorString()
//
// Description:
//		Converts an HRESULT to a displayable string.
//
// Parameters:
//		hRes (in) - HRESULT to be converted.
//
// Returns:
//		ptr to displayable string.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
LPCTSTR CConsumer::ErrorString(HRESULT hRes)
{
    TCHAR szBuffer2[19];
	static TCHAR szBuffer[24];
	LPCTSTR psz;

    switch(hRes) 
    {
    case WBEM_NO_ERROR:
		psz = _T("WBEM_NO_ERROR");
		break;
    case WBEM_S_FALSE:
		psz = _T("WBEM_S_FALSE");
		break;
    case WBEM_S_NO_MORE_DATA:
		psz = _T("WBEM_S_NO_MORE_DATA");
		break;
	case WBEM_E_FAILED:
		psz = _T("WBEM_E_FAILED");
		break;
	case WBEM_E_NOT_FOUND:
		psz = _T("WBEM_E_NOT_FOUND");
		break;
	case WBEM_E_ACCESS_DENIED:
		psz = _T("WBEM_E_ACCESS_DENIED");
		break;
	case WBEM_E_PROVIDER_FAILURE:
		psz = _T("WBEM_E_PROVIDER_FAILURE");
		break;
	case WBEM_E_TYPE_MISMATCH:
		psz = _T("WBEM_E_TYPE_MISMATCH");
		break;
	case WBEM_E_OUT_OF_MEMORY:
		psz = _T("WBEM_E_OUT_OF_MEMORY");
		break;
	case WBEM_E_INVALID_CONTEXT:
		psz = _T("WBEM_E_INVALID_CONTEXT");
		break;
	case WBEM_E_INVALID_PARAMETER:
		psz = _T("WBEM_E_INVALID_PARAMETER");
		break;
	case WBEM_E_NOT_AVAILABLE:
		psz = _T("WBEM_E_NOT_AVAILABLE");
		break;
	case WBEM_E_CRITICAL_ERROR:
		psz = _T("WBEM_E_CRITICAL_ERROR");
		break;
	case WBEM_E_INVALID_STREAM:
		psz = _T("WBEM_E_INVALID_STREAM");
		break;
	case WBEM_E_NOT_SUPPORTED:
		psz = _T("WBEM_E_NOT_SUPPORTED");
		break;
	case WBEM_E_INVALID_SUPERCLASS:
		psz = _T("WBEM_E_INVALID_SUPERCLASS");
		break;
	case WBEM_E_INVALID_NAMESPACE:
		psz = _T("WBEM_E_INVALID_NAMESPACE");
		break;
	case WBEM_E_INVALID_OBJECT:
		psz = _T("WBEM_E_INVALID_OBJECT");
		break;
	case WBEM_E_INVALID_CLASS:
		psz = _T("WBEM_E_INVALID_CLASS");
		break;
	case WBEM_E_PROVIDER_NOT_FOUND:
		psz = _T("WBEM_E_PROVIDER_NOT_FOUND");
		break;
	case WBEM_E_INVALID_PROVIDER_REGISTRATION:
		psz = _T("WBEM_E_INVALID_PROVIDER_REGISTRATION");
		break;
	case WBEM_E_PROVIDER_LOAD_FAILURE:
		psz = _T("WBEM_E_PROVIDER_LOAD_FAILURE");
		break;
	case WBEM_E_INITIALIZATION_FAILURE:
		psz = _T("WBEM_E_INITIALIZATION_FAILURE");
		break;
	case WBEM_E_TRANSPORT_FAILURE:
		psz = _T("WBEM_E_TRANSPORT_FAILURE");
		break;
	case WBEM_E_INVALID_OPERATION:
		psz = _T("WBEM_E_INVALID_OPERATION");
		break;
	case WBEM_E_INVALID_QUERY:
		psz = _T("WBEM_E_INVALID_QUERY");
		break;
	case WBEM_E_INVALID_QUERY_TYPE:
		psz = _T("WBEM_E_INVALID_QUERY_TYPE");
		break;
	case WBEM_E_ALREADY_EXISTS:
		psz = _T("WBEM_E_ALREADY_EXISTS");
		break;
    case WBEM_S_ALREADY_EXISTS:
        psz = _T("WBEM_S_ALREADY_EXISTS");
        break;
    case WBEM_S_RESET_TO_DEFAULT:
        psz = _T("WBEM_S_RESET_TO_DEFAULT");
        break;
    case WBEM_S_DIFFERENT:
        psz = _T("WBEM_S_DIFFERENT");
        break;
    case WBEM_E_OVERRIDE_NOT_ALLOWED:
        psz = _T("WBEM_E_OVERRIDE_NOT_ALLOWED");
        break;
    case WBEM_E_PROPAGATED_QUALIFIER:
        psz = _T("WBEM_E_PROPAGATED_QUALIFIER");
        break;
    case WBEM_E_PROPAGATED_PROPERTY:
        psz = _T("WBEM_E_PROPAGATED_PROPERTY");
        break;
    case WBEM_E_UNEXPECTED:
        psz = _T("WBEM_E_UNEXPECTED");
        break;
    case WBEM_E_ILLEGAL_OPERATION:
        psz = _T("WBEM_E_ILLEGAL_OPERATION");
        break;
    case WBEM_E_CANNOT_BE_KEY:
        psz = _T("WBEM_E_CANNOT_BE_KEY");
        break;
    case WBEM_E_INCOMPLETE_CLASS:
        psz = _T("WBEM_E_INCOMPLETE_CLASS");
        break;
    case WBEM_E_INVALID_SYNTAX:
        psz = _T("WBEM_E_INVALID_SYNTAX");
        break;
    case WBEM_E_NONDECORATED_OBJECT:
        psz = _T("WBEM_E_NONDECORATED_OBJECT");
        break;
    case WBEM_E_READ_ONLY:
        psz = _T("WBEM_E_READ_ONLY");
        break;
    case WBEM_E_PROVIDER_NOT_CAPABLE:
        psz = _T("WBEM_E_PROVIDER_NOT_CAPABLE");
        break;
    case WBEM_E_CLASS_HAS_CHILDREN:
        psz = _T("WBEM_E_CLASS_HAS_CHILDREN");
        break;
    case WBEM_E_CLASS_HAS_INSTANCES:
        psz = _T("WBEM_E_CLASS_HAS_INSTANCES");
        break;
    case WBEM_E_QUERY_NOT_IMPLEMENTED:
        psz = _T("WBEM_E_QUERY_NOT_IMPLEMENTED");
        break;
    case WBEM_E_ILLEGAL_NULL:
        psz = _T("WBEM_E_ILLEGAL_NULL");
        break;
    case WBEM_E_INVALID_QUALIFIER_TYPE:
        psz = _T("WBEM_E_INVALID_QUALIFIER_TYPE");
        break;
    case WBEM_E_INVALID_PROPERTY_TYPE:
        psz = _T("WBEM_E_INVALID_PROPERTY_TYPE");
        break;
    case WBEM_E_VALUE_OUT_OF_RANGE:
        psz = _T("WBEM_E_VALUE_OUT_OF_RANGE");
        break;
    case WBEM_E_CANNOT_BE_SINGLETON:
        psz = _T("WBEM_E_CANNOT_BE_SINGLETON");
        break;
	default:
        _itoa_s(hRes, szBuffer2, 16);
	  StringCbCopy(szBuffer, sizeof(szBuffer),_T("0x"));
      StringCbCat(szBuffer, sizeof(szBuffer), szBuffer2);
	  psz = szBuffer;
	}
	return psz;
}

// **************************************************************************
//
//	ValueToString()
//
// Description:
//		Converts a variant to a displayable string.
//
// Parameters:
//		pValue (in) - variant to be converted.
//		pbuf (out) - ptr to receive displayable string.
//
// Returns:
//		Same as pbuf.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
#define BLOCKSIZE (32 * sizeof(WCHAR))
#define CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop */

LPWSTR CConsumer::ValueToString(VARIANT *pValue, WCHAR **pbuf)
{
   DWORD iNeed = 0;
   DWORD iVSize = 0;
   DWORD iCurBufSize = 0;

   WCHAR *vbuf = NULL;
   WCHAR *buf = NULL;

   switch (pValue->vt) 
   {
   case VT_NULL: 
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
			StringCbCopyW(buf, BLOCKSIZE, L"<null>");
         break;

   case VT_BOOL: {
         VARIANT_BOOL b = pValue->boolVal;
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
		 {
			if (!b) {
				StringCbCopyW(buf, BLOCKSIZE, L"FALSE");

			} else {
				StringCbCopyW(buf, BLOCKSIZE, L"TRUE");
			}
		}
         break;
      }

   case VT_UI1: {
     	unsigned char b = pValue->bVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            if (b >= 32)
            {
                StringCbPrintfW(buf, BLOCKSIZE, L"'%c' (%hu, 0x%hX)", b, (unsigned char)b, b);
            }
            else
            {
                StringCbPrintfW(buf, BLOCKSIZE, L"%hu (0x%hX)", (unsigned char)b, b);
            }
        }
        break;
	}
   case VT_I2:  {
        USHORT i = pValue->uiVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%hu (0x%hX)", i, i);
        }
        break;
    }

   case VT_I4: {
         LONG l = pValue->lVal;
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%d (0x%X)", l, l);
        }
        break;
      }

   case VT_R4: {
        float f = pValue->fltVal;
        buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE,  L"%10.4f", f);
        }
        break;
      }

   case VT_R8: {
		double d = pValue->dblVal;
        buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
        if (buf)
        {
            StringCbPrintfW(buf, BLOCKSIZE, L"%10.4f", d);
        }
        break;
	  }

   case VT_BSTR: {
		 LPWSTR pWStr = pValue->bstrVal;
		 size_t bufSize = (wcslen(pWStr) * sizeof(WCHAR)) + sizeof(WCHAR) + (2 * sizeof(WCHAR));
		 buf = (WCHAR *)malloc(bufSize);
		 if (buf)
			StringCbPrintfW(buf, bufSize, L"\"%wS\"", pWStr);
		 break;
		}

	// the sample GUI is too simple to make it necessary to display
	// these 'complicated' types--so ignore them.
   case VT_DISPATCH:  // Currently only used for embedded objects
   case VT_BOOL|VT_ARRAY: 
   case VT_UI1|VT_ARRAY: 
   case VT_I2|VT_ARRAY: 
   case VT_I4|VT_ARRAY: 
   case VT_R4|VT_ARRAY: 
   case VT_R8|VT_ARRAY: 
   case VT_BSTR|VT_ARRAY: 
   case VT_DISPATCH | VT_ARRAY: 
         break;

   default:
         buf = (WCHAR *)malloc(BLOCKSIZE);
		 if (buf)
			StringCbCopyW(buf, BLOCKSIZE, L"<conversion error>");
   }

   *pbuf = buf;   
   return buf;
}


