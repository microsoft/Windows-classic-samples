//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


#pragma once

#include "stdafx.h"

// THROW_SYSTEM_ERROR is used to differentiate Win32 versus WinRT implementation for the error handling
// defined here. Define THROW_SYSTEM_ERROR in stdafx.h if this is being used in a Win32 project, but not
// if it's being used in a WinRT project.


namespace DX
{
#ifdef THROW_SYSTEM_ERROR // Win32
    inline std::system_error CreateDXException(HRESULT hr)
    {
        return std::system_error(std::error_code(hr, std::system_category()));
    }
#else // Throw WinRT exception.
    inline Platform::Exception^ CreateDXException(HRESULT hr)
    {
        return Platform::Exception::CreateException(hr);
    }
#endif // THROW_SYSTEM_ERROR


    // Convert DirectX error codes to exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch Win32 API errors.
            throw CreateDXException(hr);
        }
    }

} // DX namespace

