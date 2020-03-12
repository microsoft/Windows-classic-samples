#include "private.h"
#include "commdlg.h"
//
// MSIME.H can be found in http://msdn2.microsoft.com/en-us/library/ms970233.aspx
//
#include "msime.h"
#include "TextInputCtrl.h"

extern HINSTANCE g_hInst;
extern UINT WM_MSIME_MOUSE;

#define TEXTINPUTCTRL_CLASSNAME TEXT("TextInputCtrlClassName")
#define TIMERID_CARET 0x100


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

ATOM CTextInputCtrl::RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)s_WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXTINPUTCTRL_CLASSNAME;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

HWND CTextInputCtrl::Create(HWND hwndParent)
{
    _hwnd = CreateWindow(TEXTINPUTCTRL_CLASSNAME, 
                        NULL, 
                        WS_CHILD,
                        0, 0, 0, 0, 
                        hwndParent, 
                        NULL, 
                        g_hInst, 
                        this);


    return _hwnd;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CALLBACK CTextInputCtrl::s_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HIMC himc;
    CTextInputCtrl *ptic;

    switch (message)
    {
        case WM_CREATE:
            SetThis(hwnd, ((CREATESTRUCT *)lParam)->lpCreateParams);
            SetTimer(hwnd, TIMERID_CARET, GetCaretBlinkTime(), NULL);
            GetThis(hwnd)->OnCreate(hwnd, wParam, lParam);
            break;


        case WM_PAINT:
            ptic = GetThis(hwnd);
            hdc = BeginPaint(hwnd, &ps);
            if (ptic)
                ptic->OnPaint(hdc);
            EndPaint(hwnd, &ps);
            break;

        case WM_KEYDOWN:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnKeyDown(wParam, lParam);
            break;

        case WM_LBUTTONDOWN:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnLButtonDown(wParam, lParam);
            break;

        case WM_LBUTTONUP:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnLButtonUp(wParam, lParam);
            break;

        case WM_RBUTTONDOWN:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnRButtonDown(wParam, lParam);
            break;

        case WM_RBUTTONUP:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnRButtonUp(wParam, lParam);
            break;

        case WM_MOUSEMOVE:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnMouseMove(wParam, lParam);
            break;

        case WM_SETFOCUS:
            //
            // Level 2 Support
            //
            ptic = GetThis(hwnd);
            if (ptic)
            {
                ptic->_editor.SetCompositionForm();
                ptic->SetCompositionFont();
            }
            break;

        case WM_IME_STARTCOMPOSITION:
            //
            // Level 2 Support
            //
            ptic = GetThis(hwnd);
            if (ptic)
            {
                ptic->_editor.OnStartComposition();
                ptic->_editor.SetCompositionForm();
                ptic->SetCompositionFont();
            }

            // If it is a near caret IME, we should keep Level 2 path.
            if (IsNearCaretIME())
            {
                return DefWindowProc(hwnd, message, wParam, lParam);
            }

            break;

        case WM_IME_COMPOSITION:
            himc = ImmGetContext(hwnd);

            if (himc)
            {
                BOOL fNearCaret = IsNearCaretIME();

                if (lParam & GCS_RESULTSTR)
                {
                    LONG nSize = ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0);
                    if (nSize)
                    {
                        LPWSTR psz = (LPWSTR)LocalAlloc(LPTR, nSize + sizeof(WCHAR));
                        if (psz)
                        {
                            ImmGetCompositionString(himc, GCS_RESULTSTR, psz, nSize);
                            ptic = GetThis(hwnd);
                            if (ptic)
                            {
                                ptic->_editor.InsertResultAtComposition(psz);
                                ptic->_editor.UpdateLayout(&ptic->_lfCurrentFont);
                                ptic->_editor.SetCompositionForm();
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                            LocalFree(psz);
                        }
                    }
                }

                if ((lParam & GCS_COMPSTR) && !fNearCaret)
                {
                    LONG lDeltaStart = ImmGetCompositionString(himc, GCS_DELTASTART, NULL, 0);
                    LONG lCursorPos = ImmGetCompositionString(himc, GCS_CURSORPOS, NULL, 0);
                    LONG lSize = ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0);
                    LONG lAttrSize = ImmGetCompositionString(himc, GCS_COMPATTR, NULL, 0);
                    LONG lClauseInfoSize = ImmGetCompositionString(himc, GCS_COMPCLAUSE, NULL, 0);

                    if (lSize > 0)
                    {
                        BYTE *prgAttr = NULL;
                        LONG *prgClauseInfo = NULL;
                        LPWSTR psz = (LPWSTR)LocalAlloc(LPTR, lSize + sizeof(WCHAR));

                        if (lAttrSize)
                            prgAttr = (BYTE *)LocalAlloc(LPTR, lAttrSize + sizeof(BYTE));

                        if (lClauseInfoSize)
                            prgClauseInfo = (LONG *)LocalAlloc(LPTR, lClauseInfoSize + sizeof(LONG));

                        if (psz)
                        {
                            if (ImmGetCompositionString(himc, GCS_COMPSTR, psz, lSize) > 0)
                            {
                                if (prgAttr)
                                {
                                    if (ImmGetCompositionString(himc, GCS_COMPATTR, prgAttr, lAttrSize) <= 0)
                                    {
                                        LocalFree(prgAttr);
                                        prgAttr = NULL;
                                        lAttrSize = 0;
                                    }
                                }
    
                                if (prgClauseInfo)
                                {
                                    if (ImmGetCompositionString(himc, GCS_COMPCLAUSE, prgClauseInfo, lClauseInfoSize) <= 0)
                                    {
                                        LocalFree(prgClauseInfo);
                                        prgClauseInfo = NULL;
                                        lClauseInfoSize = 0;
                                    }
                                }
    
                                ptic = GetThis(hwnd);
                                if (ptic)
                                {
                                    ptic->_editor.UpdateComposition(psz, lDeltaStart, lCursorPos,
                                                                    prgAttr, lAttrSize, prgClauseInfo, lClauseInfoSize);
                                    ptic->_editor.UpdateLayout(&ptic->_lfCurrentFont);
                                    ptic->_editor.SetCompositionForm();
                                    InvalidateRect(hwnd, NULL, TRUE);
                                }
                            }
                            LocalFree(psz);
                        }
                        if (prgAttr)
                            LocalFree(prgAttr);
                        if (prgClauseInfo)
                            LocalFree(prgClauseInfo);
                    }
                }
                ImmReleaseContext(hwnd, himc);

                // If it is a near caret IME, we should keep Level 2 path.
                if ((lParam & GCS_COMPSTR) && fNearCaret)
                {
                    lParam &= ~(GCS_RESULTCLAUSE |
                                GCS_RESULTREADCLAUSE |
                                GCS_RESULTREADSTR |
                                GCS_RESULTSTR);

                    return DefWindowProc(hwnd, message, wParam, lParam);
                }

                
            }
            break;

        case WM_IME_ENDCOMPOSITION:

            // The composition ends. 
            ptic = GetThis(hwnd);
            if (ptic)
            {
                // We need to clear the attributes.
                // There is a case that the composition string is canceled and it did not 
                // completed (GCS_RESULTSTR did not come).
                ptic->_editor.ClearAttrAndClauseInfo();
                ptic->_editor.UpdateLayout(&ptic->_lfCurrentFont);
                InvalidateRect(hwnd, NULL, TRUE);
            }

            // If it is a near caret IME, we should keep Level 2 path.
            if (IsNearCaretIME())
            {
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;

        case WM_IME_NOTIFY:
            if (wParam == IMN_OPENCANDIDATE)
            {
                ptic = GetThis(hwnd);
                if (ptic)
                {
                    ptic->_editor.SetCandidateForm();
                }
            }
            return DefWindowProc(hwnd, message, wParam, lParam);

        case WM_IME_SETCONTEXT:
            return DefWindowProc(hwnd, message, wParam, lParam & ~ISC_SHOWUICOMPOSITIONWINDOW);

        case WM_IME_REQUEST:
            switch (wParam)
            {
                case IMR_QUERYCHARPOSITION:
                {
                    ptic = GetThis(hwnd);
                    if (ptic)
                    {
                        ptic->_editor.QueryCharPosition((IMECHARPOSITION *)lParam);
                        return 1;
                    }
                }

                case IMR_DOCUMENTFEED:
                {
                    ptic = GetThis(hwnd);
                    if (ptic)
                    {
                        return ptic->_editor.OnDocumentFeed((RECONVERTSTRING *)lParam);
                    }
                }

                case IMR_RECONVERTSTRING:
                {
                    ptic = GetThis(hwnd);
                    if (ptic)
                    {
                        return ptic->_editor.OnReconvertString((RECONVERTSTRING *)lParam);
                    }
                }

                case IMR_CONFIRMRECONVERTSTRING:
                {
                    ptic = GetThis(hwnd);
                    if (ptic)
                    {
                        return ptic->_editor.OnConfirmReconvertString((RECONVERTSTRING *)lParam);
                    }
                }
            }
            break;

        case WM_IME_CHAR:
            //
            // wParam is a character of the result string. 
            // if we don't want to receive WM_CHAR message for this character,
            // we should not call DefWindowProc().
            //
            return DefWindowProc(hwnd, message, wParam, lParam);

        case WM_CHAR:
            //
            // wParam is a character of the result string. 
            //
            
            switch ((WCHAR)wParam)
            {
                case 0x08:
                case 0x0a:
                    return 0;
                default:
                    break;
            }

            ptic = GetThis(hwnd);
            if (ptic)
            {
                WCHAR wc[2];
                wc[0] = (WCHAR)wParam;
                wc[1] = L'\0';
                ptic->_editor.InsertAtSelection(wc);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_TIMER:
            if (wParam == TIMERID_CARET)
            {
                ptic = GetThis(hwnd);
                if (ptic)
                {
                    HDC hdc = GetDC(hwnd);
                    ptic->_editor.BlinkCaret(hdc);
                    ReleaseDC(hwnd, hdc);
                }
            }

            break;
    
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnPaint(HDC hdc)
{
    _editor.Render(hdc, &_lfCurrentFont);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    UINT nSelStart;
    UINT nSelEnd;
    switch (0xff & wParam)
    {
        case VK_LEFT:
             if (GetKeyState(VK_SHIFT) & 0x80)
             {
                 nSelStart = _editor.GetSelectionStart();
                 nSelEnd = _editor.GetSelectionEnd();
                 if (nSelStart > 0)
                 {
                     _editor.MoveSelection(nSelStart - 1, nSelEnd);
                 }
             }
             else
             {
                 _editor.MoveSelectionPrev();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_RIGHT:
             if (GetKeyState(VK_SHIFT) & 0x80)
             {
                 nSelStart = _editor.GetSelectionStart();
                 nSelEnd = _editor.GetSelectionEnd();
                 _editor.MoveSelection(nSelStart, nSelEnd + 1);
             }
             else
             {
                 _editor.MoveSelectionNext();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_UP:
             _editor.MoveSelectionUpDown(TRUE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_DOWN:
             _editor.MoveSelectionUpDown(FALSE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_HOME:
             _editor.MoveSelectionToLineFirstEnd(TRUE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_END:
             _editor.MoveSelectionToLineFirstEnd(FALSE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_DELETE:
             nSelStart = _editor.GetSelectionStart();
             nSelEnd = _editor.GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 _editor.DeleteAtSelection(FALSE);
             }
             else
             {
                 _editor.RemoveText(nSelStart, nSelEnd - nSelStart);
                 _editor.MoveSelection(nSelStart, nSelStart);
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_BACK:
             nSelStart = _editor.GetSelectionStart();
             nSelEnd = _editor.GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 _editor.DeleteAtSelection(TRUE);
                 _editor.MoveSelectionPrev();
             }
             else
             {
                 _editor.RemoveText(nSelStart, nSelEnd - nSelStart);
                 _editor.MoveSelection(nSelStart, nSelStart);
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    _editor.SetWnd(hwnd);
    _editor.SetCompositionForm();

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (SendMSIMEMOUSE(lParam))
    {
        return;
    }

    POINT pt;
    _uSelDragStart = (UINT)-1;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (_editor.MoveSelectionAtPoint(pt))
    {
        InvalidateRect(_hwnd, NULL, TRUE);
        _uSelDragStart = _editor.GetSelectionStart();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (SendMSIMEMOUSE(lParam))
    {
        return;
    }

    UINT nSelStart = _editor.GetSelectionStart();
    UINT nSelEnd = _editor.GetSelectionEnd();
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (_editor.MoveSelectionAtPoint(pt))
    {
        UINT nNewSelStart = _editor.GetSelectionStart();
        UINT nNewSelEnd = _editor.GetSelectionEnd();
        _editor.MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd));
        InvalidateRect(_hwnd, NULL, TRUE);
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (SendMSIMEMOUSE(lParam))
    {
        return;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (SendMSIMEMOUSE(lParam))
    {
        return;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (SendMSIMEMOUSE(lParam))
    {
        return;
    }

    if (wParam & MK_LBUTTON)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (_editor.MoveSelectionAtPoint(pt))
        {
            UINT nNewSelStart = _editor.GetSelectionStart();
            UINT nNewSelEnd = _editor.GetSelectionEnd();
            _editor.MoveSelection(min(_uSelDragStart, nNewSelStart), max(_uSelDragStart, nNewSelEnd));
            InvalidateRect(_hwnd, NULL, TRUE);
        }
    }
}
 
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::SetFont(HWND hwndParent)
{
    CHOOSEFONT  cf;
    LOGFONT     lf = _lfCurrentFont;

    cf.lStructSize    = sizeof(CHOOSEFONT);
    cf.hwndOwner      = hwndParent;
    cf.lpLogFont      = &lf;
    cf.Flags          = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
    cf.rgbColors      = RGB(0, 0, 0);
    cf.lCustData      = 0;
    cf.lpfnHook       = NULL;
    cf.lpTemplateName = NULL;
    cf.hInstance      = NULL;
    cf.lpszStyle      = NULL;
    cf.nFontType      = SCREEN_FONTTYPE;
    cf.nSizeMin       = 0;
    cf.nSizeMax       = 0;

    if (ChooseFont(&cf))
    {
        _lfCurrentFont = lf;

        // Level 2 Support
        SetCompositionFont();

        InvalidateRect(_hwnd, NULL, TRUE);
    }

}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::SetCompositionFont()
{
    HIMC himc = ImmGetContext(_hwnd);
    if (himc)
    {
        ImmSetCompositionFont(himc, &_lfCurrentFont);
    }
    ImmReleaseContext(_hwnd, himc);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextInputCtrl::SendMSIMEMOUSE(LPARAM lParam)
{
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    HWND hwndIme;
    WPARAM wParamPos = _editor.PointToMouseWPARAM(pt);
    if (wParamPos && (hwndIme = ImmGetDefaultIMEWnd(_hwnd)))
    {
        HIMC himc = ImmGetContext(_hwnd);

        if (GetKeyState(VK_LBUTTON) & 0x80)
           wParamPos |= IMEMOUSE_LDOWN;
        if (GetKeyState(VK_RBUTTON) & 0x80)
           wParamPos |= IMEMOUSE_RDOWN;

        SendMessage(hwndIme, WM_MSIME_MOUSE, wParamPos, (LPARAM)himc);
        ImmReleaseContext(_hwnd, himc);
        return TRUE;
    }
    return FALSE;
}
