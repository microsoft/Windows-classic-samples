// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    UI formatted text editor.
//
// Usage:       Type to edit text.
//              Arrow keys move, +shift selects, +ctrl moves whole word
//              Left drag selects.
//              Middle drag pans.
//              Scroll wheel scrolls, +shift for horizontal, +ctrl zooms.
//
//----------------------------------------------------------------------------
#include "Common.h"
#include "DrawingEffect.h"
#include "RenderTarget.h"
#include "EditableLayout.h"
#include "TextEditor.h"


namespace
{
    // Private helper functions.

    inline D2D1::Matrix3x2F& Cast(DWRITE_MATRIX& matrix)
    {
        // DWrite's matrix, D2D's matrix, and GDI's XFORM
        // are all compatible.
        return *reinterpret_cast<D2D1::Matrix3x2F*>(&matrix);
    }

    inline DWRITE_MATRIX& Cast(D2D1::Matrix3x2F& matrix)
    {
        return *reinterpret_cast<DWRITE_MATRIX*>(&matrix);
    }

    inline int RoundToInt(float x)
    {
        return static_cast<int>(floor(x + .5));
    }

    inline double DegreesToRadians(float degrees)
    {
        return degrees * M_PI * 2.0f / 360.0f;
    }

    inline float GetDeterminant(DWRITE_MATRIX const& matrix)
    {
        return matrix.m11 * matrix.m22 - matrix.m12 * matrix.m21;
    }

    void ComputeInverseMatrix(
        DWRITE_MATRIX const& matrix,
        OUT DWRITE_MATRIX& result
        )
    {
        // Used for hit-testing, mouse scrolling, panning, and scroll bar sizing.

        float invdet = 1.f / GetDeterminant(matrix);
        result.m11 =  matrix.m22 * invdet;
        result.m12 = -matrix.m12 * invdet;
        result.m21 = -matrix.m21 * invdet;
        result.m22 =  matrix.m11 * invdet;
        result.dx  = (matrix.m21 * matrix.dy  - matrix.dx  * matrix.m22) * invdet;
        result.dy  = (matrix.dx  * matrix.m12 - matrix.m11 * matrix.dy)  * invdet;
    }

    D2D1_POINT_2F GetPageSize(IDWriteTextLayout* textLayout)
    {
        // Use the layout metrics to determine how large the page is, taking
        // the maximum of the content size and layout's maximal dimensions.

        DWRITE_TEXT_METRICS textMetrics;
        textLayout->GetMetrics(&textMetrics);

        float width  = std::max(textMetrics.layoutWidth, textMetrics.left + textMetrics.width);
        float height = std::max(textMetrics.layoutHeight, textMetrics.height);

        D2D1_POINT_2F pageSize = {width, height};
        return pageSize;
    }

    bool IsLandscapeAngle(float angle)
    {
        // Returns true if the angle is rotated 90 degrees clockwise
        // or anticlockwise (or any multiple of that).
        return fmod(abs(angle) + 45.0f, 180.0f) >= 90.0f;
    }
}


////////////////////////////////////////////////////////////////////////////////
// Initialization.


ATOM TextEditor::RegisterWindowClass()
{
    // Registers window class.
    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = &WindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = HINST_THISCOMPONENT;
    wcex.hIcon         = NULL;
    wcex.hCursor       = LoadCursor(NULL, IDC_IBEAM);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = TEXT("DirectWriteEdit");
    wcex.hIconSm       = NULL;

    return RegisterClassEx(&wcex);
}


TextEditor::TextEditor(IDWriteFactory* factory)
:   renderTarget_(),
    pageBackgroundEffect_(),
    textSelectionEffect_(),
    imageSelectionEffect_(),
    caretBackgroundEffect_(),
    textLayout_(),
    layoutEditor_(factory)
{
    // Creates editor window.

    InitDefaults();
    InitViewDefaults();
}


