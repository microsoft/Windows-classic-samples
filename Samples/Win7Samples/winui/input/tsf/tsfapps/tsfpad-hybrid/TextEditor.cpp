
#include "private.h"
#include "TextEditor.h"

extern ITfThreadMgr *g_pThreadMgr;
extern TfClientId g_TfClientId;

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelection(UINT nSelStart, UINT nSelEnd)
{
    UINT nTextLength = GetTextLength();
    if (nSelStart >= nTextLength)
        nSelStart = nTextLength;

    if (nSelEnd >= nTextLength)
        nSelEnd = nTextLength;

    _nSelStart = nSelStart;
    _nSelEnd = nSelEnd;

    _pTextStore->OnSelectionChange();

    SetCompositionForm();
    if (!_layout.IsInterimCaret())
        FlushCompositionString();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionNext()
{
    UINT nTextLength = GetTextLength();
    if (_nSelEnd < nTextLength)
       _nSelEnd++;

    _nSelStart = _nSelEnd;
    _pTextStore->OnSelectionChange();

    SetCompositionForm();
    FlushCompositionString();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionPrev()
{
    if (_nSelStart > 0)
        _nSelStart--;

    _nSelEnd = _nSelStart;
    _pTextStore->OnSelectionChange();

    SetCompositionForm();
    FlushCompositionString();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionAtPoint(POINT pt)
{
    BOOL bRet = FALSE;
    UINT nSel = _layout.CharPosFromPoint(pt);
    if (nSel != (UINT)-1)
    {
        MoveSelection(nSel, nSel);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionUpDown(BOOL bUp)
{
    RECT rc;
    if (!_layout.RectFromCharPos(_nSelStart, &rc))
        return FALSE;

    POINT pt;
    pt.x = rc.left;
    if (bUp)
    {
        pt.y = rc.top - ((rc.bottom - rc.top) / 2);
        if (pt.y < 0)
            return FALSE;
    }
    else
    {
        pt.y = rc.bottom + ((rc.bottom - rc.top) / 2);
    }

    return MoveSelectionAtPoint(pt);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionToLineFirstEnd(BOOL bFirst)
{
    BOOL bRet = FALSE;
    UINT nSel;

    if (bFirst)
    {
        nSel = _layout.FineFirstEndCharPosInLine(_nSelStart, TRUE);
    }
    else
    {
        nSel = _layout.FineFirstEndCharPosInLine(_nSelEnd, FALSE);
    }

    if (nSel != (UINT)-1)
    {
        MoveSelection(nSel, nSel);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
    LONG lOldSelEnd = _nSelEnd;
    if (!RemoveText(_nSelStart, _nSelEnd - _nSelStart))
        return FALSE;

    if (!InsertText(_nSelStart, psz, lstrlen(psz)))
        return FALSE;

    _nSelStart += lstrlen(psz);
    _nSelEnd = _nSelStart;

    _pTextStore->OnTextChange(_nSelStart, lOldSelEnd, _nSelEnd);
    _pTextStore->OnSelectionChange();
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
    if (!fBack && (_nSelEnd < GetTextLength()))
    {
        if (!RemoveText(_nSelEnd, 1))
            return FALSE;

        _pTextStore->OnTextChange(_nSelEnd, _nSelEnd + 1, _nSelEnd);
    }

    if (fBack && (_nSelStart > 0))
    {
        if (!RemoveText(_nSelStart - 1, 1))
            return FALSE;

        _nSelStart--;
        _nSelEnd = _nSelStart;

        _pTextStore->OnTextChange(_nSelStart, _nSelStart + 1, _nSelStart);
        _pTextStore->OnSelectionChange();
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteSelection()
{
    ULONG nSelOldEnd = _nSelEnd;
    RemoveText(_nSelStart, _nSelEnd - _nSelStart);

    _nSelEnd = _nSelStart;

    _pTextStore->OnTextChange(_nSelStart, nSelOldEnd, _nSelStart);
    _pTextStore->OnSelectionChange();

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::Render(HDC hdc, const LOGFONT *plf)
{
    HFONT hFont = CreateFontIndirect(plf);

    if (hFont)
    {
        HFONT hFontOrg = (HFONT)SelectObject(hdc, hFont);

        _layout.Layout(hdc, GetTextBuffer(), GetTextLength());
        _layout.Render(hdc, GetTextBuffer(), GetTextLength(), _nSelStart, _nSelEnd, _pCompositionRenderInfo, _nCompositionRenderInfo);

        SelectObject(hdc, hFontOrg);
        DeleteObject(hFont);
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::UpdateLayout(const LOGFONT *plf)
{
    HDC hdc = GetDC(_hwnd);
    if (hdc)
    {
        HFONT hFont = CreateFontIndirect(plf);
        if (hFont)
        {
            HFONT hFontOrg = (HFONT)SelectObject(hdc, hFont);

            _layout.Layout(hdc, GetTextBuffer(), GetTextLength());

            SelectObject(hdc, hFontOrg);
            DeleteObject(hFont);
        }
        ReleaseDC(_hwnd, hdc);
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InitTSF()
{

    _pTextStore = new CTextStore(this);
    if (!_pTextStore)
    {
        return FALSE;
    }

    if (FAILED(g_pThreadMgr->CreateDocumentMgr(&_pDocumentMgr)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->CreateContext(g_TfClientId, 0, (ITextStoreACP *)_pTextStore, &_pInputContext, &_ecTextStore)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->Push(_pInputContext)))
    {
        return FALSE;
    }

    ITfDocumentMgr *pDocumentMgrPrev;
    g_pThreadMgr->AssociateFocus(_hwnd, _pDocumentMgr, &pDocumentMgrPrev);
    if (pDocumentMgrPrev)
        pDocumentMgrPrev->Release();

    _pTextEditSink = new CTextEditSink(this);
    if (!_pTextEditSink)
    {
        return FALSE;
    }

    _pTextEditSink->_Advise(_pInputContext);

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::UninitTSF()
{
    if (_pTextEditSink)
    {
        _pTextEditSink->_Unadvise();
        _pTextEditSink->Release();
        _pTextEditSink = NULL;
    }

    if (_pDocumentMgr)
    {
        _pDocumentMgr->Pop(TF_POPF_ALL);
    }

    if (_pInputContext)
    {
        _pInputContext->Release();
        _pInputContext = NULL;
    }

    if (_pDocumentMgr)
    {
        _pDocumentMgr->Release();
        _pDocumentMgr = NULL;
    }

    if (_pTextStore)
    {
        _pTextStore->Release();
        _pTextStore = NULL;
    }

    return TRUE;
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::SetFocusDocumentMgr()
{
    if (_pDocumentMgr)
    {
        // g_pThreadMgr->SetFocus(_pDocumentMgr);
    }
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::ClearCompositionRenderInfo()
{
    if (_pCompositionRenderInfo)
    {
        LocalFree(_pCompositionRenderInfo);
        _pCompositionRenderInfo = NULL;
        _nCompositionRenderInfo = 0;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda)
{
    if (_pCompositionRenderInfo)
    {
        void *pvNew = LocalReAlloc(_pCompositionRenderInfo, 
                                   (_nCompositionRenderInfo + 1) * sizeof(COMPOSITIONRENDERINFO),
                                   LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!pvNew)
            return FALSE;

        _pCompositionRenderInfo = (COMPOSITIONRENDERINFO *)pvNew;
    }
    else
    {
        _pCompositionRenderInfo = (COMPOSITIONRENDERINFO *)LocalAlloc(LPTR,
                                   (_nCompositionRenderInfo + 1) * sizeof(COMPOSITIONRENDERINFO));
        if (!_pCompositionRenderInfo)
            return FALSE;
    }
    _pCompositionRenderInfo[_nCompositionRenderInfo].nStart = nStart;
    _pCompositionRenderInfo[_nCompositionRenderInfo].nEnd = nEnd;
    _pCompositionRenderInfo[_nCompositionRenderInfo].da = *pda;
    _nCompositionRenderInfo++;

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::TerminateCompositionString()
{
    if (_pTextStore->GetCurrentCompositionView())
    {
        ITfContextOwnerCompositionServices *pCompositionServices;
        if (_pInputContext->QueryInterface(IID_ITfContextOwnerCompositionServices, (void **)&pCompositionServices) == S_OK)
        {
            pCompositionServices->TerminateComposition(_pTextStore->GetCurrentCompositionView());
            pCompositionServices->Release();
        }
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::AleartMouseSink(POINT pt, DWORD dwBtnState, BOOL *pbEaten)
{
    BOOL bRet = FALSE;
    UINT nSel = _layout.ExactCharPosFromPoint(pt);
    if (nSel == (UINT)-1)
    {
        return;
    }

    RECT rc;
    if (!_layout.RectFromCharPos(nSel, &rc))
    {
        return;
    }
    
    int nPos = (pt.x - rc.left) * 4 / (rc.right - rc.left) + 2;
    UINT uEdge = nSel + (nPos / 4);
    UINT uQuadrant = nPos % 4;

    _pTextStore->OnMouseEvent(uEdge, uQuadrant, dwBtnState, pbEaten);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::SetCompositionForm()
{
    HIMC himc = ImmGetContext(_hwnd);
    if (himc)
    {
        RECT rc;
        COMPOSITIONFORM cf;
        cf.dwStyle = CFS_POINT;
        if (_layout.RectFromCharPos(_nSelEnd, &rc))
        {
            cf.ptCurrentPos.x = rc.left;
            cf.ptCurrentPos.y = rc.top;
        }
        else
        {
            cf.ptCurrentPos.x = 0;
            cf.ptCurrentPos.y = 0;
        }
        ImmSetCompositionWindow(himc, &cf);
    }

    ImmReleaseContext(_hwnd, himc);

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::FlushCompositionString()
{
    // We flush the composition string at selection change.
    HIMC himc = ImmGetContext(_hwnd);
    if (himc)
    {
        ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        ImmReleaseContext(_hwnd, himc);
    }
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InsertResultAtComposition(LPCWSTR psz)
{
    if (!RemoveText(_nCompStart, _nCompEnd - _nCompStart))
        return FALSE;

    if (!InsertText(_nCompStart, psz, lstrlen(psz)))
        return FALSE;

    _nSelStart = _nCompStart += lstrlen(psz);
    _nSelEnd = _nSelStart;
    _nCompStart = _nSelStart;
    _nCompEnd = _nSelEnd;

    ClearCompositionRenderInfo();
 
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::UpdateComposition(LPCWSTR psz, LONG lDeltaStart, LONG lCursorPos,
                                    const BYTE *prgAttr, LONG lAttr, const LONG *prgClauseInfo, LONG lClauseInfo)
{
    UINT nDeltaStart = (lDeltaStart >= 0) ? (UINT)lDeltaStart : 0;
    UINT nCursorPos = (lCursorPos >= 0) ? (UINT)lCursorPos : 0;
    if (!RemoveText(_nCompStart + nDeltaStart, _nCompEnd - _nCompStart - nDeltaStart))
        return FALSE;

    if (!InsertText(_nCompStart + nDeltaStart, psz + nDeltaStart, lstrlen(psz) - nDeltaStart))
        return FALSE;

    _nCompEnd = _nCompStart + lstrlen(psz);
    _nSelStart = _nCompStart + nCursorPos;
    _nSelEnd = _nCompStart + nCursorPos;

    ClearCompositionRenderInfo();

    
    if (prgAttr && prgClauseInfo)
    {
        for (UINT i = 0; i < (lClauseInfo / sizeof(DWORD)) - 1; i++)
        {
            TF_DISPLAYATTRIBUTE da;
            memset(&da, 0, sizeof(da));

            da.crLine.type = TF_CT_SYSCOLOR;
            da.crLine.nIndex = COLOR_WINDOWTEXT;

            switch (prgAttr[prgClauseInfo[i]])
            {
                case ATTR_INPUT:
                    da.fBoldLine = TRUE;
                    da.lsStyle = TF_LS_DOT;
                    break;

                case ATTR_TARGET_CONVERTED:
                case ATTR_TARGET_NOTCONVERTED:
                    da.fBoldLine = TRUE;
                    da.lsStyle = TF_LS_SOLID;
                    break;

                case ATTR_CONVERTED:
                case ATTR_INPUT_ERROR:
                case ATTR_FIXEDCONVERTED:
                default:
                    da.lsStyle = TF_LS_SOLID;
                    break;
            }
            AddCompositionRenderInfo(_nCompStart + prgClauseInfo[i], _nCompStart + prgClauseInfo[i+1], &da);
        }
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::SetCandidateForm()
{
    HIMC himc = ImmGetContext(_hwnd);
    if (himc)
    {
        RECT rc;
        RECT rcStart;
        RECT rcEnd;
        CANDIDATEFORM cf;
        cf.dwIndex = 0;
        cf.dwStyle = CFS_EXCLUDE;

        if (_layout.RectFromCharPos(_nCompStart, &rcStart) &&
            _layout.RectFromCharPos(_nCompEnd, &rcEnd) &&
            _layout.RectFromCharPos(_nSelEnd, &rc))
        {
            cf.ptCurrentPos.x = rc.left;
            cf.ptCurrentPos.y = rc.bottom - 1;
            if (rcStart.top == rcEnd.top)
            {
                cf.rcArea.left = min(rcStart.left, rcEnd.left);
                cf.rcArea.right = max(rcStart.right, rcEnd.right);
            }
            else
            {
                RECT rcClient;
                GetClientRect(_hwnd, &rcClient);
                cf.rcArea.left = rcClient.left;
                cf.rcArea.right = rcClient.right;
            }
            cf.rcArea.top = min(rcStart.top, rcEnd.top);
            cf.rcArea.bottom = max(rcStart.bottom, rcEnd.bottom);
        }
        else
        {
            cf.ptCurrentPos.x = 0;
            cf.ptCurrentPos.y = 0;
            cf.rcArea.left = 0;
            cf.rcArea.right = 0;
            cf.rcArea.top = 0;
            cf.rcArea.bottom = 0;
        }
        ImmSetCandidateWindow(himc, &cf);
    }
    ImmReleaseContext(_hwnd, himc);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::QueryCharPosition(IMECHARPOSITION *pcpos)
{
    RECT rc;
    RECT rcClient;

    if (_layout.RectFromCharPos(_nCompStart + pcpos->dwCharPos, &rc))
    {
        pcpos->pt.x = rc.left;
        pcpos->pt.y = rc.top;
    }
    else
    {
        pcpos->pt.x = 0;
        pcpos->pt.y = 0;
    }
    ClientToScreen(_hwnd, &pcpos->pt);

    pcpos->cLineHeight = _layout.GetLineHeight();

    GetClientRect(_hwnd, &rcClient);
    ClientToScreen(_hwnd, (POINT *)&rcClient.left);
    ClientToScreen(_hwnd, (POINT *)&rcClient.right);
    pcpos->rcDocument = rcClient;

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

ULONG CTextEditor::OnDocumentFeed(RECONVERTSTRING *pReconv)
{
    // Return the current composition string.
    UINT nCompStart = (_nCompEnd - _nCompStart) ? _nCompStart : _nSelStart;
    UINT nCompEnd = (_nCompEnd - _nCompStart) ? _nCompEnd : _nSelEnd;
    DWORD dwSize;
    DWORD dwStrStart;
    DWORD dwStrLen;

    if (nCompStart > 20)
    {
        dwStrStart = nCompStart - 20;
    }
    else
    {
        dwStrStart = 0;
    }

    if (nCompEnd + 20 > GetTextLength())
    {
        dwStrLen = GetTextLength() - dwStrStart;
    }
    else
    {
        dwStrLen = nCompEnd + 20 - dwStrStart;
    }

    dwSize = sizeof(RECONVERTSTRING) + (dwStrLen * sizeof(WCHAR));
    if (!pReconv)
    {
        return dwSize;
    }

    if (pReconv->dwSize < dwSize)
    {
        return 0;
    }

    pReconv->dwStrLen = dwStrLen;
    pReconv->dwStrOffset = sizeof(RECONVERTSTRING);
    memcpy((BYTE *)pReconv + sizeof(RECONVERTSTRING), (void *)(GetTextBuffer() + dwStrStart), dwStrLen * sizeof(WCHAR));

    pReconv->dwCompStrLen = nCompEnd - nCompStart;
    pReconv->dwCompStrOffset = (nCompStart - dwStrStart) * sizeof(WCHAR);
    pReconv->dwTargetStrLen = nCompEnd - nCompStart;
    pReconv->dwTargetStrOffset = (nCompStart - dwStrStart) * sizeof(WCHAR);

    return dwSize;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

ULONG CTextEditor::OnReconvertString(RECONVERTSTRING *pReconv)
{
    DWORD dwSize;
    DWORD dwStrStart;
    DWORD dwStrLen;

    // If there is a composition string already, we don't allow reconversion.
    if (_nCompEnd - _nCompStart)
    {
        return 0;
    }

    // Return the current selection to be the composition string
    // and give 40 surrounded charcters (20 prev and 20 after) for adjusting the composition string.
    if (_nSelStart > 20)
    {
        dwStrStart = _nSelStart - 20;
    }
    else
    {
        dwStrStart = 0;
    }

    if (_nSelEnd + 20 > GetTextLength())
    {
        dwStrLen = GetTextLength() - dwStrStart;
    }
    else
    {
        dwStrLen = _nSelEnd + 20 - dwStrStart;
    }

    dwSize = sizeof(RECONVERTSTRING) + (dwStrLen * sizeof(WCHAR));
    if (!pReconv)
    {
        return dwSize;
    }

    if (pReconv->dwSize < dwSize)
    {
        return 0;
    }

    pReconv->dwStrLen = dwStrLen;
    pReconv->dwStrOffset = sizeof(RECONVERTSTRING);
    memcpy((BYTE *)pReconv + sizeof(RECONVERTSTRING), (void *)(GetTextBuffer() + dwStrStart), dwStrLen * sizeof(WCHAR));

    pReconv->dwCompStrLen = _nSelEnd - _nSelStart;
    pReconv->dwCompStrOffset = (_nSelStart - dwStrStart) * sizeof(WCHAR);
    pReconv->dwTargetStrLen = _nSelEnd - _nSelStart;
    pReconv->dwTargetStrOffset = (_nSelStart - dwStrStart) * sizeof(WCHAR);

    return dwSize;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

ULONG CTextEditor::OnConfirmReconvertString(RECONVERTSTRING *pReconv)
{
    DWORD dwStrStart;
    // If there is a composition string already, we don't allow reconversion.
    if (_nCompEnd - _nCompStart)
    {
        return 0;
    }

    // The given reconvert string starts at _nSelStart - 20 if _nSelStart is bigenough.
    if (_nSelStart > 20)
    {
        dwStrStart = _nSelStart - 20;
    }
    else
    {
        dwStrStart = 0;
    }

    // Adjust selection to be converted.
    _nSelStart = dwStrStart + pReconv->dwCompStrOffset / sizeof(WCHAR);
    _nSelEnd = _nSelStart + pReconv->dwCompStrLen;

    return 1;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

WPARAM CTextEditor::PointToMouseWPARAM(POINT pt)
{
    BOOL bRet = FALSE;
    UINT nSel = _layout.ExactCharPosFromPoint(pt);
    if (nSel == (UINT)-1)
    {
        return 0;
    }

    if ((nSel < _nCompStart) || (nSel >= _nCompEnd))
    {
        return 0;
    }

    RECT rc;
    if (!_layout.RectFromCharPos(nSel, &rc))
    {
        return 0;
    }
    
    int nPos = (pt.x - rc.left) * 4 / (rc.right - rc.left) + 2;
    return (WPARAM)(((nSel - _nCompStart + (nPos / 4)) << 16) + ((nPos % 4) << 8));
}

