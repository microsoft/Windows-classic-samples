#include "stdafx.h"

#include "DeviceListener.h"
#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include <StrSafe.h>
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <roapi.h>
#include <wrl.h>
#include <map>
#include <vector>
#include <Pathcch.h>
#include <windows.foundation.h>
#include <windows.foundation.numerics.h>
#include <windows.foundation.collections.h>
#include <windows.ui.input.h>
#include <wrl\client.h>
#include <winstring.h>

#define RETURN_IF_FAILED(hr) { if(FAILED(hr)) return hr; }

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

DeviceListener::DeviceListener(HANDLE console)
{
    _console = console;
}

HRESULT DeviceListener::Init(HWND hwnd)
{
    bool isRoInit = false;
    HRESULT hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        PrintMsg(L"RO_INIT_MULTITHREADED SUCCEEDED\n", FOREGROUND_GREEN);
        isRoInit = true;
    }
    else
    {
        PrintMsg(L"RO_INIT_MULTITHREADED FAILED\n", FOREGROUND_RED);
    }

    if (isRoInit)
    { 
        hr = RegisterForEvents(hwnd);

        if (SUCCEEDED(hr))
        {
            hr = PopulateMenuItems();
        }

        if (SUCCEEDED(hr))
        {
            PrintMsg(L"Successfully initialized \n", FOREGROUND_GREEN);
        }
        else
        {
            PrintMsg(L"Failed to initialize\n", FOREGROUND_RED);
        }
    }

    return hr;
}

HRESULT DeviceListener::SetRotationResolution(double res)
{
    return _controller->put_RotationResolutionInDegrees(res);
}

HRESULT DeviceListener::RegisterForEvents(HWND hwnd)
{
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Input_RadialController).Get(),
        &_controllerInterop));

    RETURN_IF_FAILED(_controllerInterop->CreateForWindow(hwnd, IID_PPV_ARGS(&_controller)));

    // Wire events
    RETURN_IF_FAILED(_controller->add_ScreenContactContinued(
        Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactContinuedEventArgs*>>(this, &DeviceListener::OnScreenContactContinued).Get(),
        &_screenContactContinuedToken));

    RETURN_IF_FAILED(_controller->add_ScreenContactStarted(
        Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactStartedEventArgs*>>(this, &DeviceListener::OnScreenContactStarted).Get(),
        &_screenContactStartedToken));

    RETURN_IF_FAILED(_controller->add_ScreenContactEnded(
        Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnScreenContactEnded).Get(),
        &_screenContactEndedToken));

    RETURN_IF_FAILED(_controller->add_ControlLost(
        Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnControlLost).Get(),
        &_controlLostToken));

    RETURN_IF_FAILED(_controller->add_ControlAcquired(
        Callback<ITypedEventHandler<RadialController*, RadialControllerControlAcquiredEventArgs*>>(this, &DeviceListener::OnControlAcquired).Get(),
        &_controlAcquiredToken));

    RETURN_IF_FAILED(_controller->add_RotationChanged(
        Callback<ITypedEventHandler<RadialController*, RadialControllerRotationChangedEventArgs*>>(this, &DeviceListener::OnRotationChanged).Get(),
        &_rotatedToken));

    // Lambda callback
    RETURN_IF_FAILED(_controller->add_ButtonClicked(
        Callback<ITypedEventHandler<RadialController*, RadialControllerButtonClickedEventArgs*>>([this]
        (IRadialController*, IRadialControllerButtonClickedEventArgs* args)
    {
        PrintMsg(L"ButtonClicked\n", FOREGROUND_BLUE | FOREGROUND_GREEN);
        RETURN_IF_FAILED(LogContact(args));

        return S_OK;
    }).Get(),
        &_buttonClickedToken));

    return S_OK;
}

HRESULT DeviceListener::PopulateMenuItems()
{
    RETURN_IF_FAILED(_controller->get_Menu(&_menu));

    PrintMsg(L"Got Menu \n", FOREGROUND_BLUE);

    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
        &_menuItemStatics));


    RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"My Ink").Get(),
        RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_InkColor,
        _knownIconItem1Token));

    RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"Ruler").Get(),
        RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_Ruler,
        _knownIconItem2Token));

    RETURN_IF_FAILED(AddMenuItemFromSystemFont());

    return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ RadialControllerMenuKnownIcon icon, _In_ EventRegistrationToken registrationToken)
{
    // Get menu items
    ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
    RETURN_IF_FAILED(_menu->get_Items(&menuItems));

    // Create item from known icon
    ComPtr<IRadialControllerMenuItem> menuItem;
    RETURN_IF_FAILED(_menuItemStatics->CreateFromKnownIcon(itemName, icon, &menuItem));

    RETURN_IF_FAILED(AddMenuItem(menuItem, itemName, registrationToken));

    return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromSystemFont()
{
    // Create item from system font
    ComPtr<IRadialControllerMenuItemStatics2> menuItemStatics2;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
        &menuItemStatics2));

    HSTRING toolDisplayName = HStringReference(L"System Font Item").Get();
    ComPtr<IRadialControllerMenuItem> systemFontItem;
    menuItemStatics2->CreateFromFontGlyph(toolDisplayName, HStringReference(L"\x2764").Get(), HStringReference(L"Segoe UI Emoji").Get(), &systemFontItem);

    RETURN_IF_FAILED(AddMenuItem(systemFontItem, toolDisplayName, _systemFontItemToken));

    // Get font uri
    ComPtr<IUriRuntimeClass> fontUri;
    RETURN_IF_FAILED(GetFontUri(&fontUri));

    // Create item from custom font
    toolDisplayName = HStringReference(L"Custom Font Item").Get();
    ComPtr<IRadialControllerMenuItem> customFontItem;
    menuItemStatics2->CreateFromFontGlyphWithUri(toolDisplayName, HStringReference(L"\ue102").Get(), HStringReference(L"Symbols").Get(), fontUri.Get(), &customFontItem);

    RETURN_IF_FAILED(AddMenuItem(customFontItem, toolDisplayName, _customFontItemToken));
}

