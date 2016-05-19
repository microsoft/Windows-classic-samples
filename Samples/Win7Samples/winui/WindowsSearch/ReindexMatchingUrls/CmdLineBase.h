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
// of usage text, and grouping of related commands.

#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>


// Base class for a single command
class CCmdBase
{
public:
    CCmdBase(PCWSTR pszName, PCWSTR pszDescription, PCWSTR pszArgTemplate);
    virtual ~CCmdBase() {}

    // methods for executing the command, and obtaining usage information
    HRESULT Execute( PCWSTR* ppszArgs, int cArgs);
    void PrintUsage() const;
    PCWSTR GetName(bool fIncludePrefix = false) const;
    PCWSTR GetDescription() const;
    PCWSTR GetArgTemplate() const;
    void AddPrefix(PCWSTR pszPrefix);

    // toggles debug output for all classes derived from CCmdBase
    static void SetDebugMode(BOOL fDebug);

protected:
    // methods to be overridden by the command implementation
    virtual HRESULT v_ProcessArguments( PCWSTR*, int) { return S_OK; }
    virtual HRESULT v_ExecuteCommand() = 0;
    virtual void v_PrintInstructions() const {}

    // methods for generating output (use instead of printf)
    void Output(PCWSTR pszFormat, ...) const;
    void ParseError(PCWSTR pszFormat, ...);
    void RuntimeError(PCWSTR pszFormat, ...);

    // methods to add specify option handlers; call these from the subclass constructor
    template <class CCommandImpl>
    void AddStringOptionHandler(PCWSTR pszOption, PCWSTR pszUsage, HRESULT (CCommandImpl::*pmfnSetOption)(PCWSTR))
    {
        COptionHandler *pHandler = new (std::nothrow) CStringOptionHandler<CCommandImpl>(pmfnSetOption, pszOption, pszUsage);
        if (pHandler)
        {
            _mapOptionHandlers.Add(pszOption, &pHandler);
            delete pHandler;
        }
    }

    template <typename ARGVALUE> struct ARGENTRY; // forward decl
    template <class CCommandImpl, typename ARGVALUE>
    void AddEnumOptionHandler(PCWSTR pszOption, PCWSTR pszDescription, PCWSTR pszUsage,
                              HRESULT (CCommandImpl::*pmfnSetOption)(ARGVALUE),
                              const ARGENTRY<ARGVALUE> *pLookupTable, const UINT cLookupTable)
    {
        COptionHandler *pHandler = new (std::nothrow) CEnumeratedOptionHandler<CCommandImpl, ARGVALUE>(pmfnSetOption, pszOption, pszDescription, pszUsage, pLookupTable, cLookupTable);
        if (pHandler)
        {
            _mapOptionHandlers.Add(pszOption, &pHandler);
            delete pHandler;
        }
    }

    // Simple associative array implementation
    template <typename VALUE>
    class CStringMap
    {
    public:
        CStringMap() :
            _pHead(NULL),
            _ppInsertAt(&_pHead)
        {}

        ~CStringMap()
        {
            delete _pHead;
        }

        __inline bool IsEmpty() const { return _pHead == NULL; }

        HRESULT Add(PCWSTR pszKey, VALUE **ppValue)
        {
            MAPPING *pNew = new (std::nothrow) MAPPING;
            HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                hr = SHStrDup(pszKey, &pNew->pszKey);
                if (SUCCEEDED(hr))
                {
                    pNew->pValue = *ppValue;
                    *ppValue = NULL; // transfer ownership

                    *_ppInsertAt = pNew;
                    _ppInsertAt = &pNew->pNext;
                    pNew = NULL; // transfer ownership
                }
                delete pNew;
            }
            return hr;
        }

