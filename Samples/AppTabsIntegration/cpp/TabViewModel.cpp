#include "pch.h"
#include "TabViewModel.h"
#include "TabViewModel.g.cpp"

namespace winrt
{
    using namespace winrt::AppTabsIntegration;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::UI::Shell;
}

namespace
{
    winrt::Windows::UI::Color RandomColor()
    {
        static constexpr winrt::Windows::UI::Color colors[] = {
            { 0x00, 0x00, 0x00, 0xFF },
            { 0xFF, 0x00, 0x00, 0xFF },
            { 0x00, 0xFF, 0x00, 0xFF },
            { 0xFF, 0xFF, 0x00, 0xFF },
            { 0x00, 0x00, 0xFF, 0xFF },
            { 0xFF, 0x00, 0xFF, 0xFF },
            { 0x00, 0xFF, 0xFF, 0xFF },
            { 0xFF, 0xFF, 0xFF, 0xFF },
        };
        return colors[static_cast<uint32_t>(GetTickCount64() / 16) % ARRAYSIZE(colors)];
    }

    // A TabDataModel object holds a strong reference to its paired WindowTab object as a member.
    // A WindowTab objects holds a weak reference to the paired TabDataModel object in its Tag.
    // The weak reference avoids a reference cycle.

    // Weak references are not IInspectable, so we wrap one inside an object that *is* IInspectable
    // so we can use it as a Tag.
    template<typename T>
    struct InspectableWeakReference : winrt::implements<InspectableWeakReference<T>, winrt::IInspectable>
    {
        InspectableWeakReference(winrt::weak_ref<T> const& weak) : m_weak(weak) {}
        T get() const { return m_weak.get(); }

        winrt::weak_ref<T> m_weak;
    };
}

namespace winrt::AppTabsIntegration::implementation
{
    TabViewModel::TabViewModel(const winrt::hstring& name, winrt::Windows::UI::Shell::WindowTab const& windowTab)
        : m_windowTab(windowTab), m_color(RandomColor())
    {
        Name(name);

        // A few lines above, we initialized m_windowTab to hold a reference to the windowTab.
        // Here, we create the weak reference from the windowTab back to the TabViewModel.
        if (m_windowTab)
        {
            windowTab.Tag(winrt::make<InspectableWeakReference<winrt::TabViewModel>>(*this));
        }
    }

    /* static */ winrt::TabViewModel TabViewModel::GetFromWindowTab(winrt::Windows::UI::Shell::WindowTab const& windowTab)
    {
        // From the WindowTab, get the Tag, then resolve it from a weak reference back to a strong reference.
        return winrt::get_self<InspectableWeakReference<winrt::TabViewModel>>(windowTab.Tag())->get();
    }

    /* static */ winrt::WindowTab TabViewModel::GetWindowTab(winrt::TabViewModel const& tabViewModel)
    {
        // From the TabViewModel, get the WindowTab.
        return winrt::get_self<TabViewModel>(tabViewModel)->m_windowTab;
    }

    winrt::hstring TabViewModel::Name() const
    {
        return m_name;
    }

    void TabViewModel::Name(const winrt::hstring& value)
    {
        m_name = value;

        if (m_windowTab)
        {
            // Propagate the app's tab title into the WindowTab title.
            m_windowTab.Title(value);
        }

        NotifyChange(L"Name");
    }

    winrt::Windows::UI::Color TabViewModel::Color() const
    {
        return m_color;
    }

    void TabViewModel::Color(winrt::Windows::UI::Color const& value)
    {
        if (value != m_color)
        {
            m_color = value;

            if (m_windowTab)
            {
                // Inform the Window Tab Manager that the thumbnail has changed.
                m_windowTab.ReportThumbnailAvailable();
            }

            NotifyChange(L"Color");
        }
    }
}
