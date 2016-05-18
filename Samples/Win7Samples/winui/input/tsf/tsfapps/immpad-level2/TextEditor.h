
#pragma once

#include "TextLayout.h"
#include "TextContainer.h"

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextEditor : public CTextContainer
{
public:
    CTextEditor() 
    {
        _nSelStart = 0;
        _nSelEnd = 0;
    }

    ~CTextEditor() 
    {
    }
    
    void MoveSelection(UINT nSelStart, UINT nSelEnd);
    BOOL MoveSelectionAtPoint(POINT pt);
    BOOL InsertAtSelection(LPCWSTR psz);
    BOOL DeleteAtSelection(BOOL fBack);

    void MoveSelectionNext();
    void MoveSelectionPrev();
    BOOL MoveSelectionUpDown(BOOL bUp);
    BOOL MoveSelectionToLineFirstEnd(BOOL bFirst);

    void Render(HDC hdc, const LOGFONT *plf);
    void UpdateLayout(const LOGFONT *plf);

    UINT GetSelectionStart() {return _nSelStart;}
    UINT GetSelectionEnd() {return _nSelEnd;}
    void BlinkCaret(HDC hdc)
    {
         _layout.BlinkCaret(hdc);
    }

    void SetWnd(HWND hwnd) {_hwnd = hwnd;}

    // Level 2 support
    void SetCompositionForm();

private:
    UINT _nSelStart;
    UINT _nSelEnd;
    HWND _hwnd;

    CTextLayout _layout;
};

