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

            ////////////////////////////////////
            //                                //
            // Transaction Sample Application //
            //                                //
            ////////////////////////////////////


#define UNICODE                     // For all MSMQ applications

#include <stdio.h>


//------------------------------------------------------------------------------
// Include MS DTC-specific header files.

//------------------------------------------------------------------------------
#define INITGUID
#include <transact.h>

// Because we are compiling in UNICODE, there is a problem with DTC.
//#include <xolehlp.h>
extern HRESULT DtcGetTransactionManager(
                                    LPSTR  pszHost,
                                    LPSTR   pszTmName,
                                    REFIID rid,
                                    DWORD   dwReserved1,
                                    WORD    wcbReserved2,
                                    void FAR * pvReserved2,
                                    void** ppvObject )  ;

//------------------------------------------------------------------------------
// Include the ODBC-specific header file.
//------------------------------------------------------------------------------
#ifndef DBNTWIN32
#define DBNTWIN32

#include <SQLEXT.h>

// from  <odbcss.h>
#define SQL_COPT_SS_BASE               1200
#define SQL_COPT_SS_ENLIST_IN_DTC      (SQL_COPT_SS_BASE+7) // Enlist in a Viper transaction

// Define constant for use with SQL_ENLIST_IN_DTC.
#define SQL_DTC_DONE 0L       // Delimits end of Viper transaction

#endif

//--------------------------------------------------------------------------
//  Enable Ansi ODBC on VC5.
//--------------------------------------------------------------------------

#ifdef  SQLExecDirect
#undef  SQLExecDirect
#define SQLExecDirect SQLExecDirectA
#endif

#ifdef  SQLSetConnectOption
#undef  SQLSetConnectOption
#define SQLSetConnectOption  SQLSetConnectOptionA
#endif

#ifdef  SQLError
#undef  SQLError
#define SQLError  SQLErrorA
#endif

#ifdef  SQLConnect
#undef  SQLConnect
#define SQLConnect  SQLConnectA
#endif

//------------------------------------------------------------------------------
// Include the MSMQ-specific header file.
//------------------------------------------------------------------------------
#include "mq.h"

//------------------------------------------------------------------------------
// Define constants
//------------------------------------------------------------------------------
#define STR_LEN      40
#define MAX_VAR      20
#define MAX_FORMAT  100


//------------------------------------------------------------------------------
// Define datatypes
//------------------------------------------------------------------------------
typedef struct DBCONN
{
   char  pszSrv [STR_LEN];    // Data source name, configured through control panel
   char  pszUser [STR_LEN];   // Logon user name
   char  pszPasswd[STR_LEN];  // Logon user password
   HDBC  hdbc;                // Handle to an ODBC database connection
   HSTMT hstmt;               // ODBC statement handle, for use with SQLExecDirect

}  DBCONN;


//------------------------------------------------------------------------------
// Define globals
//------------------------------------------------------------------------------

// Global DB connection structure for the server
static DBCONN  gSrv =
   {  "MSMQDemo",
      "sa",
      "",
      SQL_NULL_HDBC,
      SQL_NULL_HSTMT
   };


// Service type GUID type for MQTransTest queues
static CLSID guidMQTransTestType =
{ 0xb856ab1, 0x16b6, 0x11d0, { 0x80, 0x48, 0x0, 0xa0, 0x24, 0x53, 0xc1, 0x6f } };


// Handle to ODBC environment
HENV  g_hEnv = SQL_NULL_HENV ;


// Buffer for computer name
WCHAR g_wszMachineName[ MAX_COMPUTERNAME_LENGTH + 1 ];


//--------------------------------------------------------------------------
// Forward declaration of routines used.
//--------------------------------------------------------------------------

void LogonToDB(DBCONN *ptr);
void ExecuteStatement(DBCONN *ptr, char* pszBuf, BOOL ProcessFlag);
BOOL ProcessRetCode(char*   pszFuncName,
                    DBCONN  *ptr,
                    RETCODE retcode,
                    BOOL    fExit = TRUE);
void DoSQLError(DBCONN *ptr);
void FreeODBCHandles(DBCONN *ptr);
void Error(char *s, HRESULT hr);
void Syntax();
void LocateTargetQueue(CLSID *pGuidType, WCHAR wsFormat[MAX_FORMAT]);
void PrepareSendMessageProperties(MSGPROPID     amPropId[MAX_VAR],
                          MQPROPVARIANT aPropVar[MAX_VAR],
                          MQMSGPROPS    &msgprops,
                          DWORD        &TransferSum);
void CreateQueue(CLSID *pGuidType, WCHAR wsFormat[]);
void GetMachineName();
void DisplayDollars (DBCONN *ptr, char *psAccount);


//------------------------------------------------------------------------------
// Sender Mode:
//
// The sender side does the following:
//   1. Creates the "SenderAccount" database.
//   2. Locates an MSMQ queue of type MQTransTest and opens it.
//       (NOTE: For simplicity, this sample assumes that there is only one queue of this type.)
//   3. In a loop:
//            Prompts the user to enter TransferSum.
//            Creates a transaction using MS DTC.
//            Within the transaction:
//                 Updates the "SenderAccount" database (subtracts TransferSum).
//                 Sends a message to the receiver side.
//            Commits the transaction.
//
//    4. Cleans up.
//
//
//
// The transaction in Sender mode includes two operations:
// (1) Updating the "SenderAccount" database (subtracting TransferSum).
// (2) Sending a message to the receiver side.
//------------------------------------------------------------------------------