        HRESULT Find(PCWSTR pszKey, VALUE **ppValue) const
        {
            *ppValue = NULL;
            HRESULT hr = E_FAIL;
            for (MAPPING *pCur = _pHead; FAILED(hr) && pCur; pCur = pCur->pNext)
            {
                if (StrCmpIC(pszKey, pCur->pszKey) == 0)
                {
                    *ppValue = pCur->pValue;
                    hr = S_OK;
                }
            }
            return hr;
        }

        template <class CCaller>
        void ForEachValue(void (CCaller::*pmfnDo)(VALUE *pValue) const, const CCaller *pCmd) const
        {
            for (MAPPING *pCur = _pHead; pCur; pCur = pCur->pNext)
            {
                (pCmd->*pmfnDo)(pCur->pValue);
            }
        }

    private:
        struct MAPPING
        {
            MAPPING() :
                pszKey(NULL),
                pValue(NULL),
                pNext(NULL)
            {}

            ~MAPPING()
            {
                CoTaskMemFree(pszKey);
                delete pValue;
                delete pNext;
            }

            PWSTR pszKey;
            VALUE *pValue;
            MAPPING *pNext;
        };

        MAPPING *_pHead;
        MAPPING **_ppInsertAt;
    };

    // option handling framework
    class COptionHandler
    {
    public:
        virtual ~COptionHandler() {}
        virtual HRESULT v_SetOption(CCmdBase *pCmd, PCWSTR pszValue) const = 0;
        virtual void v_PrintUsage(const CCmdBase *pCmd) const = 0;
    };

    template <class CCommandImpl, typename ARGVALUE>
    class COptionHandlerBase :
        public COptionHandler
    {
    public:
        typedef HRESULT (CCommandImpl::*PMFNSETOPTION)(ARGVALUE);
        COptionHandlerBase(PMFNSETOPTION pmfnSetOption, PCWSTR pszName, PCWSTR pszUsage) :
            _pmfnSetOption(pmfnSetOption)
        {
            StringCchPrintf(_szName, ARRAYSIZE(_szName), L"%s[:ARG]", pszName);
            StringCchCopy(_szUsage, ARRAYSIZE(_szUsage), pszUsage);
        }

        HRESULT v_SetOption(CCmdBase *pCmd, PCWSTR pszValue) const
        {
            CCommandImpl *pCmdImpl = dynamic_cast<CCommandImpl*>(pCmd);
            return pCmd ? v_SetOption(pCmdImpl, pszValue) : E_UNEXPECTED;
        }

    protected:
        virtual HRESULT v_SetOption(CCommandImpl *pCmdImpl, PCWSTR pszValue) const = 0;
        PMFNSETOPTION _pmfnSetOption;
        WCHAR _szName[32];
        WCHAR _szUsage[256];
    };

    template <class CCommandImpl>
    class CStringOptionHandler :
        public COptionHandlerBase<CCommandImpl, PCWSTR>
    {
    public:
        CStringOptionHandler(PMFNSETOPTION pmfnSetOption, PCWSTR pszName, PCWSTR pszUsage) :
            COptionHandlerBase(pmfnSetOption, pszName, pszUsage)
        {}

        HRESULT v_SetOption(CCommandImpl *pCmd, PCWSTR pszValue) const
        {
            return (pCmd->*_pmfnSetOption)(pszValue);
        }

        void v_PrintUsage(const CCmdBase *pCmd) const
        {
            pCmd->Output(L" -%-18s %s\n", _szName, _szUsage);
        }
    };

    template <typename ARGVALUE>
    struct ARGENTRY
    {
        WCHAR szArg[32];
        ARGVALUE value;
        WCHAR szUsage[128];
    };

    template <class CCommandImpl, typename ARGVALUE>
    class CEnumeratedOptionHandler :
        public COptionHandlerBase<CCommandImpl, ARGVALUE>
    {
    public:
        CEnumeratedOptionHandler(PMFNSETOPTION pmfnSetOption, PCWSTR pszName, PCWSTR pszDescription, PCWSTR pszUsage,
                                 const ARGENTRY<ARGVALUE> *pLookupTable, const UINT cLookupTable) :
            COptionHandlerBase(pmfnSetOption, pszName, pszUsage),
            _cLookupTable(0)
        {
            StringCchCopy(_szDescription, ARRAYSIZE(_szDescription), pszDescription);
            _pLookupTable = new (std::nothrow) ARGENTRY<ARGVALUE>[cLookupTable];
            if (_pLookupTable)
            {
                _cLookupTable = cLookupTable;
                for (UINT i = 0; i < _cLookupTable; i++)
                {
                    _pLookupTable[i] = pLookupTable[i];
                }
            }
        }

        ~CEnumeratedOptionHandler()
        {
            delete[] _pLookupTable;
        }

        HRESULT v_SetOption(CCommandImpl *pCmd, PCWSTR pszValue) const
        {
            HRESULT hr = E_INVALIDARG;
            for (UINT iEntry = 0; FAILED(hr) && (iEntry < _cLookupTable); iEntry++)
            {
                if (!StrCmpICW(pszValue, _pLookupTable[iEntry].szArg))
                {
                    hr = (pCmd->*_pmfnSetOption)(_pLookupTable[iEntry].value);
                }
            }

            if (FAILED(hr))
            {
                pCmd->ParseError(L"Unrecognized %s: %s\n", _szDescription, pszValue);
            }
            return hr;
        }

        void v_PrintUsage(const CCmdBase *pCmd) const
        {
            pCmd->Output(L" -%-18s %s\n", _szName, _szUsage);
            for (UINT iEntry = 0; iEntry < _cLookupTable; iEntry++)
            {
                PCWSTR pszArg = _pLookupTable[iEntry].szArg;
                pCmd->Output(L"   %-19s %s\n", (pszArg[0] ? pszArg : L"<none>"), _pLookupTable[iEntry].szUsage);
            }
        }

    private:
        WCHAR _szDescription[32];
        ARGENTRY<ARGVALUE> *_pLookupTable;
        UINT _cLookupTable;
    };

