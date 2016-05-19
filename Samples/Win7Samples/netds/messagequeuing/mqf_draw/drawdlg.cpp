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
// drawdlg.cpp : implementation file
//


#include "stdafx.h"
#include "mqfdraw.h"
#include "drawdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FORMAT_NAME_LEN 256
#define FORMAT_NAME_LIST_LEN 1026


//
// Define the distributed draw queue type.
//
CLSID guidDrawType =
{ 0x151ceac0, 0xacb5, 0x11cf, { 0x8b, 0x51, 0x00, 0x20, 0xaf, 0x92, 0x95, 0x46 } };

//
//Title
//
CString strTitle;

/////////////////////////////////////////////////////////////////////////////
// CDisdrawDlg dialog box

CDisdrawDlg::CDisdrawDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisdrawDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisdrawDlg)
	m_strFriend = _T("");
	m_iDelivery = 0;
  	m_iType = 0;
	m_strRemoteComputerName = _T("");
	m_iRadioDS = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//}}AFX_DATA_INIT
	m_fDsEnabledLocaly = 0;
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32.
}

void CDisdrawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisdrawDlg)
	DDX_Control(pDX, IDC_FRAME_FRIEND, m_cFriendFrame);
	DDX_Control(pDX, IDC_STATIC_C_LABEL, m_cComputerLabel);
 	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_STATIC_Q_LABEL, m_cQueueLabel);
	DDX_Control(pDX, IDC_STATIC_CHOOSE_MESSAGE, m_cMessageFrame);
	DDX_Control(pDX, IDC_STATIC_CHOOSE_QUEUE, m_cQueueFrame);
	DDX_Control(pDX, IDC_RADIO_EXPRESS, m_cRadioExpress);
	DDX_Control(pDX, IDC_EDIT_FRIEND_COMPUTER, m_cComputerInput);
	DDX_Control(pDX, IDC_EDIT_FRIEND, m_cQueueInput);
	DDX_Control(pDX, IDC_DRAWAREA_SCRIBLLE, m_drawScribble);
	DDX_Control(pDX, IDC_BUTTON_ATTACH, m_btnAttach);
	DDX_Text(pDX, IDC_EDIT_FRIEND, m_strFriend);
	DDV_MaxChars(pDX, m_strFriend, 50);
	DDX_Radio(pDX, IDC_RADIO_EXPRESS, m_iDelivery);
 	DDX_Radio(pDX, IDC_RADIO_PUBLICQUEUE, m_iType);
	DDX_Text(pDX, IDC_EDIT_FRIEND_COMPUTER, m_strRemoteComputerName);
	DDV_MaxChars(pDX, m_strRemoteComputerName, 50);
    DDX_Control(pDX, IDC_RADIO_RECOVERABLE, m_cRadioRecoverable);
	DDX_Control(pDX, IDC_RADIO_PUBLICQUEUE, m_cRadioPublic);
	DDX_Control(pDX, IDC_RADIO_PRIVATEQUEUE, m_cRadioPrivate);
	DDX_Control(pDX, IDC_BUTTON_ADDQUEUE, m_btnAddQueue);
	DDX_Control(pDX, IDC_CONNECTION, m_cConnectionMode);
	//}}AFX_DATA_MAP
    

}

