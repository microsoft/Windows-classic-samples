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
//

#include "stdafx.h"
#include "disdraw.h"
#include "drawdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FORMAT_NAME_LEN 256

#define MAX_VAR       20


//
// Distributed drawing queue type.
//
CLSID guidDrawType =
{ 0x151ceac0, 0xacb5, 0x11cf, { 0x8b, 0x51, 0x00, 0x20, 0xaf, 0x92, 0x95, 0x46 } };

/////////////////////////////////////////////////////////////////////////////
// CDisdrawDlg dialog

CDisdrawDlg::CDisdrawDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisdrawDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisdrawDlg)
	m_strFriend = _T("");
	m_iDelivery = 0;
	m_iRadioDS = 0;
	m_strRemoteComputerName = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDisdrawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisdrawDlg)
	DDX_Control(pDX, IDC_STATIC_C_LABEL, m_cComputerLabel);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_STATIC_Q_LABEL, m_cQueueLabel);
	DDX_Control(pDX, IDC_STATIC_CHOOSE_MESSAGE, m_cMessageFrame);
	DDX_Control(pDX, IDC_STATIC_CHOOSE_DS, m_cDsFrame);
	DDX_Control(pDX, IDC_RADIO_EXPRESS, m_cRadioExpress);
	DDX_Control(pDX, IDC_RADIO_DS, m_cRadioDS);
	DDX_Control(pDX, IDC_EDIT_FRIEND_COMPUTER, m_cComputerInput);
	DDX_Control(pDX, IDC_EDIT_FRIEND, m_cQueueInput);
	DDX_Control(pDX, IDC_CONTINUE, m_cContinueButton);
	DDX_Control(pDX, IDC_DRAWAREA_SCRIBLLE, m_drawScribble);
	DDX_Control(pDX, IDC_BUTTON_ATTACH, m_btnAttach);
	DDX_Text(pDX, IDC_EDIT_FRIEND, m_strFriend);
	DDV_MaxChars(pDX, m_strFriend, 50);
	DDX_Radio(pDX, IDC_RADIO_EXPRESS, m_iDelivery);
	DDX_Radio(pDX, IDC_RADIO_DS, m_iRadioDS);
	DDX_Text(pDX, IDC_EDIT_FRIEND_COMPUTER, m_strRemoteComputerName);
	DDV_MaxChars(pDX, m_strRemoteComputerName, 50);
    DDX_Control(pDX, IDC_RADIO_WORKGROUP, m_cRadioWorkgroup);
    DDX_Control(pDX, IDC_RADIO_RECOVERABLE, m_cRadioRecoverable);
	//}}AFX_DATA_MAP
    

}

