// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <strsafe.h>

#include "AnnotatedTextControl.h"
#include "FrameProvider.h"


bool AnnotatedTextControl::initialized = false;

TextLine sampleLines [] =
{
    { L"Impressive Title", TextStyle_Title },
    { L"This is a simple introduction to the UI Automation Document Content Provider Sample", TextStyle_Normal },
    { L"Section 1)", TextStyle_Header1 },
    { L"Item b)", TextStyle_Header2 },
    { L"\
This is a long paragraph: Lorem ipsum dolor sit amet, consectetuer adipiscing elit. \
Maecenas porttitor congue massa. Fusce posuere, magna sed pulvinar ultricies, purus \
lectus malesuada libero, sit amet commodo magna eros quis urna. Nunc viverra imperdiet \
enim. Fusce est. Vivamus a tellus. Pellentesque habitant morbi tristique senectus et netus\
et malesuada fames ac turpis egestas. Proin pharetra nonummy pede. Mauris et orci."
        , TextStyle_Normal },
    { L"And now to Emphasize a point", TextStyle_Bold },
    { L"In conclusion ...", TextStyle_Normal },
    { L"\
Another long paragraph: aaaabbbbbb abbbbb bbbbccccc ddddddd eeeeee ffff ggggggg hhhh \
iiiijjjj kkklll mmmmnnn ooooppp pqqqqrrr ssssttt uuuvvvv wwwwxxx xxyzzzz. Abcdef abcde \
12345 aaabbbb bbccc ccccddddd dddddddeee eeeeeefff fffff ggggggg hhhhiiiiiiiii jjjjjkkk\
lllm mmm mmmmnn nnnooppppp pqqqqqq qqqrrrr ssssssstt tttuuvv wwwwwwxx xxxxxyzz zzzzz."
        , TextStyle_Normal },
};

Annotation sampleAnnotations [] = 
{
 //   id, line, first, len,     ----- text -----                  author                date
    {  0,    0,     0,  10, L"Is it really that impressive?",    L"Skeptical Reader",  {2011, 2, 2, 8, 14, 30, 0, 0} },
    {  1,    3,     5,   1, L"Where did \"Item a)\" go?",        L"Eagle Eyed Editor", {2011, 4, 1,11, 10, 25, 0, 0} },
    {  2,    4,    84,   8, L"I met Maecenas once... cool guy.", L"Vivamus",           {1776, 7, 4, 4, 17, 00, 0, 0} },
    {  3,    6,     0,  -1, L"Is this really needed at all?",    L"Skeptical Reader",  {2011, 2, 2, 8, 14, 45, 0, 0} },
    {  4,    6,     15,  9, L"I'm an overlapping comment!",      L"Zealous Tester",    {2015,11, 6,17,  1, 30, 0, 0} }
};

struct StyleDefinition {
    TextStyle style;
    float fontSize;
    float indent;
    float verticalSpacing;
    int fontStyle;
    long uiaStyleId;
    PCWSTR uiaStyleName;
};

// Some parameters for our drawing
const PCWSTR textFontFace = L"Calibri";
const PCWSTR annotationFontFace = L"Arial Narrow";
const Color textColor(255,0,0,0);
const Color annotationColor(255,20,70,200);
const Color annotationTextColor(255,255,255,255);
const float annotationFontSize = 14.0f;
const float annotationPadding = 5.0f;
const float annotationInflation = 3.0f;
const float annotationLineWidth = 1.5f;
const float verticalPadding = 10.0f;

StyleDefinition styleDefinitions [] =
{
    // Style,             size, indent, vspacing, font style
    { TextStyle_Normal,  12.0f,  30.0f,     3.0f, FontStyleRegular                  , StyleId_Normal,   L"Normal" },
    { TextStyle_Header1, 24.0f,  10.0f,    10.0f, FontStyleRegular                  , StyleId_Heading1, L"Heading 1" },
    { TextStyle_Header2, 18.0f,  20.0f,     7.0f, FontStyleItalic                   , StyleId_Heading2, L"Heading 2" },
    { TextStyle_Title,   30.0f,  50.0f,    20.0f, FontStyleBold | FontStyleUnderline, StyleId_Title,    L"Title" },
    { TextStyle_Bold,    12.0f,  30.0f,     3.0f, FontStyleBold                     , StyleId_Emphasis, L"Emphasis"}
};  

StyleDefinition* GetStyleDefinition(_In_ TextStyle style)
{
    for (int i = 0; i < ARRAYSIZE(styleDefinitions); i++)
    {
        if (styleDefinitions[i].style == style)
        {
            return &(styleDefinitions[i]);
        }
    }

    // If the style requested isn't found, return the Normal style
    return &(styleDefinitions[0]);
}


