// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "RenderTest.h"
#include "IRenderer.h"
#include "TextHelpers.h"
#include "resource.h"

// Uncomment the following definition to force the DPI to a hard-coded value for testing
// purposes. Otherwise the system default DPI is used.
//#define FORCE_DPI 120

// Global Variables:
HINSTANCE g_instance;
int g_dpiX;
int g_dpiY;
float const g_formatWidth = 6.0f * 96;

IDWriteFactory* g_dwriteFactory = NULL;
ID2D1Factory*   g_d2dFactory    = NULL;

namespace
{
    // Global variables used only in this module.
    DWRITE_MEASURING_MODE g_measuringMode = DWRITE_MEASURING_MODE_NATURAL;
    IDWriteTextFormat* g_textFormat = NULL;

    std::wstring g_text;
    const static wchar_t g_defaultText[] =
        L"ClearType is a software technology developed by Microsoft that improves the "
        L"readability of text on existing LCDs (Liquid Crystal Displays), such as laptop "
        L"screens, Pocket PC screens and flat panel monitors. With ClearType font technology, "
        L"the words on your computer screen look almost as sharp and clear as those printed "
        L"on a piece of paper.";

    // Current monitor.
    HMONITOR g_monitor;

    // Current font.
    inline float PointsToDips(float points)
    {
        return points * (96.0f / 72);
    }
    wchar_t const g_defaultFamilyName[] = L"Times New Roman";
    float const g_minFontSize = PointsToDips(4);
    float g_fontSize = PointsToDips(12);
    LOGFONT g_logFont;

    // Current angle of rotation.
    int g_degrees = 0;

    // Current renderer.
    enum RendererID
    {
        RendererDWrite,
        RendererD2D
    };
    RendererID g_rendererID = RendererDWrite;
    IRenderer* g_renderer = NULL;

    // Current magnifier state.
    MagnifierInfo g_magnifier =
    {
        true,                   // visible
        MagnifierInfo::Pixel,   // type
        3,                      // scale
        0
    };

    bool g_dragging;
    POINT g_dragPos;

    // The following variables are used for the translate animation that occurs
    // on the Nudge Text Left and Nudge Text Right commands.
    DWORD g_animationStartCount;
    float g_animationDuration;
    float g_animationStartX;
    float g_animationEndX;
    float g_animationCurrentX;
}

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitializeMenuItems(HMENU popupMenu);
bool OnCommand(HWND hwnd, WORD commandID);
void UpdateAnimation(HWND hwnd);
bool IsAnimating();
HRESULT CreateRenderer(HWND hwnd);
void OnPaint(HWND hwnd);
void OnSize(HWND hwnd);
void OnMouseDown(HWND hwnd);
void OnMouseMove(HWND hwnd);
void OnMouseUp(HWND hwnd);
HRESULT OnChooseFont(HWND hwnd);
HRESULT IncreaseFontSize(HWND hwnd);
HRESULT DecreaseFontSize(HWND hwnd);
HRESULT SetFontSize(HWND hwnd, float newFontSize);
void SetMagnifierType(HWND hwnd, MagnifierInfo::Type newType);
void SetMagnifierScale(HWND hwnd, int scale);
void SetMeasuringMode(HWND hwnd, DWRITE_MEASURING_MODE newMode);
void SetTransform(HWND hwnd);
void SetRenderer(HWND hwnd, RendererID id);
HRESULT OnCopy(HWND hwnd);
HRESULT OnPaste(HWND hwnd);
void SetCaption(HWND hwnd);

int APIENTRY wWinMain(
    HINSTANCE   hInstance, 
    HINSTANCE   hPrevInstance,
    LPWSTR      commandLine,
    int         nCmdShow
    )
{
    // The Microsoft Security Development Lifecycle recommends that all
    // applications include the following call to ensure that heap corruptions
    // do not go unnoticed and therefore do not introduce opportunities
    // for security exploits.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = S_OK;

    g_instance = hInstance;

    // Get the DPI.
#ifdef FORCE_DPI
    g_dpiX = FORCE_DPI;
    g_dpiY = FORCE_DPI;
#else
    HDC hdc = GetDC(NULL);
    g_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    g_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);
