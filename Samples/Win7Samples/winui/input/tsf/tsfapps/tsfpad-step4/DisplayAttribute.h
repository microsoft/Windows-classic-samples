//
// dispattr.h
//

#pragma once

#include "ctffunc.h"

#define MAX_DISPATTRGUID 10


class CDispAttrProps
{
public:
    CDispAttrProps() 
    {
        for (int i = 0;i < MAX_DISPATTRGUID; i++)
        {
             _guid[i] = GUID_NULL;
             _rgpguid[i] = &_guid[i];
        }
        _nCount = 0;
    }

    void Add(REFGUID rguid)
    {
        if (!FindGuid(rguid) && _nCount < MAX_DISPATTRGUID)
        {
            _guid[_nCount] = rguid;
            _nCount++;
        }
        
    }

    BOOL FindGuid(REFGUID rguid)
    {
        int i;
        for (i = 0; i < _nCount; i++)
        {
            if (IsEqualGUID(_guid[i], rguid))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    int Count()
    {
        return _nCount;
    }

    GUID *GetPropTable()
    {
        return _guid;
    }

    const GUID **GetPropTablePointer()
    {
        return _rgpguid;
    }

private:
    GUID _guid[MAX_DISPATTRGUID];
    const GUID* _rgpguid[MAX_DISPATTRGUID];
    int _nCount;
};

CDispAttrProps *GetDispAttrProps();

HRESULT InitDisplayAttrbute();
HRESULT UninitDisplayAttrbute();
HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, CDispAttrProps *pDispAttrProps);
HRESULT GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfClientId *pguid);

HRESULT GetAttributeColor(TF_DA_COLOR *pdac, COLORREF *pcr);
HRESULT SetAttributeColor(TF_DA_COLOR *pdac, COLORREF cr);
HRESULT SetAttributeSysColor(TF_DA_COLOR *pdac, int nIndex);
HRESULT ClearAttributeColor(TF_DA_COLOR *pdac);

