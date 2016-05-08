#pragma once

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

typedef struct 
{
    RECT rc;
    int GetWidth() {return rc.right - rc.left;}
} CHARINFO;

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

typedef struct 
{
    UINT nPos;
    UINT nCnt;
    CHARINFO *prgCharInfo;
} LINEINFO;

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextLayout
{
public:
    CTextLayout()
    {
        _prgLines = NULL;
        _nLineCnt = 0;
        _fCaret = FALSE;
        _fInterimCaret = FALSE;
    }

    ~CTextLayout()
    {
        Clear();
    }

    BOOL Layout(HDC hdc, const WCHAR *psz,  UINT nCnt);
    BOOL Render(HDC hdc, const WCHAR *psz,  UINT nCnt, UINT nSelStart, UINT nSelEnd,
                UINT nCompStart, UINT nCompEnd,
                const BYTE *prgAttr, LONG lAttrSize, const LONG *prgClauseInfo, LONG lClauseInfo);
    BOOL RectFromCharPos(UINT nPos, RECT *prc);
    UINT CharPosFromPoint(POINT pt);
    UINT ExactCharPosFromPoint(POINT pt);
    UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);
    void BlinkCaret(HDC hdc);
    void SetInterimCaret(BOOL fSet, UINT uPos);
    int  GetLineHeight() {return _nLineHeight;}
    BOOL IsInterimCaret() {return _fInterimCaret;}

private:
    void Clear();
    HPEN CreateUnderlinePen(BYTE bAttr, int nWidth);

    int _nLineHeight;

    LINEINFO *_prgLines;
    UINT _nLineCnt;

    BOOL _fCaret;
    BOOL _fInterimCaret;
    RECT _rcCaret;

};