void Sender()
{

   ITransactionDispenser   *pTransactionDispenser;
   ITransaction            *pTransaction;
   BOOL                    fTransactionCommitFlag;
                            // Used to decide whether to commit or abort

   HRESULT              hr;
   RETCODE              retcode;
   DWORD                dwTransferSum;              // Set by the user
   char                 sUserString[ STR_LEN ];
   char                 sSQLStatement[ STR_LEN*2 ];

   MQMSGPROPS           msgprops;
   MQPROPVARIANT        aPropVar[MAX_VAR];
   MSGPROPID            amPropId[MAX_VAR];
   WCHAR                wsFormat[MAX_FORMAT];
   QUEUEHANDLE          aqh;


    printf("\nSender Side.\n\n");

   //---------------------------------------------------------------------
   // Build the "SenderAccount" database (with the sum of $1000)
   //---------------------------------------------------------------------

   printf ("Building SenderAccount with the sum of $1000...\n");

   // Get the ODBC environment handle.
   retcode = SQLAllocEnv(&g_hEnv);

   ProcessRetCode("SQLAllocEnv",0, retcode);

   // Establish a connection to the database.
   LogonToDB(&gSrv);

   // Clear the database from the previous run.
   ExecuteStatement(&gSrv,"DROP TABLE SenderAccount",FALSE);

   // Create a new table in the database.
   ExecuteStatement(&gSrv,
    "CREATE TABLE SenderAccount (Rate INTEGER CONSTRAINT c1 CHECK (Rate>=0))",TRUE);

   // Insert new data into the database.
   ExecuteStatement(&gSrv,"INSERT INTO SenderAccount   VALUES(1000)",TRUE);

   printf ("OK.\n\n");

   //-----------------------------------------------------------------------
   // Locate the destination queue and open it for sending messages.
   //-----------------------------------------------------------------------

   printf ("Searching for the receiver queue...\n");

   // Locate the destination queue.
   LocateTargetQueue (&guidMQTransTestType, wsFormat);

   // Open the destination queue.
   hr = MQOpenQueue(wsFormat, MQ_SEND_ACCESS, 0, &aqh);

   if (FAILED(hr))
   {
      Error ("MQOpenQueue failed", hr);
   }

   //--------------------------------------------------------------------------
   // Get the transaction dispenser.
   //--------------------------------------------------------------------------

   // Obtain an interface pointer from the MS DTC proxy.
   hr = DtcGetTransactionManager(
               NULL,                        // pszHost
               NULL,                        // pszTmName
               IID_ITransactionDispenser,   // IID of interface
               0,                           // Reserved -- must be NULL
               0,                           // Reserved -- must be NULL
               0,                           // Reserved -- must be NULL
               (void **)&pTransactionDispenser  // Pointer to pointer to requested interface
                                 );

   if (FAILED(hr))
   {
      Error ("DTCGetTransactionManager failed", hr);
   }

   //--------------------------------------------------------------------
   // Sender's main loop
   //--------------------------------------------------------------------
   while (TRUE)
   {

		// Prompt user to enter a value for TransferSum.
		printf ("\n\nPlease enter the sum of dollars to transfer, or '0' to quit  ==> ");

		// Read the user input.
      
		if (fgets (sUserString, STR_LEN - 1, stdin) == NULL)
		{
			printf("\nInvalid input was entered. Exiting...\n");
			exit(1);
		}

		sUserString[STR_LEN - 1] = '\0';

		//
		// Remove the final new-line character introduced by fgets.
		//
		if(sUserString[strlen(sUserString) - 1] == '\n')
		{	  
			sUserString[strlen(sUserString) - 1] = '\0';
		}

      // Convert the user string to a DWORD
      dwTransferSum = atoi(sUserString);

      // Prepare message properties to sending a message.
      PrepareSendMessageProperties (amPropId,
                                    aPropVar,
                                    msgprops,
                                    dwTransferSum);

      //---------------------------------------------------------------------
      // Create a transaction (within the sender's main loop).
      //---------------------------------------------------------------------

      printf ("\nStarting a transaction...\n\n");

      // Initiate an MS DTC transaction.
      hr = pTransactionDispenser->BeginTransaction (
            0,                         // Must be null
            ISOLATIONLEVEL_ISOLATED,   // Isolation level
            ISOFLAG_RETAIN_DONTCARE,   // Isolation flags
            0,                         // Pointer to transaction options object
            &pTransaction);            // Pointer to pointer to transaction object

      if (FAILED(hr))
      {
         Error ("BeginTransaction failed",hr);
      }

      // The default is to commit the transaction.
      fTransactionCommitFlag = TRUE;

      //
      // SQL is a resource manager in the transaction.
      // It must be enlisted.
      //

      // Enlist the database in the transaction.
      retcode = SQLSetConnectOption (gSrv.hdbc,
                                     SQL_COPT_SS_ENLIST_IN_DTC,
                                     (UDWORD)pTransaction);

      if (retcode != SQL_SUCCESS)
      {
         ProcessRetCode("SQLSetConnection", &gSrv, retcode, FALSE);
         fTransactionCommitFlag = FALSE;
      }


      // Prepare an SQL statement for updating SenderAccount.
      sprintf_s (sSQLStatement, sizeof(sSQLStatement),
               "UPDATE SenderAccount  SET Rate = Rate - %lu", dwTransferSum) ;

      // Allocate a statement handle for use with SQLExecDirect.
      retcode = SQLAllocStmt(gSrv.hdbc, &gSrv.hstmt);

      if (retcode != SQL_SUCCESS)
      {
         ProcessRetCode("SQLAllocStmt", &gSrv, retcode, FALSE);
         fTransactionCommitFlag = FALSE;
      }

      // Update the database (subtract TransferSum from SenderAccount).
      retcode = SQLExecDirect (gSrv.hstmt,(UCHAR *) sSQLStatement, SQL_NTS);

      if (retcode != SQL_SUCCESS)
      {
         ProcessRetCode("SQLExecDirect", &gSrv, retcode, FALSE);
         fTransactionCommitFlag = FALSE;
      }

      // Free the statement handle.
      retcode = SQLFreeStmt(gSrv.hstmt, SQL_DROP);

      gSrv.hstmt = SQL_NULL_HSTMT;

      //
      // MSMQ is another resource manager in the transaction.
      // Its enlistment is implicit.
      //

      // Within the transaction, send a message to the receiver side.
      hr = MQSendMessage(aqh,              // Handle to destination queue
                         &msgprops,        // pointer to MQMSGPROPS structure
                         pTransaction);    // pointer to Transaction Object


      if (FAILED(hr))
      {
         printf("\nMQSendMessage() failed. Error code: %lxh\n", (DWORD) hr);
         fTransactionCommitFlag = FALSE;
      }


      // Commit the transaction.
      if (fTransactionCommitFlag)
      {
         printf ("Committing the transaction...\n");

         hr = pTransaction->Commit(0, 0, 0);

         if (FAILED(hr))
            printf ("The transaction was aborted.\n\n");
         else
            printf ("The transaction was committed successfully.\n\n");

      }
      else
      {
         printf ("Aborting the transaction...\n");

         hr = pTransaction->Abort(0, 0, 0);

         if (FAILED(hr))
            Error("The transaction was aborted", hr);
         else
            printf ("The transaction was aborted.\n\n");
      }

      // Release the transaction.
      pTransaction->Release();

      // End enlistment of the database.
      retcode = SQLSetConnectOption (gSrv.hdbc, SQL_COPT_SS_ENLIST_IN_DTC, SQL_DTC_DONE);

      ProcessRetCode ("SQLSetConnectOption", &gSrv, retcode);

      // Display the sum of dollars in SenderAccount.
      DisplayDollars (&gSrv, "SenderAccount");

      // Quit the loop when nothing was transferred.
      if (dwTransferSum == 0)
         break;
   }

   //--------------------------------------------------------------------------
   // Cleanup
   //--------------------------------------------------------------------------

   // Release the transaction dispenser.
   pTransactionDispenser->Release();


   // Free the database.
   ExecuteStatement(&gSrv,"DROP TABLE SenderAccount",TRUE);


   // Free the ODBC handle.
   FreeODBCHandles(&gSrv);


   // Free the ODBC environment handle.
   retcode = SQLFreeEnv(g_hEnv);

   if (retcode == SQL_ERROR)
      Error ("SQL FreeEnv ",0);


   // Free the MSMQ queue handle.
   MQCloseQueue(aqh);


   printf ("\n\nThe sender side completed.\n\n");

}




