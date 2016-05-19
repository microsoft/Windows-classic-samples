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
// Unique include file for MSMQ applications
//
#include <mq.h>


//
// Various defines
//
#define MAX_VAR       20
#define MAX_FORMAT   300
#define MAX_BUFFER   500

#define DIRECT         1 
#define STANDARD       0

#define DS_ENABLED     1
#define DS_DISABLED    0


//
// GUID created with the tool "GUIDGEN"
//
static CLSID guidMQTestType =
{ 0xc30e0960, 0xa2c0, 0x11cf, { 0x97, 0x85, 0x0, 0x60, 0x8c, 0xb3, 0xe8, 0xc } };


//
// Prototypes
//
void Error(char *s, HRESULT hr);
void Syntax();


char mbsMachineName[MAX_COMPUTERNAME_LENGTH + 1];


//-----------------------------------------------------
//
// Check whether the local computer is enabled to access
// the directory service (DS-enabled).
//
//----------------------------------------------------- 
int DetectDsConnection(void)
{

    MQPRIVATEPROPS PrivateProps;
    QMPROPID       aPropId[MAX_VAR];
    MQPROPVARIANT  aPropVar[MAX_VAR];
    HRESULT        aStatus[MAX_VAR];

    DWORD          cProp;
    
    HRESULT        hr;


    //
    // Specify the PROPID_PC_DS_ENABLED property, which
	// indicates whether the local computer can access the DS.
    //
    cProp = 0;

    aPropId[cProp] = PROPID_PC_DS_ENABLED;
    aPropVar[cProp].vt = VT_NULL;
    ++cProp;	

    // Create a PRIVATEPROPS structure.
    PrivateProps.cProp = cProp;
	PrivateProps.aPropID = aPropId;
	PrivateProps.aPropVar = aPropVar;
    PrivateProps.aStatus = aStatus;
    //
    // Retrieve the information.
    //
	hr = MQGetPrivateComputerInformation(
				     NULL,
					 &PrivateProps);
	if(FAILED(hr))
	{
        Error("A DS connection cannot be detected", hr);
    }
	
    
    if(PrivateProps.aPropVar[0].boolVal == 0)
        return DS_DISABLED;
    

    return DS_ENABLED;
}


//-----------------------------------------------------
//
//  Allow a DS-enabled client to connect with a 
//  DS-disabled one.
//
//-----------------------------------------------------
int SetConnectionMode(void)
{

    char cDirectMode;
    int iRes;

    //
    // If the local computer is in a domain and not in workgroup mode,
    // we have two cases:
    //   1. Other side is a computer operating in domain mode.
    //   2. Other side is a computer operating in workgroup mode.
    //

    if(DetectDsConnection() == DS_ENABLED)
    {
            printf("Do you want to connect to a DS-disabled client (y or n) ? ");
			iRes = scanf_s("%c", &cDirectMode);
			if (iRes == 0 || iRes == EOF)
			{
				printf("\nInvalid input.\n");
				exit(1);
			}

			//
			// Flushing stdin to get rid of the new line entered earlier.
			//
			fflush(stdin);
            
            switch(tolower(cDirectMode))
            {
                case 'y':
                    return DIRECT;

                case 'n':
                    return STANDARD;

                default:
                    printf("Bye.\n");
                    exit(1);
            }
            
    }

    
    return DIRECT;     // Local computer is DS-disabled
}
    

