// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SampleSpellingClient.cpp : Implementation of a sample spell checking client

#include "onspellcheckerchanged.h"
#include "util.h"
#include "spellprint.h"
#include "commands.h"

#include <spellcheck.h>
#include <stdio.h>
#include <objidl.h>
#include <fcntl.h>
#include <io.h>
#include <strsafe.h>

HRESULT RunCommandLoop(_In_ ISpellChecker* spellChecker)
{
    wprintf(L"Commands:\n");
    wprintf(L"quit - Quit\n");
    wprintf(L"add <word> - Add word\n");
    wprintf(L"ac <word> <word> - Add autocorrect pair\n");
    wprintf(L"ign <word> - Ignore word\n");
    wprintf(L"chkb <text> - Check text (batch - pasted text or file open)\n");
    wprintf(L"chk <text> - Check text (as you type)\n");

    HRESULT hr = S_OK;
    while (SUCCEEDED(hr))
    {
        wprintf(L"> ");
        wchar_t line[MAX_PATH];
        hr = StringCchGets(line, ARRAYSIZE(line));
        
        if (SUCCEEDED(hr))
        {
            wchar_t command[MAX_PATH];
            int count = swscanf_s(line, L"%s", command, static_cast<unsigned int>(ARRAYSIZE(command)));
            hr = (count == 1) ? hr : E_FAIL;
            if (SUCCEEDED(hr))
            {
                _Analysis_assume_nullterminated_(command);
                const size_t lineSize = wcslen(line);
                const size_t cmdSize = wcslen(command);
                if (cmdSize == lineSize)
                {
                    if (wcscmp(L"quit", command) == 0)
                    {
                        break;
                    }
                    else
                    {
                        wprintf(L"Invalid command\n");
                    }
                }
                else
                {
                    PCWSTR buffer = line + wcslen(command);

                    if (wcscmp(L"add", command) == 0)
                    {
                        hr = AddCommand(spellChecker, buffer);
                    }
                    else if (wcscmp(L"ac", command) == 0)
                    {
                        hr = AutoCorrectCommand(spellChecker, buffer);
                    }
                    else if (wcscmp(L"ign", command) == 0)
                    {
                        hr = IgnoreCommand(spellChecker, buffer);
                    }
                    else if (wcscmp(L"chkb", command) == 0)
                    {
                        hr = CheckCommand(spellChecker, buffer);
                    }
                    else if (wcscmp(L"chk", command) == 0)
                    {
                        hr = CheckAsYouTypeCommand(spellChecker, buffer);
                    }
                    else
                    {
                        wprintf(L"Invalid command\n");
                    }
                }
            }
        }
    }

    PrintErrorIfFailed(L"RunCommandLoop", hr);
    return hr;
}

HRESULT RunSpellCheckingLoop(_In_ ISpellChecker* spellChecker)
{
    OnSpellCheckerChanged* eventListener = nullptr;
    HRESULT hr = PrintInfoAndOptions(spellChecker);
    if (SUCCEEDED(hr))
    {
        hr = OnSpellCheckerChanged::StartListeningToChangeEvents(spellChecker, &eventListener);
    }

    if (SUCCEEDED(hr))
    {
        hr = RunCommandLoop(spellChecker);
    }

    if (nullptr != eventListener)
    {
        OnSpellCheckerChanged::StopListeningToChangeEvents(spellChecker, eventListener);
    }

    PrintErrorIfFailed(L"RunSpellCheckingLoop", hr);
    return hr;
}

HRESULT StartSpellCheckingSession(_In_ ISpellCheckerFactory* spellCheckerFactory, _In_ PCWSTR languageTag)
{
    BOOL isSupported = FALSE;
    HRESULT hr = spellCheckerFactory->IsSupported(languageTag, &isSupported);
    if (SUCCEEDED(hr))
    {
        if (FALSE == isSupported)
        {
            wprintf(L"Language tag %s is not supported.\n", languageTag);
        }
        else
        {
            ISpellChecker* spellChecker = nullptr;
            hr = spellCheckerFactory->CreateSpellChecker(languageTag, &spellChecker);

            if (SUCCEEDED(hr))
            {
                hr = RunSpellCheckingLoop(spellChecker);
            }

            if (nullptr != spellChecker)
            {
                spellChecker->Release();
            }
        }
    }

    PrintErrorIfFailed(L"StartSpellCheckingSession", hr);
    return hr;
}

HRESULT CreateSpellCheckerFactory(_COM_Outptr_ ISpellCheckerFactory** spellCheckerFactory)
{
    HRESULT hr = CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(spellCheckerFactory));
    if (FAILED(hr))
    {
        *spellCheckerFactory = nullptr;
    }
    PrintErrorIfFailed(L"CreateSpellCheckerFactory", hr);
    return hr;
}

int __cdecl wmain(int argc, _In_reads_(argc) PCWSTR argv[])
{
    int originalOutputMode = _setmode(_fileno(stdout), _O_U16TEXT);
    HRESULT hr = ((-1 == originalOutputMode) ? E_FAIL : S_OK);
    if (SUCCEEDED(hr))
    {
        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    bool WasCOMInitialized = SUCCEEDED(hr);

    ISpellCheckerFactory* spellCheckerFactory = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = CreateSpellCheckerFactory(&spellCheckerFactory);
    }
    
    if (SUCCEEDED(hr))
    {
        if (argc == 1)
        {
            hr = PrintAvailableLanguages(spellCheckerFactory);
        }
        else if (argc == 2)
        {
            PCWSTR const languageTag = *(argv + 1);
            hr = StartSpellCheckingSession(spellCheckerFactory, languageTag);
        }
        else
        {
            wprintf(L"Usage:\n");
            wprintf(L"\"SampleClient\" - lists all the available languages\n");
            wprintf(L"\"SampleClient <language tag>\" - initiates an interactive spell checking session in the language, if supported\n");
        }
    }

    if (nullptr != spellCheckerFactory)
    {
        spellCheckerFactory->Release();
    }

    if (WasCOMInitialized)
    {
        CoUninitialize();
    }

    PrintErrorIfFailed(L"wmain", hr);
    return (FAILED(hr) ? 1 : 0);
}

