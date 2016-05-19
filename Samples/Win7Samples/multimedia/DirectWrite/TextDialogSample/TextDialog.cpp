
/************************************************************************
 *
 * File: TextDialog.cpp
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#include "TextDialogSample.h"


/******************************************************************
*                                                                 *
*  TextDialog::TextDialog constructor                             *
*                                                                 *
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

TextDialog::TextDialog() :
    hwnd_(NULL),
    cTextLength_(0),
    weight_(DWRITE_FONT_WEIGHT_NORMAL),
    style_(DWRITE_FONT_STYLE_NORMAL),
    underline_(FALSE),
    fontSize_(72.0f),
    pD2DFactory_(NULL),
    pRT_(NULL),
    pBlackBrush_(NULL),
    pDWriteFactory_(NULL),
    pTextFormat_(NULL),
    pTextLayout_(NULL)
{
}

/******************************************************************
*                                                                 *
*  TextDialog::~TextDialog destructor                             *
*                                                                 *
*  Tear down resources                                            *
*                                                                 *
******************************************************************/

TextDialog::~TextDialog()
{
    SafeRelease(&pD2DFactory_);
    SafeRelease(&pRT_);
    SafeRelease(&pBlackBrush_);
    SafeRelease(&pDWriteFactory_);
    SafeRelease(&pTextFormat_);
    SafeRelease(&pTextLayout_);

    delete [] wszText_;
    delete [] wszFontFamily_;
}

/******************************************************************
*                                                                 *
*  TextDialog::Initialize                                         *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/

HRESULT TextDialog::Initialize(HWND hwnd)
{
    // Get the dpi information.
    HDC screen = GetDC(0);
    dpiScaleX_ = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
    dpiScaleY_ = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
    ReleaseDC(0, screen);

    HRESULT hr = S_OK;

    hwnd_ = hwnd;
    
    // Set family name to Gabriola.
    const wchar_t defaultText [] = L"Gabriola";
    wszFontFamily_ = new (std::nothrow) wchar_t[ARRAYSIZE(defaultText)];

    if (wszFontFamily_ == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
   
    wcscpy_s(wszFontFamily_, ARRAYSIZE(defaultText), defaultText);


    if (SUCCEEDED(hr))
    {
        hr = hwnd_ ? S_OK : E_FAIL;

    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceIndependentResources(
            );
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::CreateDeviceIndependentResources                   *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app. These resources include the Direct2D and  *
*  DirectWrite factories; and a DirectWrite Text Format object    *
*  (used for identifying particular font characteristics) and     *
*  a Direct2D geometry.                                           *
*                                                                 *
******************************************************************/

HRESULT TextDialog::CreateDeviceIndependentResources()
{
    HRESULT hr;

    // Create Direct2D factory.
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory_
        );

    // Create a shared DirectWrite factory.
    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory_)
            );
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::CreateDeviceResources                              *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

