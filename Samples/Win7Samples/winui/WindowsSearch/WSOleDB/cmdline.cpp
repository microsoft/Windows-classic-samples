#include "WSOleDB.h"
#include "cmdline.h"

using namespace std;

int CFlagParam::Init(int argc, WCHAR * argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            break;
        }
    }
    return ERROR_SUCCESS;
}

int CExclFlagParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if ('/' == argv[i][0] || '-' == argv[i][0])
        {
            if (!_wcsicmp(&(argv[i][1]), m_pszTrueParamName))
            {
                if (!m_fExists)
                {
                    m_fExists = TRUE;
                    m_fFlag = TRUE;
                }
                else
                {
                    iRes = ERROR_INVALID_PARAMETER;
                }
            }
            else if (!_wcsicmp(&(argv[i][1]), m_pszFalseParamName))
            {
                if (!m_fExists)
                {
                    m_fExists = TRUE;
                    m_fFlag = FALSE;
                }
                else
                {
                    iRes = ERROR_INVALID_PARAMETER;
                }
            }
        }
    }

    if (ERROR_INVALID_PARAMETER == iRes)
    {
        wcout << L"/" << m_pszTrueParamName << L" and /" <<
                         m_pszFalseParamName << L" parameters can't be used together!" << endl;
    }
    else if (!m_fExists)
    {
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CSetWSTRValueParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            if (i + 1 < argc &&         // if it's not last word in command line
                '/' != argv[i+1][0] &&  // and not followed by other parameter
                '-' != argv[i+1][0])    // Parameter values starting with '/' and '-' are not supported
            {
                m_pszValue = argv[i + 1];
            }
            break;
        }
    }
    return iRes;
}

int CSetLCIDValueParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            if (i + 1 < argc &&         // if it's not last word in command line
                '/' != argv[i+1][0] &&  // and not followed by other parameter
                '-' != argv[i+1][0])    // Parameter values starting with '/' and '-' are not supported
            {
                m_lcidValue = _wtoi(argv[i + 1]);
            }
            break;
        }
    }
    return iRes;
}

int CSetIntValueParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            if (i + 1 < argc &&         // if it's not last word in command line
                '/' != argv[i+1][0])    // and not followed by other parameter
            {
                m_iValue = _wtoi(argv[i + 1]);
            }
            break;
        }
    }
    return iRes;
}

int CSetSyntaxValueParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            if (i + 1 < argc &&         // if it's not last word in command line
                '/' != argv[i+1][0] &&  // and not followed by other parameter
                '-' != argv[i+1][0])    // Parameter values starting with '/' and '-' are not supported
            {
                if (_wcsicmp(argv[i + 1], L"None") == 0)
                {
                    m_syntaxValue = SEARCH_NO_QUERY_SYNTAX;
                }
                else if (_wcsicmp(argv[i + 1], L"AQS") == 0)
                {
                    m_syntaxValue = SEARCH_ADVANCED_QUERY_SYNTAX;
                }
                else if (_wcsicmp(argv[i + 1], L"NQS") == 0)
                {
                    m_syntaxValue = SEARCH_NATURAL_QUERY_SYNTAX;
                }
                else
                {
                    iRes = ERROR_INVALID_PARAMETER;
                    wcout << argv[i + 1] << L" is not a valid query syntax!" << endl;
                }
            }
            break;
        }
    }
    return iRes;
}

int CSetExprValueParam::Init(int argc, WCHAR *argv[])
{
    int iRes;

    // This is just a param with no leading argument, and it must be the last word in the command line

    m_fExists = (L'/' != argv[argc - 1][0] && L'-' != argv[argc - 1][0]);
    if (m_fExists)
    {
        m_pszValue = argv[argc - 1];
        iRes = ERROR_SUCCESS;
    }
    else
    {
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CSetTermExpansionValueParam::Init(int argc, WCHAR* argv[])
{
    int iRes = ERROR_SUCCESS;
    for (int i = 0; i < argc; i++)
    {
        if (('/' == argv[i][0] || '-' == argv[i][0]) && !_wcsicmp(argv[i] + 1, m_pszParamName))
        {
            m_fExists = TRUE;
            if (i + 1 < argc &&         // if it's not last word in command line
                '/' != argv[i+1][0] &&  // and not followed by other parameter
                '-' != argv[i+1][0])    // Parameter values starting with '/' and '-' are not supported
            {
                if (_wcsicmp(argv[i + 1], L"None") == 0)
                {
                    m_expansionValue = SEARCH_TERM_NO_EXPANSION;
                }
                else if (_wcsicmp(argv[i + 1], L"PrefixAll") == 0)
                {
                    m_expansionValue = SEARCH_TERM_PREFIX_ALL;
                }
                else if (_wcsicmp(argv[i + 1], L"StemAll") == 0)
                {
                    m_expansionValue = SEARCH_TERM_STEM_ALL;
                }
                else
                {
                    iRes = ERROR_INVALID_PARAMETER;
                    wcout << argv[i + 1] << L" is not a valid term expansion setting!" << endl;
                }
            }
            break;
        }
    }
    return iRes;
}

int CContentLocaleParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetLCIDValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CContentPropertiesParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetWSTRValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CKeywordLocaleParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetLCIDValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CMaxResultsParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetIntValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CSelectColumnsParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetWSTRValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CSortingParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetWSTRValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CSyntaxParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetSyntaxValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CTermExpansionParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetTermExpansionValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CWhereRestrictionsParam::Init(int argc, WCHAR *argv[])
{
    int iRes = CSetWSTRValueParam::Init(argc, argv);
    if (Empty())
    {
        wcout << L"/" << m_pszParamName << L" parameter must follow nonempty value!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int CExprParam::Init(int argc, WCHAR *argv[])
{
    return CSetExprValueParam::Init(argc, argv);
}

HRESULT ParseParams(CParamBase** pParams, const DWORD dwParamCnt, int argc, WCHAR * argv[])
{
    int iRes = ERROR_SUCCESS;
    for (DWORD i = 0; ERROR_SUCCESS == iRes && i < dwParamCnt; i++)
    {
        iRes = pParams[i]->Init(argc, argv);
    }
    return HRESULT_FROM_WIN32(iRes);
}