HWND AnnotatedTextControl::Create(_In_ HWND parent, _In_ HINSTANCE instance)
{
    HWND returnHwnd = NULL;

    if (!initialized)
    {
        initialized = Initialize(instance);
    }

    if (initialized)
    {
        AnnotatedTextControl* control = new AnnotatedTextControl(sampleLines, ARRAYSIZE(sampleLines), sampleAnnotations, ARRAYSIZE(sampleAnnotations));
        if (control != NULL)
        {
            control->hwnd = CreateWindow( L"AnnotatedTextControl", L"",
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL,
                0, 0, 1, 1, parent, NULL, instance, static_cast<PVOID>(control));
            returnHwnd = control->hwnd;
        }
    }
    return returnHwnd;
}


bool AnnotatedTextControl::Initialize(_In_ HINSTANCE instance)
{
    WNDCLASS wc;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = AnnotatedTextControl::StaticWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = instance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"AnnotatedTextControl";

    if (!RegisterClass(&wc))
    {
        return false;
    }

    return true;
}

AnnotatedTextControl::AnnotatedTextControl(_In_reads_(lineCount) TextLine *lines, _In_ int lineCount,
                         _In_reads_(annotationCount) Annotation *annotations, _In_ int annotationCount)
{
    hwnd = NULL;
    AnnotatedTextControl::lines = lines;
    AnnotatedTextControl::lineCount = lineCount;
    AnnotatedTextControl::annotations = annotations;
    AnnotatedTextControl::annotationCount = annotationCount;
    currentScroll = 0;
    maxScroll = 0;
    caretPosition.line = 0;
    caretPosition.character = 0;
    isActive = false;
}

LRESULT CALLBACK AnnotatedTextControl::WndProc(_In_ HWND hwndAnnotatedTextControl, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    LRESULT lResult = 0;
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwndAnnotatedTextControl, &ps);
            OnPaint(ps.hdc);
            EndPaint(hwndAnnotatedTextControl, &ps);
            break;
        }
    case WM_GETOBJECT:
        {
            IRawElementProviderSimple * provider = new FrameProvider(hwndAnnotatedTextControl, this);
            if (provider != NULL)
            {
                lResult = UiaReturnRawElementProvider(hwndAnnotatedTextControl, wParam, lParam, provider);
                provider->Release();
            }
            break;
        }
    case WM_SIZE:
        {
            lResult = OnSize(wParam, lParam);
            break;
        }
    case WM_VSCROLL:
        {
            lResult = OnScroll(wParam, lParam);
            break;
        }
    case WM_SETFOCUS:
        {
            lResult = OnSetFocus();
            break;
        }
    case WM_KILLFOCUS:
        {
            lResult = OnKillFocus();
            break;
        }
    case WM_KEYDOWN:
        {
            lResult = OnKeyDown(wParam, lParam);
            break;
        }
    case WM_LBUTTONDOWN:
        {
            lResult = OnLButtonDown(wParam, lParam);
            break;
        }

    default:
        lResult = DefWindowProc(hwndAnnotatedTextControl, message, wParam, lParam);
        break;
    }
            
    return lResult;
}

LRESULT CALLBACK AnnotatedTextControl::StaticWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    AnnotatedTextControl * pThis = reinterpret_cast<AnnotatedTextControl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<AnnotatedTextControl*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    
    if (message == WM_NCDESTROY)
    {
        pThis = NULL;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
    }
    
    if (pThis != NULL)
    {
        return pThis->WndProc(hwnd, message, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, message, wParam, lParam);
}