HRESULT TextDialog::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(
        hwnd_,
        &rc
        );

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    if (!pRT_)
    {
        // Create a Direct2D render target.
        hr = pD2DFactory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd_,
                size
                ),
            &pRT_
            );

        // Create a black brush.
        if (SUCCEEDED(hr))
        {
            hr = pRT_->CreateSolidColorBrush(
                D2D1::ColorF(
                D2D1::ColorF::Black
                ),
                &pBlackBrush_
                );
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::DiscardDeviceResources                             *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a D3D device is lost                                      *
*                                                                 *
******************************************************************/

void TextDialog::DiscardDeviceResources()
{
    SafeRelease(&pRT_);
    SafeRelease(&pBlackBrush_);
}


/******************************************************************
*                                                                 *
*  TextDialog::DrawText                                           *
*                                                                 *
*  This method will draw text using the IDWriteTextLayout         *
*  via the Direct2D render target                                 *
*                                                                 *
******************************************************************/

HRESULT TextDialog::DrawText()
{
    RECT rc;

    GetClientRect(
        hwnd_,
        &rc
        );

    D2D1_POINT_2F origin = D2D1::Point2F(
        static_cast<FLOAT>(rc.top / dpiScaleY_),
        static_cast<FLOAT>(rc.left / dpiScaleX_)
        );

    pRT_->DrawTextLayout(
        origin,
        pTextLayout_,
        pBlackBrush_
        );


    return S_OK;
}

/******************************************************************
*                                                                 *
*  TextDialog::DrawD2DContent                                     *
*                                                                 *
*  This method draws the text                                      *
*                                                                 *
*  Note that this function will not render anything if the window *
*  is occluded (eg. obscured by other windows or off monitor).    *
*  Also, this function will automatically discard device-specific *
*  resources if the D3D device disappears during execution, and   *
*  will recreate the resources the next time it's invoked.        *
*                                                                 *
******************************************************************/

HRESULT TextDialog::DrawD2DContent()
{
    HRESULT hr;

    hr = CreateDeviceResources();

    if (!(pRT_->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        pRT_->BeginDraw();

        pRT_->SetTransform(D2D1::IdentityMatrix());

        pRT_->Clear(D2D1::ColorF(D2D1::ColorF::White));

        // Call the DrawText method of this class.
        if (SUCCEEDED(hr))
        {
            DrawText();
        }

        if (SUCCEEDED(hr))
        {
            hr = pRT_->EndDraw(
                );
        }
    }

    if (FAILED(hr))
    {
        DiscardDeviceResources();
    }

    return hr;
}

//
// Functions for creating objects and setting font family, style, weight, size and so on.
//

/******************************************************************
*                                                                 *
*  TextDialog::SetFont                                            *
*                                                                 *
*  This method sets the font family for the entire range of text  *
*  in the text layout object then redraws the text by using the   *
*  DrawD2DContent method of this class.                              *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetFont(wchar_t *fontFamily)
{
    HRESULT hr;

    DWRITE_TEXT_RANGE textRange = {0, cTextLength_};

    hr = pTextLayout_->SetFontFamilyName(fontFamily, textRange);

    if (SUCCEEDED(hr))
    {
        // Delete the old family name.
        delete [] wszFontFamily_;

        // Store the font family for later.
        wszFontFamily_ = fontFamily;
    }

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetFontSize                                        *
*                                                                 *
*  This method sets the font size for the entire range of text    *
*  in the text layout object then redraws the text by using the   *
*  DrawD2DContent method of this class.                              *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetFontSize(float size)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = {0, cTextLength_};
    
    hr = pTextLayout_->SetFontSize(size, textRange);

    if (SUCCEEDED(hr))
    {
        // Store the font size for later.
        fontSize_ = size;
    }

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetText                                            *
*                                                                 *
*  This method creates a new IDWriteTextLayout object with the    *
*  pecified text.                                                *
*                                                                 *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetText(wchar_t *text)
{
    HRESULT hr;
    
    // Store the text for later.
    cTextLength_ = (UINT32) wcslen(text);

    delete [] wszText_;

    wszText_ = new (std::nothrow) wchar_t [cTextLength_ + 1];

    if (wszText_ == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    wcscpy_s(wszText_, cTextLength_ + 1, text);

    // Release any exisiting format object.
    SafeRelease(&pTextFormat_);

    // Create a text format.
    hr = pDWriteFactory_->CreateTextFormat(
            wszFontFamily_,              // Font family name.
            NULL,                        // Font collection (NULL sets it to use the system font collection).
            weight_,
            style_,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize_,
            L"en-us",
            &pTextFormat_
            );

    // Center align (horizontally) the text.
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    // Center align (vertically) the text.
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    // Release any existing layout object.
    SafeRelease(&pTextLayout_);
    
    // Create a text layout using the text format.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextLayout(
            wszText_,      // The string to be laid out and formatted.
            cTextLength_,  // The length of the string.
            pTextFormat_,  // The text format to apply to the string (contains font information, etc).
            680.0f,         // The width of the layout box.
            260.0f,         // The height of the layout box.
            &pTextLayout_  // The IDWriteTextLayout interface pointer.
            );
    }

    // The following code sets the typography for the text layout.
    // This will only change the look for fonts that support this stylistic set.
    // Gabriola does, most of the older fonts do not.  If the font does not support
    // the stylistic set, these methods will return S_OK but there will be no effect.

    // Declare a typography pointer.
    IDWriteTypography* pTypography = NULL;

    // Create a typography interface object.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTypography(&pTypography);
    }

    // Set the stylistic set.
    DWRITE_FONT_FEATURE fontFeature = {DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,
                                       1};
    if (SUCCEEDED(hr))
    {
        hr = pTypography->AddFontFeature(fontFeature);
    }

    // Set the typography for the entire string.
    DWRITE_TEXT_RANGE textRange = {0,
                                   cTextLength_};

    if (SUCCEEDED(hr))
    {
        hr = pTextLayout_->SetTypography(pTypography, textRange);
    }

    // Set the underline for the entire string.
    if (SUCCEEDED(hr))
    {
        hr = pTextLayout_->SetUnderline(underline_, textRange);
    }

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    SafeRelease(&pTypography);

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::EnumerateFonts                                     *
*                                                                 *
*  This method enumerates the fonts in the system font collection *
*  and adds them to the combo box list.                           *
*                                                                 *
*                                                                 *
******************************************************************/

HRESULT TextDialog::EnumerateFonts(HWND comboBox)
{
    HRESULT hr;

    IDWriteFontCollection* pFontCollection = NULL;

    hr = pDWriteFactory_->GetSystemFontCollection(&pFontCollection);

    UINT32 familyCount = pFontCollection->GetFontFamilyCount();

    for (UINT32 i = 0; i < familyCount; ++i)
    {
        UINT32 index = 0;
        BOOL exists = false;
        UINT32 length = 0;
        wchar_t *name = NULL;

        IDWriteFontFamily* pFontFamily = NULL;
        IDWriteLocalizedStrings* pFamilyNames = NULL;

        // Get the font family.
        if (SUCCEEDED(hr))
        {
            hr = pFontCollection->GetFontFamily(i, &pFontFamily);
        }

        // Get a list of localized strings for the family name.
        if (SUCCEEDED(hr))
        {
            hr = pFontFamily->GetFamilyNames(&pFamilyNames);
        }

        // Select the en-us locale.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
        }

        // If the pecified locale doesn't exist, select the default.
        if (!exists)
            index = 0;

        // Get the string length.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetStringLength(index, &length);
        }

        if (SUCCEEDED(hr))
        {
            // Allocate a string big enough to hold the name.
            name = new(std::nothrow) wchar_t[length+1];
            if (name == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        // Get the family name.
        if (SUCCEEDED(hr))
        {
            hr = pFamilyNames->GetString(index, name, length+1);
        }

        if (SUCCEEDED(hr))
        {
            // Add it to the combo box.
            SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM) name);
        }

        SafeRelease(&pFamilyNames);
        SafeRelease(&pFontFamily);

        delete [] name;
    }

    SafeRelease(&pFontCollection);

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetBold                                            *
*                                                                 *
*  This method toggles the weight to and from bold for the        *
*  entire range of text in the text layout object then redraws    *
*  the text by using the DrawD2DContent method of this class.      *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetBold(bool bold)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = {0, cTextLength_};

    if (bold)
    {
        weight_ = DWRITE_FONT_WEIGHT_BOLD;
    }
    else
    {
        weight_ = DWRITE_FONT_WEIGHT_NORMAL;
    }
    
    // Set the font weight.
    hr = pTextLayout_->SetFontWeight(weight_, textRange);

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetItalic                                          *
*                                                                 *
*  This method toggles the style to and from italic for the       *
*  entire range of text in the text layout object then redraws    *
*  the text by using the DrawD2DContent method of this class.      *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetItalic(bool italic)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = {0, cTextLength_};

    if (italic)
    {
        style_ = DWRITE_FONT_STYLE_ITALIC;
    }
    else
    {
        style_ = DWRITE_FONT_STYLE_NORMAL;
    }
    
    // Set the font style.
    hr = pTextLayout_->SetFontStyle(style_, textRange);

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetUnderline                                       *
*                                                                 *
*  This method sets toggles the underline for the entire range of *
*  text in the text layout object then redraws the text by using  *
*  the DrawD2DContent method of this class.                          *
*                                                                 *
******************************************************************/

HRESULT TextDialog::SetUnderline(bool underline)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = {0, cTextLength_};

    underline_ = underline;

    // Set the underline.
    hr = pTextLayout_->SetUnderline(underline_, textRange);

    if (SUCCEEDED(hr))
    {
        // Redraw the content.
        DrawD2DContent();
    }

    return hr;
}