#endif

    // We're going to make the client area 7 x 5 inches initially.
    int const clientWidth = 7 * g_dpiX;
    int const clientHeight = 5 * g_dpiY;

    // Initialize the magnifier size and center the focus rectangle in the client area.
    g_magnifier.magnifierSize.cx = g_dpiX * 3;
    g_magnifier.magnifierSize.cy = g_dpiY * 3 / 2;
    g_magnifier.focusPos.x = (clientWidth - (g_magnifier.magnifierSize.cx / g_magnifier.scale)) / 2;
    g_magnifier.focusPos.y = (clientHeight - (g_magnifier.magnifierSize.cy / g_magnifier.scale)) / 2;

    try
    {
        g_text.assign(g_defaultText);
    }
    catch (...)
    {
        hr = ExceptionToHResult(); // Do not propagate exceptions up.
    }

    // Create the factory objects.
    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED, 
                __uuidof(IDWriteFactory), 
                reinterpret_cast<IUnknown**>(&g_dwriteFactory)
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED, 
                __uuidof(ID2D1Factory), 
                NULL,
                (IID_PPV_ARGS(&g_d2dFactory))
                );
    }

    if (SUCCEEDED(hr))
    {
        // Initialize the LOGFONT and use it to construct the text format object.
        // We use LOGFONT in this application only because we rely on the Win32 
        // common font dialog.
        memset(&g_logFont, 0, sizeof(g_logFont));
        g_logFont.lfWeight = FW_NORMAL;
        g_logFont.lfCharSet = DEFAULT_CHARSET;
        g_logFont.lfOutPrecision = OUT_OUTLINE_PRECIS;
        memcpy(g_logFont.lfFaceName, g_defaultFamilyName, sizeof(g_defaultFamilyName));

        SafeRelease(&g_textFormat);
        hr = CreateTextFormatFromLOGFONT(g_logFont, g_fontSize, &g_textFormat);
    }

    ATOM classAtom = 0;
    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(wcex), 0 };
        wcex.style = CS_HREDRAW|CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DWRITERENDERTEST);
        wcex.lpszClassName = L"RenderTest";

        classAtom = RegisterClassEx(&wcex);
        if (classAtom == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    HACCEL accelerators = NULL;
    if (SUCCEEDED(hr))
    {
        // Load the accelerator table.
        accelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
        if (accelerators == NULL)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    HWND hwnd = NULL;
    if (SUCCEEDED(hr))
    {
        // Compute a window size that will give us our desired client size.
        RECT windowRect = { 0, 0, clientWidth, clientHeight };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, TRUE);

        // Create the window.
        hwnd = CreateWindow(
            MAKEINTATOM(classAtom), 
            L"", // caption (we'll set it later)
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 
            windowRect.right - windowRect.left, 
            windowRect.bottom - windowRect.top, 
            NULL, 
            NULL, 
            hInstance, 
            NULL
            );

        if (hwnd == NULL)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    MSG msg = {};
    if (SUCCEEDED(hr))
    {
        SetCaption(hwnd);
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);

        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(hwnd, accelerators, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    delete g_renderer; g_renderer = NULL;
    SafeRelease(&g_textFormat);
    SafeRelease(&g_d2dFactory);
    SafeRelease(&g_dwriteFactory);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (OnCommand(hwnd, LOWORD(wParam)))
            break;
        else
            return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_INITMENUPOPUP:
        InitializeMenuItems(reinterpret_cast<HMENU>(wParam));
        break;

    case WM_PAINT:
        OnPaint(hwnd);
        break;

    case WM_ERASEBKGND:
        return true;

    case WM_SIZE:
        OnSize(hwnd);
        break;

    case WM_WINDOWPOSCHANGED:
        {
            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
            if (monitor != g_monitor)
            {
                g_monitor = monitor;
                if (g_renderer != NULL)
                    g_renderer->SetMonitor(g_monitor);

                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_LBUTTONDOWN:
        OnMouseDown(hwnd);
        break;

    case WM_MOUSEMOVE:
        OnMouseMove(hwnd);
        break;

    case WM_LBUTTONUP:
        OnMouseUp(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

void InitializeMenuItems(HMENU popupMenu)
{
    int count = GetMenuItemCount(popupMenu);
    for (int i = 0; i < count; ++i)
    {
        MENUITEMINFO info;
        info.cbSize = sizeof(info);
        info.fMask = MIIM_ID | MIIM_STATE;

        if (!GetMenuItemInfo(popupMenu, i, TRUE, &info))
            break;

        bool check = false;
        bool disable = false;

        switch (info.wID)
        {
        case ID_EDIT_COPY:
            disable = g_text.empty();
            break;

        case ID_EDIT_PASTE:
            disable = !IsClipboardFormatAvailable(CF_TEXT);
            break;

        case ID_EDIT_DECREASETEXTSIZE:
            disable = (g_fontSize == g_minFontSize);
            break;

        case ID_VIEW_SHOWMAGNIFIER:
            check = g_magnifier.visible;
            break;

        case ID_MAGNIFIERTYPE_VECTOR:
            check = (g_magnifier.type == MagnifierInfo::Vector);
            disable = (g_measuringMode != DWRITE_MEASURING_MODE_NATURAL);
            break;

        case ID_MAGNIFIERTYPE_PIXELS:
            check = (g_magnifier.type == MagnifierInfo::Pixel);
            break;

        case ID_MAGNIFIERTYPE_SUBPIXELS:
            check = (g_magnifier.type == MagnifierInfo::Subpixel);
            disable = (g_rendererID == RendererD2D);
            break;

        case ID_MAGNIFIERSCALE_3X:
            check = (g_magnifier.scale == 3);
            break;

        case ID_MAGNIFIERSCALE_6X:
            check = (g_magnifier.scale == 6);
            break;

        case ID_OPTIONS_NATURALMODE:
            check = (g_measuringMode == DWRITE_MEASURING_MODE_NATURAL);
            break;

        case ID_OPTIONS_GDICLASSICMODE:
            check = (g_measuringMode == DWRITE_MEASURING_MODE_GDI_CLASSIC);
            break;

        case ID_OPTIONS_GDINATURALMODE:
            check = (g_measuringMode == DWRITE_MEASURING_MODE_GDI_NATURAL);
            break;

        case ID_OPTIONS_USEDIRECT2D:
            check = (g_rendererID == RendererD2D);
            break;

        case ID_OPTIONS_USEDIRECTWRITE:
            check = (g_rendererID == RendererDWrite);
            break;
        }

        UINT newState = 
            (check ? MFS_CHECKED : 0) | 
            (disable ? MFS_DISABLED : 0);

        if (newState != info.fState)
        {
            info.fMask = MIIM_STATE;
            info.fState = newState;
            SetMenuItemInfo(popupMenu, i, TRUE, &info);
        }
    }
}

bool OnCommand(HWND hwnd, WORD commandID)
{
    HRESULT hr = S_OK;

    switch (commandID)
    {
    case IDM_EXIT:
        DestroyWindow(hwnd);
        break;

    case ID_EDIT_COPY:
        OnCopy(hwnd);
        break;

    case ID_EDIT_PASTE:
        OnPaste(hwnd);
        break;

    case ID_EDIT_FONT:
        hr = OnChooseFont(hwnd);
        break;

    case ID_EDIT_INCREASETEXTSIZE:
        hr = IncreaseFontSize(hwnd);
        break;

    case ID_EDIT_DECREASETEXTSIZE:
        hr = DecreaseFontSize(hwnd);
        break;

    case ID_VIEW_ROTATELEFT:
        if ((g_degrees -= 3) < 0)
            g_degrees += 360;
        SetTransform(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case ID_VIEW_ROTATERIGHT:
        if ((g_degrees += 3) >= 360)
            g_degrees -= 360;
        SetTransform(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case ID_VIEW_NUDGELEFT:
        g_animationStartCount = GetTickCount();
        g_animationDuration   = 0.1f;
        g_animationStartX     = g_animationCurrentX;
        g_animationEndX       = g_animationCurrentX - 10.0f;
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case ID_VIEW_NUDGERIGHT:
        g_animationStartCount = GetTickCount();
        g_animationDuration   = 0.1f;
        g_animationStartX     = g_animationCurrentX;
        g_animationEndX       = g_animationCurrentX + 10.0f;
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case ID_VIEW_SHOWMAGNIFIER:
        g_magnifier.visible = !g_magnifier.visible;
        if (g_renderer != NULL)
        {
            g_renderer->SetMagnifier(g_magnifier);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case ID_MAGNIFIERTYPE_VECTOR:
        SetMagnifierType(hwnd, MagnifierInfo::Vector);
        break;

    case ID_MAGNIFIERTYPE_PIXELS:
        SetMagnifierType(hwnd, MagnifierInfo::Pixel);
        break;

    case ID_MAGNIFIERTYPE_SUBPIXELS:
        SetMagnifierType(hwnd, MagnifierInfo::Subpixel);
        break;

    case ID_MAGNIFIERSCALE_3X:
        SetMagnifierScale(hwnd, 3);
        break;

    case ID_MAGNIFIERSCALE_6X:
        SetMagnifierScale(hwnd, 6);
        break;

    case ID_OPTIONS_NATURALMODE:
        SetMeasuringMode(hwnd, DWRITE_MEASURING_MODE_NATURAL);
        break;

    case ID_OPTIONS_GDICLASSICMODE:
        SetMeasuringMode(hwnd, DWRITE_MEASURING_MODE_GDI_CLASSIC);
        break;

    case ID_OPTIONS_GDINATURALMODE:
        SetMeasuringMode(hwnd, DWRITE_MEASURING_MODE_GDI_NATURAL);
        break;

    case ID_OPTIONS_USEDIRECT2D:
        SetRenderer(hwnd, RendererD2D);
        break;

    case ID_OPTIONS_USEDIRECTWRITE:
        SetRenderer(hwnd, RendererDWrite);
        break;

    default:
        return false;
    }

    if (FAILED(hr))
        PostQuitMessage(hr);

    return true;
}

HRESULT IncreaseFontSize(HWND hwnd)
{
    float newFontSize = (g_fontSize < PointsToDips(24)) ?
        g_fontSize + PointsToDips(0.5f) :
        g_fontSize * 1.25f;

    return SetFontSize(hwnd, newFontSize);
}

HRESULT DecreaseFontSize(HWND hwnd)
{
    float newFontSize = (g_fontSize <= PointsToDips(24)) ?
        g_fontSize - PointsToDips(0.5f) :
        g_fontSize * (1/1.25f);

    return SetFontSize(hwnd, std::max(newFontSize, g_minFontSize));
}

HRESULT SetFontSize(HWND hwnd, float newFontSize)
{
    IDWriteTextFormat* newTextFormat = NULL;
    HRESULT hr = CreateTextFormatFromLOGFONT(g_logFont, newFontSize, &newTextFormat);
    if (FAILED(hr))
    {
        return (hr == DWRITE_E_NOFONT) ? S_OK : hr;
    }

    g_fontSize = newFontSize;
    SafeSet(&g_textFormat, newTextFormat);

    if (g_renderer != NULL)
    {
        g_renderer->SetFormat(g_textFormat);
    }

    InvalidateRect(hwnd, NULL, TRUE);

    SafeRelease(&newTextFormat);

    return S_OK;
}

HRESULT OnChooseFont(HWND hwnd)
{
    HRESULT hr = S_OK;

    LOGFONT logFont = g_logFont;

    CHOOSEFONT font  = { 0 };
    font.lStructSize = sizeof(font);
    font.hwndOwner   = hwnd;
    font.lpLogFont   = &logFont;
    font.iPointSize  = static_cast<int>(g_fontSize * (720 / 96.0f));

    // Don't show vertical fonts because we don't do vertical layout and don't show
    // bitmap fonts because DirectWrite doesn't support them.
    font.Flags = CF_SCREENFONTS | CF_SCALABLEONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT;

    // Show the common font dialog box.
    if (ChooseFont(&font))
    {
        // The lfFaceName might not be initialized if the user didn't select a face name.
        if (logFont.lfFaceName[0] == L'\0')
            memcpy(logFont.lfFaceName, g_logFont.lfFaceName, sizeof(logFont.lfFaceName));

        float newFontSize = font.iPointSize * (96.0f / 720);

        // Map the Win32 font properties to an IDWriteTextFormat.
        IDWriteTextFormat* newTextFormat = NULL;
        HRESULT hr = CreateTextFormatFromLOGFONT(logFont, newFontSize, &newTextFormat);

        if (SUCCEEDED(hr))
        {
            // Save the new font properties.
            g_logFont = logFont;
            g_fontSize = newFontSize;
            SafeAttach(&g_textFormat, SafeDetach(&newTextFormat));

            if (g_renderer != NULL)
            {
                g_renderer->SetFormat(g_textFormat);
            }

            InvalidateRect(hwnd, NULL, TRUE);
        }

        SafeRelease(&newTextFormat);
    }

    // Potentially expected error, but not fatal,
    // so just do nothing.
    if (hr == DWRITE_E_NOFONT)
        hr = S_OK;

    return hr;
}

void SetMagnifierType(HWND hwnd, MagnifierInfo::Type newType)
{
    if (newType != g_magnifier.type)
    {
        if (newType == MagnifierInfo::Vector && g_measuringMode != DWRITE_MEASURING_MODE_NATURAL)
            return;

        g_magnifier.type = newType;

        if (g_magnifier.visible && g_renderer != NULL)
        {
            g_renderer->SetMagnifier(g_magnifier);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
}

void SetMagnifierScale(HWND hwnd, int scale)
{
    if (scale != g_magnifier.scale)
    {
        g_magnifier.scale = scale;

        if (g_magnifier.visible && g_renderer != NULL)
        {
            g_renderer->SetMagnifier(g_magnifier);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
}

void SetMeasuringMode(HWND hwnd, DWRITE_MEASURING_MODE newMode)
{
    if (newMode != g_measuringMode)
    {
        if (g_renderer != NULL)
        {
            g_renderer->SetMeasuringMode(newMode);
        }

        g_measuringMode = newMode;

        if (newMode != DWRITE_MEASURING_MODE_NATURAL && g_magnifier.type == MagnifierInfo::Vector)
        {
            SetMagnifierType(hwnd, MagnifierInfo::Pixel);
        }

        InvalidateRect(hwnd, NULL, TRUE);
        SetCaption(hwnd);
    }
}

void SetRenderer(HWND hwnd, RendererID id)
{
    if (id != g_rendererID)
    {
        delete g_renderer; g_renderer = NULL;

        g_rendererID = id;

        if (id == RendererD2D && (g_magnifier.type == MagnifierInfo::Subpixel))
        {
            SetMagnifierType(hwnd, MagnifierInfo::Pixel);
        }

        InvalidateRect(hwnd, NULL, TRUE);
        SetCaption(hwnd);
    }
}

void SetTransform(HWND hwnd)
{
    if (g_renderer != NULL)
    {
        // Compute the center of the client area in DIPs.
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        float centerX = PixelsToDipsX(clientRect.right) * 0.5f;;
        float centerY = PixelsToDipsY(clientRect.bottom) * 0.5f;

        // Create a transform that rotates around the center point.
        DWRITE_MATRIX transform = MakeRotateTransform(float(g_degrees), centerX, centerY);

        // Add the current horizontal displacement.
        transform.dx += g_animationCurrentX;

        // Set the transform.
        g_renderer->SetTransform(transform);
    }
}

HRESULT OnCopy(HWND hwnd)
{
    HRESULT hr = E_FAIL;

    if (OpenClipboard(hwnd))
    {
        if (EmptyClipboard())
        {
            size_t const byteCount = (g_text.size() + 1) * sizeof(wchar_t);

            HGLOBAL handle = GlobalAlloc(GMEM_DDESHARE, byteCount);

            void* data = GlobalLock(handle);
            if (data != NULL)
            {
                memcpy(data, g_text.c_str(), byteCount);
                GlobalUnlock(handle);
            }

            if (SetClipboardData(CF_UNICODETEXT, handle) == NULL)
            {
                hr = S_OK;
                GlobalFree(handle);
            }
        }
        CloseClipboard();
    }

    return hr;
}

HRESULT OnPaste(HWND hwnd)
{
    HRESULT hr = E_FAIL;

    bool textChanged = false;

    if (OpenClipboard(hwnd))
    {
        HGLOBAL handle = GetClipboardData(CF_UNICODETEXT);

        if (handle != NULL)
        {
            size_t const maxLength = GlobalSize(handle) / sizeof(wchar_t);

            wchar_t const* begin = static_cast<wchar_t const*>(GlobalLock(handle));
            if (begin != NULL)
            {
                wchar_t const* end = std::find(begin, begin + maxLength, L'\0');

                try
                {
                    g_text.assign(begin, end);
                    textChanged = true;
                    hr = S_OK;
                }
                catch (...)
                {
                    hr = ExceptionToHResult(); // Do not propagate exceptions up.
                }

                GlobalUnlock(handle);
            }
        }

        CloseClipboard();
    }

    if (textChanged && g_renderer != NULL)
    {
        g_renderer->SetText(g_text.c_str());
        InvalidateRect(hwnd, NULL, TRUE);
    }

    return hr;
}

bool IsAnimating()
{
    return g_animationStartX != g_animationEndX;
}

void UpdateAnimation(HWND hwnd)
{
    if (!IsAnimating())
        return;

    // Compute the elapsed time since the start of the animation in seconds.
    DWORD const tickCount = GetTickCount();
    float elapsed = (tickCount - g_animationStartCount) * (1.0f / 1000);

    if (elapsed < g_animationDuration)
    {
        // We're still animating. Compuate the current x by interpolating between 
        // the start x and end x.
        float r = elapsed / g_animationDuration;
        g_animationCurrentX = (g_animationStartX * (1 - r)) + (g_animationEndX * r);
    }
    else
    {
        // We're done with this animation. Let both the current and start x equal the end x.
        g_animationCurrentX = g_animationEndX;
        g_animationStartX   = g_animationEndX;

        // If we're not at zero, we'll start a new animation ending at zero. The duration of
        // this animation depends on the distance.
        g_animationEndX = 0;
        g_animationStartCount = tickCount;
        g_animationDuration = fabs(g_animationStartX) / 10;
    }

    SetTransform(hwnd);
}

HRESULT CreateRenderer(HWND hwnd)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    delete g_renderer; g_renderer = NULL;

    if (g_rendererID == RendererD2D)
    {
        g_renderer = CreateD2DRenderer(
            hwnd,
            clientRect.right,
            clientRect.bottom,
            g_textFormat,
            g_text.c_str()
            );
    }
    else
    {
        g_renderer = CreateDWriteRenderer(
            hwnd,
            clientRect.right,
            clientRect.bottom,
            g_textFormat,
            g_text.c_str()
            );
    }
    if (g_renderer == NULL)
        return E_FAIL;

    g_renderer->SetMeasuringMode(g_measuringMode);

    if (g_monitor != NULL)
        g_renderer->SetMonitor(g_monitor);

    g_renderer->SetMagnifier(g_magnifier);

    SetTransform(hwnd);

    return S_OK;
}

void OnPaint(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Update the current state.
    UpdateAnimation(hwnd);
    if (g_renderer == NULL)
    {
        hr = CreateRenderer(hwnd);
    }

    // Paint the current frame.
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetLayout(ps.hdc, LAYOUT_BITMAPORIENTATIONPRESERVED);
        hr = g_renderer->Draw(hdc);
        EndPaint(hwnd, &ps);
    }

    if (FAILED(hr))
    {
        PostQuitMessage(hr);
        return;
    }

    // Invalidate if we're animating.
    if (IsAnimating())
    {
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void OnSize(HWND hwnd)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // Reposition the magnifier.
    g_magnifier.magnifierPos.x = clientRect.right  - g_magnifier.magnifierSize.cx - (g_dpiX / 8);
    g_magnifier.magnifierPos.y = clientRect.bottom - g_magnifier.magnifierSize.cy - (g_dpiY / 8);

    // Update the renderer if it exists.
    if (g_renderer != NULL)
    {
        g_renderer->SetWindowSize(
            clientRect.right,
            clientRect.bottom
            );

        g_renderer->SetMagnifier(g_magnifier);

        SetTransform(hwnd);
    }
}

void OnMouseDown(HWND hwnd)
{
    if (!g_magnifier.visible)
        return;

    GetCursorPos(&g_dragPos);
    ScreenToClient(hwnd, &g_dragPos);

    RECT focusRect =
    {
        g_magnifier.focusPos.x,
        g_magnifier.focusPos.y,
        g_magnifier.focusPos.x + static_cast<int>(g_magnifier.magnifierSize.cx / g_magnifier.scale),
        g_magnifier.focusPos.y + static_cast<int>(g_magnifier.magnifierSize.cy / g_magnifier.scale)
    };

    if (!PtInRect(&focusRect, g_dragPos))
        return;

    SetCapture(hwnd);
    g_dragging = true;
}

void OnMouseMove(HWND hwnd)
{
    if (g_dragging)
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);

        g_magnifier.focusPos.x += pt.x - g_dragPos.x;
        g_magnifier.focusPos.y += pt.y - g_dragPos.y;

        g_dragPos = pt;

        if (g_renderer != NULL)
            g_renderer->SetMagnifier(g_magnifier);
        
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void OnMouseUp(HWND hwnd)
{
    if (g_dragging)
    {
        ReleaseCapture();
        g_dragging = false;
    }
}

void SafeAppend(wchar_t* buffer, size_t bufferSize, size_t* length, wchar_t* text)
{
    // Append the new text if it fits into the buffer, and adjust the length.
    // Otherwise leave the buffer alone.
    size_t c = wcsnlen(text, UINT32_MAX);
    if (*length + c < bufferSize)
    {
        memcpy(buffer + *length, text, c * sizeof(buffer[0]));
        *length += c;
        buffer[*length] = '\0';
    }
}

void SafeAppend(wchar_t* buffer, size_t bufferSize, size_t* length, UINT stringID)
{
    *length += LoadString(
                    g_instance,
                    stringID,
                    buffer + *length,
                    static_cast<int>(bufferSize - *length)
                    );
}

void SetCaption(HWND hwnd)
{
    size_t const bufferSize = 256;
    wchar_t caption[bufferSize];
    size_t length = 0;

    // Load the window title.
    SafeAppend(caption, bufferSize, &length, IDS_APP_TITLE);

    // Add a hyphen separator.
    SafeAppend(caption, bufferSize, &length, L" - ");

    // Append a string representing the measuring mode.
    switch (g_measuringMode)
    {
    case DWRITE_MEASURING_MODE_NATURAL:
        SafeAppend(caption, bufferSize, &length, IDS_NATURAL_MODE);
        break;

    case DWRITE_MEASURING_MODE_GDI_CLASSIC:
        SafeAppend(caption, bufferSize, &length, IDS_GDI_CLASSIC_MODE);
        break;

    case DWRITE_MEASURING_MODE_GDI_NATURAL:
        SafeAppend(caption, bufferSize, &length, IDS_GDI_NATURAL_MODE);
        break;
    }

    // Add a comma separator.
    SafeAppend(caption, bufferSize, &length, L", ");

    // Append a string representing the renderer implementation.
    switch (g_rendererID)
    {
    case RendererD2D:
        SafeAppend(caption, bufferSize, &length, IDS_USING_D2D);
        break;

    case RendererDWrite:
        SafeAppend(caption, bufferSize, &length, IDS_USING_DWRITE);
        break;
    }

    SetWindowText(hwnd, caption);
}

