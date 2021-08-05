// ApplicationLoopback.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include "LoopbackCapture.h"

void usage()
{
    std::wcout <<
        L"Usage: ApplicationLoopback <pid> <includetree|excludetree> <outputfilename>\n"
        L"\n"
        L"<pid> is the process ID to capture or exclude from capture\n"
        L"includetree includes audio from that process and its child processes\n"
        L"excludetree includes audio from all processes except that process and its child processes\n"
        L"<outputfilename> is the WAV file to receive the captured audio (10 seconds)\n"
        L"\n"
        L"Examples:\n"
        L"\n"
        L"ApplicationLoopback 1234 includetree CapturedAudio.wav\n"
        L"\n"
        L"  Captures audio from process 1234 and its children.\n"
        L"\n"
        L"ApplicationLoopback 1234 excludetree CapturedAudio.wav\n"
        L"\n"
        L"  Captures audio from all processes except process 1234 and its children.\n";
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 4)
    {
        usage();
        return 0;
    }

    DWORD processId = wcstoul(argv[1], nullptr, 0);
    if (processId == 0)
    {
        usage();
        return 0;
    }

    bool includeProcessTree;
    if (wcscmp(argv[2], L"includetree") == 0)
    {
        includeProcessTree = true;
    }
    else if (wcscmp(argv[2], L"excludetree") == 0)
    {
        includeProcessTree = false;
    }
    else
    {
        usage();
        return 0;
    }

    PCWSTR outputFile = argv[3];

    CLoopbackCapture loopbackCapture;
    HRESULT hr = loopbackCapture.StartCaptureAsync(processId, includeProcessTree, outputFile);
    if (FAILED(hr))
    {
        wil::unique_hlocal_string message;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PWSTR)&message, 0, nullptr);
        std::wcout << L"Failed to start capture\n0x" << std::hex << hr << L": " << message.get() << L"\n";
    }
    else
    {
        std::wcout << L"Capturing 10 seconds of audio." << std::endl;
        Sleep(10000);

        loopbackCapture.StopCaptureAsync();

        std::wcout << L"Finished.\n";
    }

    return 0;
}
