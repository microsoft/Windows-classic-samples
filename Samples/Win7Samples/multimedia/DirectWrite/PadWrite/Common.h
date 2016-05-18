// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------
#pragma once

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER                  // Minimum platform is Windows 7
#define WINVER 0x0601
#endif

#ifndef _WIN32_WINNT            // Minimum platform is Windows 7
#define _WIN32_WINNT 0x0601
#endif

#ifndef _WIN32_WINDOWS          // Minimum platform is Windows 7
#define _WIN32_WINDOWS 0x0601
#endif

#ifndef WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX                // Use the standard's templated min/max
#define NOMINMAX
#endif

#ifndef _USE_MATH_DEFINES       // We do want PI defined
#define _USE_MATH_DEFINES
#endif

////////////////////////////////////////

// C headers:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <utility>
#include <math.h>

// C++ headers:
#include <algorithm>

#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <string>
#include <vector>

// Windows headers:

#include <windows.h>
#include <windowsx.h>
#include <winnls.h>
#include <unknwn.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <strsafe.h>
#include <commdlg.h>

////////////////////////////////////////
// Common macro definitions:

#if !defined(NDEBUG) && !defined(DEBUG)
    #define DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// Use the double macro technique to stringize the actual value of s
#define STRINGIZE_(s) STRINGIZE2_(s)
#define STRINGIZE2_(s) #s

#define FAILURE_LOCATION "\r\nFunction: " __FUNCTION__ "\r\nLine: " STRINGIZE_(__LINE__)

#if (_MSC_VER >= 1200) // want to use std::min and std::max
#undef min
#undef max
#define min(x,y) _cpp_min(x,y)
#define max(x,y) _cpp_max(x,y)
#endif

// Ignore unreferenced parameters, since they are very common
// when implementing callbacks.
#pragma warning(disable : 4100)


////////////////////////////////////////
// Application specific headers/functions:

#define APPLICATION_TITLE "PadWrite - DirectWrite layout SDK sample"

// Needed text editor backspace deletion.
inline bool IsSurrogate(UINT32 ch) throw()
{
    // 0xD800 <= ch <= 0xDFFF
    return (ch & 0xF800) == 0xD800;
}


inline bool IsHighSurrogate(UINT32 ch) throw()
{
    // 0xD800 <= ch <= 0xDBFF
    return (ch & 0xFC00) == 0xD800;
}


inline bool IsLowSurrogate(UINT32 ch) throw()
{
    // 0xDC00 <= ch <= 0xDFFF
    return (ch & 0xFC00) == 0xDC00;
}


// Maps exceptions to equivalent HRESULTs,
inline HRESULT ExceptionToHResult() throw()
{
    try
    {
        throw;  // Rethrow previous exception.
    }
    catch(std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }
}


////////////////////////////////////////
// COM help.


// Releases a COM object and nullifies pointer.
template <typename InterfaceType>
inline void SafeRelease(InterfaceType** currentObject)
{
    if (*currentObject != NULL)
    {
        (*currentObject)->Release();
        *currentObject = NULL;
    }
}


// Acquires an additional reference, if non-null.
template <typename InterfaceType>
inline InterfaceType* SafeAcquire(InterfaceType* newObject)
{
    if (newObject != NULL)
        newObject->AddRef();

    return newObject;
}


// Sets a new COM object, releasing the old one.
template <typename InterfaceType>
inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeAcquire(newObject);
    SafeRelease(currentObject);
    *currentObject = newObject;
}


// Releases a COM object and nullifies pointer.
template <typename InterfaceType>
inline InterfaceType* SafeDetach(InterfaceType** currentObject)
{
    InterfaceType* oldObject = *currentObject;
    *currentObject = NULL;
    return oldObject;
}


// Sets a new COM object, acquiring the reference.
template <typename InterfaceType>
inline void SafeAttach(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeRelease(currentObject);
    *currentObject = newObject;
}


// Generic COM base implementation for classes, since DirectWrite uses
// callbacks for several different kinds of objects, particularly the
// text renderer and inline objects.
//
// Example:
//
//  class RenderTarget : public ComBase<QiList<IDWriteTextRenderer> >
//
template <typename InterfaceChain>
class ComBase : public InterfaceChain
{
public:
    explicit ComBase() throw()
    :   refValue_(0)
    { }

    // IUnknown interface
    IFACEMETHOD(QueryInterface)(IID const& iid, OUT void** ppObject)
    {
        *ppObject = NULL;
        InterfaceChain::QueryInterfaceInternal(iid, ppObject);
        if (*ppObject == NULL)
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }

    IFACEMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&refValue_);
    }

    IFACEMETHOD_(ULONG, Release)()
    {
        ULONG newCount = InterlockedDecrement(&refValue_);
        if (newCount == 0)
            delete this;

        return newCount;
    }

    virtual ~ComBase()
    { }

protected:
    ULONG refValue_;

private:
    // No copy construction allowed.
    ComBase(const ComBase& b);
    ComBase& operator=(ComBase const&);
};


struct QiListNil
{
};


// When the QueryInterface list refers to itself as class,
// which hasn't fully been defined yet.
template <typename InterfaceName, typename InterfaceChain>
class QiListSelf : public InterfaceChain
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

        *ppObject = static_cast<InterfaceName*>(this);
    }
};


// When this interface is implemented and more follow.
template <typename InterfaceName, typename InterfaceChain = QiListNil>
class QiList : public InterfaceName, public InterfaceChain
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

        *ppObject = static_cast<InterfaceName*>(this);
    }
};


// When the this is the last implemented interface in the list.
template <typename InterfaceName>
class QiList<InterfaceName, QiListNil> : public InterfaceName
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return;

        *ppObject = static_cast<InterfaceName*>(this);
    }
};