//------------------------------------------------------------------------------
// Receiver Mode:
//
// The receiver side does the following:
//    1. Creates the "ReceiverAccount" database.
//    2. Creates an MSMQ transactional public queue of type
//       MQTransTest locally and opens it.
//    3. In a loop:
//            Creates a transaction using MS DTC.
//            Within the transaction:
//                 Receives a message from the queue (containing the value of TransferSum).
//                 Updates the "ReceiverAccount" database (adds TransferSum).
//            Commits the transaction.
//    4. Cleans up.
//
//
//
// The transaction in the receiver mode includes two operations:
// (1) Receiving a message (sent by the sender side) from the queue.
// (2) Updating the "ReceiverAccount" database  (adding TransferSum).
//------------------------------------------------------------------------------

void Receiver()
{
   MSGPROPID            amPropId[MAX_VAR];
   MQMSGPROPS           msgprops;
   MQPROPVARIANT        aPropVar[MAX_VAR];
   DWORD             cProps;
   HRESULT              hr;
   WCHAR             wsFormat[MAX_FORMAT];
   QUEUEHANDLE          aqh;

   ITransactionDispenser   *pTransactionDispenser;
   ITransaction         *pTransaction;
   BOOL              TransactionCommitFlag;  // Used to decide whether to commit or abort

   RETCODE              retcode;
   DWORD             TransferSum;

   DWORD             MessageBuffer;       // Message body containing TransferSum
   char              sSQLStatement[STR_LEN*2];





   printf ("\nReceiver Side.\n\n");

   //-----------------------------------------------------------------------
   // Build a "ReceiverAccount" database (with the sum of $500).
   //-----------------------------------------------------------------------

   printf ("Building a ReceiverAccount with a sum of $500...\n");

   // Get ODBC environment handle.
   retcode = SQLAllocEnv(&g_hEnv);

   ProcessRetCode("SQLAllocEnv",0, retcode);

   // Establish a connection to the database.
   LogonToDB(&gSrv);

   // Clear the table from the previous run.
   ExecuteStatement(&gSrv,"DROP TABLE ReceiverAccount",FALSE);

   // Create a new table.
   ExecuteStatement(&gSrv,"CREATE TABLE ReceiverAccount (Rate INTEGER CONSTRAINT c2 CHECK (Rate>0))",TRUE);

   // Insert new data into the table.
   ExecuteStatement(&gSrv,"INSERT INTO ReceiverAccount  VALUES(500)",TRUE);

   printf ("OK.\n\n");

   //-----------------------------------------------------------------------
   // Create queue and open it for receiving messages.
   //-----------------------------------------------------------------------

   printf ("Creating a receiver queue...\n");

   // Create the queue.
   CreateQueue (&guidMQTransTestType, wsFormat);

   // Prepare message properties for reading messages.
   cProps = 0;

   amPropId[cProps] =             PROPID_M_BODY;

   aPropVar[cProps].vt =          VT_UI1 | VT_VECTOR;
   aPropVar[cProps].caub.cElems =  sizeof(MessageBuffer);
   aPropVar[cProps].caub.pElems =  (unsigned char *)&MessageBuffer;
   cProps++;

   // Create a MSGPROPS structure.
   msgprops.cProp =    cProps;
   msgprops.aPropID =  amPropId;
   msgprops.aPropVar = aPropVar;
   msgprops.aStatus =  0;

   // Open the queue.
   hr = MQOpenQueue(wsFormat, MQ_RECEIVE_ACCESS, 0, &aqh);

   //
   // Things are a little bit tricky. MQCreateQueue succeeded, but in the 
   // case of a public queue, this does not mean that MQOpenQueue
   // will succeed, because replication delays are possible. The queue is
   // registered in the DS, but it might take a replication interval
   // until the replica reaches the server that I am connected to.
   // To overcome this, open the queue in a loop.
   //
   if (hr == MQ_ERROR_QUEUE_NOT_FOUND)
   {
       int iCount = 0 ;
       while((hr == MQ_ERROR_QUEUE_NOT_FOUND) && (iCount < 120))
       {
          printf(".");

          // Wait a bit.
          iCount++ ;
          Sleep(500);

          // Retry.
          hr = MQOpenQueue(wsFormat, MQ_RECEIVE_ACCESS, 0, &aqh);
       }
   }

   if (FAILED(hr))
   {
      Error ("MQOpenQueue failed", hr);
   }

   printf("OK.");


   //--------------------------------------------------------------------------
   // Get the transaction dispenser.
   //--------------------------------------------------------------------------

   // Obtain an interface pointer from the MS DTC proxy.
   hr = DtcGetTransactionManager(
         NULL, NULL, // pszHost, pszTmName
         IID_ITransactionDispenser,         // IID of the requested interface
         0,0,0,                             // Reserved -- must be NULL
         (void **)&pTransactionDispenser);  // Pointer to pointer to the requested interface


   if (FAILED(hr))
      Error ("DTCGetTransactionManager failed", hr);


   //--------------------------------------------------------------------------
   // Receiver Main Loop
   //--------------------------------------------------------------------------
   while (TRUE)
   {

      printf ("\n\nWaiting for a message to come...\n");

      // Peek outside the transaction to avoid database lock
      // for a long/infinite period.
      //
      //dwSize = sizeof(wsResponse);
      hr = MQReceiveMessage(
                    aqh,                     // Handle to queue
                    INFINITE,                // Time-out
                    MQ_ACTION_PEEK_CURRENT,  // Peek action
                    &msgprops,               // Message properties
                    NULL,                    // No OVERLAPPED structure
                    NULL,                    // No callback function
                    NULL,                    // No cursor
                    NULL                     // No transaction yet
                           );

      if (FAILED(hr))
         Error("MQReceiveMessage (PEEKING) failed", hr);



      //--------------------------------------------------------------------------
      // Create a transaction.
      //--------------------------------------------------------------------------
      printf ("\n\nStarting a transaction...\n\n");


      // Initiate an MS DTC transaction.
      hr = pTransactionDispenser->BeginTransaction (
            0,                               // Must be NULL
            ISOLATIONLEVEL_ISOLATED,         // Isolation level
            ISOFLAG_RETAIN_DONTCARE,         // Isolation flags
            0,                               // Pointer to transaction options object
            &pTransaction);                  // Pointer to pointer to transaction object

      if (FAILED(hr))
         Error ("BeginTransaction failed", hr);


      // The default is to commit the transaction.
      TransactionCommitFlag = TRUE;

      //
      // SQL is a resource manager in the transaction.
      // It must be enlisted.
      //

      // Enlist the database in the transaction.
      retcode = SQLSetConnectOption (gSrv.hdbc, SQL_COPT_SS_ENLIST_IN_DTC, (UDWORD)pTransaction);

      if (retcode != SQL_SUCCESS)
         TransactionCommitFlag = FALSE;



      // Receive a message from the queue
      //dwSize = sizeof(wsResponse);
      hr = MQReceiveMessage(
            aqh,                              // Handle to queue
            INFINITE,                         // Time-out interval
            MQ_ACTION_RECEIVE,                // Receive action
            &msgprops,                        // Message properties
            NULL,NULL,NULL,                   // No OVERLAPPED structure, callback function, or cursor
            pTransaction);                    // Pointer to transaction object

      if (FAILED(hr))
         TransactionCommitFlag = FALSE;


      // The message buffer holds TransferSum.
      TransferSum = (DWORD)MessageBuffer;


      // Prepare an SQL statement to update ReceiverAccount.
      sprintf_s(sSQLStatement, sizeof(sSQLStatement), "UPDATE ReceiverAccount   SET Rate = Rate + %i", TransferSum);


      // Allocate a statement handle for use with SQLExecDirect.
      retcode = SQLAllocStmt(gSrv.hdbc,&gSrv.hstmt);

      if (retcode != SQL_SUCCESS)
         TransactionCommitFlag = FALSE;


      // Update the database (add TransferSum to ReceiverAccount).
      retcode = SQLExecDirect (gSrv.hstmt,(UCHAR *) sSQLStatement, SQL_NTS);

      if (retcode != SQL_SUCCESS)
         TransactionCommitFlag = FALSE;


      // Free the statement handle.
      retcode = SQLFreeStmt(gSrv.hstmt, SQL_DROP);

      gSrv.hstmt = SQL_NULL_HSTMT;



      // Commit the transaction.
      if (TransactionCommitFlag)
      {
         printf ("Committing the transaction...   ");

         hr = pTransaction->Commit(0, 0, 0);

         if (FAILED(hr))
            printf ("Failed... The transaction was aborted.\n\n");
         else
            printf ("The transaction was committed successfully.\n\n");

      }


      // Abort the transaction.
      else
      {
         printf ("Aborting the transaction...\n");

         hr = pTransaction->Abort(0, 0, 0);

         if (FAILED(hr))
            Error("The transaction was aborted", hr);
         else
            printf ("The transaction was aborted.\n\n");

      }



      // Release the transaction.
      pTransaction->Release();


      // End the enlistment of the database.
      retcode = SQLSetConnectOption (gSrv.hdbc, SQL_COPT_SS_ENLIST_IN_DTC, SQL_DTC_DONE);

      ProcessRetCode ("SQLSetConnectOption", &gSrv, retcode);


      // Display the number of dollars in ReceiverAccount.
      DisplayDollars (&gSrv, "ReceiverAccount");


      // Decide whether to continue the loop.
      if (TransferSum == 0)
         break;


   }


   //--------------------------------------------------------------------------
   // Cleanup
   //--------------------------------------------------------------------------

   // Release the transaction dispenser.
   pTransactionDispenser->Release();


   // Free the database.
   ExecuteStatement(&gSrv,"DROP TABLE ReceiverAccount",TRUE);


   // Free the ODBC handle.
   FreeODBCHandles(&gSrv);


   // Free the ODBC environment handle.
   retcode = SQLFreeEnv(g_hEnv);

   if (retcode == SQL_ERROR)
      Error ("SQL FreeEnv ",0);


   // Free the queue handle.
    MQCloseQueue(aqh);


   // Delete the queue from the directory service.
   MQDeleteQueue(wsFormat);


   printf ("\n\nReceiver side completed.\n\n");
}

