// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This code is not part of SDK sample itself.
//
// The code contained in CmdLineBase.h/.cpp provides the boiler-plate code to create a simple
// command-line applications with built-in support for option processing, automatic generation
// of usage text, and grouping and subclassing of related commands.

#include "CmdLineBase.h"


CCmdBase::CCmdBase(PCWSTR pszName, PCWSTR pszDescription, PCWSTR pszArgTemplate) :
    _fPrintUsageAndExit(FALSE),
    _fError(FALSE)
{
    StringCchCopy(_szName,         ARRAYSIZE(_szName),         pszName);
    StringCchCopy(_szPrefixedName, ARRAYSIZE(_szPrefixedName), pszName);
    StringCchCopy(_szDescription,  ARRAYSIZE(_szDescription),  pszDescription);
    StringCchCopy(_szArgTemplate,  ARRAYSIZE(_szArgTemplate),  pszArgTemplate);
}

HRESULT CCmdBase::Execute(PCWSTR* ppszArgs, int cArgs)
{
    HRESULT hr;
    PCWSTR pszFirstArg = cArgs ? ppszArgs[0] : NULL;
    if (pszFirstArg &&
        (pszFirstArg[0] == L'-' || pszFirstArg[0] == L'/') &&
        (!StrCmpICW(++pszFirstArg, L"help") ||
         !StrCmpICW(pszFirstArg, L"h") ||
         !StrCmpICW(pszFirstArg, L"?")))
    {
        hr = S_FALSE;
        PrintUsage();
    }
    else
    {
        hr = _ProcessOptions(&ppszArgs, &cArgs);
        if (SUCCEEDED(hr))
        {
            hr = v_ProcessArguments(ppszArgs, cArgs);
            if (SUCCEEDED(hr))
            {
                hr = v_ExecuteCommand();
            }
        }
    }
    return hr;
}

HRESULT CCmdBase::_ProcessOptions(PCWSTR** pppszArgs, int* pcArgs)
{
    HRESULT hr = E_INVALIDARG;

    if (pppszArgs && *pppszArgs && pcArgs)
    {
        PCWSTR* ppszArgs = *pppszArgs;
        int cArgs = *pcArgs;

        // process prefixed options using registered handlers
        hr = S_OK;
        int iArg = 0;
        while (SUCCEEDED(hr) &&
               (iArg < cArgs) &&
               (ppszArgs[iArg][0] == L'-' ||
                ppszArgs[iArg][0] == L'/'))
        {
            PCWSTR pszOptionName = &(ppszArgs[iArg][1]);
            PWSTR pszOptionArgs = const_cast<PWSTR>(wcschr(pszOptionName, L':'));
            if (pszOptionArgs)
            {
                *pszOptionArgs = L'\0';
                pszOptionArgs++;
            }
            else
            {
                pszOptionArgs = L"";
            }

            COptionHandler *pHandler;
            hr = _mapOptionHandlers.Find(pszOptionName, &pHandler);
            if (SUCCEEDED(hr))
            {
                hr = pHandler->v_SetOption(this, pszOptionArgs);
            }
            else
            {
                ParseError(L"Unrecognized option: %s%s%s\n",
                           ppszArgs[iArg],
                           (wcslen(pszOptionArgs) ? L":" : L""),
                           pszOptionArgs);
            }

            iArg++;
        }

        if (SUCCEEDED(hr))
        {
            // leave remaining arguments for the derived class
            *pppszArgs = ppszArgs + iArg;
            *pcArgs = cArgs - iArg;
        }
    }

    return hr;
}

void CCmdBase::_PrintUsageForOption(COptionHandler *pOption) const
{
    pOption->v_PrintUsage(this);
}

void CCmdBase::PrintUsage() const
{
    bool fHaveOptions = !_mapOptionHandlers.IsEmpty();
    Output(L"Usage: %s %s%s\n\n", GetName(true), (fHaveOptions ? L"[OPTIONS] " : L""), GetArgTemplate());
    if (_szDescription[0])
    {
        Output(L"%s\n\n", GetDescription());
    }
    if (fHaveOptions)
    {
        Output(L"Options:\n");
        _mapOptionHandlers.ForEachValue(&CCmdBase::_PrintUsageForOption, this);
        Output(L"\n");
    }
    v_PrintInstructions();
}

