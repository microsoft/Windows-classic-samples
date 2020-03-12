// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "DirectComposition_LayeredChildWindow.h"

CApplication *CApplication::s_application           = nullptr;
CComPtr<IMFPMediaPlayer> CApplication::s_pPlayer    = nullptr;             // The MFPlay player object

BOOL m_bHasVideo = FALSE;

const float CApplication::s_fanimationTime = 6.0;     // 6 seconds animation

CApplication::CApplication(HINSTANCE instance) :
    m_hInstance(instance),
    m_bControlOn(TRUE),
    m_hMainWindow(NULL),
    m_hControlChildWindow(NULL),
    m_hVideoChildWindow(NULL),
    m_hTextChildWindow(NULL)
{
    s_application = this;
}

CApplication::~CApplication()
{
    s_application = nullptr;
}

// Provides the entry point to the application.
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE,  _In_ LPWSTR, _In_ INT)
{
    CApplication application(hInstance);
    return application.Run();
}

int CApplication::Run()
{
    int result = 0;

    if (SUCCEEDED(Initialize()))
    {
       result = EnterMessageLoop();
    }

    else
    {
       MessageBoxW(NULL, L"An error occuring when running the sample", NULL, MB_OK);
    }

    Destroy();

    return result;
}

//------------------------------------------------------
// Initialization
//------------------------------------------------------

HRESULT CApplication::Initialize()
{
    HRESULT hr = InitializeMainWindow();

    if (SUCCEEDED(hr))
    {
        hr = InitializeLayeredChildWindows();
    }

    if (SUCCEEDED(hr))
    {
        hr = MoveLayeredChildWindows();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateD3D11Device();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionDevice();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionRenderTarget();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDCompositionVisualTree();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateTransforms();
    }

    if (SUCCEEDED(hr))
    {
        // Commit the batch.
        hr = m_pDevice->Commit();
    }

    return hr;
}

// Creates the main application window.
HRESULT CApplication::InitializeMainWindow()
{
    HRESULT hr = S_OK;

    // Register the window class.
    WNDCLASSEX wc     = {0};
    wc.cbSize         = sizeof(wc);
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WindowProc;
    wc.hInstance      = m_hInstance;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName  = "DirectComposition Window Class";

    RegisterClassEx(&wc);

    // Creates the m_hMainWindow window.
    m_hMainWindow = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,                          // Extended window style
                                   wc.lpszClassName,                                // Name of window class
                                   "DirectComposition Layered Child Window Sample", // Title-bar string
                                   WS_OVERLAPPED | WS_SYSMENU,                      // Top-level window
                                   CW_USEDEFAULT,                                   // Horizontal position
                                   CW_USEDEFAULT,                                   // Vertical position
                                   1000,                                            // Width
                                   700,                                             // Height
                                   NULL,                                            // Parent
                                   NULL,                                            // Class menu
                                   GetModuleHandle(NULL),                           // Handle to application instance
                                   NULL                                             // Window-creation data
                                   );

    if (!m_hMainWindow)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        ShowWindow(m_hMainWindow, SW_SHOWDEFAULT);

        WCHAR fontTypeface[32] = { 0 };

        hr = !LoadStringW(m_hInstance, IDS_FONT_TYPEFACE, fontTypeface, ARRAYSIZE(fontTypeface)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopyW(m_fontTypeface, ARRAYSIZE(m_fontTypeface), fontTypeface);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightLogo[32] = { 0 };

        hr = !LoadStringW(m_hInstance, IDS_FONT_HEIGHT_LOGO, fontHeightLogo, ARRAYSIZE(fontHeightLogo)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            m_fontHeightLogo = _wtoi(fontHeightLogo);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightTitle[32] = { 0 };

        hr = !LoadStringW(m_hInstance, IDS_FONT_HEIGHT_TITLE, fontHeightTitle, ARRAYSIZE(fontHeightTitle)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            m_fontHeightTitle = _wtoi(fontHeightTitle);
        }
    }

    if (SUCCEEDED(hr))
    {
        WCHAR fontHeightDescription[32] = { 0 };

        hr = !LoadStringW(m_hInstance, IDS_FONT_HEIGHT_DESCRIPTION, fontHeightDescription, ARRAYSIZE(fontHeightDescription)) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            m_fontHeightDescription = _wtoi(fontHeightDescription);
        }
    }

    return hr;
}