BEGIN_MESSAGE_MAP(CDisdrawDlg, CDialog)
	//{{AFX_MSG_MAP(CDisdrawDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ATTACH, OnButtonAttach)
	ON_EN_CHANGE(IDC_EDIT_FRIEND, OnChangeEditFriend)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CONTINUE, OnConnect)
	ON_EN_CHANGE(IDC_EDIT_FRIEND_COMPUTER, OnChangeEditFriendComputer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisdrawDlg message handlers


BOOL LocateQueue(CString m_strLabel, WCHAR *wcsFormatName, DWORD dwNumChars)
{
	//
	// Set restrictions to locate the drawing queue with the label specified.
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
        size_t nResultedStringLength=0;
	mbstowcs_s(
            &nResultedStringLength,
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
	// Request the queue instance for the queue specified.
	//
	cProps = 0;
	QUEUEPROPID aPropId[1];
	MQCOLUMNSET Column;
	
	aPropId[cProps] = PROPID_Q_INSTANCE;
	cProps++;

	Column.cCol = cProps;
	Column.aCol = aPropId;


	//
	// Locate the queue specified.
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
    	// Form the pathname of the receiving queue.
    	//
    	char mbsPathName[MQ_MAX_Q_NAME_LEN+1];
    	DWORD dwNumChars = sizeof(mbsPathName);
    	GetComputerName(mbsPathName, &dwNumChars);
	strcat_s(mbsPathName, sizeof(mbsPathName), "\\");
        strcat_s(mbsPathName, sizeof(mbsPathName), m_strLogin);


    	//
    	// Prepare the receiving queue properties.
    	//
    	DWORD cProps = 0;
    	QUEUEPROPID  aPropId[3];
    	MQPROPVARIANT aPropVar[3];
	    MQQUEUEPROPS propsQueue;
	    
		WCHAR wcsPathName[MQ_MAX_Q_NAME_LEN+1];
                size_t nResultedStringLength=0;
		mbstowcs_s(
                    &nResultedStringLength,
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
                nResultedStringLength=0;
		mbstowcs_s(
                    &nResultedStringLength,
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
	// Open the receiving queue (it may be necessary to retry due to replication latency).
	//
    int iCount = 0;
	while ((hr = MQOpenQueue(wcsFormatName, 
                             MQ_RECEIVE_ACCESS, 
                             MQ_DENY_NONE ,
                             &m_hqIncoming)) == MQ_ERROR_QUEUE_NOT_FOUND)
    {
		
        Sleep (500);
        iCount++;
        if (iCount >=30)
        {
            //
            // A period of 15 seconds past without locating the queue.
            // We allow the user to stop attempts to open the queue.
            //
            int iRes = MessageBox("The receiving queue was not located.\nDo you want to continue searching?",
                                NULL,
                                MB_YESNO);
            if(iRes == IDYES)
            {
                //
                // Continue searching.
                //
                iCount =0;
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
	// Prepare the message properties to receive.
	//
	DWORD cProps = 0;
	MQMSGPROPS    propsMessage;
	MQPROPVARIANT aPropVar[2];
	MSGPROPID     aPropId[2];

	WCHAR wcsBody[MAX_MSG_BODY_LEN+1];
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
	char mbsBody[MAX_MSG_BODY_LEN + 1];

    while (TRUE)
    {
        //
        // Synchronously receive a message from the incoming queue.
        //
        hr = MQReceiveMessage(pDrawDlg->m_hqIncoming, INFINITE, 
                              MQ_ACTION_RECEIVE, &propsMessage, 
							  NULL, NULL, NULL, NULL);

		//
		// Determine if the message contains a keystroke or a line.
		//
        if (!FAILED(hr))
        {
			//
			// Convert the body to a multibyte null-terminated string. 
			//
            UINT uNumChars = aPropVar[1].ulVal/sizeof(WCHAR);
            size_t nResultedStringLength=0;
            wcstombs_s(
                &nResultedStringLength,
                mbsBody, 
                sizeof(mbsBody),
                wcsBody, 
                uNumChars
                );

			//
			// Add the keystroke to the drawing.
			//
			if (uNumChars == 1)
            {
				pDrawDlg->m_drawScribble.AddKeystroke(mbsBody);
            }
			
			//
			// Add the line received to the drawing.
			//
			else
			{
                		sscanf_s(mbsBody, "%07ld%07ld%07ld%07ld", 
					   &line.ptStart.x, &line.ptStart.y, 
					   &line.ptEnd.x, &line.ptEnd.y);
				pDrawDlg->m_drawScribble.AddLine(line);
			}
        }
    }


    return 0;
}


BOOL CDisdrawDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog box.  The framework does this automatically
	//  when the application's main window is not a dialog box.
	SetIcon(m_hIcon, TRUE);			// Use the large icon.
	SetIcon(m_hIcon, FALSE);		// Use the small icon.
	
    
	//
	// Display the logon name in the title bar.
	//
	SetWindowText(m_strLogin);


	//
	// No queues are open yet.
	//
	m_hqIncoming = NULL;
	m_hqOutgoing = NULL;
                               
    //
    // Establish connection mode.
    //
    m_fDsEnabledLocaly = IsDsEnabledLocaly();

    if (!m_fDsEnabledLocaly)
    {
        //
        // The computer is DS-disabled. This is the right place to open the receiving queue
        // because it is a local private queue. This operation is not time-consuming.
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
            // The receiving queue could not be opened.
            //
		    MessageBox("The receiving queue cannot be opened.");
        }

    }

    initializeUi(m_fDsEnabledLocaly);

   	return TRUE;  // Return TRUE unless you set the focus to a control.
}


// If you add a minimize button to your dialog box, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void CDisdrawDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Device context for painting

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


// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CDisdrawDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CDisdrawDlg::SendKeystroke(UINT uChar)
{
	//
	// Send the keystroke to the friend queue, if there is one.
	//
	if (m_hqOutgoing != NULL)
	{
		//
		// Prepare the message properties to send the message.
		//
		DWORD cProps = 0;
		MQMSGPROPS    propsMessage;
		MQPROPVARIANT aPropVar[5];
		MSGPROPID     aPropId[5];

		WCHAR wcsBody[1];
		swprintf_s(wcsBody, sizeof(wcsBody)/sizeof(wcsBody[0]), L"%c", uChar); 
		aPropId[cProps]				= PROPID_M_BODY;
		aPropVar[cProps].vt			= VT_UI1 | VT_VECTOR;
		aPropVar[cProps].caub.cElems = sizeof(wcsBody);
		aPropVar[cProps].caub.pElems = (UCHAR *)wcsBody;
		cProps++;

		WCHAR wcsLabel[MQ_MAX_MSG_LABEL_LEN+1];
		swprintf_s(wcsLabel, sizeof(wcsLabel)/sizeof(wcsLabel[0]), L"Key: %c", uChar);
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
	// Send the line to the friend queue, if there is one.
	//
	if (m_hqOutgoing != NULL)
	{
		//
		// Prepare the message properties to send the message.
		//
		DWORD cProps = 0;
		MQMSGPROPS    propsMessage;
		MQPROPVARIANT aPropVar[5];
		MSGPROPID     aPropId[5];

		WCHAR wcsBody[MAX_MSG_BODY_LEN+1];
		swprintf_s(wcsBody, sizeof(wcsBody)/sizeof(wcsBody[0]), L"%07ld%07ld%07ld%07ld", 
				 line.ptStart.x, line.ptStart.y, line.ptEnd.x, line.ptEnd.y);
		aPropId[cProps]				= PROPID_M_BODY;
		aPropVar[cProps].vt			= VT_UI1 | VT_VECTOR;
		aPropVar[cProps].caub.cElems = sizeof(wcsBody);
		aPropVar[cProps].caub.pElems = (UCHAR *)wcsBody;
		cProps++;

		WCHAR wcsLabel[MQ_MAX_MSG_LABEL_LEN+1];
		swprintf_s(wcsLabel, sizeof(wcsLabel)/sizeof(wcsLabel[0]), L"%ld,%ld To %ld,%ld", 
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


void CDisdrawDlg::OnButtonAttach() 
{

    //
    // The constructor changes the cursor to an hourglass.
    // When the object goes out of scope, the cursor changes back to normal.
    //
    CWaitCursor wait ;
    WCHAR wcsFormatName[MAX_FORMAT_NAME_LEN+1];

	if(m_fDsEnabledLocaly && m_iRadioDS == 0) 
    {
        //
        // Connect with a remote computer in standard mode. 
        //
        
        //
	    // Obtain the name of the friend queue.
	    //
	    UpdateData(TRUE);
	    m_strFriend.MakeUpper();
        //
        // Make sure the friend queue exists.
        //
	    if (!LocateQueue(m_strFriend, wcsFormatName, sizeof(wcsFormatName)/sizeof(wcsFormatName[0])) )
	    {
		    AfxMessageBox("No such queue exists.");
		    return;
	    }
	    
        
    }
    else if(!m_fDsEnabledLocaly || m_iRadioDS == 1) 
    {
        //
        // Connect with a remote computer in direct mode. 
        //
        
        //
	    // Obtain the name of the friend queue and the remote computer.
	    //
	    UpdateData(TRUE);
	    m_strFriend.MakeUpper();
        m_strRemoteComputerName.MakeUpper();


        //
        // Make sure the friend queue exists. Since we are connecting to 
        // a private queue, we cannot obtain its format name from the
        // directory service (DS), so we will asuume it exists and try to open it.
        //
       
	    //
	    // Open the friend queue for sending (in direct mode)
        // Please Note:
        // Opening a remote queue for sending will always  
        // succeed even if the queue or computer does not exist.
        // When working in direct mode, we must use private queues. 
        // We cannot know whether a given private queue on another computer exists or not,
        // so here we will just assume that it does. To make the application more robust,
        // an acknowledgement queue should be created on the local computer, and
        // a request for acknowledgement messages should be added to the messages sent.
        // Then the application can notify the user when a negative adknowledgement message
        // (NACK) is received.
        //

   
        //
        // Compose the format name.
        //
        char mbsFormatName[MAX_FORMAT_NAME_LEN+1];
        sprintf_s(mbsFormatName, sizeof(mbsFormatName), "DIRECT=OS:%s\\private$\\%s",m_strRemoteComputerName,m_strFriend);
        size_t nResultedStringLength=0;
        mbstowcs_s(
            &nResultedStringLength,
            wcsFormatName,
            sizeof(wcsFormatName)/sizeof(wcsFormatName[0]),
            mbsFormatName,
            sizeof(mbsFormatName)-1
            );

    }

    //
	// Open the remote queue for sending.
    // The action is the same in both cases.
	//
	HANDLE hqNewFriend;
	HRESULT hr = MQOpenQueue(wcsFormatName, MQ_SEND_ACCESS, MQ_DENY_NONE , &hqNewFriend);
	if (FAILED(hr))
		AfxMessageBox("The remote queue cannot be opened.");

	else
	{
		//
		// Close the previous friend queue and update the title bar.
		//
		if (m_hqOutgoing != NULL)
        {
			MQCloseQueue(m_hqOutgoing);
        }
		m_hqOutgoing = hqNewFriend;
		CString strTitle = m_strLogin + " connected to " + m_strFriend;
		SetWindowText(strTitle);
		m_btnAttach.EnableWindow(FALSE);
	}
    
}


void CDisdrawDlg::OnChangeEditFriend() 
{

	m_btnAttach.EnableWindow(TRUE);
}


void CDisdrawDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default.

	//
	// Close any open queues.
	//
	if (m_hqIncoming != NULL)
		MQCloseQueue(m_hqIncoming);

	if (m_hqOutgoing != NULL)
		MQCloseQueue(m_hqOutgoing);

	CDialog::OnClose();
}


BOOL CDisdrawDlg::IsDsEnabledLocaly()
/*++

Routine Description:
    
      The routine checks whether the local computer is DS-enabled
      or DS-disabled.

Arguments:
    
      None

Return Value:
    
      TRUE     -  DS-enabled.
      FALSE    -  DS-disabled.

--*/

{
       
    MQPRIVATEPROPS PrivateProps;
    QMPROPID       aPropId[MAX_VAR];
    MQPROPVARIANT  aPropVar[MAX_VAR];
    DWORD          cProp;  
    HRESULT        hr;
    //
    // Prepare a DS-enabled property.
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
    // This code is designed to allow compilation on both 
    // Windows NT 4.0 and Windows 2000.
    //
    HINSTANCE hMqrtLibrary = GetModuleHandle(TEXT("mqrt.dll"));
	ASSERT(hMqrtLibrary != NULL);

    typedef HRESULT (APIENTRY *MQGetPrivateComputerInformation_ROUTINE)(LPCWSTR , MQPRIVATEPROPS*);
    MQGetPrivateComputerInformation_ROUTINE pfMQGetPrivateComputerInformation = 
          (MQGetPrivateComputerInformation_ROUTINE)GetProcAddress(hMqrtLibrary,
													 "MQGetPrivateComputerInformation");
    if(pfMQGetPrivateComputerInformation == NULL)
    {
        //
        // There is no entry point in the DLL that matches this routine.
        // It must be an old version of mqrt.dll -> product one.
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
        MessageBox("No DS connection can be detected.");        
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




void CDisdrawDlg::initializeUi(BOOL fConnectedToDS)
{
    if(fConnectedToDS)
    {
        //
        // Show the relevant controls for a DS-enabled computer.
        //
        m_cDsFrame.ShowWindow(SW_SHOW);
        m_cRadioDS.ShowWindow(SW_SHOW);
        m_cRadioWorkgroup.ShowWindow(SW_SHOW);
        m_cContinueButton.ShowWindow(SW_SHOW);
        m_CancelButton.ShowWindow(SW_SHOW);




    }
    else
    {
        //
        //Show the relevant controls for a DS-disabled computer. 
        //
        m_cMessageFrame.ShowWindow(SW_SHOW);
        m_cRadioExpress.ShowWindow(SW_SHOW);
        m_cRadioRecoverable.ShowWindow(SW_SHOW);
        m_cQueueInput.ShowWindow(SW_SHOW);
        m_cQueueLabel.ShowWindow(SW_SHOW);
        m_cComputerInput.ShowWindow(SW_SHOW);
        m_cComputerLabel.ShowWindow(SW_SHOW);
        m_CancelButton.ShowWindow(SW_SHOW);
        m_btnAttach.ShowWindow(SW_SHOW);
       

    }


}

void CDisdrawDlg::OnConnect() 
{
    //
    // See what the user selected.
    //
    UpdateData(TRUE);
    if(m_iRadioDS == 0)
    {
        //
        // The DS is enabled locally, and the user chose to connect in standard mode.
        //
        
         CWaitCursor wait ;

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


        //
        // Hide relevant controls.
        //
        m_cDsFrame.ShowWindow(SW_HIDE);
        m_cRadioDS.ShowWindow(SW_HIDE);
        m_cRadioWorkgroup.ShowWindow(SW_HIDE);
        m_cContinueButton.ShowWindow(SW_HIDE);

        //
        // Show relevant controls.
        //
        
        m_cMessageFrame.ShowWindow(SW_SHOW);
        m_cRadioExpress.ShowWindow(SW_SHOW);
        m_cRadioRecoverable.ShowWindow(SW_SHOW);
        m_cQueueInput.ShowWindow(SW_SHOW);
        m_cQueueLabel.ShowWindow(SW_SHOW);
        
        m_CancelButton.ShowWindow(SW_SHOW);
        m_btnAttach.ShowWindow(SW_SHOW);
        
    }
    else//m_iRadio == 1
    {
        //
        // The DS is enabled locally, and the user chose to connect in direct mode.
        //
        //
        // The constructor changes the cursor to an hourglass.
        // When the object goes out of scope, the cursor changes back to normal.
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


        
        //
        // Hide relevant controls.
        //
        m_cDsFrame.ShowWindow(SW_HIDE);
        m_cRadioDS.ShowWindow(SW_HIDE);
        m_cRadioWorkgroup.ShowWindow(SW_HIDE);
        m_cContinueButton.ShowWindow(SW_HIDE);
        
        //
        // Show relevant controls.
        //
        m_cMessageFrame.ShowWindow(SW_SHOW);
        m_cRadioExpress.ShowWindow(SW_SHOW);
        m_cRadioRecoverable.ShowWindow(SW_SHOW);
        m_cQueueInput.ShowWindow(SW_SHOW);
        m_cQueueLabel.ShowWindow(SW_SHOW);
        m_cComputerInput.ShowWindow(SW_SHOW);
        m_cComputerLabel.ShowWindow(SW_SHOW);
        m_CancelButton.ShowWindow(SW_SHOW);
        m_btnAttach.ShowWindow(SW_SHOW);

    }

	
}

BOOL CDisdrawDlg::OpenPrivateReceiveQueue()
{ 
   
    //
    // Compose the pathname to the receiving queue and make it private.
    //
    char mbsPathName[MQ_MAX_Q_NAME_LEN+1];
    DWORD dwNumChars = sizeof(mbsPathName);
    GetComputerName(mbsPathName, &dwNumChars);
    strcat_s(mbsPathName, sizeof(mbsPathName), "\\");
    strcat_s(mbsPathName, sizeof(mbsPathName), "private$");
    strcat_s(mbsPathName, sizeof(mbsPathName), "\\");
    strcat_s(mbsPathName, sizeof(mbsPathName), m_strLogin);


    //
    // Prepare the properties of the receiving queue.
    //
    DWORD cProps = 0;
    QUEUEPROPID  aPropId[3];
    MQPROPVARIANT aPropVar[3];
	MQQUEUEPROPS propsQueue;
	
	WCHAR wcsPathName[MQ_MAX_Q_NAME_LEN+1];
        size_t nResultedStringLength=0;
	mbstowcs_s(
            &nResultedStringLength,
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
        nResultedStringLength=0;
	mbstowcs_s(
            &nResultedStringLength,
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
    	// If the receiving queue already exists, obtain its format name.
	    //
        
        WCHAR wcsLoginName[MQ_MAX_Q_NAME_LEN+1];
        nResultedStringLength=0;
	mbstowcs_s(
            &nResultedStringLength,
            wcsLoginName, 
            sizeof(wcsLoginName)/sizeof(wcsLoginName[0]),
            m_strLogin, 
            sizeof(wcsLoginName)/sizeof(wcsLoginName[0])-1
            );
        swprintf_s(wcsFormatName, sizeof(wcsFormatName)/sizeof(wcsFormatName[0]), L"DIRECT=OS:.\\private$\\%s",wcsLoginName);

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

void CDisdrawDlg::OnChangeEditFriendComputer() 
{
    //
    // Allow specifying another remote computer.
    //
	m_btnAttach.EnableWindow(TRUE);
	
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
       MessageBox("The system failed to initialize receiving queue.\nNo Drawing will be received");
    }

}
