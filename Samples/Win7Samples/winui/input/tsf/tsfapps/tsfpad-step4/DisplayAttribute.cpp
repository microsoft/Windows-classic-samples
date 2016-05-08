#include "private.h"
#include "DisplayAttribute.h"

ITfDisplayAttributeMgr *g_pdam = NULL;

CDispAttrProps *GetDispAttrProps()
{
    IEnumGUID *pEnumProp = NULL;
    CDispAttrProps *pProps = NULL;
    ITfCategoryMgr *pcat;
    HRESULT hr = E_FAIL;
    if (SUCCEEDED(hr = CoCreateInstance(CLSID_TF_CategoryMgr,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_ITfCategoryMgr,
                                   (void**)&pcat)))
    {
        hr = pcat->EnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROPERTY, &pEnumProp);
        pcat->Release();
    }

    //
    // make a database for Display Attribute Properties.
    //
    if (SUCCEEDED(hr) && pEnumProp)
    {
         GUID guidProp;
         pProps = new CDispAttrProps;

         //
         // add System Display Attribute first.
         // so no other Display Attribute property overwrite it.
         //
         pProps->Add(GUID_PROP_ATTRIBUTE);
         while(pEnumProp->Next(1, &guidProp, NULL) == S_OK)
         {
             if (!IsEqualGUID(guidProp, GUID_PROP_ATTRIBUTE))
                 pProps->Add(guidProp);
         }
    }

    if (pEnumProp)
        pEnumProp->Release();

    return pProps;
}

//+---------------------------------------------------------------------------
//
//  InitDisplayAttributeLib
//
//----------------------------------------------------------------------------

HRESULT InitDisplayAttrbute()
{

    if (g_pdam)
        g_pdam->Release();

    g_pdam = NULL;

    if (FAILED(CoCreateInstance(CLSID_TF_DisplayAttributeMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfDisplayAttributeMgr, 
                                   (void**)&g_pdam)))
    {
        return E_FAIL;
    }


    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  UninitDisplayAttributeLib
//
//----------------------------------------------------------------------------

HRESULT UninitDisplayAttrbute()
{

    if (g_pdam)
    {
        g_pdam->Release();
        g_pdam = NULL;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  GetDisplayAttributeTrackPropertyRange
//
//----------------------------------------------------------------------------

HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, CDispAttrProps *pDispAttrProps)
{
    ITfReadOnlyProperty *pProp = NULL;
    HRESULT hr = E_FAIL;
    GUID  *pguidProp = NULL;
    const GUID **ppguidProp;
    ULONG ulNumProp = 0;
    ULONG i;

    if (!pDispAttrProps)
         goto Exit;
 
    pguidProp = pDispAttrProps->GetPropTable();
    if (!pguidProp)
         goto Exit;

    ulNumProp = pDispAttrProps->Count();
    if (!ulNumProp)
         goto Exit;

    // TrackProperties wants an array of GUID *'s
    if ((ppguidProp = (const GUID **)LocalAlloc(LMEM_ZEROINIT, sizeof(GUID *)*ulNumProp)) == NULL)
        return E_OUTOFMEMORY;

    for (i=0; i<ulNumProp; i++)
    {
        ppguidProp[i] = pguidProp++;
    }
    
    if (SUCCEEDED(hr = pic->TrackProperties(ppguidProp, 
                                            ulNumProp,
                                            0,
                                            NULL,
                                            &pProp)))
    {
        *ppProp = pProp;
    }

    LocalFree(ppguidProp);

Exit:
    return hr;
}

//+---------------------------------------------------------------------------
//
//  GetDisplayAttributeData
//
//----------------------------------------------------------------------------

HRESULT GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfGuidAtom *pguid)
{
    VARIANT var;
    IEnumTfPropertyValue *pEnumPropertyVal;
    TF_PROPERTYVAL tfPropVal;
    GUID guid;
    TfGuidAtom gaVal;
    ITfDisplayAttributeInfo *pDAI;

    HRESULT hr = E_FAIL;

    ITfCategoryMgr *pcat = NULL;
    if (FAILED(hr = CoCreateInstance(CLSID_TF_CategoryMgr,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_ITfCategoryMgr,
                               (void**)&pcat)))
    {
        return hr;
    }

    hr = S_FALSE;
    if (SUCCEEDED(pProp->GetValue(ec, pRange, &var)))
    {
        if (SUCCEEDED(var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, 
                                                  (void **)&pEnumPropertyVal)))
        {
            while (pEnumPropertyVal->Next(1, &tfPropVal, NULL) == S_OK)
            {
                if (tfPropVal.varValue.vt == VT_EMPTY)
                    continue; // prop has no value over this span

                gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                pcat->GetGUID(gaVal, &guid);

                if ((g_pdam != NULL) && SUCCEEDED(g_pdam->GetDisplayAttributeInfo(guid, &pDAI, NULL)))
                {
                    //
                    // Issue: for simple apps.
                    // 
                    // Small apps can not show multi underline. So
                    // this helper function returns only one 
                    // DISPLAYATTRIBUTE structure.
                    //
                    if (pda)
                    {
                        pDAI->GetAttributeInfo(pda);
                    }

                    if (pguid)
                    {
                        *pguid = gaVal;
                    }

                    pDAI->Release();
                    hr = S_OK;
                    break;
                }
            }
            pEnumPropertyVal->Release();
        }
        VariantClear(&var);
    }

    pcat->Release();

    return hr;
}