// Creates the layered child windows.
HRESULT CApplication::InitializeLayeredChildWindows()
{
    HRESULT hr = S_OK;

    // Register the window class.
    WNDCLASSEX wcex     = {0};
    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowProc;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wcex.lpszClassName  = "DirectCompositionChildWindow-Child";

    RegisterClassEx(&wcex);

    // Create the playback control child window.
    m_hControlChildWindow = CreateWindowEx(WS_EX_LAYERED,                           // Extended window style
                                           wcex.lpszClassName,                      // Name of window class
                                           NULL,                                    // Title-bar string
                                           WS_CHILD | WS_CLIPSIBLINGS,              // Child window
                                           0, 0, 0, 0,                              // Window will be resized via MoveWindow
                                           m_hMainWindow,                           // Parent
                                           NULL,                                    // Class menu
                                           GetModuleHandle(NULL),                   // Handle to application instance
                                           NULL);                                   // Window-creation data

    if (!m_hControlChildWindow)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        SetLayeredWindowAttributes(m_hControlChildWindow, 0, 130, LWA_ALPHA);

        // Create the text child window.
        m_hTextChildWindow = CreateWindowEx(WS_EX_LAYERED,
                                            wcex.lpszClassName,
                                            NULL,
                                            WS_CHILD | WS_CLIPSIBLINGS,
                                            0, 0, 0, 0,
                                            m_hMainWindow,
                                            NULL,
                                            GetModuleHandle(NULL),
                                            NULL);

        if (!m_hTextChildWindow)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        SetLayeredWindowAttributes(m_hTextChildWindow, 0, 130, LWA_ALPHA);

        // Create the video child window.
        m_hVideoChildWindow = CreateWindowEx(WS_EX_LAYERED,
                                             wcex.lpszClassName,
                                             NULL,
                                             WS_CHILD | WS_CLIPSIBLINGS,
                                             0, 0, 0, 0,
                                             m_hMainWindow,
                                             NULL,
                                             GetModuleHandle(NULL),
                                             NULL);

        if (!m_hVideoChildWindow)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Enable visual styles for buttons.
    InitCommonControls();

    if (SUCCEEDED(hr))
    {
        SetLayeredWindowAttributes(m_hVideoChildWindow, 0, 255, LWA_ALPHA);

        CloakWindow(TRUE, m_hVideoChildWindow);

        // Create Play/Stop button.
        m_hwndButton[0] = CreateWindowEx(WS_EX_TOPMOST,
                                         "BUTTON",
                                         "Play/Stop",
                                         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                         50,
                                         50,
                                         80,
                                         30,
                                         m_hControlChildWindow,
                                         (HMENU)ID_PLAYSTOP,
                                         GetModuleHandle(NULL),
                                         NULL);

        if (!m_hwndButton[0])
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Create Rotate button.
        m_hwndButton[1] = CreateWindowEx(WS_EX_TOPMOST,
                                         "BUTTON",
                                         "Rotate",
                                         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                         180,
                                         50,
                                         80,
                                         30,
                                         m_hControlChildWindow,
                                         (HMENU)ID_ROTATE,
                                         GetModuleHandle(NULL),
                                         NULL);

        if (!m_hwndButton[1])
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Create Scale button.
        m_hwndButton[2] = CreateWindowEx(WS_EX_TOPMOST,
                                         "BUTTON",
                                         "Scale",
                                         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                         310,
                                         50,
                                         80,
                                         30,
                                         m_hControlChildWindow,
                                         (HMENU)ID_SCALE,
                                         GetModuleHandle(NULL),
                                         NULL);

        if (!m_hwndButton[2])
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Create Skew button.
        m_hwndButton[3] = CreateWindowEx(WS_EX_TOPMOST,
                                         "BUTTON",
                                         "Skew",
                                         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                         440,
                                         50,
                                         80,
                                         30,
                                         m_hControlChildWindow,
                                         (HMENU)ID_SKEW,
                                         GetModuleHandle(NULL),
                                         NULL);

        if (!m_hwndButton[3])
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        ShowWindow(m_hControlChildWindow, SW_SHOWDEFAULT);
        ShowWindow(m_hTextChildWindow, SW_SHOWDEFAULT);
        ShowWindow(m_hVideoChildWindow, SW_SHOWDEFAULT);
    }

    return hr;
}

