// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __COMSPYSQLAUDIT_H_
#define __COMSPYSQLAUDIT_H_

#include "resource.h"       // main symbols

// Note: in string arguments to SQL functions are declared non-const
inline SQLWCHAR* SqlStringArg(__in LPCWSTR pwsz)
{
    return const_cast<SQLWCHAR*>(pwsz);
}

/////////////////////////////////////////////////////////////////////////////
// CComSpySqlAudit
class ATL_NO_VTABLE CComSpySqlAudit : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CComSpySqlAudit, &CLSID_ComSpyAudit>,
    public IComSpyAudit
{

private:
    HENV m_henv;
    HDBC m_hdbc;
    HSTMT m_hstmt;
    LONGLONG m_PerformanceFrequency;
    CComPtr<IUnknown> m_pUnkMarshaler;

    LPCWSTR GetLastSqlErrorMessage(__out_ecount(cch) WCHAR* pwsz, __in SQLSMALLINT cch);
    long PerfCountToTickCount(LONGLONG perfCount);

public:
    CComSpySqlAudit()
    {
        m_hdbc = m_henv = m_hstmt = NULL;
        m_PerformanceFrequency = 0;
        m_pUnkMarshaler = NULL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_COMSPYSQLAUDIT)
    DECLARE_NOT_AGGREGATABLE(CComSpySqlAudit)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CComSpySqlAudit)
        COM_INTERFACE_ENTRY(IComSpyAudit)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
    END_COM_MAP()

    HRESULT FinalConstruct()
    {
        
        QueryPerformanceFrequency((LARGE_INTEGER *)&m_PerformanceFrequency);
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        m_pUnkMarshaler.Release();
        RETCODE rc;
        if (m_hstmt)
        {
            SQLFreeStmt( m_hstmt, SQL_DROP );    
        }
        if (m_hdbc)
        {
           rc = SQLDisconnect( m_hdbc);
           SQLFreeConnect( m_hdbc);
           SQLFreeEnv( m_henv );
        }
    }