/*
//-----------------------------------------------------
// Check if teh local computer is enabled to access the 
// directory service (DS-enabled).
//----------------------------------------------------- 
bool DetectDsConnection(void)
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
        Error("A DS connection cannot be detected", hr);
	   
    return PrivateProps.aPropVar[0].boolVal;
}
*/
BOOL IsDsEnabledLocaly()
/*++

Routine Description:
    
      The routine checks whether the local computer operates in
      domain (DS-enabled) mode or in workgroup (DS-disabled) mode.

Arguments:
    
      None

Return Value:
    
      TRUE     -  domain (DS-enabled) mode.
      FALSE    -  workgroup (DS-disabled) mode.

--*/

{
       
    MQPRIVATEPROPS PrivateProps;
    QMPROPID       aPropId[MAX_VAR];
    MQPROPVARIANT  aPropVar[MAX_VAR];
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
    //
    // Create a PRIVATEPROPS structure.
    //
    PrivateProps.cProp = cProp;
	PrivateProps.aPropID = aPropId;
	PrivateProps.aPropVar = aPropVar;
    PrivateProps.aStatus = NULL;

    //
    // Retrieve the information.
    //
    

    //
    // This code is used to detect a DS connection.
    // This code is designed to allow compilation both on 
    // Windows NT 4.0 and on Windows 2000 and later.
    //
    HINSTANCE hMqrtLibrary = GetModuleHandle(TEXT("mqrt.dll"));
	if(hMqrtLibrary == NULL)
	{
		printf("An incomplete installation of MSMQ was detected. Exiting...\n");
		exit(1);
	}

    typedef HRESULT (APIENTRY *MQGetPrivateComputerInformation_ROUTINE)(LPCWSTR , MQPRIVATEPROPS*);
    MQGetPrivateComputerInformation_ROUTINE pfMQGetPrivateComputerInformation = 
          (MQGetPrivateComputerInformation_ROUTINE)GetProcAddress(hMqrtLibrary,
													 "MQGetPrivateComputerInformation");
    if(pfMQGetPrivateComputerInformation == NULL)
    {
        //
        // There is no entry point in the DLL that matches this routine.
        // It must be an old version of mqrt.dll (MSMQ 1.0).
        // It will be OK to handle this case as the case of DS-enabled computer.
        //
        return TRUE;
    }

	hr = pfMQGetPrivateComputerInformation(
				     NULL,
					 &PrivateProps);
	if(FAILED(hr))
	{
        //
        // We were not able to determine whether the computer is DS-enabled or DS-disabled.
        // Notify the user and assume the worst case (i.e., the computer is DS-disasbled).
        //
        Error("No DS connection could be detected", hr);
        return FALSE;
    }                             
	
    
    if(PrivateProps.aPropVar[0].boolVal == 0)
    {
        //
        // DS-disabled.
        //
        return FALSE;
    }

    return TRUE;

}