HRESULT CApplication::MoveLayeredChildWindows()
{
    HRESULT hr = S_OK;

    RECT rcClient;
    UINT ichildX;     // Position of the left side of the window
    UINT ichildY;     // Position of the top side of the window
    UINT ichildcX;    // Width of the window
    UINT ichildcY;    // Height of the window

    GetClientRect(m_hMainWindow, &rcClient);

    ichildX = rcClient.left;            // Takes the value of the left side of the main window
    ichildY = rcClient.bottom - 120;    // Takes the value of 120 pixels above the bottom of the main window
    ichildcX = rcClient.right;          // Takes the value of the right side of the main window
    ichildcY = 120;                     // Constant height of 120 pixels

    // Move the Control window to its location.
    if (!MoveWindow(m_hControlChildWindow,  // Window
                    ichildX,                // New position of the left side of the window
                    ichildY,                // New position of the top side of the window
                    ichildcX,               // New width
                    ichildcY,               // New height
                    TRUE))                  // Repaint the window
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Move the Text window to its location.
        if (!MoveWindow(m_hTextChildWindow,
                        ichildX,
                        0,
                        ichildcX,
                        ichildcY,
                        TRUE))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Move the Video window to its location.
        if(!MoveWindow(m_hVideoChildWindow,
                       ichildX,
                       rcClient.top,
                       ichildcX,
                       rcClient.bottom,
                       TRUE))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

HRESULT CApplication::CreateD3D11Device()
{
    HRESULT hr = S_OK;

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };

    D3D_FEATURE_LEVEL featureLevelSupported;

    for (int i = 0; i < sizeof(driverTypes) / sizeof(driverTypes[0]); ++i)
    {
        CComPtr<ID3D11Device> d3d11Device;
        CComPtr<ID3D11DeviceContext> d3d11DeviceContext;

        hr = D3D11CreateDevice(
            nullptr,
            driverTypes[i],
            NULL,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            NULL,
            0,
            D3D11_SDK_VERSION,
            &d3d11Device,
            &featureLevelSupported,
            &d3d11DeviceContext);

        if (SUCCEEDED(hr))
        {
            _d3d11Device = d3d11Device.Detach();
            _d3d11DeviceContext = d3d11DeviceContext.Detach();

            break;
        }
    }

    return hr;
}

HRESULT CApplication::CreateDCompositionDevice()
{
    HRESULT hr = (_d3d11Device == nullptr) ? E_UNEXPECTED : S_OK;

    CComPtr<IDXGIDevice> dxgiDevice;

    if (SUCCEEDED(hr))
    {
        hr = _d3d11Device->QueryInterface(&dxgiDevice);
    }

    if (SUCCEEDED(hr))
    {
        hr = DCompositionCreateDevice(dxgiDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void **>(&m_pDevice));
    }

    return hr;
}

HRESULT CApplication::CreateDCompositionRenderTarget()
{
    HRESULT hr = ((m_pDevice == nullptr) || (m_hMainWindow == NULL)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        // FALSE puts the composition content beneath the Win32 buttons.
        hr = m_pDevice->CreateTargetForHwnd(m_hMainWindow, FALSE, &m_pHwndRenderTarget);
    }

    return hr;
}

