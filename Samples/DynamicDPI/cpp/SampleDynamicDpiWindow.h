//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SampleDesktopWindow.h"
#include "SampleMultiDPIImage.h"
#include "SampleElementTextBlock.h"
#include <ShellScalingApi.h>
#include "SampleEngine.h"
#include "SampleRichEditControl.h"

#define RICH_EDIT_X_OFFSET  250

class CSampleDynamicDpiWindow : public CSampleDesktopWindow, public IDeviceNotify
{
public:
    CSampleDynamicDpiWindow();
    ~CSampleDynamicDpiWindow();

    // IDeviceNotify Interface functions.
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    // Initializes member variables and objects required by CSampleDynamicDpiWindow class.
    HRESULT Initialize(_In_ RECT bounds, _In_ std::wstring title);

protected:
    // Create and release resources required for Dx functionality.
    virtual void CreateDeviceIndependentResources();
    virtual void ReleaseDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void ReleaseDeviceResources();

    // Extends drawing functionality provided by base Desktop Window class.
    virtual void Draw();

    // Extend message handlers provided by base Desktop Window class.
    virtual void OnPointerUp(float _In_ x, float _In_ y);
    virtual void OnPointerDown(float _In_ x, float _In_ y);
    virtual void OnPointerUpdate(float _In_ x, float _In_ y);
    virtual void OnDisplayChange();
    virtual void OnDpiChange(int _In_ Dpi, _In_ LPRECT newRect);

private:
    std::shared_ptr<CSampleMultiDPIImage> m_bitmap;
    std::shared_ptr<CSampleElementTextBlock> m_Title;
    std::shared_ptr<CSampleElementTextBlock> m_CurrentDpi;
    std::wstring m_dpiString;
    std::wstring m_resourcePath;
    void RebuildCurrentDpiString();

    CSampleRichEditWindow m_RichEditWindow;
    CSampleEngine m_engine;

private:
	RECT CalcWindowRectNewDpi(_In_ RECT oldRect, _In_ float oldDpi, _In_ float newDPI);
    FLOAT GetNewDpi(RECT rect);
    float GetDpiForWindow();
};