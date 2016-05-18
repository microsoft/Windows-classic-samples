// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ComSpyAudit.h"
#ifndef DBNTWIN32
#define DBNTWIN32
#include <SQL.h>
#include <SQLEXT.h>
#include <ODBCINST.h>
#endif // DBNTWIN32 
#include <ODBCINST.h>
#include "ComSpySqlAudit.h"
#include <strsafe.h>


// Warning: constructing SQL statements from input using printf and similar
// functions is inherently insecure and should not be used in production code.
// Consider binding parameter markers with SQLBindParameter.
#define SQL_EXEC_STATEMENT(FORMAT, ...)                                     \
    {                                                                       \
        _ASSERTE(m_henv && m_hdbc && m_hstmt);                              \
        if (!m_henv || !m_hdbc || !m_hstmt)                                 \
            return E_UNEXPECTED;                                            \
        WCHAR wszBuffer[1024];                                              \
        HRESULT hrString = StringCchPrintfEx(                               \
            wszBuffer, ARRAYSIZE(wszBuffer),                                \
            NULL, NULL, STRSAFE_NO_TRUNCATION,                              \
            FORMAT, __VA_ARGS__);                                           \
        _ASSERTE(SUCCEEDED(hrString));                                      \
        SQLRETURN rc;                                                       \
        rc = SQLExecDirect(m_hstmt, wszBuffer, SQL_NTS);                    \
        if (!SQL_SUCCEEDED(rc))                                             \
        {                                                                   \
            ATLTRACE(L"Warning: SQLExecDirect failed with message %s\n",    \
                GetLastSqlErrorMessage(wszBuffer, ARRAYSIZE(wszBuffer)));   \
            return E_FAIL;                                                  \
        }                                                                   \
    }

long CComSpySqlAudit::PerfCountToTickCount(LONGLONG perfCount)
{
    long lTickCount = 0;
    if (m_PerformanceFrequency != 0)
        lTickCount = (long)((1000 * perfCount) / m_PerformanceFrequency);

    return lTickCount;
}

inline LPCWSTR YesOrNo(BOOL b)
{
    return b ? L"Yes" : L"No";
}

inline LPCWSTR TrueOrFalse(BOOL b)
{
    return b ? L"True" : L"False";
}


//////////////////////////Thread Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnThreadStart(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszThreadID,
                LPCWSTR pwszW2KThreadID,
                DWORD dwThreadCnt)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, W2KThreadID, ThreadCnt) \
             values ('%s',%d,'%s','%s','%s', %d)",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            pwszW2KThreadID,
            dwThreadCnt);

    return S_OK;
}


STDMETHODIMP CComSpySqlAudit::OnThreadTerminate(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszThreadID,
                LPCWSTR pwszW2KThreadID,
                DWORD dwThreadCnt
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, W2KThreadID, ThreadCnt) \
             values ('%s',%d,'%s','%s','%s', %d)",
            L"OnThreadTerminate",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            pwszW2KThreadID,
            dwThreadCnt);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnThreadBindToApartment(
                            LONGLONG perfCount,
                            LPCWSTR pwszGuidApp,
                            LPCWSTR pwszThreadID,                    
                            LPCWSTR pwszAptID,    
                            DWORD dwActivityCnt)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, ApartmentID, ActivityCnt) \
             values ('%s', %d,'%s','%s','%s', %d)",
            L"OnThreadBindToApartment",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            pwszAptID,
            dwActivityCnt);

    return S_OK;
}


STDMETHODIMP CComSpySqlAudit::OnThreadUnBind(
                             LONGLONG perfCount,
                             LPCWSTR pwszGuidApp,
                             LPCWSTR pwszThreadID,                    
                             LPCWSTR pwszAptID,    
                             DWORD dwActivityCnt
                )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, Threadid, ApartmentID, ActivityCnt) \
             values ('%s',%d,'%s','%s','%s', %d)",
            L"OnThreadUnBind",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            pwszAptID,
            dwActivityCnt);

    return S_OK;
}


STDMETHODIMP CComSpySqlAudit::OnThreadAssignApartment(
                            LONGLONG perfCount,
                            LPCWSTR pwszGuidApp,
                            LPCWSTR pwszGuidActivityID,
                            LPCWSTR pwszAptID    )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ActivityID, ApartmentID) \
             values ('%s',%d,'%s','%s','%s')",
            L"OnThreadAssignApartment",
            lTickCount,
            pwszGuidApp,
            pwszGuidActivityID,
            pwszAptID);            

    return S_OK;
}