HRESULT CApplication::CreateDCompositionVisualTree()
{
    HRESULT hr = ((m_pDevice == nullptr) || (m_hMainWindow == NULL)) ? E_UNEXPECTED : S_OK;

    if (SUCCEEDED(hr))
    {
        // Create the root visual.
        hr = m_pDevice->CreateVisual(&m_pRootVisual);
    }

    if (SUCCEEDED(hr))
    {
        // Make the visual the root of the tree.
        hr = m_pHwndRenderTarget->SetRoot(m_pRootVisual);
    }

    if (SUCCEEDED(hr))
    {
        // Create the control child visual.
        hr = m_pDevice->CreateVisual(&m_pControlChildVisual);
    }

    if (SUCCEEDED(hr))
    {
        // Create the text child visual.
        hr = m_pDevice->CreateVisual(&m_pTextChildVisual);
    }

    if (SUCCEEDED(hr))
    {
        // Create the Video child visual.
        hr = m_pDevice->CreateVisual(&m_pVideoChildVisual);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateSurfaceFromHwnd(m_hControlChildWindow, &m_pControlsurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateSurfaceFromHwnd(m_hTextChildWindow, &m_pTextsurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateSurfaceFromHwnd(m_hVideoChildWindow, &m_pVideosurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        // Set the content of the Control child visual.
        hr = m_pControlChildVisual->SetContent(m_pControlsurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        // Set the content of the Text child visual.
        hr = m_pTextChildVisual->SetContent(m_pTextsurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        // Set the content of the video child visual.
        hr = m_pVideoChildVisual->SetContent(m_pVideosurfaceTile);
    }

    if (SUCCEEDED(hr))
    {
        // Add the video child visual to the visual tree.
        hr = m_pRootVisual->AddVisual(m_pVideoChildVisual, TRUE, NULL);
    }

    if (SUCCEEDED(hr))
    {
        // Add the text child visual to the visual tree.
        hr = m_pRootVisual->AddVisual(m_pTextChildVisual, TRUE, m_pVideoChildVisual);
    }

    if (SUCCEEDED(hr))
    {
        // Add the control child visual to the visual tree.
        hr = m_pRootVisual->AddVisual(m_pControlChildVisual, TRUE, m_pTextChildVisual);
    }

    return hr;
}

HRESULT CApplication::CreateTransforms()
{
    RECT rcClient;

    GetClientRect(m_hMainWindow, &rcClient);

    // Create a translate transform for the control child visual.
    HRESULT hr = m_pDevice->CreateTranslateTransform(&m_pControlTranslateTransform);

    if (SUCCEEDED(hr))
    {
        // Set the offset of x-axis of the control child visual
        hr = m_pControlChildVisual->SetOffsetX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the offset of y-axis of the control child visual
        hr = m_pControlChildVisual->SetOffsetY((float)rcClient.bottom - 120);
    }

    if (SUCCEEDED(hr))
    {
        // Set the offset of x-axis of control translate transform
        hr = m_pControlTranslateTransform->SetOffsetX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the offset of y-axis of control translate transform
        hr = m_pControlTranslateTransform->SetOffsetY(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Apply translate transform to the control child visual.
        hr = m_pControlChildVisual->SetTransform(m_pControlTranslateTransform);
    }

    // Create a translate transform for the text child visual.
    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateTranslateTransform(&m_pTextTranslateTransform);
    }

    if (SUCCEEDED(hr))
    {
        // Set the offset of x-axis of text translate transform
        hr = m_pTextTranslateTransform->SetOffsetX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the offset of y-axis of text translate transform
        hr = m_pTextTranslateTransform->SetOffsetY(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Apply translate transform to the text child visual.
        hr = m_pTextChildVisual->SetTransform(m_pTextTranslateTransform);
    }

    //Create a transform group (rotate, scale, skew) for the video child visual
    if (SUCCEEDED(hr))
    {
        // Create a rotate transform.
        hr = m_pDevice->CreateRotateTransform(&m_pRotateTransform);
    }

    if (SUCCEEDED(hr))
    {
        // Set the initial angle for rotation transform.
        hr = m_pRotateTransform->SetAngle(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the center of x-axis for rotation transform.
        hr = m_pRotateTransform->SetCenterX(rcClient.right / 2.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the center of y-axis for rotation transform.
        hr = m_pRotateTransform->SetCenterY(rcClient.bottom/ 2.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Create a scale transform.
        hr = m_pDevice->CreateScaleTransform(&m_pScaleTransform);
    }

    if (SUCCEEDED(hr))
    {
        // Set the initial scale x-value for scale transform.
        hr = m_pScaleTransform->SetScaleX(1.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the initial scale y-value for scale transform.
        hr = m_pScaleTransform->SetScaleY(1.0f);
    }


    if (SUCCEEDED(hr))
    {
        // Set the center of x-axis for scale transform.
        hr = m_pScaleTransform->SetCenterX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the center of y-axis for scale transform.
        hr = m_pScaleTransform->SetCenterY(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Create a skew transform.
        hr = m_pDevice->CreateSkewTransform(&m_pSkewTransform);
    }

    if (SUCCEEDED(hr))
    {
        // Set the initial horizontal angle for skew transform.
        hr = m_pSkewTransform->SetAngleX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the initial vertical angle for skew transform.
        hr = m_pSkewTransform->SetAngleY(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the center of x-axis for skew transform.
        hr = m_pSkewTransform->SetCenterX(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Set the center of y-axis for skew transform.
        hr = m_pSkewTransform->SetCenterY(0.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Create a transform group.
        IDCompositionTransform *pTransformArray[] = { m_pRotateTransform, m_pScaleTransform, m_pSkewTransform };
        hr = m_pDevice->CreateTransformGroup(pTransformArray, 3, &m_pTransformGroup);
    }

    if (SUCCEEDED(hr))
    {
        // Apply transform group to the video child visual.
        hr = m_pVideoChildVisual->SetTransform(m_pTransformGroup);
    }

    return hr;
}

//------------------------------------------------------
// Direct Composition Transforms
//------------------------------------------------------

HRESULT CApplication::SetTranslation(float beginPositionX, float endPositionX, float beginPositionY, float endPositionY, float beginTime, float endTime, IDCompositionTranslateTransform *translateTransform)
{
    CComPtr<IDCompositionAnimation> offsetXAnimation;

    HRESULT hr = CreateTranslateAnimation(beginPositionX, endPositionX, beginTime, endTime, &offsetXAnimation);

    if (SUCCEEDED(hr))
    {
        hr = translateTransform->SetOffsetX(offsetXAnimation);
    }

    CComPtr<IDCompositionAnimation> offsetYAnimation;

    if (SUCCEEDED(hr))
    {
        hr = CreateTranslateAnimation(beginPositionY, endPositionY, beginTime, endTime, &offsetYAnimation);
    }

    if (SUCCEEDED(hr))
    {
        hr = translateTransform->SetOffsetY(offsetYAnimation);
    }

    return hr;
}

HRESULT CApplication::OnRotate()
{
    HRESULT hr = SetRotation(360.0f, 0.0f, 3.0f, m_pRotateTransform);

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Commit();
    }

    return hr;
}

HRESULT CApplication::SetRotation(float rotationAngle, float beginTime, float endTime, IDCompositionRotateTransform *rotateTransform)
{
    CComPtr<IDCompositionAnimation> angleAnimation;

    HRESULT hr = CreateLinearAnimation(0.0f, rotationAngle, beginTime, endTime, &angleAnimation);

    if (SUCCEEDED(hr))
    {
        hr = rotateTransform->SetAngle(angleAnimation);
    }

    return hr;
}

HRESULT CApplication::OnScale()
{
    HRESULT hr = SetScaling(1.0f, 1.0f, -0.1f, -0.1f, 0.0f, 2.0f, m_pScaleTransform);

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Commit();
    }

    return hr;
}

HRESULT CApplication::SetScaling(float beginScaleX, float beginScaleY, float endScaleX, float endScaleY, float beginTime, float endTime, IDCompositionScaleTransform *scaleTransform)
{
    CComPtr<IDCompositionAnimation> scaleAnimationX;

    HRESULT hr = CreateLinearAnimation(beginScaleX, endScaleX, beginTime, endTime, &scaleAnimationX);

    if (SUCCEEDED(hr))
    {
        hr = scaleTransform->SetScaleX(scaleAnimationX);
    }

    CComPtr<IDCompositionAnimation> scaleAnimationY;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(beginScaleY, endScaleY, beginTime, endTime, &scaleAnimationY);
    }

    if (SUCCEEDED(hr))
    {
        hr = scaleTransform->SetScaleY(scaleAnimationY);
    }

    return hr;
}

HRESULT CApplication::OnSkew()
{
    HRESULT hr = SetSkewing(90.0f, 0.0f, 0.0f, 2.0f, m_pSkewTransform);

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Commit();
    }

    return hr;
}

HRESULT CApplication::SetSkewing(float skewAngleX, float skewAngleY, float beginTime, float endTime, IDCompositionSkewTransform *skewTransform)
{
    CComPtr<IDCompositionAnimation> skewAnimationX;

    HRESULT hr = CreateLinearAnimation(0.0f, skewAngleX, beginTime, endTime, &skewAnimationX);

    if (SUCCEEDED(hr))
    {
        hr = skewTransform->SetAngleX(skewAnimationX);
    }

    CComPtr<IDCompositionAnimation> skewAnimationY;

    if (SUCCEEDED(hr))
    {
        hr = CreateLinearAnimation(0.0f, skewAngleY, beginTime, endTime, &skewAnimationY);
    }

    if (SUCCEEDED(hr))
    {
        hr = skewTransform->SetAngleY(skewAnimationY);
    }

    return hr;
}

HRESULT CApplication::CreateLinearAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **linearAnimation)
{
    HRESULT hr = (linearAnimation == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *linearAnimation = nullptr;

        hr = (m_pDevice == nullptr) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionAnimation> animation;

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateAnimation(&animation);
    }

    // Ensures animation start value takes effect immediately
    if (SUCCEEDED(hr))
    {
        if (beginTime > 0.0)
        {
            hr = animation->AddCubic(0.0, beginValue, 0.0f, 0.0f, 0.0f);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->AddCubic(beginTime, beginValue, (endValue - beginValue) / (endTime - beginTime), 0.0f, 0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->AddCubic(endTime, endValue, -(endValue - beginValue) / (endTime - beginTime), 0.0f, 0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->End(2* endTime, beginValue);
    }

    if (SUCCEEDED(hr))
    {
        *linearAnimation = animation.Detach();
    }

    return hr;
}

HRESULT CApplication::CreateTranslateAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **linearAnimation)
{
    HRESULT hr = (linearAnimation == nullptr) ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        *linearAnimation = nullptr;

        hr = (m_pDevice == nullptr) ? E_UNEXPECTED : S_OK;
    }

    CComPtr<IDCompositionAnimation> animation;

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->CreateAnimation(&animation);
    }

    // Ensures animation start value takes effect immediately
    if (SUCCEEDED(hr))
    {
        if (beginTime > 0.0)
        {
            hr = animation->AddCubic(0.0, beginValue, 0.0f, 0.0f, 0.0f);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->AddCubic(beginTime, beginValue, (endValue - beginValue) / (endTime - beginTime), 0.0f, 0.0f);
    }

    if (SUCCEEDED(hr))
    {
        hr = animation->End(endTime, endValue);
    }

    if (SUCCEEDED(hr))
    {
        *linearAnimation = animation.Detach();
    }

    return hr;
}

int CApplication::EnterMessageLoop()
{
    int result = 0;

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    result = static_cast<int>(msg.wParam);

    return result;
}

//------------------------------------------------------
// In Action
//------------------------------------------------------

// Main window procedure
LRESULT CALLBACK CApplication::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        case WM_COMMAND:
            result = s_application->OnCommand(wParam);
            break;

        case WM_RBUTTONUP:
            result = s_application->OnRightClick();
            break;

        case WM_TIMER:
            result = s_application->OnTimer();
            break;

        case WM_PAINT:
            result = s_application->OnPaint(hwnd);
            break;

        case WM_CLOSE:
            result = s_application->OnClose(hwnd);
            break;

        case WM_DESTROY:
            result = s_application->OnDestroy(hwnd);
            break;

        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

// Handles the WM_COMMAND message.
HRESULT CApplication::OnCommand(int id)
{
    HRESULT hr = S_OK;
    IDCompositionAnimation* pAnimation = NULL;

    switch (id)
    {
        case ID_ROTATE:
            hr = s_application->OnRotate();
            break;

        case ID_SCALE:
            hr = s_application->OnScale();
            break;

        case ID_SKEW:
            hr = s_application->OnSkew();
            break;

        case ID_PLAYSTOP:
            // Play/Stop the video.
            if (!s_pPlayer)
            {
                PlayMediaFile();
            }
            else
            {
                MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;

                hr = s_pPlayer->GetState(&state);

                if (SUCCEEDED(hr))
                {
                    if (state == MFP_MEDIAPLAYER_STATE_PAUSED || state == MFP_MEDIAPLAYER_STATE_STOPPED)
                    {
                        hr = s_pPlayer->Play();
                    }
                    else if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
                    {
                        hr = s_pPlayer->Pause();
                    }
                }
            }
            break;
    }

    return hr;
}

// Handles the WM_RBUTTONDOWN message.
HRESULT CApplication::OnRightClick()
{
    HRESULT hr = S_OK;
    IDCompositionAnimation* pAnimation = NULL;

    if (m_bControlOn)
    {
        // Kill the timer.
        KillTimer(m_hControlChildWindow, IDT_TIMER);

        // Animate the Control window.
        // Adjust the opacity after the Control window is cloaked.
        SetLayeredWindowAttributes(m_hControlChildWindow, 0, 130, LWA_ALPHA);

        // Cloak the Control window.
        CloakWindow(TRUE, m_hControlChildWindow);

        // Hide the Control window. - Animating the control window offscreen
        // Set the sliding in animation.
        hr = SetTranslation(0.0f, 0.0f, 0.0f, 300.f, 0.0f, 2.0f, m_pControlTranslateTransform);

        if (SUCCEEDED(hr))
        {
            // Animate the Text window.
            // Cloak the Text window.
            CloakWindow(TRUE, m_hTextChildWindow);

            // Hide the text window. - Animating the text window offscreen
            // Set the sliding in animation.
            hr = SetTranslation(0.0f, 0.0f, 0.0f, -300.f, 0.0f, 2.0f, m_pTextTranslateTransform);
        }

        if (SUCCEEDED(hr))
        {
            m_bControlOn = false;
        }
    }
    else
    {
        // Animate the Control window.
        // Show the Control window. - animating the control window on screen

        // Set the sliding out animation.
        hr = SetTranslation(0.0f, 0.0f, 140.0f, 0.0f, 0.0f, 1.0f, m_pControlTranslateTransform);

        if (SUCCEEDED(hr))
        {
            // Animate the Text window.
            // Show the Text window. - animating the text window on screen
            hr = SetTranslation(0.0f, 0.0f, -140.0f, 0.0f, 0.0f, 1.0f, m_pTextTranslateTransform);
        }

        if (SUCCEEDED(hr))
        {
            m_bControlOn = true;

            // Timer is used to dertermine DirectComposition end animation time.
            // Kill and then set the timer.
            KillTimer(m_hControlChildWindow, IDT_TIMER);
            SetTimer(m_hControlChildWindow, IDT_TIMER, UINT((1000.0f/s_fanimationTime)*s_fanimationTime), (TIMERPROC)NULL);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDevice->Commit();
    }

    return hr;
}

// Handles the WM_TIMER message.
HRESULT CApplication::OnTimer()
{
    // Adjust the opacity after the Control window is uncloaked.
    SetLayeredWindowAttributes(m_hControlChildWindow, 0, 130, LWA_ALPHA);

    // Uncloak the Control window
    CloakWindow(FALSE, m_hControlChildWindow);

    // Uncloak the Text window
    CloakWindow(FALSE, m_hTextChildWindow);

    // Kill the timer.
    KillTimer(m_hControlChildWindow, IDT_TIMER);

    return S_OK;
}

// Handles the WM_PAINT message.
HRESULT CApplication::OnPaint(HWND hwnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HRESULT hr = S_OK;

    hdc = BeginPaint(hwnd, &ps);

    if (hwnd == m_hControlChildWindow)
    {
        // Paint the control window.
        RECT rcClient;

        // Get the dimensions of the control window.
        if (!GetClientRect(m_hControlChildWindow, &rcClient))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }

        // Drawtext on the control window.
        HFONT hinstruction = CreateFontW(m_fontHeightDescription, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, m_fontTypeface);    // Instruction Font and Size (Same as Description's)
        if (hinstruction != NULL)
        {
            HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hinstruction));

            rcClient.top = 50;
            rcClient.left = 650;

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

            DrawTextW(hdc, L"Right-click if you want to hide me.", -1, &rcClient, DT_WORDBREAK | DT_CENTER);

            SelectObject(hdc, hOldFont);

            DeleteObject(hinstruction);
        }
    }
    else if (hwnd == m_hTextChildWindow)
    {
        // Paint the text window.
        RECT rcClient;

        // Get the dimensions of the main window.
        GetClientRect(m_hMainWindow, &rcClient);

        SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

        // Logo
        HFONT hlogo = CreateFontW(m_fontHeightLogo, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, m_fontTypeface);    // Logo Font and Size
        if (hlogo != NULL)
        {
            HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hlogo));

            SetBkMode(hdc, TRANSPARENT);

            rcClient.top = 10;
            rcClient.left = 30;

            DrawTextW(hdc, L"Windows samples", -1, &rcClient, DT_WORDBREAK);

            SelectObject(hdc, hOldFont);

            DeleteObject(hlogo);
        }

        // Title
        HFONT htitle = CreateFontW(m_fontHeightTitle, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, m_fontTypeface);    // Title Font and Size
        if (htitle != NULL)
        {
            HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, htitle));

            rcClient.top = 25;
            rcClient.left = 30;

            DrawTextW(hdc, L"DirectComposition Layered Child Window Sample", -1, &rcClient, DT_WORDBREAK);

            SelectObject(hdc, hOldFont);

            DeleteObject(htitle);
        }

        // Description
        HFONT hdescription = CreateFontW(m_fontHeightDescription, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, m_fontTypeface);    // Description Font and Size
        if (hdescription != NULL)
        {
            HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hdescription));

            rcClient.top = 90;
            rcClient.left = 30;

            DrawTextW(hdc, L"This sample shows DirectComposition applying Transform properties on layered child windows based content.", -1, &rcClient, DT_WORDBREAK);

            SelectObject(hdc, hOldFont);

            DeleteObject(hdescription);
        }
    }
    else
    {
        // Paint the video window.
        if (s_pPlayer && m_bHasVideo)
        {
            // Playback has started and there is video.
            // Do not draw the window background, because the video
            // frame fills the entire client area.
            s_pPlayer->UpdateVideo();
        }
    }

    EndPaint(hwnd, &ps);
    return hr;
}

// Handles the WM_CLOSE message.
LRESULT CApplication::OnClose(HWND /*hwnd*/)
{
    // Close the MFPlay player object.
    if (s_pPlayer)
    {
        s_pPlayer->Shutdown();
        s_pPlayer = nullptr;
    }

    // Close the application callback object.
    if (s_pPlayerCB)
    {
        s_pPlayerCB = nullptr;
    }

    // Destroy the main window.
    DestroyWindow(m_hMainWindow);

    return 0;
}

// Handles the WM_DESTROY message.
LRESULT CApplication::OnDestroy(HWND /*hwnd*/)
{
    PostQuitMessage(0);

    return 0;
}

//------------------------------------------------------
// Destroy
//------------------------------------------------------

VOID CApplication::Destroy()
{
    DestroyTransforms();
    DestroyMainWindow();
    DestroyLayeredChildWindows();
    DestroyDCompositionVisualTree();
    DestroyDCompositionRenderTarget();
    DestroyDCompositionDevice();
    DestroyD3D11Device();
    CoUninitialize();
}

VOID CApplication::DestroyTransforms()
{
    m_pControlTranslateTransform = nullptr;

    m_pTextTranslateTransform = nullptr;

    m_pRotateTransform = nullptr;

    m_pSkewTransform = nullptr;

    m_pScaleTransform = nullptr;
}

VOID CApplication::DestroyMainWindow()
{
    if (m_hMainWindow != NULL)
    {
       DestroyWindow(m_hMainWindow);
       m_hMainWindow = NULL;
    }
}

VOID CApplication::DestroyLayeredChildWindows()
{
    if (m_hControlChildWindow != NULL)
    {
       DestroyWindow(m_hControlChildWindow);
       m_hControlChildWindow = NULL;
    }

    if (m_hTextChildWindow != NULL)
    {
       DestroyWindow(m_hTextChildWindow);
       m_hTextChildWindow = NULL;
    }

    if (m_hVideoChildWindow != NULL)
    {
       DestroyWindow(m_hVideoChildWindow);
       m_hVideoChildWindow = NULL;
    }
}

VOID CApplication::DestroyDCompositionVisualTree()
{
    m_pControlsurfaceTile = nullptr;

    m_pTextsurfaceTile = nullptr;

    m_pVideosurfaceTile = nullptr;
}

VOID CApplication::DestroyDCompositionRenderTarget()
{
    m_pHwndRenderTarget = nullptr;
}

VOID CApplication::DestroyDCompositionDevice()
{
    m_pDevice = nullptr;
}

VOID CApplication::DestroyD3D11Device()
{
    _d3d11DeviceContext = nullptr;
    _d3d11Device = nullptr;
}

// Cloaking removes the window from the view,
// while still allowing it to retain and update its bitmap.
HRESULT CApplication::CloakWindow(BOOL cloakHwnd, HWND hwnd)
{
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloakHwnd, sizeof(cloakHwnd));

    return hr;
}

// Show error messages.
void CApplication::ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
    WCHAR msg[MAX_PATH];
    HRESULT hr = StringCbPrintf((STRSAFE_LPSTR)msg, sizeof(msg), (STRSAFE_LPCSTR)(L"%s (hr=0x%08X)"), format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, (LPCSTR)msg, NULL, MB_ICONERROR);
    }
}

