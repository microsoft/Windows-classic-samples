#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace Windows::Foundation;

// Main function
int WINAPI wWinMain(_In_ HINSTANCE /* hInstance */, _In_opt_ HINSTANCE /* hPrevInstance */, _In_ LPWSTR /* lpCmdLine */, _In_ int /* nCmdShow */)
{
    HRESULT hr = Initialize(RO_INIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        {
            DesktopToastsApp app;
            hr = app.Initialize();

            if (SUCCEEDED(hr))
            {
                app.RunMessageLoop();
            }
        }
        Uninitialize();
    }

    return SUCCEEDED(hr);
}

DesktopToastsApp::DesktopToastsApp() : _hwnd(nullptr), _hEdit(nullptr)
{
}

DesktopToastsApp::~DesktopToastsApp()
{
}

// In order to display toasts, a desktop application must have a shortcut on the Start menu.
// Also, an AppUserModelID must be set on that shortcut.
// The shortcut should be created as part of the installer. The following code shows how to create
// a shortcut and assign an AppUserModelID using Windows APIs. You must download and include the 
// Windows API Code Pack for Microsoft .NET Framework for this code to function
//
// Included in this project is a wxs file that be used with the WiX toolkit
// to make an installer that creates the necessary shortcut. One or the other should be used.

HRESULT DesktopToastsApp::TryCreateShortcut()
{
    wchar_t shortcutPath[MAX_PATH];
    DWORD charWritten = GetEnvironmentVariable(L"APPDATA", shortcutPath, MAX_PATH);
    HRESULT hr = charWritten > 0 ? S_OK : E_INVALIDARG;

    if (SUCCEEDED(hr))
    {
        errno_t concatError = wcscat_s(shortcutPath, ARRAYSIZE(shortcutPath), L"\\Microsoft\\Windows\\Start Menu\\Programs\\Desktop Toasts App.lnk");
        hr = concatError == 0 ? S_OK : E_INVALIDARG;
        if (SUCCEEDED(hr))
        {
            DWORD attributes = GetFileAttributes(shortcutPath);
            bool fileExists = attributes < 0xFFFFFFF;

            if (!fileExists)
            {
                hr = InstallShortcut(shortcutPath);
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }
    return hr;
}

// Install the shortcut
HRESULT DesktopToastsApp::InstallShortcut(_In_z_ wchar_t *shortcutPath)
{
    wchar_t exePath[MAX_PATH];
    
    DWORD charWritten = GetModuleFileNameEx(GetCurrentProcess(), nullptr, exePath, ARRAYSIZE(exePath));

    HRESULT hr = charWritten > 0 ? S_OK : E_FAIL;
    
    if (SUCCEEDED(hr))
    {
        ComPtr<IShellLink> shellLink;
        hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));

        if (SUCCEEDED(hr))
        {
            hr = shellLink->SetPath(exePath);
            if (SUCCEEDED(hr))
            {
                hr = shellLink->SetArguments(L"");
                if (SUCCEEDED(hr))
                {
                    ComPtr<IPropertyStore> propertyStore;

                    hr = shellLink.As(&propertyStore);
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT appIdPropVar;
                        hr = InitPropVariantFromString(AppId, &appIdPropVar);
                        if (SUCCEEDED(hr))
                        {
                            hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
                            if (SUCCEEDED(hr))
                            {
                                hr = propertyStore->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    ComPtr<IPersistFile> persistFile;
                                    hr = shellLink.As(&persistFile);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = persistFile->Save(shortcutPath, TRUE);
                                    }
                                }
                            }
                            PropVariantClear(&appIdPropVar);
                        }
                    }
                }
            }
        }
    }
    return hr;
}

