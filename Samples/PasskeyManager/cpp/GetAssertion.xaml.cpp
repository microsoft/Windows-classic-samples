#include "pch.h"
#include "App.xaml.h"
#include "GetAssertion.xaml.h"
#if __has_include("GetAssertion.g.cpp")
#include "GetAssertion.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Navigation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PasskeyManager::implementation
{
    PasskeyManager::CredentialListViewModel GetAssertion::CredentialList()
    {
        return m_credentialListViewModel;
    }

    Windows::Foundation::IAsyncAction GetAssertion::OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs)
    {
        createButton().IsEnabled(false);
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        std::optional<PluginOpertaionOptions> args = curApp->m_pluginOperationOptions;
        if (args == std::nullopt || (*args).matchingCredentials.empty())
        {
            textContent().Inlines().Clear();
            Microsoft::UI::Xaml::Documents::Run statusTextBlock;
            statusTextBlock.Text(L"Plugin failed to retrieve any credentials");
            textContent().Inlines().Append(statusTextBlock);
        }
        else
        {
            std::wstring rpNameStr((*args).rpName);
            rpNameBlock().Text(rpNameStr);

            m_credentialListViewModel.credentials().Clear();
            auto& credVec = (*args).matchingCredentials;
            for (auto& cred : credVec)
            {
                auto buffer = winrt::Windows::Storage::Streams::DataWriter();
                buffer.WriteBytes(winrt::array_view<UINT8>(cred->pbCredentialID, cred->pbCredentialID + cred->cbCredentialID));
                auto credentialIdBuffer = buffer.DetachBuffer();

                std::vector<UINT8> credId(cred->pbCredentialID, cred->pbCredentialID + cred->cbCredentialID);
                auto credential = winrt::make<PasskeyManager::implementation::Credential>(cred->pUserInformation->pwszName, cred->pRpInformation->pwszName, credentialIdBuffer);
                m_credentialListViewModel.credentials().Append(credential);
            }
        }
        co_return;
    }

    void GetAssertion::Create_Assertion(IInspectable const&, RoutedEventArgs const&)
    {
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        curApp->PluginCompleteAction();
    }

    void GetAssertion::Cancel_Plugin_Action(IInspectable const&, RoutedEventArgs const&)
    {
        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        curApp->PluginCancelAction();
    }

    void GetAssertion::UpdatePasskeyOperationStatus(HRESULT hr)
    {
        if (this->m_credentialListViewModel.credentials().Size() == 0)
        {
            this->textContent().Inlines().Clear();
            Microsoft::UI::Xaml::Documents::Run statusTextBlock;
            statusTextBlock.Text(L"Plugin failed to retrieve any credentials");
            this->textContent().Inlines().Append(statusTextBlock);
        }
        else
        {
            auto cred = this->m_credentialListViewModel.credentials().GetAt(0).as<PasskeyManager::implementation::Credential>();
            auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(cred->CredentialId());
            std::vector<UINT8> credId(reader.UnconsumedBufferLength());
            reader.ReadBytes(credId);
            CredentialOptionFlags options = CredentialOptionFlags::None;
            if (FAILED(hr))
            {
                options |= CredentialOptionFlags::OperationFailed;
            }
            else
            {
                options |= CredentialOptionFlags::OperationSucceeded;
            }

            auto newCredWithResult = winrt::make<PasskeyManager::implementation::Credential>(
                cred->UserName(), cred->RpName(), cred->CredentialId(), options);
            m_credentialListViewModel.credentials().Clear();
            m_credentialListViewModel.credentials().Append(newCredWithResult);
        }
        createButton().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
        cancelButton().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
    }

    void GetAssertion::SelectionChanged(IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&)
    {
        Microsoft::UI::Xaml::Controls::ListView listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
        auto selectedItem = listView.SelectedItem();
        if (selectedItem)
        {
            auto selectedCred = selectedItem.as<PasskeyManager::implementation::Credential>();
            com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
            if (curApp->SetSelectedCredentialId(selectedCred->CredentialId()))
            {
                createButton().IsEnabled(true);
                //clear all but selected item from list to indicate the selection is final.
                if (listView.Items().Size() > 1)
                {
                    m_credentialListViewModel.credentials().Clear();
                    m_credentialListViewModel.credentials().Append(*selectedCred.detach());
                    listView.SelectedIndex(0);
                }
            }
        }
    }

    void GetAssertion::ListView_Loaded(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        Microsoft::UI::Xaml::Controls::ListView listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
        if (listView.Items().Size() == 1)
        {
            listView.SelectedIndex(0);
            auto selectedItem = listView.Items().GetAt(0);
            auto selectedCred = selectedItem.as<PasskeyManager::implementation::Credential>();
            com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
            if (curApp->SetSelectedCredentialId(selectedCred->CredentialId()))
            {
                createButton().IsEnabled(true);
            }
        }
        else
        {
            createButton().IsEnabled(false);
        }
    }
}
