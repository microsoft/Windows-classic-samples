#include "fvcommands.h"
#include "utils.h"
#include <new>  // std::nothrow

extern HINSTANCE g_hInst;

// Sub Commands for Settings
const FVCOMMANDITEM CFolderViewCommandProvider::c_FVTaskSettings[] =
{
    // Icon reference should be replaced by absolute reference to own icon resource.
    {&GUID_Setting1, IDS_SETTING1,   IDS_SETTING1_TT,  L"shell32.dll,-16710",  0, CFolderViewCommandProvider::s_OnSetting1, NULL, 0},
    {&GUID_Setting2, IDS_SETTING2,   IDS_SETTING2_TT,  L"shell32.dll,-16710",  0, CFolderViewCommandProvider::s_OnSetting2, NULL, 0},
    {&GUID_Setting3, IDS_SETTING3,   IDS_SETTING3_TT,  L"shell32.dll,-16710",  0, CFolderViewCommandProvider::s_OnSetting3, NULL, 0}
};

// Top-level commands
const FVCOMMANDITEM CFolderViewCommandProvider::c_FVTasks[] =
{
    // Icon reference should be replaced by absolute reference to own icon resource.
    {&GUID_Display,  IDS_DISPLAY,    IDS_DISPLAY_TT,   L"shell32.dll,-42",     0,                  CFolderViewCommandProvider::s_OnDisplay, NULL,             0 },
    {&GUID_Settings, IDS_SETTINGS,   IDS_SETTINGS_TT,  L"shell32.dll,-16710",  ECF_HASSUBCOMMANDS, NULL,                                    c_FVTaskSettings, ARRAYSIZE(c_FVTaskSettings)}
};

IFACEMETHODIMP CFolderViewCommandProvider::GetCommands(IUnknown * /* punkSite */, REFIID riid, void **ppv)
{
    *ppv = NULL;
    CFolderViewCommandEnumerator *pFVCommandEnum = new (std::nothrow) CFolderViewCommandEnumerator(c_FVTasks, ARRAYSIZE(c_FVTasks));
    HRESULT hr = pFVCommandEnum ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pFVCommandEnum->QueryInterface(riid, ppv);
        pFVCommandEnum->Release();
    }
    return hr;
}

HRESULT CFolderViewCommandProvider::s_OnDisplay(IShellItemArray *psiItemArray, IUnknown * /* pv */)
{
    return DisplayItem(psiItemArray, NULL);
}

HRESULT CFolderViewCommandProvider::s_OnSetting1(IShellItemArray * /* psiItemArray */, IUnknown * /* pv */)
{
    WCHAR sz[100];
    HRESULT hr = LoadString(g_hInst, IDS_SETTING1, sz, ARRAYSIZE(sz)) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, sz, sz, MB_OK);
    }
    return hr;
}

HRESULT CFolderViewCommandProvider::s_OnSetting2(IShellItemArray * /* psiItemArray */, IUnknown * /* pv */)
{
    WCHAR sz[100];
    HRESULT hr = LoadString(g_hInst, IDS_SETTING2, sz, ARRAYSIZE(sz)) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, sz, sz, MB_OK);
    }
    return hr;
}

HRESULT CFolderViewCommandProvider::s_OnSetting3(IShellItemArray * /* psiItemArray */, IUnknown * /* pv */)
{
    WCHAR sz[100];
    HRESULT hr = LoadString(g_hInst, IDS_SETTING3, sz, ARRAYSIZE(sz)) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, sz, sz, MB_OK);
    }
    return hr;
}

