// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

//
// Purpose: 
//   This console application sample demonstrates how to use NT
//   completion ports in MSMQ to asynchronously receive messages in an
//   efficient manner. This mechanism is scalable in the number of
//   queues and messages by adding more processors and threads.
//   Similarly, generic completion port handlers can be provided to
//   handle other NT resources as well as queues.
//  
//   Both the MSMQ COM components and the MQ API are used in this program.
//   Note that the COM components are used for creating a queue, open
//   the queue, and sending messages. Conversely, the MQ API is used
//   to implement completion port-based asynchronous receive.
//
// Requirements:
//   VC5 is required.
//   MSMQ must be installed. Specifically mqoa.dll must be registered
//   and on the path.
//   Project settings:
//     - The include path must include the location of mq.h: 
//          e.g. ...\msmq\sdk\include
//     - The link library path must include the location of mqrt.lib:
//          e.g ...\msmq\sdk\lib
//
// Overview:
//   The following steps comprise this sample:
//     - A global MSMQQueueInfo object is used to reference the sample's
//       single queue.
//     - Initialize OLE.
//     - Create a new completion port. 
//     - Create a bunch of threads with a generic CompletionPortThread start 
//       routine parameterized with the completion port handle from the previous step.
//     - Open the queue and associate its handle with the completion port.
//       - Note that the queue is deleted and recreated if already exists;
//         otherwise, a new queue is created.
//     - Enable a bunch of asynchronous message receive requests on the queue.
//       Since the queue is associated with the completion port, each of these
//       requests results in the CompletionPortThread handler being notified
//       asynchronously by NT when an async receive message operation "completes."
//       - Note that the NT scheduler will select the "best" available completion
//         port thread that is synchronously waiting for a completion notification.
//     - Finally, to test the completion port handlers, a bunch of messages is sent
//       to the queue and the program hibernates.
//     - To exit, just kill the console application window.
//
// Warning:  *** Only limited error checking and handling are provided. ***
//
#include <windows.h>
#include <stdio.h>
#import <mqoa.tlb> no_namespace
#include <mq.h>

//
// Global queue object.
//
IMSMQQueueInfoPtr g_qinfo;

//
// Structure containing both an OVERLAPPED structure and other MSMQ
// application-specific data.
//
struct RECEIVE_CONTEXT
{
    OVERLAPPED ov;
    HANDLE hQueue;
    MQMSGPROPS *pmsgprops;
};


//
// AllocMsgProps
//  Parameters:
//    prc      IN  receive context containing an OVERLAPPED structure and
//                 additional MSMQ-specific information: queue handle and
//                 message properties. 
// 
//  Purpose:
//    Allocates a property identifier array for the PROPID_M_APPSPECIFIC property.
//    This is just because we're lazy and don't want to allocate/deallocate
//    buffers for the message body, label, etc. The PROPID_M_APPSPECIFIC
//    property allows us to "uniquely" stamp each messsages for identification
//    purposes (in this case, with an ordinal number). 
//
void AllocMsgProps(RECEIVE_CONTEXT *prc)
{
    int cProp = 1;
    prc->pmsgprops->aPropID = new MSGPROPID[cProp];
    prc->pmsgprops->aPropVar = new MQPROPVARIANT[cProp];
    prc->pmsgprops->aStatus = new HRESULT[cProp];

    int iProp = 0;
    prc->pmsgprops->aPropID[iProp] = PROPID_M_APPSPECIFIC;
    prc->pmsgprops->aPropVar[iProp].vt = VT_UI4;
    prc->pmsgprops->cProp = cProp;
}