//------------------------------------------------------------------------------
//  MAIN
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   DWORD dwSize;

    if(argc != 2)
        Syntax();

    // Fail if the local computer is DS-disabled.
    if(!IsDsEnabledLocaly())
    {
        printf("We cannot work on a DS-disabled computer.\nExiting...");
        exit(1);
    }


    // Retrieve computer name.
    dwSize = MAX_COMPUTERNAME_LENGTH;
	if(!GetComputerName(g_wszMachineName, &dwSize))
	{
		printf("The computer name cannot be retrieved.\nExiting...");
        exit(1);
	}

    if(strcmp(argv[1], "-s") == 0)
        Sender();

    else if(strcmp(argv[1], "-r") == 0)
        Receiver();

    else
        Syntax();

    return(1);

}



//------------------------------------------------------------------------------
// Subroutines
//------------------------------------------------------------------------------

void Error(char *s, HRESULT hr)
{

    printf("\n\n%s (0x%X). Exiting...\n", s, hr);
    exit(1);
}

//------------------------------------------------------------------------------

void Syntax()
{
    printf("\n");
    printf("Syntax: msmqtrans -s | -r\n");
    printf("\t-s - Sender Side\n");
    printf("\t-r - Receiver Side\n");
    exit(1);

}

//------------------------------------------------------------------------------

