//---------------------------------------------------------------------
// This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//---------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <objidl.h>
#include <txdtc.h>
#include <xolehlp.h>
#include <xa.h>

// Each TM must uniquely identify itself to its subordinates.
// We will use TM_GUID to uniquely identify this TM to MSDTC
// when connecting to MSDTC as its superior
// Note: Do not duplicate this GUID, generate a new one for each
// XA Superior application
#define TM_GUID "58b78708-cd1a-473e-9401-ba399c0e89fc"

// Use this extern C section as is.
// 
// XA protocol requires every XA RM to implement an xa_switch_t structure.
// The xa_switch_t structure contains information such as the RMs name, 
// end points, and various flags which indicates supported operations 
// by the RM.
// 
// The following C section makes a reference to the xa_switch_t 
// structure implemented by the MSDTC proxy, which will later be used to 
// communicate with MSDTC. 
//
// Every XA superior application must create this reference to be able to 
// communicate with MSDTC using the XA protocol.
extern "C"
{
    extern xa_switch_t  msqlsrvxa1;
    xa_switch_t  const *xasw  =  &msqlsrvxa1;
}

// Forward declarations.
HRESULT FormatOpenString(char* pszRMInfo, size_t sizeInBytesRMInfo);
RPC_STATUS GetNewXID(XID* pXid);
int XAOpenEntry (char* pszRMInfo, int rmID);
int XAStartEntry (XID* xid, int rmID);
int XAEndEntry (XID* xid, int rmID);
int XAPrepareEntry (XID* xid, int rmID);
int XACommitEntry (XID* xid, int rmID);
int XACloseEntry (char* pszRMInfo, int rmID);

int __cdecl wmain(int argc, WCHAR* argv[])
{
    // XA protocol requires the TM to assign an integer identifier for each of
    // its RMs. The TM passes this ID on subsequent XA operations to identify
    // the RM within the scope of a transaction. The id stays valid
    // until the TM closes the transaction.
    // We will use the following ID to identify the MSDTC subordinate RM
    int  rmID                   = 12345;

    char pszRMInfo[MAXINFOSIZE] = "";

    XID xid;
    
    // Calculate the XA open string for this superior RM
    if (FAILED(FormatOpenString(pszRMInfo, MAXINFOSIZE))) 
        goto cleanup;

    // Open the subordinate resource manager to start an XA session
    wprintf(L"Opening the resource manager to prepare it for use in an XA transaction...\n");
    if (XAOpenEntry(pszRMInfo, rmID) != XA_OK)
        goto cleanup;

    // Create a new XA Transaction ID (XID)  
    wprintf(L"Creating a new XA transaction ID ...\n");
    if (GetNewXID(&xid) != RPC_S_OK)
        goto cleanup;
    
    // Start work on the transaction branch.
    // This informs the subordinate resource manager that an application 
    // can do work using the transaction identified by xid.
    wprintf(L"Inform the RM that applications can do work using this transaction...\n");
    if (XAStartEntry(&xid, rmID) != XA_OK)
        goto cleanup;

    // Do work here.
    wprintf(L"\nDo work...\n\n");
    
    // End work on the transaction branch.
    wprintf(L"Inform the RM that applications cannot do any work using this transaction...\n");
    if (XAEndEntry(&xid, rmID) != XA_OK)
        goto cleanup;

    // The transaction manager can call more than one start and end pairs.
    // Only the work in between a start and an end calls are included in the transaction.
    
    // Ask the resource manager to prepare the transaction.
    wprintf(L"Prepare the transaction...\n");
    if (XAPrepareEntry(&xid, rmID) != XA_OK)
        goto cleanup;

    // Ask the resource manager to commit.
    wprintf(L"Commit the transaction...\n");
    if (XACommitEntry(&xid, rmID) != XA_OK)
        goto cleanup;

    wprintf(L"Transaction comitted.\n");

    // Close the connection to the resource manager for this transaction.
    wprintf(L"Closing the connection with the resource manager for this transaction.\n\n");
    if (XACloseEntry(pszRMInfo, rmID)  != XA_OK)
        goto cleanup;
    
cleanup:

    return 0;
}

// In the XA protocol, when a TM opens a connection to an RM,
// it identifies itself with a formatted open string, which 
// contains the name of the TM and an id which uniquely identifies the TM.
// The RM can then use this information for recovery purposes if it 
// needs to connect back to the TM.
// 
// This sample connects to MSDTC as an XA superior TM. The following method
// calculates an XA open string to uniquely identify this TM.
HRESULT FormatOpenString(char* pszRMInfo, size_t sizeInBytesRMInfo)
{
    HRESULT hr = E_FAIL;
    DWORD tmNameLen = MAX_COMPUTERNAME_LENGTH+1;
    char tmName[MAX_COMPUTERNAME_LENGTH+1] = "";
    char pszRMInformation[MAXINFOSIZE] = "";

    // Get the current computer name
    if (GetComputerNameA(tmName, &tmNameLen))
    {
        // Set a GUID as the recovery ID for the Resource Manager 
        _snprintf_s(pszRMInformation, MAXINFOSIZE, 
            "TM=%s, RmRecoveryGuid=%s", tmName,TM_GUID);
        wprintf(L"Configuration:\nRMINFO: %hs\n\n", pszRMInformation);
        strcpy_s(pszRMInfo, sizeInBytesRMInfo, pszRMInformation);
        hr = S_OK;
    }
    else
    {
        DWORD err = GetLastError();
        wprintf(L"ERROR: GetComputerNameA() returned 0x%x\n", err);
        hr = HRESULT_FROM_WIN32(err);
    }

    return hr;
    
}

