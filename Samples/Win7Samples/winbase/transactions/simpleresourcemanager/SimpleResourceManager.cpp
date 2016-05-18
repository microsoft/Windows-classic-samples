//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <string>
#include <iostream>

#include "txdtc.h"
#include "txcoord.h"
#include "xolehlp.h"

// This is the GUID of the Resource Manager that uniquely identifies it
// It should not be reused. Generate a new one for each Resource Manager
// {08C6599C-C781-42a2-B1E6-C7E4C95F6938}
static const GUID ResourceManagerGUID = { 0x8c6599c, 0xc781, 0x42a2, { 0xb1, 0xe6, 0xc7, 0xe4, 0xc9, 0x5f, 0x69, 0x38 } };

class DTCResourceManager : public IResourceManagerSink, ITransactionResourceAsync
{
public:
    DTCResourceManager();
    bool Init();
    ~DTCResourceManager();
    bool OpenConnection(byte* transactionToken, ULONG tokenSize);
    bool DoWork();
    bool CloseConnection();

    virtual HRESULT STDMETHODCALLTYPE PrepareRequest(
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfRM,
            /* [in] */ BOOL fWantMoniker,
            /* [in] */ BOOL fSinglePhase);
        
    virtual HRESULT STDMETHODCALLTYPE CommitRequest(
            /* [in] */ DWORD grfRM,
            /* [unique][in] */ XACTUOW *pNewUOW);
        
    virtual HRESULT STDMETHODCALLTYPE AbortRequest(
            /* [unique][in] */ BOID *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [unique][in] */ XACTUOW *pNewUOW);
        
    virtual HRESULT STDMETHODCALLTYPE TMDown(void);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ void **ppvObject);
            
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
            
    virtual ULONG STDMETHODCALLTYPE Release( void);

private:

    HRESULT RegisterWithMSDTC();
    bool DoRecovery();

    // pointer used to enlist in transactions
    IResourceManager* _pRMManager;
    // pointer used to reply to enlistment requests
    ITransactionEnlistmentAsync* _pEnlist;

    // RM name
    const std::string _rmName;

    // used for IUnknown lifetime management
    ULONG _cRef;
};

DTCResourceManager::DTCResourceManager() : _rmName("NativeRMSample")
{
    _pRMManager = NULL;
}

DTCResourceManager::~DTCResourceManager()
{}

// Init method needs to be called to initialize the resource manager before any work is done
bool DTCResourceManager::Init()
{
    HRESULT hr = S_OK;
    hr = RegisterWithMSDTC();
    if( FAILED(hr) )
    {
        std::cout << "The resource manager failed to Init. Error # " << std::hex << hr << std::endl;
        return false;
    }

    // recover any transactions that are not completed
    bool returnValue = DoRecovery();
    
    return returnValue;
}