void AnnotatedTextControl::OnPaint(_In_ HDC hdc)
{
    Graphics graphics(hdc);

    SolidBrush textBrush(textColor);
    const StringFormat* stringFormat = StringFormat::GenericDefault();

    graphics.SetCompositingMode(CompositingModeSourceOver);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);

    // Draw each of the lines in turn
    for(int i = 0; i < lineCount; i++)
    {
        StyleDefinition* styleDef = GetStyleDefinition(lines[i].style);
        RectF lineRect = GetLinePosition(i, &graphics);
        Font font(textFontFace, styleDef->fontSize, styleDef->fontStyle);
        graphics.DrawString(lines[i].text, -1, &font, lineRect, stringFormat, &textBrush);
    }

    SolidBrush annotationBrush(annotationColor);
    SolidBrush annotationTextBrush(annotationTextColor);
    Pen annotationPen(annotationColor, annotationLineWidth);

    graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // Draw each of the annotations in turn
    for (unsigned int i = 0; i < annotationCount; i++)
    {
        RectF annotationRect = GetAnnotationPosition(i, &graphics);
        RectF annotationRectInflated(annotationRect);
        annotationRectInflated.Inflate(annotationInflation, annotationInflation);
        graphics.FillRectangle(&annotationBrush, annotationRectInflated);
        Font font(annotationFontFace, annotationFontSize);
        graphics.DrawString(annotations[i].text, -1, &font, annotationRect, stringFormat, &annotationTextBrush);

        // Get the region, and convert it to an array of rectangles
        Region *region = GetLineCharactersPosition(annotations[i].line, annotations[i].start_char, annotations[i].length, &graphics);
        if (region != NULL)
        {
            Matrix identity; // Identity is the default matrix
            UINT scansCount = region->GetRegionScansCount(&identity);
            
            RectF *regionRects = new RectF[scansCount];
            if (regionRects != NULL)
            {
                int count;
                if (region->GetRegionScans(&identity, regionRects, &count) == Ok && count > 0)
                {
                    // First draw the outlines of all of the rectangles
                    graphics.DrawRectangles(&annotationPen, regionRects, count);

                    // Next draw a line from the first rectangle to the annotation rectangle;
                    PointF charactersRightSide(regionRects[0].GetRight(), regionRects[0].GetTop() + regionRects[0].Height / 2.0f);
                    PointF annotationLeftSide(annotationRectInflated.GetLeft(), annotationRectInflated.GetTop() + annotationRectInflated.Height / 2.0f);
                    graphics.DrawLine(&annotationPen, charactersRightSide, annotationLeftSide);
                }
                delete [] regionRects;
            }
            delete region;
        }
        else
        {
            OutputDebugString(L"Failed to get region\n");
        }
    }
}

LRESULT AnnotatedTextControl::OnSize(_In_ WPARAM wParam, _In_ LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int height = GetDesiredHeight();
    maxScroll = max(height - clientRect.bottom, 0);
    currentScroll = min(maxScroll, currentScroll);

    SCROLLINFO si;
    si.cbSize = sizeof(si); 
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = 0; 
    si.nMax   = maxScroll > 0 ? height : 0;
    si.nPage  = clientRect.bottom;
    si.nPos   = currentScroll; 

    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    return 0;
}

LRESULT AnnotatedTextControl::OnScroll(_In_ WPARAM wParam, _In_ LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask  = SIF_TRACKPOS; 
    GetScrollInfo(hwnd, SB_VERT, &si);

    int newScrollPosition;    // new position 

    switch (LOWORD(wParam)) 
    { 
        // User clicked the scroll bar shaft above the scroll box. 
    case SB_PAGEUP: 
        newScrollPosition = currentScroll - 50; 
        break; 

        // User clicked the scroll bar shaft below the scroll box. 
    case SB_PAGEDOWN: 
        newScrollPosition = currentScroll + 50; 
        break; 

        // User clicked the top arrow. 
    case SB_LINEUP: 
        newScrollPosition = currentScroll - 5; 
        break; 

        // User clicked the bottom arrow. 
    case SB_LINEDOWN: 
        newScrollPosition = currentScroll + 5; 
        break; 

        // User dragged the scroll box. 
    case SB_THUMBPOSITION: 
        newScrollPosition = si.nTrackPos; 
        break; 

    default: 
        newScrollPosition = currentScroll; 
    } 

    // New position must be between 0 and the screen height. 
    newScrollPosition = max(0, newScrollPosition); 
    newScrollPosition = min(maxScroll, newScrollPosition); 

    // If the current position does not change, do not scroll.
    if (newScrollPosition != currentScroll) 
    {
        int yDelta = newScrollPosition - currentScroll;

        // Reset the current scroll position. 
        currentScroll = newScrollPosition; 

        // Scroll the window. (The system repaints most of the 
        // client area when ScrollWindowEx is called; however, it is 
        // necessary to call UpdateWindow in order to repaint the 
        // rectangle of pixels that were invalidated.) 
        ScrollWindowEx(hwnd, -0, -yDelta, (CONST RECT *) NULL, 
            (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, 
            SW_ERASE | SW_INVALIDATE ); 
        UpdateWindow(hwnd); 

        // Reset the scroll bar. 
        si.cbSize = sizeof(si); 
        si.fMask  = SIF_POS; 
        si.nPos   = currentScroll; 
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
    }
    return 0;
}

LRESULT AnnotatedTextControl::OnSetFocus()
{
    RectF caretScreenPos = GetCaretScreenPosition();
    // Create the Caret
    CreateCaret(hwnd, (HBITMAP) NULL, 0 /* Use the default width */, static_cast<int>(caretScreenPos.Height));
    SetCaretPos(static_cast<int>(caretScreenPos.X), static_cast<int>(caretScreenPos.Y));
    ShowCaret(hwnd);
    isActive = true;
    return 0; 
}

LRESULT AnnotatedTextControl::OnKillFocus()
{
    DestroyCaret();
    isActive = false;
    return 0;
}

LRESULT AnnotatedTextControl::OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    bool changed = false;
    EndPoint newCaretPos = caretPosition;
   
    switch (wParam) 
    { 
    case VK_HOME:       // Home 
        newCaretPos.line = 0;
        newCaretPos.character = 0;
        changed = true;
        break; 

    case VK_END:        // End 
        newCaretPos.line = GetLineCount() - 1;
        newCaretPos.character = GetLineLength(newCaretPos.line);
        changed = true;
        break;

    case VK_LEFT:       // Left arrow 
        changed = StepCharacter(caretPosition, false, &newCaretPos);
        break; 

    case VK_RIGHT:      // Right arrow 
        changed = StepCharacter(caretPosition, true, &newCaretPos); 
        break; 

    case VK_UP:         // Up arrow 
        // Go to the same position in the previous line
        newCaretPos.line--;
        newCaretPos.line = max(newCaretPos.line, 0);
        newCaretPos.character = min(newCaretPos.character, GetLineLength(newCaretPos.line));
        changed = true;
        break; 

    case VK_DOWN:       // Down arrow 
        // Go to the same position in the next line
        newCaretPos.line++;
        newCaretPos.line = min(newCaretPos.line, GetLineCount() - 1);
        newCaretPos.character = min(newCaretPos.character, GetLineLength(newCaretPos.line));
        changed = true;
        break;

    default:
        break;
    }

    if (changed)
    {
        UpdateCaret(newCaretPos);
    }

    return 0;
}

