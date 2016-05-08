
#pragma once

#include "TextLayout.h"
#include "TextContainer.h"
#include "TextStore.h"
#include "TextEditSink.h"

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
         
        _pCompositionRenderInfo = NULL;
        _nCompositionRenderInfo = 0;

        _nCompStart = 0;
        _nCompEnd = 0;

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
    void MoveSelectionToComposition()
    {
         MoveSelection(_nCompStart, _nCompEnd);
    }

    void Render(HDC hdc, const LOGFONT *plf);
    void UpdateLayout(const LOGFONT *plf);

    UINT GetSelectionStart() {return _nSelStart;}
    UINT GetSelectionEnd() {return _nSelEnd;}
    void BlinkCaret(HDC hdc)
    {
         _layout.BlinkCaret(hdc);
    }

    void SetInterimCaret(BOOL fSet)
    {
        _layout.SetInterimCaret(fSet, _nSelStart);

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

    void ClearCompositionRenderInfo();
    BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);

    void TerminateCompositionString();

    void AleartMouseSink(POINT pt, DWORD dwBtnState, BOOL *pbEaten);

    // Level 2 support
    void SetCompositionForm();
    void SetCandidateForm();

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

    CTextLayout _layout;

    CTextStore *_pTextStore;
    ITfDocumentMgr *_pDocumentMgr;
    ITfContext *_pInputContext;
    TfEditCookie _ecTextStore;

    CTextEditSink *_pTextEditSink;

    COMPOSITIONRENDERINFO *_pCompositionRenderInfo;
    int _nCompositionRenderInfo;

};

