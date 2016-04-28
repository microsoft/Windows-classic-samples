// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma warning(disable:4458)
#include <gdiplus.h>
#pragma warning(default:4458)

using namespace Gdiplus;

enum TextStyle
{
    TextStyle_Normal,
    TextStyle_Header1,
    TextStyle_Header2,
    TextStyle_Title,
    TextStyle_Bold
};

struct TextLine
{
    PCWSTR text;     // the text of the line
    TextStyle style; // the text style of the line
};

struct Annotation
{
    int id;         // A unique identifier for the annotaiton
    int line;       // the line the annotation refers to
    int start_char; // the first character the annotaiton refers to in the line
    int length;     // the length of the text the annotation refers to
    PCWSTR text;    // the text of the annotation
    PCWSTR author;  // the author of the annotation
    SYSTEMTIME time; // the creation time of the comment
};

struct EndPoint
{
    int line;
    int character;
};

struct Range
{
    EndPoint begin;
    EndPoint end;
};

inline int QuickCompareEndpoints(_In_ EndPoint endpoint1, _In_ EndPoint endpoint2)
{
    if (endpoint1.line < endpoint2.line)
    {
        return -2;
    }
    else if (endpoint1.line > endpoint2.line)
    {
        return 2;
    }
    else 
    {
        if (endpoint1.character < endpoint2.character)
        {
            return -1;
        }
        else if (endpoint1.character > endpoint2.character)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

class AnnotatedTextControl
{
public:
    AnnotatedTextControl(_In_reads_(lineCount) TextLine *lines, _In_ int lineCount,
                         _In_reads_(annotationCount) Annotation *annotations, _In_ int annotationCount);
    static HWND Create(_In_ HWND parent, _In_ HINSTANCE instance);

    float GetTextAreaWidth();
    RectF GetLinePosition(_In_ int line, _In_ Graphics *graphics);
    Region * GetLineCharactersPosition(_In_ int line, _In_ int first, _In_ int length, _In_ Graphics *graphics);
    RectF GetAnnotationPosition(_In_ unsigned int annotation, _In_ Graphics *graphics);
    int GetLineCount();

    _Post_equal_to_(this->annotationCount)
    unsigned int GetAnnotationCount() const;

    TextLine *GetLine(_In_ int line);

    _When_(annotation < this->annotationCount, _Post_equal_to_(&this->annotations[annotation]))
    _When_(annotation >= this->annotationCount, _Ret_null_)
    Annotation *GetAnnotation(_In_ unsigned int annotation) const;

    int GetDesiredHeight();
    EndPoint SearchForClosestEndPoint(_In_ float x, _In_ float y);
    EndPoint GetEnd();
    int GetLineLength(_In_ int line);
    VARIANT GetAttributeAtPoint(_In_ EndPoint start, _In_ TEXTATTRIBUTEID attribute);
    bool StepCharacter(_In_ EndPoint start, _In_ bool forward, _Out_ EndPoint *end);
    bool StepLine(_In_ EndPoint start, _In_ bool forward, _Out_ EndPoint *end);
    bool IsActive();
    EndPoint GetCaretPosition();

private:
    static bool Initialize(_In_ HINSTANCE instance);
    static bool initialized;
    static LRESULT CALLBACK StaticWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    void OnPaint(_In_ HDC hdc);
    LRESULT OnSize(_In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT OnScroll(_In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT OnSetFocus();
    LRESULT OnKillFocus();
    LRESULT OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT OnLButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam);

    SizeF GetLineDimensions(_In_ int line, _In_ Graphics *graphics);
    SizeF GetAnnotationDimensions(_In_ unsigned int annotation, _In_ Graphics *graphics);
    RectF GetCaretScreenPosition();
    void UpdateCaret(_In_ EndPoint newPosition);

    HWND hwnd;
    TextLine *lines;
    int lineCount;
    Annotation *annotations;
    unsigned int annotationCount;
    int currentScroll;
    int maxScroll;

    EndPoint caretPosition;
    bool isActive;
};

// This is a method in TextAreaProvider that fires UIA events for the Caret moving
void NotifyCaretPositionChanged(_In_ HWND hwnd, _In_ AnnotatedTextControl *control);