//--------------------------------------------------------
//
// Receiver Mode
// -------------
// The receiver side does the following:
//    1. Creates a queue on the local computer
//       of type "guidMQTestType".
//       The queue is either public or private, depending
//       on the connection we want to establish.
//    2. Opens the queue.
//    3. In a loop,
//          receives messages
//          and prints the message body and message label.
//    4. Cleans up handles.
//    5. Deletes the queue from the directory service.
//
//--------------------------------------------------------
void Receiver(int dDirectMode)
{

    MQQUEUEPROPS  qprops;
    MQMSGPROPS    msgprops;
    MQPROPVARIANT aPropVar[MAX_VAR];
    QUEUEPROPID   aqPropId[MAX_VAR];
    MSGPROPID     amPropId[MAX_VAR];
    DWORD         cProps;

    WCHAR         wcsFormat[MAX_FORMAT];

    UCHAR         Buffer[MAX_BUFFER];
    WCHAR         wcsMsgLabel[MQ_MAX_MSG_LABEL_LEN+1];
    WCHAR         wcsPathName[MAX_FORMAT];


    DWORD         dwNumChars;
    QUEUEHANDLE   qh;

    HRESULT       hr;
	int			  nPos;

    printf("\nReceiver Mode on Machine: %s\n\n", mbsMachineName);


    //
    // Set properties to create a queue on the local computer.
    //
    cProps = 0;

    // Set the path name.
    if(dDirectMode == DIRECT)  // Private queue path name
    {
		nPos = _snwprintf_s(
                           wcsPathName, 
                           sizeof(wcsPathName)/sizeof(wcsPathName[0]), 
                           sizeof(wcsPathName)/sizeof(wcsPathName[0]) - 1, 
                           L"%S\\private$\\MSMQTest", 
                           mbsMachineName
                           );
		if(nPos < 0)
		{
			printf("The path name is too long for the buffer. Exiting...\n");
			exit(1);
		}	

    }
    else                       // Public queue path name
    {    
		nPos = _snwprintf_s(
                           wcsPathName, 
                           sizeof(wcsPathName)/sizeof(wcsPathName[0]), 
                           sizeof(wcsPathName)/sizeof(wcsPathName[0]) - 1, 
                           L"%S\\MSMQTest", 
                           mbsMachineName
                           );
		if(nPos < 0)
		{
			printf("The path name is too long for the buffer. Exiting...\n");
			exit(1);
		}	

    }

    aqPropId[cProps]         = PROPID_Q_PATHNAME;
    aPropVar[cProps].vt      = VT_LPWSTR;
    aPropVar[cProps].pwszVal = wcsPathName;
    cProps++;

    // Specify the service type GUID of the queue.
    // (This property will be used to locate all the queues of this type.)
    aqPropId[cProps]         = PROPID_Q_TYPE;
    aPropVar[cProps].vt      = VT_CLSID;
    aPropVar[cProps].puuid   = &guidMQTestType;
    cProps++;

    // Add a descriptive label to the queue.
    // (A label is useful for administration through the MSMQ admin tools.)
    aqPropId[cProps]         = PROPID_Q_LABEL;
    aPropVar[cProps].vt      = VT_LPWSTR;
    aPropVar[cProps].pwszVal = L"Sample application of MSMQ SDK";
    cProps++;

    // Create a QUEUEPROPS structure.
    qprops.cProp    = cProps;
    qprops.aPropID  = aqPropId;
    qprops.aPropVar = aPropVar;
    qprops.aStatus  = 0;


    //
    // Create the queue.
    //
    dwNumChars = MAX_FORMAT;
    hr = MQCreateQueue(
            NULL,           // IN:     Default security
            &qprops,        // IN/OUT: Queue properties
            wcsFormat,      // OUT:    Format name
            &dwNumChars);   // IN/OUT: Size of the format name

    if (FAILED(hr))
    {
        //
        // API failed, but not because the queue exists.
        //
        if (hr != MQ_ERROR_QUEUE_EXISTS)
            Error("MQCreateQueue failed", hr);

        //
        // The queue exist, so get its format name.
        //
        printf("The queue already exists. Open it anyway.\n");

        if(dDirectMode == DIRECT)
        // It's a private queue, so we know its direct format name.
        {
			int n = _snwprintf_s(
                                    wcsFormat, 
                                    sizeof(wcsFormat)/sizeof(wcsFormat[0]), 
                                    sizeof(wcsFormat)/sizeof(wcsFormat[0]) - 1, 
                                    L"DIRECT=OS:%s", 
                                    wcsPathName);
			if(n < 0)
			{
				printf("The format name is too long for the buffer. Exiting...\n");
				exit(1);
			}	

        }
        else
        // It's a public queue, so we must get its format name from the DS.
        {
            dwNumChars = sizeof(wcsFormat)/sizeof(wcsFormat[0]);
            hr = MQPathNameToFormatName(
                       wcsPathName,     // IN:     Queue path name
                       wcsFormat,       // OUT:    Format name
                       &dwNumChars);    // IN/OUT: Size of the format name

            if (FAILED(hr))
                Error("The format name cannot be retrieved", hr);
        }
    }


    //
    // Open the queue with receive access.
    //
    hr = MQOpenQueue(
             wcsFormat,          // IN:  Queue format name
             MQ_RECEIVE_ACCESS,  // IN:  Want to receive from queue
             0,                  // IN:  Allow sharing
             &qh);               // OUT: Handle of open queue

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
    while (hr == MQ_ERROR_QUEUE_NOT_FOUND)
    {
       printf(".");

       // Wait a bit.
       Sleep(500);

       // Retry.
       hr = MQOpenQueue(wcsFormat, MQ_RECEIVE_ACCESS, 0, &qh);
    }

    if (FAILED(hr))
         Error("The queue cannot be opened", hr);


    //
    // Main receiver loop
    //
    if(dDirectMode == DIRECT)
    {
        printf("\n* Working in workgroup (direct) mode.\n");
    }
    printf("\n* Waiting for messages...\n");
    
    for(;;)
    {
        //
        // Set the message properties for reading messages.
        //
        cProps = 0;

        // Ask for the body of the message.
        amPropId[cProps]            = PROPID_M_BODY;
        aPropVar[cProps].vt         = VT_UI1 | VT_VECTOR;
        aPropVar[cProps].caub.cElems = sizeof(Buffer);
        aPropVar[cProps].caub.pElems = Buffer;
        cProps++;

        // Ask for the label of the message.
        amPropId[cProps]         = PROPID_M_LABEL;
        aPropVar[cProps].vt      = VT_LPWSTR;
        aPropVar[cProps].pwszVal = wcsMsgLabel;
        cProps++;

        // Ask for the length of the label of the message.
        amPropId[cProps]         = PROPID_M_LABEL_LEN;
        aPropVar[cProps].vt      = VT_UI4;
        aPropVar[cProps].ulVal   = sizeof(wcsMsgLabel);
        cProps++;

        // Create a MSGPROPS structure.
        msgprops.cProp    = cProps;
        msgprops.aPropID  = amPropId;
        msgprops.aPropVar = aPropVar;
        msgprops.aStatus  = 0;


        //
        // Receive the message.
        //
        hr = MQReceiveMessage(
               qh,                // IN:     Queue handle
               INFINITE,          // IN:     Time-out
               MQ_ACTION_RECEIVE, // IN:     Read operation
               &msgprops,         // IN/OUT: Message properties to retrieve
               NULL,              // IN/OUT: No OVERLAPPED structure
               NULL,              // IN:     No callback
               NULL,              // IN:     No cursor
               NULL);             // IN:     Not part of a transaction

        if (FAILED(hr))
            Error("MQReceiveMessage failed", hr);

        //
        // Display the received message.
        //
        printf("%S : %s\n", wcsMsgLabel, Buffer);

        //
        // Check for a request to end the application.
        //
        if (_stricmp(Buffer, "quit") == 0)
            break;

    } /* while (1) */

    //
    // Cleanup: Close the handle to the queue.
    //
    MQCloseQueue(qh);


    //
    // In the concluding stage, we delete the queue from the directory
    // service. (We don't need to do this. In case of a public queue, 
    // leaving it in the DS enables sender applications to send messages 
    // even if the receiver is not available.)
    //
    hr = MQDeleteQueue(wcsFormat);
    if (FAILED(hr))
        Error("The queue cannot be deleted", hr);
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
void StandardSender(void)
{
    DWORD         cProps;

    MQMSGPROPS    msgprops;
    MQPROPVARIANT aPropVar[MAX_VAR];
    QUEUEPROPID   aqPropId[MAX_VAR];
    MSGPROPID     amPropId[MAX_VAR];

    MQPROPERTYRESTRICTION aPropRestriction[MAX_VAR];
    MQRESTRICTION Restriction;

    MQCOLUMNSET      Column;
    HANDLE        hEnum;

    WCHAR         wcsFormat[MAX_FORMAT];

    UCHAR         Buffer[MAX_BUFFER];
    WCHAR         wcsMsgLabel[MQ_MAX_MSG_LABEL_LEN+1];

    DWORD         i;

    DWORD         cQueue;
    DWORD         dwNumChars;
    QUEUEHANDLE   aqh[MAX_VAR];

    HRESULT       hr;
	int			  nPos;


    printf("\nSender mode on the computer %s\n\n", mbsMachineName);


    //
    // Prepare parameters for locating queues.
    //

    //
    // 1. Restriction: Queues with PROPID_Q_TYPE = MSMQTest service type GUIID
    //
    cProps = 0;
    aPropRestriction[cProps].rel         = PREQ;
    aPropRestriction[cProps].prop        = PROPID_Q_TYPE;
    aPropRestriction[cProps].prval.vt    = VT_CLSID;
    aPropRestriction[cProps].prval.puuid = &guidMQTestType;
    cProps++;

    Restriction.cRes      = cProps;
    Restriction.paPropRes = aPropRestriction;


    //
    // 2. Columnset (the queue properties to retrieve) = queue GUID
    //
    cProps = 0;
    aqPropId[cProps] = PROPID_Q_INSTANCE;
    cProps++;

    Column.cCol = cProps;
    Column.aCol = aqPropId;


    //
    // Issue the query to locate the queues.
    //
    hr = MQLocateBegin(
             NULL,          // IN:  Context must be NULL
             &Restriction,  // IN:  Restriction
             &Column,       // IN:  Properties to return
             NULL,          // IN:  No need to sort
             &hEnum);       // OUT: Enumeration handle

    if (FAILED(hr))
        Error("MQLocateBegin failed", hr);

    //
    // Get the results (up to MAX_VAR).
    // (For more results, call MQLocateNext in a loop.)
    //
    cQueue = MAX_VAR;
    hr = MQLocateNext(
             hEnum,         // IN:     Enumeration handle
             &cQueue,       // IN/OUT: Number of properties
             aPropVar);     // OUT:    Properties of the queue located

    if (FAILED(hr))
        Error("MQLocateNext failed", hr);

    //
    // End the query and release the resouces associated with it.
    //
    hr = MQLocateEnd(hEnum);

    if (cQueue == 0)
    {
        //
        // No queue could be found, so exit.
        //
        printf("No queue was registered.");
        exit(0);
    }


    printf("\tQueue(s) found: %d\n", cQueue);


    //
    // Open a handle for each of the queues found.
    //
    for (i = 0; i < cQueue; i++)
    {
        // Convert the queue GUID to a format name.
        dwNumChars = MAX_FORMAT;
        hr = MQInstanceToFormatName(
                  aPropVar[i].puuid,    // IN:     Queue GUID
                  wcsFormat,            // OUT:    Format name
                  &dwNumChars);         // IN/OUT: Size of format name

        if (FAILED(hr))
            Error("MQInstanceToFormatName failed", hr);

        //
        // Open the queue with send access.
        //
        hr = MQOpenQueue(
                 wcsFormat,           // IN:  Queue format name
                 MQ_SEND_ACCESS,      // IN:  Want to send to queue
                 0,                   // IN:  Must be 0 for send access
                 &aqh[i]);            // OUT: Handle of open queue

        if (FAILED(hr))
            Error("MQOpenQueue failed", hr);

        //
        // Free the memory that was allocated for the queue GUID during the query.
        //
        MQFreeMemory(aPropVar[i].puuid);
    }


    printf("\nEnter \"quit\" to exit.\n");


    //
    // Construct the message label.
    //
	nPos = _snwprintf_s(
                   wcsMsgLabel, 
                   sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]), 
                   sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]) - 1, 
                   L"Message from %S", 
                   mbsMachineName
                   );
	if(nPos < 0)
	{
		printf("The label is too long for the buffer. Exiting...\n");
		exit(1);
	}	

    //
    // Main sender loop
    //
    for(;;)
    {
        //
        // Get a string from the console.
        //
        printf("Enter a string: ");
		if (fgets(Buffer, MAX_BUFFER - 1, stdin) == NULL)
            break;
		
		Buffer[MAX_BUFFER-1] = '\0';
		//
		// Deleting the new-line character.
		//
		if(Buffer[strlen(Buffer) - 1] == '\n')
		{
			Buffer[strlen(Buffer)-1] = '\0'; 
		}


        //
        // Prepare the message properties for sending messages.
        //
        cProps = 0;

        // Set the message body.
        amPropId[cProps]            = PROPID_M_BODY;
        aPropVar[cProps].vt         = VT_UI1 | VT_VECTOR;
        aPropVar[cProps].caub.cElems = sizeof(Buffer);
        aPropVar[cProps].caub.pElems = Buffer;
        cProps++;

        // Set the message label.
        amPropId[cProps]            = PROPID_M_LABEL;
        aPropVar[cProps].vt         = VT_LPWSTR;
        aPropVar[cProps].pwszVal    = wcsMsgLabel;
        cProps++;

        // Create a MSGPROPS structure.
        msgprops.cProp    = cProps;
        msgprops.aPropID  = amPropId;
        msgprops.aPropVar = aPropVar;
        msgprops.aStatus  = 0;


        //
        // Send the message to all the queues.
        //
        for (i = 0; i < cQueue; i++)
        {
            hr = MQSendMessage(
                    aqh[i],     // IN: Queue handle
                    &msgprops,  // IN: Message properties to send
                    NULL);      // IN: Not part of a transaction

            if (FAILED(hr))
                Error("MQSendMessage failed", hr);
        }

        //
        // Check for a request to end the application.
        //
        if (_stricmp(Buffer, "quit") == 0)
            break;

    } /* for */


    //
    // Close all the queue handles.
    //
    for (i = 0; i < cQueue; i++)
        MQCloseQueue(aqh[i]);

}



