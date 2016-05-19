
#include "private.h"
#include "TextEditor.h"

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

    SetCompositionForm();
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
    if (!RemoveText(_nSelStart, _nSelEnd - _nSelStart))
        return FALSE;

    if (!InsertText(_nSelStart, psz, lstrlen(psz)))
        return FALSE;

    _nSelStart += lstrlen(psz);
    _nSelEnd = _nSelStart;
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
    }

    if (fBack && (_nSelStart > 0))
    {
        if (!RemoveText(_nSelStart - 1, 1))
            return FALSE;
    }

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
        _layout.Render(hdc, GetTextBuffer(), GetTextLength(), _nSelStart, _nSelEnd,
                       _nCompStart, _nCompEnd,
                       _prgAttr, _lAttr, _prgClauseInfo, _lClauseInfo);

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

    ClearAttrAndClauseInfo();
 
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::ClearAttrAndClauseInfo()
{
    _nCompStart = _nSelStart;
    _nCompEnd = _nSelEnd;

    if (_prgAttr)
    {
        LocalFree(_prgAttr);
        _prgAttr = NULL;
    }

    if (_prgClauseInfo)
    {
        LocalFree(_prgClauseInfo);
        _prgClauseInfo = NULL;
    }
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

    if (_prgAttr)
    {
        LocalFree(_prgAttr);
        _lAttr = 0;
        _prgAttr = NULL;
    }

    if (_prgClauseInfo)
    {
        LocalFree(_prgClauseInfo);
        _prgClauseInfo = NULL;
        _lClauseInfo = 0;
    }

    _prgAttr = (BYTE *)LocalAlloc(LPTR, lAttr);
    if (_prgAttr)
    {
        memcpy(_prgAttr, prgAttr, lAttr);
        _lAttr = lAttr / sizeof(BYTE);
    }

    _prgClauseInfo = (LONG *)LocalAlloc(LPTR, lClauseInfo);
    if (_prgClauseInfo)
    {
        memcpy(_prgClauseInfo, prgClauseInfo, lClauseInfo);
        _lClauseInfo = lClauseInfo / sizeof(LONG);
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