STDMETHODIMP CComSpySqlAudit::OnThreadUnassignApartment(
                     LONGLONG perfCount,
                     LPCWSTR pwszGuidApp,                
                     LPCWSTR pwszAptID    )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ApartmentID) \
             values ('%s', %d,'%s','%s')",
            L"OnThreadUnAssignApartment",
            lTickCount,
            pwszGuidApp,             
            pwszAptID);            

    return S_OK;
}    
            

STDMETHODIMP CComSpySqlAudit::OnThreadWorkEnque(
                     LONGLONG perfCount,
                     LPCWSTR pwszGuidApp,
                     LPCWSTR pwszThreadID,                    
                     LPCWSTR MsgWorkID,    
                     DWORD dwQueueLen)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, MsgWorkID,QueueLen) \
             values ('%s', %d,'%s','%s','%s', %d)",
            L"OnThreadWorkEnque",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            MsgWorkID,
            dwQueueLen);            

    return S_OK;
}
                
STDMETHODIMP CComSpySqlAudit::OnThreadWorkPrivate(
                     LONGLONG perfCount,
                     LPCWSTR pwszGuidApp,
                     LPCWSTR pwszThreadID,                    
                     LPCWSTR MsgWorkID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, MsgWorkID) \
             values ('%s', %d, '%s','%s','%s')",
            L"OnThreadWorkPrivate",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            MsgWorkID);            

    return S_OK;
}                    
                
STDMETHODIMP CComSpySqlAudit::OnThreadWorkPublic(
                             LONGLONG perfCount,
                             LPCWSTR pwszGuidApp,
                             LPCWSTR pwszThreadID,                    
                             LPCWSTR MsgWorkID,    
                             DWORD dwQueueLen)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, MsgWorkID,QueueLen) \
             values ('%s', %d,'%s','%s','%s', %d)",
            L"OnThreadWorkPublic",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            MsgWorkID,
            dwQueueLen);            

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnThreadWorkRedirect(
                            LONGLONG perfCount,
                            LPCWSTR pwszGuidApp,
                            LPCWSTR pwszThreadID,                    
                            LPCWSTR MsgWorkID,    
                            DWORD dwQueueLen,
                            LPCWSTR pwszThreadNum)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, MsgWorkID,QueueLen, ThreadNum) \
             values ('%s',%d,'%s','%s','%s', %d, '%s')",
            L"OnThreadWorkPublic",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            MsgWorkID,
            dwQueueLen,
            pwszThreadNum);            

    return S_OK;
}
                
STDMETHODIMP CComSpySqlAudit::OnThreadWorkReject(
                             LONGLONG perfCount,
                             LPCWSTR pwszGuidApp,
                             LPCWSTR pwszThreadID,                    
                             LPCWSTR MsgWorkID,    
                             DWORD dwQueueLen)                
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ThreadEvents (Event, lTickCount, Application, ThreadID, MsgWorkID,QueueLen) \
             values ('%s', %d,'%s','%s','%s', %d)",
            L"OnThreadWorkPublic",
            lTickCount,
            pwszGuidApp,
            pwszThreadID,
            MsgWorkID,
            dwQueueLen);            

    return S_OK;
}            

//////////////////////////Application Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnAppActivation(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp                
            )
{
   long lTickCount = PerfCountToTickCount(perfCount);
    
    SQL_EXEC_STATEMENT(
            L"insert into AppEvents values ('%s',%d,'%s')",
            L"OnAppActivation",
            lTickCount, 
            pwszGuidApp);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnAppShutdown(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp               
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);
    
    SQL_EXEC_STATEMENT(
            L"insert into AppEvents values ('%s',%d,'%s')",
            L"OnAppShutdown",
            lTickCount, 
            pwszGuidApp);

    return S_OK;
}

//////////////////////////Transaction Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnTransactionStart(
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,                
                LPCWSTR pwszGuidTxID,            
                LPCWSTR pwszGuidTSID,
                BOOL bRoot
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into TransactionEvents (Event,lTickCount,Application, TransactionID, TSID, Root) \
             values ('%s',%d,'%s','%s','%s','%s' )",
            L"OnTransactionStart",
            lTickCount, 
            pwszGuidApp,
            pwszGuidTxID,
            pwszGuidTSID,
            YesOrNo(bRoot));

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnTransactionPrepared(
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,                
                LPCWSTR pwszGuidTxID,
                BOOL bVoteYes
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into TransactionEvents (Event,lTickCount,Application, TransactionID, VoteYes) \
             values ('%s',%d,'%s','%s','%s' )",
            L"OnTransactionPrepared",
            lTickCount, 
            pwszGuidApp,
            pwszGuidTxID,        
            YesOrNo(bVoteYes));

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnTransactionCommit(
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidTxID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into TransactionEvents (Event,lTickCount,Application, TransactionID) \
             values ('%s',%d,'%s','%s')",
            L"OnTransactionCommit",
            lTickCount, 
            pwszGuidApp,
            pwszGuidTxID);

    return S_OK;
}


