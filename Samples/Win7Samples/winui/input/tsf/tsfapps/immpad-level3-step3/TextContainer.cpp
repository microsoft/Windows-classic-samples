

#include "private.h"
#include "TextContainer.h"

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::InsertText(int nPos, const WCHAR *psz, UINT nCnt)
{
    if (nCnt == 0)
        return TRUE;
    
    if (!EnsureBuffer(_nTextSize + nCnt))
    {
        return FALSE;
    }

    memmove(_psz + nPos + nCnt, _psz + nPos, (_nTextSize - nPos) * sizeof(WCHAR));
    memcpy(_psz + nPos, psz, nCnt * sizeof(WCHAR));
    _nTextSize += nCnt;
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::RemoveText(int nPos, UINT nCnt)
{
    if (!nCnt)
        return TRUE;

    if (nPos + nCnt - 1 > _nTextSize)
        nCnt = _nTextSize - nPos;

    memmove(_psz + nPos, _psz + nPos + nCnt, (_nTextSize - nPos - nCnt) * sizeof(WCHAR));
    _nTextSize -= nCnt;
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::GetText(int nPos, WCHAR *psz, UINT nCnt)
{
    if (!nCnt)
        return FALSE;

    if (nPos + nCnt - 1 > _nTextSize)
        nCnt = _nTextSize - nPos;
   
    memcpy(psz, _psz + nPos, nCnt * sizeof(WCHAR));

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextContainer::EnsureBuffer(UINT nNewTextSize)
{
    if (!nNewTextSize)
    {
        if (_psz)
            LocalFree(_psz);
        _psz = NULL;
        _nBufferSize = 0;
        _nTextSize = 0;
        return FALSE;
    }

    if (nNewTextSize <= _nTextSize)
        goto Exit;

    if (_psz)
    { 
        void *pvNew = LocalReAlloc(_psz, nNewTextSize * sizeof(WCHAR), LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        _psz = (WCHAR *)pvNew;
    }
    else
    {
        _psz = (WCHAR *)LocalAlloc(LPTR, nNewTextSize * sizeof(WCHAR));
        if (!_psz)
            return FALSE;
    }
    _nBufferSize = nNewTextSize;
Exit:
    return TRUE;
}