private:
    HRESULT _ProcessOptions(PCWSTR** pppszArgs, int* pcArgs);
    void _PrintUsageForOption(COptionHandler *pOption) const;

    static BOOL _fDebug;
    BOOL _fPrintUsageAndExit;
    BOOL _fError;

    WCHAR _szName[32];
    WCHAR _szDescription[256];
    WCHAR _szArgTemplate[128];
    WCHAR _szPrefixedName[MAX_PATH];
    CStringMap<COptionHandler> _mapOptionHandlers;
};

// command implementation that server as a wrapper for a set of sub-commands, ala netsh.exe
class CMetaCommand :
    public CCmdBase
{
public:
    typedef HRESULT (*PFNCREATECOMMAND)(CCmdBase** ppCmd, PCWSTR pszPrefix);

    CMetaCommand(PCWSTR pszName, PCWSTR pszDescription, PFNCREATECOMMAND* pCmds, int cCmds);

    virtual void v_PrintInstructions() const;

    template <class CCmdType>
    static HRESULT Create(CCmdBase** ppCmd, PCWSTR pszPrefix)
    {
        HRESULT hr = E_OUTOFMEMORY;
        *ppCmd = new (std::nothrow) CCmdType();
        if (*ppCmd)
        {
            (*ppCmd)->AddPrefix(pszPrefix);
            hr = S_OK;
        }
        return hr;
    }

protected:
    virtual HRESULT v_ProcessArguments( PCWSTR* ppszArgs, int cArgs);
    virtual HRESULT v_ExecuteCommand();

    HRESULT AddCommand(PFNCREATECOMMAND pfnCreate);

private:
    void _PrintDescriptionForCommand(CCmdBase *pCmd) const;

    CStringMap<CCmdBase> _mapCmds;
    CCmdBase *_pSpecifiedCmd; // weak reference
    PCWSTR* _ppszArgs;
    int _cArgs;
};