STDMETHODIMP CComSpySqlAudit::OnTransactionAborted(
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidTxID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into TransactionEvents (Event,lTickCount,Application, TransactionID) \
             values ('%s',%d,'%s','%s')",
            L"OnTransactionAborted",
            lTickCount, 
            pwszGuidApp,
            pwszGuidTxID);

    return S_OK;
}


//////////////////////////Method Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnMethodCall(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,  
                LPCWSTR pwszObjectID,
                LPCWSTR pwszGuidClassID,
                LPCWSTR pwszGuidInterfaceID,
                LPCWSTR pwszMethod
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);
    
    SQL_EXEC_STATEMENT(
            L"insert into MethodEvents (Event,lTickCount, Application, ObjectID, ClassID, InterfaceID, Method) \
             values('%s',%ld, '%s','%s', '%s', '%s', '%s')",                           
            L"OnMethodCall",
            lTickCount,
            pwszGuidApp,
            pwszObjectID,
            pwszGuidClassID,
            pwszGuidInterfaceID,
            pwszMethod);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnMethodReturn(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp, 
                LPCWSTR pwszObjectID,
                LPCWSTR pwszGuidClassID,
                LPCWSTR pwszGuidInterfaceID,
                LPCWSTR pwszMethod,
                HRESULT hr,
                LPCWSTR pwszCallTime
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into MethodEvents  values ('%s', %d, '%s', '%s', '%s', '%s', '%s', %d, '%s')",
            L"OnMethodReturn",
            lTickCount,
            pwszGuidApp, 
            pwszObjectID,
            pwszGuidClassID,
            pwszGuidInterfaceID,
            pwszMethod,
            hr,
            pwszCallTime);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnMethodException(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp, 
                LPCWSTR pwszObjectID,
                LPCWSTR pwszGuidClassID,
                LPCWSTR pwszGuidInterfaceID,
                LPCWSTR pwszMethod
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into MethodEvents (Event,lTickCount, Application, ObjectID, ClassID, InterfaceID, Method) \
             values ('%s', %d, '%s', '%s', '%s', '%s', '%s')",
            L"OnMethodException",
            lTickCount,
            pwszGuidApp, 
            pwszObjectID,
            pwszGuidClassID,
            pwszGuidInterfaceID,
            pwszMethod);

    return S_OK;
}


//////////////////////////Object Construction Events///////////////////////////////////
//share table with Object Events 
STDMETHODIMP CComSpySqlAudit::OnObjectConstruct(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszClassID, 
             LPCWSTR pwszConstructString, 
             LPCWSTR pwszObjectID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event,lTickCount,Application, ClassID,ConstructString,ObjectID) \
             values ('%s', %d, '%s', '%s', '%s', '%s')",
            L"OnObjectConstruct",
            lTickCount,
            pwszGuidApp,
            pwszClassID,
            pwszConstructString,
            pwszObjectID);

    return S_OK;
}

//////////////////////////Instance Events///////////////////////////////////
//share table with Object Events 
STDMETHODIMP CComSpySqlAudit::OnObjectCreate(
                            LONGLONG perfCount,
                            LPCWSTR pwszGuidApp,
                            LPCWSTR pwszGuidActivityID,
                            LPCWSTR pwszClassID,
                            LPCWSTR pwszTSID,
                            LPCWSTR pwszContextID,
                            LPCWSTR pwszObjectID)            
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ActivityID, ClassID, TSID, ContextID,ObjectID) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s', '%s')",
            L"OnObjectCreate",
            lTickCount,
            pwszGuidApp,
            pwszGuidActivityID,
            pwszClassID,
            pwszTSID,
            pwszContextID,
            pwszObjectID); 

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjectDestroy(
                            LONGLONG perfCount,
                            LPCWSTR pwszGuidApp,
                            LPCWSTR pwszContextID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event,lTickCount,Application,ContextID) \
             values ('%s',%d, '%s', '%s')",
            L"OnObjectDestroy",
            lTickCount,
            pwszGuidApp,             
            pwszContextID);
             
    return S_OK;
}

