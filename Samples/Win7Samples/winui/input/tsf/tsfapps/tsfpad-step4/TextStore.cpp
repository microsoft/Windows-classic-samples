#include "private.h"
#include "TextStore.h"
#include "TextEditor.h"
#include "TextInputCtrl.h"
#include "initguid.h"
#include "InputScope.h"
#include "tsattrs.h"

//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextStore::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITextStoreACP))
    {
        *ppvObj = (ITextStoreACP *)this;
    }
    else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
    {
        *ppvObj = (ITfContextOwnerCompositionSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfMouseTrackerACP))
    {
        *ppvObj = (ITfMouseTrackerACP *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextStore::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextStore::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask)
{

    if (!IsEqualGUID(riid, IID_ITextStoreACPSink))
    {
        return TS_E_NOOBJECT;
    }

    if (FAILED(punk->QueryInterface(IID_ITextStoreACPSink, (void **)&_pSink)))
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::UnadviseSink(IUnknown *punk)
{
    // we're dealing with TSF. We don't have to check punk is same instance of _pSink.
    _pSink->Release();
    _pSink = NULL;

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
{
    if (!_pSink)
    {
        return E_UNEXPECTED;
    }

    *phrSession = _pSink->OnLockGranted(dwLockFlags);
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetStatus(TS_STATUS *pdcs)
{
    pdcs->dwDynamicFlags = 0;
    pdcs->dwStaticFlags = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::QueryInsert(LONG acpInsertStart, LONG acpInsertEnd, ULONG cch, LONG *pacpResultStart, LONG *pacpResultEnd)
{
    *pacpResultStart = acpInsertStart;
    *pacpResultEnd = acpInsertEnd;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection, ULONG *pcFetched)
{
    *pcFetched = 0;
    if ((ulCount > 0) && ((ulIndex == 0) || (ulIndex == TS_DEFAULT_SELECTION)))
    {
        pSelection[0].acpStart = _pEditor->GetSelectionStart();
        pSelection[0].acpEnd = _pEditor->GetSelectionEnd();
        pSelection[0].style.ase = TS_AE_END;
        pSelection[0].style.fInterimChar = _pEditor->GetLayout()->IsInterimCaret();
        *pcFetched = 1;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection)
{
    if (ulCount > 0)
    { 
        _pEditor->MoveSelection(pSelection[0].acpStart, pSelection[0].acpEnd);

        CTextInputCtrl *pCtrl =  CTextInputCtrl::GetThis(_pEditor->GetWnd());
        if (pCtrl)
        {
            _pEditor->UpdateLayout(pCtrl->GetFont());
        }
        _pEditor->InvalidateRect();
        _pEditor->SetInterimCaret(pSelection->style.fInterimChar);
    }

    return S_OK;;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetText(LONG acpStart, LONG acpEnd, __out_ecount(cchPlainReq) WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut, TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut, LONG *pacpNext)
{

    if ((cchPlainReq == 0) && (ulRunInfoReq == 0))
    {
        return S_OK;
    }

    if (acpEnd == -1)
        acpEnd = _pEditor->GetTextLength();

    acpEnd = min(acpEnd, acpStart + (int)cchPlainReq);

    if ((acpStart != acpEnd) &&
        !_pEditor->GetText(acpStart, pchPlain, acpEnd - acpStart))
    {
        return E_FAIL;
    }

    *pcchPlainOut = acpEnd - acpStart;
    if (ulRunInfoReq)
    {
        prgRunInfo[0].uCount = acpEnd - acpStart;
        prgRunInfo[0].type = TS_RT_PLAIN;
        *pulRunInfoOut = 1;
    }

    *pacpNext = acpEnd;

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, __in_ecount(cch) const WCHAR *pchText, ULONG cch, TS_TEXTCHANGE *pChange)
{
    LONG acpRemovingEnd;

    if (acpStart > (LONG)_pEditor->GetTextLength())
        return E_INVALIDARG;

    acpRemovingEnd = min(acpEnd, (LONG)_pEditor->GetTextLength() + 1);
    if (!_pEditor->RemoveText(acpStart, acpRemovingEnd - acpStart))
        return E_FAIL;

    if (!_pEditor->InsertText(acpStart, pchText, cch))
        return E_FAIL;

    pChange->acpStart = acpStart;
    pChange->acpOldEnd = acpEnd;
    pChange->acpNewEnd = acpStart + cch;

    _pEditor->InvalidateRect();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject **ppDataObject)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown **ppunk)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject *pDataObject, TS_TEXTCHANGE *pChange)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    PrepareAttributes(cFilterAttrs, paFilterAttrs);
    if (!_nAttrVals)
        return S_FALSE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    PrepareAttributes(cFilterAttrs, paFilterAttrs);
    if (!_nAttrVals)
        return S_FALSE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset)
{
    *pacpNext = 0;
    *pfFound = FALSE;
    *plFoundOffset = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched)
{
    *pcFetched = 0;
    for (int i = 0; (i < (int)ulCount) && (i < _nAttrVals) ; i++)
    {
        paAttrVals[i] = _attrval[i];
        *pcFetched++;
    }

    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetEndACP(LONG *pacp)
{
    *pacp = _pEditor->GetTextLength();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetActiveView(TsViewCookie *pvcView)
{
    *pvcView = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetACPFromPoint(TsViewCookie vcView, const POINT *pt, DWORD dwFlags, LONG *pacp)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped)
{
    RECT rcStart;
    RECT rcEnd;

    if (_pEditor->GetLayout()->RectFromCharPos(acpStart, &rcStart) &&
        _pEditor->GetLayout()->RectFromCharPos(acpEnd, &rcEnd))
    {
        if (rcStart.top == rcEnd.top)
        {
            prc->left = min(rcStart.left, rcEnd.left);
            prc->right = max(rcStart.right, rcEnd.right);
        }
        else
        {
            RECT rcClient;
            GetClientRect(_pEditor->GetWnd(), &rcClient);
            prc->left = rcClient.left;
            prc->right = rcClient.right;
        }
        prc->top = min(rcStart.top, rcEnd.top);
        prc->bottom = max(rcStart.bottom, rcEnd.bottom);
    }
    else
    {
        prc->left = 0;
        prc->right = 0;
        prc->top = 0;
        prc->bottom = 0;
    }

    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->left);
    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->right);
    *pfClipped = FALSE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetScreenExt(TsViewCookie vcView, RECT *prc)
{
    GetClientRect(_pEditor->GetWnd(), prc);
    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->left);
    ClientToScreen(_pEditor->GetWnd(), (POINT *)&prc->right);
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::GetWnd(TsViewCookie vcView, HWND *phwnd)
{
    *phwnd = _pEditor->GetWnd();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable)
{
    return E_NOTIMPL;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::InsertTextAtSelection(DWORD dwFlags, __in_ecount(cch) const WCHAR *pchText, ULONG cch, LONG *pacpStart, LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
    LONG acpStart = _pEditor->GetSelectionStart();
    LONG acpEnd = _pEditor->GetSelectionEnd();

    if (dwFlags & TS_IAS_QUERYONLY)
    {
        *pacpStart = acpStart;
        *pacpEnd = acpStart + cch;
        return S_OK;
    }

    if (!_pEditor->RemoveText(acpStart, acpEnd - acpStart))
        return E_FAIL;

    if (pchText && !_pEditor->InsertText(acpStart, pchText, cch))
        return E_FAIL;

    
    if (pacpStart)
    {
        *pacpStart = acpStart;
    }

    if (pacpEnd)
    {
        *pacpEnd = acpStart + cch;
    }

    if (pChange)
    {
        pChange->acpStart = acpStart;
        pChange->acpOldEnd = acpEnd;
        pChange->acpNewEnd = acpStart + cch;
    }

    _pEditor->MoveSelection(acpStart, acpStart + cch);
    _pEditor->InvalidateRect();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject, LONG *pacpStart, 
        LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
    return E_NOTIMPL;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::AdviseMouseSink(ITfRangeACP *range, ITfMouseSink *pSink, DWORD *pdwCookie)
{
    DWORD dwCookie;
    if (!_prgMouseSinks)
    {
        _prgMouseSinks = (MOUSESINK *)LocalAlloc(LPTR, sizeof(MOUSESINK));
        if (!_prgMouseSinks)
        {
            return E_OUTOFMEMORY;
        }
        _nMouseSinks = 1;
        dwCookie = 1;
    }
    else
    {
        UINT i;
        for (i = 0; i < _nMouseSinks;i ++)
        {
            if (_prgMouseSinks[i].pRange == NULL)
            {
                dwCookie = i + 1;
                break;
            }
        }
        if (i == _nMouseSinks)
        {
            void *pvNew = LocalReAlloc(_prgMouseSinks, 
                                       (_nMouseSinks + 1) * sizeof(MOUSESINK), 
                                       LMEM_MOVEABLE | LMEM_ZEROINIT);
            if (!pvNew)
            {
                return E_OUTOFMEMORY;
            }
            _prgMouseSinks = (MOUSESINK *)pvNew;
            _nMouseSinks++;
            dwCookie = _nMouseSinks;
        }
    }

    _prgMouseSinks[dwCookie - 1].pRange = range;
    _prgMouseSinks[dwCookie - 1].pMouseSink = pSink;
    _prgMouseSinks[dwCookie - 1].pRange->AddRef();
    _prgMouseSinks[dwCookie - 1].pMouseSink->AddRef();
    *pdwCookie = dwCookie;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::UnadviseMouseSink(DWORD dwCookie)
{
    _prgMouseSinks[dwCookie - 1].pRange->Release();
    _prgMouseSinks[dwCookie - 1].pRange = NULL;

    _prgMouseSinks[dwCookie - 1].pMouseSink->Release();
    _prgMouseSinks[dwCookie - 1].pMouseSink = NULL;

    return S_OK;
}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

#define IF_ATTR_INPUTSCOPE           1
#define IF_ATTR_FONT_STYLE_HEIGHT    2
#define IF_ATTR_FONT_FACENAME        3
#define IF_ATTR_FONT_SIZEPTS         4
#define IF_ATTR_TEXT_READONLY        5
#define IF_ATTR_TEXT_ORIENTATION     6
#define IF_ATTR_TEXT_VERTICALWRITING 7

const GUID *c_rgSupportedAttr[7] = {&GUID_PROP_INPUTSCOPE,
                                    &TSATTRID_Font_Style_Height,
                                    &TSATTRID_Font_FaceName,
                                    &TSATTRID_Font_SizePts,
                                    &TSATTRID_Text_ReadOnly,
                                    &TSATTRID_Text_Orientation,
                                    &TSATTRID_Text_VerticalWriting};

void CTextStore::PrepareAttributes(ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    _nAttrVals= 0;
    memset(_attrval, 0, sizeof(_attrval));

    for (int i = 0; i < ARRAYSIZE(c_rgSupportedAttr); i++)
    {
        if (cFilterAttrs)
        {
            BOOL fFound = FALSE;
            for (ULONG j = 0; j < cFilterAttrs; j++)
            {
                if (IsEqualGUID(*c_rgSupportedAttr[i], paFilterAttrs[j]))
                {
                    fFound = TRUE;
                    break;
                }
            }

            if (!fFound)
            {
                continue;
            }
        }

        _attrval[_nAttrVals].idAttr = *c_rgSupportedAttr[i];
        _attrval[_nAttrVals].dwOverlapId = i + 1;

        switch (i + 1)
        {
            case IF_ATTR_INPUTSCOPE:
                _attrval[_nAttrVals].varValue.vt = VT_UNKNOWN;
                _attrval[_nAttrVals].varValue.punkVal = NULL;
                break;

            case IF_ATTR_FONT_STYLE_HEIGHT:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = _pEditor->GetLineHeight();
                break;

            case IF_ATTR_FONT_FACENAME:
                _attrval[_nAttrVals].varValue.vt = VT_BSTR;
                _attrval[_nAttrVals].varValue.bstrVal = NULL;
                break;

            case IF_ATTR_FONT_SIZEPTS:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = (int)((double)_pEditor->GetLineHeight() / 96.0 * 72.0);
                break;

            case IF_ATTR_TEXT_READONLY:
                _attrval[_nAttrVals].varValue.vt = VT_BOOL;
                _attrval[_nAttrVals].varValue.bVal = FALSE;
                break;

            case IF_ATTR_TEXT_ORIENTATION:
                _attrval[_nAttrVals].varValue.vt = VT_I4;
                _attrval[_nAttrVals].varValue.lVal = 0;
                break;

            case IF_ATTR_TEXT_VERTICALWRITING:
                _attrval[_nAttrVals].varValue.vt = VT_BOOL;
                _attrval[_nAttrVals].varValue.bVal = FALSE;
                break;
        }

        _nAttrVals++;
    }

}


//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
{
    if (_pCurrentCompositionView)
    {
        _pCurrentCompositionView->Release();
        _pCurrentCompositionView = NULL;
    }

    _pCurrentCompositionView = pComposition;
    _pCurrentCompositionView->AddRef();

    *pfOk = TRUE;
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::OnUpdateComposition(ITfCompositionView *pComposition,
                                ITfRange *pRangeNew)
{
    if (_pCurrentCompositionView)
    {
        _pCurrentCompositionView->Release();
        _pCurrentCompositionView = NULL;
    }

    _pCurrentCompositionView = pComposition;
    _pCurrentCompositionView->AddRef();
    return S_OK;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

STDAPI CTextStore::OnEndComposition(ITfCompositionView *pComposition)
{
    if (_pCurrentCompositionView)
    {
        _pCurrentCompositionView->Release();
        _pCurrentCompositionView = NULL;
    }

    return S_OK;
}