HRESULT DTCResourceManager::RegisterWithMSDTC()
{
    IResourceManagerFactory* pRMFactory = NULL;
    HRESULT hr = S_OK ;
    
    hr = DtcGetTransactionManager(
    NULL,              // [in] char * pszHost,
    NULL,              // [in] char * pszTmName,
    IID_IResourceManagerFactory,    // [in] REFIID riid,
    0,                // [in] DWORD dwReserved1,
    0,                // [in] WORD wcbVarLenReserved2,
    (void *)NULL,          // [in] void * pvVarDataReserved2,
    (void **)&pRMFactory // [out] void ** ppv
    );
    if (FAILED (hr))
    {        
        std::cout << "DtcGetTransactionManager failed: Error # " << std::hex << hr << ".";
        std::cout << " Make sure MSDTC service is running." << std::endl;
        goto cleanup;
    }

    std::cout << "The resource manager is registering with MSDTC" << std::endl;
    hr = pRMFactory->Create(const_cast<GUID*>(&ResourceManagerGUID),
        const_cast<char*>(_rmName.c_str()), this, &_pRMManager);
    if (FAILED (hr)) 
    {        
        std::cout << "IResourceManagerFactory->Create failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }


cleanup:
    if(pRMFactory)
    {
        pRMFactory->Release();
        pRMFactory = NULL;
    }
    if( FAILED(hr) )
    {
        std::cout << "Failed to register the resource manager with MSDTC" << std::endl;
    }
    return hr;
}

bool DTCResourceManager::DoRecovery()
{
    std::cout << "The resource manager is doing recovery for any in doubt transaction found in the log" << std::endl;
    // Typical things that happen here:
    // - read log file
    // - for each tx in the log file, call IResourceManager::Reenlist
    // - if all Reenlists succeeded, call IResourceManager::ReenlistmentComplete()
    return true;
}

// OpenConnection is called by clients to initiate work with the resource manager under a specified transaction
// The transaction is passed in as a transaction token, to show how transactions objects can be serialized across
// processes
bool DTCResourceManager::OpenConnection(byte* transactionToken, ULONG tokenSize)
{
    std::cout << "The resource manager received an OpenConnection request. Enlisting in the transaction..." << std::endl;
    ITransactionReceiverFactory* pTxReceiverFactory = NULL;
    HRESULT hr = DtcGetTransactionManager(
    NULL,              // [in] char * pszHost,
    NULL,              // [in] char * pszTmName,
    IID_ITransactionReceiverFactory,    // [in] REFIID riid,
    0,                // [in] DWORD dwReserved1,
    0,                // [in] WORD wcbVarLenReserved2,
    (void *)NULL,          // [in] void * pvVarDataReserved2,
    (void **)&pTxReceiverFactory // [out] void ** ppv
    );
    if (FAILED (hr))
    {        
        std::cout << "DtcGetTransactionManager for ITransactionReceiverFactory failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    ITransactionReceiver* pTxReceiver = NULL;
    hr = pTxReceiverFactory->Create(&pTxReceiver);
    if (FAILED(hr))
    {        
        std::cout << "pTxReceiverFactory->Create failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    ITransaction* pTx = NULL;
    hr = pTxReceiver->UnmarshalPropagationToken(tokenSize, transactionToken, &pTx);
    if (FAILED(hr))
    {        
        std::cout << "pTxReceiver->UnmarshalPropagationToken failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    XACTUOW uow;
    LONG isoLevel;

    hr = _pRMManager->Enlist(pTx, this, &uow, &isoLevel, &_pEnlist);
    if (FAILED(hr))
    {        
        std::cout << "pRMManager->Enlist failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

cleanup:
    if( pTx )
    {
        pTx->Release();
        pTx = NULL;
    }
    if( pTxReceiver )
    {
        pTxReceiver->Release();
        pTxReceiver = NULL;
    }

    if( pTxReceiverFactory )
    {
        pTxReceiverFactory->Release();
        pTxReceiverFactory = NULL;
    }

    if( FAILED(hr) )
    {
        std::cout << "OpenConnection failed" << std::endl;
        return false;
    }
    return true;
}

// DoWork simulates work requested by the client to be done under the transaction specified in the OpenConnection
// This is similar to UPDATE SQL statements sent to your regular database
bool DTCResourceManager::DoWork()
{
    std::cout << "The resource manager is doing work as part of the transaction" << std::endl;
    // the work is usually done in a temporary area for this particular transaction
    return true;
}

// The client indicates that it has finished work with the resource manager
bool DTCResourceManager::CloseConnection()
{
    std::cout << "The resource manager is asked to close the connection with the client" << std::endl;
    return true;
}

// Event received from MSDTC during Phase 1 of the transaction
HRESULT DTCResourceManager::PrepareRequest(BOOL fRetaining, DWORD grfRM, BOOL fWantMoniker, BOOL fSinglePhase)
{
    std::cout << "The resource manager received PrepareRequest. Preparing..." << std::endl;
    // The work done in DoWork is moved from the temporary area to a "prepared" area
    // Once the work is moved in the "prepared" area and PrepareRequestDone, the resource manager must be able
    // to either Commit or Abort this "work" at any time when asked by the Transaction Manager, even surviving crashes
    // To survive crashes, the resource manager will save the "prepare info" to the log file before replying PrepareRequestDone
    // The prepare info is obtained using the following steps:
    // 1. Call QueryInterface on the ITransactionEnlistmentAsync interface on the enlistment object with an riid of IID_IPrepareInfo2
    // 2. Using IPrepareInfo2 methods, GetPrepareInfoSize and GetPrepareInfo, obtain the byte array containing the prepare info
    // At start up, after a crash, the resource manager will read the log file and for each prepare info found, it will
    // call Reenlist
    _pEnlist->PrepareRequestDone(S_OK, NULL, NULL);
    return S_OK;
}

// Event received from MSDTC during Phase 2 to indicate that the transaction needs to be commited
HRESULT DTCResourceManager::CommitRequest(DWORD grfRM, XACTUOW *pNewUOW)
{
    std::cout << "The resource manager received CommitRequest. Commiting..." << std::endl;
    // move the "prepared" work from the prepared area to the real data
    _pEnlist->CommitRequestDone(S_OK);
    std::cout << "The resource manager has completed the transaction." << std::endl;
    return S_OK;
}

// Event received from MSDTC during Phase 2 to indicate that the transaction needs to be aborted
HRESULT DTCResourceManager::AbortRequest(BOID *pboidReason, BOOL fRetaining, XACTUOW *pNewUOW)
{
    std::cout << "The resource manager received AbortRequest. Aborting..." << std::endl;
    // remove any temporary or prepared work as part of this transaction
    _pEnlist->AbortRequestDone(S_OK);
    return S_OK;
}

// If MSDTC goes down, or the current process looses the connection with MSDTC, then this event is raised
HRESULT DTCResourceManager::TMDown(void)
{
    assert(false); // not yet implemented
    // here we put recovery code that will "restart the RM": reconnect with MSDTC, re-read the log and 
    // recover transactions found in it, and start listening for clients
    return S_OK;
}

HRESULT DTCResourceManager::QueryInterface(REFIID i_iid, void** o_ppv)
{
    HRESULT hr = S_OK;

    if( NULL == o_ppv )
    {
        return E_INVALIDARG;
    }

    if( IID_IUnknown  == i_iid )
    {
        *o_ppv = (IUnknown*)((IResourceManagerSink*)this);
    }
    else if( IID_IResourceManagerSink == i_iid )
    {
        *o_ppv = ( IResourceManagerSink* ) this;
    }
    else
    {
        *o_ppv = NULL;
        hr = E_NOINTERFACE;
    }

    if( SUCCEEDED(hr) )
    {
        ((IUnknown *) *o_ppv)->AddRef();
    }

    return hr;
}

ULONG DTCResourceManager::AddRef()
{
    return ( InterlockedIncrement( (LONG*) &this->_cRef ) );
}

ULONG DTCResourceManager::Release()
{
    ULONG localCount = InterlockedDecrement( (LONG*) &this->_cRef );
    
    if ( 0 == localCount )
    {
        delete this;
    }

    return localCount;
}

// This function will create an MSDTC transaction 
bool CreateDTCTransaction(ITransaction** ppTransaction)
{
    if(!ppTransaction)
    {
        std::cout << "NULL argument passed to CreateDTCTransaction" << std::endl;
        return false;
    }

    ITransactionDispenser* pTransactionDispenser = NULL;
    ITransaction* pTransaction = NULL;
    HRESULT hr = S_OK ;

    // Obtain a transaction dispenser interface pointer from the DTC.
    hr = DtcGetTransactionManager(
    NULL,              // [in] char * pszHost,
    NULL,              // [in] char * pszTmName,
    IID_ITransactionDispenser,    // [in] REFIID riid,
    0,                // [in] DWORD dwReserved1,
    0,                // [in] WORD wcbVarLenReserved2,
    (void *)NULL,          // [in] void * pvVarDataReserved2,
    (void **)&pTransactionDispenser // [out] void ** ppv
    );
    if (FAILED (hr))
    {        
        std::cout << "DtcGetTransactionManager failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    std::cout << "The client creates a transaction" << std::endl;
    // Initiate a DTC transaction.
    hr = pTransactionDispenser->BeginTransaction(
    NULL,                        // [in] IUnknown * punkOuter,
    ISOLATIONLEVEL_SERIALIZABLE, // [in] ISOLEVEL isoLevel,
    ISOFLAG_RETAIN_NONE,         // [in] ULONG isoFlags,
    NULL,                        // [in] ITransactionOptions * pOptions,
    &pTransaction                // [out] ITransaction * ppTransaction
    );
    if (FAILED (hr))
    {        
        std::cout << "BeginTransaction failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

cleanup:
    if(pTransactionDispenser)
    {
        pTransactionDispenser->Release();
        pTransactionDispenser = NULL;
    }
    if( FAILED(hr) )
    {
        std::cout << "CreateDTCTransaction failed. No transaction was created." << std::endl;
        return false;
    }
    else
    {
        *ppTransaction = pTransaction;
        return true;
    }
}

// This function will "marshal" a transaction object into a byte array called token
// The token can be passed across processes, machines to resource managers so that they can enlist 
//    in the transaction
bool MarshalDTCTransaction(ITransaction* pTransaction, byte** ppTxToken, ULONG* pTokenSize)
{
    if( (!pTransaction) || (!ppTxToken) || (!pTokenSize) )
    {
        std::cout << "NULL argument passed to MarshalDTCTransaction" << std::endl;
        return false;
    }

    HRESULT hr = S_OK;

    // Obtain a transaction dispenser interface pointer from the DTC.
    ITransactionDispenser* pTransactionDispenser = NULL;
    hr = DtcGetTransactionManager(
    NULL,              // [in] char * pszHost,
    NULL,              // [in] char * pszTmName,
    IID_ITransactionDispenser,    // [in] REFIID riid,
    0,                // [in] DWORD dwReserved1,
    0,                // [in] WORD wcbVarLenReserved2,
    (void *)NULL,          // [in] void * pvVarDataReserved2,
    (void **)&pTransactionDispenser // [out] void ** ppv
    );
    if (FAILED (hr))
    {
        std::cout << "DtcGetTransactionManager failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    ITransactionTransmitterFactory* pTxTransmitterFactory = NULL;
    hr = pTransactionDispenser->QueryInterface(IID_ITransactionTransmitterFactory, (void**)&pTxTransmitterFactory);
    if (FAILED(hr))
    {
        std::cout << "pTransactionDispenser->QueryInterface(IID_ITransactionTransmitterFactory , pTxTransmitterFactory) failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    ITransactionTransmitter* pTxTransmitter = NULL;
    hr = pTxTransmitterFactory->Create(&pTxTransmitter);
    if (FAILED(hr))
    {        
        std::cout << "pTxTransmitterFactory->Create failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    hr = pTxTransmitter->Set(pTransaction);
    if (FAILED(hr)) 
    {
        std::cout << "pTxTransmitter->Set failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    ULONG tokenSize = 0;
    hr = pTxTransmitter->GetPropagationTokenSize(&tokenSize);
    if (FAILED(hr)) 
    {
        std::cout << "pTxTransmitter->GetPropagationTokenSize failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    byte* token = NULL;
    try
    {
        token = new byte[tokenSize];
    }
    catch( std::bad_alloc )
    {
        token = NULL;
    }
    if( NULL == token )
    {
        std::cout << "Failed to allocate memory." << std::endl;
        hr = E_FAIL;
        goto cleanup;
    }
    ULONG tokenSizeUsed = 0;
    hr = pTxTransmitter->MarshalPropagationToken(tokenSize, token, &tokenSizeUsed);
    if (FAILED(hr)) 
    {        
        std::cout << "pTxTransmitter->MarshalPropagationToken failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }

    hr = pTxTransmitter->Reset();
    if (FAILED(hr)) 
    {        
        std::cout << "pTxTransmitter->Reset failed: Error # " << std::hex << hr << std::endl;
        goto cleanup;
    }
cleanup:
    if( pTxTransmitter )
    {
        pTxTransmitter->Release();
        pTxTransmitter = NULL;
    }
    if( pTxTransmitterFactory )
    {
        pTxTransmitterFactory->Release();
        pTxTransmitterFactory = NULL;
    }
    if( pTransactionDispenser )
    {
        pTransactionDispenser->Release();
        pTransactionDispenser = NULL;
    }
    if( FAILED(hr) )
    {
        std::cout << "MarshalDTCTransaction failed" << std::endl;
        delete[] token;
        token = NULL;
        return false;
    }
    
    *ppTxToken = token;
    *pTokenSize = tokenSize;
    return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
    // Starting up the resource manager
    // Note: In most of the cases, a "durable" resource manager lives in its own process
    // Here the resource manager is in the same process with the client for readability
    DTCResourceManager* resourceManager = NULL;
    try
    {
        resourceManager = new DTCResourceManager();    
    }
    catch( std::bad_alloc )
    {
        resourceManager = NULL;
    }
    if( NULL == resourceManager )
    {
        std::cout << "Failed to allocate memory. The program will exit." << std::endl;
        exit(1);
    }

    if(!resourceManager->Init())
    {
        std::cout << "The resource manager failed to start. The program will exit." << std::endl;
        exit(1);
    }


    std::cout << "The client starts..." << std::endl;

    // client starts, creates a transaction, does some work on the resource manager and later calls commit

    ITransaction* pTransaction = NULL;
    if( !CreateDTCTransaction(&pTransaction) )
    {
        std::cout << "Failed to create transaction. The program will exit." << std::endl;
        exit(1); // Replace with specific error handling
    }

    byte* txToken = NULL;
    ULONG txTokenSize = 0;
    if( !MarshalDTCTransaction(pTransaction, &txToken, &txTokenSize) )
    {
        std::cout << "Failed to marshal the transaction. The program will exit." << std::endl;
        exit(1); // Replace with specific error handling
    }

    std::cout << "The client asks the resource manager to do work as part of the transaction" << std::endl;
    if( !resourceManager->OpenConnection(txToken, txTokenSize))
    {
        std::cout << "The client failed to open the connection with the resource manager. The program will exit." << std::endl;
        exit(1);
    }
    resourceManager->DoWork();
    resourceManager->CloseConnection();

    std::cout << "The client commits the transaction" << std::endl;
    // Commit the transaction.
    HRESULT hr = S_OK;
    hr = pTransaction->Commit(
    FALSE,                // [in] BOOL fRetaining,
    XACTTC_SYNC_PHASEONE, // [in] DWORD grfTC,
    0                     // [in] DWORD grfRM
    );
    if (FAILED(hr))
    {        
        std::cout << "pTransaction->Commit() failed: Error # " << std::hex << hr << std::endl;
        exit(1); // Replace with specific error handling.
    }
     
    // Release the transaction object.
    pTransaction->Release();
    pTransaction = NULL;

    delete[] txToken;
    txToken = NULL;


    std::cout << std::endl << "The client exits" << std::endl;
    
    // Since the resource manager is sharing its lifetime with the client process in this sample, 
    // to avoid "failed to notify" transactions, we will ask for the user to keep this process alive
    // until the resource manager will complete the transaction
    std::cout << std::endl << "Press ENTER after you see the Resource Manager completing the transaction." << std::endl;
    std::cin.get();

    delete resourceManager;
    resourceManager = NULL;

    return 0;
}

