#include "pch.h"
#include "winrt/WinRTComponent.h"

int wmain(int /*argc*/, wchar_t** /*argv*/)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    winrt::WinRTComponent::MessageHolder messageHolder;
    std::wcout << messageHolder.Message().c_str() << std::endl;
}
