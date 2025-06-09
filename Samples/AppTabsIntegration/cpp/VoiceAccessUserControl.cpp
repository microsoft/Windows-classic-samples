#include "pch.h"
#include "VoiceAccessUserControl.h"
#include "VoiceAccessUserControl.g.cpp"

namespace winrt::AppTabsIntegration::implementation
{
    VoiceAccessUserControl::VoiceAccessUserControl()
    {
        // Initialize default values
        m_mode = L"Listen";
        m_isVoiceEnabled = true;
    }

    winrt::hstring VoiceAccessUserControl::Mode() const
    {
        return m_mode;
    }

    void VoiceAccessUserControl::Mode(winrt::hstring const& value)
    {
        if (m_mode != value)
        {
            m_mode = value;
            NotifyChange(L"Mode");
            UpdateModeDisplay();
        }
    }

    bool VoiceAccessUserControl::IsVoiceEnabled() const
    {
        return m_isVoiceEnabled;
    }

    void VoiceAccessUserControl::IsVoiceEnabled(bool value)
    {
        if (m_isVoiceEnabled != value)
        {
            m_isVoiceEnabled = value;
            NotifyChange(L"IsVoiceEnabled");
            UpdateModeDisplay();
        }
    }

    void VoiceAccessUserControl::UpdateModeDisplay()
    {
        // Update the display based on current mode and voice enabled state
        // This method would typically update UI elements based on the current state
    }
}