void LocateTargetQueue (CLSID *pGuidType, WCHAR wsFormat[MAX_FORMAT])
{

   DWORD      dwSize;
   DWORD      i;

   DWORD      cQueue;
   DWORD      cProps;
   HRESULT       hr;
   MQPROPERTYRESTRICTION   aPropRestriction[MAX_VAR];
   MQRESTRICTION        Restriction;
   MQCOLUMNSET          Column;
   QUEUEPROPID          aqPropId[MAX_VAR];
   HANDLE               hEnum;
   MQPROPVARIANT        aPropVar[MAX_VAR];

    //
    // Prepare parameters for locating queues.
    //
    // 
    // 1. Restriction: Queues with PROPID_Q_TYPE = MSMQTransTest service type GUIID
    //
    cProps = 0;

    aPropRestriction[cProps].rel = PREQ;
    aPropRestriction[cProps].prop = PROPID_Q_TYPE;
    aPropRestriction[cProps].prval.vt = VT_CLSID;
    aPropRestriction[cProps].prval.puuid = pGuidType;
    cProps++;

    Restriction.cRes = cProps;
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
    hr = MQLocateBegin(NULL, &Restriction, &Column, NULL, &hEnum);

   if (FAILED(hr))
      Error ("MQLocateBegin failed", hr);


    //
    // Get the results.
    //
    cQueue = MAX_VAR;
    hr = MQLocateNext(hEnum, &cQueue, aPropVar);

   if (FAILED(hr))
      Error ("MQLocateNext failed", hr);

    hr = MQLocateEnd(hEnum);

    if(cQueue == 0)
    {
        // No queue could be found, so exit
        printf("No queue was found. Exiting...\n\n");
        exit(0);
    }


    printf("FOUND %d.\n", cQueue);

    dwSize = sizeof(WCHAR)*MAX_FORMAT;

   // Transform the queue GUID into a format name.
   hr = MQInstanceToFormatName(aPropVar[0].puuid, wsFormat, &dwSize);

   if (FAILED(hr))
      Error ("MQInstancetoFormatName failed", hr);


   // Free the memory that was allocated for the queue GUID during the query.
    for(i = 0; i < cQueue; i++)
      MQFreeMemory(aPropVar[i].puuid);


}