// IComSpyAudit
public:
    STDMETHOD(Init)(LPCWSTR pwszDSN, LPCWSTR pwszUser, LPCWSTR pwszPw);

    /////////////////////Thread Events/////////////////////////
    STDMETHOD(OnThreadStart)(
                     LONGLONG perfCount,
                     LPCWSTR pwszGuidApp,
                     LPCWSTR pwszThreadID,
                     LPCWSTR pwszW2KThreadID,
                     DWORD dwThreadCnt     
                );

    STDMETHOD(OnThreadTerminate)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,
                 LPCWSTR pwszW2KThreadID,
                 DWORD dwThreadCnt 
            );

    STDMETHOD(OnThreadBindToApartment)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR pwszAptID,    
                 DWORD dwActivityCnt
            );

    STDMETHOD(OnThreadUnBind)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR pwszAptID,    
                 DWORD dwActivityCnt
            );

    STDMETHOD(OnThreadAssignApartment)(
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidActivityID,
                LPCWSTR pwszAptID                     
            );
    STDMETHOD(OnThreadUnassignApartment)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,                
                 LPCWSTR pwszAptID                     
            );

    STDMETHOD(OnThreadWorkEnque)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR MsgWorkID,    
                 DWORD dwQueueLen
            );
    STDMETHOD(OnThreadWorkPrivate)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR MsgWorkID                        
            );
    STDMETHOD(OnThreadWorkPublic)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR MsgWorkID,    
                 DWORD dwQueueLen
            );
    STDMETHOD(OnThreadWorkRedirect)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR MsgWorkID,    
                 DWORD dwQueueLen,
                 LPCWSTR pwszThreadNum
            );
    STDMETHOD(OnThreadWorkReject)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszThreadID,                    
                 LPCWSTR MsgWorkID,    
                 DWORD dwQueueLen                
            );

    /////////////////////Application Events/////////////////////////
    STDMETHOD(OnAppActivation)(                
                LONGLONG perfCount,                
                LPCWSTR pwszGuidApp                                       
            );

    STDMETHOD(OnAppShutdown)(                
                LONGLONG perfCount,                
                LPCWSTR pwszGuidApp                           
            );

    /////////////////////Transaction Events/////////////////////////
    STDMETHOD(OnTransactionStart)(            
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,                
                LPCWSTR pwszGuidTxID,            
                LPCWSTR pwszGuidTSID,
                BOOL bRoot
            );

    STDMETHOD(OnTransactionPrepared)(        
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,                
                LPCWSTR pwszGuidTxID,
                BOOL bVoteYes
            );

    STDMETHOD(OnTransactionAborted)(        
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidTxID
            );
    STDMETHOD(OnTransactionCommit)(        
                LONGLONG perfCount,            
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidTxID
            );
    
    /////////////////////Method Events/////////////////////////
    STDMETHOD(OnMethodCall)(                
                LONGLONG perfCount,    
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszObjectID,                
                LPCWSTR pwszGuidClassID,            
                LPCWSTR pwszGuidInterfaceID,                
                LPCWSTR pwszMethod
            );

    STDMETHOD(OnMethodReturn)(                
                LONGLONG perfCount,    
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszObjectID,                
                LPCWSTR pwszGuidClassID,            
                LPCWSTR pwszGuidInterfaceID,                
                LPCWSTR pwszMethod,                
                HRESULT hr,
                LPCWSTR pwszCallTime
            );

    STDMETHOD(OnMethodException)(            
                LONGLONG perfCount,    
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszObjectID,                    
                LPCWSTR pwszGuidClassID,                
                LPCWSTR pwszGuidInterfaceID,                
                LPCWSTR pwszMethod        
            );

    /////////////////////ObjectConstruction Events/////////////////////////
    STDMETHOD(OnObjectConstruct)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszClassID, 
                 LPCWSTR pwszConstructString, 
                 LPCWSTR pwszObjectID
            );

    /////////////////////Instance Events/////////////////////////
    STDMETHOD(OnObjectCreate)(            
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszGuidActivityID,
                LPCWSTR pwszClassID,
                LPCWSTR pwszTSID,
                LPCWSTR pwszContextID,
                LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjectDestroy)(                
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID        
            );

    /////////////////////Object Events/////////////////////////
    STDMETHOD(OnObjectActivate)(            
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID,
                LPCWSTR pwszObjectID        
            );

    STDMETHOD(OnObjectDeactivate)(            
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID,
                LPCWSTR pwszObjectID                
            );

    STDMETHOD(OnDisableCommit)(                
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID                            
            );

    STDMETHOD(OnEnableCommit)(                
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID        
            );

    STDMETHOD(OnSetComplete)(                
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID
            );
    STDMETHOD(OnSetAbort)(                
                LONGLONG perfCount,
                LPCWSTR pwszGuidApp,
                LPCWSTR pwszContextID    
            );

   /////////////////////Resource Events/////////////////////////
   STDMETHOD(OnResourceCreate)(                
                    LONGLONG perfCount,
                    LPCWSTR pwszGuidApp,                    
                    LPCWSTR pwszObjectID,
                    LPCWSTR pwszType,
                    LPCWSTR pwszResId,
                    BOOL bEnlisted            
            );   

    STDMETHOD(OnResourceAllocate)(        
                LONGLONG perfCount,    
                LPCWSTR pwszGuidApp,    
                LPCWSTR pwszObjectID,    
                LPCWSTR pwszType,    
                LPCWSTR pwszResId,
                BOOL bEnlisted,
                LPCWSTR pwszNumRated,
                LPCWSTR pwszRating
            );

    STDMETHOD(OnResourceRecycle)(        
                LONGLONG perfCount,        
                LPCWSTR pwszGuidApp,        
                LPCWSTR pwszObjectID,        
                LPCWSTR pwszType,
                LPCWSTR pwszResId
            );

    STDMETHOD(OnResourceDestroy)(        
                LONGLONG perfCount,        
                LPCWSTR pwszGuidApp,        
                LPCWSTR pwszObjectID,        
                HRESULT hResult,
                LPCWSTR pwszType,
                LPCWSTR pwszResId
            );

    STDMETHOD(OnResourceTrack)(                
                    LONGLONG perfCount,
                    LPCWSTR pwszGuidApp,                    
                    LPCWSTR pwszObjectID,
                    LPCWSTR pwszType,
                    LPCWSTR pwszResId,
                    BOOL bEnlisted            
            );

    /////////////////////Security Events/////////////////////////
    STDMETHOD(OnAuthenticate)(                                                
                  LONGLONG perfCount,
                  LPCWSTR pwszGuidApp,
                  LPCWSTR pwszGuidActivity,
                  LPCWSTR pwszObjectID,
                  LPCWSTR pwszGuidIID,
                  LPCWSTR pwszMethod,
                  LPCWSTR pwszOriginalUser,
                  LPCWSTR pwszDirectUser, 
                  BOOL bCurrentUserInpersonatingInProc
            );

    STDMETHOD(OnAuthenticateFail)(                                
                  LONGLONG perfCount,
                  LPCWSTR pwszGuidApp,
                  LPCWSTR pwszGuidActivity,
                  LPCWSTR pwszObjectID,
                  LPCWSTR pwszGuidIID,
                  LPCWSTR pwszMethod,
                  LPCWSTR pwszOriginalUser,
                  LPCWSTR pwszDirectUser, 
                  BOOL bCurrentUserInpersonatingInProc
            );

    /////////////////////Identity Events///////////////////////// 
    STDMETHOD(OnIISRequestInfo)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszObjectID,
                 LPCWSTR pwszClientIP,
                 LPCWSTR pwszServerIP,
                 LPCWSTR pwszURL
            );

    /////////////////////ObjectPool Events/////////////////////////
    STDMETHOD(OnObjPoolPutObject)(
                  LONGLONG perfCount,
                  LPCWSTR pwszGuidApp, 
                  LPCWSTR pwszClassID,
                  int nReason,                          
                  DWORD dwAvailableObjs,
                  LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjPoolGetObject)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,  
                 LPCWSTR pwszGuidActivityID, 
                 LPCWSTR pwszClassID,                          
                 DWORD dwAvailableObjs,
                 LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjPoolRecycleToTx)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,  
                 LPCWSTR pwszGuidActivityID, 
                 LPCWSTR pwszClassID,      
                 LPCWSTR pwszGuidTx, 
                 LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjPoolGetFromTx)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,  
                 LPCWSTR pwszGuidActivityID, 
                 LPCWSTR pwszClassID,      
                 LPCWSTR pwszGuidTx, 
                 LPCWSTR pwszObjectID
            );

    /////////////////////ObjectPool2 Events/////////////////////////
    STDMETHOD(OnObjPoolCreateObject)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,  
                 LPCWSTR pwszClassID, 
                 DWORD dwAvailableObjs,
                 LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjPoolDestroyObject)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,  
                 LPCWSTR pwszClassID, 
                 DWORD dwAvailableObjs,
                 LPCWSTR pwszObjectID
            );

    STDMETHOD(OnObjPoolCreateDecision)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp, 
                 DWORD dwThreadsWaiting, 
                 DWORD dwAvailableObjs, 
                 DWORD dwCreatedObjs, 
                 DWORD dwMin, 
                 DWORD dwMax
            );

    STDMETHOD(OnObjPoolTimeout)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp, 
                 LPCWSTR pwszClassID, 
                 LPCWSTR pwszGuidActivityID, 
                 DWORD dwTimeout);

    STDMETHOD(OnObjPoolCreatePool)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp, 
                 LPCWSTR pwszClassID,
                 DWORD dwMin, 
                 DWORD dwMax, 
                 DWORD dwTimeout
            );
    
    /////////////////////Activity Events/////////////////////
    STDMETHOD(OnActivityCreate)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidActivityID
            );

    STDMETHOD(OnActivityDestroy)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidActivityID
            );

    STDMETHOD(OnActivityEnter)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidCurrentApp,
                 LPCWSTR pwszGuidEnteredApp,
                 LPCWSTR pwszW2KThreadID
            );

    STDMETHOD(OnActivityTimeout)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidCurrentApp,
                 LPCWSTR pwszGuidEnteredApp,
                 LPCWSTR pwszW2KThreadID,        
                 DWORD dwTimeout
            );

    STDMETHOD(OnActivityReenter)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidCurrentApp,
                 LPCWSTR pwszW2KThreadID,
                 DWORD dwCallDepth
            );

    STDMETHOD(OnActivityLeave)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidCurrentApp,
                 LPCWSTR pwszGuidLeftApp
            );

    STDMETHOD(OnActivityLeaveSame)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidCurrentApp,
                 DWORD dwCallDepth
            );



    /////////////////////Queued Components Events/////////////////////
    STDMETHOD(OnQCRecord)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszObjectID,
                 LPCWSTR pwszQueueName,
                 LPCWSTR pwszGuidMsgID,
                 LPCWSTR pwszGuidWorkFlowID,
                 HRESULT hr
            );
    STDMETHOD(OnQCQueueOpen)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszQueueName,
                 LPCWSTR pwszQueueID,
                 HRESULT hr
            );
    STDMETHOD(OnQCReceive)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszQueueID,
                 LPCWSTR pwszGuidMsgID,
                 LPCWSTR pwszGuidWorkFlowID,
                 HRESULT hr
            );
    STDMETHOD(OnQCReceiveFail)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszQueueID,
                 HRESULT hr
            );
    STDMETHOD(OnQCMoveToReTryQueue)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidMsgID,
                 LPCWSTR pwszGuidWorkFlowID,
                 ULONG RetryIndex
            );
    STDMETHOD(OnQCMoveToDeadQueue)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidMsgID,
                 LPCWSTR pwszGuidWorkFlowID
            );
    STDMETHOD(OnQCPlayback)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszObjectID,
                 LPCWSTR pwszGuidMsgID,
                 LPCWSTR pwszGuidWorkFlowID,
                 HRESULT hr
            );


    /////////////////////Exception Events/////////////////////
    STDMETHOD(OnExceptionUser)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszCode,
                 LPCWSTR pwszAddress,
                 LPCWSTR pwszStackTrace
            );


    /////////////////////CRM Events/////////////////////
    STDMETHOD(OnCRMRecoveryStart)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp
            );

    STDMETHOD(OnCRMRecoveryDone)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp
            );

    STDMETHOD(OnCRMCheckpoint)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp
            );

    STDMETHOD(OnCRMBegin)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID,
                 LPCWSTR pwszGuidActivityID,
                 LPCWSTR pwszGuidTxUOWID,
                 LPCWSTR pwszProgIdCompensator,
                 LPCWSTR pwszDescription                    
            );                

    STDMETHOD(OnCRMPrepare)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMCommit)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMAbort)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMIndoubt)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMDone)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMRelease)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMAnalyze)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID,                
                 LPCWSTR pwszCrmRecordType,
                 DWORD dwRecordSize
            );

    STDMETHOD(OnCRMWrite)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID,
                 BOOL bVariants,
                 DWORD dwRecordSize);

    STDMETHOD(OnCRMForget)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMForce)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID
            );

    STDMETHOD(OnCRMDeliver)(
                 LONGLONG perfCount,
                 LPCWSTR pwszGuidApp,
                 LPCWSTR pwszGuidClerkCLSID,
                 BOOL bVariants,
                 DWORD dwRecordSize
            );

    /////////////////////Load Balancing Events/////////////////////
    STDMETHOD(TargetUp)(
             LONGLONG perfCount,
             LPCWSTR pwszServerName, 
             LPCWSTR pwszClsidEng
        );

    STDMETHOD(TargetDown)(
             LONGLONG perfCount,
             LPCWSTR pwszServerName, 
             LPCWSTR pwszClsidEng
        );

    STDMETHOD(EngineDefined)(
             LONGLONG perfCount,
             LPCWSTR pwszPropName, 
             VARIANT *varPropValue, 
             LPCWSTR pwszClsidEng
        );
};

#endif //__COMSPYSQLAUDIT_H_
