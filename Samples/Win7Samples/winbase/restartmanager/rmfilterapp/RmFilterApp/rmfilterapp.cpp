/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

Module Name:

    rmfilterapp.cpp
 
Abstract:

	This module contains the skeleton code for a Windows native 
	API console based Restart Manager conductor process sample.

Notes:
    This example requires Windows 7 or Windows Server 2008 R2.

	The following code snippet shows an example of a primary installer starting and using a Restart Manager session. 	
	
	This example shows how a primary installer can use Restart Manager to shutdown and restart a process. 
	The example assumes that Calculator is already running before starting the Restart Manager session.
--*/

#include <windows.h>
#include <restartmanager.h>

int _cdecl wmain()
{
    DWORD dwErrCode         = ERROR_SUCCESS;
    DWORD dwSessionHandle   = 0xFFFFFFFF; // Invalid handle value

    //
    // CCH_RM_SESSION_KEY: Character count of the  
    // Text-Encoded session key,defined in RestartManager.h
    //
    WCHAR sessKey[CCH_RM_SESSION_KEY+1];
    
    // Number of calc files to be registered.
    DWORD dwFiles = 2;   

    //
    // NOTE:We register two calc executable files. 
    // The second one is for the redirection of 32 bit calc on 
    // 64 bit machines. Even if you are using a 32 bit machine,  
    // you don't need to comment out the second line. 
    //
    LPCWSTR rgsFiles[]      =	{L"C:\\Windows\\System32\\calc.exe",
								 L"C:\\Windows\\SysWow64\\calc.exe"};

    UINT nRetry             = 0; 
    UINT nAffectedApps      = 0;
    UINT nProcInfoNeeded    = 0;
    RM_REBOOT_REASON dwRebootReasons    = RmRebootReasonNone;
    RM_PROCESS_INFO *rgAffectedApps     = NULL;
    
    //
    // Start a Restart Manager Session
    //
    dwErrCode = RmStartSession(&dwSessionHandle, 0, sessKey);
    if (ERROR_SUCCESS != dwErrCode)
    {
        goto RM_CLEANUP;
    }

    //
    // Register items with Restart Manager
    //
    // NOTE: we only register two calc executable files 
    //in this sample. You can register files, processes 
    // (in the form of process ID), and services (in the   
    // form of service short names) with Restart Manager.
    //
    dwErrCode = RmRegisterResources(dwSessionHandle,
                                    dwFiles, 
                                    rgsFiles,       // Files
                                    0,  
                                    NULL,           // Processes
                                    0, 
                                    NULL);          // Services 
                                    
    if (ERROR_SUCCESS != dwErrCode)
    {
        goto RM_CLEANUP;
    }

    //
    // Obtain the list of affected applications/services.
    //
    // NOTE: Restart Manager returns the results into the  
    // buffer allocated by the caller. The first call to 
    // RmGetList() will return the size of the buffer  
    // (i.e. nProcInfoNeeded) the caller needs to allocate. 
    // The caller then needs to allocate the buffer  
    // (i.e. rgAffectedApps) and make another RmGetList() 
    // call to ask Restart Manager to write the results 
    // into the buffer. However, since Restart Manager 
    // refreshes the list every time RmGetList()is called, 
    // it is possible that the size returned by the first 
    // RmGetList()call is not sufficient to hold the results  
    // discovered by the second RmGetList() call. Therefore, 
    // it is recommended that the caller follows the 
    // following practice to handle this race condition:
    //   
    //    Use a loop to call RmGetList() in case the buffer 
    //    allocated according to the size returned in previous 
    //    call is not enough. 	 
    // 
    // In this example, we use a do-while loop trying to make 
    // 3 RmGetList() calls (including the first attempt to get 
    // buffer size) and if we still cannot succeed, we give up. 
    //
    do
    {
        dwErrCode = RmGetList(dwSessionHandle,
                              &nProcInfoNeeded,
                              &nAffectedApps, 
                              rgAffectedApps, 
                              (LPDWORD) &dwRebootReasons);
        if (ERROR_SUCCESS == dwErrCode)
        {
            //
            // RmGetList() succeeded
            //
            break;
        }

        if (ERROR_MORE_DATA != dwErrCode)
        {
            //
            // RmGetList() failed, with errors 
            // other than ERROR_MORE_DATA
            //
            goto RM_CLEANUP;
        }

        //
        // RmGetList() is asking for more data
        //
        nAffectedApps = nProcInfoNeeded;
        
        if (NULL != rgAffectedApps)
        {
            delete []rgAffectedApps;
            rgAffectedApps = NULL;
        }

        rgAffectedApps = new RM_PROCESS_INFO[nAffectedApps];
    } while ((ERROR_MORE_DATA == dwErrCode) && (nRetry ++ < 3));

    if (ERROR_SUCCESS != dwErrCode)
    {
        goto RM_CLEANUP;
    }

    if (RmRebootReasonNone != dwRebootReasons)
    {
        //
        // Restart Manager cannot mitigate a reboot. 
        // We goes to the clean up. The caller may want   
        // to add additional code to handle this scenario.
        //
        goto RM_CLEANUP;
    }
    
    //
    // Now rgAffectedApps contains the affected applications 
    // and services. The number of applications and services
    // returned is nAffectedApps. The result of RmGetList can 
    // be interpreted by the user to determine subsequent  
    // action (e.g. ask user's permission to shutdown).
    //
    // CALLER CODE GOES HERE...
     
    //
    // Shut down all running instances of affected 
    // applications and services.
    //
    dwErrCode = RmShutdown(dwSessionHandle, 0, NULL);
    if (ERROR_SUCCESS != dwErrCode)
    {
        goto RM_CLEANUP;
    }

    //
    // An installer can now replace or update
    // the calc executable file.
    //
    // CALLER CODE GOES HERE...


    //
    // Restart applications and services, after the 
    // files have been replaced or updated.
    //
    dwErrCode = RmRestart(dwSessionHandle, 0, NULL);
    if (ERROR_SUCCESS != dwErrCode)
    {
        goto RM_CLEANUP;
    }

  RM_CLEANUP:
    
    if (NULL != rgAffectedApps)
    {
        delete [] rgAffectedApps;
        rgAffectedApps = NULL;
    }

    if (0xFFFFFFFF != dwSessionHandle)
    {
        //
        // Clean up the Restart Manager session.
        //
        RmEndSession(dwSessionHandle);
        dwSessionHandle = 0xFFFFFFFF;
    }

    return 0;
}
