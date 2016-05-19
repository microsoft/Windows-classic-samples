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
// Includes
//
#include <stdio.h>
#include <windows.h>

//
// Unique include file for ActiveX MSMQ applications
//
#include <mqoai.h>

//
// Various defines
//
#define MAX_VAR       20
#define MAX_BUFFER   500

//
// GUID created with the tool "GUIDGEN"
//
static WCHAR strGuidMQTestType[] =
  L"{c30e0960-a2c0-11cf-9785-00608cb3e80c}";
//
// Prototypes
//
void PrintError(char *s, HRESULT hr);
HRESULT Syntax();

char mbsMachineName[MAX_COMPUTERNAME_LENGTH + 1];

// Some useful macros
#define RELEASE(punk) if (punk) { (punk)->Release(); (punk) = NULL; }
#define ADDREF(punk) ((punk) ? (punk)->AddRef() : 0)
#define PRINTERROR(s, hr) { PrintError(s, hr); goto Cleanup; }


//-----------------------------------------------------
//
// Check whether the local computer is enabled to 
// access the directory service (DS-enabled).
//
//----------------------------------------------------- 
short DetectDsConnection(void)
{
    IMSMQApplication2 *pqapp = NULL;
    short fDsConnection;
    HRESULT hresult;

    hresult = CoCreateInstance(
                   CLSID_MSMQApplication,
                   NULL,      // punkOuter
                   CLSCTX_SERVER,
                   IID_IMSMQApplication2,
                   (LPVOID *)&pqapp);
     
    if (FAILED(hresult)) 
	{
		printf("The attempt to create an MSMQApplication object failed (0x%X). Exiting...\n", hresult);
		RELEASE(pqapp);
		exit(1);
	}

    pqapp->get_IsDsEnabled(&fDsConnection); 
    RELEASE(pqapp);
    return fDsConnection;
}


//-----------------------------------------------------
//
//  Allow a DS-enabled client to connect to a 
//  DS-disabled one.
//
//-----------------------------------------------------
bool SetConnectionMode(void)
{

    char cDirectMode;
    

    //
    // If the local computer is in a domain and not in workgroup mode,
    // we have two cases:
    //   1. Other side is a computer in a domain.
    //   2. Other side is a computer working in workgroup mode.
    //

    if(DetectDsConnection() != 0)
    {
            printf("Do you want to connect to a DS-disabled computer (Y or N) ? ");
			
			int iRes = scanf_s("%c", &cDirectMode);
			if (iRes == 0 || iRes == EOF)
			{
				printf("\nInvalid input was entered.\n");
				exit(1);
			}
            
            switch(tolower(cDirectMode))
            {
                case 'y':
                    return true;

                case 'n':
                    return false;

                default:
                    printf("Bye.\n");
                    exit(1);
            }
            
    }
    
    return true;     // Local computer is DS-disabled.
}



//--------------------------------------------------------
//
// Receiver Mode
// -------------
// The receiver side does the following:
//    1. Creates a local queue of type "guidMQTestType".
//       The queue is either public or private, depending
//       on the connection that we want to establish.
//    2. Opens the queue.
//    3. In a loop,
//          receives messages and
//          prints the message body and message label.
//    4. Closes the handle to the queue.
//    5. Deletes the queue from the directory service.
//
//--------------------------------------------------------