//
// HandleReceivedMessage: 
//  Parameters:
//    prc      IN  receive context containing an OVERLAPPED structure and
//                 additional MSMQ-specific information: queue handle and
//                 message properties. 
// 
//  Purpose:
//    Inspects the value of HRESULT returned by the MSMQ device driver 
//    in the OVERLAPPED structure. Provides more detailed last-error
//    information. Then displays the message -- in this case just the
//    message's PROPID_M_APPSPECIFIC property.
//
void HandleReceivedMessage(RECEIVE_CONTEXT* prc)
{
    //
    // Get receive message final status.
    //
    HRESULT rc = MQGetOverlappedResult(&prc->ov);

    //
    // Handle the status and message.
    //
    if (SUCCEEDED(rc))
    {
      //
      // Get the received message. PROPID_M_ APPSPECIFIC is the single property
      // that we set and now retrieve.
      //
      long lAppSpecific = prc->pmsgprops->aPropVar[0].lVal;
      printf("Thread id: %x Message received with the app-specific data: %d\n", 
        GetCurrentThreadId(), 
        lAppSpecific);
    }
}

//
// HandleErrors
// Parameters:
//    _com_error
//
// Purpose: 
//   Displays an error and aborts further execution.
//
void HandleErrors(_com_error comerr)
{
    HRESULT hr = comerr.Error();
    printf("An error occurred. Error code: %x\nExiting...", hr);
    exit(hr);
};

//
// EnableAsyncReceive:
//  Parameters:
//    prc      IN  receive context containing an OVERLAPPED structure and
//                 additional MSMQ-specific information: queue handle and
//                 message properties. 
// 
//  Purpose:
//    Makes an MSMQ asynchronous receive request, specifying
//    an OVERLAPPED structure with the appropriate queue, which
//    has been associated already with a completion port.
//
HRESULT EnableAsyncReceive(RECEIVE_CONTEXT* prc)
{
    //
    // Re-enable asynchronous receiving.
    //
    return MQReceiveMessage(
            prc->hQueue,
            INFINITE,               // Time-out interval
            MQ_ACTION_RECEIVE,
            prc->pmsgprops,
            &prc->ov,               // OVERLAPPED
            NULL,                   // No callback function
            NULL,                   // No cursor
            NULL);                  // No transaction pointer
}

//
// CompletionPortThread:
//  Parameters:
//    lParam     IN  completion port handle.
//
//  Purpose:
//    Start routine for each worker thread.
//    Waits for the completion port to complete. When notification arrives, 
//    handles the received message and re-enables MSMQ asynchronous receiving.
//
DWORD WINAPI CompletionPortThread(LPVOID lParam)
{
    HANDLE hPort = (HANDLE)lParam;
    HRESULT hr = NOERROR;

    for (;;)
    {
      //
      // Wait for completion notification.
      //
      DWORD dwNoOfBytes;
      ULONG_PTR dwKey;
      OVERLAPPED* pov = NULL;
      BOOL fSuccess = GetQueuedCompletionStatus(
                          hPort,
                          &dwNoOfBytes,
                          &dwKey,
                          &pov,
                          INFINITE    // Notification time-out interval
                          );
      //
      // A NULL value of pov is returned if completion port notification
      // failed. In this case, fSuccess is guaranteed to be FALSE.
      // When fSuccess is TRUE, the OVERLAPPED structure may still
      // contain an error code. It is inspected in HandleReceivedMessage.
      //
      if (pov == NULL)
      {
        //
        // Unrecoverable error occurred in the completion port. Wait for the next notification.
        //
        continue;
      }
      RECEIVE_CONTEXT* prc = CONTAINING_RECORD(pov, RECEIVE_CONTEXT, ov);
      HandleReceivedMessage(prc);

      //
      // Start the next message receive operation.
      //
      hr = EnableAsyncReceive(prc);
      if (FAILED(hr))
        return hr;
    } 
    
    //
    // The queue is unreachable.
    //
    return 0;
}