HRESULT TextEditor::Create(
    HWND parentHwnd,
    const wchar_t* text,
    IDWriteTextFormat* textFormat,
    IDWriteFactory* factory,
    OUT TextEditor** textEditor
    )
{
    *textEditor = NULL;
    HRESULT hr  = S_OK;

    // Create and initialize.
    TextEditor* newTextEditor = SafeAcquire(new(std::nothrow) TextEditor(factory));
    if (newTextEditor == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    hr = newTextEditor->Initialize(parentHwnd, text, textFormat);
    if (FAILED(hr))
        SafeRelease(&newTextEditor);

    *textEditor = SafeDetach(&newTextEditor);

    return hr;
}


HRESULT TextEditor::Initialize(HWND parentHwnd, const wchar_t* text, IDWriteTextFormat* textFormat)
{
    HRESULT hr = S_OK;

    // Set the initial text.
    try
    {
        text_.assign(text);
    }
    catch (...)
    {
        return ExceptionToHResult();
    }

    // Create an ideal layout for the text editor based on the text and format,
    // favoring document layout over pixel alignment.
    hr = layoutEditor_.GetFactory()->CreateTextLayout(
            text_.c_str(),
            static_cast<UINT32>(text_.size()),
            textFormat,
            580,            // maximum width
            420,            // maximum height
            &textLayout_
            );

    if (FAILED(hr))
        return hr;

    // Get size of text layout; needed for setting the view origin.
    float layoutWidth  = textLayout_->GetMaxWidth();
    float layoutHeight = textLayout_->GetMaxHeight();
    originX_ = layoutWidth  / 2;
    originY_ = layoutHeight / 2;

    // Set the initial text layout and update caret properties accordingly.
    UpdateCaretFormatting();

    // Set main two colors for drawing.
    pageBackgroundEffect_  = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::White));
    textSelectionEffect_   = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::LightSkyBlue));
    imageSelectionEffect_  = SafeAcquire(new(std::nothrow) DrawingEffect(0x80000000 | D2D1::ColorF::LightSkyBlue));
    caretBackgroundEffect_ = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::Black));

    if (pageBackgroundEffect_  == NULL
    ||  textSelectionEffect_   == NULL
    ||  imageSelectionEffect_  == NULL
    ||  caretBackgroundEffect_ == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Create text editor window (hwnd is stored in the create event)
    CreateWindowEx(
            WS_EX_STATICEDGE,
            L"DirectWriteEdit",
            L"",
            WS_CHILDWINDOW|WS_VSCROLL|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            parentHwnd,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
    if (hwnd_ == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


inline void TextEditor::InitDefaults()
{
    hwnd_                = NULL;

    caretPosition_       = 0;
    caretAnchor_         = 0;
    caretPositionOffset_ = 0;

    currentlySelecting_  = false;
    currentlyPanning_    = false;
    previousMouseX       = 0;
    previousMouseY       = 0;
}


inline void TextEditor::InitViewDefaults()
{
    scaleX_     = 1;
    scaleY_     = 1;
    angle_      = 0;
    originX_    = 0;
    originY_    = 0;
}


void TextEditor::SetRenderTarget(RenderTarget* target)
{
    SafeSet(&renderTarget_, target);
    PostRedraw();
}


void TextEditor::OnDestroy()
{
    SafeRelease(&renderTarget_);
}


////////////////////////////////////////////////////////////////////////////////
// Message dispatch.


LRESULT CALLBACK TextEditor::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Relays messages for the text editor to the internal class.

    TextEditor* window = reinterpret_cast<TextEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_NCCREATE:
        {
            // Associate the data structure with this window handle.
            CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<TextEditor*>(pcs->lpCreateParams);
            window->hwnd_ = hwnd;
            window->AddRef(); // implicit reference via HWND
            SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToUlong(window));

            return DefWindowProc(hwnd, message, wParam, lParam);
        }

    case WM_PAINT:
    case WM_DISPLAYCHANGE:
        window->OnDraw();
        break;

    case WM_ERASEBKGND: // don't want flicker
        return true;

    case WM_DESTROY:
        window->OnDestroy();
        break;

    case WM_NCDESTROY:
        // Remove implicit reference via HWND.
        // After this, the window and data structure no longer exist.
        window->Release();
        break;

    case WM_KEYDOWN:
        window->OnKeyPress(static_cast<UINT>(wParam));
        break;

    case WM_CHAR:
        window->OnKeyCharacter(static_cast<UINT>(wParam));
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        SetFocus(hwnd);
        SetCapture(hwnd);
        window->OnMousePress(message, float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
        break;

    case WM_MOUSELEAVE:
    case WM_CAPTURECHANGED:
        window->OnMouseExit();
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        ReleaseCapture();
        window->OnMouseRelease(message, float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
        break;

    case WM_SETFOCUS:
        {
            RectF rect;
            window->GetCaretRect(rect);
            window->UpdateSystemCaret(rect);
        }
        break;

    case WM_KILLFOCUS:
        DestroyCaret();
        break;

    case WM_MOUSEMOVE:
        window->OnMouseMove(float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        {
            // Retrieve the lines-to-scroll or characters-to-scroll user setting,
            // using a default value if the API failed.
            UINT userSetting;
            BOOL success = SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &userSetting, 0);
            if (success == FALSE)
                userSetting = 1;

            // Set x,y scroll difference,
            // depending on whether horizontal or vertical scroll.
            float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float yScroll = (zDelta / WHEEL_DELTA) * userSetting;
            float xScroll = 0;
            if (message == WM_MOUSEHWHEEL)
            {
                xScroll = -yScroll;
                yScroll = 0;
            }

            window->OnMouseScroll(xScroll, yScroll);
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        window->OnScroll(message, LOWORD(wParam));
        break;

    case WM_SIZE:
        {
            UINT width  = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            window->OnSize(width, height);
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Drawing/scrolling/sizing.


void TextEditor::OnDraw()
{
    PAINTSTRUCT ps;
    BeginPaint(hwnd_, &ps);

    if (renderTarget_ != NULL) // in case event received before we have a target
    {
        renderTarget_->BeginDraw();
        renderTarget_->Clear(D2D1::ColorF::LightGray);
        DrawPage(*renderTarget_);
        renderTarget_->EndDraw();
    }
    EndPaint(hwnd_, &ps);
}


void TextEditor::DrawPage(RenderTarget& target)
{
    // Draws the background, page, selection, and text.

    // Calculate actual location in render target based on the
    // current page transform and location of edit control.
    D2D1::Matrix3x2F pageTransform;
    GetViewMatrix(&Cast(pageTransform));

    // Scale/Rotate canvas as needed
    DWRITE_MATRIX previousTransform;
    target.GetTransform(previousTransform);
    target.SetTransform(Cast(pageTransform));

    // Draw the page
    D2D1_POINT_2F pageSize = GetPageSize(textLayout_);
    RectF pageRect = {0, 0, pageSize.x, pageSize.y};

    target.FillRectangle(pageRect, *pageBackgroundEffect_);

    // Determine actual number of hit-test ranges
    DWRITE_TEXT_RANGE caretRange = GetSelectionRange();
    UINT32 actualHitTestCount = 0;

    if (caretRange.length > 0)
    {
        textLayout_->HitTestTextRange(
            caretRange.startPosition,
            caretRange.length,
            0, // x
            0, // y
            NULL,
            0, // metrics count
            &actualHitTestCount
            );
    }

    // Allocate enough room to return all hit-test metrics.
    std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestCount);

    if (caretRange.length > 0)
    {
        textLayout_->HitTestTextRange(
            caretRange.startPosition,
            caretRange.length,
            0, // x
            0, // y
            &hitTestMetrics[0],
            static_cast<UINT32>(hitTestMetrics.size()),
            &actualHitTestCount
            );
    }

    // Draw the selection ranges behind the text.
    if (actualHitTestCount > 0)
    {
        // Note that an ideal layout will return fractional values,
        // so you may see slivers between the selection ranges due
        // to the per-primitive antialiasing of the edges unless
        // it is disabled (better for performance anyway).
        target.SetAntialiasing(false);

        for (size_t i = 0; i < actualHitTestCount; ++i)
        {
            const DWRITE_HIT_TEST_METRICS& htm = hitTestMetrics[i];
            RectF highlightRect = {
                htm.left,
                htm.top,
                (htm.left + htm.width),
                (htm.top  + htm.height)
            };
            
            target.FillRectangle(highlightRect, *textSelectionEffect_);
        }

        target.SetAntialiasing(true);
    }

    // Draw our caret onto the render target.
    RectF caretRect;
    GetCaretRect(caretRect);
    target.SetAntialiasing(false);
    target.FillRectangle(caretRect, *caretBackgroundEffect_);
    target.SetAntialiasing(true);

    // Draw text
    target.DrawTextLayout(textLayout_, pageRect);

    // Draw the selection ranges in front of images.
    // This shades otherwise opaque images so they are visibly selected,
    // checking the isText field of the hit-test metrics.
    if (actualHitTestCount > 0)
    {
        // Note that an ideal layout will return fractional values,
        // so you may see slivers between the selection ranges due
        // to the per-primitive antialiasing of the edges unless
        // it is disabled (better for performance anyway).
        target.SetAntialiasing(false);

        for (size_t i = 0; i < actualHitTestCount; ++i)
        {
            const DWRITE_HIT_TEST_METRICS& htm = hitTestMetrics[i];
            if (htm.isText)
                continue; // Only draw selection if not text.

            RectF highlightRect = {
                htm.left,
                htm.top,
                (htm.left + htm.width),
                (htm.top  + htm.height)
            };
            
            target.FillRectangle(highlightRect, *imageSelectionEffect_);
        }

        target.SetAntialiasing(true);
    }

    // Restore transform
    target.SetTransform(previousTransform);
}


void TextEditor::RefreshView()
{
    // Redraws the text and scrollbars.

    UpdateScrollInfo();
    PostRedraw();
}


void TextEditor::OnScroll(UINT message, UINT request)
{
    // Reacts to scroll bar changes.

    SCROLLINFO scrollInfo = {sizeof(scrollInfo)};
    scrollInfo.fMask = SIF_ALL;
 
    int barOrientation = (message == WM_VSCROLL) ? SB_VERT : SB_HORZ;

    if (!GetScrollInfo(hwnd_, barOrientation, &scrollInfo) )
        return;

    // Save the position for comparison later on
    int oldPosition = scrollInfo.nPos;

    switch (request)
    {
    case SB_TOP:        scrollInfo.nPos  = scrollInfo.nMin;      break;
    case SB_BOTTOM:     scrollInfo.nPos  = scrollInfo.nMax;      break;
    case SB_LINEUP:     scrollInfo.nPos -= 10;                   break;
    case SB_LINEDOWN:   scrollInfo.nPos += 10;                   break;
    case SB_PAGEUP:     scrollInfo.nPos -= scrollInfo.nPage;     break;
    case SB_PAGEDOWN:   scrollInfo.nPos += scrollInfo.nPage;     break;
    case SB_THUMBTRACK: scrollInfo.nPos  = scrollInfo.nTrackPos; break;
    default:
         break;
    }

    if (scrollInfo.nPos < 0)
        scrollInfo.nPos = 0;
    if (scrollInfo.nPos > scrollInfo.nMax - signed(scrollInfo.nPage))
        scrollInfo.nPos = scrollInfo.nMax - scrollInfo.nPage;

    scrollInfo.fMask = SIF_POS;
    SetScrollInfo(hwnd_, barOrientation, &scrollInfo, TRUE);

    // If the position has changed, scroll the window 
    if (scrollInfo.nPos != oldPosition)
    {
        // Need the view matrix in case the editor is flipped/mirrored/rotated.
        D2D1::Matrix3x2F pageTransform;
        GetInverseViewMatrix(&Cast(pageTransform));

        float inversePos = float(scrollInfo.nMax - scrollInfo.nPage - scrollInfo.nPos);

        D2D1_POINT_2F scaledSize = {pageTransform._11 + pageTransform._21,
                                    pageTransform._12 + pageTransform._22};

        // Adjust the correct origin.
        if ((barOrientation == SB_VERT) ^ IsLandscapeAngle(angle_))
        {
            originY_ = float(scaledSize.y >= 0 ? scrollInfo.nPos : inversePos);
        }
        else
        {
            originX_ = float(scaledSize.x >= 0 ? scrollInfo.nPos : inversePos);
        }

        ConstrainViewOrigin();
        PostRedraw();
    }
}


void TextEditor::UpdateScrollInfo()
{
    // Updates scroll bars.

    if (textLayout_ == NULL)
        return;

    // Determine scroll bar's step size in pixels by multiplying client rect by current view.
    RECT clientRect;
    GetClientRect(hwnd_, &clientRect);

    D2D1::Matrix3x2F pageTransform;
    GetInverseViewMatrix(&Cast(pageTransform));

    // Transform vector of viewport size
    D2D1_POINT_2F clientSize = {float(clientRect.right), float(clientRect.bottom)};
    D2D1_POINT_2F scaledSize = {clientSize.x * pageTransform._11 + clientSize.y * pageTransform._21,
                                clientSize.x * pageTransform._12 + clientSize.y * pageTransform._22};

    float x = originX_;
    float y = originY_;
    D2D1_POINT_2F pageSize = GetPageSize(textLayout_);

    SCROLLINFO scrollInfo = {sizeof(scrollInfo)};
    scrollInfo.fMask    = SIF_PAGE|SIF_POS|SIF_RANGE;

    if (IsLandscapeAngle(angle_))
    {
        std::swap(x, y);
        std::swap(pageSize.x, pageSize.y);
        std::swap(scaledSize.x, scaledSize.y);
    }

    // Set vertical scroll bar.
    scrollInfo.nPage    = int(abs(scaledSize.y));
    scrollInfo.nPos     = int(scaledSize.y >= 0 ? y : pageSize.y - y);
    scrollInfo.nMin     = 0;
    scrollInfo.nMax     = int(pageSize.y) + scrollInfo.nPage;
    SetScrollInfo(hwnd_, SB_VERT, &scrollInfo, TRUE);
    scrollInfo.nPos = 0;
    scrollInfo.nMax = 0;
    GetScrollInfo(hwnd_, SB_VERT, &scrollInfo);

    // Set horizontal scroll bar.
    scrollInfo.nPage    = int(abs(scaledSize.x));
    scrollInfo.nPos     = int(scaledSize.x >= 0 ? x : pageSize.x - x);
    scrollInfo.nMin     = 0;
    scrollInfo.nMax     = int(pageSize.x) + scrollInfo.nPage;
    SetScrollInfo(hwnd_, SB_HORZ, &scrollInfo, TRUE);
}


void TextEditor::OnSize(UINT width, UINT height)
{
    if (renderTarget_ != NULL)
        renderTarget_->Resize(width, height);

    RefreshView();
}


////////////////////////////////////////////////////////////////////////////////
// Input handling.


void TextEditor::OnMousePress(UINT message, float x, float y)
{
    MirrorXCoordinate(x);

    if (message == WM_LBUTTONDOWN)
    {
        // Start dragging selection.
        currentlySelecting_ = true;

        bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
        SetSelectionFromPoint(x, y, heldShift);
    }
    else if (message == WM_MBUTTONDOWN)
    {
        previousMouseX    = x;
        previousMouseY    = y;
        currentlyPanning_ = true;
    }
}


void TextEditor::OnMouseRelease(UINT message, float x, float y)
{
    MirrorXCoordinate(x);

    if (message == WM_LBUTTONUP)
    {
        currentlySelecting_ = false;
    }
    else if (message == WM_MBUTTONUP)
    {
        currentlyPanning_ = false;
    }
}


void TextEditor::OnMouseMove(float x, float y)
{
    // Selects text or pans.

    MirrorXCoordinate(x);

    if (currentlySelecting_)
    {
        // Drag current selection.
        SetSelectionFromPoint(x, y, true);
    }
    else if (currentlyPanning_)
    {
        DWRITE_MATRIX matrix;
        GetInverseViewMatrix(&matrix);

        float xDif = x - previousMouseX;
        float yDif = y - previousMouseY;
        previousMouseX = x;
        previousMouseY = y;

        originX_ -= (xDif * matrix.m11 + yDif * matrix.m21);
        originY_ -= (xDif * matrix.m12 + yDif * matrix.m22);
        ConstrainViewOrigin();

        RefreshView();
    }
}


void TextEditor::OnMouseScroll(float xScroll, float yScroll)
{
    // Pans or scales the editor.

    bool heldShift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
    bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

    if (heldControl)
    {
        // Scale
        float scaleFactor = (yScroll > 0) ? 1.0625f : 1/1.0625f;
        SetScale(scaleFactor, scaleFactor, true);
    }
    else
    {
        // Pan
        DWRITE_MATRIX matrix;
        GetInverseViewMatrix(&matrix);

        yScroll *= MouseScrollFactor;
        xScroll *= MouseScrollFactor; // for mice that support horizontal panning
        if (heldShift)
            std::swap(xScroll, yScroll);

        originX_ -= (xScroll * matrix.m11 + yScroll * matrix.m21);
        originY_ -= (xScroll * matrix.m12 + yScroll * matrix.m22);
        ConstrainViewOrigin();

        RefreshView();
    }
}


void TextEditor::OnMouseExit()
{
    currentlySelecting_ = false;
    currentlyPanning_   = false;
}


void TextEditor::MirrorXCoordinate(IN OUT float& x)
{
    // On RTL builds, coordinates may need to be restored to or converted
    // from Cartesian coordinates, where x increases positively to the right.
    if (GetWindowLong(hwnd_, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
    {
        RECT rect;
        GetClientRect(hwnd_, &rect);
        x = float(rect.right) - x - 1;
    }
}


void TextEditor::ConstrainViewOrigin()
{
    // Keep the page on-screen by not allowing the origin
    // to go outside the page bounds.

    D2D1_POINT_2F pageSize = GetPageSize(textLayout_);

    if (originX_ > pageSize.x)
        originX_ = pageSize.x;
    if (originX_ < 0)
        originX_ = 0;

    if (originY_ > pageSize.y)
        originY_ = pageSize.y;
    if (originY_ < 0)
        originY_ = 0;
}


void TextEditor::OnKeyPress(UINT32 keyCode)
{
    // Handles caret navigation and special presses that
    // do not generate characters.

    bool heldShift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
    bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

    UINT32 absolutePosition = caretPosition_ + caretPositionOffset_;

    switch (keyCode)
    {
    case VK_RETURN:
        // Insert CR/LF pair
        DeleteSelection();
        layoutEditor_.InsertTextAt(textLayout_, text_, absolutePosition, L"\r\n", 2, &caretFormat_);
        SetSelection(SetSelectionModeAbsoluteLeading, absolutePosition + 2, false, false);
        RefreshView();
        break;

    case VK_BACK:
        // Erase back one character (less than a character though).
        // Since layout's hit-testing always returns a whole cluster,
        // we do the surrogate pair detection here directly. Otherwise
        // there would be no way to delete just the diacritic following
        // a base character.

        if (absolutePosition != caretAnchor_)
        {
            // delete the selected text
            DeleteSelection();
        }
        else if (absolutePosition > 0)
        {
            UINT32 count = 1;
            // Need special case for surrogate pairs and CR/LF pair.
            if (absolutePosition >= 2
            &&  absolutePosition <= text_.size())
            {
                wchar_t charBackOne = text_[absolutePosition - 1];
                wchar_t charBackTwo = text_[absolutePosition - 2];
                if ((IsLowSurrogate(charBackOne) && IsHighSurrogate(charBackTwo))
                ||  (charBackOne == '\n' && charBackTwo == '\r'))
                {
                    count = 2;
                }
            }
            SetSelection(SetSelectionModeLeftChar, count, false);
            layoutEditor_.RemoveTextAt(textLayout_, text_, caretPosition_, count);
            RefreshView();
        }
        break;

    case VK_DELETE:
        // Delete following cluster.

        if (absolutePosition != caretAnchor_)
        {
            // Delete all the selected text.
            DeleteSelection();
        }
        else
        {
            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY;

            // Get the size of the following cluster.
            textLayout_->HitTestTextPosition(
                absolutePosition,
                false,
                &caretX,
                &caretY,
                &hitTestMetrics
                );

            layoutEditor_.RemoveTextAt(textLayout_, text_, hitTestMetrics.textPosition, hitTestMetrics.length);

            SetSelection(SetSelectionModeAbsoluteLeading, hitTestMetrics.textPosition, false);
            RefreshView();
        }
        break;

    case VK_TAB:
        break; // want tabs

    case VK_LEFT: // seek left one cluster
        SetSelection(heldControl ? SetSelectionModeLeftWord : SetSelectionModeLeft, 1, heldShift);
        break;

    case VK_RIGHT: // seek right one cluster
        SetSelection(heldControl ? SetSelectionModeRightWord : SetSelectionModeRight, 1, heldShift);
        break;

    case VK_UP: // up a line
        SetSelection(SetSelectionModeUp, 1, heldShift);
        break;

    case VK_DOWN: // down a line
        SetSelection(SetSelectionModeDown, 1, heldShift);
        break;

    case VK_HOME: // beginning of line
        SetSelection(heldControl ? SetSelectionModeFirst : SetSelectionModeHome, 0, heldShift);
        break;

    case VK_END: // end of line
        SetSelection(heldControl ? SetSelectionModeLast : SetSelectionModeEnd, 0, heldShift);
        break;

    case 'C':
        if (heldControl)
            CopyToClipboard();
        break;

    case VK_INSERT:
        if (heldControl)
            CopyToClipboard();
        else if (heldShift)
            PasteFromClipboard();
        break;

    case 'V':
        if (heldControl)
            PasteFromClipboard();
        break;

    case 'X':
        if (heldControl)
        {
            CopyToClipboard();
            DeleteSelection();
        }
        break;

    case 'A':
        if (heldControl)
            SetSelection(SetSelectionModeAll, 0, true);
        break;
    }
}


void TextEditor::OnKeyCharacter(UINT32 charCode)
{
    // Inserts text characters.

    // Allow normal characters and tabs
    if (charCode >= 0x20 || charCode == 9)
    {
        // Replace any existing selection.
        DeleteSelection();

        // Convert the UTF32 character code from the Window message to UTF16,
        // yielding 1-2 code-units. Then advance the caret position by how
        // many code-units were inserted.

        UINT32 charsLength = 1;
        wchar_t chars[2] = {static_cast<wchar_t>(charCode), 0};

        // If above the basic multi-lingual plane, split into
        // leading and trailing surrogates.
        if (charCode > 0xFFFF)
        {
            // From http://unicode.org/faq/utf_bom.html#35
            chars[0] = wchar_t(0xD800 + (charCode >> 10)  - (0x10000 >> 10));
            chars[1] = wchar_t(0xDC00 + (charCode & 0x3FF));
            charsLength++;
        }
        layoutEditor_.InsertTextAt(textLayout_, text_, caretPosition_ + caretPositionOffset_, chars, charsLength, &caretFormat_);
        SetSelection(SetSelectionModeRight, charsLength, false, false);

        RefreshView();
    }
}


////////////////////////////////////////////////////////////////////////////////
// Caret navigation and selection.


UINT32 TextEditor::GetCaretPosition()
{
    return caretPosition_ + caretPositionOffset_;
}


DWRITE_TEXT_RANGE TextEditor::GetSelectionRange()
{
    // Returns a valid range of the current selection,
    // regardless of whether the caret or anchor is first.

    UINT32 caretBegin = caretAnchor_;
    UINT32 caretEnd   = caretPosition_ + caretPositionOffset_;
    if (caretBegin > caretEnd)
        std::swap(caretBegin, caretEnd);

    // Limit to actual text length.
    UINT32 textLength = static_cast<UINT32>(text_.size());
    caretBegin = std::min(caretBegin,   textLength);
    caretEnd   = std::min(caretEnd,     textLength);

    DWRITE_TEXT_RANGE textRange = {caretBegin, caretEnd - caretBegin};
    return textRange;
}


void TextEditor::GetLineMetrics(
    OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics
    )
{
    // Retrieves the line metrics, used for caret navigation, up/down and home/end.

    DWRITE_TEXT_METRICS textMetrics;
    textLayout_->GetMetrics(&textMetrics);

    lineMetrics.resize(textMetrics.lineCount);
    textLayout_->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
}


void TextEditor::GetLineFromPosition(
    const DWRITE_LINE_METRICS* lineMetrics, // [lineCount]
    UINT32 lineCount,
    UINT32 textPosition,
    OUT UINT32* lineOut,
    OUT UINT32* linePositionOut
    )
{
    // Given the line metrics, determines the current line and starting text
    // position of that line by summing up the lengths. When the starting
    // line position is beyond the given text position, we have our line.

    UINT32 line = 0;
    UINT32 linePosition = 0;
    UINT32 nextLinePosition = 0;
    for ( ; line < lineCount; ++line)
    {
        linePosition = nextLinePosition;
        nextLinePosition = linePosition + lineMetrics[line].length;
        if (nextLinePosition > textPosition)
        {
            // The next line is beyond the desired text position,
            // so it must be in the current line.
            break;
        }
    }
    *linePositionOut = linePosition;
    *lineOut = std::min(line, lineCount - 1);
    return;
}


void TextEditor::AlignCaretToNearestCluster(bool isTrailingHit, bool skipZeroWidth)
{
    // Uses hit-testing to align the current caret position to a whole cluster,
    // rather than residing in the middle of a base character + diacritic,
    // surrogate pair, or character + UVS.

    DWRITE_HIT_TEST_METRICS hitTestMetrics;
    float caretX, caretY;

    // Align the caret to the nearest whole cluster.
    textLayout_->HitTestTextPosition(
        caretPosition_,
        false,
        &caretX,
        &caretY,
        &hitTestMetrics
        );

    // The caret position itself is always the leading edge.
    // An additional offset indicates a trailing edge when non-zero.
    // This offset comes from the number of code-units in the
    // selected cluster or surrogate pair.
    caretPosition_        = hitTestMetrics.textPosition;
    caretPositionOffset_  = (isTrailingHit) ? hitTestMetrics.length : 0;

    // For invisible, zero-width characters (like line breaks
    // and formatting characters), force leading edge of the
    // next position.
    if (skipZeroWidth && hitTestMetrics.width == 0)
    {
        caretPosition_      += caretPositionOffset_;
        caretPositionOffset_ = 0;
    }
}


bool TextEditor::SetSelectionFromPoint(float x, float y, bool extendSelection)
{
    // Returns the text position corresponding to the mouse x,y.
    // If hitting the trailing side of a cluster, return the
    // leading edge of the following text position.

    BOOL isTrailingHit;
    BOOL isInside;
    DWRITE_HIT_TEST_METRICS caretMetrics;

    // Remap display coordinates to actual.
    DWRITE_MATRIX matrix;
    GetInverseViewMatrix(&matrix);

    float transformedX = (x * matrix.m11 + y * matrix.m21 + matrix.dx);
    float transformedY = (x * matrix.m12 + y * matrix.m22 + matrix.dy);

    textLayout_->HitTestPoint(
        transformedX,
        transformedY,
        &isTrailingHit,
        &isInside,
        &caretMetrics
        );

    // Update current selection according to click or mouse drag.
    SetSelection(
        isTrailingHit ? SetSelectionModeAbsoluteTrailing : SetSelectionModeAbsoluteLeading,
        caretMetrics.textPosition,
        extendSelection
        );

    return true;
}


bool TextEditor::SetSelection(SetSelectionMode moveMode, UINT32 advance, bool extendSelection, bool updateCaretFormat)
{
    // Moves the caret relatively or absolutely, optionally extending the
    // selection range (for example, when shift is held).

    UINT32 line = UINT32_MAX; // current line number, needed by a few modes
    UINT32 absolutePosition    = caretPosition_ + caretPositionOffset_;
    UINT32 oldAbsolutePosition = absolutePosition;
    UINT32 oldCaretAnchor      = caretAnchor_;

    switch (moveMode)
    {
    case SetSelectionModeLeft:
        caretPosition_ += caretPositionOffset_;
        if (caretPosition_ > 0)
        {
            --caretPosition_;
            AlignCaretToNearestCluster(false, true);

            // special check for CR/LF pair
            absolutePosition = caretPosition_ + caretPositionOffset_;
            if (absolutePosition >= 1
            &&  absolutePosition < text_.size()
            &&  text_[absolutePosition - 1] == '\r'
            &&  text_[absolutePosition    ] == '\n')
            {
                caretPosition_ = absolutePosition - 1;
                AlignCaretToNearestCluster(false, true);
            }
        }
        break;

    case SetSelectionModeRight:
        caretPosition_ = absolutePosition;
        AlignCaretToNearestCluster(true, true);

        // special check for CR/LF pair
        absolutePosition = caretPosition_ + caretPositionOffset_;
        if (absolutePosition >= 1
        &&  absolutePosition < text_.size()
        &&  text_[absolutePosition - 1] == '\r'
        &&  text_[absolutePosition]     == '\n')
        {
            caretPosition_ = absolutePosition + 1;
            AlignCaretToNearestCluster(false, true);
        }
        break;

    case SetSelectionModeLeftChar:
        caretPosition_       = absolutePosition;
        caretPosition_      -= std::min(advance, absolutePosition);
        caretPositionOffset_ = 0;
        break;

    case SetSelectionModeRightChar:
        caretPosition_       = absolutePosition + advance;
        caretPositionOffset_ = 0;
        {
            // Use hit-testing to limit text position.
            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY;

            textLayout_->HitTestTextPosition(
                caretPosition_,
                false,
                &caretX,
                &caretY,
                &hitTestMetrics
                );
            caretPosition_ = std::min(caretPosition_, hitTestMetrics.textPosition + hitTestMetrics.length);
        }
        break;

    case SetSelectionModeUp:
    case SetSelectionModeDown:
        {
            // Retrieve the line metrics to figure out what line we are on.
            std::vector<DWRITE_LINE_METRICS> lineMetrics;
            GetLineMetrics(lineMetrics);

            UINT32 linePosition;
            GetLineFromPosition(
                &lineMetrics.front(),
                static_cast<UINT32>(lineMetrics.size()),
                caretPosition_,
                &line,
                &linePosition
                );

            // Move up a line or down
            if (moveMode == SetSelectionModeUp)
            {
                if (line <= 0)
                    break; // already top line
                line--;
                linePosition -= lineMetrics[line].length;
            }
            else
            {
                linePosition += lineMetrics[line].length;
                line++;
                if (line >= lineMetrics.size())
                    break; // already bottom line
            }

            // To move up or down, we need three hit-testing calls to determine:
            // 1. The x of where we currently are.
            // 2. The y of the new line.
            // 3. New text position from the determined x and y.
            // This is because the characters are variable size.

            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY, dummyX;

            // Get x of current text position
            textLayout_->HitTestTextPosition(
                caretPosition_,
                caretPositionOffset_ > 0, // trailing if nonzero, else leading edge
                &caretX,
                &caretY,
                &hitTestMetrics
                );

            // Get y of new position
            textLayout_->HitTestTextPosition(
                linePosition,
                false, // leading edge
                &dummyX,
                &caretY,
                &hitTestMetrics
                );

            // Now get text position of new x,y.
            BOOL isInside, isTrailingHit;
            textLayout_->HitTestPoint(
                caretX,
                caretY,
                &isTrailingHit,
                &isInside,
                &hitTestMetrics
                );

            caretPosition_       = hitTestMetrics.textPosition;
            caretPositionOffset_ = isTrailingHit ? (hitTestMetrics.length > 0) : 0;
        }
        break;

    case SetSelectionModeLeftWord:
    case SetSelectionModeRightWord:
        {
            // To navigate by whole words, we look for the canWrapLineAfter
            // flag in the cluster metrics.

            // First need to know how many clusters there are.
            std::vector<DWRITE_CLUSTER_METRICS> clusterMetrics;
            UINT32 clusterCount;
            textLayout_->GetClusterMetrics(NULL, 0, &clusterCount);

            if (clusterCount == 0)
                break;

            // Now we actually read them.
            clusterMetrics.resize(clusterCount);
            textLayout_->GetClusterMetrics(&clusterMetrics.front(), clusterCount, &clusterCount);

            caretPosition_ = absolutePosition;

            UINT32 clusterPosition  = 0;
            UINT32 oldCaretPosition = caretPosition_;

            if (moveMode == SetSelectionModeLeftWord)
            {
                // Read through the clusters, keeping track of the farthest valid
                // stopping point just before the old position.
                caretPosition_       = 0;
                caretPositionOffset_ = 0; // leading edge
                for (UINT32 cluster  = 0; cluster < clusterCount; ++cluster)
                {
                    clusterPosition += clusterMetrics[cluster].length;
                    if (clusterMetrics[cluster].canWrapLineAfter)
                    {
                        if (clusterPosition >= oldCaretPosition)
                            break;

                        // Update in case we pass this point next loop.
                        caretPosition_ = clusterPosition;
                    }
                }
            }
            else // SetSelectionModeRightWord
            {
                // Read through the clusters, looking for the first stopping point
                // after the old position.
                for (UINT32 cluster = 0; cluster < clusterCount; ++cluster)
                {
                    UINT32 clusterLength = clusterMetrics[cluster].length;
                    caretPosition_       = clusterPosition;
                    caretPositionOffset_ = clusterLength; // trailing edge
                    if (clusterPosition >= oldCaretPosition && clusterMetrics[cluster].canWrapLineAfter)
                        break; // first stopping point after old position.

                    clusterPosition += clusterLength;
                }
            }
        }
        break;

    case SetSelectionModeHome:
    case SetSelectionModeEnd:
        {
            // Retrieve the line metrics to know first and last position
            // on the current line.
            std::vector<DWRITE_LINE_METRICS> lineMetrics;
            GetLineMetrics(lineMetrics);

            GetLineFromPosition(
                &lineMetrics.front(),
                static_cast<UINT32>(lineMetrics.size()),
                caretPosition_,
                &line,
                &caretPosition_
                );

            caretPositionOffset_ = 0;
            if (moveMode == SetSelectionModeEnd)
            {
                // Place the caret at the last character on the line,
                // excluding line breaks. In the case of wrapped lines,
                // newlineLength will be 0.
                UINT32 lineLength    = lineMetrics[line].length - lineMetrics[line].newlineLength;
                caretPositionOffset_ = std::min(lineLength, 1u);
                caretPosition_      += lineLength - caretPositionOffset_;
                AlignCaretToNearestCluster(true);
            }
        }
        break;

    case SetSelectionModeFirst:
        caretPosition_       = 0;
        caretPositionOffset_ = 0;
        break;

    case SetSelectionModeAll:
        caretAnchor_ = 0;
        extendSelection = true;
        __fallthrough;

    case SetSelectionModeLast:
        caretPosition_       = UINT32_MAX;
        caretPositionOffset_ = 0;
        AlignCaretToNearestCluster(true);
        break;

    case SetSelectionModeAbsoluteLeading:
        caretPosition_       = advance;
        caretPositionOffset_ = 0;
        break;

    case SetSelectionModeAbsoluteTrailing:
        caretPosition_       = advance;
        AlignCaretToNearestCluster(true);
        break;
    }

    absolutePosition = caretPosition_ + caretPositionOffset_;

    if (!extendSelection)
        caretAnchor_ = absolutePosition;

    bool caretMoved = (absolutePosition != oldAbsolutePosition)
                   || (caretAnchor_     != oldCaretAnchor);

    if (caretMoved)
    {
        // update the caret formatting attributes
        if (updateCaretFormat)
            UpdateCaretFormatting();

        PostRedraw();

        RectF rect;
        GetCaretRect(rect);
        UpdateSystemCaret(rect);
    }

    return caretMoved;
}


void TextEditor::GetCaretRect(OUT RectF& rect)
{
    // Gets the current caret position (in untransformed space).

    RectF zeroRect = {};
    rect = zeroRect;

    if (textLayout_ == NULL)
        return;

    // Translate text character offset to point x,y.
    DWRITE_HIT_TEST_METRICS caretMetrics;
    float caretX, caretY;

    textLayout_->HitTestTextPosition(
        caretPosition_,
        caretPositionOffset_ > 0, // trailing if nonzero, else leading edge
        &caretX,
        &caretY,
        &caretMetrics
        );

    // If a selection exists, draw the caret using the
    // line size rather than the font size.
    DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
    if (selectionRange.length > 0)
    {
        UINT32 actualHitTestCount = 1;
        textLayout_->HitTestTextRange(
            caretPosition_,
            0, // length
            0, // x
            0, // y
            &caretMetrics,
            1,
            &actualHitTestCount
            );

        caretY = caretMetrics.top;
    }

    // The default thickness of 1 pixel is almost _too_ thin on modern large monitors,
    // but we'll use it.
    DWORD caretIntThickness = 2;
    SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretIntThickness, FALSE);
    const float caretThickness = float(caretIntThickness);

    // Return the caret rect, untransformed.
    rect.left   = caretX    - caretThickness / 2.0f;
    rect.right  = rect.left + caretThickness;
    rect.top    = caretY;
    rect.bottom = caretY + caretMetrics.height;
}


void TextEditor::UpdateSystemCaret(const RectF& rect)
{
    // Moves the system caret to a new position.

    // Although we don't actually use the system caret (drawing our own
    // instead), this is important for accessibility, so the magnifier
    // can follow text we type. The reason we draw our own directly
    // is because intermixing DirectX and GDI content (the caret) reduces
    // performance.

    // Gets the current caret position (in untransformed space).

    if (GetFocus() != hwnd_) // Only update if we have focus.
        return;

    D2D1::Matrix3x2F pageTransform;
    GetViewMatrix(&Cast(pageTransform));

    // Transform caret top/left and size according to current scale and origin.
    D2D1_POINT_2F caretPoint = pageTransform.TransformPoint(D2D1::Point2F(rect.left, rect.top));

    float width  = (rect.right - rect.left);
    float height = (rect.bottom - rect.top);
    float transformedWidth  = width * pageTransform._11 + height * pageTransform._21;
    float transformedHeight = width * pageTransform._12 + height * pageTransform._22;

    // Update the caret's location, rounding to nearest integer so that
    // it lines up with the text selection.

    int intX      = RoundToInt(caretPoint.x);
    int intY      = RoundToInt(caretPoint.y);
    int intWidth  = RoundToInt(transformedWidth);
    int intHeight = RoundToInt(caretPoint.y + transformedHeight) - intY;

    CreateCaret(hwnd_, NULL, intWidth, intHeight);
    SetCaretPos(intX, intY);

    // Don't actually call ShowCaret. It's enough to just set its position.
}


void TextEditor::UpdateCaretFormatting()
{
    UINT32 currentPos = caretPosition_ + caretPositionOffset_;

    if (currentPos > 0)
    {
        --currentPos; // Always adopt the trailing properties.
    }

    // Get the family name
    caretFormat_.fontFamilyName[0] = '\0';
    textLayout_->GetFontFamilyName(currentPos, &caretFormat_.fontFamilyName[0], ARRAYSIZE(caretFormat_.fontFamilyName));

    // Get the locale
    caretFormat_.localeName[0] = '\0';
    textLayout_->GetLocaleName(currentPos, &caretFormat_.localeName[0], ARRAYSIZE(caretFormat_.localeName));

    // Get the remaining attributes...
    textLayout_->GetFontWeight(   currentPos, &caretFormat_.fontWeight);
    textLayout_->GetFontStyle(    currentPos, &caretFormat_.fontStyle);
    textLayout_->GetFontStretch(  currentPos, &caretFormat_.fontStretch);
    textLayout_->GetFontSize(     currentPos, &caretFormat_.fontSize);
    textLayout_->GetUnderline(    currentPos, &caretFormat_.hasUnderline);
    textLayout_->GetStrikethrough(currentPos, &caretFormat_.hasStrikethrough);

    // Get the current color.
    IUnknown* drawingEffect = NULL;
    textLayout_->GetDrawingEffect(currentPos, &drawingEffect);
    caretFormat_.color = 0;
    if (drawingEffect != NULL)
    {
        DrawingEffect& effect = *reinterpret_cast<DrawingEffect*>(drawingEffect);
        caretFormat_.color = effect.GetColor();
    }

    SafeRelease(&drawingEffect);
}


////////////////////////////////////////////////////////////////////////////////
// Selection/clipboard actions.


void TextEditor::CopyToClipboard()
{
    // Copies selected text to clipboard.

    DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
    if (selectionRange.length <= 0)
        return;

    // Open and empty existing contents.
    if (OpenClipboard(hwnd_))
    {
        if (EmptyClipboard())
        {
            // Allocate room for the text
            size_t byteSize         = sizeof(wchar_t) * (selectionRange.length + 1);
            HGLOBAL hClipboardData  = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);

            if (hClipboardData != NULL)
            {
                void* memory = GlobalLock(hClipboardData);  // [byteSize] in bytes

                if (memory != NULL)
                {
                    // Copy text to memory block.
                    const wchar_t* text = text_.c_str();
                    memcpy(memory, &text[selectionRange.startPosition], byteSize);
                    GlobalUnlock(hClipboardData);

                    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) != NULL)
                    {
                        hClipboardData = NULL; // system now owns the clipboard, so don't touch it.
                    }
                }
                GlobalFree(hClipboardData); // free if failed
            }
        }
        CloseClipboard();
    }
}


void TextEditor::DeleteSelection()
{
    // Deletes selection.

    DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
    if (selectionRange.length <= 0)
        return;

    layoutEditor_.RemoveTextAt(textLayout_, text_, selectionRange.startPosition, selectionRange.length);

    SetSelection(SetSelectionModeAbsoluteLeading, selectionRange.startPosition, false);
    RefreshView();
}


void TextEditor::PasteFromClipboard()
{
    // Pastes text from clipboard at current caret position.

    DeleteSelection();

    UINT32 characterCount = 0;

    // Copy Unicode text from clipboard.

    if (OpenClipboard(hwnd_))
    {
        HGLOBAL hClipboardData = GetClipboardData(CF_UNICODETEXT);

        if (hClipboardData != NULL)
        {
            // Get text and size of text.
            size_t byteSize                 = GlobalSize(hClipboardData);
            void* memory                    = GlobalLock(hClipboardData); // [byteSize] in bytes
            const wchar_t* text             = reinterpret_cast<const wchar_t*>(memory);
            characterCount                  = static_cast<UINT32>(wcsnlen(text, byteSize / sizeof(wchar_t)));

            if (memory != NULL)
            {
                // Insert the text at the current position.
                layoutEditor_.InsertTextAt(
                    textLayout_,
                    text_,
                    caretPosition_ + caretPositionOffset_,
                    text,
                    characterCount
                    );
                GlobalUnlock(hClipboardData);
            }
        }
        CloseClipboard();
    }

    SetSelection(SetSelectionModeRightChar, characterCount, true);
    RefreshView();
}


HRESULT TextEditor::InsertText(const wchar_t* text)
{
    UINT32 absolutePosition = caretPosition_ + caretPositionOffset_;

    return layoutEditor_.InsertTextAt(
                textLayout_,
                text_,
                absolutePosition,
                text,
                static_cast<UINT32>(wcsnlen(text, UINT32_MAX)),
                &caretFormat_
                );
}


////////////////////////////////////////////////////////////////////////////////
// Current view.


void TextEditor::ResetView()
{
    // Resets the default view.

    InitViewDefaults();

    // Center document
    float layoutWidth  = textLayout_->GetMaxWidth();
    float layoutHeight = textLayout_->GetMaxHeight();
    originX_ = layoutWidth  / 2;
    originY_ = layoutHeight / 2;

    RefreshView();
}


float TextEditor::SetAngle(float angle, bool relativeAdjustement)
{
    if (relativeAdjustement)
        angle_ += angle;
    else
        angle_ = angle;

    RefreshView();

    return angle_;
}


void TextEditor::SetScale(float scaleX, float scaleY, bool relativeAdjustement)
{
    if (relativeAdjustement)
    {
        scaleX_ *= scaleX;
        scaleY_ *= scaleY;
    }
    else
    {
        scaleX_ = scaleX;
        scaleY_ = scaleY;
    }
    RefreshView();
}


void TextEditor::GetScale(OUT float* scaleX, OUT float* scaleY)
{
    *scaleX = scaleX_;
    *scaleY = scaleY_;
}


void TextEditor::GetViewMatrix(OUT DWRITE_MATRIX* matrix) const
{
    // Generates a view matrix from the current origin, angle, and scale.

    // Need the editor size for centering.
    RECT rect;
    GetClientRect(hwnd_, &rect);

    // Translate the origin to 0,0
    DWRITE_MATRIX translationMatrix = {
        1, 0,
        0, 1,
        -originX_, -originY_
    };

    // Scale and rotate
    double radians = DegreesToRadians(fmod(angle_, 360.0f));
    double cosValue = cos(radians);
    double sinValue = sin(radians);

    // If rotation is a quarter multiple, ensure sin and cos are exactly one of {-1,0,1}
    if (fmod(angle_, 90.0f) == 0)
    {
        cosValue = floor(cosValue + .5);
        sinValue = floor(sinValue + .5);
    }

    DWRITE_MATRIX rotationMatrix = {
        float( cosValue * scaleX_), float(sinValue * scaleX_),
        float(-sinValue * scaleY_), float(cosValue * scaleY_),
        0, 0
    };

    // Set the origin in the center of the window
    float centeringFactor = .5f;
    DWRITE_MATRIX centerMatrix = {
        1, 0,
        0, 1,
        floor(float(rect.right * centeringFactor)), floor(float(rect.bottom * centeringFactor))
    };

    D2D1::Matrix3x2F resultA, resultB;

    resultB.SetProduct(Cast(translationMatrix), Cast(rotationMatrix));
    resultA.SetProduct(resultB,                 Cast(centerMatrix)  );

    // For better pixel alignment (less blurry text)
    resultA._31 = floor(resultA._31);
    resultA._32 = floor(resultA._32);

    *matrix = *reinterpret_cast<DWRITE_MATRIX*>(&resultA);
}


void TextEditor::GetInverseViewMatrix(OUT DWRITE_MATRIX* matrix) const
{
    // Inverts the view matrix for hit-testing and scrolling.

    DWRITE_MATRIX viewMatrix;
    GetViewMatrix(&viewMatrix);
    ComputeInverseMatrix(viewMatrix, *matrix);
}