// Prepare the main window
HRESULT DesktopToastsApp::Initialize()
{
    HRESULT hr = TryCreateShortcut();
    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex;

        ATOM atom;

        // Register window class
        wcex.cbSize        = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DesktopToastsApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName  = nullptr;
        wcex.lpszClassName = L"DesktopToastsApp";
        wcex.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
        atom = RegisterClassEx(&wcex);
        
        hr = atom ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
                // Create window
            _hwnd = CreateWindow(
                L"DesktopToastsApp",
                L"Desktop Toasts Demo App",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                350,
                200,
                nullptr,
                nullptr,
                HINST_THISCOMPONENT,
                this
                );

            hr = _hwnd ? S_OK : E_FAIL;

            if (SUCCEEDED(hr))
            {              
                CreateWindow(
                            L"BUTTON", 
                            L"View Text Toast", 
                            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 
                            0, 
                            0, 
                            150, 
                            25, 
                            _hwnd, 
                            reinterpret_cast<HMENU>(HM_TEXTBUTTON), 
                            HINST_THISCOMPONENT, 
                            nullptr
                            );
                _hEdit = CreateWindow(
                            L"EDIT", 
                            L"Whatever action you take on the displayed toast will be shown here.", 
                            ES_LEFT | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_BORDER, 
                            0, 
                            30, 
                            300, 
                            50, 
                            _hwnd, nullptr, 
                            HINST_THISCOMPONENT, 
                            nullptr
                            );

                ShowWindow(_hwnd, SW_SHOWNORMAL);
                UpdateWindow(_hwnd);
            }
        } 
    } 
    else
    {
        MessageBox(nullptr, L"Failed to install shortcut. Try running this application as administrator.", L"Failed launch", MB_OK | MB_ICONERROR);
    }

    return hr;
}

// Standard message loop
void DesktopToastsApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Display the toast using classic COM. Note that is also possible to create and display the toast using the new C++ /ZW options (using handles,
// COM wrappers, etc.) 
HRESULT DesktopToastsApp::DisplayToast()
{
    ComPtr<IToastNotificationManagerStatics> toastStatics;
    HRESULT hr = GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &toastStatics);

    if (SUCCEEDED(hr))
    {
        ComPtr<IXmlDocument> toastXml;
        hr = CreateToastXml(toastStatics.Get(), &toastXml);
        if (SUCCEEDED(hr))
        {
            hr = CreateToast(toastStatics.Get(), toastXml.Get());
        }
    }
    return hr;
}

// Create the toast XML from a template
HRESULT DesktopToastsApp::CreateToastXml(_In_ IToastNotificationManagerStatics *toastManager, _Outptr_ IXmlDocument** inputXml)
{    
    // Retrieve the template XML
    HRESULT hr = toastManager->GetTemplateContent(ToastTemplateType_ToastImageAndText04, inputXml);
    if (SUCCEEDED(hr))
    {    
        wchar_t *imagePath = _wfullpath(nullptr, L"toastImageAndText.png", MAX_PATH);

        hr = imagePath != nullptr ? S_OK : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND); 
        if (SUCCEEDED(hr))
        {
            hr = SetImageSrc(imagePath, *inputXml);
            if (SUCCEEDED(hr))
            {
                wchar_t* textValues[] = {
                    L"Line 1",
                    L"Line 2",
                    L"Line 3"
                };

                UINT32 textLengths[] = {6, 6, 6};

                hr = SetTextValues(textValues, 3, textLengths, *inputXml);
            }
        }
    }
    return hr;
}

// Set the value of the "src" attribute of the "image" node
HRESULT DesktopToastsApp::SetImageSrc(_In_z_ wchar_t *imagePath, _In_ IXmlDocument *toastXml)
{
    wchar_t imageSrc[MAX_PATH] = L"file:///";
    HRESULT hr = StringCchCat(imageSrc, ARRAYSIZE(imageSrc), imagePath);
    if (SUCCEEDED(hr))
    {
        ComPtr<IXmlNodeList> nodeList;
        hr = toastXml->GetElementsByTagName(StringReferenceWrapper(L"image").Get(), &nodeList);
        if (SUCCEEDED(hr))
        {
            ComPtr<IXmlNode> imageNode;
            hr = nodeList->Item(0, &imageNode);
            if (SUCCEEDED(hr))
            {
                ComPtr<IXmlNamedNodeMap> attributes;

                hr = imageNode->get_Attributes(&attributes);
                if (SUCCEEDED(hr))
                {
                    ComPtr<IXmlNode> srcAttribute;

                    hr = attributes->GetNamedItem(StringReferenceWrapper(L"src").Get(), &srcAttribute);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetNodeValueString(StringReferenceWrapper(imageSrc).Get(), srcAttribute.Get(), toastXml);
                    }
                }
            }
        }
    }
    return hr;
}

