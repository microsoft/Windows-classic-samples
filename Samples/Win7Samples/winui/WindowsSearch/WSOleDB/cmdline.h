#pragma once

class CParamBase
{
public:
    CParamBase() : m_fExists(FALSE)
    {
    }
    virtual int Init(int argc, WCHAR * argv[]) = 0;
    BOOL Exists() { return m_fExists;  }
protected:
    BOOL m_fExists;
};

class CFlagParam: public CParamBase
{
public:
    CFlagParam(PCWSTR pszParamName) : m_pszParamName(pszParamName)
    {
    }
    virtual int Init(int argc, WCHAR * argv[]);
protected:
    PCWSTR m_pszParamName;
};

class CExclFlagParam: public CParamBase
{
public:
    CExclFlagParam(PCWSTR pszTrueParamName, PCWSTR pszFalseParamName) : m_fFlag(FALSE), m_pszTrueParamName(pszTrueParamName), m_pszFalseParamName(pszFalseParamName)
    {
    }
    virtual int Init(int argc, WCHAR * argv[]);
    BOOL Get() { return m_fFlag; }
protected:
    BOOL m_fFlag;
    PCWSTR m_pszTrueParamName;
    PCWSTR m_pszFalseParamName;
};

class CSetWSTRValueParam : public CParamBase
{
public:
    CSetWSTRValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_pszValue(NULL)
    {
    }
    virtual int Init(int argc, WCHAR * argv[]);
    PCWSTR Get() {  return m_pszValue; }
    BOOL Empty() {  return (NULL == m_pszValue); } // parameter exists but value was not set

protected:
    PCWSTR m_pszParamName;
    PCWSTR m_pszValue;
};

class CSetExprValueParam : public CParamBase
{
public:
    CSetExprValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_pszValue(NULL)
    {
    }
    virtual int Init(int argc, WCHAR * argv[]);
    PCWSTR Get() {  return m_pszValue; }
    BOOL Empty() {  return (NULL == m_pszValue); } // parameter exists but value was not set

protected:
    PCWSTR m_pszParamName;
    PCWSTR m_pszValue;
};

class CSetSyntaxValueParam : public CParamBase
{
public:
    CSetSyntaxValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_pszValue(NULL)
    {
    }
    virtual int Init(int argc, WCHAR *argv[]);
    SEARCH_QUERY_SYNTAX Get() { return m_syntaxValue; }
    BOOL Empty() { return (NULL == m_pszValue); }

protected:
    PCWSTR m_pszParamName;
    PCWSTR m_pszValue;
    SEARCH_QUERY_SYNTAX m_syntaxValue;
};

class CSetTermExpansionValueParam : public CParamBase
{
public:
    CSetTermExpansionValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_pszValue(NULL)
    {
    }
    virtual int Init(int argc, WCHAR *argv[]);
    SEARCH_TERM_EXPANSION Get() { return m_expansionValue; }
    BOOL Empty() { return (NULL == m_pszValue); }

protected:
    PCWSTR m_pszParamName;
    PCWSTR m_pszValue;
    SEARCH_TERM_EXPANSION m_expansionValue;
};

class CSetLCIDValueParam : public CParamBase
{
public:
    CSetLCIDValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_lcidValue(GetUserDefaultLCID())
    {
        m_Set = FALSE;
    }
    virtual int Init(int argc, WCHAR *argv[]);
    LCID Get() { return m_lcidValue; }
    BOOL Empty() { return m_Set; }

protected:
    PCWSTR m_pszParamName;
    LCID m_lcidValue;
    BOOL m_Set;
};

class CSetIntValueParam : public CParamBase
{
public:
    CSetIntValueParam(PCWSTR pszName) : m_pszParamName(pszName), m_iValue(0)
    {
        m_Set = FALSE;
    }
    virtual int Init(int argc, WCHAR *argv[]);
    int Get() { return m_iValue; }
    BOOL Empty() { return m_Set; }

protected:
    PCWSTR m_pszParamName;
    LCID m_iValue;
    BOOL m_Set;
};

class CContentLocaleParam : public CSetLCIDValueParam
{
public:
    CContentLocaleParam(LCID lcidDefValue) : CSetLCIDValueParam(L"ContentLocale")
    {
        m_lcidValue = lcidDefValue;
        m_fExists = TRUE;
    }
    virtual int Init(int argc, WCHAR *argv[]);
};

class CContentPropertiesParam : public CSetWSTRValueParam
{
public:
    CContentPropertiesParam(PCWSTR pszDefValue) : CSetWSTRValueParam(L"ContentProperties")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CKeywordLocaleParam : public CSetLCIDValueParam
{
public:
    CKeywordLocaleParam(LCID lcidDefValue) : CSetLCIDValueParam(L"KeywordLocale")
    {
        m_lcidValue = lcidDefValue;
        m_fExists = TRUE;
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CMaxResultsParam : public CSetIntValueParam
{
public:
    CMaxResultsParam(int iDefValue) : CSetIntValueParam(L"MaxResults")
    {
        m_iValue = iDefValue;
        m_fExists = TRUE;
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CExprParam : public CSetExprValueParam
{
public:
    CExprParam(PCWSTR pszDefValue) : CSetExprValueParam(L"")
    {
        m_pszValue = pszDefValue;
        m_fExists = TRUE;
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CSelectColumnsParam : public CSetWSTRValueParam
{
public:
    CSelectColumnsParam(PCWSTR pszDefValue) : CSetWSTRValueParam(L"Select")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CSortingParam : public CSetWSTRValueParam
{
public:
    CSortingParam(PCWSTR pszDefValue) : CSetWSTRValueParam(L"Sorting")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CSyntaxParam : public CSetSyntaxValueParam
{
public:
    CSyntaxParam(PCWSTR pszDefValue) : CSetSyntaxValueParam(L"Syntax")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;
            m_syntaxValue = SEARCH_ADVANCED_QUERY_SYNTAX; // default to AQS
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CTermExpansionParam : public CSetTermExpansionValueParam
{
public:
    CTermExpansionParam(PCWSTR pszDefValue) : CSetTermExpansionValueParam(L"TermExpansion")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;

            // Force default to None
            m_expansionValue = SEARCH_TERM_NO_EXPANSION;
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

class CWhereRestrictionsParam : public CSetWSTRValueParam
{
public:
    CWhereRestrictionsParam(PCWSTR pszDefValue) : CSetWSTRValueParam(L"Where")
    {
        if (pszDefValue)
        {
            m_pszValue = pszDefValue;
            m_fExists = TRUE;
        }
    }

    virtual int Init(int argc, WCHAR *argv[]);
};

HRESULT ParseParams(CParamBase** pParams, const DWORD dwParamCnt, int argc, WCHAR * argv[]);