HRESULT Receiver(bool fDirectMode)
{
    IMSMQMessage *pmessageReceive = NULL;
    IMSMQQueue *pqReceive = NULL;
    IMSMQQueueInfo  *pqinfo = NULL;
    BSTR bstrPathName = NULL;
    BSTR bstrServiceType = NULL;
    BSTR bstrLabel = NULL;
    BSTR bstrMsgLabel = NULL;
    VARIANT varIsTransactional, varIsWorldReadable, varBody, varBody2, varWantDestQueue, varWantBody, varReceiveTimeout;
    
	WCHAR wcsPathName[MAX_COMPUTERNAME_LENGTH +100];

	BOOL fQuit = FALSE;
    HRESULT hresult = NOERROR;

    printf("\nReceiver Mode on Machine: %s\n\n", mbsMachineName);

    //
    // Create an MSMQQueueInfo object.
    //
    hresult = CoCreateInstance(
                   CLSID_MSMQQueueInfo,
                   NULL,      // punkOuter
                   CLSCTX_SERVER,
                   IID_IMSMQQueueInfo,
                   (LPVOID *)&pqinfo);
    if (FAILED(hresult)) {
      PRINTERROR("The queue cannot be created", hresult);
    }

    //
    // Prepare properties for creating a queue on the local computer.
    //

    // Set the path name.
    if(fDirectMode)             // Private queue path name
    {
		if(_snwprintf_s(wcsPathName, sizeof(wcsPathName)/sizeof(wcsPathName[0]), (sizeof(wcsPathName)/sizeof(wcsPathName[0]))-1,
		L"%S\\private$\\MSMQTest", mbsMachineName) < 0)
		{
			// overflow
			PRINTERROR("Path name creation failed", hresult); 
		}
		else
		{
			wcsPathName[(sizeof(wcsPathName)/sizeof(wcsPathName[0]))-1] = L'\0';
		}
 
    }
    else                       // Public queue path name
    {    
		if(_snwprintf_s(wcsPathName, sizeof(wcsPathName)/sizeof(wcsPathName[0]), (sizeof(wcsPathName)/sizeof(wcsPathName[0]))-1,
		L"%S\\MSMQTest", mbsMachineName) < 0)
		{
			// Overflow occurred.
			PRINTERROR("Path name creation failed", hresult); 
		}
		else
		{
			wcsPathName[(sizeof(wcsPathName)/sizeof(wcsPathName[0]))-1] = L'\0';
		}
  
    }

    bstrPathName = SysAllocString(wcsPathName);
    if (bstrPathName == NULL) {
      PRINTERROR("OOM: pathname", E_OUTOFMEMORY);
    }
    pqinfo->put_PathName(bstrPathName);

    //
    // Set the type of the queue.
    // (This property will be used to locate all the queues of this type.)
    //
    bstrServiceType = SysAllocString(strGuidMQTestType);
    if (bstrServiceType == NULL) {
      PRINTERROR("OOM: ServiceType", E_OUTOFMEMORY);
    }
    pqinfo->put_ServiceTypeGuid(bstrServiceType);

    //
    // Add a descriptive label to the queue.
    // (This property is useful for administration through the MSMQ admin tools.)
    //
    bstrLabel =
      SysAllocString(L"Sample ActiveX application of MSMQ SDK");
    if (bstrLabel == NULL) {
      PRINTERROR("OOM: label ", E_OUTOFMEMORY);
    }
    pqinfo->put_Label(bstrLabel);

    //
    // Specify whether the queue is transactional.
    //
    VariantInit(&varIsTransactional);
    varIsTransactional.vt = VT_BOOL;
    varIsTransactional.boolVal = MQ_TRANSACTIONAL_NONE;
    VariantInit(&varIsWorldReadable);
    varIsWorldReadable.vt = VT_BOOL;
    varIsWorldReadable.boolVal = FALSE;
    //
    // Create the queue.
    //
    hresult = pqinfo->Create(&varIsTransactional, &varIsWorldReadable);
    if (FAILED(hresult)) {
      //
      // The API failed, but not because the queue exists.
      //
      if (hresult != MQ_ERROR_QUEUE_EXISTS) 
        PRINTERROR("The queue cannot be created", hresult);      
    }

    //
    // Open the queue with receive access.
    //
    hresult = pqinfo->Open(MQ_RECEIVE_ACCESS,
                           MQ_DENY_NONE,
                           &pqReceive);

    //
    // Things are a little bit tricky. MQCreateQueue succeeded, but in the 
    // case of a public queue, this does not mean that MQOpenQueue
    // will succeed, because replication delays are possible. The queue is
    // registered in the DS, but it might take a replication interval
    // until the replica reaches the server that I am connected to.
    // To overcome this, open the queue in a loop.
    //
    // In this specific case, this can happen only if this program
    // is run on an MSMQ 1.0 backup server controller (BSC) or on
    // a client connected to a BSC.
    // To be totally on the safe side, we should have put some code
    // to exit the loop after a few retries, but this is just a sample.
    //
    while (hresult == MQ_ERROR_QUEUE_NOT_FOUND) {
      printf(".");

      // Wait a bit.
      Sleep(500);

      // Retry.
      hresult = pqinfo->Open(MQ_RECEIVE_ACCESS,
                             MQ_DENY_NONE,
                             &pqReceive);
    }
    if (FAILED(hresult)) {
      PRINTERROR("The queue cannot be opened", hresult);
    }

    //
    // Main receiver loop
    //
    printf("\nWaiting for messages...\n");
    while (!fQuit) {
      //
      // Receive the message.
      //
      VariantInit(&varWantDestQueue);
      VariantInit(&varWantBody);
      VariantInit(&varReceiveTimeout);
      varWantDestQueue.vt = VT_BOOL;
      varWantDestQueue.boolVal = TRUE;    // Yes, we want the destination queue.
      varWantBody.vt = VT_BOOL;
      varWantBody.boolVal = TRUE;         // Yes, we want the message body.
      varReceiveTimeout.vt = VT_I4;
      varReceiveTimeout.lVal = INFINITE;  // Infinite time-out
      hresult = pqReceive->Receive(
                  NULL,
                  &varWantDestQueue,
                  &varWantBody,
                  &varReceiveTimeout,
                  &pmessageReceive);
      if (FAILED(hresult)) {
        PRINTERROR("Receive failed", hresult);
      }

      //
      // Display the received message.
      //
      pmessageReceive->get_Label(&bstrMsgLabel);
      VariantInit(&varBody);
      VariantInit(&varBody2);
      hresult = pmessageReceive->get_Body(&varBody);
      if (FAILED(hresult)) {
        PRINTERROR("The message body cannot be retrieved", hresult);
      }
      hresult = VariantChangeType(&varBody2,
                                  &varBody,
                                  0,
                                  VT_BSTR);
      if (FAILED(hresult)) {
        PRINTERROR("The message body cannot be converted to a string", hresult);
      }
      printf("%S : %s\n", bstrMsgLabel, (char *)V_BSTR(&varBody2));
      //
      // Check for a request to end the application.
      //
      if (_stricmp((char *)V_BSTR(&varBody2), "quit") == 0) {
        fQuit = TRUE;
      }

      VariantClear(&varBody);
      VariantClear(&varBody2);

      //
      // Release the current message.
      //
      RELEASE(pmessageReceive);
    } /* while (!fQuit) */

    //
    // Cleanup: Close the handle to the queue.
    //
    pqReceive->Close();
    if (FAILED(hresult)) {
      PRINTERROR("The queue cannot be closed", hresult);
    }

    //
    // In the concluding stage, we delete the queue from the directory
    // service. (We don't need to do this. In case of a public queue, 
    // leaving it in the DS enables sender applications to send messages 
    // even if the receiver is not available.)
    //
    hresult = pqinfo->Delete();
    if (FAILED(hresult)) {
      PRINTERROR("The queue cannot be deleted", hresult);
    }
    // Fall through...

Cleanup:
    SysFreeString(bstrPathName);
    SysFreeString(bstrMsgLabel);
    SysFreeString(bstrServiceType);
    SysFreeString(bstrLabel);
    RELEASE(pmessageReceive);
    RELEASE(pqReceive);
    RELEASE(pqinfo);
    return hresult;
}



