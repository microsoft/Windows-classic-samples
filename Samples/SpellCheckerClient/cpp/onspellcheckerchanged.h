// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// onspellcheckerchanged.h : Implementation of the listener class for changes that may affect spell checking results

#pragma once

#include "util.h"
#include "spellcheck.h"

#include <stdio.h>
#include <new.h>

class OnSpellCheckerChanged : public ISpellCheckerChangedEventHandler
{
public:
    static HRESULT StartListeningToChangeEvents(_In_ ISpellChecker* spellChecker, _COM_Outptr_ OnSpellCheckerChanged** eventListener)
    {
        OnSpellCheckerChanged* onChanged = new(std::nothrow) OnSpellCheckerChanged();
        HRESULT hr = (nullptr == onChanged) ? E_OUTOFMEMORY : S_OK;

        if (SUCCEEDED(hr))
        {
            hr = spellChecker->add_SpellCheckerChanged(onChanged, &onChanged->_eventCookie);
        }

        if (FAILED(hr))
        {
            delete onChanged;
            onChanged = nullptr;
        }

        *eventListener = onChanged;
        PrintErrorIfFailed(L"StartListeningToChangeEvents", hr);
        return hr;
    }

    static void StopListeningToChangeEvents(_In_ ISpellChecker* spellChecker, _In_ OnSpellCheckerChanged* eventHandler)
    {
        HRESULT hr = spellChecker->remove_SpellCheckerChanged(eventHandler->_eventCookie);
        eventHandler->Release();
        PrintErrorIfFailed(L"StopListeningToChangeEvents", hr);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        InterlockedIncrement(&_count);
        return _count;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG count = InterlockedDecrement(&_count);
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, _COM_Outptr_ void** ppv)
    {
        *ppv = nullptr;
        if (riid == IID_IUnknown)
        {
            *ppv = this;
        }
        else if (riid == __uuidof(ISpellCheckerChangedEventHandler*))
        {
            *ppv = this;
        }
        else
        {
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    IFACEMETHODIMP Invoke(_In_ ISpellChecker* /*sender*/)
    {
        wprintf(L"Spell checker changed.\n");
        return S_OK;
    }

private:

    OnSpellCheckerChanged()
        : _count(1)
    {
    }

    virtual ~OnSpellCheckerChanged()
    {
    }

    ULONG _count;
    DWORD _eventCookie;
};