//////////////////////////Object Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnObjectActivate(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID,
                LPCWSTR pwszObjectID                    
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID,ObjectId) \
             values ('%s', %d,  '%s', '%s','%s')",
            L"OnObjectActivate",
            lTickCount,
            pwszGuidApp,
            pwszContextID,
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjectDeactivate(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID,
                LPCWSTR pwszObjectID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID,ObjectId) \
             values ('%s', %d,  '%s', '%s','%s')",
            L"OnObjectDeactivate",
            lTickCount,
            pwszGuidApp,
            pwszContextID,
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnDisableCommit(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID                
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID) \
             values ('%s', %d,  '%s', '%s')",
            L"OnDisableCommit",
            lTickCount,
            pwszGuidApp,
            pwszContextID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnEnableCommit(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID    
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID) \
             values ('%s', %d,  '%s', '%s')",
            L"OnEnableCommit",
            lTickCount,
            pwszGuidApp,
            pwszContextID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnSetComplete(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID) \
             values ('%s', %d,  '%s', '%s')",
            L"OnSetComplete",
            lTickCount,
            pwszGuidApp,
            pwszContextID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnSetAbort(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectEvents (Event, lTickCount,Application, ContextID) \
             values ('%s', %d,  '%s', '%s')",
            L"OnSetAbort",
            lTickCount,
            pwszGuidApp,
            pwszContextID);

    return S_OK;
}


//////////////////////////Resource Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnResourceCreate(
                    LONGLONG perfCount,
                    LPCWSTR pwszGuidApp,                    
                    LPCWSTR pwszObjectID,
                    LPCWSTR pwszType,
                    LPCWSTR pwszResId,
                    BOOL bEnlisted    
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ResourceEvents (Event,lTickCount,Application, ObjectID, Type, ResourceID, Enlisted) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s')",
            L"OnResourceCreate",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,
            pwszType,
            pwszResId,    
            TrueOrFalse(bEnlisted));

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnResourceAllocate(
                LONGLONG perfCount,    
                LPCWSTR pwszGuidApp,    
                LPCWSTR pwszObjectID,    
                LPCWSTR pwszType,    
                LPCWSTR pwszResId,
                BOOL bEnlisted,
                LPCWSTR pwszNumRated,
                LPCWSTR pwszRating
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);
    
    SQL_EXEC_STATEMENT(
            L"insert into ResourceEvents (Event,lTickCount,Application, ObjectID, Type, ResourceID, Enlisted, NumRated, Rating) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
            L"OnResourceAllocate",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,
            pwszType,
            pwszResId,    
            TrueOrFalse(bEnlisted),
            pwszNumRated,
            pwszRating);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnResourceRecycle(
                LONGLONG perfCount,        
                LPCWSTR pwszGuidApp,        
                LPCWSTR pwszObjectID,        
                LPCWSTR pwszType,
                LPCWSTR pwszResId
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ResourceEvents (Event,lTickCount,Application, ObjectID, Type, ResourceID) \
             values ('%s',%d, '%s', '%s', '%s', '%s')",
            L"OnResourceRecycle",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,
            pwszType,
            pwszResId);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnResourceDestroy(
                LONGLONG perfCount,        
                LPCWSTR pwszGuidApp,        
                LPCWSTR pwszObjectID,        
                HRESULT hResult,
                LPCWSTR pwszType,
                LPCWSTR pwszResId
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ResourceEvents (Event,lTickCount,Application,ObjectID, Type, ResourceID,HResult) \
             values ('%s',%d, '%s','%s', '%s', '%s', %d)",
            L"OnResourceDestroy",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,             
            pwszType,
            pwszResId,
            hResult);

    return S_OK;
}
    

STDMETHODIMP CComSpySqlAudit::OnResourceTrack(
                    LONGLONG perfCount,
                    LPCWSTR pwszGuidApp,                    
                    LPCWSTR pwszObjectID,
                    LPCWSTR pwszType,
                    LPCWSTR pwszResId,
                    BOOL bEnlisted    
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ResourceEvents (Event,lTickCount,Application, ObjectID, Type, ResourceID, Enlisted) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s')",
            L"OnResourceTrack",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,
            pwszType,
            pwszResId,    
            TrueOrFalse(bEnlisted));

    return S_OK;
}                