//-----------------------------------------------------
//
// Sender Mode
// -----------
// The sender side does the following:
//
//    In domain (standard) mode:
//    1. Locates all queues of type "guidMQTestType."
//    2. Opens handles to all the queues.
//    3. In a loop,
//          sends messages to all those queues.
//    4. Cleans up handles.
//
//    If we work in workgroup (direct) mode:
//    1. Opens a handle to a private queue labeled
//       "MSMQTest" on the computer specified.
//    2. Sends messages to that queue.
//    3. Cleans up handles.
//-----------------------------------------------------


//-----------------------------------------------------
//
// Sender in domain (standard) mode
//
//-----------------------------------------------------
HRESULT StandardSender()
{
    IMSMQQuery *pquery = NULL;
    IMSMQQueueInfo *rgpqinfo[MAX_VAR];
    IMSMQQueue *rgpqSend[MAX_VAR];
    IMSMQQueueInfo *pqinfo = NULL;
    IMSMQQueueInfos *pqinfos = NULL;
    IMSMQMessage *pmessage = NULL;
    char szBuffer[MAX_BUFFER] = {0};
    WCHAR wcsMsgLabel[MQ_MAX_MSG_LABEL_LEN] = {0};
    BSTR bstrServiceType = NULL;
    BSTR bstrLabel = NULL;
    BSTR bstrBody = NULL;
    VARIANT varBody;
    DWORD i;
    DWORD cQueue = 0;
    HRESULT hresult = NOERROR;

    printf("\nSender mode on the computer %s\n\n", mbsMachineName);

    //
    // Create query object for lookup.
    //
    hresult = CoCreateInstance(
                   CLSID_MSMQQuery,
                   NULL,      // punkOuter
                   CLSCTX_SERVER,
                   IID_IMSMQQuery,
                   (LPVOID *)&pquery);
    if (FAILED(hresult)) {
      PRINTERROR("A query object cannot be created", hresult);
    }

    //
    // Prepare parameters to locate all queues that
    // have the test service type GUID.
    //
    VARIANT varGuidQueue;
    VARIANT varStrLabel;
    VARIANT varGuidServiceType;
    VARIANT varRelServiceType;
    VARIANT varRelLabel;
    VARIANT varCreateTime;
    VARIANT varModifyTime;
    VARIANT varRelCreateTime;
    VARIANT varRelModifyTime;

    VariantInit(&varGuidQueue);
    VariantInit(&varStrLabel);
    VariantInit(&varGuidServiceType);
    VariantInit(&varRelServiceType);
    VariantInit(&varRelLabel);
    VariantInit(&varCreateTime);
    VariantInit(&varModifyTime);
    VariantInit(&varRelCreateTime);
    VariantInit(&varRelModifyTime);

    //
    // We only want to specify the service type GUID, so we
    // set the other variant parameters to VT_ERROR to simulate
    // "missing," i.e. optional, parameters.
    //
    V_VT(&varGuidQueue) = VT_ERROR;
    V_VT(&varStrLabel) = VT_ERROR;
    V_VT(&varRelServiceType) = VT_ERROR;
    V_VT(&varRelLabel) = VT_ERROR;
    V_VT(&varCreateTime) = VT_ERROR;
    V_VT(&varModifyTime) = VT_ERROR;
    V_VT(&varRelCreateTime) = VT_ERROR;
    V_VT(&varRelModifyTime) = VT_ERROR;
    bstrServiceType = SysAllocString(strGuidMQTestType);
    if (bstrServiceType == NULL) {
      PRINTERROR("OOM: Service Type GUID", E_OUTOFMEMORY);
    }
    V_VT(&varGuidServiceType) = VT_BSTR;
    V_BSTR(&varGuidServiceType) = bstrServiceType;

    hresult = pquery->LookupQueue(&varGuidQueue,
                                  &varGuidServiceType,
                                  &varStrLabel,
                                  &varCreateTime,
                                  &varModifyTime,
                                  &varRelServiceType,
                                  &varRelLabel,
                                  &varRelCreateTime,
                                  &varRelModifyTime,
                                  &pqinfos);
    if (FAILED(hresult)) {
      PRINTERROR("LookupQueue failed", hresult);
    }

    //
    // Reset the queue collection object.
    //
    hresult = pqinfos->Reset();
    if (FAILED(hresult)) {
      PRINTERROR("Reset failed", hresult);
    }

    //
    // Open each of the queues found.
    //
    cQueue = 0;
    hresult = pqinfos->Next(&rgpqinfo[cQueue]);
    if (FAILED(hresult)) {
      PRINTERROR("Next failed", hresult);
    }
    pqinfo = rgpqinfo[cQueue];
    while (pqinfo) {
      //
      // Open the queue with send access.
      //

      hresult = pqinfo->Open(
                  MQ_SEND_ACCESS,
                  MQ_DENY_NONE,
                  &rgpqSend[cQueue]);
      if (FAILED(hresult)) {
        PRINTERROR("Open failed", hresult);
      }
    
	  cQueue++;
      hresult = pqinfos->Next(&rgpqinfo[cQueue]);
      if (FAILED(hresult)) {
        PRINTERROR("Next failed", hresult);
      }
      pqinfo = rgpqinfo[cQueue];
    }
    
    if (cQueue == 0) {
      //
      // No queue could be found, so exit.
      //
      PRINTERROR("No queue is registered", hresult = E_INVALIDARG);
    }
    printf("\tQueue(s) found: %d\n", cQueue);
    printf("\nEnter \"quit\" to exit.\n");

    //
    // Build the message label property.
    //

	if(_snwprintf_s(wcsMsgLabel, sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]), (sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]))-1,
		L"Message from %S", mbsMachineName) < 0)
	{
		// overflow
		PRINTERROR("Label creation failed", hresult); 
	}
	else
	{
		wcsMsgLabel[(sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]))-1] = L'\0';
	}






    bstrLabel = SysAllocString(wcsMsgLabel);
    if (bstrLabel == NULL)
        PRINTERROR("OOM: label", E_OUTOFMEMORY);

    fflush(stdin);
    //
    // Main sender loop
    //
    while (1) {
      //
      // Get a string from the console.
      //
      printf("Enter a string: ");
      if (fgets(szBuffer, MAX_BUFFER - 1, stdin) == NULL)
        break;

  	  szBuffer[MAX_BUFFER - 1] = '\0';

	  //
	  // Delete the new-line character from the string.
	  //
	  if(szBuffer[strlen(szBuffer) - 1] == '\n')
	  {
	  
		szBuffer[strlen(szBuffer) - 1] = '\0';
	  
	  }


      //
      // Create a message object.
      //
      hresult = CoCreateInstance(
                     CLSID_MSMQMessage,
                     NULL,      // punkOuter
                     CLSCTX_SERVER,
                     IID_IMSMQMessage,
                     (LPVOID *)&pmessage);
      //
      // Send the message to all the queues.
      //
      for (i = 0; i < cQueue; i++) 
      {
        hresult = pmessage->put_Label(bstrLabel);

        //
        // This isn't a "true" Unicode string, of course.
        //
        bstrBody = SysAllocStringByteLen(szBuffer, strlen(szBuffer) + 1);
        if (bstrBody == NULL) {
          PRINTERROR("OOM: message body", E_OUTOFMEMORY);
        }
        VariantInit(&varBody);
        V_VT(&varBody) = VT_BSTR;
        V_BSTR(&varBody) = bstrBody;
        hresult = pmessage->put_Body(varBody);
        if (FAILED(hresult)) {
          PRINTERROR("put_body failed", hresult);
        }
        hresult = pmessage->Send(rgpqSend[i], NULL);
        if (FAILED(hresult)) {
          PRINTERROR("Send failed", hresult);
        }
        VariantClear(&varBody);
        bstrBody = NULL;
      }
      RELEASE(pmessage);

      //
      // Check for a request to end the application.
      //
      if (_stricmp(szBuffer, "quit") == 0)
        break;
    } /* while (1) */