BEGIN_MESSAGE_MAP(CDisdrawDlg, CDialog)
	//{{AFX_MSG_MAP(CDisdrawDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ATTACH, OnButtonAttach)
	ON_BN_CLICKED(IDC_BUTTON_ADDQUEUE, OnButtonAddQueue)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_PRIVATEQUEUE, OnRadioPrivatequeue)
	ON_BN_CLICKED(IDC_RADIO_PUBLICQUEUE, OnRadioPublicqueue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisdrawDlg message handle


BOOL LocateQueue(CString m_strLabel, WCHAR *wcsFormatName, DWORD dwNumChars)
{
	//
	// Set restrictions to locate a draw queue with the specified label.
	//
	DWORD cProps = 0;
	MQPROPERTYRESTRICTION aPropRestriction[2];
	MQRESTRICTION Restriction;

	aPropRestriction[cProps].rel           = PREQ;
	aPropRestriction[cProps].prop          = PROPID_Q_TYPE;
	aPropRestriction[cProps].prval.vt      = VT_CLSID;
	aPropRestriction[cProps].prval.puuid   = &guidDrawType;
	cProps++;

	WCHAR wcsLabel[MQ_MAX_Q_LABEL_LEN+1];
        size_t  nResultedStringSize=0;
	mbstowcs_s(
            &nResultedStringSize, 
            wcsLabel, 
            sizeof(wcsLabel)/sizeof(wcsLabel[0]), 
            m_strLabel, 
            sizeof(wcsLabel)/sizeof(wcsLabel[0])-1
            );
	aPropRestriction[cProps].rel           = PREQ;
	aPropRestriction[cProps].prop          = PROPID_Q_LABEL;
	aPropRestriction[cProps].prval.vt      = VT_LPWSTR;
	aPropRestriction[cProps].prval.pwszVal = wcsLabel;
	cProps++;

	Restriction.cRes = cProps;
	Restriction.paPropRes = aPropRestriction;


	//
	// Retrieve the GUID of the specified queue.
	//
	cProps = 0;
	QUEUEPROPID aPropId[1];
	MQCOLUMNSET Column;
	
	aPropId[cProps] = PROPID_Q_INSTANCE;
	cProps++;

	Column.cCol = cProps;
	Column.aCol = aPropId;


	//
	// Locate the specified queue.
	//
	HANDLE hEnum;
	BOOL fFound = FALSE;
	HRESULT hr = MQLocateBegin(NULL, &Restriction, &Column, NULL, &hEnum);
	if (!FAILED(hr))
	{
	    MQPROPVARIANT aPropVar[1];
		DWORD cQueue = 1;
		hr = MQLocateNext(hEnum, &cQueue, aPropVar);
		if (!FAILED(hr) && cQueue > 0)
		{
			//
			// Obtain the format name for the queue located.
			//
			hr = MQInstanceToFormatName(aPropVar[0].puuid, wcsFormatName, &dwNumChars);
            MQFreeMemory(aPropVar[0].puuid);
			if (!FAILED(hr))
				fFound = TRUE;
		}

		MQLocateEnd(hEnum);
	}


    return fFound;
}



BOOL CDisdrawDlg::OpenReceiveQueue()
{
    //
    // Do not create the receiving queue if it already exists in the enterprise.
    //
    HRESULT hr;
	WCHAR wcsFormatName[MAX_FORMAT_NAME_LEN+1];
    if (!LocateQueue(m_strLogin, wcsFormatName, sizeof(wcsFormatName)/sizeof(wcsFormatName[0])))
    {
    	//
    	// Form the path name of the receiving queue.
    	//
    	char mbsPathName[MQ_MAX_Q_NAME_LEN+1];
    	DWORD dwNumChars = sizeof(mbsPathName);
		if(!GetComputerName(mbsPathName, &dwNumChars))
		{
			return FALSE;
		}
		
		if( (sizeof(mbsPathName)-1) <= (dwNumChars + m_strLogin.GetLength() + 1))
		{
			return FALSE;
		}

		strncat_s(mbsPathName, sizeof(mbsPathName), "\\",1);
		strncat_s(mbsPathName, sizeof(mbsPathName), m_strLogin,m_strLogin.GetLength());


    	//
    	// Prepare the receiving queue properties.
    	//
    	DWORD cProps = 0;
    	QUEUEPROPID  aPropId[3];
    	MQPROPVARIANT aPropVar[3];
	    MQQUEUEPROPS propsQueue;
	    
            WCHAR wcsPathName[MQ_MAX_Q_NAME_LEN+1];
            size_t  nResultedStringSize=0;
	    mbstowcs_s(
                &nResultedStringSize, 
                wcsPathName, 
                sizeof(wcsPathName)/sizeof(wcsPathName[0]), 
                mbsPathName, 
                sizeof(wcsPathName)/sizeof(wcsPathName[0])-1 
                );

	    aPropId[cProps]			 = PROPID_Q_PATHNAME;
	    aPropVar[cProps].vt		 = VT_LPWSTR;
	    aPropVar[cProps].pwszVal = wcsPathName;
	    cProps++;

	    aPropId[cProps]			 = PROPID_Q_TYPE;
	    aPropVar[cProps].vt		 = VT_CLSID;
	    aPropVar[cProps].puuid	 = &guidDrawType;
	    cProps++;

            WCHAR wcsLabel[MQ_MAX_Q_LABEL_LEN+1];
            nResultedStringSize=0;
	    mbstowcs_s(
                &nResultedStringSize, 
                wcsLabel, 
                sizeof(wcsLabel)/sizeof(wcsLabel[0]),  
                m_strLogin, 
                sizeof(wcsLabel)/sizeof(wcsLabel[0])-1
                );
	    aPropId[cProps]			 = PROPID_Q_LABEL;
	    aPropVar[cProps].vt		 = VT_LPWSTR;
	    aPropVar[cProps].pwszVal = wcsLabel;
	    cProps++;

	    propsQueue.cProp	= cProps;
	    propsQueue.aPropID	= aPropId;
	    propsQueue.aPropVar = aPropVar;
	    propsQueue.aStatus	= NULL;


	    //
	    // Create the receiving queue.
	    //
	    dwNumChars = sizeof(wcsFormatName)/sizeof(wcsFormatName[0]);
	    hr = MQCreateQueue(NULL, &propsQueue, wcsFormatName, &dwNumChars);	


	    //
	    // If the receiving queue already exists, obtain its format name.
	    //
	    if (hr == MQ_ERROR_QUEUE_EXISTS)
            {
                    dwNumChars = sizeof(wcsFormatName)/sizeof(wcsFormatName[0]);
		    hr = MQPathNameToFormatName(wcsPathName, wcsFormatName, &dwNumChars);
            }

	    if (FAILED(hr))
		    return FALSE;
    }

	//
	// Open the receiving queue (may need to retry due to replication latency).
	//
    int iCount = 0;
	while ((hr = MQOpenQueue(wcsFormatName, 
                             MQ_RECEIVE_ACCESS, 
                             MQ_DENY_NONE ,
                             &m_hqIncoming)) == MQ_ERROR_QUEUE_NOT_FOUND)
    {
		
        Sleep (500);
        iCount++;
        if (iCount >= 30)
        {
            //
            // Because 15 seconds have past without locating the queue,
            // we allow the user to stop making attempts to open the queue. 
            //
            int iRes = MessageBox("The receiving queue has not beeen located.\nDo you want to keep searching?",
                                NULL,
                                MB_YESNO);
            if(iRes == IDYES)
            {
                //
                // Continue searching.
                //
                iCount = 0;
            }
            else
            {
                //
                // Stop searching.
                //
                break;
            }
        }
    }

	if (FAILED(hr))
		return FALSE;


	return TRUE;
}


DWORD ReceiveUpdates(CDisdrawDlg *pDrawDlg)
{
   	//
	// Prepare the message properties for receiving a message.
	//
	DWORD cProps = 0;
	MQMSGPROPS    propsMessage;
	MQPROPVARIANT aPropVar[2];
	MSGPROPID     aPropId[2];

	WCHAR wcsBody[MAX_MSG_BODY_LEN];
	aPropId[cProps]				= PROPID_M_BODY;
	aPropVar[cProps].vt			= VT_UI1 | VT_VECTOR;
	aPropVar[cProps].caub.cElems = sizeof(wcsBody);
	aPropVar[cProps].caub.pElems = (UCHAR *)wcsBody;
	cProps++;

    aPropId[cProps]				= PROPID_M_BODY_SIZE;
	aPropVar[cProps].vt			= VT_UI4;
	cProps++;

	propsMessage.cProp    = cProps;
	propsMessage.aPropID  = aPropId;
	propsMessage.aPropVar = aPropVar;
	propsMessage.aStatus  = NULL;


    //
    // Keep receiving updates sent to the incoming queue.
    //
    HRESULT hr;
    LINE line;
	char mbsBody[MAX_MSG_BODY_LEN];

    while (TRUE)
    {
        //
        // Synchronously receive a message from the incoming queue.
        //
        hr = MQReceiveMessage(pDrawDlg->m_hqIncoming, INFINITE, 
                              MQ_ACTION_RECEIVE, &propsMessage, 
							  NULL, NULL, NULL, NULL);

		//
		// Determine whether the message contains a keystroke or a line.
		//
        if (!FAILED(hr))
        {
			//
			// Convert the body to a multi-byte null-terminated string. 
			//
			UINT uNumChars = aPropVar[1].ulVal/sizeof(WCHAR);
			if( sizeof(mbsBody) < uNumChars)
			{
				AfxMessageBox("The message body length exceeds the allowed value. Exiting...");
				exit(1);
			}

            size_t  nResultedStringSize=0;
	    size_t rc = wcstombs_s(&nResultedStringSize, mbsBody, sizeof(mbsBody), wcsBody, uNumChars);
			if (rc != (size_t)(0))
			{
				AfxMessageBox("An unsupported wide character was encountered. Ignoring...");
				continue;
			}


			//
			// Add the keystroke to the drawing.
			//
			if (uNumChars == 1)
            {
				pDrawDlg->m_drawScribble.AddKeystroke(mbsBody);
            }
			
			//
			// Add the received line to the drawing.
			//
			else
			{
				int iRes = sscanf_s(mbsBody, "%07ld%07ld%07ld%07ld",  
					   &line.ptStart.x, &line.ptStart.y, 
					   &line.ptEnd.x, &line.ptEnd.y);
				if (iRes != 0 && iRes != EOF)
				{
					pDrawDlg->m_drawScribble.AddLine(line);
				}
			}
        }
    }


    return 0;
}


BOOL CDisdrawDlg::OnInitDialog()
{
	m_iFormatNameList = FORMAT_NAME_LIST_LEN;
	m_wcsFormatNameList = new WCHAR[FORMAT_NAME_LIST_LEN];
    m_wcsFormatNameList[0] = L'\0';
	CDialog::OnInitDialog();

	// Set the icon for this dialog box.  The framework does this automatically
	// when the application's main window is not a dialog box.
	SetIcon(m_hIcon, TRUE);			// Set the big icon.
	SetIcon(m_hIcon, FALSE);		// Set the small icon.
	
    
	//
	// Display the logon name in the window title.
	//
	SetWindowText("Listening to: " + m_strLogin);


	//
	// No queues are open yet.
	//
	m_hqIncoming = NULL;
	m_hqOutgoing = NULL;
                               

    if (!m_fDsEnabledLocaly)
    {
        //
        // Access to the DS is disabled. This is the right place to open the receiving queue.
        // Because we open a local private queue, this operation is not time consuming.
        //
        //
	    // Open the receiving queue and receive incoming messages.
	    //
        
        if (OpenPrivateReceiveQueue())
        {
            startReceiveUpdateThread();
        }
	    else
        {
            //
            // The attempt to open the receiving queue failed.
            //
		    MessageBox("The receiving queue cannot be opened.");
        }
		m_cComputerInput.ShowWindow(SW_SHOW);
		m_cComputerLabel.ShowWindow(SW_SHOW);

		
		m_cRadioPrivate.ShowWindow(SW_HIDE);
		m_cRadioPublic.ShowWindow(SW_HIDE);
		m_cQueueFrame.ShowWindow(SW_HIDE);
		m_iType = 1;
		m_cConnectionMode.SetWindowText("Mode: Workgroup");

    }


	if((m_iRadioDS == 0)&&(m_fDsEnabledLocaly))
    {
        //
        // Access to the DS is enabled locally, and the user chose to connect in domain (standard) mode.
        //
        
         CWaitCursor wait;

        //
	    // Open the receiving queue and receive incoming messages.
	    //
        
        if (OpenReceiveQueue())
        {
            startReceiveUpdateThread();                         
        }
	    else
        {
		    AfxMessageBox("The receiving queue cannot be opened.");
        }
		m_cQueueFrame.ShowWindow(SW_SHOW);
		m_cRadioPublic.ShowWindow(SW_SHOW);
		m_cRadioPrivate.ShowWindow(SW_SHOW);
		m_cConnectionMode.SetWindowText("Mode: Domain");
	}
	else if(m_fDsEnabledLocaly)  // m_iRadio == 1
    {
        //
        // Access to the DS is enabled locally, and the user chose to connect in workgroup (direct) mode.
        //
        //
        // Change the cursor to an hourglass. The constructor changes the cursor.
        // When the object goes out of scope, the cursor returns to normal.
        //
        CWaitCursor wait ;

        //
	    // Open the receiving private queue and receive incoming messages.
	    //
        
        if (OpenPrivateReceiveQueue())
        {
            startReceiveUpdateThread();
        }
	    else
        {
		    AfxMessageBox("The receiving queue cannot be opened.");
        }

		m_cQueueFrame.ShowWindow(SW_SHOW);
		m_cRadioPublic.ShowWindow(SW_SHOW);
		m_cRadioPrivate.ShowWindow(SW_SHOW);
		m_cConnectionMode.SetWindowText("Mode: Domain");
	}
	strTitle = m_strLogin + " connected to ";


   	return TRUE;  // Return TRUE unless you set the focus to a control.
}





// If you add a minimize button to your dialog box, you will need the code below
// to draw the icon. For MFC applications using the document/view model,
// this is done automatically for you by the framework.

void CDisdrawDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center the icon in the client rectangle.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDisdrawDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CDisdrawDlg::SendKeystroke(UINT uChar)
{
	//
	// Send the keystroke to the friend queue, if any exists.
	//
	if (m_hqOutgoing != NULL)
	{
		//
		// Prepare the message properties for sending a messages.
		//
		DWORD cProps = 0;
		MQMSGPROPS    propsMessage;
		MQPROPVARIANT aPropVar[5];
		MSGPROPID     aPropId[5];

		WCHAR wcsBody[2];
		_snwprintf_s(wcsBody, sizeof(wcsBody)/sizeof(wcsBody[0]), 1, L"%c", uChar);

		aPropId[cProps]				= PROPID_M_BODY;
		aPropVar[cProps].vt			= VT_UI1 | VT_VECTOR;
		aPropVar[cProps].caub.cElems = sizeof(wcsBody[0]);
		aPropVar[cProps].caub.pElems = (UCHAR *)wcsBody;
		cProps++;

		WCHAR wcsLabel[MQ_MAX_MSG_LABEL_LEN+1];
		int n = _snwprintf_s(wcsLabel, sizeof(wcsLabel)/sizeof(wcsLabel[0]), 6, L"Key: %c", uChar); 
		if(n < 0)
		{
			AfxMessageBox("The label too long for the buffer specified.");
			exit(1);
		}

		aPropId[cProps]				= PROPID_M_LABEL;
		aPropVar[cProps].vt			= VT_LPWSTR;
		aPropVar[cProps].pwszVal	= wcsLabel;
		cProps++;

        UpdateData(TRUE);
		aPropId[cProps]				= PROPID_M_DELIVERY;
		aPropVar[cProps].vt			= VT_UI1;
		aPropVar[cProps].bVal		= m_iDelivery;
		cProps++;

		aPropId[cProps]				= PROPID_M_PRIORITY;
		aPropVar[cProps].vt			= VT_UI1;
		aPropVar[cProps].bVal		= 4;
		cProps++;

		aPropId[cProps]				= PROPID_M_BODY_TYPE;
		aPropVar[cProps].vt			= VT_UI4;
		aPropVar[cProps].lVal		= (long)VT_BSTR;
		cProps++;

		propsMessage.cProp    = cProps;
		propsMessage.aPropID  = aPropId;
		propsMessage.aPropVar = aPropVar;
		propsMessage.aStatus  = NULL;


		//
		// Send the message to the outgoing queue.
		//
		MQSendMessage(m_hqOutgoing, &propsMessage, NULL);
	}
}


void CDisdrawDlg::SendMouseMovement(LINE line)
{
	//
	// Send the line to the friend queue, if any exists.
	//
	if (m_hqOutgoing != NULL)
	{
		//
		// Prepare the message properties for sending.
		//
		DWORD cProps = 0;
		MQMSGPROPS    propsMessage;
		MQPROPVARIANT aPropVar[5];
		MSGPROPID     aPropId[5];

		WCHAR wcsBody[MAX_MSG_BODY_LEN];
		swprintf_s(wcsBody, sizeof(wcsBody)/sizeof(wcsBody[0]), L"%07ld%07ld%07ld%07ld", 
				 line.ptStart.x, line.ptStart.y, line.ptEnd.x, line.ptEnd.y);

		aPropId[cProps]				= PROPID_M_BODY;
		aPropVar[cProps].vt			= VT_UI1 | VT_VECTOR;
		aPropVar[cProps].caub.cElems = sizeof(wcsBody);
		aPropVar[cProps].caub.pElems = (UCHAR *)wcsBody;
		cProps++;

		WCHAR wcsLabel[MQ_MAX_MSG_LABEL_LEN+1];
		swprintf_s(wcsLabel, sizeof(wcsLabel)/sizeof(wcsLabel[0]), L"%07ld,%07ld To %07ld,%07ld", 
				 line.ptStart.x, line.ptStart.y, line.ptEnd.x, line.ptEnd.y);

		aPropId[cProps]				= PROPID_M_LABEL;
		aPropVar[cProps].vt			= VT_LPWSTR;
		aPropVar[cProps].pwszVal	= wcsLabel;
		cProps++;

        UpdateData(TRUE);
		aPropId[cProps]				= PROPID_M_DELIVERY;
		aPropVar[cProps].vt			= VT_UI1;
		aPropVar[cProps].bVal		= m_iDelivery;
		cProps++;

		aPropId[cProps]				= PROPID_M_PRIORITY;
		aPropVar[cProps].vt			= VT_UI1;
		aPropVar[cProps].bVal		= 3;
		cProps++;

		aPropId[cProps]				= PROPID_M_BODY_TYPE;
		aPropVar[cProps].vt			= VT_UI4;
		aPropVar[cProps].lVal		= (long)VT_BSTR;
		cProps++;

		propsMessage.cProp    = cProps;
		propsMessage.aPropID  = aPropId;
		propsMessage.aPropVar = aPropVar;
		propsMessage.aStatus  = NULL;


		//
		// Send the message to the outgoing queue.
		//
		MQSendMessage(m_hqOutgoing, &propsMessage, NULL);
	}
}




void CDisdrawDlg::OnButtonAddQueue()
{
	CWaitCursor wait;
	WCHAR wcsFormatName[MAX_FORMAT_NAME_LEN+1];
	UpdateData(TRUE);
	if (!m_fDsEnabledLocaly)
		m_iType = 1;
	if (0 == m_iType)	// public queue
	{
	
		m_strFriend.MakeUpper();

		if (!LocateQueue(m_strFriend, wcsFormatName, sizeof(wcsFormatName)/sizeof(wcsFormatName[0])))
	    {
		    AfxMessageBox("No such friend queue was found.");
		    return;
		}

	}
	else
	{
		//
		// Connect to a computer in workgroup (direct) mode. 
		//
	
		//
		// Obtain the names of the friend queue and the remote computer.
		//
		m_strFriend.MakeUpper();
		m_strRemoteComputerName.MakeUpper();

		//
		// Verify that the friend queue exists. Because we are connecting to 
		// a private queue, we cannot obtain its format name through the DS,
		// and we shall asuume that it exists and shall try to open it.
		//
   
		

		//
		// Form the format name.
		//
		char mbsFormatName[MAX_FORMAT_NAME_LEN+1];
		int n = _snprintf_s(
                             mbsFormatName, 
                             sizeof(mbsFormatName), 
                             sizeof(mbsFormatName) - 1, 
                             "DIRECT=OS:%s\\private$\\%s", 
                             (LPCTSTR)m_strRemoteComputerName, 
                             (LPCTSTR)m_strFriend
                             );
		if(n < 0)
		{
			AfxMessageBox("The format name is too long for the buffer specified. Exiting...");
			exit(1);
		}

                size_t  nResultedStringSize=0;
                mbstowcs_s(
                      &nResultedStringSize, 
                      wcsFormatName, 
                      sizeof(wcsFormatName)/sizeof(wcsFormatName[0]), 
                      mbsFormatName, 
                      sizeof(wcsFormatName)/sizeof(wcsFormatName[0])-1
                      );

	}

	//
	// Make sure that there is enough space for the new format name.
	//
	if((wcslen(m_wcsFormatNameList) + wcslen(wcsFormatName) + 2) > m_iFormatNameList)
	{
		UINT iLength = m_iFormatNameList + FORMAT_NAME_LIST_LEN + 1;

		WCHAR *tempList = new WCHAR[iLength];
		if (tempList == NULL)
		{
			AfxMessageBox("There is insufficient memory. Exiting...");
			exit(1);
		}

		wcsncpy_s(tempList, iLength, m_wcsFormatNameList, iLength - 1);
		tempList[iLength - 1] = '\0';
		
		m_iFormatNameList += FORMAT_NAME_LIST_LEN + 1;
		delete []  m_wcsFormatNameList;
		m_wcsFormatNameList = tempList;
	}
	if(m_wcsFormatNameList[0] != L'\0')
		wcsncat_s(m_wcsFormatNameList, m_iFormatNameList, L",", 1);

	//
	// Add the new format name to the list.
	//
	wcsncat_s(m_wcsFormatNameList, m_iFormatNameList, wcsFormatName, wcslen(wcsFormatName) + 1);

	m_btnAttach.EnableWindow(TRUE);
	strTitle = strTitle + " " + m_strFriend;

}




void CDisdrawDlg::OnButtonAttach() 
{


	// Open the remote queue for sending.
	
	HANDLE hqNewFriend;
	HRESULT hr = MQOpenQueue(m_wcsFormatNameList, MQ_SEND_ACCESS, MQ_DENY_NONE , &hqNewFriend);
	if (FAILED(hr))
		AfxMessageBox("The remote queue cannot be opened.");

	else
	{
		//
		// Close the previous friend queue and update the window title.
		//
		if (m_hqOutgoing != NULL)
        {
			MQCloseQueue(m_hqOutgoing);
        }
		m_hqOutgoing = hqNewFriend;


		SetWindowText(strTitle);
		m_btnAttach.EnableWindow(FALSE);
	}

	//
	// Hide the data.
	//
	m_cComputerLabel.ShowWindow(SW_HIDE);
	m_cQueueLabel.ShowWindow(SW_HIDE);
	m_cQueueFrame.ShowWindow(SW_HIDE);
	m_cComputerInput.ShowWindow(SW_HIDE);
	m_cQueueInput.ShowWindow(SW_HIDE);
	m_btnAttach.ShowWindow(SW_HIDE);
	m_cRadioPublic.ShowWindow(SW_HIDE);
	m_cRadioPrivate.ShowWindow(SW_HIDE);
	m_btnAddQueue.ShowWindow(SW_HIDE);
	m_cFriendFrame.ShowWindow(SW_HIDE);

	//
	// Show the data.
	//
	m_cMessageFrame.ShowWindow(SW_SHOW);
	m_cRadioExpress.ShowWindow(SW_SHOW);
	m_cRadioRecoverable.ShowWindow(SW_SHOW);
    
}



void CDisdrawDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call teh default.

	//
	// Close the open queues, if any exist.
	//
	if (m_hqIncoming != NULL)
		MQCloseQueue(m_hqIncoming);

	if (m_hqOutgoing != NULL)
		MQCloseQueue(m_hqOutgoing);

	CDialog::OnClose();
}