//////////////////////////Security Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnAuthenticate(
                  LONGLONG perfCount,
                  LPCWSTR pwszGuidApp,
                  LPCWSTR pwszGuidActivity,
                  LPCWSTR pwszObjectID,
                  LPCWSTR pwszGuidIID,
                  LPCWSTR pwszMethod,
                  LPCWSTR pwszOriginalUser,
                  LPCWSTR pwszDirectUser, 
                  BOOL bCurrentUserInpersonatingInProc
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);        

    SQL_EXEC_STATEMENT(
            L"insert into SecurityEvents (Event,lTickCount,Application, ActivityID, ObjectID, InterfaceID, Method, OriginalUser, DirectUser,Impersonation) \
             values ('%s', %d, '%s', '%s', '%s', '%s','%s', '%s', '%s', '%s')",
            L"OnAuthenticate",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivity, 
            pwszObjectID,
            pwszGuidIID,
            pwszMethod,
            pwszOriginalUser,
            pwszDirectUser, 
            YesOrNo(bCurrentUserInpersonatingInProc));

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnAuthenticateFail(
                  LONGLONG perfCount,
                  LPCWSTR pwszGuidApp,
                  LPCWSTR pwszGuidActivity,
                  LPCWSTR pwszObjectID,
                  LPCWSTR pwszGuidIID,
                  LPCWSTR pwszMethod,
                  LPCWSTR pwszOriginalUser,
                  LPCWSTR pwszDirectUser, 
                  BOOL bCurrentUserInpersonatingInProc
            )
{
    long lTickCount = PerfCountToTickCount(perfCount);
    
    SQL_EXEC_STATEMENT(
            L"insert into SecurityEvents (Event,lTickCount,Application, ActivityID, ObjectID, InterfaceID, Method, OriginalUser, DirectUser,Impersonation) \
             values ('%s', %d, '%s', '%s', '%s', '%s','%s', '%s', '%s', '%s')",
            L"OnAuthenticateFail",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivity, 
            pwszObjectID,
            pwszGuidIID,
            pwszMethod,
            pwszOriginalUser,
            pwszDirectUser, 
            YesOrNo(bCurrentUserInpersonatingInProc));

    return S_OK;
}

//////////////////////////Identity Events///////////////////////////////////
//Share table security events
STDMETHODIMP CComSpySqlAudit::OnIISRequestInfo(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszObjectID,
             LPCWSTR pwszClientIP,
             LPCWSTR pwszServerIP,
             LPCWSTR pwszURL
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into SecurityEvents (Event,lTickCount,Application, ObjectID,ClientIP, ServerIP, URL) \
             values ('%s',%d, '%s', '%s','%s','%s','%s')",
            L"OnIISRequestInfo",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,                 
            pwszClientIP,
            pwszServerIP,
            pwszURL);

    return S_OK;
}

//////////////////////////Object Pool Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnObjPoolPutObject(
                          LONGLONG perfCount,
                          LPCWSTR pwszGuidApp, 
                          LPCWSTR pwszClassID,
                          int nReason,                          
                          DWORD dwAvailableObjs,
                          LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ClassID, Reason, AvailableObjs, ObjectID) \
             values ('%s',%d, '%s', '%s', %d, %d, '%s')",
            L"OnObjPoolPutObject",
            lTickCount, 
            pwszGuidApp,
            pwszClassID,
            nReason,
            dwAvailableObjs,    
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjPoolGetObject(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,  
             LPCWSTR pwszGuidActivityID, 
             LPCWSTR pwszClassID,                          
             DWORD dwAvailableObjs,
             LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ActivityID, ClassID, AvailableObjs, ObjectID) \
             values ('%s',%d, '%s', '%s', '%s', %d, '%s')",
            L"OnObjPoolGetObject",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivityID,
            pwszClassID,
            dwAvailableObjs,    
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjPoolRecycleToTx(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,  
             LPCWSTR pwszGuidActivityID, 
             LPCWSTR pwszClassID,      
             LPCWSTR pwszGuidTx, 
             LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ActivityID, ClassID, TransactionID, ObjectID) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s')",
            L"OnObjPoolRecycleToTx",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivityID,
            pwszClassID,
            pwszGuidTx,    
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjPoolGetFromTx(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,  
             LPCWSTR pwszGuidActivityID, 
             LPCWSTR pwszClassID,      
             LPCWSTR pwszGuidTx, 
             LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ActivityID, ClassID, TransactionID, ObjectID) \
             values ('%s',%d, '%s', '%s', '%s', '%s', '%s')",
            L"OnObjPoolGetFromTx",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivityID,
            pwszClassID,
            pwszGuidTx,    
            pwszObjectID);

    return S_OK;
}

