
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
        _nCompStart = 0;
        _nCompEnd = 0;

        _prgAttr = NULL;
        _prgClauseInfo = NULL;
        _lAttr = 0;
        _lClauseInfo = 0;
    }

    ~CTextEditor() 
    {
        if (_prgAttr)
           LocalFree(_prgAttr);

        if (_prgClauseInfo)
           LocalFree(_prgClauseInfo);
    }
    
    void MoveSelection(UINT nSelStart, UINT nSelEnd);
    BOOL MoveSelectionAtPoint(POINT pt);
    BOOL InsertAtSelection(LPCWSTR psz);
    BOOL DeleteAtSelection(BOOL fBack);

    BOOL InsertResultAtComposition(LPCWSTR psz);
    BOOL UpdateComposition(LPCWSTR psz, LONG lDeltaStart, LONG lCursorPos,
                           const BYTE *prgAttr, LONG lAttr, const LONG *prgClauseInfo, LONG lClauseInfo);
    void OnStartComposition()
    {
        _nCompStart = _nSelStart;
        _nCompEnd = _nSelEnd;
    }

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
    void SetCandidateForm();

    void ClearAttrAndClauseInfo();
    void FlushCompositionString();

    void QueryCharPosition(IMECHARPOSITION *pcpos);
    ULONG OnDocumentFeed(RECONVERTSTRING *pReconv);
    ULONG OnReconvertString(RECONVERTSTRING *pReconv);
    ULONG OnConfirmReconvertString(RECONVERTSTRING *pReconv);

    WPARAM PointToMouseWPARAM(POINT pt);

private:
    UINT _nSelStart;
    UINT _nSelEnd;
    HWND _hwnd;

    UINT _nCompStart;
    UINT _nCompEnd;
    BYTE *_prgAttr;
    LONG *_prgClauseInfo;
    LONG _lAttr;
    LONG _lClauseInfo;

    CTextLayout _layout;
};

