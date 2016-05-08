// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "ChooseFont.h"
#include "FontEnumeration.h"
#include "GdiTextRenderer.h"
#include "resource.h"

HINSTANCE g_hInstance;
IDWriteFactory* g_dwrite = NULL;


/******************************************************************
*                                                                 *
* The ChooseFontDialog class controls all the UI associated with  *
* the dialog and uses the support routines in FontEnumeration.cpp *
* to enumerate fonts and get various information about each one.  *
*                                                                 *
******************************************************************/

class ChooseFontDialog
{
public:

    ChooseFontDialog();

    HRESULT GetTextFormat(IDWriteTextFormat** textFormat);
    HRESULT GetTextFormat(IDWriteTextFormat* textFormatIn, IDWriteTextFormat** textFormatOut);

    ~ChooseFontDialog();

private:

    HWND                    m_dialog;
    WCHAR                   m_localeName[LOCALE_NAME_MAX_LENGTH];
    IDWriteFontCollection*  m_fontCollection;
    IDWriteTextFormat*      m_currentTextFormat;
    
    HRESULT OnFontFamilySelect();
    HRESULT OnFontFaceSelect();
    HRESULT OnFontSizeSelect();

    HRESULT OnFontFamilyNameEdit(HWND hwndFontFamilies);
    HRESULT OnFontFaceNameEdit(HWND hwndFontFaces);
    HRESULT OnFontSizeNameEdit(HWND hwndFontSizes);

    HRESULT DrawSampleText(HDC sampleDC);

    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
};


/******************************************************************
*                                                                 *
* ChooseFontDialog::ChooseFontDialog                              *
*                                                                 *
* Prepare to display the dialog                                   *
*                                                                 *
******************************************************************/