//////////////////////////Object Pool Events 2///////////////////////////////////
//Share table with Object Pool
STDMETHODIMP CComSpySqlAudit::OnObjPoolCreateObject(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,  
             LPCWSTR pwszClassID, 
             DWORD dwAvailableObjs,
             LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ClassID,AvailableObjs, ObjectID) \
             values ('%s',%d, '%s', '%s', %d, '%s')",
            L"OnObjPoolCreateObject",
            lTickCount, 
            pwszGuidApp,        
            pwszClassID,
            dwAvailableObjs,    
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjPoolDestroyObject(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,  
             LPCWSTR pwszClassID, 
             DWORD dwAvailableObjs,
             LPCWSTR pwszObjectID)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ActivityID, ClassID, TransactionID, ObjectID) \
             values ('%s',%d, '%s', '%s', %d, '%s')",
            L"OnObjPoolDestroyObject",
            lTickCount, 
            pwszGuidApp,        
            pwszClassID,
            dwAvailableObjs,    
            pwszObjectID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnObjPoolCreateDecision(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp, 
             DWORD dwThreadsWaiting, 
             DWORD dwAvailableObjs, 
             DWORD dwCreatedObjs, 
             DWORD dwMin, 
             DWORD dwMax)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ThreadsWaiting, AvailableObjs, CreatedObjs, Min, Max) \
             values ('%s',%d, '%s', %d, %d, %d, %d, %d)",
            L"OnObjPoolCreateDecision",
            lTickCount, 
            pwszGuidApp,
            dwThreadsWaiting,             
            dwAvailableObjs,
            dwCreatedObjs,
            dwMin,
            dwMax);

    return S_OK;
}             

STDMETHODIMP CComSpySqlAudit::OnObjPoolTimeout(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp, 
             LPCWSTR pwszClassID, 
             LPCWSTR pwszGuidActivityID, 
             DWORD dwTimeout)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ClassID, ActivityID,Timeout) \
             values ('%s',%d, '%s', '%s', '%s',%d)",
            L"OnObjPoolTimeout",
            lTickCount, 
            pwszGuidApp,
            pwszClassID,
            pwszGuidActivityID,                          
            dwTimeout);

    return S_OK;
}    

STDMETHODIMP CComSpySqlAudit::OnObjPoolCreatePool(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp, 
             LPCWSTR pwszClassID,
             DWORD dwMin, 
             DWORD dwMax, 
             DWORD dwTimeout)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ObjectPoolEvents (Event,lTickCount,Application, ClassID, Min, Max, Timeout) \
             values ('%s',%d, '%s', '%s', %d, %d, %d)",
            L"OnObjPoolCreatePool",
            lTickCount, 
            pwszGuidApp,
            pwszClassID,                          
            dwMin,
            dwMax,
            dwTimeout);

    return S_OK;
}             


//////////////////////////Activity Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnActivityCreate(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidActivityID
        )
{ 
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, ActivityID) \
             values ('%s',%d, '%s', '%s')",
            L"OnActivityCreate",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivityID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnActivityDestroy(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidActivityID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, ActivityID) \
             values ('%s',%d, '%s', '%s')",
            L"OnActivityDestroy",
            lTickCount, 
            pwszGuidApp,
            pwszGuidActivityID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnActivityEnter(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidCurrentApp,
             LPCWSTR pwszGuidEnteredApp,
             LPCWSTR pwszW2KThreadID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, CurrentApp, EnteredApp,W2KThreadID) \
             values ('%s',%d, '%s', '%s','%s', '%s')",
            L"OnActivityEnter",
            lTickCount, 
            pwszGuidApp,
            pwszGuidCurrentApp,
            pwszGuidEnteredApp,
            pwszW2KThreadID);

    return S_OK;
}


STDMETHODIMP CComSpySqlAudit::OnActivityTimeout(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidCurrentApp,
             LPCWSTR pwszGuidEnteredApp,
             LPCWSTR pwszW2KThreadID,        
             DWORD dwTimeout
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, CurrentApp, EnteredApp,W2KThreadID,Timeout) \
             values ('%s',%d, '%s', '%s','%s', '%s',%d)",
            L"OnActivityTimeout",
            lTickCount, 
            pwszGuidApp,
            pwszGuidCurrentApp,
            pwszGuidEnteredApp,
            pwszW2KThreadID,
            dwTimeout);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnActivityReenter(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidCurrentApp,
             LPCWSTR pwszW2KThreadID,
             DWORD dwCallDepth
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, CurrentApp, W2KThreadID,CallDepth) \
             values ('%s',%d, '%s', '%s','%s',%d)",
            L"OnActivityReenter",
            lTickCount, 
            pwszGuidApp,
            pwszGuidCurrentApp,             
            pwszW2KThreadID,
            dwCallDepth);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnActivityLeave(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidCurrentApp,
             LPCWSTR pwszGuidLeftApp
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, CurrentApp, LeftApp) \
             values ('%s',%d, '%s', '%s','%s')",
            L"OnActivityLeave",
            lTickCount, 
            pwszGuidApp,
            pwszGuidCurrentApp,
            pwszGuidLeftApp);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnActivityLeaveSame(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidCurrentApp,
             DWORD dwCallDepth
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ActivityEvents (Event,lTickCount,Application, CurrentApp,CallDepth) \
             values ('%s',%d, '%s', '%s',%d)",
            L"OnActivityLeaveSame",
            lTickCount, 
            pwszGuidApp,
            pwszGuidCurrentApp,                 
            dwCallDepth);

    return S_OK;
}



