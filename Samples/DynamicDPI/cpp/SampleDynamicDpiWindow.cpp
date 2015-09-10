//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleDynamicDpiWindow.h"
#include "SampleRichEditControl.h"
#include "SampleCustomColorFontButton.h"

using namespace std;
using namespace Microsoft::WRL;

namespace UILayout
{
    static const float TitlePositionX = 20.0F;
    static const float TitlePositionY = 10.0F;
    static const float SubTitlePositionY = 60.0F;

    static const float SmallXOffset = 42.0f;
    static const float ButtonPositionX = 20.0F;
    static const float ButtonPositionY = 130.0F;
    static const float ButtonPpemSize = 24.0F;
};

// <summary> 
// Initializes the elements that will be used to draw the application UI.
// Unless otherwise specified, each of these elements is intended to
// resize, relayout or show different UI in response to DPI changes.
// </summary>
CSampleDynamicDpiWindow::CSampleDynamicDpiWindow()
{
    m_RichEditWindow.SetDefaultDPI(GetDpiForWindow());

    WCHAR szPath[MAX_PATH];
    DX::ThrowIfFailed(GetModuleFileName(NULL, szPath, MAX_PATH) == 0 ? E_FAIL : S_OK);
    auto path = wstring(szPath);
    auto found = path.find_last_of('\\');
    m_resourcePath = path.substr(0, found);

    // DPI string will be used to display information about the DPI mode detected.
    m_dpiString = (L"");

    // Create a bitmap that will show a different image based on DPI detected.
    m_bitmap = make_shared<CSampleMultiDPIImage>(100.0F, 130.0F);
    m_engine.AddElement(m_bitmap);

    // Prepare static text that will resize based on DPI.
    auto title = make_shared<CSampleElementTextBlock>(UILayout::TitlePositionX, UILayout::TitlePositionY, 450.0F, 50.0F, 30.0F);
    title->SetText(wstring(L"Direct2D Per-monitor DPI Aware"));
    m_engine.AddElement(title);

    // Create static text that displays DPI information and will resize based on DPI.
    m_CurrentDpi = make_shared<CSampleElementTextBlock>(UILayout::TitlePositionX, UILayout::SubTitlePositionY, 650.0F, 50.0F, 20.0F);
    m_engine.AddElement(m_CurrentDpi);

    // Buttons to change font size of rich edit control.
    auto button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 0 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::IncreaseSize);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.IncrementFontSize();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 1 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::DecreaseSize);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.DecrementFontSize();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 2 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::BulletList);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetBulleted();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 3 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::NumberList);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetNumbered();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 4 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Bold);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetFormatBold();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 5 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Underline);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetFormatUnderline();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 6 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Italics);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetFormatItalic();
    });
    m_engine.AddElement(button);
    
    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 7 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Highlight);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetFormatBackgroundColor();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 8 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::SetColor);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetFormatColor();
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 9 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::LeftJustify);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetAlignment(PFA_LEFT);
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 10 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::CenterJustify);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.SetAlignment(PFA_CENTER);
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 11 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::RightJustify);
    button->SetClickHandler([&]()
    {
       m_RichEditWindow.SetAlignment(PFA_RIGHT);
    });
    m_engine.AddElement(button);
    
    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 12 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Outdent);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.IncrementIndent(-720);
    });
    m_engine.AddElement(button);

    button = make_shared<CSampleCustomColorFontButton>(UILayout::ButtonPositionX + 13 * UILayout::SmallXOffset, UILayout::ButtonPositionY, UILayout::ButtonPpemSize, CustomGlyphId::Indent);
    button->SetClickHandler([&]()
    {
        m_RichEditWindow.IncrementIndent(720);
    });
    m_engine.AddElement(button);
}

CSampleDynamicDpiWindow::~CSampleDynamicDpiWindow()
{
    m_deviceResources->RegisterDeviceNotify(nullptr);
}

// <summary>
// This method initializes member variables and objects required by this class.
// This method will be called once base class initialize functionality completes.
// </summary>
HRESULT 
CSampleDynamicDpiWindow::Initialize(
    _In_    RECT bounds,
    _In_    std::wstring title
    )
{
    HRESULT hr = S_OK;

    // First run Initialize method for base class.
    hr = CSampleDesktopWindow::Initialize(bounds, title);

    // Create and initialize rich edit control as a child of this window.
    if (SUCCEEDED(hr))
    {
        hr = m_RichEditWindow.Initialize(this->m_hWnd);
    }

    // Register this class as a DeviceLostHandler.  This enables the app to release and recreate the
    // device specific resources associated with the app.
    m_deviceResources->RegisterDeviceNotify(this);

    return hr;
}

// <summary>
// This method is called when the m_deviceResources class determines that there has been an error in the
// underlying graphics hardware device.  All device dependent resources should be released on this event.
// </summary>
void
CSampleDynamicDpiWindow::OnDeviceLost()
{
    ReleaseDeviceResources();
}

// <summary>
// This method is called when the m_deviceResources class is recovering from an error in the
// underlying graphics hardware device.  A new device has been created and all device dependent
// resources should be recreated on this event.
// </summary>
void
CSampleDynamicDpiWindow::OnDeviceRestored()
{
    CreateDeviceResources();
}

void
CSampleDynamicDpiWindow::CreateDeviceIndependentResources()
{
    // Load the custom fonts.
    m_deviceResources->LoadFonts(m_resourcePath);

    m_engine.CreateDeviceIndependentResources(m_deviceResources);
}

void 
CSampleDynamicDpiWindow::ReleaseDeviceIndependentResources()
{
    m_engine.ReleaseDeviceIndependentResources();
}