// XA defines a structure called XID, which is used to uniquely identify
// a specific transaction branch.
//
// The GetNewXID method calculates a new unique XID.
RPC_STATUS GetNewXID(XID* pXid)
{
    UUID txid;
    RPC_STATUS rpcStatus = RPC_S_OK;
    char* pszTxid = NULL;

    if (NULL==pXid)
    {        
        wprintf(L"ERROR: Invalid argument to GetNewXID().\n");
        return RPC_S_INVALID_ARG;
    }
     
    ZeroMemory(pXid, sizeof(XID));
    
    // XID structure contains four fields
    //
    // formatID:     identifies the format used to uniquely identify 
    //               the transaction.
    //               Setting formatID to 0 indicates OSI CCR naming is used 
    //               to uniquely identify the transaction.
    //               If any other naming scheme is used the field should be 
    //               greater than 0.
    //               -1 indicates XID is null.
    // data:         data field consists of two fields named gtrid and bqual.
    //               When gtrid and bqual are together they uniquely identify 
    //               the transaction.
    //               Neither gtrid, nor bqual can be null terminated.
    // gtrid_length: specifies the length of the gtrid field in bytes.
    //               can have any value between 1 and 64.
    // bqual_length: specifies the length of the bqual field in bytes.
    //               can have any value between 1 and 64.
    
    // The following code generates a unique id and fills the XID structure.

    rpcStatus = UuidCreate(&txid);

    if (rpcStatus == RPC_S_OK)
    {
        rpcStatus = UuidToStringA(&txid, (unsigned char **) &pszTxid);
        if (rpcStatus == RPC_S_OK)
        {
            pXid->formatID = 0x1234;
            pXid->gtrid_length = (long) strlen(pszTxid);
            pXid->bqual_length = 1;
            strncpy_s(pXid->data, pszTxid, pXid->gtrid_length);
            pXid->data[pXid->gtrid_length] = '0';
            RpcStringFreeA((unsigned char **) &pszTxid);
        }
        else
        {
            wprintf(L"ERROR: UuidToStringA() returned %d\n", rpcStatus);
        }
    }
    else
    {
        wprintf(L"ERROR: UuidCreate() returned %d\n", rpcStatus);
    }

    return rpcStatus;
}


// Open the resource manager
int XAOpenEntry (char* pszRMInfo, int rmID)
{
    int returnVal = xasw->xa_open_entry(pszRMInfo, rmID, TMNOFLAGS);
    if (returnVal != XA_OK)
    {
        wprintf(L"ERROR: xa_open_entry() returned %d\n. Make sure MSDTC is cofigured to allow XA transactions.\n", returnVal);
    }
    return returnVal;
}

// Start work on the transaction branch
int XAStartEntry (XID* xid, int rmID)
{
    int returnVal= xasw->xa_start_entry(xid, rmID, TMNOFLAGS);
    if (returnVal!= XA_OK)
    {
        wprintf(L"ERROR: xa_start_entry(TMNOFLAGS) returned %d\n", returnVal);
    }
    return returnVal;
}

// End work on the transaction branch
int XAEndEntry (XID* xid, int rmID)
{
    int returnVal = xasw->xa_end_entry(xid, rmID, TMSUCCESS);
    if (returnVal != XA_OK)
    {
        wprintf(L"ERROR: xa_end_entry(TMSUCCESS) returned %d\n", returnVal);
    }
    return returnVal;
}

// Prepare to commit work done on the transaction branch
int XAPrepareEntry (XID* xid, int rmID)
{
    int returnVal= xasw->xa_prepare_entry(xid, rmID, TMNOFLAGS);
    if (returnVal != XA_OK)
    {
        wprintf(L"ERROR: xa_prepare_entry(TMNOFLAGS) returned %d\n", returnVal);
    }
    return returnVal;
}

// Commit the work done on the transaction branch
int XACommitEntry (XID* xid, int rmID)
{
    int returnVal = xasw->xa_commit_entry(xid, rmID, TMNOFLAGS);
    if (returnVal != XA_OK)
    {
        wprintf(L"ERROR: xa_commit_entry(TMNOFLAGS) returned %d\n", returnVal);
    }
    return returnVal;
}

// Close the resource manager
int XACloseEntry (char* pszRMInfo, int rmID)
{
    int returnVal = xasw->xa_close_entry(pszRMInfo, rmID, TMNOFLAGS);
    if (returnVal!= XA_OK)
    {
        wprintf(L"ERROR: xa_close_entry(TMNOFLAGS) returned %d\n", returnVal);
    }
    return returnVal;
}