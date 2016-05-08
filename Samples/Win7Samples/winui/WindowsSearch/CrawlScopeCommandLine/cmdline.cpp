#include "csmcmd.h"

int CFlagParam::Init(int argc, wchar_t *argv[], int& rParamsProcessed)
{
    rParamsProcessed = 0;
    if (argc && ('/' == argv[0][0] || '-' == argv[0][0]) && !_wcsicmp(argv[0] + 1, m_szParamName))
    {
        m_fExists = TRUE;
        rParamsProcessed = 1;
    }

    return ERROR_SUCCESS;
}

int CExclFlagParam::Init(int argc, wchar_t *argv[], int& rParamsProcessed)
{
    int iRes = ERROR_SUCCESS;
    rParamsProcessed = 0;
    if (argc &&('/' == argv[0][0] || '-' == argv[0][0]))
    {
        if (!_wcsicmp(&(argv[0][1]), m_szTrueParamName))
        {
            if (!m_fExists)
            {
                m_fExists = TRUE;
                m_fFlag = TRUE;
                rParamsProcessed = 1;
            }
            else
            {
                iRes = ERROR_INVALID_PARAMETER;
            }
        }
        else if (!_wcsicmp(&(argv[0][1]), m_szFalseParamName))
        {
            if (!m_fExists)
            {
                m_fExists = TRUE;
                m_fFlag = FALSE;
                rParamsProcessed = 1;
            }
            else
            {
                iRes = ERROR_INVALID_PARAMETER;
            }
        }
    }

    if (ERROR_INVALID_PARAMETER == iRes)
    {
        wcout << L"/" << m_szTrueParamName << L" and /" <<
                         m_szFalseParamName << L" parameters can't be used together!" << endl;
    }

    return iRes;
}

int CSetValueParam::Init(int argc, wchar_t *argv[], int& rParamsProcessed)
{
    int iRes = ERROR_SUCCESS;
    rParamsProcessed = 0;
    if (('/' == argv[0][0] || '-' == argv[0][0]) && !_wcsicmp(argv[0] + 1, m_szParamName))
    {
        if (!m_fExists)
        {
            m_fExists = TRUE;
            if (1 < argc &&           // if it's not last word in command line
                '/' != argv[1][0] &&  // and not followed by other parameter
                '-' != argv[1][0])    // Parameter values starting with '/' and '-' are not supported
            {
                m_szValue = argv[1];
                rParamsProcessed = 2;
            }
            else
            {
                iRes = ERROR_INVALID_PARAMETER;
                wcout << L"No valid value following parameter /" << m_szParamName << L"!" << endl;
            }
        }
        else
        {
            iRes = ERROR_INVALID_PARAMETER;
            wcout << L"More than one instane of /" << m_szParamName << L" were found in command line!" << endl;
        }
    }

    return iRes;
}

int ParseParams(CParamBase** pParams, const DWORD dwParamCnt, int argc, WCHAR * argv[])
{
    int iRes = ERROR_SUCCESS;
    int i = 0;
    while (ERROR_SUCCESS == iRes && argc > i)
    {
        int iIncrement = 0;
        for (DWORD j = 0; ERROR_SUCCESS == iRes && j < dwParamCnt; j++)
        {
            iRes = pParams[j]->Init(argc - i, argv + i, iIncrement);
            if (ERROR_SUCCESS == iRes && 0 != iIncrement)
            {
                i += iIncrement;
                break;
            }
        }

        if (ERROR_SUCCESS == iRes && 0 == iIncrement)
        {
            iRes = ERROR_INVALID_PARAMETER;
            wcout << L"Unknown parameter: " << argv[i] << endl;
        }
    }

    return iRes;
}
