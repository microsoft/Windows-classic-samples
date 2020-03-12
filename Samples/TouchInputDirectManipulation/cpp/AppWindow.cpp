// Copyright (c) Microsoft Corporation. All rights reserved 

#include "stdafx.h"

#include "AppWindow.h"
#include "objbase.h"
#include <directmanipulation.h>

#define WNDCLASSNAME (L"DirectManipulationAppWindow")
#define WNDTITLE (L"Direct Manipulation Sample Application")

#define WINDOW_WIDTH (1024)
#define WINDOW_HEIGHT (768)

#define TEXTBLOCK_WIDTH (375)
#define TEXTBLOCK_HEIGHT (75)

#define OUTER_VIEWPORT_TEXT_MARGIN_X (50.0f)
#define OUTER_VIEWPORT_TEXT_MARGIN_Y (125.0f)
#define OUTER_VIEWPORT_TEXT_WIDTH (300.0f)
#define OUTER_VIEWPORT_TEXT_HEIGHT (50.0f)

#define OUTER_CONTENT_WIDTH (2000)
#define OUTER_CONTENT_HEIGHT (2000)

#define INNER_VIEWPORT_OFFSET (200)

#define BUTTON_OFFSET_X (5.0f)
#define BUTTON_OFFSET_Y (200.0f)
#define BUTTON_BORDER_WIDTH (2.0f)
#define BUTTON_TEXT_PADDING (5.0f)
#define BUTTON_WIDTH (175)
#define BUTTON_HEIGHT (40)

namespace DManipSample
{
    CAppWindow::CAppWindow()
    {
        // Populate outer content rect as it has a static size.
        _contentOuterRect.top = 0;
        _contentOuterRect.left = 0;
        _contentOuterRect.right = OUTER_CONTENT_WIDTH;
        _contentOuterRect.bottom = OUTER_CONTENT_HEIGHT;
    }