//
// CreateWorkingThreads
//  Parameters:
//    hPort   IN    completion port handle.
// 
//  Purpose:
//    Creates a certain number of worker threads whose start routine
//    is CompletionPortThread, associating each of them with
//    the incoming port handle.
//
void CreateWorkingThreads(HANDLE hPort)
{
    //
    // Start several threads to handle the completion port.
    // The number of threads that you create depends on number of processors
    // in the system and the expected serialization in the working thread logic.
    //
    const int xNumberOfProcessors = 1;
    const int xNumberOfThreads = 2 * xNumberOfProcessors;

    for (int i = 0; i < xNumberOfThreads; i++)
    {
      DWORD dwThreadId;
      HANDLE hThread = CreateThread(
                          NULL,                   // Thread security
                          0,                      // Default stack
                          CompletionPortThread,   // Start routine
                          hPort,                  // Start routine parameter
                          0,                      // Run immediately
                          &dwThreadId             // Thread identifier
                          );
      CloseHandle(hThread);
    }
}

//
// OpenQueueForAsyncReceiveWithCompletionPort
//  Parameters:
//    hPort   IN    completion port handle
//
//  Purpose:
//    Deletes and recreates the existing queue if already exists; 
//    otherwise, creates a new queue which is then opened for receive.
//    Finally, the newly opened queue handle is associated with the incoming
//    completion port.
//
IMSMQQueuePtr OpenQueueForAsyncReceiveWithCompletionPort(
    HANDLE hPort)
{
    IMSMQQueuePtr qRec;

    //
    // Delete the existing queue, ignoring errors.
    //
    try {
      g_qinfo->Delete();
    } 
    catch(_com_error comerr) {
    };

    //
    // Recreate the queue.
    //
    g_qinfo->Create();
    //
    // Open the queue to receive messages from it.
    //
    qRec = g_qinfo->Open(MQ_RECEIVE_ACCESS, MQ_DENY_NONE);

    //
    // Associate the queue with the completion port.
    //
    CreateIoCompletionPort(
      (HANDLE)qRec->Handle,       // Queue to associate port with
      hPort,                      // Port handle
      0, 
      0);
    return qRec;
}

//
// SendSomeMessages
//  Parameters:
//    cMsgs     IN    number of messages to send.
//
//  Purpose:
//    Sends a bunch of messages to the queue.
//
void SendSomeMessages(int cMsgs)
{
    IMSMQQueuePtr qSend;
    IMSMQMessagePtr msg("MSMQ.MSMQMessage");

    //
    // Open the queue for sending messages.
    //
    qSend = g_qinfo->Open(MQ_SEND_ACCESS, MQ_DENY_NONE);

    //
    // Send a bunch of messages.
    //
    for (int i = 0; i < cMsgs; i++)
    {
      msg->AppSpecific = i;
      msg->Send(qSend);
    }
}

//
// InitiateAsyncReceiveWithCompletionPort
//  Parameters:
//    q         IN    queue object to enable asynchronous messaging on
//
//  Purpose:
//    Creates several (a bunch!) MSMQ asynchronous message receive requests. Each
//    one is completion-port based.
//
HRESULT InitiateAsyncReceiveWithCompletionPort(IMSMQQueuePtr q)
{
    HRESULT hr = NOERROR;

    //
    // Kick off several overlapped receives.
    //
    const DWORD cOverlappedReceives = 1;

    for (int i = 0; i < cOverlappedReceives; i++)
    {
      //
      // Allocate and set a receive context.
      //
      RECEIVE_CONTEXT* prc = new RECEIVE_CONTEXT;
      memset(prc, 0, sizeof(RECEIVE_CONTEXT));
      prc->hQueue = (HANDLE)q->Handle;
      prc->pmsgprops = new MQMSGPROPS;
      AllocMsgProps(prc);
      hr = EnableAsyncReceive(prc);
      if (FAILED(hr)) {
        return hr;
      }
    }
    return hr;
}


//
// Check whether the local computer has access to the directory service (DS).
// 
short DetectDsConnection(void)
{
    HRESULT hresult;
    IMSMQApplication2Ptr pApp;

    hresult = CoInitialize(NULL);
    if (FAILED(hresult))
    {
        printf("\nCOM cannot be initialized. Error code: %d\n", hresult);
        exit(1);
    }

    hresult = pApp.CreateInstance(__uuidof(MSMQApplication));
    if (FAILED(hresult))
    {
        printf("\nThe MSMQApplication object cannot be created. Error code: %d\n", hresult);
        exit(1);
    }

    return pApp->GetIsDsEnabled();
}


