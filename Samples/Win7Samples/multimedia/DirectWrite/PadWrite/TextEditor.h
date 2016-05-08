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
#pragma once


class DECLSPEC_UUID("2EF2C6E3-5352-41c1-89D0-1C7F7F99359B") TextEditor
    :   public ComBase<QiList<IUnknown> >
{
public:
    enum SetSelectionMode
    {
        SetSelectionModeLeft,               // cluster left
        SetSelectionModeRight,              // cluster right
        SetSelectionModeUp,                 // line up
        SetSelectionModeDown,               // line down
        SetSelectionModeLeftChar,           // single character left (backspace uses it)
        SetSelectionModeRightChar,          // single character right
        SetSelectionModeLeftWord,           // single word left
        SetSelectionModeRightWord,          // single word right
        SetSelectionModeHome,               // front of line
        SetSelectionModeEnd,                // back of line
        SetSelectionModeFirst,              // very first position
        SetSelectionModeLast,               // very last position
        SetSelectionModeAbsoluteLeading,    // explicit position (for mouse click)
        SetSelectionModeAbsoluteTrailing,   // explicit position, trailing edge
        SetSelectionModeAll                 // select all text
    };

public:
    ////////////////////
    // Creation/destruction
    TextEditor(IDWriteFactory* factory);
    HRESULT static Create(
        HWND parentHwnd,
        const wchar_t* text,
        IDWriteTextFormat* textFormat,
        IDWriteFactory* factory,
        OUT TextEditor** textEditor
        );

    ~TextEditor()
    {
        SafeRelease(&textLayout_);
        SafeRelease(&renderTarget_);
        SafeRelease(&pageBackgroundEffect_);
        SafeRelease(&textSelectionEffect_);
        SafeRelease(&imageSelectionEffect_);
        SafeRelease(&caretBackgroundEffect_);
    }

    static ATOM RegisterWindowClass();
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND GetHwnd() {return hwnd_;}
    void OnDestroy();

    ////////////////////
    // Input dispatch
    void OnMousePress(UINT message, float x, float y);
    void OnMouseRelease(UINT message, float x, float y);
    void OnMouseMove(float x, float y);
    void OnMouseScroll(float xScroll, float yScroll);
    void OnMouseExit();
    void OnKeyPress(UINT32 keyCode);
    void OnKeyCharacter(UINT32 charCode);
    void MirrorXCoordinate(IN OUT float& x);

    ////////////////////
    // Drawing/view change
    void OnDraw();
    void DrawPage(RenderTarget& target);
    void OnSize(UINT width, UINT height);
    void OnScroll(UINT message, UINT request);
    void GetCaretRect(OUT RectF& rect);
    void UpdateSystemCaret(const RectF& rect);
    void SetRenderTarget(RenderTarget* target);
    void PostRedraw() { InvalidateRect(hwnd_, NULL, FALSE); }

    ////////////////////
    // Used by the main application to respond to button commands.
    void CopyToClipboard();
    void DeleteSelection();
    void PasteFromClipboard();
    HRESULT InsertText(const wchar_t* text);

    ////////////////////
    // Layout/editing/caret navigation
    void UpdateCaretFormatting();
    bool SetSelection(SetSelectionMode moveMode, UINT32 advance, bool extendSelection, bool updateCaretFormat = true);
    DWRITE_TEXT_RANGE GetSelectionRange();
    UINT32 GetCaretPosition();
    IDWriteTextLayout* GetLayout() { return textLayout_; }
    EditableLayout::CaretFormat& GetCaretFormat() { return caretFormat_; }

    ////////////////////
    // Current view
    float GetAngle() {return angle_;}
    float SetAngle(float angle, bool relativeAdjustement);
    void  SetScale(float scaleX, float scaleY, bool relativeAdjustement);
    void  GetScale(OUT float* scaleX, OUT  float* scaleY);
    void  GetViewMatrix(OUT DWRITE_MATRIX* matrix) const;
    void  GetInverseViewMatrix(OUT DWRITE_MATRIX* matrix) const;
    void  ResetView();
    void  RefreshView();

protected:
    HRESULT Initialize(HWND parentHwnd, const wchar_t* text, IDWriteTextFormat* textFormat);
    void InitDefaults();
    void InitViewDefaults();

    bool SetSelectionFromPoint(float x, float y, bool extendSelection);
    void AlignCaretToNearestCluster(bool isTrailingHit = false, bool skipZeroWidth = false);

    void UpdateScrollInfo();
    void ConstrainViewOrigin();
    void TextWasEdited();

    void GetLineMetrics(OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics);
    void GetLineFromPosition(
        const DWRITE_LINE_METRICS* lineMetrics,   // [lineCount]
        UINT32 lineCount,
        UINT32 textPosition,
        OUT UINT32* lineOut,
        OUT UINT32* linePositionOut
        );

protected:
    HWND hwnd_;

    RenderTarget*               renderTarget_;
    DrawingEffect*              pageBackgroundEffect_;
    DrawingEffect*              textSelectionEffect_;
    DrawingEffect*              imageSelectionEffect_;
    DrawingEffect*              caretBackgroundEffect_;
    IDWriteTextLayout*          textLayout_;
    EditableLayout              layoutEditor_;

    std::wstring text_;

    ////////////////////
    // Selection/Caret navigation
    ///
    // caretAnchor equals caretPosition when there is no selection.
    // Otherwise, the anchor holds the point where shift was held
    // or left drag started.
    //
    // The offset is used as a sort of trailing edge offset from
    // the caret position. For example, placing the caret on the
    // trailing side of a surrogate pair at the beginning of the
    // text would place the position at zero and offset of two.
    // So to get the absolute leading position, sum the two.
    UINT32 caretAnchor_;
    UINT32 caretPosition_;
    UINT32 caretPositionOffset_;    // > 0 used for trailing edge

    // Current attributes of the caret, which can be independent
    // of the text.
    EditableLayout::CaretFormat caretFormat_;

    ////////////////////
    // Mouse manipulation
    bool currentlySelecting_ : 1;
    bool currentlyPanning_   : 1;
    float previousMouseX;
    float previousMouseY;

    enum {MouseScrollFactor = 10};

    ////////////////////
    // Current view
    float scaleX_;          // horizontal scaling
    float scaleY_;          // vertical scaling
    float angle_;           // in degrees
    float originX_;         // focused point in document (moves on panning and caret navigation)
    float originY_;
    float contentWidth_;    // page size - margin left - margin right (can be fixed or variable)
    float contentHeight_;
};