LRESULT AnnotatedTextControl::OnLButtonDown(_In_ WPARAM wParam, _In_ LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    float xPos = static_cast<float>(LOWORD(lParam)); 
    float yPos = static_cast<float>(HIWORD(lParam)); 
    EndPoint click = SearchForClosestEndPoint(xPos, yPos);
    UpdateCaret(click);
    return 0;
}

float AnnotatedTextControl::GetTextAreaWidth()
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // We split the area, 2/3 for text, 1/3 for annotations
    return (2.0f * clientRect.right) / 3.0f;
}

SizeF AnnotatedTextControl::GetLineDimensions(_In_ int line, _In_ Graphics *graphics)
{
    if (line < 0 || line >= lineCount)
    {
        return SizeF();
    }

    StyleDefinition* styleDef = GetStyleDefinition(lines[line].style);
    Font font(textFontFace, styleDef->fontSize, styleDef->fontStyle);
    SizeF layoutSize(GetTextAreaWidth() - styleDef->indent, 1200.0f);
    const StringFormat* stringFormat = StringFormat::GenericDefault();
    SizeF lineSize;
    Status status = graphics->MeasureString(lines[line].text, -1, &font, layoutSize, stringFormat, &lineSize);

    if (status != Ok)
    {
        lineSize = SizeF();
    }

    return lineSize;
}

RectF AnnotatedTextControl::GetLinePosition(_In_ int line, _In_ Graphics *graphics)
{
    if (line < 0 || line >= lineCount)
    {
        return RectF();
    }

    float yPos = 0.0f;

    for(int i = 0; i < line; i++)
    {
        SizeF lineSize = GetLineDimensions(i, graphics);
        yPos += lineSize.Height;
        StyleDefinition* styleDef = GetStyleDefinition(lines[i].style);
        yPos += styleDef->verticalSpacing;
    }

    SizeF thisLineSize = GetLineDimensions(line, graphics);
    StyleDefinition* thisStyleDef = GetStyleDefinition(lines[line].style);
    PointF origin(thisStyleDef->indent, yPos + verticalPadding - currentScroll);
    return RectF(origin, thisLineSize);
}