// Plays a media file, using the IMFPMediaPlayer interface.
HRESULT CApplication::PlayMediaFile()
{
    HRESULT hr = S_OK;

    // Create the MFPlayer object.
    if (s_pPlayer == nullptr)
    {
        s_pPlayerCB = new MediaPlayerCallback();

        if (s_pPlayerCB == nullptr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = MFPCreateMediaPlayer(NULL,
                                  FALSE,                // Start playback automatically?
                                  MFP_OPTION_NONE,      // Flags
                                  s_pPlayerCB,          // Callback pointer
                                  m_hVideoChildWindow,  // Video window
                                  &s_pPlayer
                                  );
    }

    // Create a new media item for this URL.
    if (SUCCEEDED(hr))
    {
        hr = s_pPlayer->CreateMediaItemFromURL(L"media/TallShip-medium.wmv", FALSE, 0, NULL);
    }

    // The CreateMediaItemFromURL method completes asynchronously.
    // The application will receive an MFP_EVENT_TYPE_MEDIAITEM_CREATED
    // event. See MediaPlayerCallback::OnMediaPlayerEvent().
    return hr;
}

// Called when the IMFPMediaPlayer::CreateMediaItemFromURL method
// completes.
void CApplication::OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent)
{
    HRESULT hr = S_OK;

    // The media item was created successfully.
    if (s_pPlayer)
    {
        BOOL bHasVideo = FALSE, bIsSelected = FALSE;

        // Check if the media item contains video.
        hr = pEvent->pMediaItem->HasVideo(&bHasVideo, &bIsSelected);

        if (FAILED(hr))
        {
            ShowErrorMessage(L"Error playing this file.", hr);
        }

        m_bHasVideo = bHasVideo && bIsSelected;

        // Set the media item on the player. This method completes asynchronously.
        s_pPlayer->SetMediaItem(pEvent->pMediaItem);
    }
}

// Called when the IMFPMediaPlayer::SetMediaItem method completes.
void CApplication::OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT * /*pEvent*/)
{
    HRESULT hr = s_pPlayer->Play();

    if (FAILED(hr))
    {
        ShowErrorMessage(L"IMFPMediaPlayer::Play failed.", hr);
    }
}

// Implements IMFPMediaPlayerCallback::OnMediaPlayerEvent.
// This callback method handles events from the MFPlay object.
void MediaPlayerCallback::OnMediaPlayerEvent(_In_ MFP_EVENT_HEADER * pEventHeader)
{
    if (FAILED(pEventHeader->hrEvent))
    {
        CApplication::ShowErrorMessage(L"Playback error", pEventHeader->hrEvent);
        return;
    }

    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        CApplication::OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        CApplication::OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
        break;
    }
}