//////////////////////////Queued Components Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnQCRecord(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszObjectID,
             LPCWSTR pwszQueueName,
             LPCWSTR pwszGuidMsgID,
             LPCWSTR pwszGuidWorkFlowID,
             HRESULT hr
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, ObjectID,QueueName, MsgID, WorkFlowID, HResult) \
             values ('%s',%d, '%s', '%s','%s','%s','%s',%d)",
            L"OnQCRecord",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,                 
            pwszQueueName,
            pwszGuidMsgID,
            pwszGuidWorkFlowID,
            hr);

    return S_OK;
}
STDMETHODIMP CComSpySqlAudit::OnQCQueueOpen(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszQueueName,
             LPCWSTR pwszQueueID,
             HRESULT hr
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, QueueName, QueueID, HResult) \
             values ('%s',%d, '%s', '%s','%s', %d)",
            L"OnQCQueueOpen",
            lTickCount, 
            pwszGuidApp,
            pwszQueueName,                 
            pwszQueueID,             
            hr);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnQCReceive(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszQueueID,
             LPCWSTR pwszGuidMsgID,
             LPCWSTR pwszGuidWorkFlowID,
             HRESULT hr
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, QueueID, MsgID, WrokFlowID, HResult) \
             values ('%s',%d, '%s', '%s','%s','%s', %d)",
            L"OnQCReceive",
            lTickCount, 
            pwszGuidApp,                             
            pwszQueueID,
            pwszGuidMsgID,
            pwszGuidWorkFlowID,
            hr);

    return S_OK;
}
        
STDMETHODIMP CComSpySqlAudit::OnQCReceiveFail(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszQueueID,
             HRESULT hr
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, QueueID, HResult) \
             values ('%s',%d, '%s', '%s', %d)",
            L"OnQCReceiveFail",
            lTickCount, 
            pwszGuidApp,                             
            pwszQueueID,             
            hr);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnQCMoveToReTryQueue(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidMsgID,
             LPCWSTR pwszGuidWorkFlowID,
             ULONG RetryIndex
        )
{

    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, MsgID, WorkFlowID, RetryIndex) \
             values ('%s',%d, '%s', '%s', '%s',%d)",
            L"OnQCMoveToReTryQueue",
            lTickCount, 
            pwszGuidApp,                             
            pwszGuidMsgID,
            pwszGuidWorkFlowID,
            RetryIndex);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnQCMoveToDeadQueue(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidMsgID,
             LPCWSTR pwszGuidWorkFlowID
        )
{

    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, MsgID, WorkFlowID) \
             values ('%s',%d, '%s', '%s', '%s')",
            L"OnQCMoveToDeadQueue",
            lTickCount, 
            pwszGuidApp,                             
            pwszGuidMsgID,
            pwszGuidWorkFlowID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnQCPlayback(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszObjectID,
             LPCWSTR pwszGuidMsgID,
             LPCWSTR pwszGuidWorkFlowID,
             HRESULT hr
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into QCEvents (Event,lTickCount,Application, ObjectID, MsgID, WorkFlowID, HResult) \
             values ('%s',%d, '%s', '%s', '%s','%s',%d)",
            L"OnQCMoveToDeadQueue",
            lTickCount, 
            pwszGuidApp,
            pwszObjectID,
            pwszGuidMsgID,
            pwszGuidWorkFlowID,
            hr);

    return S_OK;
}

//////////////////////////Exception Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnExceptionUser(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszCode,
             LPCWSTR pwszAddress,
             LPCWSTR pwszStackTrace
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into ExceptionEvents (Event,lTickCount,Application, Code,Address,StackTrace) \
             values ('%s',%d, '%s', '%s','%s','%s')",
            L"OnExceptionUser",
            lTickCount, 
            pwszGuidApp,
            pwszCode,                 
            pwszAddress,
            pwszStackTrace);
    
    return S_OK;
}