    int CAppWindow::ShowAndServiceWindow()
    {
        HRESULT hr = S_OK;

        HINSTANCE hModule = GetModuleHandle(nullptr);

        // Receive mouse events as WM_POINTER* messages
        EnableMouseInPointer(TRUE);

        WNDCLASSEX wc;
        wc.cbSize           = sizeof(wc);
        wc.lpszClassName    = WNDCLASSNAME;
        wc.lpfnWndProc      = CAppWindow::s_WndProc;
        wc.style            = CS_VREDRAW | CS_HREDRAW;
        wc.hInstance        = hModule;
        wc.hIcon            = nullptr;
        wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground    = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wc.lpszMenuName     = nullptr;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hIconSm          = nullptr;

        ATOM registeredClass = RegisterClassEx(&wc);
        if(!registeredClass)
        {
            WCHAR error[MAX_PATH] = {0};
            hr = StringCchPrintf(error, ARRAYSIZE(error), L"Failed to register class. GetLastError()=%d", GetLastError());

            if(SUCCEEDED(hr))
            {
                MessageBox(nullptr, error, L"Error", MB_OK);
            }

            return 2;
        }

        _hWnd = CreateWindow(WNDCLASSNAME,
                             WNDTITLE,
                             WS_OVERLAPPEDWINDOW,
                             0,
                             0,
                             WINDOW_WIDTH,
                             WINDOW_HEIGHT, 
                             nullptr, 
                             nullptr, 
                             hModule, 
                             this);

        if(!_hWnd)
        {
            WCHAR error[MAX_PATH] = {0};
            hr = StringCchPrintf(error, ARRAYSIZE(error), L"Could not create window. GetLastError()=%d", GetLastError());

            if(SUCCEEDED(hr))
            {
                MessageBox(nullptr, error, L"Error", MB_OK);
            }

            return 1;
        }

        hr = _Initialize();
        
        ShowWindow(_hWnd, SW_SHOW);
        UpdateWindow(_hWnd);

        RECT windowRect;
        GetWindowRect(_hWnd, &windowRect);

        if(FAILED(hr))
        {
            WCHAR error[MAX_PATH] = {0};
            hr = StringCchPrintf(error, ARRAYSIZE(error), L"Encountered error while initializing sample. HRESULT=0x%x", hr);

            if(SUCCEEDED(hr))
            {
                MessageBox(nullptr, error, L"Error", MB_OK);
            }

            return -2;
        }

        if(SUCCEEDED(hr))
        {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0) > 0)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return 0;
    }

    LRESULT CALLBACK CAppWindow::s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result = 0;

        // If we are creating the window, set the pointer to the instance of CAppWindow associated with the window as the HWND's user data.
        // That way when we get messages besides WM_CREATE we can call the instance's WndProc and reference non-static member variables.
        if(message == WM_CREATE)
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<CAppWindow*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams)));
        }
        else
        {
            CAppWindow* appWindow = reinterpret_cast<CAppWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

            if(appWindow)
            {
                result = appWindow->_WndProc(hWnd, message, wParam, lParam);
            }
            else
            {
                result = DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        return result;
    }

    LRESULT CAppWindow::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result = 0;
        HRESULT hr = S_OK;

        switch (message)
        {
        case WM_SIZE:
            {
                hr = _SizeDependentChanges();
            }
            break;
        case UWM_REDRAWSTATUS:
            {
                hr = _DrawViewportStatusText();
            }
            break;
        case WM_POINTERDOWN:
            {
                // Determine if the button was hit. If so, change the button state.
                UINT contactId = GET_POINTERID_WPARAM(wParam);
                BOOL buttonHit = FALSE;
                hr = _HitTestRect(contactId, hWnd, FALSE, BUTTON_OFFSET_X, BUTTON_OFFSET_Y, BUTTON_OFFSET_X + BUTTON_WIDTH, BUTTON_OFFSET_Y + BUTTON_HEIGHT, &buttonHit);

                if(SUCCEEDED(hr) && buttonHit)
                {
                    hr = _ChangeButtonState(_dcompVisualButtonDown);
                }
            }
            break;
        case WM_POINTERUPDATE:
            {
                // 
                // If the updated contact is not over the button, check to see if the contact is already being tracked by the outer viewport. If so, ignore it.
                //
                UINT contactId = GET_POINTERID_WPARAM(wParam);
                BOOL buttonHit = FALSE;
                BOOL outerViewportHasContact = _OuterViewportHasContact(contactId);

                if(!outerViewportHasContact)
                {
                    // Check if the button was hit.
                    hr = _HitTestRect(contactId, hWnd, FALSE, BUTTON_OFFSET_X, BUTTON_OFFSET_Y, BUTTON_OFFSET_X + BUTTON_WIDTH, BUTTON_OFFSET_Y + BUTTON_HEIGHT, &buttonHit);

                    if(SUCCEEDED(hr) && !buttonHit)
                    {
                        BOOL innerViewportHit = FALSE;
                
                        // Check if the inner viewport was hit.
                        if(SUCCEEDED(hr))
                        {
                            hr = _HitTestRect(contactId, hWnd, TRUE, static_cast<float>(_viewportInnerRect.left), static_cast<float>(_viewportInnerRect.top), static_cast<float>(_viewportInnerRect.right), static_cast<float>(_viewportInnerRect.bottom), &innerViewportHit);
                        }

                        if(SUCCEEDED(hr))
                        {
                            if(innerViewportHit)
                            {
                                hr = _viewportInner->SetContact(contactId);
                            }

                            if(SUCCEEDED(hr))
                            {
                                hr = _viewportOuter->SetContact(contactId);
                            }

                            if(SUCCEEDED(hr))
                            {
                                // Add to our list of active contacts.
                                _activeContacts.push_back(contactId);
                            }
                        }
                    }
                }
            }
            break;
        case WM_POINTERUP:
            {
                _activeContacts.remove(GET_POINTERID_WPARAM(wParam));

                if(_dcompVisualButtonCurrent == _dcompVisualButtonDown)
                {
                    hr = _ChangeButtonState(_dcompVisualButtonUpClicked);
                }
            }
            break;
        case WM_POINTERCAPTURECHANGED:
            {
                if(_dcompVisualButtonCurrent == _dcompVisualButtonDown)
                {
                    hr = _ChangeButtonState(_dcompVisualButtonUpCapturedByViewport);
                }
            }
            break;
        case WM_KEYDOWN:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            {
                DWORD targetContact = (message == WM_KEYDOWN) ? DIRECTMANIPULATION_KEYBOARDFOCUS : DIRECTMANIPULATION_MOUSEFOCUS;
                BOOL fHandled = FALSE;  
                MSG msg = { hWnd, message, wParam, lParam };  
                HRESULT setContactHr = _viewportOuter->SetContact(targetContact);  
                
                if(SUCCEEDED(setContactHr))
                {
                    hr = _manager->ProcessInput(&msg, &fHandled);
                }
                else
                {
                    hr = setContactHr;
                }

                // Call ReleaseContact if SetContact succeeded despite the result of ProcessInput. However,
                // only overwrite hr if ProcessInput succeeded. Otherwise leave the HRESULT of ProcessInput
                // in place.
                if(SUCCEEDED(setContactHr))
                {
                    HRESULT releaseContactHr = _viewportOuter->ReleaseContact(targetContact);
                    if(SUCCEEDED(hr))
                    {
                        hr = releaseContactHr;
                    }
                }
            } 
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
        default:
            result = DefWindowProc(hWnd, message, wParam, lParam);
        }

        return result;
    }

    HRESULT CAppWindow::_HitTestRect(UINT contactId, HWND hWnd, BOOL applyTransform, float left, float top, float right, float bottom, BOOL* innerViewportHit)
    {
        HRESULT hr = S_OK;

        // Translate content rect based on the current output transform of the outer viewport to see if we should call SetContact on the inner (nested) viewport.
        POINTER_INFO info = {0};
        GetPointerInfo(contactId, &info);

        POINTF topLeftFloat = {left, top};
        POINTF bottomRightFloat = {right, bottom};
            
        if(applyTransform)
        {
            // Determine the outer viewport's content transform (since it may have been translated or scaled) so it can be taken into account when doing the hit testing for the
            // inner viewport.
            float contentTransform[6] = {0};
            hr = _contentOuter->GetContentTransform(contentTransform, ARRAYSIZE(contentTransform));

            if(SUCCEEDED(hr))
            {
                // Transform the inner viewport's top left and bottom right by the outer viewport's scale factor.
                topLeftFloat.x *= contentTransform[0];
                topLeftFloat.y *= contentTransform[0];
                bottomRightFloat.x *= contentTransform[0];
                bottomRightFloat.y *= contentTransform[0];

                // Transform the inner viewport's top left and bottom right by the outer viewport's translate values. If the content transform contained a rotation value, we 
                // would need to factor the rotation in. The visuals and dmanip transforms in this sample cannot be given rotation values, simplifying the math.
                topLeftFloat.x += contentTransform[4];
                topLeftFloat.y += contentTransform[5];
                bottomRightFloat.x += contentTransform[4];
                bottomRightFloat.y += contentTransform[5];
            }
        }

        POINT topLeft = {static_cast<int>(topLeftFloat.x), static_cast<int>(topLeftFloat.y)};
        POINT bottomRight = {static_cast<int>(bottomRightFloat.x), static_cast<int>(bottomRightFloat.y)};
        ClientToScreen(hWnd, &topLeft);
        ClientToScreen(hWnd, &bottomRight);

        // Determine if the contact hit the inner viewport.
        RECT transformedRect = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
        *innerViewportHit = PtInRect(&transformedRect, info.ptPixelLocation);

        return hr;
    }

    HRESULT CAppWindow::_InitializeDevices()
    {
        HRESULT hr = S_OK;
        
        D3D_FEATURE_LEVEL featureLevels[] =   
        {  
            D3D_FEATURE_LEVEL_11_1,  
            D3D_FEATURE_LEVEL_11_0,  
            D3D_FEATURE_LEVEL_10_1,  
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1,
        };  

        hr = D3D11CreateDevice(  
            nullptr,   
            D3D_DRIVER_TYPE_HARDWARE,   
            nullptr,   
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,          
            featureLevels,   
            ARRAYSIZE(featureLevels),   
            D3D11_SDK_VERSION,  
            &_d3dDevice,   
            nullptr,  
            nullptr);

        if(FAILED(hr))
        {
            // If the device could not be created with hardware acceleration, attempt to create via WARP.
            hr = D3D11CreateDevice(  
                nullptr,   
                D3D_DRIVER_TYPE_WARP,   
                nullptr,   
                D3D11_CREATE_DEVICE_BGRA_SUPPORT,              
                featureLevels,   
                ARRAYSIZE(featureLevels),   
                D3D11_SDK_VERSION,  
                &_d3dDevice,   
                nullptr,  
                nullptr);
        }

        if(SUCCEEDED(hr))
        {
            hr = _d3dDevice.As(&_dxgiDevice);
        }
        
        if(SUCCEEDED(hr))
        {
            hr = DCompositionCreateDevice2(_dxgiDevice.Get(), IID_PPV_ARGS(&_dcompDevice));
        }
        
        if(SUCCEEDED(hr))
        {
            hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&_d2dFactory));
        }
        
        return hr;
    }

    HRESULT CAppWindow::_InitializeManagerAndViewport()
    {
        HRESULT hr = S_OK;

        //
        // Create and activate the instance of IDirectManipulationManager. This instance is used to
        // CoCreate other DirectManipulation objects and enable / disable DirectManipulation
        // per HWND.
        //
        if(SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_DirectManipulationManager,
                                nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&_manager));
        }

        ComPtr<IDirectManipulationUpdateManager> updateManager;

        if(SUCCEEDED(hr))
        {
            hr = _manager->GetUpdateManager(IID_PPV_ARGS(&updateManager));
        }
                
        //
        // Create the instance of IDirectManipulationDCompCompositor. This instance will handle transformation of
        // the dcomp visuals and provide the instance of IDirectManipulationFrameInfoProvider that will manage
        // frame timing.
        //
        if(SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_DCompManipulationCompositor,
                                nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&_compositor));
        }

        if(SUCCEEDED(hr))
        {
            hr = _compositor->SetUpdateManager(updateManager.Get());
        }

        ComPtr<IDirectManipulationFrameInfoProvider> frameInfo;

        if(SUCCEEDED(hr))
        {
            hr = _compositor.As(&frameInfo);
        }

        //
        // Create the inner and outer viewports and set their boundaries in the associated HWND.
        //
        if(SUCCEEDED(hr))
        {
            hr = _manager->CreateViewport(frameInfo.Get(), _hWnd, IID_PPV_ARGS(&_viewportOuter));
        }

        if(SUCCEEDED(hr))
        {
            hr = _manager->CreateViewport(frameInfo.Get(), _hWnd, IID_PPV_ARGS(&_viewportInner));
        }

        //
        // Enable the desired configuration for each viewport.
        //
        DIRECTMANIPULATION_CONFIGURATION targetConfiguration = DIRECTMANIPULATION_CONFIGURATION_INTERACTION
                                | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_X
                                | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_Y
                                | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_INERTIA
                                | DIRECTMANIPULATION_CONFIGURATION_RAILS_X
                                | DIRECTMANIPULATION_CONFIGURATION_RAILS_Y
                                | DIRECTMANIPULATION_CONFIGURATION_SCALING
                                | DIRECTMANIPULATION_CONFIGURATION_SCALING_INERTIA;

        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->ActivateConfiguration(targetConfiguration);
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->ActivateConfiguration(targetConfiguration);
        }

        //
        // Create instances of CViewportEventHandler for each viewport and associate the instance
        // with the corresponding viewport.
        //
        if(SUCCEEDED(hr))
        {
            _handlerOuter = Make<CViewportEventHandler>(_hWnd);

            if(_handlerOuter == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        
        if(SUCCEEDED(hr))
        {
            _handlerInner = Make<CViewportEventHandler>(_hWnd);

            if(_handlerInner == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->AddEventHandler(_hWnd, _handlerOuter.Get(), &_viewportOuterHandlerCookie);
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->AddEventHandler(_hWnd, _handlerInner.Get(), &_viewportInnerHandlerCookie);
        }

        //
        // Get the content instances for each viewport so it is available when performing hit testing and
        // rendering the associated viewport's DirectComposition content.
        //
        ComPtr<IDirectManipulationPrimaryContent> primaryContentOuter;
        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->GetPrimaryContent(IID_PPV_ARGS(&primaryContentOuter));
        }

        ComPtr<IDirectManipulationPrimaryContent> primaryContentInner;
        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->GetPrimaryContent(IID_PPV_ARGS(&primaryContentInner));
        }

        if(SUCCEEDED(hr))
        {
            hr = primaryContentOuter.As(&_contentOuter);
        }

        if(SUCCEEDED(hr))
        {
            hr = primaryContentInner.As(&_contentInner);
        }

        // Limit how far the viewports can be zoomed (scaled).
        if(SUCCEEDED(hr))
        {
            hr = primaryContentOuter->SetZoomBoundaries(1.0f, 5.0f);
        }

        if(SUCCEEDED(hr))
        {
            hr = primaryContentInner->SetZoomBoundaries(0.5f, 5.0f);
        }
        
        // Activate DirectManipulation on our target window.
        if(SUCCEEDED(hr))
        {
            hr = _manager->Activate(_hWnd);
        }

        return hr;
    }

    HRESULT CAppWindow::_InitializeDrawing()
    {
        HRESULT hr = S_OK;
        
        hr = _dcompDevice->CreateTargetForHwnd(_hWnd, TRUE, &_dcompTarget);

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualOuterParent);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualText);
        }
        
        //
        // The DirectComposition visuals will be set up in a hierarchy with parent-child order
        // _dcompVisualOuterParent->_dcompVisualOuterChild->_dcompVisualInnerParent->_dcompVisualInnerChild.
        // Once IDirectManipulationDCompCompositor::AddVisual is called, DirectManipulation will lie between
        // inner parent and child visuals and outer parent and child visuals.
        //
        if(SUCCEEDED(hr))
        {
            hr = _dcompTarget->SetRoot(_dcompVisualOuterParent.Get());
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualOuterChild);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualInnerParent);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualInnerChild);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualButtonDown);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualButtonUpCapturedByViewport);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualButtonUpClicked);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateVisual(&_dcompVisualButtonUpDefault);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualOuterParent->AddVisual(_dcompVisualOuterChild.Get(), FALSE, nullptr);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualOuterChild->AddVisual(_dcompVisualInnerParent.Get(), FALSE, nullptr);
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualInnerParent->AddVisual(_dcompVisualInnerChild.Get(), FALSE, nullptr);
        }

        if(SUCCEEDED(hr))
        {
            _dcompVisualButtonCurrent = _dcompVisualButtonUpDefault;
            hr = _dcompVisualOuterParent->AddVisual(_dcompVisualButtonUpDefault.Get(), FALSE, nullptr);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualOuterParent->AddVisual(_dcompVisualText.Get(), TRUE, _dcompVisualOuterChild.Get());
        }

        if(SUCCEEDED(hr))
        {    
            hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &_writeFactory);
        }

        if(SUCCEEDED(hr))
        {
            hr = _writeFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us", &_statusTextFormat);
        }

        if(SUCCEEDED(hr))
        {
            hr = _writeFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 25.0f, L"en-us", &_viewportTextFormat);
        }

        if(SUCCEEDED(hr))
        {       
            hr = _dcompDevice->CreateSurface(
                TEXTBLOCK_WIDTH, 
                TEXTBLOCK_HEIGHT, 
                DXGI_FORMAT_B8G8R8A8_UNORM, 
                DXGI_ALPHA_MODE_IGNORE, 
                &_textSurface);
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualText->SetContent(_textSurface.Get());
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualText->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        //
        // Load Circles.png in preparation for painting the content of the inner viewport.
        //
        ComPtr<IWICImagingFactory> factory;
        if(SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
        }

        ComPtr<IWICBitmapDecoder> decoder;
        if(SUCCEEDED(hr))
        {
            hr = factory->CreateDecoderFromFilename(L".\\Circles.png", nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
        }

        ComPtr<IWICBitmapFrameDecode> source;
        if(SUCCEEDED(hr))
        {
            hr = decoder->GetFrame(0, &source);
        }

        if(SUCCEEDED(hr))
        {
            hr = factory->CreateFormatConverter(&_bitmapConverter);
        }

        if(SUCCEEDED(hr))
        {
            hr = _bitmapConverter->Initialize(source.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
        }

        UINT imageWidth = 0;
        UINT imageHeight = 0;
        if(SUCCEEDED(hr))
        {
            hr = _bitmapConverter->GetSize(&imageWidth, &imageHeight);
        }

        if(SUCCEEDED(hr))
        {
            // Populate the inner viewport and client rect based on the image size. The default
            // size will be half the image width and height.
            _viewportInnerRect.top = INNER_VIEWPORT_OFFSET;
            _viewportInnerRect.left = INNER_VIEWPORT_OFFSET;
            _viewportInnerRect.right = INNER_VIEWPORT_OFFSET + (imageWidth / 2);
            _viewportInnerRect.bottom = INNER_VIEWPORT_OFFSET + (imageHeight / 2);

            _contentInnerRect.top = 0;
            _contentInnerRect.left = 0;
            _contentInnerRect.right = imageWidth;
            _contentInnerRect.bottom = imageHeight;
        }
        
        return hr;
    }
    
    HRESULT CAppWindow::_DrawOuterPrimaryContent()
    {
        HRESULT hr = S_OK;
        
        UINT width = _contentOuterRect.right - _contentOuterRect.left;
        UINT height = _contentOuterRect.bottom - _contentOuterRect.top;
        
        ComPtr<IDCompositionSurface> outerSurface;
        hr = _dcompDevice->CreateSurface(
            width, 
            height,
            DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_ALPHA_MODE_IGNORE, 
            &outerSurface);

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualOuterChild->SetContent(outerSurface.Get());
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualOuterChild->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        // Surface begin draw
        POINT point = {0, 0};
        ComPtr<IDXGISurface1> dxSurface;
        if(SUCCEEDED(hr))
        {
            hr = outerSurface->BeginDraw(nullptr, IID_PPV_ARGS(&dxSurface), &point);
        }            

        ComPtr<ID2D1RenderTarget> renderTarget;
        if(SUCCEEDED(hr))
        {
            D2D1_RENDER_TARGET_PROPERTIES targetProperties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96, 96);
            hr = _d2dFactory->CreateDxgiSurfaceRenderTarget(dxSurface.Get(), &targetProperties, &renderTarget);
        }
        
        // Render target begin draw
        if(SUCCEEDED(hr))
        {
            renderTarget->BeginDraw();
        }

        ComPtr<ID2D1SolidColorBrush> purpleBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple), &purpleBrush);
        }

        ComPtr<ID2D1SolidColorBrush> grayBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &grayBrush);
        }
        
        if(SUCCEEDED(hr))
        {        
            for(int x = 0; x < SQUARES_PER_SIDE; x++)
            {
                for(int y = 0; y < SQUARES_PER_SIDE; y++)
                {
                    float fractionX = width / 8.0f;
                    float fractionY = height / 8.0f;

                    D2D1_RECT_F fillRect = D2D1::RectF((x * fractionX) + point.x, (y * fractionY) + point.y, ((x + 1) * fractionX) + point.x, ((y + 1) * fractionY) + point.y);
                    ComPtr<ID2D1SolidColorBrush> brush = nullptr;
                    if((x + y) % 2 == 1)
                    {
                        brush = grayBrush;
                    }
                    else
                    {
                        brush = purpleBrush;
                    }

                    renderTarget->FillRectangle(fillRect, brush.Get());
                }
            }
        }

        ComPtr<ID2D1SolidColorBrush> whiteBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush);
        }

        WCHAR panText[] = L"Pan and Zoom Me!";
        D2D1_RECT_F textRect = D2D1::RectF(OUTER_VIEWPORT_TEXT_MARGIN_X, OUTER_VIEWPORT_TEXT_MARGIN_Y, OUTER_VIEWPORT_TEXT_WIDTH + OUTER_VIEWPORT_TEXT_MARGIN_X, OUTER_VIEWPORT_TEXT_HEIGHT + OUTER_VIEWPORT_TEXT_MARGIN_Y);
        renderTarget->DrawTextW(panText, ARRAYSIZE(panText), _viewportTextFormat.Get(), textRect, whiteBrush.Get());

        // End Draw
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->EndDraw();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = outerSurface->EndDraw();
        }
        
        // Associate the DirectComposition content we just rendered with DirectManipulation. This allows DirectManipulation
        // to control these surfaces automatically, and off the main UI thread.
        if(SUCCEEDED(hr))
        {
            hr = _compositor->AddContent(_contentOuter.Get(), _dcompDevice.Get(), _dcompVisualOuterParent.Get(), _dcompVisualOuterChild.Get());
        }
  
        if(SUCCEEDED(hr))
        {
            WCHAR buttonText[] = L"I'm a button! Press me!";
            hr = _DrawButtonState(_dcompVisualButtonUpDefault, TRUE, buttonText, wcsnlen_s(buttonText, ARRAYSIZE(buttonText)));
        }

        if(SUCCEEDED(hr))
        {
            WCHAR buttonText[] = L"I was clicked.";
            hr = _DrawButtonState(_dcompVisualButtonUpClicked, TRUE, buttonText, wcsnlen_s(buttonText, ARRAYSIZE(buttonText)));
        }

        if(SUCCEEDED(hr))
        {
            WCHAR buttonText[] = L"Input was captured.";
            hr = _DrawButtonState(_dcompVisualButtonUpCapturedByViewport, TRUE, buttonText, wcsnlen_s(buttonText, ARRAYSIZE(buttonText)));
        }

        if(SUCCEEDED(hr))
        {
            WCHAR buttonText[] = L"Release or drag now.";
            hr = _DrawButtonState(_dcompVisualButtonDown, FALSE, buttonText, wcsnlen_s(buttonText, ARRAYSIZE(buttonText)));
        }

        return hr;
    }

    HRESULT CAppWindow::_DrawInnerPrimaryContent()
    {
        HRESULT hr = S_OK;

        ComPtr<IDCompositionSurface> innerSurface;
        hr = _dcompDevice->CreateSurface(
            _contentInnerRect.right - _contentInnerRect.left, 
            _contentInnerRect.bottom - _contentInnerRect.top, 
            DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_ALPHA_MODE_IGNORE, 
            &innerSurface);

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualInnerChild->SetContent(innerSurface.Get());
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualInnerChild->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        // Surface begin draw
        POINT point;
        ComPtr<IDXGISurface1> dxSurface;
        hr = innerSurface->BeginDraw(nullptr, IID_PPV_ARGS(&dxSurface), &point);          

        ComPtr<ID2D1RenderTarget> renderTarget;
        if(SUCCEEDED(hr))
        {
            D2D1_RENDER_TARGET_PROPERTIES targetProperties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96, 96);
            hr = _d2dFactory->CreateDxgiSurfaceRenderTarget(dxSurface.Get(), &targetProperties, &renderTarget);
        }
        
        // Render target begin draw
        if(SUCCEEDED(hr))
        {
            renderTarget->BeginDraw();
        }
        
        ComPtr<ID2D1Bitmap> bitmap;
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->CreateBitmapFromWicBitmap(_bitmapConverter.Get(), nullptr, &bitmap);
        }
        
        ComPtr<ID2D1BitmapBrush> bitmapBrush;
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->CreateBitmapBrush(bitmap.Get(), &bitmapBrush);
        }

        float contentX = static_cast<float>(_contentInnerRect.right - _contentInnerRect.left);
        float contentY = static_cast<float>(_contentInnerRect.bottom - _contentInnerRect.top);
        D2D1_RECT_F fillRect = D2D1::RectF(0.0f + point.x, 0.0f + point.y, contentX + point.x, contentY + point.y);
        renderTarget->DrawBitmap(bitmap.Get(), fillRect);

        //End Draw
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->EndDraw();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = innerSurface->EndDraw();
        }

        // Associate the DirectComposition content we just rendered with DirectManipulation. This allows DirectManipulation
        // to control these surfaces automatically, and off the main UI thread.
        if(SUCCEEDED(hr))
        {
            hr = _compositor->AddContent(_contentInner.Get(), _dcompDevice.Get(), _dcompVisualInnerParent.Get(), _dcompVisualInnerChild.Get());
        }

        return hr;
    }

    HRESULT CAppWindow::_DrawViewportStatusText()
    {
        HRESULT hr = S_OK;

        WCHAR statusTextInner[MAX_PATH] = {0};
        WCHAR statusTextOuter[MAX_PATH] = {0};
        WCHAR statusText[MAX_PATH] = {0};

        hr = _handlerOuter.Get()->GetViewportStatus(statusTextOuter, ARRAYSIZE(statusTextOuter));
        
        if(SUCCEEDED(hr))
        {
            hr = _handlerInner.Get()->GetViewportStatus(statusTextInner, ARRAYSIZE(statusTextInner));
        }
        
        if(SUCCEEDED(hr))
        {
            hr = StringCchPrintf(statusText, ARRAYSIZE(statusText), L"Outer viewport status: %s\r\nInner viewport status: %s", statusTextOuter, statusTextInner);
        }

        // Surface begin draw
        POINT point = {0, 0};
        ComPtr<IDXGISurface1> dxSurface;
        if(SUCCEEDED(hr))
        {
            hr = _textSurface->BeginDraw(nullptr, IID_PPV_ARGS(&dxSurface), &point);     
        }

        ComPtr<ID2D1RenderTarget> renderTarget;        
        if(SUCCEEDED(hr))
        {
            D2D1_RENDER_TARGET_PROPERTIES targetProperties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96, 96);

            hr = _d2dFactory->CreateDxgiSurfaceRenderTarget(dxSurface.Get(), &targetProperties, &renderTarget);
        }
        
        // Render target begin draw
        if(SUCCEEDED(hr))
        {
            renderTarget->BeginDraw();
        }

        ComPtr<ID2D1SolidColorBrush> redBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &redBrush);
        }

        ComPtr<ID2D1SolidColorBrush> blueBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AliceBlue), &blueBrush);
        }

        const D2D1_RECT_F fillRect = {0.0f + point.x, 0.0f + point.y, static_cast<float>(TEXTBLOCK_WIDTH) + point.x, static_cast<float>(TEXTBLOCK_HEIGHT) + point.y};
        if(SUCCEEDED(hr))
        {
            
            renderTarget->FillRectangle(fillRect, blueBrush.Get());
        }

        if(SUCCEEDED(hr))
        {
            renderTarget->DrawTextW(statusText, wcsnlen_s(statusText, ARRAYSIZE(statusText)), _statusTextFormat.Get(), &fillRect, redBrush.Get());
        }

        //End Draw
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->EndDraw();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _textSurface->EndDraw();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->Commit();
        }

        return hr;
    }

    HRESULT CAppWindow::_DrawButtonState(ComPtr<IDCompositionVisual2> buttonVisual, BOOL isUp, WCHAR* buttonText, size_t buttonTextLength)
    {
        HRESULT hr = S_OK;
        
        ComPtr<IDCompositionSurface> buttonSurface;
        hr = _dcompDevice->CreateSurface(
            BUTTON_WIDTH, 
            BUTTON_HEIGHT,
            DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_ALPHA_MODE_IGNORE, 
            &buttonSurface);

        if(SUCCEEDED(hr))
        {
            hr = buttonVisual->SetOffsetX(BUTTON_OFFSET_X);
        }

        if(SUCCEEDED(hr))
        {
            hr = buttonVisual->SetOffsetY(BUTTON_OFFSET_Y);
        }

        if(SUCCEEDED(hr))
        {
            hr = buttonVisual->SetContent(buttonSurface.Get());
        }
        
        if(SUCCEEDED(hr))
        {
            hr = buttonVisual->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        // Surface begin draw
        POINT point = {0, 0};
        ComPtr<IDXGISurface1> dxSurface;
        if(SUCCEEDED(hr))
        {
            hr = buttonSurface->BeginDraw(nullptr, IID_PPV_ARGS(&dxSurface), &point);
        }            

        ComPtr<ID2D1RenderTarget> renderTarget;
        if(SUCCEEDED(hr))
        {
            D2D1_RENDER_TARGET_PROPERTIES targetProperties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96, 96);
            hr = _d2dFactory->CreateDxgiSurfaceRenderTarget(dxSurface.Get(), &targetProperties, &renderTarget);
        }
        
        // Render target begin draw
        if(SUCCEEDED(hr))
        {
            renderTarget->BeginDraw();
        }

        ComPtr<ID2D1SolidColorBrush> grayBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &grayBrush);
        }

        ComPtr<ID2D1SolidColorBrush> lightGrayBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &lightGrayBrush);
        }
        
        if(SUCCEEDED(hr))
        {        
            D2D1_RECT_F fillBorderRect = D2D1::RectF(static_cast<float>(point.x), static_cast<float>(point.y), static_cast<float>(BUTTON_WIDTH + point.x), static_cast<float>(BUTTON_HEIGHT + point.y));
            D2D1_RECT_F fillInnerRect = D2D1::RectF(point.x + BUTTON_BORDER_WIDTH, point.y + BUTTON_BORDER_WIDTH, BUTTON_WIDTH + point.x - BUTTON_BORDER_WIDTH, BUTTON_HEIGHT + point.y - BUTTON_BORDER_WIDTH);

            if(isUp)
            {
                renderTarget->FillRectangle(fillBorderRect, grayBrush.Get());
                renderTarget->FillRectangle(fillInnerRect, lightGrayBrush.Get());
            }
            else
            {
                renderTarget->FillRectangle(fillBorderRect, lightGrayBrush.Get());
                renderTarget->FillRectangle(fillInnerRect, grayBrush.Get());
            }
        }

        ComPtr<ID2D1SolidColorBrush> blackBrush;
        if(SUCCEEDED(hr))
        {        
            hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);
        }

        D2D1_RECT_F textRect = D2D1::RectF(point.x + BUTTON_BORDER_WIDTH + BUTTON_TEXT_PADDING, point.y + BUTTON_BORDER_WIDTH + BUTTON_TEXT_PADDING, BUTTON_WIDTH + point.x - BUTTON_BORDER_WIDTH - BUTTON_TEXT_PADDING, + BUTTON_HEIGHT + point.y - BUTTON_BORDER_WIDTH - BUTTON_TEXT_PADDING);
        renderTarget->DrawTextW(buttonText, buttonTextLength, _statusTextFormat.Get(), textRect, blackBrush.Get());

        // End Draw
        if(SUCCEEDED(hr))
        {
            hr = renderTarget->EndDraw();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = buttonSurface->EndDraw();
        }

        return hr;
    }

    HRESULT CAppWindow::_ChangeButtonState(ComPtr<IDCompositionVisual2> buttonState)
    {
        HRESULT hr = S_OK;
        hr = _dcompVisualOuterParent->RemoveVisual(_dcompVisualButtonCurrent.Get());

        if(SUCCEEDED(hr))
        {
            _dcompVisualButtonCurrent = buttonState;
            hr = _dcompVisualOuterParent->AddVisual(buttonState.Get(), FALSE, nullptr);
        }                    
                    
        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->Commit();
        }

        return hr;
    }

    BOOL CAppWindow::_OuterViewportHasContact(UINT contactId)
    {
        std::list<UINT>::iterator contact = std::find(_activeContacts.begin(), _activeContacts.end(), contactId);
        
        if(contact != _activeContacts.end())
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    HRESULT CAppWindow::_SizeDependentChanges()
    {
        HRESULT hr = S_OK;

        // Set the outer viewport rect to the size of the client area.
        ::GetClientRect(_hWnd, &_viewportOuterRect);

        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->SetViewportRect(&_viewportOuterRect);
        }
        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->SetViewportRect(&_viewportInnerRect);
        }

        //
        // Set the content size for each viewport's primary content object.
        //
        if(SUCCEEDED(hr))
        {
            hr = _contentOuter->SetContentRect(&_contentOuterRect);
        }

        if(SUCCEEDED(hr))
        {
            hr = _contentInner->SetContentRect(&_contentInnerRect);
        }

        ComPtr<IDCompositionRectangleClip> clipRect;
        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->CreateRectangleClip(&clipRect);
        }
        
        if(SUCCEEDED(hr))
        {
            clipRect->SetLeft(static_cast<float>(_viewportOuterRect.left));
            clipRect->SetTop(static_cast<float>(_viewportOuterRect.top));
            clipRect->SetRight(static_cast<float>(_viewportOuterRect.right));
            clipRect->SetBottom(static_cast<float>(_viewportOuterRect.bottom));
        
            hr = _dcompVisualOuterParent->SetClip(clipRect.Get());
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualInnerParent->SetOffsetX(static_cast<float>(_viewportInnerRect.left));
        }

        if(SUCCEEDED(hr))
        {
            hr = _dcompVisualInnerParent->SetOffsetY(static_cast<float>(_viewportInnerRect.top));
        }

        if(SUCCEEDED(hr))
        {
            ComPtr<IDCompositionRectangleClip> clip;
            _dcompDevice->CreateRectangleClip(&clip);
            
            hr = clip->SetLeft(0.0f);

            if(SUCCEEDED(hr))
            {
                hr = clip->SetTop(0.0f);
            }

            if(SUCCEEDED(hr))
            {
                hr = clip->SetRight(static_cast<float>(_viewportInnerRect.right - _viewportInnerRect.left));
            }
            
            if(SUCCEEDED(hr))
            {
                hr = clip->SetBottom(static_cast<float>(_viewportInnerRect.bottom - _viewportInnerRect.top));
            }

            hr = _dcompVisualInnerParent->SetClip(clip.Get());
        }

        //
        // Render the content
        // 
        if(SUCCEEDED(hr))
        {
            hr = _DrawOuterPrimaryContent();
        }

        if(SUCCEEDED(hr))
        {
            hr = _DrawInnerPrimaryContent();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _dcompDevice->Commit();
        }

        return hr;
    }

    HRESULT CAppWindow::_Initialize()
    {
        HRESULT hr = S_OK;
        
        //
        // Create the Direct3D, DirectComposition and Direct2D objects, then initialize our drawing code
        //
        hr = _InitializeDevices();
        
        if(SUCCEEDED(hr))
        {
            hr = _InitializeManagerAndViewport();
        }
        
        if(SUCCEEDED(hr))
        {
            hr = _InitializeDrawing();
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->SetChaining(DIRECTMANIPULATION_MOTION_ALL);
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->SetChaining(DIRECTMANIPULATION_MOTION_ALL);
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportOuter->Enable();
        }

        if(SUCCEEDED(hr))
        {
            hr = _viewportInner->Enable();
        }
        
        return hr;
    }
}