//
// MAIN
//
int main()
{
    HRESULT hr;
    char strPathName[522];
    char strComputerName[256];
    char strQueueName[256];
    int dNoOfMessages;

    //
    // Initialize OLE.
    //
    hr = OleInitialize(NULL);
    if (FAILED(hr)) 
        return hr;  

    try 
    {
        g_qinfo = IMSMQQueueInfoPtr("MSMQ.MSMQQueueInfo");
        IMSMQQueuePtr qRec;    

        //
        // Create a new completion port.
        //
        HANDLE hPort = CreateIoCompletionPort(
                      INVALID_HANDLE_VALUE,     // Create a new port.
                      NULL,                     // Create a new port.
                      0, 
                      0);

        //
        // Create some worker threads to handle asynchronous receive operations.
        //
        CreateWorkingThreads(hPort);

        //
        // Get a queue path name.
        //
        printf("\nEnter queue computer name: ");
		if (fgets(strComputerName, 255, stdin) == NULL)
		{
			printf("\nInvalid input was entered.\n");
            exit(1);
		}
		//
		// Remove the \n at the end of the string created by fgets.
		//
		if(strComputerName[strlen(strComputerName) - 1] == '\n')
		{	  
			strComputerName[strlen(strComputerName) - 1] = '\0';
		}

        printf("\nEnter queue name: ");
		if (fgets(strQueueName, 255, stdin) == NULL)
		{
			printf("\nInvalid input was entered.\n");
            exit(1);
		}
		if(strQueueName[strlen(strQueueName) - 1] == '\n')
		{ 
			strQueueName[strlen(strQueueName) - 1] = '\0';
		}
        
        if(strComputerName[0] == 0 || strQueueName[0] == 0)
        {
            printf("\nInvalid parameters were supplied. Exiting...\n");
            exit(1);
        }

        //
        // Open the queue and associate it with the completion port.
        //
		int nRes, nSize;
		nSize = (sizeof(strComputerName)/sizeof(strComputerName[0])) + (sizeof(strQueueName)/sizeof(strQueueName[0])) -1;

        if(DetectDsConnection())
        //
		// Access to the DS is enabled, so work with a public queue.
        //
		{
			nRes = _snprintf_s(strPathName, sizeof(strPathName), nSize+1, "%s\\%s", strComputerName, strQueueName);
        }

        else
		//
        // Access to the DS is disabled, so work with a private queue.
        //
		{
			nRes = _snprintf_s(strPathName, sizeof(strPathName), nSize+10, "%s\\private$\\%s", strComputerName, strQueueName);
        }

		if(nRes < 0)
		{
			printf("The path name is too long for the buffer specified.\n");
			exit(1);
		}	

		strPathName[nRes] = '\0';
        g_qinfo->PathName = strPathName;
        qRec = OpenQueueForAsyncReceiveWithCompletionPort(hPort);

        //
        // Invoke MSMQ to start an asynchronous receive operation on the queue several times.
        //
        hr = InitiateAsyncReceiveWithCompletionPort(qRec);
        if (FAILED(hr)) {
        return hr;
        }

        //
        // Send some messages.
        //
        printf("\nEnter the number of messages to send: ");
		int iRes = scanf_s("%d", &dNoOfMessages);
		if (iRes == 0 || iRes == EOF)
		{
			printf("\nInvalid input was entered.\n");
			exit(1);
		}
      
        SendSomeMessages(dNoOfMessages); 

        //
        // Wait forever. Kill the application manually to exit.
        //
        printf("\nKill the application manually to exit.\n");
        Sleep(INFINITE);
        }
    catch(_com_error comerr) {
      HandleErrors(comerr);
    };
    return 0;
}