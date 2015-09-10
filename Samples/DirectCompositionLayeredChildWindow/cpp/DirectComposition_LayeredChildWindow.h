// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <d3d11_1.h>
#include <atlbase.h>
#include <windows.h>
#include <dcomp.h>
#include <dwmapi.h>

#include <windowsx.h>
#include <strsafe.h>
#include "resource.h"

#include <mfplay.h>
#include <mferror.h>

class CApplication
{
    public: 
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static VOID ShowErrorMessage(PCWSTR format, HRESULT hr);
        static VOID OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent);
        static VOID OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT *pEvent);

    public:
        explicit CApplication(HINSTANCE hInstance);
        ~CApplication();

        int Run();

    private:		
        CApplication(const CApplication &); 

	private:
        HRESULT PlayMediaFile();

        HRESULT Initialize();

        // Initialize main window functions including layered child window
        HRESULT InitializeMainWindow();
        HRESULT InitializeLayeredChildWindows();
        HRESULT MoveLayeredChildWindows();

        // Initialize Direct Composition and D3D functions
        HRESULT CreateD3D11Device();
        HRESULT CreateDCompositionDevice();
        HRESULT CreateDCompositionRenderTarget();
        HRESULT CreateDCompositionVisualTree();
        HRESULT CreateTransforms();

        // Window message handlers
        LRESULT OnClose(HWND hwnd);
        HRESULT OnCommand(int id);
        LRESULT OnDestroy(HWND hwnd); 
        HRESULT OnPaint(HWND hwnd);
        HRESULT OnRightClick();
        HRESULT OnTimer();

        // Direct Composition Transforms
        HRESULT OnRotate();
        HRESULT OnScale();
        HRESULT OnSkew();
        
        HRESULT SetRotation(float beginRotationAngle, float beginTime, float endTime, IDCompositionRotateTransform *rotateTransform);
        HRESULT SetScaling(float beginScaleX, float beginScaleY, float endScaleX, float endScaleY, float beginTime, float endTime, IDCompositionScaleTransform *scaleTransform);
        HRESULT SetSkewing(float skewAngleX, float skewAngleY, float beginTime, float endTime, IDCompositionSkewTransform *skewTransform);
        HRESULT SetTranslation(float beginPositionX, float endPositionX, float beginPositionY, float endPositionY, float beginTime, float endTime, IDCompositionTranslateTransform *translateTransform);

        HRESULT CreateLinearAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **linearAnimation);
        HRESULT CreateTranslateAnimation(float beginValue, float endValue, float beginTime, float endTime, IDCompositionAnimation **linearAnimation);

        // Cloak/uncloak windows
        HRESULT CloakWindow(BOOL cloakHwnd, HWND hwnd);

        int EnterMessageLoop();

        // Destroy
        VOID Destroy();
        VOID DestroyMainWindow();
        VOID DestroyLayeredChildWindows();
        VOID DestroyDCompositionVisualTree();
        VOID DestroyDCompositionRenderTarget();
        VOID DestroyDCompositionDevice();
        VOID DestroyD3D11Device();
        VOID DestroyTransforms();

    private:
        static CApplication *s_application;
        static CComPtr<IMFPMediaPlayer> s_pPlayer;

        static const float s_fanimationTime;

        HINSTANCE m_hInstance;

        WCHAR m_fontTypeface[32];
        int   m_fontHeightLogo;
        int   m_fontHeightTitle;
        int   m_fontHeightDescription;

        HWND  m_hMainWindow;         // Main window
        HWND  m_hControlChildWindow; // Control window
        HWND  m_hVideoChildWindow;   // Video window
        HWND  m_hTextChildWindow;    // Text window
        HWND  m_hwndButton[4];       // Control buttons

        CComPtr<ID3D11Device> _d3d11Device;
        CComPtr<ID3D11DeviceContext> _d3d11DeviceContext;

        CComPtr<IDCompositionDevice> m_pDevice;
        CComPtr<IDCompositionTarget> m_pHwndRenderTarget;
        CComPtr<IDCompositionVisual> m_pRootVisual;
        CComPtr<IDCompositionVisual> m_pControlChildVisual;
        CComPtr<IDCompositionVisual> m_pTextChildVisual;
        CComPtr<IDCompositionVisual> m_pVideoChildVisual;
        CComPtr<IUnknown> m_pControlsurfaceTile;
        CComPtr<IUnknown> m_pTextsurfaceTile;
        CComPtr<IUnknown> m_pVideosurfaceTile;

        CComPtr<IDCompositionTranslateTransform> m_pControlTranslateTransform;
        CComPtr<IDCompositionTranslateTransform> m_pTextTranslateTransform;
        CComPtr<IDCompositionTransform> m_pTransformGroup;

        CComPtr<IDCompositionRotateTransform> m_pRotateTransform;
        CComPtr<IDCompositionSkewTransform> m_pSkewTransform;
        CComPtr<IDCompositionScaleTransform> m_pScaleTransform;

        //MediaPlayerCallBack
        BOOL m_bControlOn;
        CComPtr<IMFPMediaPlayerCallback> s_pPlayerCB;

        enum    
        {
            ID_PLAYSTOP,        // Play/Stop button ID
            ID_ROTATE,          // Rotate button ID 
            ID_SCALE,           // Scale button ID 
            ID_SKEW,            // Skew button ID
            IDT_TIMER           // Timer ID
        };                 
};

// Implements the callback interface for MFPlay events.
class MediaPlayerCallback : public IMFPMediaPlayerCallback 
{
    private:
    
        long m_cRef; // Reference count

    public:
    
        MediaPlayerCallback() : m_cRef(1)
        {
        }

        STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
        {
            HRESULT hr = S_OK;
            *ppv = NULL; 

            if (riid == __uuidof(IMFPMediaPlayerCallback))
            {
                *ppv = static_cast<IMFPMediaPlayerCallback*>(this);
            }

            else if (riid == __uuidof(IUnknown))
            {
                *ppv = static_cast<IUnknown*>(this);
            }

            else
            {
                hr = E_NOINTERFACE;
            }

            if (SUCCEEDED(hr))
            {
                AddRef();
            }

            return hr;
        }

        STDMETHODIMP_(ULONG) AddRef() 
        {
            return InterlockedIncrement(&m_cRef); 
        }
    
        STDMETHODIMP_(ULONG) Release()
        {
            ULONG count = InterlockedDecrement(&m_cRef);
        
            if (count == 0)
            {
                delete this;
            }

            return count;
        }

        // IMFPMediaPlayerCallback methods
        void STDMETHODCALLTYPE OnMediaPlayerEvent(_In_ MFP_EVENT_HEADER *pEventHeader);
};