//------------------------------------------------------------------------------

void PrepareSendMessageProperties (MSGPROPID     amPropId[MAX_VAR],
                           MQPROPVARIANT aPropVar[MAX_VAR],
                           MQMSGPROPS    &msgprops,
                           DWORD     &TransferSum)
{

   DWORD      cProps;

    cProps = 0;
    amPropId[cProps] =             PROPID_M_BODY;
    aPropVar[cProps].vt =          VT_UI1 | VT_VECTOR;
    aPropVar[cProps].caub.cElems =  sizeof(TransferSum);
    aPropVar[cProps].caub.pElems =  (unsigned char *)&TransferSum;
    cProps++;

    // Create a MSGPROPS structure.
    msgprops.cProp =    cProps;
    msgprops.aPropID =  amPropId;
    msgprops.aPropVar = aPropVar;
    msgprops.aStatus =  0;

}

//--------------------------------------------------------------------------

void  CreateQueue (CLSID *pGuidType, WCHAR wsFormat[])
{
   QUEUEPROPID       aqPropId[MAX_VAR];
   WCHAR             wsPathName[1000];  // Long path name
   MQPROPVARIANT     aPropVar[MAX_VAR];
   DWORD             cProps;
   MQQUEUEPROPS      qprops;
   DWORD             dwSize;
   HRESULT           hr;

   //---------------------------------------------------------------------
   // Prepare properties to create a queue on the local computer.
   //---------------------------------------------------------------------
   cProps = 0;

   // Set the path name property.
   aqPropId[cProps] =          PROPID_Q_PATHNAME;

   swprintf_s(wsPathName, sizeof(wsPathName)/sizeof(wsPathName[0]), TEXT("%s\\MSMQDemo"), g_wszMachineName);
   aPropVar[cProps].vt =       VT_LPWSTR;
   aPropVar[cProps].pwszVal =  wsPathName;
   cProps++;

   // Set the transactional level of the queue to transactional.
   aqPropId[cProps] =        PROPID_Q_TRANSACTION;

   aPropVar[cProps].vt =     VT_UI1;
   aPropVar[cProps].bVal =   MQ_TRANSACTIONAL;
   cProps++;

   // Set the service type GUID of the queue (used to locate queues in a query).
   aqPropId[cProps] =        PROPID_Q_TYPE;

   aPropVar[cProps].vt =     VT_CLSID;
   aPropVar[cProps].puuid =  pGuidType;
   cProps++;

   // Create a QUEUEPROPS structure.
   qprops.cProp =    cProps;
   qprops.aPropID =  aqPropId;
   qprops.aPropVar = aPropVar;
   qprops.aStatus =  0;

   //-----------------------------------------------------------------------
   // Create the queue.
   //-----------------------------------------------------------------------
   dwSize = sizeof(WCHAR)*MAX_FORMAT;
   hr = MQCreateQueue(NULL, &qprops, wsFormat, &dwSize);

   if(FAILED(hr))
   {
      // The API failed, but not because the queue exists.
      if(hr != MQ_ERROR_QUEUE_EXISTS)
            Error("MQCreateQueue failed", hr);

      // The queue exists, so get its format name.
      // Note: Because the queue already exists, this sample assumes
      // that it was created earlier by this program, so we do not 
      // check whether the queue is transactional. If at this point the
      // queue is not transactional, the transactions will abort later.
      //
      hr = MQPathNameToFormatName(wsPathName, wsFormat, &dwSize);

      if (FAILED(hr))
         Error ("The format name cannot be retrieved", hr);
   }
}

//-------------------------------------------------------------------------------

void LogonToDB(DBCONN *ptr)
{
   RETCODE retcode = 0;

   retcode = SQLAllocConnect(g_hEnv, &(ptr->hdbc) );

   if (ProcessRetCode("SQLAllocConnect",ptr,retcode))
   {
      retcode = SQLConnect(ptr->hdbc,
                  (UCHAR *)(ptr->pszSrv),
                  SQL_NTS,
                  (UCHAR *)(ptr->pszUser),
                  SQL_NTS,
                  (UCHAR *)(ptr->pszPasswd),
                  SQL_NTS
                  );

      ProcessRetCode("SQLConnect",ptr,retcode);
   }
}

//------------------------------------------------------------------------------

void ExecuteStatement(DBCONN *ptr, char* pszBuf,BOOL ProcessFlag)
{
   RETCODE retcode = 0;

   // Allocate a statement handle for use with SQLExecDirect.
   retcode = SQLAllocStmt(ptr->hdbc,&(ptr->hstmt));

   if (ProcessFlag)
      ProcessRetCode("SQLAllocStmt",ptr,retcode);

   // Execute the string passed as a SQL statement.
    retcode = SQLExecDirect (ptr->hstmt,(UCHAR *) pszBuf,SQL_NTS);

   if (ProcessFlag)
      ProcessRetCode("SQLExecDirect",ptr,retcode);

   // Free the statement handle.
   retcode = SQLFreeStmt(ptr->hstmt, SQL_DROP);
   ptr->hstmt = SQL_NULL_HSTMT;

   if (ProcessFlag)
      ProcessRetCode("SQLFreeStmt",ptr,retcode);

}

