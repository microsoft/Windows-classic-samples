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

IUIAutomation *_automation;

// Maximum Text length we will read
const int maxLength = 10000;
// Maximum number of comments we'll search for
const int maxComments = 25;

// a class used to wrap and access a Comment Element
class Comment
{
public:
    // This takes ownership and responsibility for releasing the element reference
    // So it does not AddRef it, the caller should not Release the element after
    // it has created the Comment object, as it has passed ownership to it.
    Comment(_In_ IUIAutomationElement *element) : _commentElement(element)
    {
        _annotation = NULL;
    }

    ~Comment()
    {
        _commentElement->Release();
        if (_annotation != NULL)
        {
            _annotation->Release();
        }
    }

    bool Compare(_In_opt_ Comment *other)
    {
        bool retVal = false;
        if (other != NULL)
        {
            BOOL compResult;
            HRESULT hr = _automation->CompareElements(_commentElement, other->_commentElement, &compResult);
            if (FAILED(hr))
            {
                wprintf(L"CompareElements failed, HR: 0x%08x\n", hr);
            }
            else
            {
                retVal = (compResult == TRUE);
            }
        }
        return retVal;
    }

    // Should be immediately after constructor, other calls rely on _annotation being set
    HRESULT GetAnnotationPattern()
    {
        HRESULT hr = _commentElement->GetCurrentPatternAs(UIA_AnnotationPatternId, IID_PPV_ARGS(&_annotation));
        if (FAILED(hr))
        {
            wprintf(L"Failed to Get Annotation Pattern, HR: 0x%08x\n", hr);
        }
        else if (_annotation == NULL)
        {
            wprintf(L"Element does not actually support Annotation Pattern\n");
            hr = E_FAIL;
        }
        return hr;
    }

    HRESULT RangeFromAnnotation(_In_ IUIAutomationTextPattern2 *textPattern, _Outptr_result_maybenull_ IUIAutomationTextRange **range)
    {
        HRESULT hr = textPattern->RangeFromAnnotation(_commentElement, range);
        if (FAILED(hr))
        {
            wprintf(L"RangeFromAnnotation failed, HR: 0x%08x\n", hr);
        }
        return hr;
    }

    void Print(_In_ bool summary)
    {
        BSTR name;
        HRESULT hr = _commentElement->get_CurrentName(&name);
        if (FAILED(hr))
        {
            wprintf(L"Failed to Get Name Property, HR: 0x%08x\n", hr);
        }
        else
        {
            if (summary)
            {
                wprintf(L"\"%30s\"\n", name);
            }
            else
            {
                BSTR typeName;
                hr = _annotation->get_CurrentAnnotationTypeName(&typeName);
                if (FAILED(hr))
                {
                    wprintf(L"Failed to Get AnnotationTypeName Property, HR: 0x%08x\n", hr);
                }
                else
                {
                    BSTR author;
                    hr = _annotation->get_CurrentAuthor(&author);
                    if (FAILED(hr))
                    {
                        wprintf(L"Failed to Get Author Property, HR: 0x%08x\n", hr);
                    }
                    else
                    {
                        BSTR dateTime;
                        hr = _annotation->get_CurrentDateTime(&dateTime);
                        if (FAILED(hr))
                        {
                            wprintf(L"Failed to Get DateTime Property, HR: 0x%08x\n", hr);
                        }
                        else
                        {
                            wprintf(L"Type: %s\nAuthor: %s\nDate/Time: %s\n%s\n", typeName, author, dateTime, name);
                            SysFreeString(dateTime);
                        }
                        SysFreeString(author);
                    }
                    SysFreeString(typeName);
                }
            }
            SysFreeString(name);
        }
    }
private:
    IUIAutomationElement* _commentElement;
    IUIAutomationAnnotationPattern* _annotation;
};

// A class used to wrap and access a Text Range
class Range
{
public:
    // This takes ownership and responsibility for releasing the range reference
    // So it does not AddRef it, the caller should not Release the range after
    // it has created the range object, as it has passed ownership to it.
    Range(_In_opt_ IUIAutomationTextRange *range)
    {
        _range = range;
    }

    ~Range()
    {
        if (_range != NULL)
        {
            _range->Release();
        }
    }