void
CSampleDynamicDpiWindow::CreateDeviceResources()
{
    m_engine.CreateDeviceResources();
}

void
CSampleDynamicDpiWindow::ReleaseDeviceResources()
{
    m_engine.ReleaseDeviceResources();
}

// <summary>
// Draw client area.
//      Function overrides virtual member function to
//      draw member components defined in DPI window class.
// </summary>
void
CSampleDynamicDpiWindow::Draw()
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    RECT rect = { };
    auto d2dContext = m_deviceResources->GetD2DDeviceContext();
    auto sizeF = d2dContext->GetSize();
    float deviceResourcesDPI = m_deviceResources->GetDpi();

    // Prepare elements for drawing.
    if (m_dpiString.length() == 0)
    {
        RebuildCurrentDpiString();
        m_CurrentDpi->SetText(m_dpiString);
    }

    // Set layout position for bitmap relative to the window.
    float left = (sizeF.width * m_deviceResources->GetDpi() / 96.0F) - m_bitmap->GetWidth() - 35.0F;
    m_bitmap->SetPosition(left, 25.0F);

    // Set Rich edit control relative to bitmap.

    // GetClientRect returns the client area in physical pixels.
    fResult = ::GetClientRect(m_hWnd, &rect);

    // Position the offset of the rich edit window in DIPs.
    rect.top += (LONG) (180 * (deviceResourcesDPI / 96));
    if (fResult)
    {
        hr = m_RichEditWindow.PositionRelativeToRect(rect);
    }

    // For each member element, issue drawing instructions.
    if (SUCCEEDED(hr))
    {
        m_engine.Draw();
    }
}

void
CSampleDynamicDpiWindow::OnPointerUp(
    _In_    float x,
    _In_    float y
    )
{
    m_engine.PointerUp(x, y);
}

void
CSampleDynamicDpiWindow::OnPointerDown(
    _In_    float x,
    _In_    float y
    )
{
    m_engine.PointerDown(x, y);
}

void
CSampleDynamicDpiWindow::OnPointerUpdate(
    _In_    float x,
    _In_    float y
    )
{
    m_engine.PointerUpdate(x, y);
}

void
CSampleDynamicDpiWindow::OnDisplayChange()
{
    RECT current;
    ZeroMemory(&current, sizeof(current));
    GetWindowRect(&current);

    float oldDpix, oldDpiy, newDpi;
    m_deviceResources->GetD2DDeviceContext()->GetDpi(&oldDpix, &oldDpiy);
    newDpi = GetDpiForWindow();

    if (oldDpix != newDpi)
    {
        auto newRect = CalcWindowRectNewDpi(current, oldDpix, newDpi);
        SetNewDpi(newDpi);
        m_RichEditWindow.OnDPIChanged(newDpi);
        SetWindowPos(0, &newRect, 0);
        m_dpiString.clear();
    }
}

void
CSampleDynamicDpiWindow::OnDpiChange(
    _In_ int dpi,
    _In_ LPRECT newRect
    )
{
    float changedDpi = static_cast<float>(dpi);
    SetNewDpi(changedDpi);
    m_RichEditWindow.OnDPIChanged(changedDpi);
    SetWindowPos(0, newRect, 0);
    m_dpiString.clear();
}

RECT
CSampleDynamicDpiWindow::CalcWindowRectNewDpi(
    _In_    RECT oldRect,
    _In_    float oldDpi,
    _In_    float newDpi
    )
{
    float oldWidth = static_cast<float>(oldRect.right - oldRect.left);
    float oldHeight = static_cast<float>(oldRect.bottom - oldRect.top);

    int newWidth = static_cast<int>(oldWidth * newDpi / oldDpi);
    int newHeight = static_cast<int>(oldHeight * newDpi / oldDpi);

    RECT newRect = { oldRect.left, oldRect.top, newWidth, oldRect.top + newHeight };
    return newRect;
}

// <summary>
// This method gathers DPI information detected and stores this
//     in the m_dpiString object.
// The m_dpiString is used to display this information as part
//     of the application UI.
// </summary>
void
CSampleDynamicDpiWindow::RebuildCurrentDpiString()
{
    UINT dpix(0), dpiy(0);

    auto monitor = MonitorFromWindow(m_hWnd,MONITOR_DEFAULTTONEAREST);
    
    DX::ThrowIfFailed(GetDpiForMonitor(monitor, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI, &dpix, &dpiy));
    auto windowDpi = static_cast<float>(dpix);

    auto hdc = ::GetDC(NULL);
    auto desktopDpi = GetDeviceCaps(hdc,LOGPIXELSX);
    ::ReleaseDC(NULL,hdc);

    m_dpiString.append(wstring(L"Windows 8.1 DPI mode: "));
    wstringstream currentWindowDpi;
    currentWindowDpi << (windowDpi/96.0F*100);
    m_dpiString.append(currentWindowDpi.str());
    m_dpiString.append(L"% ");

    m_dpiString.append(wstring(L"Desktop System DPI:"));

    float desktopScaleFactorFloat = (static_cast<float>(desktopDpi) / 96.0F) * 100;
    wstringstream desktopScaleFactor;
    desktopScaleFactor << desktopScaleFactorFloat;
    m_dpiString.append(desktopScaleFactor.str());

    m_dpiString.append(L"%");
}

float
CSampleDynamicDpiWindow::GetDpiForWindow()
{
    auto monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

	UINT newDpiX;
	UINT newDpiY;
	if (FAILED(GetDpiForMonitor(monitor, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI, &newDpiX, &newDpiY)))
	{
		newDpiX = 96;
		newDpiX = 96;
	}
    return static_cast<float>(newDpiX);
}
