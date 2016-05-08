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

#include "stdafx.h"
#include "..\QueueDef.h"
#import "mqoa.tlb"			no_namespace 			// MSMQ COM object
							


#ifdef _DEBUG
	#import "GraphObj.dll" \
							no_namespace			// Point and Line object

#else
	#import "GraphObj.dll" \
							no_namespace			// Point and Line object
                            
#endif



// Initialize OLE library
struct InitOle  _init_InitOle_;



void main()
{
    try {
		// For these ActiveX components we need only smart interface pointer
		IMSMQQueuePtr		pQueue;
		IMSMQMessagePtr		pMessage;

		// Instanciate the follwing ActiveX components
		IMSMQQueueInfoPtr	pQueueInfo(__uuidof(MSMQQueueInfo));

		int					n = 1;

		cout << "\n\nMSMQ IPersistStream Test Object Receiver\n";
		cout << "==========================================\n\n";
		

		// Open the queue
		pQueueInfo->PathName = ".\\IStreamTest";
		try {
			pQueue = pQueueInfo->Open(MQ_RECEIVE_ACCESS, MQ_DENY_NONE);
		} catch (_com_error& e) {
			if (e.Error() == MQ_ERROR_QUEUE_NOT_FOUND) {
				cout << "\n\nERROR queue does not exist\n";
				exit(-1);
			} else
				dump_com_error(e);
		}

		// Get the next message
		pMessage = pQueue->Receive();

		IPointPtr		pPoint((IUnknown*)pMessage->Body);
		
		// Check if it's a Point object
		if (pPoint != NULL) {

			cout << "\nGot a Point Object: " << pPoint->x << "   " << pPoint->y << "\n";
		
		}
		else {
			ILinePtr	pLine((IUnknown*)pMessage->Body);
			
			// Check if it's a Line object
			if (pLine != NULL) {
				cout << "\nGot a Line Object: " << pLine->x1 << " " << pLine->y1 << " " << pLine->x2 << " " << pLine->y2 << "\n";
			}
			else
				cout << "\nUnknown Object !!!\n";
		}
		pQueue->Close();


    } catch (_com_error& e) {
		dump_com_error(e);
    }
}