    // Assumes that _range is NOT null
    void FindAndPrintStyle(_In_ int style)
    {
        IUIAutomationTextRange *remainingRange;
        HRESULT hr = _range->Clone(&remainingRange);
        if (FAILED(hr))
        {
            wprintf(L"Clone failed on range, HR: 0x%08x\n", hr);
        }
        else
        {
            VARIANT styleVar;
            styleVar.vt = VT_I4;
            styleVar.lVal = style;
            while (SUCCEEDED(hr))
            {
                IUIAutomationTextRange *foundRange;
                hr = remainingRange->FindAttribute(UIA_StyleIdAttributeId, styleVar, FALSE, &foundRange);
                if (FAILED(hr))
                {
                    wprintf(L"FindAttribute failed, HR: 0x%08x\n", hr);
                }
                else
                {
                    if (foundRange != NULL)
                    {
                        BSTR text;
                        hr = foundRange->GetText(maxLength, &text);
                        if (FAILED(hr))
                        {
                            wprintf(L"GetText failed, HR: 0x%08x\n", hr);
                        }
                        else
                        {
                            wprintf(L" - \"%s\"\n", text);
                            SysFreeString(text);
                        }

                        // Next, to continue the loop, we move the beginning endpoint of our working range (remainingRange), forward
                        // to the ending point of the last range we found. This gives us a range containing all the text we haven't
                        // searched yet.
                        hr = remainingRange->MoveEndpointByRange(TextPatternRangeEndpoint_Start, foundRange, TextPatternRangeEndpoint_End);
                        foundRange->Release();
                        if (FAILED(hr))
                        {
                            wprintf(L"MoveEndpointByRange failed, HR: 0x%08x\n", hr);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            remainingRange->Release();
        }
    }

    void FindAndPrintHeaders()
    {
        if (_range == NULL)
        {
            wprintf(L"Headers: <None>\n");
        }
        else
        {
            wprintf(L"Title:\n");
            FindAndPrintStyle(StyleId_Title);
            wprintf(L"Heading 1:\n");
            FindAndPrintStyle(StyleId_Heading1);
            wprintf(L"Heading 2:\n");
            FindAndPrintStyle(StyleId_Heading2);
            wprintf(L"Heading 3:\n");
            FindAndPrintStyle(StyleId_Heading3);
        }
    }

    // This is a quick check of whether a specific range (the input), is past the endpoint
    // of this range object (the internal _range of this Range object). It does this by
    // checking whether the start point of the input range is greater than or equal to the
    // endpoint of the internal range.
    bool IsThisRangePastMyEndpoint(_In_ IUIAutomationTextRange *range)
    {
        bool retVal = true;
        int compare;
        HRESULT hr = _range->CompareEndpoints(TextPatternRangeEndpoint_End, range, TextPatternRangeEndpoint_Start, &compare);
        if (FAILED(hr))
        {
            // If this call fails, assume we're past the end
            wprintf(L"CompareEndpoints failed on range, HR: 0x%08x\n", hr);
        }
        else
        {
            retVal = compare <= 0;
        }
        return retVal;
    }

    // This will extract the comments from an IUIAutomationElementArray, adding any new ones to the comments array
    HRESULT GetNewCommentsFromArray(_In_ IUIAutomationElementArray *commentElements, _Inout_updates_to_(commentCount, *foundCommentCount) Comment **comments,
        _In_ int commentCount, _Inout_ int *foundCommentCount)
    {
        HRESULT hr = S_OK;
        int count;
        hr = commentElements->get_Length(&count);
        if (FAILED(hr))
        {
            wprintf(L"get_Length failed on element array, HR: 0x%08x\n", hr);
        }
        else
        {
            for (int i = 0; i < count && SUCCEEDED(hr); i++)
            {
                IUIAutomationElement *element;
                hr = commentElements->GetElement(i, &element);
                if (FAILED(hr) || element == NULL)
                {
                    wprintf(L"GetElement failed on element array, HR: 0x%08x\n", hr);
                }
                else
                {
                    bool addedComment = false;
                    Comment *newComment = new Comment(element);
                    if (newComment == NULL)
                    {
                        wprintf(L"Not enough memory to create a new Comment\n");
                        // Comment failed to be created, so did not take ownership of element
                        element->Release();
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = newComment->GetAnnotationPattern();
                        if (SUCCEEDED(hr))
                        {
                            bool found = false;
                            // check if the comment is already in the array
                            for (int j = 0; j < *foundCommentCount; j++)
                            {
                                if (newComment->Compare(comments[j]))
                                {
                                    found = true;
                                    break;
                                }
                            }

                            // If it's not there, add it to the list
                            if (!found)
                            {
                                if ((*foundCommentCount) < commentCount)
                                {
                                    // The array takes ownership of the comment at this point
                                    comments[*foundCommentCount] = newComment;
                                    addedComment = true;
                                    (*foundCommentCount)++;
                                }
                            }
                        }
                        if (!addedComment)
                        {
                            delete newComment;
                        }
                    }
                }
                _Analysis_assume_((*foundCommentCount) <= commentCount);
            }
        }
       
        return hr;
    }

    // This call will move the range argument, so don't pass in a range you don't want moved, clone it first
    HRESULT WalkForCommentsInRange(_In_ IUIAutomationTextRange *range, _Inout_updates_to_(commentCount, *foundCommentCount) Comment **comments,
        _In_ int commentCount, _Inout_ int *foundCommentCount)
    {
        HRESULT hr = S_OK;

        // These are singleton objects so don't need to be released
        IUnknown *notSupported = NULL;
        IUnknown *mixedAttribute = NULL;

        hr = _automation->get_ReservedNotSupportedValue(&notSupported);
        if (FAILED(hr))
        {
            wprintf(L"get_ReservedNotSupportedValue failed, HR: 0x%08x\n", hr);
        }
        else
        {
            hr = _automation->get_ReservedMixedAttributeValue(&mixedAttribute);
            if (FAILED(hr))
            {
                wprintf(L"get_ReservedMixedAttributeValue failed, HR: 0x%08x\n", hr);
            }
        }

        // A way to prevent this from running into a loop if a provider doesn't implement Move properly
        int sanityCount = 100;

        while (!IsThisRangePastMyEndpoint(range) && SUCCEEDED(hr) && sanityCount-- > 0)
        {
            // Get the list of comments on the current range
            VARIANT varComments;
            hr = range->GetAttributeValue(UIA_AnnotationObjectsAttributeId, &varComments);
            if (FAILED(hr))
            {
                wprintf(L"GetAttributeValue failed on range, HR: 0x%08x\n", hr);
            }
            else
            {
                if (varComments.vt != VT_EMPTY && varComments.vt != VT_UNKNOWN )
                {
                    wprintf(L"Unexpected Type for AnnotationObjectsAttribute, vt: 0x%08x\n", varComments.vt);
                    hr = E_FAIL;
                }
                else if (varComments.vt == VT_UNKNOWN)
                {
                    if (varComments.punkVal == notSupported)
                    {
                        wprintf(L"AnnotationObjectsAttribute not supported on this Range\n");
                        hr = E_FAIL;
                    }
                    else if (varComments.punkVal == mixedAttribute)
                    {
                        // Since we're walking by format, mixed Attribute value should never come up
                        wprintf(L"Unexpected Mixed Attribute Value\n");
                        hr = E_FAIL;
                    }
                    else
                    {
                        IUIAutomationElementArray *commentElements;
                        hr = varComments.punkVal->QueryInterface(IID_PPV_ARGS(&commentElements));
                        if (FAILED(hr))
                        {
                            wprintf(L"Could not QI to IUIAutomationElementArray, HR: 0x%08x\n", hr);
                        }
                        else
                        {
                            hr = GetNewCommentsFromArray(commentElements, comments, commentCount, foundCommentCount);
                            commentElements->Release();
                        }
                    }
                }
                VariantClear(&varComments);
            }

            // If nothing has gone wrong, advance the range
            if (SUCCEEDED(hr))
            {
                int moveCount;
                hr = range->Move(TextUnit_Format, 1, &moveCount);
                if (FAILED(hr))
                {
                    wprintf(L"Move by format failed on range, HR: 0x%08x\n", hr);
                }
                else if (moveCount != 1)
                {
                    wprintf(L"Failed to advance forward for some reason, terminating loop\n");
                    hr = E_FAIL;
                }
            }
        }
        return hr;
    }

    HRESULT FindCommentsInRange(_Out_writes_to_(commentCount, *foundCommentCount) Comment **comments, _In_ int commentCount, _Out_ int *foundCommentCount)
    {
        *foundCommentCount = 0;
        IUIAutomationTextRange *movingRange;
        HRESULT hr = _range->Clone(&movingRange);
        if (FAILED(hr))
        {
            wprintf(L"Clone failed on range, HR: 0x%08x\n", hr);
        }
        else
        {
            // First collapse the range to the starting endpoint by moving the end endpoint back by the whole document
            hr = movingRange->MoveEndpointByRange(TextPatternRangeEndpoint_End, movingRange, TextPatternRangeEndpoint_Start);
            if (FAILED(hr))
            {
                wprintf(L"MoveEndpointByUnit failed on range, HR: 0x%08x\n", hr);
            }
            else
            {
                hr = movingRange->ExpandToEnclosingUnit(TextUnit_Format);
                if (FAILED(hr))
                {
                    wprintf(L"ExpandToEnclosingUnit failed on range, HR: 0x%08x\n", hr);
                }
                else
                {
                    // Now walk through in Format units, getting the annotations
                    hr = WalkForCommentsInRange(movingRange, comments, commentCount, foundCommentCount);
                }
            }
            movingRange->Release();
        }
        return hr;
    }

    void FindAndPrintComments()
    {
        if (_range == NULL)
        {
            wprintf(L"Comments: <None>\n");
        }
        else
        {
            wprintf(L"Comments:\n");
            Comment **comments = new Comment*[maxComments];
            if (comments == NULL)
            {
                wprintf(L"Not enough memory to create a Comment Array\n");
            }
            else
            {
                int foundComments;
                HRESULT hr = FindCommentsInRange(comments, maxComments, &foundComments);
                if (SUCCEEDED(hr))
                {
                    for (int i = 0; i < foundComments; i++)
                    {
                        wprintf(L"%2d: ", i);
                        comments[i]->Print(true);
                    }

                    for (int i = 0; i < foundComments; i++)
                    {
                        delete comments[i];
                    }
                }
                delete [] comments;
            }
        }
    }

    void SelectComment(_In_ int commentNum, _Outptr_result_maybenull_ Comment **comment)
    {
        *comment = NULL;

        if (_range != NULL)
        {
            Comment **comments = new Comment*[maxComments];
            if (comments == NULL)
            {
                wprintf(L"Not enough memory to create a Comment Array\n");
            }
            else
            {
                int foundComments;
                HRESULT hr = FindCommentsInRange(comments, maxComments, &foundComments);
                if (SUCCEEDED(hr))
                {
                    if (commentNum >= 0 && commentNum < foundComments)
                    {
                        *comment = comments[commentNum];
                        comments[commentNum] = NULL;
                    }

                    for (int i = 0; i < foundComments; i++)
                    {
                        if (i != commentNum)
                        {
                            delete comments[i];
                        }
                    }
                }
                delete [] comments;
            }
        }
    }

    void PrintCaretInfo()
    {
        if (_range == NULL)
        {
            wprintf(L"No Caret Range\n");
        }
        else
        {
           
            VARIANT var;
            HRESULT hr = _range->GetAttributeValue(UIA_IsActiveAttributeId, &var);
            if (FAILED(hr))
            {
                wprintf(L"GetAttributeValue on UIA_IsActiveAttributeId failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (var.vt != VT_BOOL)
                {
                    wprintf(L"Unexpected variant type for UIA_IsActiveAttributeId, vt: 0x%08x\n", var.vt);
                }
                else
                {
                    if (var.boolVal == VARIANT_TRUE)
                    {
                        wprintf(L"Active\n");
                    }
                    else
                    {
                        wprintf(L"Inactive\n");
                    }
                }
            }
           
            hr = _range->GetAttributeValue(UIA_CaretPositionAttributeId, &var);
            if (FAILED(hr))
            {
                wprintf(L"GetAttributeValue on UIA_CaretPositionAttributeId failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (var.vt == VT_UNKNOWN)
                {
                    wprintf(L"This provider does not support UIA_CaretPositionAttributeId\n");
                }
                else if (var.vt != VT_I4)
                {
                    wprintf(L"Unexpected variant type for UIA_CaretPositionAttributeId, vt: 0x%08x\n", var.vt);
                }
                else
                {
                    // The caret Position attribute is only guaranteed to be accurate when the caret is at
                    // the beginning or end of a line... otherwise, it is indeterminate.
                    if (var.lVal == CaretPosition_EndOfLine)
                    {
                        wprintf(L"Position is at the end of a line\n");
                    }
                    else if (var.lVal == CaretPosition_BeginningOfLine)
                    {
                        wprintf(L"Position is at the beginning of a line\n");
                    }
                    else if (var.lVal == CaretPosition_Unknown)
                    {
                        wprintf(L"Position type is unknown (likely within a line)\n");
                    }
                    else
                    {
                        wprintf(L"Position type is not a known enum value: 0x%08x\n", var.lVal);
                    }
                }
            }

            hr = _range->GetAttributeValue(UIA_CaretBidiModeAttributeId, &var);
            if (FAILED(hr))
            {
                wprintf(L"GetAttributeValue on UIA_CaretBidiModeAttributeId failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (var.vt == VT_UNKNOWN)
                {
                    wprintf(L"This provider does not support UIA_CaretBidiModeAttributeId\n");
                }
                else if (var.vt != VT_I4)
                {
                    wprintf(L"Unexpected variant type for UIA_CaretBidiModeAttributeId, vt: 0x%08x\n", var.vt);
                }
                else
                {
                    if (var.lVal == CaretBidiMode_LTR)
                    {
                        wprintf(L"Text at caret is Left-to-Right reading\n");
                    }
                    else if (var.lVal == CaretBidiMode_RTL)
                    {
                        wprintf(L"Text at caret is Right-to-Left reading\n");
                    }
                    else
                    {
                        wprintf(L"BiDi Mode type is not a known enum value: 0x%08x\n", var.lVal);
                    }
                }
            }

            hr = _range->GetAttributeValue(UIA_SelectionActiveEndAttributeId, &var);
            if (FAILED(hr))
            {
                wprintf(L"GetAttributeValue on UIA_SelectionActiveEndAttributeId failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (var.vt == VT_UNKNOWN)
                {
                    wprintf(L"This provider does not support UIA_SelectionActiveEndAttributeId\n");
                }
                else if (var.vt != VT_I4)
                {
                    wprintf(L"Unexpected variant type for UIA_SelectionActiveEndAttributeId, vt: 0x%08x\n", var.vt);
                }
                else
                {
                    if (var.lVal == ActiveEnd_None)
                    {
                        wprintf(L"The selection has no active end\n");
                    }
                    else if (var.lVal == ActiveEnd_Start)
                    {
                        wprintf(L"Active end of selection is the start of the selection\n");
                    }
                    else if (var.lVal == CaretBidiMode_RTL)
                    {
                        wprintf(L"Active end of selection is the end of the selection\n");
                    }
                    else
                    {
                        wprintf(L"Active end value is not a known enum value: 0x%08x\n", var.lVal);
                    }
                }
            }

            SAFEARRAY *psa = NULL;
            hr = _range->GetBoundingRectangles(&psa);
            if (FAILED(hr))
            {
                wprintf(L"GetBoundingRectangles on caret range failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (psa != NULL)
                {
                    RECT *pRectArray = NULL;
                    int cRectCount = 0;

                    hr = _automation->SafeArrayToRectNativeArray(psa, &pRectArray, &cRectCount);
                    if (FAILED(hr))
                    {
                        wprintf(L"SafeArrayToRectNativeArray on caret range rects failed, HR: 0x%08x\n", hr);
                    }
                    else if (pRectArray == NULL)
                    {
                        wprintf(L"SafeArrayToRectNativeArray returned a NULL rect array");
                    }
                    else
                    {
                        // The caret range should never have more than one bounding rect.
                        if (cRectCount > 1)
                        {
                            wprintf(L"Caret range has an unexpected count of bounding rects, count: %d\n", cRectCount);
                        }
                        else if (cRectCount == 0)
                        {
                            wprintf(L"Caret range has no bounding rect\n");
                        }
                        else
                        {
                            wprintf(L"Caret range bounding rect is left: %d, top: %d, right: %d, bottom: %d\n", 
                                pRectArray[0].left, pRectArray[0].top, pRectArray[0].right, pRectArray[0].bottom);
                        }

                        ::CoTaskMemFree(pRectArray);
                    }

                    SafeArrayDestroy(psa);
                }
                else
                {
                    wprintf(L"GetBoundingRectangles on caret range returned no rects.\n");
                }
            }
        }
    }

    void Print(_In_ bool summary)
    {
        if (_range == NULL)
        {
            if (summary)
            {
                wprintf(L"Active Range: Empty [0 characters]\n");
            }
        }
        else
        {
            BSTR text;
            HRESULT hr = _range->GetText(maxLength, &text);
            if (FAILED(hr))
            {
                wprintf(L"GetText failed, HR: 0x%08x\n", hr);
            }
            else
            {
                if (summary)
                {
                    UINT length = SysStringLen(text);
                    WCHAR shortString[40];
                    hr = StringCchCopy(shortString, ARRAYSIZE(shortString), text);
                    if (hr == STRSAFE_E_INSUFFICIENT_BUFFER)
                    {
                        WCHAR tail[] = L"...";
                        int pos = ARRAYSIZE(shortString) - ARRAYSIZE(tail);
                        hr = StringCchCopy(&shortString[pos], ARRAYSIZE(tail), tail);
                    }

                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"Active Range: \"%s\" [%d characters]\n", shortString, length);
                    }
                }
                else
                {
                    wprintf(L"Active Range:\n%s\n", text);
                }
                SysFreeString(text);
            }
        }
    }

private:
    IUIAutomationTextRange *_range;
};

enum CommandId {
    CommandId_Invalid   = -1,
    CommandId_Help      =  0,
    CommandId_Print     =  1,
    CommandId_Document  =  2,
    CommandId_Selection =  3,
    CommandId_Visible   =  4,
    CommandId_Headers   =  5,
    CommandId_Comment   =  6,
    CommandId_Comments  =  7,
    CommandId_CommentRange=8,
    CommandId_PrintComment=9,
    CommandId_Exit      =  10,
    CommandId_Caret     =  11
};

PCWSTR CommandText[] = {
    L"help\n",
    L"print\n",
    L"document\n",
    L"selection\n",
    L"visible\n",
    L"headers\n",
    L"comment ",
    L"comments\n",
    L"commentrange\n",
    L"printcomment\n",
    L"exit\n",
    L"caret\n"
};

class TextPatternExplorer
{
public:
    // This takes ownership and responsibility for releasing the element reference
    // So it does not AddRef it, the caller should not Release the element after
    // it has created the Comment object, as it has passed ownership to it.
    TextPatternExplorer(_In_ IUIAutomationElement *element) : _element(element)
    {
        _textPattern = NULL;
    }

    ~TextPatternExplorer()
    {
        _element->Release();
        if (_textPattern != NULL)
        {
            _textPattern->Release();
        }
    }

    void Run()
    {
        HRESULT hr = GetTextPattern();
        if (SUCCEEDED(hr))
        {
            Welcome();
            Range *currentRange = GetRange(CommandId_Document, NULL);
            Comment *currentComment = NULL;
            while (currentRange != NULL)
            {
                currentRange->Print(true);
                if (currentComment != NULL)
                {
                    wprintf(L"Active Comment:");
                    currentComment->Print(true);
                }
                WCHAR input[40];
                wprintf(L">");
                fgetws(input, ARRAYSIZE(input), stdin);
                CommandId cmdId = GetCommand(input, ARRAYSIZE(input));

                switch(cmdId)
                {
                case CommandId_Help:
                    {
                        Welcome();
                        break;
                    }
                case CommandId_Print:
                    {
                        currentRange->Print(false);
                        break;
                    }
                case CommandId_Document:
                case CommandId_Selection:
                case CommandId_Visible:
                    {
                        delete currentRange;
                        currentRange = GetRange(cmdId, NULL);
                        break;
                    }
                case CommandId_Caret:
                    {
                        wprintf(L"Pausing for 2 seconds to allow you to focus the text control if desired...\n");
                        Sleep(2000);
                        delete currentRange;
                        bool isActive;
                        currentRange = GetRange(cmdId, &isActive);
                        if (currentRange != NULL)
                        {
                            wprintf(L"Got Caret Range (%s):\n", isActive ? L"active" : L"inactive" );
                            currentRange->PrintCaretInfo();
                        }
                        break;
                    }
                case CommandId_Headers:
                    {
                        currentRange->FindAndPrintHeaders();
                        break;
                    }
                case CommandId_Comment:
                    {
                        SelectComment(input, currentRange, &currentComment);
                        break;
                    }
                case CommandId_Comments:
                    {
                        currentRange->FindAndPrintComments();
                        break;
                    }
                case CommandId_CommentRange:
                    {
                        RangeFromComment(currentComment, &currentRange);
                        break;
                    }
                case CommandId_PrintComment:
                    {
                        if (currentComment != NULL)
                        {
                            currentComment->Print(false);
                        }
                        break;
                    }
                case CommandId_Exit:
                    {
                        delete currentRange;
                        currentRange = NULL;
                        break;
                    }
                default:
                    wprintf(L"Invalid command, type help to see a list of valid commands.\n");
                }
            }
        }
    }

private:
    void SelectComment(_In_ PWSTR cmdString, _In_ Range *range, _Inout_ Comment **comment )
    {
        if (*comment != NULL)
        {
            delete *comment;
            *comment = NULL;
        }

        int commentNumber;
        int retVal = swscanf_s(cmdString, L"comment %d", &commentNumber);
        if (retVal == 1)
        {
            range->SelectComment(commentNumber, comment);
        }
    }

    void RangeFromComment(_In_opt_ Comment *comment, _Inout_ Range **range)
    {
        Range *newRange= NULL;
        if (comment == NULL)
        {
            newRange = new Range(NULL);
            if (newRange == NULL)
            {
                wprintf(L"Not enough memory to create a new Range object\n");
            }
        }
        else
        {
            IUIAutomationTextRange *textRange;
            HRESULT hr = comment->RangeFromAnnotation(_textPattern, &textRange);

            if (SUCCEEDED(hr))
            {
                // We can validly get a NULL range, if the comment points to nothing
                newRange = new Range(textRange);
                if (newRange == NULL)
                {
                    wprintf(L"Failed to create internal Range object\n");
                    if (textRange != NULL)
                    {
                        textRange->Release();
                    }
                }
            }
        }

        // Any failure to create a new Range object will result in the old range being kept
        if (newRange != NULL)
        {
            delete *range;
            *range = newRange;
        }
    }

    CommandId GetCommand(_In_reads_(cmdLength) PWSTR cmd, _In_ int cmdLength)
    {
        _wcslwr_s(cmd, cmdLength);
        CommandId ret = CommandId_Invalid;
        for (int i = 0; i < ARRAYSIZE(CommandText); i++)
        {
            size_t commandLength;
            HRESULT hr = StringCchLength(CommandText[i], cmdLength, &commandLength);
            // Compare the strings, and the terminating null
            if (SUCCEEDED(hr))
            {
                if (wcsncmp(CommandText[i], cmd, commandLength) == 0)
                {
                    ret = static_cast<CommandId>(i);
                    break;
                }
            }
        }
        return ret;
    }

    void Welcome()
    {
        wprintf(L"Text Document Explorer:\n\n");
        wprintf(L"Available Commands:\n");
        wprintf(L"    help            Show this help screen\n");
        wprintf(L"    print           Print the text of the current active range\n");
        wprintf(L"    document        Set the active range to the whole document\n");
        wprintf(L"    selection       Set the active range to the current selection\n");
        wprintf(L"    visible         Set the active range to the currently visible text\n");
        wprintf(L"    headers         Find all the header text in the active range and print it\n");
        wprintf(L"    comments        List all the comments in the active range\n");
        wprintf(L"    comment {n}     Set the active comment to comment {n} in the active range\n");
        wprintf(L"    commentrange    Set the active range to the the active comment's range\n");
        wprintf(L"    printcomment    Print the active comment\n");
        wprintf(L"    caret           Gets the caret range and prints some caret specific information\n");
        wprintf(L"    exit            Quit\n");
    }

    Range* GetRange(_In_ CommandId cmd, _Out_opt_ bool *active)
    {
        Range* retVal = NULL;
        if (active != NULL)
        {
            *active = false;
        }

        IUIAutomationTextRange *uiaRange = NULL;
        HRESULT hr = E_FAIL;
        if (cmd == CommandId_Document)
        {
            hr = _textPattern->get_DocumentRange(&uiaRange);
        }
        else if (cmd == CommandId_Caret)
        {
            BOOL isActive;
            hr = _textPattern->GetCaretRange(&isActive, &uiaRange);
            if (SUCCEEDED(hr) && active != NULL)
            {
                *active = !!isActive;
            }
        }
        else
        {
            // These two properties return arrays of ranges, for complicated text patterns
            // for this example we just get the first range from the array
            IUIAutomationTextRangeArray *uiaRangeArray = NULL;
            if (cmd == CommandId_Selection)
            {
                hr = _textPattern->GetSelection(&uiaRangeArray);
            }
            else if (cmd == CommandId_Visible)
            {
                hr = _textPattern->GetVisibleRanges(&uiaRangeArray);
            }

            if (SUCCEEDED(hr) && uiaRangeArray != NULL)
            {
                int length;
                hr = uiaRangeArray->get_Length(&length);
                if (SUCCEEDED(hr) && length > 0)
                {
                    hr = uiaRangeArray->GetElement(0, &uiaRange);
                }
                uiaRangeArray->Release();
            }
        }

        if (FAILED(hr))
        {
             wprintf(L"Failed to Get the Requested Range, HR: 0x%08x\n", hr);
             uiaRange = NULL;
        }

        // We can validly get a NULL range, if the selection is Empty, or nothing is Visible
        // And even if we have an error, we should wrap the NULL so we have a Range object

        // If the Range gets created it takes ownership of the IUIAutomationTextRange object
        retVal = new Range(uiaRange);
        if (retVal == NULL)
        {
            wprintf(L"Failed to create internal Range object\n");
            if (uiaRange != NULL)
            {
                uiaRange->Release();
            }
        }
        return retVal;
    }

    HRESULT GetTextPattern()
    {
        HRESULT hr = _element->GetCurrentPatternAs(UIA_TextPattern2Id, IID_PPV_ARGS(&_textPattern));
        if (FAILED(hr))
        {
             wprintf(L"Failed to Get Text Pattern, HR: 0x%08x\n", hr);
        }
        else if (_textPattern == NULL)
        {
             wprintf(L"Element does not actually support Text Pattern 2\n");
             hr = E_FAIL;
        }
        return hr;
    }


    IUIAutomationTextPattern2 *_textPattern;
    IUIAutomationElement *_element;
};

// Will search an element itself and all its children and descendants for the first element that supports Text Pattern 2
HRESULT FindTextPatternElement(_In_ IUIAutomationElement *element, _Outptr_result_maybenull_ IUIAutomationElement **textElement)
{
    HRESULT hr = S_OK;

    // Create a condition that will be true for anything that supports Text Pattern 2
    IUIAutomationCondition* textPatternCondition;
    VARIANT trueVar;
    trueVar.vt = VT_BOOL;
    trueVar.boolVal = VARIANT_TRUE;
    hr = _automation->CreatePropertyCondition(UIA_IsTextPattern2AvailablePropertyId, trueVar, &textPatternCondition);

    if (FAILED(hr))
    {
        wprintf(L"Failed to CreatePropertyCondition, HR: 0x%08x\n", hr);
    }
    else
    {
        // Actually do the search
        hr = element->FindFirst(TreeScope_Subtree, textPatternCondition, textElement);
        if (FAILED(hr))
        {
            wprintf(L"FindFirst failed, HR: 0x%08x\n", hr);
        }
        else if (*textElement == NULL)
        {
            wprintf(L"No element supporting TextPattern2 found.\n");
            hr = E_FAIL;
        }
        textPatternCondition->Release();
    }

    return hr;
}

void Usage()
{
    wprintf(L"Usage:\n\n");
    wprintf(L"UiaDocumentClient [hwnd]\n\n");
    wprintf(L"Explore the text pattern for the specific [hwnd] (in hex)\n");
    wprintf(L"If no [hwnd] is supplied, it will get the element under the mouse pointer.\n\n");
}

int _cdecl wmain(_In_ int argc, _In_reads_(argc) WCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argv);

    // We only take 0 or 1 argument, since the program name is the first argument this
    // means any arg count more than 2 is wrong.
    if (argc > 2)
    {
        Usage();
    }
    else
    {
        // Initialize COM before using UI Automation
        HRESULT hr = CoInitialize(NULL);
        if (FAILED(hr))
        {
            wprintf(L"CoInitialize failed, HR:0x%08x\n", hr);
        }
        else
        {
            hr = CoCreateInstance(__uuidof(CUIAutomation8), NULL,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_automation));
            if (FAILED(hr))
            {
                wprintf(L"Failed to create a CUIAutomation8, HR: 0x%08x\n", hr);
            }
            else
            {
                IUIAutomationElement *element = NULL;
                if (argc == 1)
                {
                    wprintf( L"Getting element at cursor in 3 seconds...\n" );
                    Sleep(3000);

                    POINT pt;
                    GetCursorPos(&pt);
                    hr = _automation->ElementFromPoint(pt, &element);
                    if (FAILED(hr))
                    {
                        wprintf( L"Failed to ElementFromPoint, HR: 0x%08x\n\n", hr );
                    }
                }
                else if (argc == 2)
                {
                    long hwndAsLong;
                    int ret = swscanf_s(argv[1], L"%x", &hwndAsLong);
                    if (ret != 1)
                    {
                        wprintf( L"The hwnd parameter needs to be a number in hex.\n" );
                        Usage();
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                        HWND hwnd = static_cast<HWND>(LongToHandle(hwndAsLong));
                        if (!IsWindow(hwnd))
                        {
                            wprintf( L"The hwnd specifier 0x%08x is not a valid HWND.\n", hwndAsLong );
                            hr = E_INVALIDARG;
                        }
                        else
                        {
                            hr = _automation->ElementFromHandle(hwnd, &element);
                            if (FAILED(hr))
                            {
                                wprintf( L"Failed to ElementFromHandle, HR: 0x%08x\n\n", hr );
                            }
                        }
                    }
                }

                if (SUCCEEDED(hr) && element != NULL)
                {
                    IUIAutomationElement *textElement = NULL;
                    hr = FindTextPatternElement(element, &textElement);
                    if (SUCCEEDED(hr) && textElement != NULL)
                    {
                        // The TextPatternExplorer takes ownership of the textElement, so no need to release it here
                        TextPatternExplorer explorer(textElement);
                        explorer.Run();
                    }
                    element->Release();
                }
                _automation->Release();
            }
            CoUninitialize();
        }
    }

    return 0;
}
