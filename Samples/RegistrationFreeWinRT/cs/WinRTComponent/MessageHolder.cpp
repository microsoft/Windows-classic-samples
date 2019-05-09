#include "pch.h"
#include "MessageHolder.h"
#include "MessageHolder.g.cpp"

namespace winrt::WinRTComponent::implementation
{
    hstring MessageHolder::Message()
    {
        return m_message;
    }

    void MessageHolder::Message(hstring value)
    {
        m_message = std::move(value);
    }
}
