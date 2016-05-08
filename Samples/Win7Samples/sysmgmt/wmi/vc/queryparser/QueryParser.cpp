// **************************************************************************
//
// Copyright (c) Microsoft Corporation, All Rights Reserved
//
// File:  QueryParser.cpp 
//
// Description:
//      WMI Query Parser Sample.
//      This sample shows how to use the Query Parser.  It takes a query as a command
//      line argument and passes it off to the parser to be parsed and then uses various
//      functions to get pieces of the path. the sample has been designed to handle only simple

//		select queries.
//
// **************************************************************************


#include <windows.h>
#include <stdio.h>
#include <wmiutils.h>
#include <strsafe.h>

LPWSTR g_pQuery = 0;

//***************************************************************************
//
// DumpFeatureMasks
//
// Purpose: Outputs the mask in readable format
//
//***************************************************************************

void DumpFeatureMasks(unsigned __int64 mask)
{
    printf("Query Features:\n");

    if (mask & WMIQ_RPNF_WHERE_CLAUSE_PRESENT)
        printf("WMIQ_RPNF_WHERE_CLAUSE_PRESENT\n");

    if (mask & WMIQ_RPNF_QUERY_IS_CONJUNCTIVE)
        printf("WMIQ_RPNF_QUERY_IS_CONJUNCTIVE\n");

    if (mask & WMIQ_RPNF_QUERY_IS_DISJUNCTIVE)
        printf("WMIQ_RPNF_QUERY_IS_DISJUNCTIVE\n");

    if (mask & WMIQ_RPNF_PROJECTION)
        printf("WMIQ_RPNF_PROJECTION\n");

    if (mask & WMIQ_RPNF_FEATURE_SELECT_STAR)
        printf("WMIQ_RPNF_FEATURE_SELECT_STAR\n");

    if (mask & WMIQ_RPNF_EQUALITY_TESTS_ONLY)
        printf("WMIQ_RPNF_EQUALITY_TESTS_ONLY\n");

    if (mask & WMIQ_RPNF_COUNT_STAR)
        printf("WMIQ_RPNF_COUNT_STAR\n");

    if (mask & WMIQ_RPNF_PROP_TO_PROP_TESTS)
        printf("WMIQ_RPNF_PROP_TO_PROP_TESTS\n");

    if (mask & WMIQ_RPNF_ORDER_BY)
        printf("WMIQ_RPNF_ORDER_BY\n");

    if (mask & WMIQ_RPNF_ISA_USED)
        printf("WMIQ_RPNF_ISA_USED\n");

    if (mask & WMIQ_RPNF_GROUP_BY_HAVING)
        printf("WMIQ_RPNF_GROUP_BY_HAVING\n");

}

//***************************************************************************
//
// DumpSelectList
//
// Purpose: Lists the selected columns
//
//***************************************************************************

void DumpSelectList(ULONG uCount, SWbemQueryQualifiedName **pNames)
{
    unsigned u;

    printf("----BEGIN SELECT CLAUSE---\n");
    printf("Selected target columns/props = ");

    for (u = 0; u < uCount; u++)
    {
        SWbemQueryQualifiedName *p = pNames[u];

        // Print out current name.
        // Note that we support qualified names,
        // like p1.a,  p2.b, etc.
        // For simple cases, the m_uNameListSize is 1.
        // ===========================================

        for (unsigned u2 = 0; u2 < p->m_uNameListSize; u2++)
        {
            if (u2 > 0)
                printf(".");
            printf("%S", p->m_ppszNameList[u2]);
        }

        printf(", ");
    }

    printf("\n");
    printf("---END SELECT CLAUSE-----\n");
}

//***************************************************************************
//
// DumpSubexpression
//
// Purpose: dumps out one of the subexpressions that make up the where clause
//
//***************************************************************************

