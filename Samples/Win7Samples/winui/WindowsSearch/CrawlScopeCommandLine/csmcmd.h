#pragma once

#pragma comment(lib, "SearchSDK.lib")

#include <iostream>
#include <iomanip>
#include <windows.h>
#include <assert.h>
#include <SearchAPI.h>

using namespace std;

// Base abstract class for command line parsing classes
class CParamBase
{
public:
    CParamBase():m_fExists(FALSE){}
    virtual int Init(int argc, wchar_t *argv[], int& rParamsProcessed) = 0;
    BOOL Exists() { return m_fExists;   }
protected:
    BOOL m_fExists;
};

// Simple flag-type parameter, no value, just
// checking if it's present in command line or not.
class CFlagParam: public CParamBase
{
public:
    CFlagParam(PCWSTR szParamName):
        m_szParamName(szParamName)
    {}
    virtual int Init(int argc, wchar_t *argv[], int& rParamsProcessed);
protected:
    PCWSTR m_szParamName;
};

// Parameter with two alternative values, one value is associated with TRUE,
// another one is FALSE correspondingly.
class CExclFlagParam: public CParamBase
{
public:
    CExclFlagParam(PCWSTR szTrueParamName, PCWSTR szFalseParamName):
        m_fFlag(FALSE),
        m_szTrueParamName(szTrueParamName),
        m_szFalseParamName(szFalseParamName)
    {}
    virtual int Init(int argc, wchar_t *argv[], int& rParamsProcessed);
    BOOL Get() {    return m_fFlag; }
protected:
    BOOL m_fFlag;
    PCWSTR m_szTrueParamName;
    PCWSTR m_szFalseParamName;
};

// Parameter followed by some value
class CSetValueParam: public CParamBase
{
public:
    CSetValueParam(PCWSTR szName):
        m_szParamName(szName),
        m_szValue(NULL)
    {}
    virtual int Init(int argc, wchar_t *argv[], int& rParamsProcessed);
    PCWSTR Get() {    return m_szValue; }

protected:
    PCWSTR m_szParamName;
    PCWSTR m_szValue;
};

// Command line parsing function
extern int ParseParams(CParamBase** pParams, const DWORD dwParamCnt, int argc, WCHAR * argv[]);