Region * AnnotatedTextControl::GetLineCharactersPosition(_In_ int line, _In_ int first, _In_ int length, _In_ Graphics *graphics)
{
    if (line < 0 || line >= lineCount)
    {
        OutputDebugString(L"GetLineCharactersPosition: Bad line input\n");
        return NULL;
    }

    int strLength = GetLineLength(line);
    if (first < 0 || first > strLength)
    {
        OutputDebugString(L"GetLineCharactersPosition: Bad first char\n");
        return NULL;
    }

    // If the length either goes past the end of the line or is -1 then set it to the end of the line
    if (length == -1 || first + length > static_cast<int>(strLength))
    {
        length = strLength - first;
    }

    RectF boundingRect = GetLinePosition(line, graphics);

    CharacterRange range(first, length);
    StringFormat format;
    format.SetMeasurableCharacterRanges(1, &range);

    StyleDefinition* styleDef = GetStyleDefinition(lines[line].style);
    Font font(textFontFace, styleDef->fontSize, styleDef->fontStyle);

    // Get the number of ranges that have been set, and allocate memory to 
    // store the regions that correspond to the ranges.
    Region *charRangeRegion = new Region;
    if (charRangeRegion != NULL)
    {
        Status status = graphics->MeasureCharacterRanges(lines[line].text, -1, &font, boundingRect, &format, 1, charRangeRegion);
        if (status == Ok)
        {
            return charRangeRegion;
        }
        delete charRangeRegion;
    }
    return NULL;
}


SizeF AnnotatedTextControl::GetAnnotationDimensions(_In_ unsigned int annotation, _In_ Graphics *graphics)
{
    if (annotation >= annotationCount)
    {
        return SizeF();
    }
    
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    Font font(annotationFontFace, annotationFontSize);
    SizeF layoutSize(clientRect.right - GetTextAreaWidth() - annotationPadding * 2.0f, 1200.0f);
    const StringFormat* stringFormat = StringFormat::GenericDefault();
    SizeF annotationSize;
    Status status = graphics->MeasureString(annotations[annotation].text, -1, &font, layoutSize, stringFormat, &annotationSize);

    if (status != Ok)
    {
        annotationSize = SizeF();
    }
    return annotationSize;
}

RectF AnnotatedTextControl::GetAnnotationPosition(_In_ unsigned int annotation, _In_ Graphics *graphics)
{
    if (annotation >= annotationCount)
    {
         return RectF();
    }

    float yPos = annotationPadding;

    for(unsigned int i = 0; i < annotation; i++)
    {
        SizeF annotationSize = GetAnnotationDimensions(i, graphics);
        yPos += annotationSize.Height;
        yPos += (annotationPadding * 2.0f);
    }

    SizeF thisLineSize = GetAnnotationDimensions(annotation, graphics);
    PointF origin(GetTextAreaWidth() + annotationPadding, yPos + verticalPadding - currentScroll);
    return RectF(origin, thisLineSize);
}

int AnnotatedTextControl::GetLineCount()
{
    return lineCount;
}

_Post_equal_to_(this->annotationCount)
unsigned int AnnotatedTextControl::GetAnnotationCount() const
{
    return annotationCount;
}

TextLine *AnnotatedTextControl::GetLine(_In_ int line)
{
    if (line < 0 || line >= lineCount)
    {
        return NULL;
    }
    return &lines[line];
}

_When_(annotation < this->annotationCount, _Post_equal_to_(&this->annotations[annotation]))
_When_(annotation >= this->annotationCount, _Ret_null_)
Annotation *AnnotatedTextControl::GetAnnotation(_In_ unsigned int annotation) const
{
    if (annotation >= annotationCount)
    {
        return NULL;
    }
    return &annotations[annotation];
}

int AnnotatedTextControl::GetDesiredHeight()
{
    Graphics graphics(hwnd);
    RectF lastText = GetLinePosition(lineCount - 1, &graphics);
    RectF lastAnnotation = GetAnnotationPosition(annotationCount - 1, &graphics);
    return static_cast<int>(max(lastText.GetBottom(), lastAnnotation.GetBottom()) + currentScroll + verticalPadding);
}