HRESULT CFolderViewCommandEnumerator::_CreateCommandFromCommandItem(FVCOMMANDITEM *pfvci, IExplorerCommand **ppExplorerCommand)
{
    CFolderViewCommand *pCommand = new (std::nothrow) CFolderViewCommand(pfvci);
    HRESULT hr = pCommand ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pCommand->QueryInterface(IID_PPV_ARGS(ppExplorerCommand));
        pCommand->Release();
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommandEnumerator::Next(ULONG celt, IExplorerCommand** apUICommand, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;
    if (_uCurrent <= _uCommands)
    {
        UINT uIndex = 0;
        HRESULT hrLocal = S_OK;
        while (uIndex < celt && _uCurrent < _uCommands && SUCCEEDED(hrLocal))
        {
            hrLocal = _CreateCommandFromCommandItem((FVCOMMANDITEM*)&(_apfvci[_uCurrent]), &(apUICommand[uIndex]));
            uIndex++;
            _uCurrent++;
        }

        if (pceltFetched != NULL)
        {
            *pceltFetched = uIndex;
        }

        if (uIndex == celt)
        {
            hr = S_OK;
        }
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommandEnumerator::Skip(ULONG celt)
{
    _uCurrent += celt;

    HRESULT hr = S_OK;
    if (_uCurrent > _uCommands)
    {
        _uCurrent = _uCommands;
        hr = S_FALSE;
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommandEnumerator::Reset()
{
    _uCurrent = 0;
    return S_OK;
}

IFACEMETHODIMP CFolderViewCommand::GetTitle(IShellItemArray * /* psiItemArray */, LPWSTR *ppszName)
{
    *ppszName = NULL;
    HRESULT hr = E_FAIL;
    if (_pfvci)
    {
        WCHAR sz[100];
        hr = LoadString(g_hInst, _pfvci->dwTitleID, sz, ARRAYSIZE(sz)) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            hr = SHStrDup(sz, ppszName);
        }
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommand::GetToolTip(IShellItemArray * /* psiItemArray */, LPWSTR *ppszInfotip)
{
    *ppszInfotip = NULL;
    HRESULT hr = E_FAIL;
    if (_pfvci)
    {
        WCHAR sz[100];
        hr = LoadString(g_hInst, _pfvci->dwToolTipID, sz, ARRAYSIZE(sz)) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            hr = SHStrDup(sz, ppszInfotip);
        }
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommand::GetIcon(IShellItemArray * /* psiItemArray */, LPWSTR *ppszIcon)
{
    *ppszIcon = NULL;
    HRESULT hr = E_FAIL;
    if (_pfvci)
    {
        hr = SHStrDup(_pfvci->pszIcon, ppszIcon);
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommand::GetState(IShellItemArray *psiItemArray, BOOL /* fOkToBeSlow */, EXPCMDSTATE *pCmdState)
{
    HRESULT hr = S_OK;
    *pCmdState = ECS_DISABLED;
    if (_pfvci)
    {
        if (*(_pfvci->pguidCanonicalName) == GUID_Display)
        {
            if (psiItemArray)
            {
                DWORD dwNumItems;
                hr = psiItemArray->GetCount(&dwNumItems);
                if ((SUCCEEDED(hr)) && (dwNumItems > 0))
                {
                    *pCmdState = ECS_ENABLED;
                }
            }
        }
        else
        {
            *pCmdState = ECS_ENABLED;
        }
    }

    return hr;
}

IFACEMETHODIMP CFolderViewCommand::GetFlags(EXPCMDFLAGS *pFlags)
{
    if (_pfvci)
    {
        *pFlags = _pfvci->ecFlags;
    }
    return S_OK;
}


IFACEMETHODIMP CFolderViewCommand::GetCanonicalName(GUID *pguidCommandName)
{
    if (_pfvci)
    {
        *pguidCommandName = *(_pfvci->pguidCanonicalName);
    }
    return S_OK;
}

IFACEMETHODIMP CFolderViewCommand::Invoke(IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    HRESULT hr = S_OK; // If no function defined - just return S_OK
    if (_pfvci && _pfvci->pfnInvoke)
    {
        hr = _pfvci->pfnInvoke(psiItemArray, pbc);
    }
    return hr;
}

IFACEMETHODIMP CFolderViewCommand::EnumSubCommands(IEnumExplorerCommand **ppEnum)
{
    CFolderViewCommandEnumerator *pFVCommandEnum = new (std::nothrow) CFolderViewCommandEnumerator(_pfvci->pFVCIChildren, _pfvci->uChildCommands);
    HRESULT hr = pFVCommandEnum ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pFVCommandEnum->QueryInterface(IID_PPV_ARGS(ppEnum));
        pFVCommandEnum->Release();
    }
    return hr;
}