//-----------------------------------------------------
//
// Sender in workgroup (direct) mode
//
//-----------------------------------------------------
void DirectSender(void)
{

    MQMSGPROPS    msgprops;
    MQPROPVARIANT aPropVar[MAX_VAR];
    MSGPROPID     amPropId[MAX_VAR];
    DWORD         cProps;

    WCHAR         wcsFormat[MAX_FORMAT];
    WCHAR         wcsReceiverComputer[MAX_COMPUTERNAME_LENGTH + 1];

    UCHAR         Buffer[MAX_BUFFER];
    WCHAR         wcsMsgLabel[MQ_MAX_MSG_LABEL_LEN+1];

    QUEUEHANDLE   qhSend;

    HRESULT       hr;
	int			  nPos;

    //
    // Get the receiver computer name.
    //
    printf("Enter receiver computer name: ");
	if (fgetws(wcsReceiverComputer, MAX_COMPUTERNAME_LENGTH, stdin) == NULL)
	{
        printf("An invalid parameter was entered. Exiting...\n");
        exit(1);
	}
	wcsReceiverComputer[MAX_COMPUTERNAME_LENGTH] = L'\0';
	//
	// Deleting the new-line character.
	//
	if(wcsReceiverComputer[wcslen(wcsReceiverComputer) - 1] == L'\n')
	{
		wcsReceiverComputer[wcslen(wcsReceiverComputer) - 1] = L'\0'; 
	}
    
    if(wcsReceiverComputer[0] == 0)
    {
        printf("An invalid parameter was entered. Exiting...\n");
        exit(1);
    }

    //
    // Open the queue with send access.
    //
	nPos = _snwprintf_s(
                   wcsFormat, 
                   sizeof(wcsFormat)/sizeof(wcsFormat[0]), 
                   sizeof(wcsFormat)/sizeof(wcsFormat[0]) - 1, 
                   L"DIRECT=OS:%s\\private$\\MSMQTest", 
                   wcsReceiverComputer
                   );
	if(nPos < 0)
	{
		printf("The format name is too long for the buffer. Exiting...\n");
		exit(1);
	}	

    hr = MQOpenQueue(
             wcsFormat,           // IN:  Queue format name
             MQ_SEND_ACCESS,      // IN:  Want to send to queue
             0,                   // IN:  Must be 0 for send access
             &qhSend);            // OUT: Handle of open queue

    if (FAILED(hr))
        Error("MQOpenQueue failed", hr);


    printf("\nEnter \"quit\" to exit\n");


    //
    // Construct the message label.
    //
	nPos = _snwprintf_s(
                   wcsMsgLabel, 
                   sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]), 
                   sizeof(wcsMsgLabel)/sizeof(wcsMsgLabel[0]) - 1, 
                   L"Message from %S", 
                   mbsMachineName);
	if(nPos < 0)
	{
		printf("The label is too long for the buffer. Exiting...\n");
		exit(1);
	}	
	
    
    fflush(stdin);

    //
    // Main sender loop
    //
    for(;;)
    {
        //
        // Get a string from the console.
        //
        printf("Enter a string: ");
        if (fgets(Buffer, MAX_BUFFER - 1, stdin) == NULL)
            break;

		Buffer[MAX_BUFFER-1] = '\0';
		//
		// Delete the new-line character.
		//
		if(Buffer[strlen(Buffer) - 1] == '\n')
		{
			Buffer[strlen(Buffer) - 1] = '\0'; 
		}

        //
        // Set the message properties for sending messages.
        //
        cProps = 0;

        // Set the body of the message.
        amPropId[cProps]            = PROPID_M_BODY;
        aPropVar[cProps].vt         = VT_UI1 | VT_VECTOR;
        aPropVar[cProps].caub.cElems = sizeof(Buffer);
        aPropVar[cProps].caub.pElems = Buffer;
        cProps++;

        // Set the label of the message.
        amPropId[cProps]            = PROPID_M_LABEL;
        aPropVar[cProps].vt         = VT_LPWSTR;
        aPropVar[cProps].pwszVal    = wcsMsgLabel;
        cProps++;
        
        // Create a MSGPROPS structure.
        msgprops.cProp    = cProps;
        msgprops.aPropID  = amPropId;
        msgprops.aPropVar = aPropVar;
        msgprops.aStatus  = 0;


        //
        // Send the message.
        //
        hr = MQSendMessage(
                qhSend,     // IN: Queue handle
                &msgprops,  // IN: Message properties to send
                NULL);      // IN: Not part of a transaction

        if (FAILED(hr))
            Error("MQSendMessage failed", hr);

        //
        // Check for a request to end the application.
        //
        if (_stricmp(Buffer, "quit") == 0)
            break;

    } /* for */


    //
    // Close the queue handle.
    //
    MQCloseQueue(qhSend);
}



