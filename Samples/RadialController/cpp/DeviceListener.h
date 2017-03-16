#pragma once

#include <wrl\client.h>
#include <windows.ui.input.h>
#include <RadialControllerInterop.h>

class DeviceListener
{
public:
    DeviceListener(HANDLE console);
    ~DeviceListener() {}

    HRESULT Init(HWND hwnd);
    HRESULT SetRotationResolution(double res);

private:
    HRESULT RegisterForEvents(HWND hwnd);
    HRESULT PopulateMenuItems();
    HRESULT AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ ABI::Windows::UI::Input::RadialControllerMenuKnownIcon icon, _In_ EventRegistrationToken registrationToken);
    HRESULT AddMenuItemFromSystemFont();
    HRESULT AddMenuItem(_In_ Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenuItem> item, _In_ HSTRING itemName, _In_ EventRegistrationToken registrationToken);
    HRESULT GetFontUri(_Out_ Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IUriRuntimeClass>* fontUri);
    HRESULT OnItemInvoked(_In_ ABI::Windows::UI::Input::IRadialControllerMenuItem* item, _In_ IInspectable* args);

    HRESULT OnScreenContactContinued(_In_ ABI::Windows::UI::Input::IRadialController*, _In_ ABI::Windows::UI::Input::IRadialControllerScreenContactContinuedEventArgs* args);
    HRESULT OnScreenContactStarted(_In_ ABI::Windows::UI::Input::IRadialController*, _In_ ABI::Windows::UI::Input::IRadialControllerScreenContactStartedEventArgs* args);
    HRESULT OnScreenContactEnded(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ IInspectable* args);
    HRESULT OnControlLost(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ IInspectable* args);
    HRESULT OnControlAcquired(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ ABI::Windows::UI::Input::IRadialControllerControlAcquiredEventArgs* args);
    HRESULT OnRotationChanged(_In_ ABI::Windows::UI::Input::IRadialController* sender, _In_ ABI::Windows::UI::Input::IRadialControllerRotationChangedEventArgs* args);

    template<typename TContactArgs>
    HRESULT LogContact(_In_ TContactArgs* args);
    void PrintMsg(WCHAR *message, WORD messageColor);

    DWORD _threadId = 0;
    HANDLE _console = nullptr;

    Microsoft::WRL::ComPtr<IRadialControllerInterop> _controllerInterop;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialController> _controller;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenu> _menu;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenuItemStatics> _menuItemStatics;

    EventRegistrationToken _rotatedToken;
    EventRegistrationToken _screenContactStartedToken;
    EventRegistrationToken _screenContactEndedToken;
    EventRegistrationToken _screenContactContinuedToken;
    EventRegistrationToken _controlLostToken;
    EventRegistrationToken _controlAcquiredToken;
    EventRegistrationToken _buttonClickedToken;
    EventRegistrationToken _knownIconItem1Token;
    EventRegistrationToken _knownIconItem2Token;
    EventRegistrationToken _systemFontItemToken;
    EventRegistrationToken _customFontItemToken;
};