EndPoint AnnotatedTextControl::SearchForClosestEndPoint(_In_ float x, _In_ float y)
{
    Graphics graphics(hwnd);
    // first find the closest line
    int beforeLine = -1;
    int inLine = -1;
    for (int i = 0; i < lineCount; i++)
    {
        RectF lineRect = GetLinePosition(i, &graphics);
        if (y < lineRect.Y)
        {
            beforeLine = i;
            break;
        }
        else if (y < lineRect.GetBottom())
        {
            inLine = i;
            break;
        }
    }

    if (beforeLine >= 0)
    {
        // The point lies between two lines
        // the position will be the start of the next line

        EndPoint ret = {beforeLine, 0};
        return ret;
    }
    else if (inLine >= 0)
    {
        // The point is in a specific line, now we need to iterate
        // and find out between which two characters
        int length = GetLineLength(inLine);

        // First get all the bounding rects for the line
        // Get the region, and convert it to an array of rectangles
        RectF inRect;
        Region *region = GetLineCharactersPosition(inLine, 0, length, &graphics);
        if (region != NULL)
        {
            Matrix identity; // Identity is the default matrix
            UINT scansCount = region->GetRegionScansCount(&identity);
            
            RectF *regionRects = new RectF[scansCount];
            if (regionRects != NULL)
            {
                int count;
                if (region->GetRegionScans(&identity, regionRects, &count) == Ok && count > 0)
                {
                    int beforeOrInRect = -1;
                    for (int i = 0; i < count; i++)
                    {
                        if (y < regionRects[i].GetBottom())
                        {
                            beforeOrInRect = i;
                            break;
                        }
                    }
                    
                    // If it's past the end, it must be close to the end
                    if (beforeOrInRect < 0)
                    {
                        beforeOrInRect = count - 1;
                    }

                    inRect = regionRects[beforeOrInRect];
                }
                delete [] regionRects;
            }
            delete region;
        }


        if (inRect.IsEmptyArea())
        {
            EndPoint ret = {inLine, 0};
            return ret;
        }

        // Essentially do a binary search through the line
        int minSearch = 0;
        int maxSearch = length;
        int searchPoint = length / 2;

        // As a binary search, this should take at most Log_2(n)
        // so even for a really long (10000 character) string
        // this should only take about 14 iterations, if it passes
        // 20 it's likely an error
        for (int i = 0; i < 20; i++)
        {
            int direction = 0;
            region = GetLineCharactersPosition(inLine, searchPoint, 1, &graphics);
            if (region != NULL)
            {
                RectF character;
                if (region->GetBounds(&character, &graphics) == Ok)
                {
                    // First check that we've found the right line
                    if (inRect.Contains(character))
                    {
                        if (x < character.X)
                        {
                            direction = -1;
                        }
                        else if (x > character.GetRight())
                        {
                            direction = 1;
                        }
                    }
                    else
                    {
                        if ( character.Y < inRect.Y )
                        {
                            direction = 1;
                        }
                        else
                        {
                            direction = -1;
                        }
                    }
                }
                delete region;
            }
            
            int distance = 0;
            if (direction == 1)
            {
                minSearch = searchPoint;
                distance = (maxSearch - searchPoint) / 2;
            }
            else if (direction == -1)
            {
                maxSearch = searchPoint;
                distance = (searchPoint - minSearch) / 2;
            }
            else
            {
                // If we didn't move, either it errored out or
                // we hit tested to a specific character. In either
                // case, searchPoint is our final result
                break;
            }
            distance = max(distance, 1);
            distance = distance * direction;

            int newPoint = searchPoint + distance;
            // The point is between two characters, set it to the second one
            // since the address is actually before a specific character
            if (newPoint == maxSearch)
            {
                searchPoint = maxSearch;
                break;
            }
            else if (newPoint == minSearch)
            {
                break;
            }

            if (newPoint < 0)
            {
                // The point is before the start
                searchPoint = 0;
                break;
            }

            if (newPoint >= length)
            {
                // The point is past the end
                searchPoint = length;
                break;
            }

            // We haven't found it yet,
            // keep iterating
            searchPoint = newPoint;
        }

        EndPoint ret = {inLine, searchPoint};
        return ret;
    }
    else
    {
        return GetEnd();
    }
}

EndPoint AnnotatedTextControl::GetEnd()
{
    EndPoint ep = {lineCount - 1, 0 };
    ep.character = GetLineLength(ep.line);
    return ep;
}

int AnnotatedTextControl::GetLineLength(_In_ int line)
{
    size_t strLength;
    if (FAILED(StringCchLength(lines[line].text, 10000, &strLength)))
    {
        strLength = 0;
    }
    return static_cast<int>(strLength);
}

