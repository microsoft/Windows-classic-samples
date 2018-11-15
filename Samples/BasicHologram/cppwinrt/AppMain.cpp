// AppMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AppMain.h"

#include <HolographicSpaceInterop.h>
#include <windows.graphics.holographic.h>
#include <winrt\Windows.Graphics.Holographic.h>

using namespace BasicHologram;

using namespace concurrency;
using namespace std::placeholders;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Core;

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    winrt::init_apartment();

    App app;

    // Initialize global strings, and perform application initialization.
    app.Initialize(hInstance);

    // Create the HWND and the HolographicSpace.
    app.CreateWindowAndHolographicSpace(hInstance, nCmdShow);

    // Main message loop:
    app.Run(hInstance);

    // Perform application teardown.
    app.Uninitialize();

    return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM App::MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &this->WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BASICHOLOGRAMMAIN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_BASICHOLOGRAMMAIN);
    wcex.lpszClassName = m_szWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

// The first method called when the IFrameworkView is being created.
// Use this method to subscribe for Windows shell events and to initialize your app.
void App::Initialize(HINSTANCE hInstance)
{
    // Initialize global strings
    //LoadStringW(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_WINDOWSPROJECT5, m_szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // At this point we have access to the device and we can create device-dependent
    // resources.
    m_deviceResources = std::make_shared<DX::DeviceResources>();

    m_main = std::make_unique<BasicHologramMain>(m_deviceResources);
}


//
//   FUNCTION: CreateWindowAndHolographicSpace(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle, creates main window, and creates HolographicSpace
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create the main program window. We also create the HolographicSpace.
//
void App::CreateWindowAndHolographicSpace(HINSTANCE hInstance, int nCmdShow)
{
    // Store the instance handle in our class variable.
    m_hInst = hInstance;

    // Create the window for the HolographicSpace.
    hWnd = CreateWindowW(
        m_szWindowClass, 
        m_szTitle,
        WS_VISIBLE,
        CW_USEDEFAULT, 
        0, 
        CW_USEDEFAULT, 
        0, 
        nullptr, 
        nullptr, 
        hInstance, 
        nullptr);

    if (!hWnd)
    {
        winrt::check_hresult(E_FAIL);
    }

    {
        // Use WinRT factory to create the holographic space.
        using namespace winrt::Windows::Graphics::Holographic;
        winrt::com_ptr<IHolographicSpaceInterop> holographicSpaceInterop = winrt::get_activation_factory<HolographicSpace, IHolographicSpaceInterop>();
        winrt::com_ptr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
        winrt::check_hresult(holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace)));

        if (!spHolographicSpace)
        {
            winrt::check_hresult(E_FAIL);
        }

        // Store the holographic space.
        m_holographicSpace = spHolographicSpace.as<HolographicSpace>();
    }

    // The DeviceResources class uses the preferred DXGI adapter ID from the holographic
    // space (when available) to create a Direct3D device. The HolographicSpace
    // uses this ID3D11Device to create and manage device-based resources such as
    // swap chains.
    m_deviceResources->SetHolographicSpace(m_holographicSpace);

    // The main class uses the holographic space for updates and rendering.
    m_main->SetHolographicSpace(hWnd, m_holographicSpace);

    // Show the window. This will activate the holographic view and switch focus to the app in Windows Mixed Reality.
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        OutputDebugStringA("Destroy\n");
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
//  FUNCTION: Run(HINSTANCE hInstance)
//
//  PURPOSE: Runs the Windows message loop and game loop.
//
int App::Run(HINSTANCE hInstance)
{
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BASICHOLOGRAMMAIN));

    MSG msg { };

    // Main message loop
    bool isRunning = true;
    while (isRunning)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                isRunning = false;
            }
            else if (msg.message == WM_LBUTTONDOWN)
            {
                m_main->OnPointerPressed();
            }
            else if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (m_windowVisible && (m_holographicSpace != nullptr))
            {
                HolographicFrame holographicFrame = m_main->Update();

                if (m_main->Render(holographicFrame))
                {
                    // The holographic frame has an API that presents the swap chain for each
                    // HolographicCamera.
                    m_deviceResources->Present(holographicFrame);
                }
                else
                {
                    Sleep(10);
                }
            }
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: Uninitialize()
//
//  PURPOSE: Tear down app resources.
//
void App::Uninitialize()
{
    m_main.reset();
    m_deviceResources.reset();
}