BOOL CDisdrawDlg::OpenPrivateReceiveQueue()
{ 
   
    //
    // Form the path name of the receiving queue, making it private.
    //
    char mbsPathName[MQ_MAX_Q_NAME_LEN+1];
    DWORD dwNumChars = sizeof(mbsPathName);

	if(!GetComputerName(mbsPathName, &dwNumChars))
	{
		return FALSE;
	}
	
	if( (sizeof(mbsPathName)-1) <= (dwNumChars + m_strLogin.GetLength() + 10))
	{
		return FALSE;
	}
	strncat_s(mbsPathName, sizeof(mbsPathName), "\\private$\\", 10);
	strncat_s(mbsPathName, sizeof(mbsPathName), m_strLogin,m_strLogin.GetLength());


    //
    // Prepare the receiving queue properties.
    //
    DWORD cProps = 0;
    QUEUEPROPID  aPropId[3];
    MQPROPVARIANT aPropVar[3];
	MQQUEUEPROPS propsQueue;
	
	WCHAR wcsPathName[MQ_MAX_Q_NAME_LEN+1];
        size_t nResultedStringSize=0;
        mbstowcs_s(
             &nResultedStringSize, 
             wcsPathName, 
             sizeof(wcsPathName)/sizeof(wcsPathName[0]), 
             mbsPathName, 
             sizeof(wcsPathName)/sizeof(wcsPathName[0])-1
             );

	aPropId[cProps]			 = PROPID_Q_PATHNAME;
	aPropVar[cProps].vt		 = VT_LPWSTR;
	aPropVar[cProps].pwszVal = wcsPathName;
	cProps++;

	aPropId[cProps]			 = PROPID_Q_TYPE;
	aPropVar[cProps].vt		 = VT_CLSID;
	aPropVar[cProps].puuid	 = &guidDrawType;
	cProps++;

	WCHAR wcsLabel[MQ_MAX_Q_LABEL_LEN+1];
        nResultedStringSize=0;
        mbstowcs_s(
             &nResultedStringSize, 
             wcsLabel, 
             sizeof(wcsLabel)/sizeof(wcsLabel[0]),  
             m_strLogin, 
             sizeof(wcsLabel)/sizeof(wcsLabel[0])-1
             );

	aPropId[cProps]			 = PROPID_Q_LABEL;
	aPropVar[cProps].vt		 = VT_LPWSTR;
	aPropVar[cProps].pwszVal = wcsLabel;
	cProps++;

	propsQueue.cProp	= cProps;
	propsQueue.aPropID	= aPropId;
	propsQueue.aPropVar = aPropVar;
	propsQueue.aStatus	= NULL;


	//
	// Create the receiving queue.
	//
    WCHAR wcsFormatName[MAX_FORMAT_NAME_LEN+1];
	dwNumChars = sizeof(wcsFormatName)/sizeof(wcsFormatName[0]);
	HRESULT hr = MQCreateQueue(NULL, &propsQueue, wcsFormatName, &dwNumChars);


	if (FAILED(hr) && hr != MQ_ERROR_QUEUE_EXISTS)
    {
		return FALSE;
    }


    if( hr ==  MQ_ERROR_QUEUE_EXISTS)
    {
       	//
    	// If the receiving queue already exists, create a format name.
	    //
        
        WCHAR wcsLoginName[MQ_MAX_Q_NAME_LEN+1];
        nResultedStringSize=0;
        mbstowcs_s(
             &nResultedStringSize, 
             wcsLoginName, 
             sizeof(wcsLoginName)/sizeof(wcsLoginName[0]),  
             m_strLogin, 
             sizeof(wcsLoginName)/sizeof(wcsLoginName[0])-1
             );

		int n = _snwprintf_s(wcsFormatName, sizeof(wcsFormatName)/sizeof(wcsFormatName[0]), sizeof(wcsFormatName)/sizeof(wcsFormatName[0])-1, L"DIRECT=OS:.\\private$\\%s",wcsLoginName);
		if(n < 0)
		{
			AfxMessageBox("The format name is too long for the buffer specified. Exiting...");
			exit(1);
		}		
		wcsFormatName[n] = L'\0';
	

    }

   	//
	// Open the receiving queue (a local private queue).
	//

	hr = MQOpenQueue(wcsFormatName, 
                             MQ_RECEIVE_ACCESS, 
                             MQ_DENY_NONE , 
                             &m_hqIncoming);


	if (FAILED(hr))
    {
		return FALSE;
    }
    return TRUE;

}



void CDisdrawDlg::startReceiveUpdateThread()
{
    DWORD dwThreadID=NULL;
    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)ReceiveUpdates,
                 this,
                 0,
                 &dwThreadID);
    if( dwThreadID == NULL)
    {
       MessageBox("The system failed to initialize the receiving queue.\nNo Drawing will be received.");
    }

}

void CDisdrawDlg::OnRadioPrivatequeue() 
{
		m_cComputerLabel.ShowWindow(SW_SHOW);
		m_cComputerInput.ShowWindow(SW_SHOW);
}

void CDisdrawDlg::OnRadioPublicqueue() 
{
		m_cComputerLabel.ShowWindow(SW_HIDE);
		m_cComputerInput.ShowWindow(SW_HIDE);
}