VARIANT AnnotatedTextControl::GetAttributeAtPoint(_In_ EndPoint start, _In_ TEXTATTRIBUTEID attribute)
{
    VARIANT retval;
    VariantInit(&retval);

    // Many attributes are constant across the range, get them here
    if ( attribute == UIA_AnimationStyleAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = AnimationStyle_None;
    }
    else if ( attribute == UIA_BackgroundColorAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = GetSysColor(COLOR_WINDOW);
    }
    else if ( attribute == UIA_BulletStyleAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = BulletStyle_None;
    }
    else if ( attribute == UIA_CapStyleAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = CapStyle_None;
    }
    else if ( attribute == UIA_CultureAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = GetThreadLocale();
    }
    else if ( attribute == UIA_FontNameAttributeId )
    {
        retval.bstrVal = SysAllocString(textFontFace);
        if (retval.bstrVal != NULL)
        {
            retval.vt = VT_BSTR;
        }
    }
    else if ( attribute == UIA_ForegroundColorAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = textColor.ToCOLORREF();
    }
    else if ( attribute == UIA_HorizontalTextAlignmentAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = HorizontalTextAlignment_Left;
    }
    else if ( attribute == UIA_IndentationTrailingAttributeId )
    {
        retval.vt = VT_R8;
        retval.dblVal = 0.0;
    }
    else if ( attribute == UIA_IsHiddenAttributeId )
    {
        retval.vt = VT_BOOL;
        retval.boolVal = VARIANT_FALSE;
    }
    else if ( attribute == UIA_IsReadOnlyAttributeId )
    {
        retval.vt = VT_BOOL;
        retval.boolVal = VARIANT_TRUE;
    }
    else if ( attribute == UIA_IsSubscriptAttributeId )
    {
        retval.vt = VT_BOOL;
        retval.boolVal = VARIANT_FALSE;
    }
    else if ( attribute == UIA_IsSuperscriptAttributeId )
    {
        retval.vt = VT_BOOL;
        retval.boolVal = VARIANT_FALSE;
    }
    else if ( attribute == UIA_MarginBottomAttributeId )
    {
        retval.vt = VT_R8;
        retval.dblVal = verticalPadding;
    }
    else if ( attribute == UIA_MarginLeadingAttributeId )
    {
        retval.vt = VT_R8;
        retval.dblVal = 0.0;
    }
    else if ( attribute == UIA_MarginTopAttributeId )
    {
        retval.vt = VT_R8;
        retval.dblVal = verticalPadding;
    }
    else if ( attribute == UIA_MarginTrailingAttributeId )
    {
        retval.vt = VT_R8;
        retval.dblVal = 0.0;
    }
    else if ( attribute == UIA_OutlineStylesAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = OutlineStyles_None;
    }
    else if ( attribute == UIA_OverlineColorAttributeId )
    {
        if (SUCCEEDED(UiaGetReservedNotSupportedValue(&retval.punkVal)))
        {
            retval.vt = VT_UNKNOWN;
        }
    }
    else if ( attribute == UIA_OverlineStyleAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = TextDecorationLineStyle_None;
    }
    else if ( attribute == UIA_StrikethroughColorAttributeId )
    {
        if (SUCCEEDED(UiaGetReservedNotSupportedValue(&retval.punkVal)))
        {
            retval.vt = VT_UNKNOWN;
        }
    }
    else if ( attribute == UIA_StrikethroughStyleAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = TextDecorationLineStyle_None;
    }
    else if ( attribute == UIA_TabsAttributeId )
    {
        if (SUCCEEDED(UiaGetReservedNotSupportedValue(&retval.punkVal)))
        {
            retval.vt = VT_UNKNOWN;
        }
    }
    else if ( attribute == UIA_UnderlineColorAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = textColor.ToCOLORREF();
    }
    else if ( attribute == UIA_TextFlowDirectionsAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = FlowDirections_RightToLeft;
    }
    else if ( attribute == UIA_LinkAttributeId )
    {
        if (SUCCEEDED(UiaGetReservedNotSupportedValue(&retval.punkVal)))
        {
            retval.vt = VT_UNKNOWN;
        }
    }
    else if ( attribute == UIA_IsActiveAttributeId )
    {
        retval.vt = VT_BOOL;
        retval.boolVal = isActive ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if ( attribute == UIA_SelectionActiveEndAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = ActiveEnd_None;
    }
    else if ( attribute == UIA_CaretPositionAttributeId )
    {
        retval.vt = VT_I4;
        if (caretPosition.character == 0)
        {
            retval.lVal = CaretPosition_BeginningOfLine;
        }
        else if (caretPosition.character == GetLineLength(caretPosition.line))
        {
            retval.lVal = CaretPosition_EndOfLine;
        }
        else
        {
            retval.lVal = CaretPosition_Unknown;
        }
    }
    else if ( attribute == UIA_CaretBidiModeAttributeId )
    {
        retval.vt = VT_I4;
        retval.lVal = CaretBidiMode_LTR;
    }
    else if ( attribute == UIA_AnnotationTypesAttributeId ||
              attribute == UIA_AnnotationObjectsAttributeId )
    {
        // Do nothing, we'll handle this at a different level
    }
    else
    {
        StyleDefinition* style = GetStyleDefinition(lines[start.line].style);
        if ( attribute == UIA_FontSizeAttributeId )
        {
            retval.vt = VT_R8;
            retval.dblVal = style->fontSize;
        }
        else if ( attribute == UIA_FontWeightAttributeId )
        {
            retval.vt = VT_I4;
            retval.lVal = (style->fontStyle & FontStyleBold) == 0 ? 400 : 700;
        }
        else if ( attribute == UIA_IndentationFirstLineAttributeId ||
             attribute == UIA_IndentationLeadingAttributeId )
        {
            retval.vt = VT_R8;
            retval.dblVal = style->indent;
        }
        else if ( attribute == UIA_IsItalicAttributeId )
        {
            retval.vt = VT_BOOL;
            retval.boolVal = (style->fontStyle & FontStyleItalic) == 0 ? VARIANT_FALSE : VARIANT_TRUE;
        }
        else if ( attribute == UIA_UnderlineStyleAttributeId )
        {
            retval.vt = VT_BOOL;
            retval.boolVal = (style->fontStyle & FontStyleUnderline) == 0 ? VARIANT_FALSE : VARIANT_TRUE;
        }
        else if ( attribute == UIA_StyleNameAttributeId )
        {
            retval.bstrVal = SysAllocString(style->uiaStyleName);
            if (retval.bstrVal != NULL)
            {
                retval.vt = VT_BSTR;
            }
        }
        else if ( attribute == UIA_StyleIdAttributeId )
        {
            retval.vt = VT_I4;
            retval.lVal = style->uiaStyleId;
        }
    }
    return retval;
}

RectF AnnotatedTextControl::GetCaretScreenPosition()
{
    Graphics graphics(hwnd);
    bool endOfLine = false;
    if (caretPosition.character >= GetLineLength(caretPosition.line))
    {
        endOfLine = true;
    }
    Region *region = GetLineCharactersPosition(caretPosition.line, caretPosition.character - (endOfLine ? 1 : 0), 1, &graphics);
    if (region != NULL)
    {
        RectF caretRect;
        if (region->GetBounds(&caretRect, &graphics) == Ok)
        {
            if (endOfLine)
            {
                caretRect.X += caretRect.Width;
            }
            caretRect.Width = 0.0f;
            return caretRect;
        }
    }
    return RectF();
}

void AnnotatedTextControl::UpdateCaret(_In_ EndPoint newPosition)
{
    bool lineChange = (caretPosition.line != newPosition.line);
    caretPosition = newPosition;
    RectF caretScreenPos = GetCaretScreenPosition();
    if (lineChange)
    {
        // If we change lines resize the caret to the appropriate new height for the line
        DestroyCaret();
        CreateCaret(hwnd, (HBITMAP) NULL, 0 /* Use the default width */, static_cast<int>(caretScreenPos.Height));
    }

    SetCaretPos(static_cast<int>(caretScreenPos.X), static_cast<int>(caretScreenPos.Y));
    // Fire a UIA event to notify pure UIA clients of the Caret position change
    NotifyCaretPositionChanged(hwnd, this);

    if (lineChange)
    {
        ShowCaret(hwnd);
    }
}

bool AnnotatedTextControl::StepCharacter(_In_ EndPoint start, _In_ bool forward, _Out_ EndPoint *end)
{
    *end = start;
    if (forward)
    {
        if (end->character >= GetLineLength(end->line))
        {
            if (end->line + 1 >= GetLineCount())
            {
                return false;
            }
            end->line++;
            end->character = 0;
        }
        else
        {
            end->character++;
        }
    }
    else
    {
        if (end->character <= 0)
        {
            if (end->line <= 0)
            {
                return false;
            }
            end->line--;
            end->character = GetLineLength(end->line);
        }
        else
        {
            end->character--;
        }
    }
    return true;
}

// This moves forward or backward by line. It target the end of lines, so if
// in the middle of a line, it moves to the end, and if it's at the end of a line
// it moves to the end of the next line.

// When moving backwards it still targets the end of the line, moving to the end of
// the previous line, except when on the first line, where it will move to the
// beginning of the line, as there isn't a previous line.

// This is done so whether we walk forward or backwards, there is a consistent
// span given, from the end of one line, to the end of the next
bool AnnotatedTextControl::StepLine(_In_ EndPoint start, _In_ bool forward, _Out_ EndPoint *end)
{
    *end = start;
    if (forward)
    {
        if (end->character >= GetLineLength(end->line))
        {
            if (end->line + 1 >= GetLineCount())
            {
                return false;
            }
            end->line++;
            end->character = GetLineLength(end->line);
        }
        else
        {
            end->character = GetLineLength(end->line);
        }
    }
    else
    {
        if (end->line <= 0)
        {
            if (end->character <= 0)
            {
                return false;
            }
            end->character = 0;
        }
        else
        {
            end->line--;
            end->character = GetLineLength(end->line);
        }
    }
    return true;
}

bool AnnotatedTextControl::IsActive()
{
    return isActive;
}

EndPoint AnnotatedTextControl::GetCaretPosition()
{
    return caretPosition;
}
