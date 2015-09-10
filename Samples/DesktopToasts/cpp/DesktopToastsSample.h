#pragma once

#define LINE_LENGTH 15 // How many characters we'll allot for each line of a toast

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef HM_TEXTBUTTON
#define HM_TEXTBUTTON 1
#endif

const wchar_t AppId[] = L"Microsoft.Samples.DesktopToasts";

class DesktopToastsApp
{
public:
    DesktopToastsApp();
    ~DesktopToastsApp();
    HRESULT Initialize();
    void RunMessageLoop();

private:
    HRESULT TryCreateShortcut();
    HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);
    static LRESULT CALLBACK WndProc(
		_In_ HWND hWnd, 
		_In_ UINT message, 
		_In_ WPARAM wParam, 
		_In_ LPARAM lParam
		);

    HRESULT TextButtonClicked();
    HRESULT DisplayToast();
    HRESULT CreateToastXml(
		_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager, 
		_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument **xml
		);

    HRESULT CreateToast(
		_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager, 
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml
		);
    HRESULT DesktopToastsApp::SetImageSrc(
		_In_z_ wchar_t *imagePath, 
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
		);
    HRESULT DesktopToastsApp::SetTextValues(
		_In_reads_(textValuesCount) wchar_t **textValues, 
		_In_ UINT32 textValuesCount, 
		_In_reads_(textValuesCount) UINT32 *textValuesLengths, 
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
		);
    HRESULT DesktopToastsApp::SetNodeValueString(
		_In_ HSTRING onputString,
		_In_ ABI::Windows::Data::Xml::Dom::IXmlNode *node, 
		_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml
		);

    HWND _hwnd;
    HWND _hEdit;
};