Cleanup:
    //
    // Close and release all the queues.
    //
    for (i = 0; i < cQueue; i++) {
      rgpqSend[i]->Close();
      rgpqSend[i]->Release();
      rgpqinfo[i]->Release();
    }
    RELEASE(pqinfos);
    RELEASE(pquery);
    RELEASE(pmessage);
    SysFreeString(bstrLabel);
    SysFreeString(bstrBody);
    SysFreeString(bstrServiceType);
    return hresult;
}


//-----------------------------------------------------
//
// Sender in direct (workgroup) mode
//
//-----------------------------------------------------
HRESULT DirectSender(void)
{
    IMSMQQueue *rgpqSend = NULL;
    IMSMQQueueInfo *pqinfo = NULL;
    IMSMQMessage *pmessage = NULL;
    char szBuffer[MAX_BUFFER];
    WCHAR wcsMsgLabel[MQ_MAX_MSG_LABEL_LEN] = {0};

    WCHAR wcsFormat[MAX_BUFFER + 100] = {0};
	WCHAR wcsReceiverComputer[MAX_BUFFER] = {0};
    BSTR bstrFormat = NULL;
    BSTR bstrServiceType = NULL;
    BSTR bstrLabel = NULL;
    BSTR bstrBody = NULL;
    VARIANT varBody;
    DWORD cQueue = 0;
    HRESULT hresult = NOERROR;


    //
    // Get the receiver computer name.
    //

	//
	// Flushing stdin to get rid of the new-line character entered earlier.
	//
	fflush(stdin);
    printf("\nEnter receiver computer name: ");

	if (fgetws(wcsReceiverComputer, MAX_BUFFER - 1, stdin) == NULL)
        return E_INVALIDARG;

	if(wcsReceiverComputer[wcslen(wcsReceiverComputer) - 1] == L'\n')
		wcsReceiverComputer[wcslen(wcsReceiverComputer) - 1] = L'\0';


	if(wcsReceiverComputer[0] == 0)
    {
        printf("You have entered an incorrect parameter. Exiting...\n");
        return E_INVALIDARG;
    }

    hresult = CoCreateInstance(
               CLSID_MSMQQueueInfo,
               NULL,      // punkOuter
               CLSCTX_SERVER,
               IID_IMSMQQueueInfo,
               (LPVOID *)&pqinfo);
    if (FAILED(hresult))
      PRINTERROR("The queue cannot be opened", hresult);

	if(_snwprintf_s(wcsFormat, sizeof(wcsFormat)/sizeof(wcsFormat[0]), (sizeof(wcsFormat)/sizeof(wcsFormat[0])) - 1,
	L"DIRECT=OS:%s\\private$\\MSMQTest", wcsReceiverComputer) < 0)
	{
		// Overflow occurred.
		PRINTERROR("The receiver format name is too long for the buffer specified", hresult); 
	}
	else
	{
		wcsFormat[(sizeof(wcsFormat)/sizeof(wcsFormat[0])) - 1] = L'\0';
	}

    bstrFormat = SysAllocString(wcsFormat);
    hresult = pqinfo->put_FormatName(bstrFormat);    
    if (FAILED(hresult))
        PRINTERROR("Open failed", hresult);

    //
    // Open the queue with send access.
    //
    hresult = pqinfo->Open(
              MQ_SEND_ACCESS,
              MQ_DENY_NONE,
              &rgpqSend);

    
    if (FAILED(hresult))
        PRINTERROR("Open failed", hresult);

    printf("\nSender mode on the computer %s.\n\n", mbsMachineName);
    printf("\nEnter \"quit\" to exit.\n");


    //
    // Build the message label property.
    //

	if(_snwprintf_s(wcsMsgLabel, sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]), (sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0])) - 1,
		L"Message from %S", mbsMachineName) < 0)
	{
		// Overflow occurred.
		PRINTERROR("Label creation failed", hresult); 
	}
	else
	{
		wcsMsgLabel[(sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0])) - 1] = L'\0';
	}

	
	bstrLabel = SysAllocString(wcsMsgLabel);
    if (bstrLabel == NULL)
        PRINTERROR("OOM: label", E_OUTOFMEMORY);


    fflush(stdin);
    //
    // Main sender loop.
    //
    while (1) 
    {
        //
        // Get a string from the console.
        //
        printf("Enter a string: ");
        if (fgets(szBuffer,sizeof(szBuffer) - 1, stdin) == NULL) 
			break;
	
		if(szBuffer[strlen(szBuffer) - 1] == '\n')
		{
	  
			szBuffer[strlen(szBuffer) - 1] = '\0';
	  
		}
        
		//
        // Create a message object.
        //
        hresult = CoCreateInstance(
                     CLSID_MSMQMessage,
                     NULL,      // punkOuter
                     CLSCTX_SERVER,
                     IID_IMSMQMessage,
                     (LPVOID *)&pmessage);
        //
        // Send the message.
        //

        hresult = pmessage->put_Label(bstrLabel);

        bstrBody = SysAllocStringByteLen(szBuffer, strlen(szBuffer) + 1);
        if (bstrBody == NULL)
          PRINTERROR("OOM: body", E_OUTOFMEMORY);

        VariantInit(&varBody);
        V_VT(&varBody) = VT_BSTR;
        V_BSTR(&varBody) = bstrBody;
        hresult = pmessage->put_Body(varBody);
        if (FAILED(hresult)) 
          PRINTERROR("put_body failed", hresult);

        hresult = pmessage->Send(rgpqSend, NULL);
        if (FAILED(hresult))
          PRINTERROR("Send failed", hresult);

        VariantClear(&varBody);
        bstrBody = NULL;

        RELEASE(pmessage);

        //
        // Check for a request to end teh application.
        //
        if (_stricmp(szBuffer, "quit") == 0)
        break;
    } /* while (1) */



