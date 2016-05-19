////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#pragma once

// Public Headers
#include <shobjidl.h>

//------------------------------------------------------------------------------
// CDeviceContextMenu
//
//      Prototype for class that implements IInitializeCommand and
//      IExplorerCommand to support both Context Menu and Command Bar
//      extensibility in the shell (for this sample we're after extending the
//      Devices and Printers folder exclusively).
//
//      Please note that IExplorerCommandState and IContextMenu also exists for
//      implementing a context menu handlers. Please see the documentation on 
//      MSDN for more info. 
//------------------------------------------------------------------------------
class CDeviceContextMenu:
    public IInitializeCommand,
    public IExplorerCommand
{
public:

    CDeviceContextMenu();

    //
    // IExplorerCommand
    //
    IFACEMETHODIMP GetTitle(
        __in IShellItemArray* psiItemArray, 
        __out LPWSTR* ppszName
        );
    IFACEMETHODIMP GetIcon(
        __in IShellItemArray* psiItemArray, 
        __out LPWSTR* ppszIcon
        );
    IFACEMETHODIMP GetToolTip(
        __in IShellItemArray* psiItemArray, 
        __out LPWSTR *ppszInfotip
        );
    IFACEMETHODIMP GetCanonicalName(
        __out GUID* pguidCommandName
        );
    IFACEMETHODIMP GetState(
        __in_opt IShellItemArray* psiItemArray, 
        BOOL fOkToBeSlow, 
        __out EXPCMDSTATE* pCmdState
        );
    IFACEMETHODIMP Invoke(
        __in IShellItemArray* psiItemArray, 
        __in IBindCtx* pbc
        );
    IFACEMETHODIMP GetFlags(
        __out EXPCMDFLAGS* pFlags
        );
    IFACEMETHODIMP EnumSubCommands(
        __deref_out IEnumExplorerCommand** ppEnum
        );

    //
    // IInitializeCommand
    //
    IFACEMETHODIMP Initialize(
        __in PCWSTR pszCommandName, 
        __in IPropertyBag* ppb
        );

    //
    // IUnknown
    //
    IFACEMETHODIMP QueryInterface(
        __in REFIID riid, 
        __deref_out void** ppvObject
        );
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

private:

    ~CDeviceContextMenu();

    HRESULT GetDeviceHardwareID(
        __in IShellItem2* pShellItem,
        __out LPWSTR* ppszHardwareID
        );

    LONG  m_cRef;
    PWSTR m_szCommandName;
       
};// CDeviceContextMenu

