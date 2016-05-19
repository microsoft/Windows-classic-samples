

#include "private.h"
#include "TextLayout.h"

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::Layout(HDC hdc, const WCHAR *psz,  UINT nCnt)
{
    Clear();

    // Count Line
    UINT i = 0;
    BOOL bNewLine = TRUE;
    _nLineCnt = 0;
    for (i = 0; i < nCnt; i++)
    {
        switch (psz[i])
        {
            case 0x0d:
            case 0x0a:
                bNewLine = TRUE;
                break;
            default:
                if (bNewLine)
                    _nLineCnt++;
                bNewLine = FALSE;
                break;
        }
    }

    _prgLines = (LINEINFO *)LocalAlloc(LPTR, _nLineCnt * sizeof(LINEINFO));
    if (!_prgLines)
        return FALSE;

    // Count character of each line.
    bNewLine = TRUE;
    int nCurrentLine = -1;
    for (i = 0; i < nCnt; i++)
    {
        switch (psz[i])
        {
            case 0x0d:
            case 0x0a:
                bNewLine = TRUE;
                break;
            default:
                if (bNewLine)
                {
                    nCurrentLine++;
                    _prgLines[nCurrentLine].nPos = i;
                    _prgLines[nCurrentLine].nCnt = 1;
                }
                else
                {
                    _prgLines[nCurrentLine].nCnt++;
                }
                bNewLine = FALSE;
                break;
        }
    }

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    _nLineHeight = tm.tmHeight + tm.tmExternalLeading;

    POINT ptCurrent;
    ptCurrent.x = 0;
    ptCurrent.y = 0;

    // Get the rectangle of each characters.
    for (i = 0; i < _nLineCnt; i++)
    {
        _prgLines[i].prgCharInfo = NULL;

        if (_prgLines[i].nCnt)
        {
            _prgLines[i].prgCharInfo = (CHARINFO *)LocalAlloc(LPTR, _prgLines[i].nCnt * sizeof(CHARINFO));
            if (!_prgLines[i].prgCharInfo)
                return FALSE;

            UINT j;
            POINT ptPrev = ptCurrent;
            for (j = 0; j < _prgLines[i].nCnt; j++)
            {
                SIZE size;
                GetTextExtentPoint32(hdc, psz+_prgLines[i].nPos, j + 1, &size);
                ptCurrent.x = size.cx;
                _prgLines[i].prgCharInfo[j].rc.left = ptPrev.x;
                _prgLines[i].prgCharInfo[j].rc.top = ptPrev.y;
                _prgLines[i].prgCharInfo[j].rc.right = ptCurrent.x;
                _prgLines[i].prgCharInfo[j].rc.bottom = ptPrev.y + _nLineHeight;

                ptPrev = ptCurrent;
            }
        }

        ptCurrent.x = 0;
        ptCurrent.y += _nLineHeight;
    }
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::Render(HDC hdc, const WCHAR *psz,  UINT nCnt, UINT nSelStart, UINT nSelEnd,
                         UINT nCompStart, UINT nCompEnd,
                         const BYTE *prgAttr, LONG lAttrSize, const LONG *prgClauseInfo, LONG lClauseInfo)
{
    POINT ptCurrent;
    ptCurrent.x = 0;
    ptCurrent.y = 0;

    // Render lines
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        if (_prgLines[i].nCnt)
        {
            TextOut(hdc, 
                    ptCurrent.x, 
                    ptCurrent.y, 
                    psz + _prgLines[i].nPos, 
                    _prgLines[i].nCnt);
        }
        ptCurrent.x = 0;
        ptCurrent.y += _nLineHeight;
    }
   
    _fCaret = FALSE;

    // Render selection/caret
    RECT rcSel;
    if (_nLineCnt)
    {
        for (UINT i = 0; i < _nLineCnt; i++)
        {
            // Rendering Selection
            if ((nSelEnd >= _prgLines[i].nPos) &&
                (nSelStart <= _prgLines[i].nPos + _prgLines[i].nCnt))
            {
                UINT nSelStartInLine = 0;
                UINT nSelEndInLine = _prgLines[i].nCnt;

                if (nSelStart > _prgLines[i].nPos)
                    nSelStartInLine = nSelStart - _prgLines[i].nPos;

                if (nSelEnd < _prgLines[i].nPos + _prgLines[i].nCnt)
                    nSelEndInLine = nSelEnd - _prgLines[i].nPos;

                if (nSelStartInLine != nSelEndInLine)
                {
                    for (UINT j = nSelStartInLine; j < nSelEndInLine; j++)
                    {
                        InvertRect(hdc, &_prgLines[i].prgCharInfo[j].rc);
                    }
                }
                else
                {
                    if (nSelStartInLine == _prgLines[i].nCnt)
                    {
                        rcSel = _prgLines[i].prgCharInfo[nSelStartInLine - 1].rc;
                        rcSel.left = rcSel.right;
                        rcSel.right++;
                    }
                    else
                    {
                        rcSel = _prgLines[i].prgCharInfo[nSelStartInLine].rc;
                        rcSel.right = rcSel.left + 1;
                    }
                    InvertRect(hdc, &rcSel);
                    _fCaret = TRUE;
                    _rcCaret = rcSel;
                }
            }

            // Rendering the composition display attribute.
            if ((nCompEnd >= _prgLines[i].nPos) &&
                (nCompStart <= _prgLines[i].nPos + _prgLines[i].nCnt))
            {
                UINT nCompStartInLine = 0;
                UINT nCompEndInLine = _prgLines[i].nCnt;
                int  nBaseLineWidth = (_nLineHeight / 18) + 1;

                if (nCompStart > _prgLines[i].nPos)
                    nCompStartInLine = nCompStart - _prgLines[i].nPos;

                if (nCompEnd < _prgLines[i].nPos + _prgLines[i].nCnt)
                    nCompEndInLine = nCompEnd - _prgLines[i].nPos;

                for (UINT j = nCompStartInLine; j < nCompEndInLine; j++)
                {
                    HPEN hpen = NULL;
                    UINT uCurrentCompPos = _prgLines[i].nPos + j - nCompStart;
                    BOOL bClause = FALSE;

                    for (LONG k = 0; k < lClauseInfo; k++)
                    {
                        if (uCurrentCompPos + 1 == (UINT)prgClauseInfo[k])
                        {
                            bClause = TRUE;
                        }
                    }

                    hpen = CreateUnderlinePen(prgAttr[uCurrentCompPos], nBaseLineWidth);
                    if (hpen)
                    {
                        HPEN hpenOrg;
                        hpenOrg = (HPEN)SelectObject(hdc, hpen);
                        RECT rc = _prgLines[i].prgCharInfo[j].rc;

                        POINT pts[2];
                        pts[0].x = rc.left;
                        pts[0].y = rc.bottom;
                        pts[1].x = rc.right - (bClause ? nBaseLineWidth : 0);
                        pts[1].y = rc.bottom;
                        Polyline(hdc, pts, 2);

                        SelectObject(hdc, hpenOrg);
                    }
                }
            }
        }
    }
    else
    {
        rcSel.left = 0;
        rcSel.top = 0;
        rcSel.right = 1;
        rcSel.bottom = _nLineHeight;
        InvertRect(hdc, &rcSel);
        _fCaret = TRUE;
        _rcCaret = rcSel;
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextLayout::BlinkCaret(HDC hdc)
{
    if (_fCaret)
        InvertRect(hdc, &_rcCaret);

    return;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::RectFromCharPos(UINT nPos, RECT *prc)
{
    memset(prc, 0, sizeof(*prc));
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        if (nPos < _prgLines[i].nPos)
           continue;

        if (nPos >= _prgLines[i].nPos + _prgLines[i].nCnt)
        {
           if (((nPos -_prgLines[i].nPos) > 0) && (nPos == _prgLines[i].nPos + _prgLines[i].nCnt))
           {
               *prc = _prgLines[i].prgCharInfo[nPos - _prgLines[i].nPos - 1].rc;
               prc->left = prc->right;
               return TRUE;
           }
           continue;
        }
        
        *prc = _prgLines[i].prgCharInfo[nPos - _prgLines[i].nPos].rc;
        return TRUE;
    }
    
    prc->top = _nLineCnt * _nLineHeight;
    prc->bottom = prc->top + _nLineHeight;
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::CharPosFromPoint(POINT pt)
{
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        for (UINT j = 0; j < _prgLines[i].nCnt; j++)
        {
            if (PtInRect(&_prgLines[i].prgCharInfo[j].rc, pt))
            {
                int nWidth = _prgLines[i].prgCharInfo[j].GetWidth();
                if (pt.x > _prgLines[i].prgCharInfo[j].rc.left + (nWidth * 3 / 4))
                {
                    return _prgLines[i].nPos + j + 1;
                }
                return _prgLines[i].nPos + j;
            }
        }
    }
    return (UINT)(-1);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        if ((_prgLines[i].nPos <= uCurPos) &&
            (_prgLines[i].nPos + _prgLines[i].nCnt >= uCurPos))
        {
            if (bFirst)
            {
                return _prgLines[i].nPos;
            }
            else
            {
                return _prgLines[i].nPos + _prgLines[i].nCnt;
            }
        }
    }
    return (UINT)(-1);
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextLayout::Clear()
{
    if (_prgLines)
    {
        for (UINT i = 0; i < _nLineCnt; i++)
        {
            if (_prgLines[i].prgCharInfo)
            {
                LocalFree(_prgLines[i].prgCharInfo);
            }
        }
        LocalFree(_prgLines);
    }
    _nLineCnt = 0;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

HPEN CTextLayout::CreateUnderlinePen(BYTE bAttr, int nWidth)
{
    const DWORD s_dwDotStyles[]  = {1,2};
    const DWORD s_dwDashStyles[] = {3,2};

    DWORD dwPenStyle = PS_GEOMETRIC | PS_SOLID;
    DWORD dwStyles = 0;
    const DWORD *lpdwStyles = NULL;
    switch (bAttr)
    {
        case ATTR_INPUT:
            dwPenStyle = PS_GEOMETRIC | PS_USERSTYLE | PS_ENDCAP_FLAT;
            dwStyles = 2;
            lpdwStyles = s_dwDotStyles;
            break;

        case ATTR_TARGET_CONVERTED:
        case ATTR_TARGET_NOTCONVERTED:
            dwPenStyle = PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT;
            nWidth *= 2;
            break;


        case ATTR_CONVERTED:
        case ATTR_INPUT_ERROR:
        case ATTR_FIXEDCONVERTED:
        default:
            dwPenStyle = PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT;
            break;
    }


    LOGBRUSH lbr;
    lbr.lbStyle = BS_SOLID;
    lbr.lbColor = 0;
    lbr.lbHatch = 0;
    lbr.lbColor = RGB(0, 0, 0);

    return ExtCreatePen(dwPenStyle, nWidth, &lbr, dwStyles, lpdwStyles);
}