// ---------------------------------------------------------------------------

void DisplayDollars (DBCONN *ptr, char *psAccount)
{

   DWORD             DollarsSum;                  // IN SQL database
   SDWORD               cbValue;                  // OUT argument for SQL query
   char              sSQLStatement[STR_LEN*2];
   RETCODE              retcode;




   // Allocate a statement handle for use with SQLExecDirect.
   retcode = SQLAllocStmt(ptr->hdbc,&(ptr->hstmt));

   ProcessRetCode("SQLAllocStmt",ptr,retcode);


   // Prepare SQL statement for issuing a query.
   int n = _snprintf_s(sSQLStatement, sizeof(sSQLStatement), STR_LEN*2 -1, "SELECT * FROM %s", psAccount);
   if(n < 0)
   {
		printf("The string is too long for the buffer specified. Exiting...\n");
		exit(1);
   }	
   sSQLStatement[n] = L'\0';



   // Issue the SQL query.
   retcode = SQLExecDirect (ptr->hstmt,(UCHAR *)sSQLStatement,SQL_NTS);

   ProcessRetCode ("SQLExecDirect",ptr,retcode);


   // Prepare a data structure for retrieving the query results.
   retcode = SQLBindCol(ptr->hstmt,1,SQL_C_ULONG,&DollarsSum,0,(SQLLEN *)&cbValue);

   ProcessRetCode ("SQLBindCol",ptr,retcode);


   // Retrieve the query results.
   retcode = SQLFetch (ptr->hstmt);

   ProcessRetCode ("SQLFetch",ptr,retcode);


   // Display the query results.
   printf ("Sum of dollars in %s is %d .\n\n",psAccount,DollarsSum);


   // Free the statement handle.
   retcode = SQLFreeStmt(ptr->hstmt, SQL_DROP);
   ptr->hstmt = SQL_NULL_HSTMT;

   ProcessRetCode("SQLFreeStmt",ptr,retcode);

}

// ---------------------------------------------------------------------------

void FreeODBCHandles(DBCONN *ptr)
{
   SQLDisconnect(ptr->hdbc);

   SQLFreeConnect(ptr->hdbc);

   ptr->hdbc   = SQL_NULL_HDBC;
   ptr->hstmt  = SQL_NULL_HSTMT;
}


// ---------------------------------------------------------------------------

BOOL ProcessRetCode(char*   pszFuncName,
                    DBCONN  *ptr,
                    RETCODE retcode,
                    BOOL    fExit)
{
   BOOL state = TRUE ;
   BOOL fExitP = fExit ;

   switch (retcode)
   {

   case SQL_SUCCESS:
         fExitP = FALSE ;
         break;

   case SQL_SUCCESS_WITH_INFO:
         fExitP = FALSE ;
         break;

   case SQL_ERROR:
         printf("%s failed - see more info\n",pszFuncName);
         DoSQLError(ptr);
         state = FALSE;
         break;

   case SQL_INVALID_HANDLE:
         printf("%s failed - SQL_INVALID_HANDLE\n",pszFuncName);
         state = FALSE;
         break;

   case SQL_NO_DATA_FOUND:
         printf("%s failed - SQL_NO_DATA_FOUND\n",pszFuncName);
         fExitP = FALSE ;
         state = FALSE;
         break;

   case SQL_STILL_EXECUTING:
         printf("%s failed - SQL_STILL_EXECUTING\n",pszFuncName);
         fExitP = FALSE ;
         state = FALSE;
         break;

   case SQL_NEED_DATA:
         printf("%s failed - SQL_NEED_DATA\n",pszFuncName);
         fExitP = FALSE ;
         state = FALSE;
         break;

   default:
         printf("%s failed - unexpected error, retcode = %x\n",pszFuncName,retcode);
         DoSQLError(ptr);
         state = FALSE;
         break;
   }

   if (fExitP)
   {
      exit(-1) ;
   }
   return state ;
}

// ---------------------------------------------------------------------------

void DoSQLError(DBCONN *ptr)
{
   UCHAR                szSqlState[6];
   UCHAR             szErrorMsg[SQL_MAX_MESSAGE_LENGTH];

   SQLINTEGER  fNativeError   = 0;
   SWORD    cbErrorMsg = 0;
   RETCODE     retcode;

   retcode = SQLError(g_hEnv,
              ptr ? ptr->hdbc : 0,
              ptr ? ptr->hstmt :0,
              szSqlState,
              &fNativeError,
              szErrorMsg,
              sizeof(szErrorMsg),
              &cbErrorMsg
              );

   if (retcode != SQL_NO_DATA_FOUND && retcode != SQL_ERROR)
   {
      if (fNativeError != 0x1645)   // ignore change database to master context message
      {
         printf("SQLError info:\n");
         printf("SqlState: %s, fNativeError: %x\n",szSqlState,fNativeError);
         printf("Error Message: %s\n\n",szErrorMsg);
      }
   }
   else
   {
      printf("SQLError() failed: %x, NO_DATA_FOUND OR SQL_ERROR\n",retcode);
   }

}
// ---------------------------------------------------------------------------