PCWSTR CCmdBase::GetName(bool fIncludePrefix) const
{
    return fIncludePrefix ? _szPrefixedName : _szName;
}

PCWSTR CCmdBase::GetDescription() const
{
    return _szDescription;
}

PCWSTR CCmdBase::GetArgTemplate() const
{
    return _szArgTemplate;
}

void CCmdBase::AddPrefix(PCWSTR pszPrefix)
{
    WCHAR szNewPrefix[MAX_PATH];
    StringCchPrintf(szNewPrefix, ARRAYSIZE(szNewPrefix), L"%s %s", _szPrefixedName, pszPrefix);
    StringCchCopy(_szPrefixedName, ARRAYSIZE(_szPrefixedName), szNewPrefix);
}

void CCmdBase::Output(PCWSTR pszFormat, ...) const
{
    va_list args;
    va_start(args, pszFormat);

    vwprintf(pszFormat, args);

    va_end(args);
}

void CCmdBase::ParseError(PCWSTR pszFormat, ...)
{
    if (!_fError)
    {
        _fError = TRUE;
        va_list args;
        va_start(args, pszFormat);

        wprintf(L"ERROR - ");
        vwprintf(pszFormat, args);
        wprintf(L"\n");
        PrintUsage();

        va_end(args);
    }
}

void CCmdBase::RuntimeError(PCWSTR pszFormat, ...)
{
    _fError = TRUE;
    va_list args;
    va_start(args, pszFormat);

    wprintf(L"ERROR - ");
    vwprintf(pszFormat, args);

    va_end(args);
}

CMetaCommand::CMetaCommand(PCWSTR pszName, PCWSTR pszDescription, CMetaCommand::PFNCREATECOMMAND* pCmds, int cCmds) :
    CCmdBase(pszName, pszDescription, L"SUBCOMMAND"),
    _pSpecifiedCmd(NULL),
    _ppszArgs(NULL),
    _cArgs(0)
{
    HRESULT hr = S_OK;
    for (int iCmd = 0; SUCCEEDED(hr) && (iCmd < cCmds); iCmd++)
    {
        hr = AddCommand(pCmds[iCmd]);
    }
}

HRESULT CMetaCommand::AddCommand(CMetaCommand::PFNCREATECOMMAND pfnCreate)
{
    CCmdBase *pCmd;
    HRESULT hr = pfnCreate(&pCmd, GetName(true));
    if (SUCCEEDED(hr))
    {
        hr = _mapCmds.Add(pCmd->GetName(), &pCmd);
        delete pCmd;
    }
    return hr;
}

HRESULT CMetaCommand::v_ProcessArguments(PCWSTR* ppszArgs, int cArgs)
{
    HRESULT hr = E_UNEXPECTED;

    if (!_pSpecifiedCmd)
    {
        hr = E_INVALIDARG;
        if (cArgs >= 1)
        {
            // look up the command name
            hr = _mapCmds.Find(ppszArgs[0], &_pSpecifiedCmd);
            if (SUCCEEDED(hr))
            {
                // save the remaining arguments for the specified command
                // this assumes that the lifetime of ppszArgs and cArgs is tied to the calling function, CCmdBase::Execute
                _ppszArgs = ppszArgs + 1;
                _cArgs = cArgs - 1;
            }
            else
            {
                ParseError(L"Command not recognized: %s\n", ppszArgs[0]);
            }
        }
        else
        {
            ParseError(L"No command specified.\n");
        }
    }

    return hr;
}

HRESULT CMetaCommand::v_ExecuteCommand()
{
    return _pSpecifiedCmd ? _pSpecifiedCmd->Execute(_ppszArgs, _cArgs) : E_UNEXPECTED;
}

void CMetaCommand::_PrintDescriptionForCommand(CCmdBase *pCmd) const
{
    Output(L"  %-12s%s\n", pCmd->GetName(), pCmd->GetDescription());
}

void CMetaCommand::v_PrintInstructions() const
{
    Output(L"Supported commands:\n");
    _mapCmds.ForEachValue(&CMetaCommand::_PrintDescriptionForCommand, this);
}