ChooseFontDialog::ChooseFontDialog()
    :   m_dialog(NULL),
        m_fontCollection(),
        m_currentTextFormat()
{
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::~ChooseFontDialog                             *
*                                                                 *
* Clean up resources                                              *
*                                                                 *
******************************************************************/

ChooseFontDialog::~ChooseFontDialog()
{
    SafeRelease(&m_fontCollection);
    SafeRelease(&m_currentTextFormat);
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::ChooseFontDialog                              *
*                                                                 *
* Create and display the dialog initialized to default attributes *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::GetTextFormat(IDWriteTextFormat** textFormat)
{
    HRESULT hr = S_OK;

    *textFormat = NULL;

    // Default to the system font collection
    SafeRelease(&m_fontCollection);
    hr = g_dwrite->GetSystemFontCollection(&m_fontCollection);

    // Default to the users' locale
    if (SUCCEEDED(hr))
    {
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNAME, &m_localeName[0], ARRAYSIZE(m_localeName));
    }

    // Create a default text format
    if (SUCCEEDED(hr))
    {
        SafeRelease(&m_currentTextFormat);
        hr = g_dwrite->CreateTextFormat(
                L"Segoe UI",
                m_fontCollection,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                10.0f,
                m_localeName,
                &m_currentTextFormat);
    }

    // Open the dialog
    if (SUCCEEDED(hr))
    {
        hr = (HRESULT) DialogBoxParam(g_hInstance, L"ChooseFont", NULL, DialogProc, (LPARAM) this);
    }

    // If all went well, and the user didn't cancel, return the new format.
    if (hr == S_OK)
        *textFormat = SafeDetach(&m_currentTextFormat);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::ChooseFontDialog                              *
*                                                                 *
* Create and display the dialog initialized to attributes in an   *
* already existing text format.                                   *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::GetTextFormat(IDWriteTextFormat* textFormatIn, IDWriteTextFormat** textFormatOut)
{
    HRESULT hr = S_OK;

    *textFormatOut = NULL;

    SafeSet(&m_currentTextFormat, textFormatIn);

    // Pull out the input font attributes
    SafeRelease(&m_fontCollection);
    hr = m_currentTextFormat->GetFontCollection(&m_fontCollection);
    if (SUCCEEDED(hr))
    {
        hr = m_currentTextFormat->GetLocaleName(&m_localeName[0], ARRAYSIZE(m_localeName));
    }
    
    // Open the dialog
    if (SUCCEEDED(hr))
    {
        hr = (HRESULT) DialogBoxParam(g_hInstance, L"ChooseFont", NULL, DialogProc, (LPARAM) this);
    }

    // If all went well, and the user didn't cancel, return the new format.
    if (hr == S_OK)
        *textFormatOut = SafeDetach(&m_currentTextFormat);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFamilySelect                            *
*                                                                 *
* Update the font face list to match the newly select font family *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFamilySelect()
{
    HRESULT hr = S_OK;

    HWND hwndFontFamilyNames = GetDlgItem(m_dialog, ID_FONT_FAMILY_NAMES);
    HWND hwndFontFaceNames   = GetDlgItem(m_dialog, ID_FONT_FACE_NAMES);
    int currentSelection     = ComboBox_GetCurSel(hwndFontFamilyNames);

    // Get the font family name
    WCHAR fontFamilyName[100];

    UINT32 fontFamilyNameLength = ComboBox_GetLBTextLen(hwndFontFamilyNames, currentSelection) + 1;
    if (fontFamilyNameLength > ARRAYSIZE(fontFamilyName))
        hr = E_NOT_SUFFICIENT_BUFFER;

    if (SUCCEEDED(hr))
    {
        ComboBox_GetLBText(hwndFontFamilyNames, currentSelection, &fontFamilyName[0]);
    }

    // Get the face names for the new font family
    IDWriteFontFamily*          fontFamily = NULL;
    std::vector<IDWriteFont*>   fonts;

    // Get the font variants for this family
    if (currentSelection != CB_ERR)
        hr = GetFonts(m_fontCollection, fontFamilyName, fonts);

    // Initialize the face name list
    std::vector<FontFaceInfo> fontFaceInfo;
    if (SUCCEEDED(hr))
    {
        ComboBox_ResetContent(hwndFontFaceNames);
        GetFontFaceInfo(fonts, m_localeName, fontFaceInfo);
    }

    if (SUCCEEDED(hr))
    {
        for (size_t i = 0; i != fontFaceInfo.size(); ++i)
        {
            int fontFaceIndex = ComboBox_AddString(hwndFontFaceNames, fontFaceInfo[i].fontFaceName);
            ComboBox_SetItemData(hwndFontFaceNames, fontFaceIndex, fontFaceInfo[i].PackedFontAttributes());
        }
    }

    // Select the best fit font face for the current attributes
    if (SUCCEEDED(hr))
    {
        FontFaceInfo desiredAttributes(
                            L"", 
                            m_currentTextFormat->GetFontWeight(), 
                            m_currentTextFormat->GetFontStyle(), 
                            m_currentTextFormat->GetFontStretch());

        int selectedFontFaceName = 0;
        ULONG bestFitAttributes = GetBestFontAttributes(m_fontCollection, fontFamilyName, desiredAttributes);

        int fontFaceCount = ComboBox_GetCount(hwndFontFaceNames);

        for (int i = 0; i != fontFaceCount; ++i)
        {
            if ((ULONG)ComboBox_GetItemData(hwndFontFaceNames, i) == bestFitAttributes)
            {
                selectedFontFaceName = i;
                break;
            }
        }

        ComboBox_SetCurSel(hwndFontFaceNames, selectedFontFaceName);
        OnFontFaceSelect();
    }

    // Release the held font list.
    for (size_t i = 0, ci = fonts.size(); i < ci; ++i)
    {
        SafeRelease(&fonts[i]);
    }
    SafeRelease(&fontFamily);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFaceSelect                              *
*                                                                 *
* Record the new font face selection and redraw the sample text.  *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFaceSelect()
{
    HRESULT hr = S_OK;

    // Signal the sample text window to redraw itself.
    InvalidateRect(GetDlgItem(m_dialog, ID_SAMPLE_BOX), NULL, false);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontSizeSelect                              *
*                                                                 *
* Record the new font size and redraw the sample text.            *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontSizeSelect()
{
    HRESULT hr = S_OK;

    // Signal the sample text window to redraw itself.
    InvalidateRect(GetDlgItem(m_dialog, ID_SAMPLE_BOX), NULL, false);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFamilyNameEdit                          *
*                                                                 *
* Watch what is typed into the edit portion of the font family    *
* combo and automatically select a name if a match is found.  As  *
* an added feature, also match against localized forms of the     *
* family name.  For example the user can type "Meiryo" on a       *
* Japanese system and it will be found even though it's displayed *
* using a localized variant of the name.                          *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFamilyNameEdit(HWND hwndFontFamilies)
{
    HRESULT hr = S_OK;

    // Save the state of the edit box selection
    DWORD editSelection      = ComboBox_GetEditSel(hwndFontFamilies);
    int   editSelectionBegin = LOWORD(editSelection);
    int   editSelectionEnd   = HIWORD(editSelection);

    // Get the text in the edit portion of the combo
    WCHAR fontFamilyName[100];
    ComboBox_GetText(hwndFontFamilies, &fontFamilyName[0], ARRAYSIZE(fontFamilyName));

    // Try to find an exact match (case-insensitive)
    int matchingFontFamily = ComboBox_FindStringExact(hwndFontFamilies, -1, fontFamilyName);
    bool usedAltMatch = false;

    if (matchingFontFamily == CB_ERR)
    {
        // If a match isn't found, scan all for alternate forms in the font
        // collection.
        IDWriteFontFamily* fontFamily = NULL;
        hr = GetFontFamily(m_fontCollection, fontFamilyName, &fontFamily);

        if (hr == S_OK)
        {
            // If a match is found, get the family name localized to the locale
            // we're using in the combo box and match against that.
            usedAltMatch = true;

            std::wstring localFontFamilyName;
            hr = GetFontFamilyName(fontFamily, m_localeName, localFontFamilyName);

            if (SUCCEEDED(hr))
            {
                matchingFontFamily = ComboBox_FindStringExact(hwndFontFamilies, -1, localFontFamilyName.c_str());
            }
        }
        else if (hr == DWRITE_E_NOFONT)
        {
            // Ignore DWRITE_E_NOFONT errors
            hr = S_OK;
        }

        SafeRelease(&fontFamily);
    }

    // Process the match, if any
    if (SUCCEEDED(hr) && matchingFontFamily != CB_ERR)
    {
        ComboBox_SetCurSel(hwndFontFamilies, matchingFontFamily);

        // SetCurSel will update the edit text to match the text of the 
        // selected item.  If we matched against an alternate name put that
        // name back.
        if (usedAltMatch)
            ComboBox_SetText(hwndFontFamilies, fontFamilyName);

        // Reset the edit selection to what is was before SetCurSel.
        ComboBox_SetEditSel(hwndFontFamilies, editSelectionBegin, editSelectionEnd);

        hr = OnFontFamilySelect();
    }

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFaceNameEdit                            *
*                                                                 *
* Watch what is typed into the edit portion of the font face      *
* combo and automatically select a name if a match is found.      *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFaceNameEdit(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Save the state of the edit box selection
    DWORD editSelection      = ComboBox_GetEditSel(hwnd);
    int   editSelectionBegin = LOWORD(editSelection);
    int   editSelectionEnd   = HIWORD(editSelection);

    // Try to find the currently typed text
    WCHAR text[100];
    ComboBox_GetText(hwnd, &text[0], ARRAYSIZE(text));

    int selectedItem = ComboBox_FindStringExact(hwnd, -1, text);
    if (selectedItem != CB_ERR)
    {
        // If text is found, select the corresponding list item, put the
        // selection state back to what it was originally, and redraw the
        // sample text
        ComboBox_SetCurSel(hwnd, selectedItem);
        ComboBox_SetEditSel(hwnd, editSelectionBegin, editSelectionEnd);
        hr = OnFontFaceSelect();
    }

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontSizeNameEdit                            *
*                                                                 *
* Watch what is typed into the edit portion of the font size      *
* combo and automatically select a name if a match is found.      *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontSizeNameEdit(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Save the state of the edit box selection
    DWORD editSelection = ComboBox_GetEditSel(hwnd);
    int   editSelectionBegin = LOWORD(editSelection);
    int   editSelectionEnd = HIWORD(editSelection);

    // Try to find the currently typed text
    WCHAR text[100];
    ComboBox_GetText(hwnd, &text[0], ARRAYSIZE(text));

    int selectedItem = ComboBox_FindStringExact(hwnd, -1, text);
    if (selectedItem != CB_ERR)
    {
        // If text is found, select the corresponding list item, put the
        // selection state back to what it was originally, and redraw the
        // sample text
        ComboBox_SetCurSel(hwnd, selectedItem);
        ComboBox_SetEditSel(hwnd, editSelectionBegin, editSelectionEnd);
    }
    hr = OnFontSizeSelect();

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::DrawSampleText                                *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::DrawSampleText(HDC sampleDC)
{
    static WCHAR sampleText[] = L"The quick brown fox jumps over the lazy dog";

    HRESULT hr = S_OK;

    HWND hwndFontFamilies   = GetDlgItem(m_dialog, ID_FONT_FAMILY_NAMES);
    HWND hwndFontFaces      = GetDlgItem(m_dialog, ID_FONT_FACE_NAMES);
    HWND hwndFontSizes      = GetDlgItem(m_dialog, ID_FONT_SIZE);
    HWND hwndSampleBox      = GetDlgItem(m_dialog, ID_SAMPLE_BOX);

    // Get the currently selected font family.  If there isn't one, then just
    // don't update the text format and continue to use whatever we had before
    int  selectedFontFamily = ComboBox_GetCurSel(hwndFontFamilies);

    if (selectedFontFamily != CB_ERR)
    {
        // Get the font family name
        WCHAR fontFamilyName[100];
        GetWindowText(hwndFontFamilies, &fontFamilyName[0], ARRAYSIZE(fontFamilyName));

        // Get the font face attributes
        int selectedFontFace = ComboBox_GetCurSel(hwndFontFaces);
        ULONG packedAttributes = (ULONG) ComboBox_GetItemData(hwndFontFaces, selectedFontFace);

        // Get the font size
        WCHAR fontSizeText[100];
        GetWindowText(hwndFontSizes, &fontSizeText[0], ARRAYSIZE(fontSizeText));

        float pointSize = float(wcstod(fontSizeText, NULL));
        if (pointSize <= 0)
            pointSize = 10;

        float dipSize = pointSize * 96 / 72;

        FontFaceInfo fontFaceInfo(fontFamilyName, packedAttributes);

        // Recreate the text format object
        SafeRelease(&m_currentTextFormat);
        hr = g_dwrite->CreateTextFormat(
                            fontFamilyName,
                            m_fontCollection,
                            fontFaceInfo.fontWeight,
                            fontFaceInfo.fontStyle,
                            fontFaceInfo.fontStretch,
                            dipSize,
                            m_localeName,
                            &m_currentTextFormat);
    }

    // Get the size of the sample box
    RECT sampleBounds = {};
    GetClientRect(hwndSampleBox, &sampleBounds);

    UINT width  = sampleBounds.right  - sampleBounds.left;
    UINT height = sampleBounds.bottom - sampleBounds.top;

    // Layout the sample text using the text format and UI bounds (converted to DIPs)
    IDWriteTextLayout* textLayout = NULL;
    if (SUCCEEDED(hr))
    {
        hr = g_dwrite->CreateTextLayout(
                sampleText, 
                ARRAYSIZE(sampleText) - 1, 
                m_currentTextFormat, 
                (float) width  * 96.0f / GetDeviceCaps(sampleDC, LOGPIXELSY), 
                (float) height * 96.0f / GetDeviceCaps(sampleDC, LOGPIXELSY), 
                &textLayout);
    }

    // Create a DWrite surface to render to
    GdiTextRenderer* textRenderer = NULL;
    if (SUCCEEDED(hr))
    {
        textRenderer = SafeAcquire(new(std::nothrow) GdiTextRenderer());
        if (textRenderer != NULL)
            hr = textRenderer->Initialize(m_dialog, sampleDC, width, height);
        else
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Fill the DWrite surface with the background color
        HDC dwriteDC = textRenderer->GetDC();
        SetDCBrushColor(dwriteDC, GetSysColor(COLOR_BTNFACE));
        FillRect(dwriteDC, &sampleBounds, GetStockBrush(DC_BRUSH));

        // Draw the text onto the DWrite surface
        hr = textLayout->Draw(NULL, textRenderer, 0, 0);

        // Copy the DWrite surface to the sample on screen
        BitBlt(
            sampleDC, 
            0,
            0,
            width,
            height,
            dwriteDC,
            0,
            0,
            SRCCOPY | NOMIRRORBITMAP);
    }

    SafeRelease(&textRenderer);
    SafeRelease(&textLayout);

    return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::WndProc                                       *
*                                                                 *
* Dispatch window message to the appropriate hander               *
*                                                                 *
******************************************************************/

INT_PTR CALLBACK ChooseFontDialog::DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG)
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

    ChooseFontDialog* this_ = (ChooseFontDialog*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (this_ != NULL)
    {
        switch (message)
        {
            HANDLE_MSG(hWnd, WM_INITDIALOG, this_->OnInitDialog);
            HANDLE_MSG(hWnd, WM_COMMAND,    this_->OnCommand);
            HANDLE_MSG(hWnd, WM_DRAWITEM,   this_->OnDrawItem);
        }
    }

    return FALSE;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnInitDialog                                  *
*                                                                 *
* Initialize the dialog by enumerating the font families and      *
* setting up a hardcoded list of standard font sizes.             *
*                                                                 *
******************************************************************/

BOOL ChooseFontDialog::OnInitDialog(HWND dialog, HWND hwndFocus, LPARAM lParam)
{
    m_dialog = dialog;

    HWND hwndFamilyNames = GetDlgItem(dialog, ID_FONT_FAMILY_NAMES);
    HWND hwndSizes = GetDlgItem(dialog, ID_FONT_SIZE);

    // Fill in the font family name list.

    std::vector<std::wstring> fontFamilyNames;
    if (FAILED(GetFontFamilyNames(m_fontCollection, m_localeName, fontFamilyNames)))
        return FALSE;

    for (size_t i = 0; i != fontFamilyNames.size(); ++i)
        ComboBox_AddString(hwndFamilyNames, fontFamilyNames[i].c_str());
    
    // Fill in the hardcoded font sizes

    static const float FontSizes[] = {
        1.5, 3.5, 4.5, 6,
        8, 9, 10, 11, 12, 14, 16, 18,
        20, 22, 24, 26, 28, 36, 48, 72
    };

    WCHAR sizeName[100];
    sizeName[0] = '\0';

    for (int i = 0; i != ARRAYSIZE(FontSizes); ++i)
    {
        StringCchPrintf(sizeName, ARRAYSIZE(sizeName), L"%.3G", FontSizes[i]);
        ComboBox_AddString(hwndSizes, sizeName);
    }

    // Select the current size

    StringCchPrintf(sizeName, ARRAYSIZE(sizeName), L"%0.0f", m_currentTextFormat->GetFontSize());

    SetWindowText(hwndSizes, sizeName);
    if (CB_ERR == ComboBox_SelectString(hwndSizes, -1, sizeName))
        SetWindowText(hwndSizes, sizeName);

    // Select the font family specified in the input text format.

    int selectedFontFamily = CB_ERR;
    std::wstring fontFamilyName;
        
    if (SUCCEEDED(GetFontFamilyNameFromFormat(m_currentTextFormat, fontFamilyName)))
    {
        selectedFontFamily = ComboBox_SelectString(hwndFamilyNames, -1, fontFamilyName.c_str());
    }

    if (selectedFontFamily == CB_ERR)
        SetWindowText(hwndFamilyNames, fontFamilyName.c_str());

    OnFontFamilySelect();

    return TRUE;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnCommand                                     *
*                                                                 *
* Dispatch button clicks, changing listbox selections, etc.       *
*                                                                 *
******************************************************************/

void ChooseFontDialog::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDCANCEL && codeNotify == BN_CLICKED)
        EndDialog(hwnd, S_FALSE);

    else if (id == IDOK && codeNotify == BN_CLICKED)
        EndDialog(hwnd, S_OK);

    else if (id == ID_FONT_FAMILY_NAMES && codeNotify == CBN_SELCHANGE)
        OnFontFamilySelect();

    else if (id == ID_FONT_FAMILY_NAMES && codeNotify == CBN_EDITCHANGE)
        OnFontFamilyNameEdit(hwndCtl);

    else if (id == ID_FONT_FACE_NAMES && codeNotify == CBN_SELCHANGE)
        OnFontFaceSelect();

    else if (id == ID_FONT_FACE_NAMES && codeNotify == CBN_EDITCHANGE)
        OnFontFaceNameEdit(hwndCtl);

    else if (id == ID_FONT_SIZE && codeNotify == CBN_SELCHANGE)
        OnFontSizeSelect();

    else if (id == ID_FONT_SIZE && codeNotify == CBN_EDITCHANGE)
        OnFontSizeNameEdit(hwndCtl);
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnDrawItem                                    *
*                                                                 *
* Redraw the sample text whenever it's window needs updating      *
*                                                                 *
******************************************************************/

void ChooseFontDialog::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
    DrawSampleText(lpDrawItem->hDC);
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    // The Microsoft Security Development Lifecycle recommends that all
    // applications include the following call to ensure that heap corruptions
    // do not go unnoticed and therefore do not introduce opportunities
    // for security exploits.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    g_hInstance = hInstance;

    DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED, 
            __uuidof(IDWriteFactory), 
            (IUnknown **) &g_dwrite);

    {
        ChooseFontDialog chooseFont;

        IDWriteTextFormat* textFormatOut = NULL;
        chooseFont.GetTextFormat(&textFormatOut);

        SafeRelease(&textFormatOut);
    }
    SafeRelease(&g_dwrite);
}