HRESULT DeviceListener::AddMenuItem(_In_ ComPtr<IRadialControllerMenuItem> item, _In_ HSTRING itemName, _In_ EventRegistrationToken registrationToken)
{
    // Set Callback
    RETURN_IF_FAILED(item->add_Invoked(
        Callback<ITypedEventHandler<RadialControllerMenuItem*, IInspectable*>>(this, &DeviceListener::OnItemInvoked).Get(),
        &registrationToken));

    // Get menu items
    ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
    RETURN_IF_FAILED(_menu->get_Items(&menuItems));

    // Add item to menu
    RETURN_IF_FAILED(menuItems->Append(item.Get()));

    // Log new item
    wchar_t message[2000];
    swprintf_s(message, 2000, L"Added %s to menu\n", WindowsGetStringRawBuffer(itemName, nullptr));
    PrintMsg(message, FOREGROUND_BLUE | FOREGROUND_GREEN);

    return S_OK;
}

HRESULT DeviceListener::GetFontUri(_Out_ ComPtr<IUriRuntimeClass>* fontUri)
{
    WCHAR currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);
    WCHAR fontFile[] = L"..\\shared\\Symbols.ttf";

    WCHAR fontPath[MAX_PATH];
    RETURN_IF_FAILED(PathCchCombine(fontPath, MAX_PATH, currentPath, fontFile));

    ComPtr<IUriRuntimeClassFactory> uriRuntimeClassFactory;
    RETURN_IF_FAILED(GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_Uri).Get(), &uriRuntimeClassFactory));

    RETURN_IF_FAILED(uriRuntimeClassFactory->CreateUri(HStringReference(fontPath).Get(), &(*fontUri)));

    return S_OK;
}

HRESULT DeviceListener::OnItemInvoked(_In_ IRadialControllerMenuItem* item, _In_ IInspectable* args)
{
    if (item != nullptr)
    {
        wchar_t message[2000];
        HSTRING str;

        item->get_DisplayText(&str);
        swprintf_s(message, 2000, L"ItemInvoked %s\n", WindowsGetStringRawBuffer(str, nullptr));
        PrintMsg(message, FOREGROUND_RED | FOREGROUND_GREEN);
    }

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactContinued(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactContinuedEventArgs* args)
{
    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactStarted(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactStartedEventArgs* args)
{
    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactEnded(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{
    PrintMsg(L"ScreenContactEnded\n", FOREGROUND_BLUE | FOREGROUND_GREEN);

    return S_OK;
}

HRESULT DeviceListener::OnControlLost(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{
    PrintMsg(L"ControlLost\n", FOREGROUND_RED);

    return S_OK;
}

HRESULT DeviceListener::OnControlAcquired(_In_ IRadialController* /*sender*/, _In_ IRadialControllerControlAcquiredEventArgs* args)
{
    PrintMsg(L"ControlAcquired\n", FOREGROUND_GREEN);
    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

HRESULT DeviceListener::OnRotationChanged(_In_ IRadialController* /*sender*/, _In_ IRadialControllerRotationChangedEventArgs* args)
{
    wchar_t message[2000];
    double delta = 0;

    args->get_RotationDeltaInDegrees(&delta);
    swprintf_s(message, 2000, L"RotationChanged delta=%lf\n", delta);
    PrintMsg(message, FOREGROUND_GREEN | FOREGROUND_RED);

    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

template<typename TContactArgs>
HRESULT DeviceListener::LogContact(_In_ TContactArgs* args)
{
    // Get contact
    ComPtr<IRadialControllerScreenContact> contact;
    RETURN_IF_FAILED(args->get_Contact(&contact));

    if (contact != nullptr)
    {
        // Get contact info
        Point position;
        Rect bounds;
        RETURN_IF_FAILED(contact->get_Position(&position));
        RETURN_IF_FAILED(contact->get_Bounds(&bounds));

        // Log contact info
        wchar_t message[2000];
        swprintf_s(message, 2000, L"\t Postion X=%lf, Y=%lf & \n \t Bounds Height=%lf, Width=%lf, X=%lf, Y=%lf\n",
            position.X, position.Y,
            bounds.Height, bounds.Width, bounds.X, bounds.Y);

        PrintMsg(message, FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    return S_OK;
}

void DeviceListener::PrintMsg(WCHAR *message, WORD messageColor)
{
    UINT bufferSize = 2000;
    size_t textLength;
    DWORD charsWritten;
    StringCchLength(message, bufferSize, &textLength);

    // Set font to message color
    SetConsoleTextAttribute(_console, messageColor);

    // Send message
    WriteConsole(_console, message, (DWORD)textLength, &charsWritten, NULL);

    // Return font to original color
    SetConsoleTextAttribute(_console, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}