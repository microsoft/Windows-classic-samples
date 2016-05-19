//
// globals.cpp
//
// Global variables.
//

#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1; // -1 /w no refs, for win95 InterlockedIncrement/Decrement compat

CRITICAL_SECTION g_cs;

/* 6565d455-5030-4c0f-8871-83f6afde514f */
const CLSID c_clsidCaseTextService = { 0x6565d455, 0x5030, 0x4c0f, {0x88, 0x71, 0x83, 0xf6, 0xaf, 0xde, 0x51, 0x4f} };

/* 4d5459db-7543-42c0-9204-9195b91f6fb8 */
const GUID c_guidCaseProfile = { 0x4d5459db, 0x7543, 0x42c0, {0x92, 0x04, 0x91, 0x95, 0xb9, 0x1f, 0x6f, 0xb8} };

/* 01679c88-5141-4ee5-a47f-c8d586ff37e1 */
const GUID c_guidLangBarItemButton = { 0x01679c88, 0x5141, 0x4ee5, {0xa4, 0x7f, 0xc8, 0xd5, 0x86, 0xff, 0x37, 0xe1} };

//+---------------------------------------------------------------------------
//
// ToggleChar
//
// Toggle the case of a single char.
//----------------------------------------------------------------------------

WCHAR ToggleChar(WCHAR ch)
{
    // toggle english ascii
    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z'))
    {
        return ch ^ 32;
    }
     
    // give up for non-ascii
    return ch;
}

//+---------------------------------------------------------------------------
//
// ToggleCase
//
// Toggle the case of all text covered by the range.  The input range is
// collapsed to its end point on exit.
//
// If fIgnoreRangeEnd == TRUE, all text following the start of range will be
// toggled, and the range will be collapsed at the end-of-doc on exit.
//----------------------------------------------------------------------------

void ToggleCase(TfEditCookie ec, ITfRange *pRange, BOOL fIgnoreRangeEnd)
{
    ITfRange *pRangeToggle;
    ULONG cch;
    ULONG i;
    DWORD dwFlags;
    WCHAR achText[64];

    // backup the current range
    if (pRange->Clone(&pRangeToggle) != S_OK)
        return;

    dwFlags = TF_TF_MOVESTART | (fIgnoreRangeEnd ? TF_TF_IGNOREEND : 0);

    while (TRUE)
    {
        // grab the next block of chars
        if (pRange->GetText(ec, dwFlags, achText, ARRAYSIZE(achText), &cch) != S_OK)
            break;

        // out of text?
        if (cch == 0)
            break;

        // toggle the case
        for (i=0; i<cch; i++)
        {
            achText[i] = ToggleChar(achText[i]);
        }

        // shift pRangeToggle so it covers just the text we read
        if (pRangeToggle->ShiftEndToRange(ec, pRange, TF_ANCHOR_START) != S_OK)
            break;

        // replace the text
        pRangeToggle->SetText(ec, 0, achText, cch);

        // prepare for next iteration
        pRangeToggle->Collapse(ec, TF_ANCHOR_END);
    }
    
    pRangeToggle->Release();
}