void DumpSubexpression(SWbemRpnQueryToken *pTemp)
{
    printf("---SUBEXPRESSION TOKEN---\n");

    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_LEFT_PROPERTY_NAME)
    {
        printf("    WMIQ_RPN_LEFT_PROPERTY_NAME\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_RIGHT_PROPERTY_NAME)
    {
        printf("    WMIQ_RPN_RIGHT_PROPERTY_NAME\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_CONST2)
    {
        printf("    WMIQ_RPN_CONST2\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_CONST)
    {
        printf("    WMIQ_RPN_CONST\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_RELOP)
    {
        printf("    WMIQ_RPN_RELOP\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_LEFT_FUNCTION)
    {
        printf("    WMIQ_RPN_LEFT_FUNCTION\n");
    }
    if (pTemp->m_uSubexpressionShape & WMIQ_RPN_RIGHT_FUNCTION)
    {
        printf("    WMIQ_RPN_RIGHT_FUNCTION\n");
    }

    switch (pTemp->m_uOperator)
    {
        case WMIQ_RPN_OP_UNDEFINED: printf("  Operator (Invalid) WMIQ_RPN_OP_UNDEFINED\n"); break;
        case WMIQ_RPN_OP_EQ : printf("  Operator =  (WMIQ_RPN_OP_EQ)\n"); break;
        case WMIQ_RPN_OP_NE : printf("  Operator != (WMIQ_RPN_OP_NE)\n"); break;
        case WMIQ_RPN_OP_GE : printf("  Operator >= (WMIQ_RPN_OP_GE)\n"); break;
        case WMIQ_RPN_OP_LE : printf("  Operator <= (WMIQ_RPN_OP_LE)\n"); break;
        case WMIQ_RPN_OP_LT : printf("  Operator <  (WMIQ_RPN_OP_LT)\n"); break;
        case WMIQ_RPN_OP_GT : printf("  Operator >  (WMIQ_RPN_OP_GT)\n"); break;
        case WMIQ_RPN_OP_LIKE : printf("  Operator LIKE (WMIQ_RPN_OP_LIKE)\n"); break;
        case WMIQ_RPN_OP_ISA  : printf("  Operator ISA  (WMIQ_RPN_OP_ISA)\n"); break;
        case WMIQ_RPN_OP_ISNOTA  : printf("  Operator ISNOTA (WMIQ_RPN_OP_ISNOTA)\n"); break;

        default:
            printf("  Operator <INVALID -- Internal Error>\n");
            break;
    }

    // Dump identifiers (propety names)


    if (pTemp->m_pRightIdent)
    {
        printf("  Right side identifier = ");
        SWbemQueryQualifiedName *p = pTemp->m_pRightIdent;
        for (unsigned u = 0; u < p->m_uNameListSize; u++)
        {
            if (u > 0)
                printf(".");
            printf("%S", p->m_ppszNameList[u]);
        }
        printf("\n");
    }

    if (pTemp->m_pLeftIdent)
    {
        printf("  Left side identifier = ");
        SWbemQueryQualifiedName *p = pTemp->m_pLeftIdent;
        for (unsigned u = 0; u < p->m_uNameListSize; u++)
        {
            if (u > 0)
                printf(".");
            printf("%S", p->m_ppszNameList[u]);
        }
        printf("\n");
    }

    printf("  Apparent Type of Query Constant is ");

    switch(pTemp->m_uConstApparentType)
    {
        case VT_NULL:
            printf("VT_NULL");
            break;
        case VT_I4:
            printf("VT_I4,  Value = %d\n", pTemp->m_Const.m_lLongVal);
            break;
        case VT_UI4:
            printf("VT_UI4, Value = %u\n", pTemp->m_Const.m_uLongVal);
            break;
        case VT_BOOL:
            printf("VT_BOOL, Value = %d\n", pTemp->m_Const.m_bBoolVal);
            break;
        case VT_R8:
            printf("VT_R8, Value = %f\n", pTemp->m_Const.m_dblVal);
            break;
        case VT_LPWSTR:
            printf("VT_LPWSTR, Value = %S\n", pTemp->m_Const.m_pszStrVal);
            break;
    }

    printf("  Function name applied to Left Side = %S\n", pTemp->m_pszLeftFunc);
    printf("  Function name applied to Right Side = %S\n", pTemp->m_pszRightFunc);

    printf("----END SUBEXPRESSION TOKEN---\n");
}

//***************************************************************************
//
// DumpWhereClause
//
// Purpose: Dumps out the where clause
//
//***************************************************************************

void DumpWhereClause(ULONG uCount, SWbemRpnQueryToken **pWhere)
{
    printf("---BEGIN WHERE CLAUSE---\n");

    unsigned u;

    for (u = 0; u < uCount; u++)
    {
        SWbemRpnQueryToken *pTemp = pWhere[u];

        switch(pTemp->m_uTokenType)
        {
            case WMIQ_RPN_TOKEN_EXPRESSION:
                DumpSubexpression(pTemp);
                break;

            case WMIQ_RPN_TOKEN_AND:
                printf(" Operator: AND\n");
                break;

            case WMIQ_RPN_TOKEN_OR:
                printf(" Operator: OR\n");
                break;

            case WMIQ_RPN_TOKEN_NOT:
                printf(" Operator: NOT\n");

            default:
                printf(" Operator: INVALID\n");
        }
    }

    printf("---END WHERE CLAUSE---\n");
}

//***************************************************************************
//
// DumpFromClause
//
// Purpose: Dumps out the from clause
//
//***************************************************************************

void DumpFromClause(SWbemRpnEncodedQuery *pQuery)
{
    printf("---FROM clause---\n");

    if (pQuery->m_uFromTargetType & WMIQ_RPN_FROM_PATH)
    {
        printf("  Selected from container/scope [%S]\n", pQuery->m_pszOptionalFromPath);
    }

    if (pQuery->m_uFromTargetType & WMIQ_RPN_FROM_UNARY)
    {
        printf("  Single class select = %S\n", pQuery->m_ppszFromList[0]);
    }
    else if (pQuery->m_uFromTargetType & WMIQ_RPN_FROM_CLASS_LIST)
    {
        for (unsigned u = 0; u < pQuery->m_uFromListSize; u++)
        {
            printf("  Selected class = %S\n", pQuery->m_ppszFromList[u]);
        }
    }
    else
    {
        printf("  No target classes selected; select ALL\n");
    }

    printf("---END FROM clause\n");
}

//***************************************************************************
//
// DumpRpn
//
// Purpose: Mainly calls the other routines which dump out the specific parts
//
//***************************************************************************

void DumpRpn(SWbemRpnEncodedQuery *pRpn)
{
    printf("------------RPN Encoded Query-----------------\n");
    printf("RPN Version = %d\n", pRpn->m_uVersion);
    printf("RPN Token Type = %d\n", pRpn->m_uTokenType);

    printf("Total detected features = %d  Feature Set = { ", pRpn->m_uDetectedArraySize);
    for (unsigned i = 0; i < pRpn->m_uDetectedArraySize; i++)
        printf("%d ", pRpn->m_puDetectedFeatures[i]);
    printf("}\n\n");

    DumpFeatureMasks(pRpn->m_uParsedFeatureMask);
    DumpSelectList(pRpn->m_uSelectListSize, pRpn->m_ppSelectList);
    DumpFromClause(pRpn);
    DumpWhereClause(pRpn->m_uWhereClauseSize, pRpn->m_ppRpnWhereClause);
}

//***************************************************************************
//
// TestQuery
//
// Purpose: Calls the parser to parse the query, and if all is well, calls
// the other routines to dump out the results.
//
//***************************************************************************

void TestQuery(IWbemQuery *pQuery)
{
    HRESULT hRes;
    ULONG uFeatures[] =
    {
        WMIQ_LF1_BASIC_SELECT,
        WMIQ_LF2_CLASS_NAME_IN_QUERY
    };

    hRes = pQuery->SetLanguageFeatures(0, sizeof(uFeatures)/sizeof(ULONG), uFeatures);

    hRes = pQuery->Parse(L"SQL", g_pQuery, 0);

    if (FAILED(hRes))
    {
        printf("Parse failed with error 0x%X\n", hRes);
        return;
    }


    SWbemRpnEncodedQuery *pRpn = 0;
    hRes = pQuery->GetAnalysis(
        WMIQ_ANALYSIS_RPN_SEQUENCE,
        0,
        (LPVOID *) &pRpn
        );

    if (SUCCEEDED(hRes))
    {
        printf("\n\nGot RPN Output\n");
        DumpRpn(pRpn);
        pQuery->FreeMemory(pRpn);
    }
}

//***************************************************************************
//
// main
//
// Purpose: Entry point for the application.
//
//***************************************************************************

void main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("\nusage:  QueryParser \"simple select query\"");
		printf("\nexample;  c:>QueryParser \"select RowA, RowB from tableName where propz=3\"");
        return;
    }

    wchar_t buf[8192];	
	HRESULT hr = StringCbPrintfW(buf, sizeof(buf), L"%S", argv[1]);
	if (FAILED(hr))
	{ 
		printf("Query string cannot exceed %d characters.\n", sizeof(buf)/sizeof(wchar_t) - 1);
		printf("StringCbPrintfW failed with error code: 0x%X.\n", hr);
		return;
	}

    g_pQuery = buf;

	wprintf(L"Query = %s\n", g_pQuery);


    // Standard COM initialization stuff.
    // ===================================

    CoInitializeEx(0, COINIT_MULTITHREADED);

    // Get the QueryParser object call the test code
    // =============================================

    IWbemQuery *pQuery = 0;

    hr = CoCreateInstance(CLSID_WbemQuery, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemQuery, (LPVOID *) &pQuery);

    if (hr != S_OK)
        printf("Failed to create Query object.\n");
    else
    {
        printf("Got a query parser object\n");
        TestQuery(pQuery);
        pQuery->Release();
    }

    CoUninitialize();
}


