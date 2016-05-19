
#pragma once

#include "TextLayout.h"
#include "TextContainer.h"
#include "TextStore.h"

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
        _pTextStore = NULL;
        _pDocumentMgr = NULL;
         
    }

    ~CTextEditor() 
    {
        if (_pTextStore)
        {
            _pTextStore->Release();
            _pTextStore = NULL;
        }

        if (_pDocumentMgr)
        {
            _pDocumentMgr->Release();
            _pDocumentMgr = NULL;
        }
    }
    
    void MoveSelection(UINT nSelStart, UINT nSelEnd);
    BOOL MoveSelectionAtPoint(POINT pt);
    BOOL InsertAtSelection(LPCWSTR psz);
    BOOL DeleteAtSelection(BOOL fBack);
    BOOL DeleteSelection();

    void MoveSelectionNext();
    void MoveSelectionPrev();
    BOOL MoveSelectionUpDown(BOOL bUp);
    BOOL MoveSelectionToLineFirstEnd(BOOL bFirst);

    void Render(HDC hdc, const LOGFONT *plf);
    UINT GetSelectionStart() {return _nSelStart;}
    UINT GetSelectionEnd() {return _nSelEnd;}
    void BlinkCaret(HDC hdc)
    {
         _layout.BlinkCaret(hdc);
    }

    void SetWnd(HWND hwnd) {_hwnd = hwnd;}
    HWND GetWnd() {return _hwnd;}

    BOOL InitTSF();
    BOOL UninitTSF();
    void SetFocusDocumentMgr();

    void InvalidateRect()
    {
        ::InvalidateRect(_hwnd, NULL, TRUE);
    }

    int GetLineHeight() {return _layout.GetLineHeight();}
    CTextLayout *GetLayout() {return &_layout;}

private:
    UINT _nSelStart;
    UINT _nSelEnd;
    HWND _hwnd;

    CTextLayout _layout;

    CTextStore *_pTextStore;
    ITfDocumentMgr *_pDocumentMgr;
    ITfContext *_pInputContext;
    TfEditCookie _ecTextStore;
};

