// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

template<typename T>
class ClassFactory : public winrt::implements<ClassFactory<T>, IClassFactory>
{
public:
    // IClassFactory
    IFACEMETHODIMP CreateInstance(_In_opt_ IUnknown* unkOuter, REFIID riid, _COM_Outptr_ void** object)
    {
        try
        {
            auto provider = winrt::make<T>();
            winrt::com_ptr<IUnknown> unkn{ provider.as<IUnknown>() };
            winrt::check_hresult(unkn->QueryInterface(riid, object));
            return S_OK;
        }
        catch (...)
        {
            // winrt::to_hresult() will eat the exception if it is a result of winrt::check_hresult,
            // otherwise the exception will get rethrown and this method will crash out as it should
            return winrt::to_hresult();
        }
    }
    IFACEMETHODIMP LockServer(BOOL lock) { return S_OK; }
};
