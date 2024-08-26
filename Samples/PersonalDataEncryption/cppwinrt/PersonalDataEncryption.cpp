#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Security.DataProtection.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <iostream>

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Security::DataProtection;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
}

void usage()
{
    std::wcout << LR"(PersonalDataEncryption sample usage.

PersonalDataEncryption.exe file <level> <path>

    Protect a file to a protection level.
    Path must be a fully-qualified path.

PersonalDataEncryption.exe folder <level> <path>

    Protect a folder to a protection level.
    Path must be a fully-qualified path.

PersonalDataEncryption.exe memory <level> <string>

    Protect a memory buffer to a protection level.

For both options, the following protection levels are supported:

        Level 0: No protection (always available).
        Level 1: Available after first unlock.
        Level 2: Avaialble when unlocked.

Examples:

Protect a file to level 1:
    PersonalDataEncryption.exe file 1 C:\Users\Abby\Documents\file.txt

Remove protection from a folder:
    PersonalDataEncryption.exe folder 1 C:\Users\Abby\Documents\Party

Protect a string to level 2:
    PersonalDataEncryption.exe memory 2 "hello, world!"
)";
}

void ProtectItem(winrt::UserDataProtectionManager const& manager, winrt::UserDataAvailability availability, winrt::IStorageItem const& item)
{
    auto status = manager.ProtectStorageItemAsync(item, availability).get();
    wchar_t const* message;
    switch (status)
    {
    case winrt::UserDataStorageItemProtectionStatus::Succeeded:
        message = L"Succeeded";
        break;

    case winrt::UserDataStorageItemProtectionStatus::NotProtectable:
        message = L"Not protectable";
        break;

    case winrt::UserDataStorageItemProtectionStatus::DataUnavailable:
        message = L"Data unavailable";
        break;

    default:
        message = L"Unknown failure";
        break;
    }

    std::wcout <<
        L"Protecting " << std::wstring_view(item.Path()) <<
        L" to level " << static_cast<int>(availability) <<
        L": " << message << L"\n";
}

void ProtectFile(winrt::UserDataProtectionManager const& manager, winrt::hstring const& path, winrt::UserDataAvailability availability)
{
    winrt::IStorageItem item;
    try
    {
        item = winrt::StorageFile::GetFileFromPathAsync(path).get();
    }
    catch (...)
    {
        std::wcout << L"Error getting StorageFile: " << std::wstring_view(winrt::to_message()) << L"\n";
    }
    ProtectItem(manager, availability, item);
}

// Note that setting the availability of a folder establishes the availability for any new items
// created in that folder, but it does not affect the availability of existing items in the folder.
// If you want to change the availability of an entire folder tree, you should first call
// ProtectStorageItemAsync to protect the folder (so that any new items are suitably protected),
// and then recursively protect all the files and subfolders inside that folder.
void ProtectFolder(winrt::UserDataProtectionManager const& manager, winrt::hstring const& path, winrt::UserDataAvailability availability)
{
    winrt::IStorageItem item;
    try
    {
        item = winrt::StorageFolder::GetFolderFromPathAsync(path).get();
    }
    catch (...)
    {
        std::wcout << L"Error getting StorageFile: " << std::wstring_view(winrt::to_message()) << L"\n";
    }
    ProtectItem(manager, availability, item);
}

void ProtectMemory(winrt::UserDataProtectionManager const& manager, winrt::hstring const& message, winrt::UserDataAvailability availability)
{
    // Convert the string to a Buffer.
    auto bufferSize = message.size() * static_cast<uint32_t>(sizeof(wchar_t));
    auto buffer = winrt::Buffer(bufferSize);
    memcpy_s(buffer.data(), bufferSize, message.c_str(), bufferSize);
    buffer.Length(bufferSize);

    // Protect the buffer.
    auto protectedBuffer = manager.ProtectBufferAsync(buffer, availability).get();

    // Zero out the original buffer to ensure it holds no data.
    memset(buffer.data(), 0, bufferSize);

    // Free the original buffer and retain the protected buffer.
    buffer = nullptr;

    std::wcout << L"Memory buffer has been protected.\n"
        << L"Press Enter to unprotect it.\n";

    std::cin.get();

    // Now unprotect the buffer to recover the string.
    auto result = manager.UnprotectBufferAsync(protectedBuffer).get();

    winrt::UserDataBufferUnprotectStatus status = result.Status();
    winrt::IBuffer unprotectedBuffer = result.UnprotectedBuffer();

    switch (status)
    {
    case winrt::UserDataBufferUnprotectStatus::Succeeded:
        // Extract the string from the unprotected buffer.
        std::wcout << L"Original message: "
            << std::wstring_view(reinterpret_cast<wchar_t*>(unprotectedBuffer.data()), unprotectedBuffer.Length() / sizeof(wchar_t))
            << L"\n";
        break;

    case winrt::UserDataBufferUnprotectStatus::Unavailable:
        std::wcout << L"Buffer cannot be unprotected at this time.\n";
        break;

    default:
        std::wcout << L"Unexpected error when unprotecting buffer.\n";
        break;
    }
}

void Run(int argc, wchar_t** argv)
{
    if (argc != 4)
    {
        usage();
        return;
    }

    wchar_t* end;
    auto availability = static_cast<winrt::UserDataAvailability>(wcstoul(argv[2], &end, 10));
    if (*end != L'\0')
    {
        // Not an integer.
        usage();
        return;
    }

    if (availability != winrt::UserDataAvailability::Always &&
        availability != winrt::UserDataAvailability::AfterFirstUnlock &&
        availability != winrt::UserDataAvailability::WhileUnlocked)
    {
        // Not a valid integer.
        usage();
        return;
    }

    winrt::UserDataProtectionManager manager = winrt::UserDataProtectionManager::TryGetDefault();
    if (!manager)
    {
        std::wcout << L"Personal Data Encryption is not enabled.\n";
        return;
    }

    if (wcscmp(argv[1], L"file") == 0)
    {
        ProtectFile(manager, argv[3], availability);
    }
    else if (wcscmp(argv[1], L"folder") == 0)
    {
        ProtectFolder(manager, argv[3], availability);
    }
    else if (wcscmp(argv[1], L"memory") == 0)
    {
        ProtectMemory(manager, argv[3], availability);
    }
    else
    {
        usage();
    }
}

int wmain(int argc, wchar_t** argv)
{
    winrt::init_apartment();
    Run(argc, argv);
    winrt::uninit_apartment();
    return 0;
}