Cleanup:
	if(rgpqSend != NULL)
	{
		rgpqSend->Close();
		rgpqSend->Release();
    }

    RELEASE(pqinfo);
    RELEASE(pmessage);
    SysFreeString(bstrFormat);
    SysFreeString(bstrLabel);
    SysFreeString(bstrBody);
    SysFreeString(bstrServiceType);
    return hresult;
}



//------------------------------------------------------
//
// Sender function
//
//------------------------------------------------------
HRESULT Sender(int fDirectMode)
{

    if(fDirectMode != 0)
        return DirectSender();

    else
        return StandardSender();
}


//-----------------------------------------------------
//
//  MAIN
//
//-----------------------------------------------------
int main(int argc, char * * argv)
{
    DWORD dwNumChars;
    HRESULT hresult = NOERROR;
    bool fDirectMode;

    if (argc != 2)
      return Syntax();

    hresult = OleInitialize(NULL);
    if (FAILED(hresult)) 
      PRINTERROR("OLE cannot be initialized", hresult);

    //
    // Retrieve the computer name.
    //
    dwNumChars = MAX_COMPUTERNAME_LENGTH;
    if(!GetComputerNameA(mbsMachineName, &dwNumChars))
	{
		printf("The computer name cannot be obtained. Exiting...\n");
		exit(1);
	}
    //
    // Detect DS connection and working mode.
    //
    fDirectMode = SetConnectionMode();


    if (strcmp(argv[1], "-s") == 0)
      hresult = Sender(fDirectMode);

    else if (strcmp(argv[1], "-r") == 0)
      hresult = Receiver(fDirectMode);

    else
      hresult = Syntax();

    printf("\nOK\n");

    // Fall through...


Cleanup:
    return (int)hresult;
}


void PrintError(char *s, HRESULT hr)
{
    printf("%s (0x%X). Exiting...\n", s, hr);
}


HRESULT Syntax()
{
    printf("\n");
    printf("Syntax: mqtestoa -s | -r\n");
    printf("\t-s: Sender\n");
    printf("\t-r: Receiver\n");
    return E_INVALIDARG;
}
