#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>

#include <wrl\client.h>

#include <wil\common.h>
#include <wil\com.h>
#include <wil\resource.h>
#include <wil\result.h>

#include "..\ProxyStub\ISparkleFinisher_h.h"

int __cdecl wmain(int argc, wchar_t** argv)
try
{
    // Get a file name.
    std::wcout << L"Enter a file name: " << std::endl;

    std::wstring fileName;
    std::getline(std::wcin, fileName);

    // Open the file
    auto fileHandle = wil::unique_hfile(CreateFileW(fileName.c_str(), // File name
        GENERIC_READ | GENERIC_WRITE, // Read/write access
        FILE_SHARE_READ | FILE_SHARE_WRITE, // Share with other processes
        nullptr, // Default security
        OPEN_ALWAYS, // Create if not exists, open otherwise.
        0, // No flags/attributes
        nullptr // No template
    ));

    THROW_LAST_ERROR_IF(!fileHandle);

    // Make our side of the event
    // Auto reset, initially unset, no name, default security, throws on fail
    wil::unique_event startEvent(wil::EventOptions::None);

    // Prepare COM
    auto uninitOnExit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    // Gain access to a sparkle finisher. COM will start the server if it's not running.
    auto sparkleFinisher = wil::CoCreateInstance<ISparkleFinisher>(CLSID_SparkleFinisher, CLSCTX_LOCAL_SERVER);

    // Tell the sparkle finisher to work on our file when we signal the start event.
    // It returns an event that is signaled when the work is finished.
    wil::unique_event finishedEvent;
    THROW_IF_FAILED(sparkleFinisher->AddSparkleFinishToFile(fileHandle.get(), startEvent.get(), &finishedEvent));

    // Build up our message.
    std::stringstream message;
    message << "Loading dishes." << std::endl;
    message << "Loading detergent." << std::endl;
    message << "Pre-washing." << std::endl;
    message << "Rinsing." << std::endl;
    message << "Washing." << std::endl;
    message << "Rinsing." << std::endl;

    // Finalize into string.
    auto completeMessage = message.str();

    // Write out to file.
    auto bytesToWrite = completeMessage.size() * sizeof(completeMessage[0]);
    DWORD bytesWritten = 0;
    THROW_IF_WIN32_BOOL_FALSE(WriteFile(fileHandle.get(),
        completeMessage.c_str(),
        static_cast<DWORD>(bytesToWrite),
        &bytesWritten,
        nullptr));

    // If we didn't write the whole thing... detect it and set error.
    THROW_HR_IF(E_UNEXPECTED, bytesWritten != bytesToWrite);

    // Signal the sparkle finisher server to do its work.
    startEvent.SetEvent();

    // Wait for the server to say that it is finished.
    finishedEvent.wait(); // Wait infinite.

    // Cleanup of all handles and resources is automatic when using WIL types!

    return 0;
}
catch (const wil::ResultException& ex)
{
    std::cout << "Sparkle Finish Client Failed with error: '" << ex.what() << "'" << std::endl;
}
catch (const std::exception& ex)
{
    std::cout << "Sparkle Finish Client Failed with error: '" << ex.what() << "'" << std::endl;
}
