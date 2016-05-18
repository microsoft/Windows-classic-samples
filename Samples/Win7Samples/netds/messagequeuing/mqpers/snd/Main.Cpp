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

	#import "GraphObj.dll" \
							no_namespace				// Point and Line object




// Initialize OLE library
struct InitOle  _init_InitOle_;

void main()
{
    try {

		// For these ActiveX components we need only smart interface pointer
		IMSMQQueueInfosPtr	pQueueInfos; 
		IMSMQQueueInfoPtr	pQueueInfo; 
		IMSMQQueuePtr		pQueue;
		IUnknownPtr			pIUnknown;

		// Instanciate the follwing ActiveX components
		IMSMQQueryPtr		pQuery(__uuidof(MSMQQuery));
		IMSMQMessagePtr		pMessage(__uuidof(MSMQMessage));
		ILinePtr			pLine(__uuidof(Line));
		IPointPtr			pPoint(__uuidof(PointGO));

		int					n = 1;
		_variant_t			varQueueNameInit(STR_QUEUE_LABEL);
		VARIANT				varQueueName;

		cout << "\n\nMSMQ IPersistStream Test\n";
		cout << "==========================\n\n";

		varQueueName = varQueueNameInit.Detach();
		
		// Find the queue
		pQueueInfos = pQuery->LookupQueue(&vtMissing, &vtMissing, &varQueueName);
		varQueueNameInit.Attach(varQueueName);
		pQueueInfos->Reset();

		if ((pQueueInfo = pQueueInfos->Next()) == NULL) {

			//  The queue was not found create it on the local system
			pQueueInfo = new IMSMQQueueInfoPtr( __uuidof(MSMQQueueInfo) );
			pQueueInfo->PathName = STR_QUEUE_NAME;
			pQueueInfo->Label = STR_QUEUE_LABEL;
			pQueueInfo->Create();
			cout << "\nNew Queue created\n";
		}

		cout << "Which object should be send ?\n";
		cout << "1 - Point\n";
		cout << "2 - Line\n";
		cout << ":";
		cin >> n;


		// Open the queue
		pQueue = pQueueInfo->Open(MQ_SEND_ACCESS, MQ_DENY_NONE);

		if (n == 1) {
			// Initialize point object
			pPoint->x = 8;
			pPoint->y = 9;
			pMessage->Body = static_cast<IUnknown*>(pPoint); // The message body gets the IUnknown pointer

		}
		else {
			// Initialize line object
			pLine->x1 = 1;
			pLine->y1 = 2;
			pLine->x2 = 3;
			pLine->y2 = 4;
			pMessage->Body = static_cast<IUnknown*>(pLine); // The message body gets the IUnknown pointer
		}
		cout << "Sending Object\n";
		pMessage->Send(pQueue);

		// Close the Queue
		pQueue->Close();

    } catch (_com_error& e) {
		dump_com_error(e);
    }
}