//////////////////////////CRM Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::OnCRMRecoveryStart(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp
        )
{

    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application) \
             values ('%s',%d, '%s')",
            L"OnCRMRecoveryStart",
            lTickCount, 
            pwszGuidApp);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMRecoveryDone(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application) \
             values ('%s',%d, '%s')",
            L"OnCRMRecoveryDone",
            lTickCount, 
            pwszGuidApp);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMCheckpoint(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application) \
             values ('%s',%d, '%s')",
            L"OnCRMCheckpoint",
            lTickCount, 
            pwszGuidApp);
    
    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMBegin(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID,
             LPCWSTR pwszGuidActivityID,
             LPCWSTR pwszGuidTxUOWID,
             LPCWSTR pwszProgIdCompensator,
             LPCWSTR pwszDescription                    
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID, ActivityID,TransactionUOWID,ProgIdCompensator,Description ) \
             values ('%s',%d, '%s','%s','%s','%s','%s','%s')",
            L"OnCRMBegin",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID,
            pwszGuidActivityID,
            pwszGuidTxUOWID,
            pwszProgIdCompensator,
            pwszDescription);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMPrepare(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{

    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMPrepare",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMCommit(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMCommit",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMAbort(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMAbort",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMIndoubt(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMIndoubt",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMDone(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMDone",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMRelease(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMRelease",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMAnalyze(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID,                
             LPCWSTR pwszCrmRecordType,
             DWORD dwRecordSize
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID,CRMRecordType,RecordSize) \
             values ('%s',%d, '%s','%s','%s',%d)",
            L"OnCRMAnalyze",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID,
            pwszCrmRecordType,
            dwRecordSize);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMWrite(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID,
             BOOL bVariants,
             DWORD dwRecordSize)
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID,Variants,RecordSize) \
             values ('%s',%d, '%s','%s','%s',%d)",
            L"OnCRMWrite",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID,
            YesOrNo(bVariants),
            dwRecordSize);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMForget(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMForget",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMForce(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID) \
             values ('%s',%d, '%s','%s')",
            L"OnCRMForce",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::OnCRMDeliver(
             LONGLONG perfCount,
             LPCWSTR pwszGuidApp,
             LPCWSTR pwszGuidClerkCLSID,
             BOOL bVariants,
             DWORD dwRecordSize
        )
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into CRMEvents (Event,lTickCount,Application, ClerkID, Variants, RecordSize) \
             values ('%s',%d, '%s','%s','%s',%d)",
            L"OnCRMDeliver",
            lTickCount, 
            pwszGuidApp,    
            pwszGuidClerkCLSID,
            YesOrNo(bVariants),
            dwRecordSize);

    return S_OK;
}

//////////////////////////Load Balancing Events///////////////////////////////////
STDMETHODIMP CComSpySqlAudit::TargetUp(
                             LONGLONG perfCount,
                             LPCWSTR pwszServerName, 
                             LPCWSTR pwszClsidEng)    
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into LBEvents (Event,lTickCount,Application, ServerName, ClsidEng) \
             values ('%s',%d, '%s','%s','%s')",
            L"TargetUp",
            lTickCount, 
            L"COM+ Load Balancing",    
            pwszServerName,
            pwszClsidEng);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit::TargetDown(
                             LONGLONG perfCount,
                             LPCWSTR pwszServerName, 
                             LPCWSTR pwszClsidEng)                            
{
    long lTickCount = PerfCountToTickCount(perfCount);

    SQL_EXEC_STATEMENT(
            L"insert into LBEvents (Event,lTickCount,Application, ServerName, ClsidEng) \
             values ('%s',%d, '%s','%s','%s')",
            L"TargetDown",
            lTickCount, 
            L"COM+ Load Balancing",    
            pwszServerName,
            pwszClsidEng);

    return S_OK;
}

STDMETHODIMP CComSpySqlAudit:: EngineDefined(
                             LONGLONG perfCount,
                             LPCWSTR pwszPropName, 
                             VARIANT *varPropValue, 
                             LPCWSTR pwszClsidEng)
{
    UNREFERENCED_PARAMETER(perfCount);
    UNREFERENCED_PARAMETER(pwszPropName);
    UNREFERENCED_PARAMETER(varPropValue);
    UNREFERENCED_PARAMETER(pwszClsidEng);
    
    return S_OK;
}

