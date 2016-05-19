#pragma once
#include "TextEditor.h"

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextInputCtrl
{
public:
    CTextInputCtrl() 
    {
        _hwnd = NULL;
        HFONT hfontTemp = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        GetObject(hfontTemp, sizeof(LOGFONT), &_lfCurrentFont);
    }

    static ATOM RegisterClass(HINSTANCE hInstance);
    HWND Create(HWND hwndParent);

    static void SetThis(HWND hWnd, LPVOID lp)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lp);
    }
    static CTextInputCtrl *GetThis(HWND hWnd)
    {
        CTextInputCtrl *p = (CTextInputCtrl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        return p;
    }

    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void OnDestroy();
    void OnSetFocus(WPARAM wParam, LPARAM lParam);
    void OnPaint(HDC hdc);
    void OnKeyDown(WPARAM wParam, LPARAM lParam);
    void OnLButtonDown(WPARAM wParam, LPARAM lParam);
    void OnLButtonUp(WPARAM wParam, LPARAM lParam);
    void OnMouseMove(WPARAM wParam, LPARAM lParam);

    void Move(int x, int y, int nWidth, int nHeight)
    {
        if (IsWindow(_hwnd))
            MoveWindow(_hwnd, x, y, nWidth, nHeight, TRUE);
    }

    HWND GetWnd() {return _hwnd;}

    void SetFont(HWND hwndParent);

private:
    HWND _hwnd;

    CTextEditor _editor;
    LOGFONT _lfCurrentFont;
    UINT _uSelDragStart;
};


