//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  DumpProperties.cpp
//
//          DumpPeoperties
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"
#include "MemoryStream.h"

#define GETTEXT_CNT 256


//
// These GUID comes from PropertyTextService\Globals.cpp
//
/* b4126de5-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropStaticCompact = {
    0xb4126de5,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* b4126de6-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropStatic = {
    0xb4126de6,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* b4126de7-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropCustom = {
    0xb4126de7,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

typedef struct tag_GUIDINFO {
    const GUID  *pguid;
    WCHAR       szDesc[50];
} GUIDINFO;

GUIDINFO g_giKnownGuids[] = {
    {&GUID_PROP_TEXTOWNER,     L"GUID_PROP_TEXTOWNER"},
    {&GUID_PROP_ATTRIBUTE,     L"GUID_PROP_ATTRIBUTE"},
    {&GUID_PROP_LANGID,        L"GUID_PROP_LANGID"},
    {&GUID_PROP_READING,       L"GUID_PROP_READING"},
    {&GUID_PROP_COMPOSING,     L"GUID_PROP_COMPOSING"},
    {&GUID_PROP_MODEBIAS,      L"GUID_PROP_COMPOSING"},
    {&c_guidPropStaticCompact, L"Property TextService Static Compact"},
    {&c_guidPropStatic,        L"Property TextService Static"},
    {&c_guidPropCustom,        L"Property TextService Custom"},
    {&GUID_NULL,           L""}};

//+---------------------------------------------------------------------------
//
// CLSIDToStringW
//
//----------------------------------------------------------------------------

BOOL CLSIDToStringW(REFGUID refGUID, WCHAR *pch)
{
    static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                   8, 9, '-', 10, 11, 12, 13, 14, 15};

    static const WCHAR szDigits[] = L"0123456789ABCDEF";

    int i;
    WCHAR *p = pch;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = L'{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == L'-')
        {
            *p++ = L'-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = L'}';
    *p   = L'\0';

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// CDumpPropertiesEditSession
//
//----------------------------------------------------------------------------

class CDumpPropertiesEditSession : public CEditSessionBase
{
public:
    CDumpPropertiesEditSession(CPropertyMonitorTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CDumpPropertiesEditSession::DoEditSession(TfEditCookie ec)
{
    _pTextService->_DumpProperties(ec, _pContext);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// DumpProperties
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::DumpProperties(ITfContext *pContext)
{
    CDumpPropertiesEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    if ((pEditSession = new CDumpPropertiesEditSession(this, pContext)) == NULL)
        goto Exit;

    pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

    pEditSession->Release();

Exit:
    return;
}

//+---------------------------------------------------------------------------
//
// _DumpPropertyInfo
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_DumpPropertyInfo(REFGUID rguid)
{
    WCHAR sz[512];
    CLSIDToStringW(rguid, sz);
    AddStringToStream(_pMemStream, sz);

    int i = 0;
    while (!IsEqualGUID(*g_giKnownGuids[i].pguid, GUID_NULL))
    {
        if (IsEqualGUID(*g_giKnownGuids[i].pguid, rguid))
        {
            AddStringToStream(_pMemStream, L"\t");
            AddStringToStream(_pMemStream, g_giKnownGuids[i].szDesc);
            break;
        }
        i++;
    }
    AddStringToStream(_pMemStream, L"\r\n");
}

//+---------------------------------------------------------------------------
//
// _IsDisplayAttributeProperty
//
//----------------------------------------------------------------------------

BOOL CPropertyMonitorTextService::_IsDisplayAttributeProperty(REFGUID rguid)
{
    IEnumGUID *pEnum;
    if (FAILED(_pCategoryMgr->EnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROPERTY, &pEnum)))
        return FALSE;

    GUID guid;
    BOOL fRet = FALSE;
    while(pEnum->Next(1, &guid, NULL) == S_OK)
    {
        if (IsEqualGUID(guid, rguid))
        {
            fRet = TRUE;
            break;
        }
    }

    pEnum->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _GetTextLengthInRange
//
//----------------------------------------------------------------------------

HRESULT CPropertyMonitorTextService::_GetTextLengthInRange(TfEditCookie ec, ITfRange *pRange, LONG *pcch)
{
    ITfRange *pTempRange;

    *pcch = 0;
    if (SUCCEEDED(pRange->Clone(&pTempRange)))
    {
        WCHAR sz[64];
        ULONG cchTemp = 0;
        while ((pTempRange->GetText(ec, TF_TF_MOVESTART, sz, sizeof(sz) / sizeof(WCHAR), &cchTemp) == S_OK) && cchTemp)
        {
            *pcch += cchTemp;
        }

        pTempRange->Release();
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _GetTextExtent
//
//----------------------------------------------------------------------------

HRESULT CPropertyMonitorTextService::_GetTextExtent(TfEditCookie ec, ITfRange *pRange, LONG *pacp, LONG *pcch)
{
    ITfContext *pContext;
    ITfRange *pStart;

    *pacp = 0;
    *pcch = 0;

    if (FAILED(pRange->GetContext(&pContext)))
        return E_FAIL;

    if (SUCCEEDED(pContext->GetStart(ec, &pStart)))
    {
        //
        // In fact, it is not good idea to try to get the position in general.
        // Accessing the text from begging may cause a performance problem
        // if it is the huge document.
        // This is a property monitor tool and provide the detail property
        // information such as debugging purpose.
        //
        pStart->ShiftEndToRange(ec, pRange, TF_ANCHOR_START);
        _GetTextLengthInRange(ec, pStart, pacp);

        //
        // Getting the length of the range is ordinal action. 
        //
        _GetTextLengthInRange(ec, pRange, pcch);

        pStart->Release();
    }
    
    pContext->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _DumpPropertyRange
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_DumpPropertyRange(REFGUID rguid, TfEditCookie ec, ITfProperty *pprop, ITfRange *prange)
{
    WCHAR sz[512];

    LONG acp;
    LONG cch;

    if (SUCCEEDED(_GetTextExtent(ec, prange, &acp, &cch)))
    {
        StringCchPrintf(sz, ARRAYSIZE(sz), L"%d\t%d", acp, cch);
        AddStringToStream(_pMemStream, sz);

        VARIANT var;
        VariantInit(&var);
        
        if (SUCCEEDED(pprop->GetValue(ec, prange, &var)))
        {

            if (IsEqualGUID(rguid, GUID_PROP_TEXTOWNER))
            {
                if (var.vt == VT_I4)
                {
                    GUID guid = GUID_NULL;
                    _pCategoryMgr->GetGUID((TfGuidAtom)var.lVal, &guid);
                    WCHAR sz[512];
                    CLSIDToStringW(guid, sz);
                    AddStringToStream(_pMemStream, L"\tclsid-");
                    AddStringToStream(_pMemStream, sz);
                }
            }
            else if (IsEqualGUID(rguid, GUID_PROP_LANGID))
            {
                if (var.vt == VT_I4)
                {
                    AddStringToStream(_pMemStream, L"\tlangid ");
                    StringCchPrintf(sz, ARRAYSIZE(sz), L"0x%04x", var.lVal);
                    AddStringToStream(_pMemStream, sz);
                }
            }
            else if (IsEqualGUID(rguid, GUID_PROP_READING))
            {
                if (var.vt == VT_BSTR)
                {
                    AddStringToStream(_pMemStream, L"\tReading-\"");
                    AddStringToStream(_pMemStream, var.bstrVal);
                    AddStringToStream(_pMemStream, L"\"");
                }
            }
            else if (IsEqualGUID(rguid, GUID_PROP_COMPOSING))
            {
                if (var.vt == VT_I4)
                {
                    AddStringToStream(_pMemStream, L"\t");
                    StringCchPrintf(sz, ARRAYSIZE(sz), L"0x%08x", var.lVal);
                    AddStringToStream(_pMemStream, sz);
                }
            }
            else if (_IsDisplayAttributeProperty(rguid))
            {
                if (var.vt == VT_I4)
                {

                    ITfDisplayAttributeInfo *pDisplatAttributeInfo;
                    GUID guid = GUID_NULL;
                    _pCategoryMgr->GetGUID((TfGuidAtom)var.lVal, &guid);
                    if (SUCCEEDED(_pDisplayAttributeMgr->GetDisplayAttributeInfo(guid, &pDisplatAttributeInfo, NULL)))
                    {
                        TF_DISPLAYATTRIBUTE da;
                        if (SUCCEEDED(pDisplatAttributeInfo->GetAttributeInfo(&da)))
                        {
                            StringCchPrintf(sz, ARRAYSIZE(sz), L"\tcrText:0x%x",da.crText.nIndex);
                            AddStringToStream(_pMemStream, sz);
                            StringCchPrintf(sz, ARRAYSIZE(sz), L" crBk:0x%x",da.crBk.nIndex);
                            AddStringToStream(_pMemStream, sz);
                            StringCchPrintf(sz, ARRAYSIZE(sz), L" lsStyle:0x%x",da.lsStyle);
                            AddStringToStream(_pMemStream, sz);
                            StringCchPrintf(sz, ARRAYSIZE(sz), L" fBoldLine:0x%x",da.fBoldLine);
                            AddStringToStream(_pMemStream, sz);
                            StringCchPrintf(sz, ARRAYSIZE(sz), L" crLine:0x%x",da.crLine.nIndex);
                            AddStringToStream(_pMemStream, sz);
                        }
                        pDisplatAttributeInfo->Release();
                    }
                }
            }
            else if (IsEqualGUID(rguid, c_guidPropStaticCompact))
            {
                if (var.vt == VT_I4)
                {
                    AddStringToStream(_pMemStream, L"\tvalue ");
                    StringCchPrintf(sz, ARRAYSIZE(sz), L"0x%04x", var.lVal);
                    AddStringToStream(_pMemStream, sz);
                }
            }
            else if (IsEqualGUID(rguid, c_guidPropStatic))
            {
                if (var.vt == VT_BSTR)
                {
                    AddStringToStream(_pMemStream, L"\tvalue-\"");
                    AddStringToStream(_pMemStream, var.bstrVal);
                    AddStringToStream(_pMemStream, L"\"");
                }
            }
            else if (IsEqualGUID(rguid, c_guidPropCustom))
            {
                if (var.vt == VT_I4)
                {
                    AddStringToStream(_pMemStream, L"\tvalue ");
                    StringCchPrintf(sz, ARRAYSIZE(sz), L"0x%04x", var.lVal);
                    AddStringToStream(_pMemStream, sz);
                }
            }

            // AddStringToStream(_pMemStream, L"\t");
            // _DumpVariant(&var);

            VariantClear(&var);
        }

        AddStringToStream(_pMemStream, L"\tText:\"");
        _DumpRange(ec, prange);
        AddStringToStream(_pMemStream, L"\"");

        AddStringToStream(_pMemStream, L"\r\n");

    }
}

//+---------------------------------------------------------------------------
//
// _DumpProperties
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_DumpProperties(TfEditCookie ec, ITfContext *pContext)
{
    IEnumTfProperties *penum = NULL;
    ITfProperty *pprop;

    if (FAILED(pContext->EnumProperties(&penum)))
        goto Exit;

    ClearStream(_pMemStream);
 
    while (penum->Next(1, &pprop, NULL) == S_OK)
    {
        IEnumTfRanges *penumRanges;
        GUID guid;

        pprop->GetType(&guid);
        _DumpPropertyInfo(guid);

        if (SUCCEEDED(pprop->EnumRanges(ec, &penumRanges, NULL)))
        {
            ITfRange *prange;
            while(penumRanges->Next(1, &prange, NULL) == S_OK)
            {
                _DumpPropertyRange(guid, ec, pprop, prange);
                prange->Release();
            }
            penumRanges->Release();
        }
        pprop->Release();
    }

    _ShowPopupWindow();

Exit:
    if (penum)
        penum->Release();
    return;
}

//+---------------------------------------------------------------------------
//
// _DumpRange
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_DumpRange(TfEditCookie ec, ITfRange *prange)
{
    ULONG cchOut = 0;
    WCHAR sz[512];

    sz[0] = L'\0';
    prange->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz) - 1, &cchOut);
    sz[cchOut] = L'\0';
    AddStringToStream(_pMemStream, sz);

    return;
}

//+---------------------------------------------------------------------------
//
// _DumpVariant
//
//----------------------------------------------------------------------------

void CPropertyMonitorTextService::_DumpVariant(VARIANT *pvar)
{
    WCHAR sz[512];
     
    switch (pvar->vt)
    {
        case VT_EMPTY:
             AddStringToStream(_pMemStream, L"VT_EMPTY");
             break;
        case VT_NULL:
             AddStringToStream(_pMemStream, L"VT_NULL");
             break;
        case VT_I4:
             StringCchPrintf(sz, ARRAYSIZE(sz), L"VT_I4:0x%08x", pvar->lVal);
             AddStringToStream(_pMemStream, sz);
             break;
        case VT_BSTR:
             AddStringToStream(_pMemStream, L"VT_BSTR:");
             AddStringToStream(_pMemStream, pvar->bstrVal);
             break;
        case VT_UNKNOWN:
             StringCchPrintf(sz, ARRAYSIZE(sz), L"VT_UNKNOWN:0x%08x", pvar->lVal);
             AddStringToStream(_pMemStream, sz);
             break;
        default:
             StringCchPrintf(sz, ARRAYSIZE(sz), L"VT_0x%04x", pvar->vt);
             AddStringToStream(_pMemStream, sz);
             break;
     }
}
