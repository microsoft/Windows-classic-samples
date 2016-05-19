//
// snoop.h
//
// CSnoopWnd declaration.
//

#ifndef SNOOP_H
#define SNOOP_H

#define MAX_SNOOP_TEXT  32

class CCaseTextService;
class CUpdateTextEditSession;

class CSnoopWnd
{
public:
    CSnoopWnd(CCaseTextService *pCase);

    static BOOL _InitClass();
    static void _UninitClass();

    BOOL _Init();
    void _Uninit();

    void _Show();
    void _Hide();

    void _UpdateText(ITfRange *pRange);
    void _UpdateText(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange);

private:

    friend CUpdateTextEditSession;

    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnPaint(HWND hWnd, HDC hdc);

    static void _SetThis(HWND hWnd, LPARAM lParam)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 
                      (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    }

    static CSnoopWnd *_GetThis(HWND hWnd)
    {
        return (CSnoopWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    CCaseTextService *_pCase;
    HWND _hWnd;
    ULONG _cchText;
    WCHAR _achText[MAX_SNOOP_TEXT];
    static ATOM _atomWndClass;
};

#endif // SNOOP_H
