
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
        _layout.Render(hdc, GetTextBuffer(), GetTextLength(), _nSelStart, _nSelEnd);

        SelectObject(hdc, hFontOrg);
        DeleteObject(hFont);
    }
}