// Set the values of each of the text nodes
HRESULT DesktopToastsApp::SetTextValues(_In_reads_(textValuesCount) wchar_t **textValues, _In_ UINT32 textValuesCount, _In_reads_(textValuesCount) UINT32 *textValuesLengths, _In_ IXmlDocument *toastXml)
{
    HRESULT hr = textValues != nullptr && textValuesCount > 0 ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        ComPtr<IXmlNodeList> nodeList;
        hr = toastXml->GetElementsByTagName(StringReferenceWrapper(L"text").Get(), &nodeList);
        if (SUCCEEDED(hr))
        {
            UINT32 nodeListLength;
            hr = nodeList->get_Length(&nodeListLength);
            if (SUCCEEDED(hr))
            {
                hr = textValuesCount <= nodeListLength ? S_OK : E_INVALIDARG;
                if (SUCCEEDED(hr))
                {
                    for (UINT32 i = 0; i < textValuesCount; i++)
                    {
                        ComPtr<IXmlNode> textNode;
                        hr = nodeList->Item(i, &textNode);
                        if (SUCCEEDED(hr))
                        {
                            hr = SetNodeValueString(StringReferenceWrapper(textValues[i], textValuesLengths[i]).Get(), textNode.Get(), toastXml);
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT DesktopToastsApp::SetNodeValueString(_In_ HSTRING inputString, _In_ IXmlNode *node, _In_ IXmlDocument *xml)
{
    ComPtr<IXmlText> inputText;

    HRESULT hr = xml->CreateTextNode(inputString, &inputText);
    if (SUCCEEDED(hr))
    {
        ComPtr<IXmlNode> inputTextNode;

        hr = inputText.As(&inputTextNode);
        if (SUCCEEDED(hr))
        {
            ComPtr<IXmlNode> pAppendedChild;
            hr = node->AppendChild(inputTextNode.Get(), &pAppendedChild);
        }
    }

    return hr;
}

// Create and display the toast
HRESULT DesktopToastsApp::CreateToast(_In_ IToastNotificationManagerStatics *toastManager, _In_ IXmlDocument *xml)
{
    ComPtr<IToastNotifier> notifier;
    HRESULT hr = toastManager->CreateToastNotifierWithId(StringReferenceWrapper(AppId).Get(), &notifier);
    if (SUCCEEDED(hr))
    {
        ComPtr<IToastNotificationFactory> factory;
        hr = GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &factory);
        if (SUCCEEDED(hr))
        {
            ComPtr<IToastNotification> toast;
            hr = factory->CreateToastNotification(xml, &toast);
            if (SUCCEEDED(hr))
            {
                // Register the event handlers
                EventRegistrationToken activatedToken, dismissedToken, failedToken;
                ComPtr<ToastEventHandler> eventHandler(new ToastEventHandler(_hwnd, _hEdit));
             
                hr = toast->add_Activated(eventHandler.Get(), &activatedToken);
                if (SUCCEEDED(hr))
                {
                    hr = toast->add_Dismissed(eventHandler.Get(), &dismissedToken);
                    if (SUCCEEDED(hr))
                    {
                        hr = toast->add_Failed(eventHandler.Get(), &failedToken);
                        if (SUCCEEDED(hr))
                        {
                            hr = notifier->Show(toast.Get());
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT DesktopToastsApp::TextButtonClicked()
{
    return DisplayToast();   
}

// Standard window procedure
LRESULT CALLBACK DesktopToastsApp::WndProc(_In_ HWND hwnd, _In_ UINT32 message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        DesktopToastsApp *app = reinterpret_cast<DesktopToastsApp *>(pcs->lpCreateParams);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) app);

        return 1;
    }
    
    DesktopToastsApp *pApp = reinterpret_cast<DesktopToastsApp *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (pApp)
    {
        switch (message)
        {
        case WM_COMMAND:
            {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case HM_TEXTBUTTON:
                pApp->TextButtonClicked();
                break;
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);
            }
            return 0;
        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
