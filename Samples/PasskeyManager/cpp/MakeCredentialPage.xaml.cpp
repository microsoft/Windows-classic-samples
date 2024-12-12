#include "pch.h"
#include "App.xaml.h"
#include "MakeCredentialPage.xaml.h"
#if __has_include("MakeCredentialPage.g.cpp")
#include "MakeCredentialPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Navigation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PasskeyManager::implementation
{
    void MakeCredentialPage::Create_Credential(IInspectable const&, RoutedEventArgs const&)
    {
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        curApp->PluginCompleteAction();
    }

    void MakeCredentialPage::Cancel_Plugin_Action(IInspectable const&, RoutedEventArgs const&)
    {
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        curApp->PluginCancelAction();
    }

    Windows::Foundation::IAsyncAction MakeCredentialPage::OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs e)
    {
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        std::optional<PluginOpertaionOptions> args = curApp->m_pluginOperationOptions;
        if (args == std::nullopt)
        {
            co_return;
        }
        else
        {
            std::wstring rpNameStr((*args).rpName);
            std::wstring userNameStr((*args).userName);
            rpNameBlock().Text(rpNameStr);
            userNameBlock().Text(userNameStr);
        }
        co_return;
    }
    void MakeCredentialPage::UpdatePasskeyOperationStatus(HRESULT hr)
    {
        if (SUCCEEDED(hr))
        {
            successBlock().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
        }
        else
        {
            failureText().Text(winrt::to_hstring(static_cast<int32_t>(hr)));
            failureBlock().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
        }
        createButton().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
        cancelButton().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
    }
}
