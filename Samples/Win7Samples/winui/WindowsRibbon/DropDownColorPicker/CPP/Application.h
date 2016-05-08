// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
#include "ribbonres.h"
#include "CommandHandler.h"

class CApplication
    : public IUIApplication
{
public:
    // Static method to create an instance of this object.
    __checkReturn static HRESULT CreateInstance(__deref_out_opt IUIApplication** ppApplication, HWND hwnd);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

    // IUIApplication methods.
    STDMETHOD(OnViewChanged)(UINT32 nViewID, __in UI_VIEWTYPE typeID, __in IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode);
    STDMETHOD(OnCreateUICommand)(UINT32 nCmdID, __in UI_COMMANDTYPE typeID, __deref_out IUICommandHandler** ppCommandHandler);
    STDMETHOD(OnDestroyUICommand)(UINT32 commandId, __in UI_COMMANDTYPE typeID, __in_opt IUICommandHandler* pCommandHandler);

private:    
    CApplication()
        : m_cRef(1)
        , m_pButtonHandler(NULL)
        , m_pColorPickerHandler(NULL)
    {
    };

    ~CApplication()
    {
        if (m_pButtonHandler != NULL)
        {
            m_pButtonHandler->Release();
        }
        if (m_pColorPickerHandler != NULL)
        {
            m_pColorPickerHandler->Release();
        }
    };

    HWND m_hWnd;    // Window handler.
    ULONG m_cRef;
    CButtonHandler* m_pButtonHandler;           // Button command handler.
    CColorPickerHandler* m_pColorPickerHandler; // Color picker command handler.
};