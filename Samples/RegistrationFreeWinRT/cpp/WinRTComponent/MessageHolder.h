#pragma once

#include "MessageHolder.g.h"

namespace winrt::WinRTComponent::implementation
{
    struct MessageHolder : MessageHolderT<MessageHolder>
    {
        MessageHolder() = default;

        hstring Message();
        void Message(hstring value);

    private:
        hstring m_message{ L"Hello from a Non-Packaged WinRT Component! :D" };
    };
}

namespace winrt::WinRTComponent::factory_implementation
{
    struct MessageHolder : MessageHolderT<MessageHolder, implementation::MessageHolder>
    {
    };
}
