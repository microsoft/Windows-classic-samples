#pragma once

#include "resource.h"

#include "Common/DeviceResources.h"
#include "BasicHologramMain.h"

namespace BasicHologram
{
    // IFrameworkView class. Connects the app with the Windows shell and handles application lifecycle events.
    class App sealed
    {
    public:
        void Initialize(HINSTANCE hInstance);
        void CreateWindowAndHolographicSpace(HINSTANCE hInstance, int nCmdShow);
        int  Run(HINSTANCE hInstance);
        void Uninitialize();

        static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    private:
        ATOM MyRegisterClass(HINSTANCE hInstance);
        void ProcessEvents(uint32_t numberOfEvents);

        std::unique_ptr<BasicHologramMain> m_main;

        HWND hWnd;

        std::shared_ptr<DX::DeviceResources>                    m_deviceResources;
        bool                                                    m_windowClosed = false;
        bool                                                    m_windowVisible = true;

        HINSTANCE m_hInst;                                // current instance
        LPCWSTR m_szTitle = L"Windows Mixed Reality Win32 App";
        LPCWSTR m_szWindowClass = L"Windows Mixed Reality Win32 App"; // The title bar text

        // The holographic space the app will use for rendering.
        winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace = nullptr;
    };
}