//------------------------------------------------------
//
// Sender function
//
//------------------------------------------------------
void Sender(int dDirectMode)
{

    if(dDirectMode == DIRECT)
        DirectSender();

    else
        StandardSender();
}


//-----------------------------------------------------
//
//  MAIN
//
//-----------------------------------------------------
int
main(int argc, char *argv[])
{
    DWORD dwNumChars;
    int dDirectMode;


    if (argc != 2)
    {
        Syntax();
    }

    //
    // Retrieve the computer name.
    //
    dwNumChars = MAX_COMPUTERNAME_LENGTH;
    if(!GetComputerName(mbsMachineName, &dwNumChars))
	{
		printf("Failed to get computer name. Exiting...\n");
		exit(1);
	}

    //
    // Detect a DS connection and determine the working mode.
    //
    
    dDirectMode = SetConnectionMode();

    if(strcmp(argv[1], "-s") == 0)
        Sender(dDirectMode);
    
    else if (strcmp(argv[1], "-r") == 0)
        Receiver(dDirectMode);

    else
        Syntax();


    printf("\nOK\n");

    return(0);
}


void Error(char *s, HRESULT hr)
{
    printf("%s (0x%X). Exiting...\n", s, hr);
    exit(1);
}


void Syntax()
{
    printf("\n");
    printf("Syntax: msmqtest -s | -r\n");
    printf("\t-s: Sender\n");
    printf("\t-r: Receiver\n");
    exit(1);
}
