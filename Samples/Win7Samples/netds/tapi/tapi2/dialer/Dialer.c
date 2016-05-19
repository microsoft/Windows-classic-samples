/*++ 

Copyright 1996 - 2000 Microsoft Corporation

Module Name:

    dialer.c

--*/


#include "dialer.h"
#include "string.h"
#include "stdlib.h"
#include "shellapi.h"
#include "assert.h"
#include <strsafe.h>
#define  assert_str(hr) assert(SUCCEEDED(hr) || !"strsafe")

#define	ISDIGIT(x)			(((x) - '0') >= 0) && (((x) - '0') <= 9)
enum NumberTypes 
{
	LOCAL_NUMBER = 7,
	EXTENDED_LOCAL_NUMBER,
	LONG_DISTANCE_NUMBER = 10,
	EXTENDED_LONG_DISTANCE_NUMBER
};


// structs
typedef struct tagLINEINFO
{
    DWORD nAddr;                    // Number of avail. addresses on the line
    BOOL  fVoiceLine;               // Is this a voice line?
    DWORD dwAPIVersion;             // API version which the line supports
    HLINE hLine;                    // line handle returned by lineOpen
    DWORD dwPermanentLineID;        // Permanent line ID retreived from devcaps
    char  szLineName[MAXBUFSIZE];   // the line's name

} LINEINFO, *LPLINEINFO;


// Global variables

// window/instance variables
HWND        ghWndMain;
HWND        ghWndDialing = NULL;
HINSTANCE   ghInst = 0;

// file name vars.
static TCHAR gszAppName[64];
static TCHAR gszINIfilename [] = "DIALER.INI";
static TCHAR gszDialerClassName[] = "DialerClass";
TCHAR const gszNULL[] = "";

// window item variables
HFONT       ghFontBtn;               // handle to 1, 2, ..., 0's font
HFONT       ghFontBtnText;           // handle to number button's text font
HFONT       ghFontBtnStar;           // handle to * and # button's font
HLINEAPP    ghLineApp = (HLINEAPP)((ULONG_PTR)NULL);        // Dialer's usage handle (regist. w/TAPI)
HCALL       ghCall = (HCALL)((ULONG_PTR)NULL);           // call handle for Dialer's call

LPSTR       gszCurrentNumber = NULL; // number of destination of current call
LPSTR       gszCurrentName = NULL;	 // name of destination of current call

BOOL        gfRegistered;            // was lineRegisterRequestRecipient()
                                     // successful?

//BOOL        gfDropping = FALSE;      // is Dialer currently dropping a call?
BOOL        gfNeedToReinit = FALSE;  // does Dialer need to re-initialize?

BOOL        gfCallRequest = FALSE;   // Does a Simple TAPI app want a call?
BOOL        gfCurrentLineAvail = TRUE; // Simple TAPI requests are only carried
                                       // out if the current chosen line is avail.
BOOL		gfMakeCallReplyPending = FALSE;

LONG        gMakeCallRequestID = 0;      // request ID returned by async TAPI fns.
LONG        gDropCallRequestID = 0;      // request ID returned by async TAPI fns.

DWORD       gnAvailDevices = 0;      // # of line devices avail. to Dialer
LINEINFO    gCurrentLineInfo;
DWORD       * gnAddr;

// global to remember where the cursor is in the edit control
DWORD       gdwStartSel;
DWORD       gdwEndSel;

DWORD       * gdwPLID;               // current line's permanent line ID
DWORD       giCurrentLine = (DWORD)-1;       // the line selected by the user
DWORD       giCurrentAddress = 0;    // the address selected by the user

// + 1 so we can work 1-based rather than 0-based (for convenience only)
// global varibles to hold the names and address of the 
TCHAR       gszSDNumber[ NSPEEDDIALS + 1 ][ MAXBUFSIZE];


// Function declarations

// button related functions
VOID ButtonFontSetup(VOID);
VOID DrawButton(HDC hdc, RECT rcBtn, BOOL fHighlighted);
VOID DrawButtonText(HDC hdc, RECT rcBtn, BOOL fHighlighted, UINT IDD_Btn);
VOID DisableDialButtons(BOOL fDisable);
VOID FitTextToButton( HWND, INT, LPSTR );

// Callback functions
BOOL CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ConnectUsingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK LineInUseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SpeedDial1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SpeedDial2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK tapiCallback ( 
							DWORD hDevice, DWORD dwMsg,
							DWORD dwCallbackInstance, 
							DWORD dwParam1, DWORD dwParam2, 
							DWORD dwParam3
						    );

// tapi related functions
VOID ManageAssistedTelephony(VOID);
VOID InitiateCall(LPCSTR szNumber, LPCSTR szName);

VOID DialerLineClose(VOID); 
VOID DialerCleanup(VOID);
VOID CloseTAPI(VOID);

DWORD GetLineInfo(DWORD iLine, LPLINEINFO lpLineInfo);
VOID GetLineInfoFailed (
						DWORD iLine, LPLINEDEVCAPS lpDevCaps, 
						LPLINEINFO lpLineInfo
					    );
LPSTR GetAddressName(DWORD iLine, DWORD iAddress);
BOOL MakeCanonicalNumber( LPCSTR szName, LPSTR szCanNumber );

// misc. helper functions
VOID ReadINI(VOID);
int errString(HWND hWnd, UINT errCode, UINT uFlags);
VOID AddToRedialList(LPCSTR szNumber);
BOOL InitializeLineBox(HWND hwndLineBox);
BOOL InitializeAddressBox(HWND hwndLineBox, HWND hwndAddressBox);
BOOL Is911 ( LPLINETRANSLATEOUTPUT lpTransOut );
VOID AmpersandCompensate( LPCSTR lpszSrc, LPSTR lpszDst );
VOID AmpersandDeCompensate( LPCSTR lpszSrc, LPSTR lpszDst );

// Dialer memory management functions
LPVOID DialerAlloc(size_t cbToAlloc);
LPVOID DialerFree(LPVOID lpMem);


// Function definitions


//***************************************************************************
//***************************************************************************
//***************************************************************************
DWORD InitializeTAPI (VOID)
{
	INT cvLine;

	DWORD iLine;
	DWORD dwPreferredPLID, dwID = (DWORD) -1;

	MSG msg;

	LPLINEINFO lpLineInfo = NULL;	// LINEINFO for each available line

    DWORD errCode;
    DWORD tc = GetTickCount();
    DWORD dwReturn = ERR_NONE;

	char szBuffer[MAXBUFSIZE];		// to read in dwPreferredPLID as a string first



	errCode = lineInitialize (
								&ghLineApp,
								ghInst,
								(LINECALLBACK) tapiCallback,
								gszAppName,
								&gnAvailDevices
							 );
	if ( errCode == LINEERR_REINIT )
	{
		// take away dialer functionality
		EnableWindow( ghWndMain, FALSE );
		DisableDialButtons(TRUE);

		// keep trying until the user cancels
		// or we stop getting LINEERR_REINIT
		while ( ( errCode = lineInitialize ( 
											&ghLineApp,              
											ghInst,
											(LINECALLBACK)tapiCallback,
											gszAppName,
											&gnAvailDevices
										    ) ) 
				 == LINEERR_REINIT )
		{
			// flush queue & yield
			while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}

			// bring up the box if 5 seconds have passed since
			if(GetTickCount() > 5000 + tc)
			{
				if ( errString( ghWndMain, ikszWarningTapiReInit, MB_RETRYCANCEL )
					 == IDCANCEL )
				{
					break;
				}
				// reset the relative counter
				tc = GetTickCount(); 
			}            
		}

		// give back dialer functionality
		DisableDialButtons( FALSE );
		EnableWindow( ghWndMain, TRUE );
	}

	if ( errCode )
    {
        dwReturn = errCode;
        goto tapiinit_exit;
    }

	// retrieve preferred line info from INI file
	GetPrivateProfileString (
								"Preference",
								"Preferred Line",
								gszNULL, 
								szBuffer,
								MAXBUFSIZE,
								gszINIfilename
							);

	// if szBuffer is not empty.
	if ( lstrcmp ( gszNULL, szBuffer ) )
	{
		dwPreferredPLID = (DWORD) atoi( szBuffer );	
	}
	else	
	{
		dwPreferredPLID = (DWORD) -1;
	}
										
	// -1 default - tells us if it ever gets set
	giCurrentLine = (DWORD) -1;			

	// allocate buffer for storing LINEINFO for all of the available lines
	// always allocate space for at least one line
	if ( gnAvailDevices == 0 )
	{
		gnAddr = (DWORD *) DialerAlloc( sizeof( DWORD ) );
		gdwPLID = (DWORD *) DialerAlloc( sizeof( DWORD ) );
		lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof( LINEINFO ) );
	}
	else
	{
		gnAddr = (DWORD *) DialerAlloc( sizeof( DWORD ) * (int)gnAvailDevices);
		gdwPLID = (DWORD *) DialerAlloc( sizeof( DWORD ) * (int)gnAvailDevices);
		lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof( LINEINFO ) * (int)gnAvailDevices );
	}

	// if no space was set aside...
	if ( lpLineInfo == NULL || gdwPLID == NULL || gnAddr == NULL )
    {
        dwReturn = LINEERR_NOMEM;
        goto tapiinit_exit;
    }

	// fill lpLineInfo[] and open each line
	for ( iLine = 0, cvLine = 0; iLine < gnAvailDevices; ++iLine )
	{
		// skip remaining processing for this if it didn't open
		if ( GetLineInfo( iLine, &lpLineInfo[iLine] ) != ERR_NONE )
			continue; 

		gnAddr [ iLine ] = lpLineInfo[iLine].nAddr;
		gdwPLID[ iLine ] = lpLineInfo[iLine].dwPermanentLineID;

		if ( lpLineInfo[iLine].dwPermanentLineID == dwPreferredPLID )
			giCurrentLine = iLine;

		// note number of lines with Interactive voice caps.
		// used to select a preferred line by default
		if ( lpLineInfo [ iLine ].fVoiceLine )
		{
			cvLine++;
			dwID = iLine;
		}
	}

	// if we couldn't find the preferred line, 
	// try and assign one by default 
	// else bring up connect using dialog
	if (  giCurrentLine == (DWORD)-1 ) 
	{
		// check if there is only one line
		// that has interactive voice caps, 
		// make it default line
		if ( cvLine == 1 ) 
		{
			giCurrentLine = dwID;

			// if the preferred address read from the INI file
			// was different i.e we are changing setting, inform
			// the user
			if ( dwPreferredPLID != -1 )
			{
				errString( ghWndMain, ERR_NEWDEFAULT, MB_ICONEXCLAMATION | MB_OK );
			}
		}
		else
		{
			gCurrentLineInfo = lpLineInfo[0];
			if ( DialogBoxParam (
									ghInst,
									MAKEINTRESOURCE(IDD_CONNECTUSING),
									ghWndMain,
									(DLGPROC)ConnectUsingProc,
									INVALID_LINE
								)
				 == -1)
			{
                dwReturn = (DWORD) -1;
			}
			else
			{
                dwReturn = ERR_NONE;
			}

            goto tapiinit_exit;
		}
	}
	gCurrentLineInfo = lpLineInfo[ giCurrentLine ];


	// select default address
	giCurrentAddress = 0;

	// get the name of the preferred address from ini file
	GetPrivateProfileString (
								"Preference",
								"Preferred Address",
								gszNULL, 
								szBuffer,
								MAXBUFSIZE,
								gszINIfilename
							);

	// if preferred address read from INI file
	if ( lstrcmp( gszNULL, szBuffer ) )
	{
		giCurrentAddress = (DWORD) atoi( szBuffer );
		
		// if the address is invalid, set default
		if ( giCurrentAddress >= gCurrentLineInfo.nAddr )
			giCurrentAddress = 0;
	}
	

tapiinit_exit:
    
    if (lpLineInfo)
    {
        DialerFree(lpLineInfo);
    }
    
	return dwReturn;;
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
int WINAPI WinMain (
					HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow
				   )
{
	HACCEL hAccel;
	MSG msg;
	DWORD errCode;
	HANDLE hImHere;


	ghInst = GetModuleHandle( NULL );
	LoadString( ghInst, ikszAppFriendlyName, gszAppName, sizeof(gszAppName)/sizeof(TCHAR) );

	//
	// Now, let's see if we've already got an instance of ourself
	hImHere = CreateMutex(NULL, TRUE, "DialersIveBeenStartedMutex");

	//
	// Is there another one of us already here?
	if ( ERROR_ALREADY_EXISTS == GetLastError() )
	{
        HWND        hDialerWnd;

        hDialerWnd = FindWindow(gszDialerClassName,
                                NULL);

        SetForegroundWindow(hDialerWnd);
        
	   CloseHandle( hImHere );
	   return 0;
	}


	{
		WNDCLASS wc;
		wc.style = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc = DefDlgProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = DLGWINDOWEXTRA;
		wc.hInstance = ghInst;
		wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_DIALER) );
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = gszDialerClassName;
		RegisterClass(&wc);
	}


	// create the dialog box and set it with info
	// from the .INI file
	ghWndMain = CreateDialog (
								ghInst,
								(LPCSTR)MAKEINTRESOURCE(IDD_DIALER),
								(HWND)NULL,
								(DLGPROC)MainWndProc
							 );

	ReadINI();
	ButtonFontSetup();

	ShowWindow(ghWndMain, SW_SHOW);
	UpdateWindow(ghWndMain);

	// limit text in Number field to TAPIMAXDESTADDRESSSIZE
	SendDlgItemMessage (
						ghWndMain,
						IDD_DCOMBO,
						CB_LIMITTEXT,
						(WPARAM)TAPIMAXDESTADDRESSSIZE,
						0
					   );

	// 0 (ERR_NONE) error code registers success - otherwise terminate
	errCode = InitializeTAPI();
	if(errCode)
	{
		errString(ghWndMain, errCode, MB_APPLMODAL | MB_ICONEXCLAMATION );

		DialerCleanup();
		return errCode;
	}

	errCode = lineRegisterRequestRecipient (
											ghLineApp,
											0, // registration instance
											LINEREQUESTMODE_MAKECALL,
											TRUE
										   );

	if(errCode)
	{
		gfRegistered = FALSE;
		errString(ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
	}
	else
	{
		gfRegistered = TRUE;
	}


	hAccel = LoadAccelerators(ghInst, gszAppName);

	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( ghWndMain == NULL || !IsDialogMessage( ghWndMain, &msg ) )
		{
			if(!TranslateAccelerator(ghWndMain, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// If: 1) Dialer is a call manager (if not, ignore requests)
		//     2) the currently chosen line is available
		//     3) there is a Simple TAPI request
		// Then: process the request
		if ( gfCurrentLineAvail && gfCallRequest )
		{
			ManageAssistedTelephony();
		}
	}


    
	DialerCleanup();

	CloseHandle( hImHere );

	return (int)(msg.wParam);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LPVOID DialerAlloc(size_t cbToAlloc)
{
	return LocalAlloc(LPTR, cbToAlloc);
}


LPVOID DialerFree(LPVOID lpMem)
{
	return LocalFree( lpMem );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID ReadINI( VOID ) 
{
    WORD cSDEntry, cLastDialed;
    DWORD cComma; 

    POINT ptLeftTop;

    char szName[ TAPIMAXCALLEDPARTYSIZE];
    char szTemp[ TAPIMAXCALLEDPARTYSIZE ];

    char szNum[MAXBUFSIZE];
    char szFieldName[MAXBUFSIZE];
    char szPt[MAXBUFSIZE];
    HRESULT hr;

    // get speed dial settings from INI file
    for(cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
    {
		
        hr = StringCbPrintfA(szFieldName,sizeof(szFieldName), "Name%d", cSDEntry);
        assert_str(hr);

        GetPrivateProfileString (
									"Speed Dial Settings",
									szFieldName,
									gszNULL,
									szName,
									TAPIMAXCALLEDPARTYSIZE - 1,
									gszINIfilename
								);

        hr = StringCbPrintfA(szFieldName, sizeof(szFieldName), "Number%d", cSDEntry);
        assert_str(hr);

        GetPrivateProfileString ( 
									"Speed Dial Settings",
									szFieldName,
									gszNULL,
									gszSDNumber[cSDEntry],
									TAPIMAXDESTADDRESSSIZE - 1,
									gszINIfilename
								);        

		if ( !lstrcmp( gszNULL, szName ) )
		{
			hr = StringCbCopyA( szName,sizeof(szName), gszSDNumber[ cSDEntry ]);
			assert_str(hr);

		}
		

		FitTextToButton( ghWndMain, IDD_DSPEEDDIAL1 + cSDEntry - 1, szName );

		AmpersandCompensate( szName, szTemp );
		SetDlgItemText (
						ghWndMain,
						IDD_DSPEEDDIAL1 + cSDEntry - 1,
						(LPCSTR)szTemp
					   ); // Label the speed dial button
	}

    
	// set up last dialed numbers in combo box (read from INI)
	for(cLastDialed = 1; cLastDialed <= NLASTDIALED; ++cLastDialed)
	{
		hr = StringCbPrintfA(szFieldName, sizeof(szFieldName), "Last dialed %d", cLastDialed);
		assert_str(hr);

		GetPrivateProfileString (
									"Last dialed numbers",
									szFieldName,
									gszNULL,
									szNum,
									MAXBUFSIZE - 1,
									gszINIfilename
								);
		if ( szNum[0] ) // i.e. if szNum isn't simply "" - if we read something
						// from the INI - put it in the combo box
			SendDlgItemMessage(
								ghWndMain,
								IDD_DCOMBO,
								CB_ADDSTRING,
								0,
								(LPARAM)(LPCSTR)szNum
							  );
	}

	// set defaults
	ptLeftTop.x = 100; 
	ptLeftTop.y = 100;

	// set the window position based on the INI data
	GetPrivateProfileString (
								"Preference",
								"Main Window Left/Top",
								gszNULL,
								szPt,
								MAXBUFSIZE - 1,
								gszINIfilename
							);

	if ( szPt[0] ) 
	{
		cComma = (int) strcspn(szPt, ",");
		szPt[cComma] = 0;
		ptLeftTop.x = atoi(szPt);
		
		// a possibly absurd check to see that the string
		// wasn't akin to "320," with no second entry
		if ( *(szPt + cComma + 1 ) ) 
			ptLeftTop.y = atoi( szPt + cComma + 1 );

		// check to see that the box is on the screen - the upper left
		// must be on the screen, along with a 50x50 box below and to
		// the right of it
		if ( ptLeftTop.x < 0
			|| ptLeftTop.x + 50 >= GetSystemMetrics(SM_CXSCREEN)
			|| ptLeftTop.y < 0
			|| ptLeftTop.y + 50 >= GetSystemMetrics(SM_CYSCREEN)            
		   )
		{
			ptLeftTop.x = 100; // set defaults if the box is off of the screen
			ptLeftTop.y = 100; // set defaults if the box is off of the screen
		}
	}

	SetWindowPos (
					ghWndMain,
					NULL,
					ptLeftTop.x,
					ptLeftTop.y,
					0,
					0,
					SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER
				 );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID ButtonFontSetup(VOID)
    {
    HDC hdc;
    int IDD;

    // define the fonts for the buttons
    hdc = GetDC(GetDesktopWindow());

    ghFontBtn = CreateFont(
        (-10)*GetDeviceCaps(hdc, LOGPIXELSY)/72,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        PROOF_QUALITY,
        VARIABLE_PITCH | FF_SWISS,
        (LPSTR)"Arial"
        );

    ghFontBtnText = CreateFont(
        (-6)*GetDeviceCaps(hdc, LOGPIXELSY)/72,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        PROOF_QUALITY,
        VARIABLE_PITCH | FF_SWISS,
        NULL
        );

    ghFontBtnStar = CreateFont(
        (-18)*GetDeviceCaps(hdc, LOGPIXELSY)/72,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        SYMBOL_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        PROOF_QUALITY,
        VARIABLE_PITCH | FF_DONTCARE,
        (LPSTR)"Symbol"
        );

    ReleaseDC(GetDesktopWindow(), hdc);


    // set the fonts for the buttons
    if(ghFontBtn)
        {
        // set fonts on number buttons
            for(IDD = IDD_DBUTTON1; IDD <= IDD_DBUTTON0; ++IDD)
            // the order is IDD_DBUTTON1, 2, 3, ..., 0 (thus from 1 to 0)
            SendMessage(
                GetDlgItem(ghWndMain, IDD),
                WM_SETFONT,
                (WPARAM)ghFontBtn,
                0L
                );
        
        // set fonts on * and # buttons
        SendMessage(
            GetDlgItem(ghWndMain, IDD_DBUTTONSTAR),
            WM_SETFONT,
            (WPARAM)ghFontBtnStar,
            0L
            );
        SendMessage(
                    GetDlgItem(ghWndMain, IDD_DBUTTONPOUND),
            WM_SETFONT,
            (WPARAM)ghFontBtnStar,
            0L
            );
        }
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
// Draws a 3D button within rcBtn on hDC
VOID DrawButton(HDC hDC, RECT rcBtn, BOOL fHighlighted)
    {
    HPEN hPenPrev, hPenShadow, hPenHighlight, hPenBlack;
    HBRUSH hBrushPrev, hBrushFace;
    int RopPrev;

    --rcBtn.right;
    --rcBtn.bottom;

    // set up pens/brush
    hPenShadow = CreatePen(PS_SOLID,0,GetSysColor(COLOR_3DSHADOW));
    hPenHighlight = CreatePen(PS_SOLID,0,GetSysColor(COLOR_3DHILIGHT));
    hPenBlack = GetStockObject(BLACK_PEN);
    hBrushFace = GetSysColorBrush(COLOR_3DFACE);
    
    // get current state so we can put it back at the end of DrawButton
    hPenPrev = SelectObject(hDC, hPenBlack);
    RopPrev = SetROP2(hDC, R2_COPYPEN);
    hBrushPrev = SelectObject(hDC, hBrushFace);

    PatBlt(
        hDC,
        rcBtn.left + 1,
        rcBtn.top + 1,
        rcBtn.right - rcBtn.left - 1,
        rcBtn.bottom - rcBtn.top - 1,
        PATCOPY
        );
    
    if(fHighlighted)
        {
        SelectObject(hDC, hPenBlack);
        MoveToEx(hDC, rcBtn.left, rcBtn.bottom - 1, NULL);
        LineTo(hDC, rcBtn.left, rcBtn.top);  //  _
        LineTo(hDC, rcBtn.right, rcBtn.top); // |

        SelectObject(hDC, hPenHighlight);
        MoveToEx(hDC, rcBtn.right, rcBtn.top, NULL);
        LineTo(hDC, rcBtn.right, rcBtn.bottom);
        LineTo(hDC, rcBtn.left - 1, rcBtn.bottom); // _|

        SelectObject(hDC, hPenShadow);
        MoveToEx(hDC, rcBtn.left + 1, rcBtn.bottom - 2, NULL);
        LineTo(hDC, rcBtn.left + 1, rcBtn.top + 1);
        LineTo(hDC, rcBtn.right - 1, rcBtn.top + 1);
        }
    else
        {
        SelectObject(hDC, hPenHighlight);
        MoveToEx(hDC, rcBtn.left, rcBtn.bottom - 1, NULL);
        LineTo(hDC, rcBtn.left, rcBtn.top);  //  _
        LineTo(hDC, rcBtn.right, rcBtn.top); // |

        SelectObject(hDC, hPenBlack);
        MoveToEx(hDC, rcBtn.right, rcBtn.top, NULL);
        LineTo(hDC, rcBtn.right, rcBtn.bottom);
        LineTo(hDC, rcBtn.left - 1, rcBtn.bottom); // _|

        SelectObject(hDC, hPenShadow);
        MoveToEx(hDC, rcBtn.left + 1, rcBtn.bottom - 1, NULL);
        LineTo(hDC, rcBtn.right - 1, rcBtn.bottom - 1);
        LineTo(hDC, rcBtn.right - 1, rcBtn.top);
        }

    // put everything back how it was
    SetROP2(hDC, RopPrev);
    SelectObject(hDC, hBrushPrev);
    SelectObject(hDC, hPenPrev);

    DeleteObject(hPenBlack);
    DeleteObject(hPenShadow);
    DeleteObject(hPenHighlight);    
    DeleteObject(hBrushFace);
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID DrawButtonText(HDC hDC, RECT rcBtn, BOOL fHighlighted, UINT IDD_Btn)
    {    
    char szLine1[MAXBUFSIZE]; // text in button up to '\n'
    LPSTR pszLine2; // text in button after '\n'
    int BkModePrev;
    HFONT hFontPrev = NULL;
    TEXTMETRIC tm;
    RECT rcText = rcBtn;
           
    BkModePrev = SetBkMode(hDC, TRANSPARENT);

    GetDlgItemText(ghWndMain, IDD_Btn, szLine1, MAXBUFSIZE);
    pszLine2 = strstr(szLine1, "\n");
    if(pszLine2)
        {
        *pszLine2 = 0; // 1 -> "TEST1 \n TEST2" becomes "TEST 1 \0"
        ++pszLine2; // now 2 -> " TEST 2"
        }

    // now szLine1 points to the null terminated first string and
    // pszLine2 points to either the null terminated second string or NULL
    // if there was no second string

    if(szLine1[0])
        {
        if(ghFontBtnText && pszLine2)
            hFontPrev = SelectObject(hDC, ghFontBtnText);
        
        GetTextMetrics(hDC, &tm);
        rcText.bottom = (rcBtn.bottom + rcBtn.top)/2 - 2;
        rcText.top = rcText.bottom - (tm.tmHeight - 1);

        if(pszLine2 == NULL)
            OffsetRect(&rcText, 0, (rcText.bottom - rcText.top)/2);

        if(fHighlighted)
            OffsetRect(&rcText, 1, 1);

        DrawText(hDC, szLine1, -1, &rcText, DT_SINGLELINE | DT_CENTER);

        if(hFontPrev)
            SelectObject(hDC, hFontPrev);
        }
    if(pszLine2) // recall that pszLine2 == NULL to represent no second string
        {
        GetTextMetrics(hDC, &tm);
        if(IDD_Btn == IDD_DBUTTONSTAR || IDD_Btn == IDD_DBUTTONPOUND)
            rcText.top = (rcBtn.bottom + rcBtn.top)/2 - (tm.tmHeight)/2;
        else
            rcText.top = (rcBtn.bottom + rcBtn.top)/2 - 2;
        rcText.bottom = rcText.top + tm.tmHeight;

        if(fHighlighted)
            OffsetRect(&rcText, 1, 1);

        DrawText(hDC, pszLine2, -1, &rcText, DT_SINGLELINE | DT_CENTER);
        }

    SetBkMode(hDC, BkModePrev);
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID DisableDialButtons(BOOL fDisable)
{
	int IDD;

	// Disable/enable Dial button
	EnableWindow( GetDlgItem( ghWndMain, IDD_DDIAL ),!fDisable) ;

	// Disable/enable Speed dial buttons
	for ( IDD = IDD_DSPEEDDIAL1; IDD <= IDD_DSPEEDDIAL8; ++IDD )
	{
		EnableWindow(GetDlgItem(ghWndMain, IDD),!fDisable);
	}
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID DialerCleanup(VOID)
    {
    RECT rc;
    WORD cItem; // count of numbers in combo box
    DWORD cLastDialed;
    char szPt[MAXBUFSIZE];
    char szNumber[TAPIMAXDESTADDRESSSIZE];
    char szFieldName[MAXBUFSIZE];
    HRESULT hr;

    CloseTAPI(); // unregister and line close

    if(!IsIconic(ghWndMain)) // if the window is not minimized, record position
        {
        GetWindowRect(ghWndMain, &rc);
        hr = StringCbPrintfA(szPt, sizeof(szPt),  "%ld, %ld", rc.left, rc.top);
        assert_str(hr);

        WritePrivateProfileString(
            "Preference",
            "Main Window Left/Top",
            szPt,
            gszINIfilename
            );
        }

    cItem = (WORD)SendDlgItemMessage(ghWndMain, IDD_DCOMBO, CB_GETCOUNT, 0, 0);

    // write out last dialed numbers from combo box (write to INI)
    for(cLastDialed = 1; cLastDialed <= NLASTDIALED; ++cLastDialed)
        {
        if(cLastDialed <= cItem)
            SendDlgItemMessage(
                ghWndMain,
                IDD_DCOMBO,
                CB_GETLBTEXT,
                cLastDialed - 1, // it's a zero-based count
                (LPARAM)(LPCSTR)szNumber);

        else
            szNumber[0] = 0;

        hr = StringCbPrintfA(szFieldName, sizeof(szFieldName), "Last dialed %d", cLastDialed);
        assert_str(hr);

        WritePrivateProfileString(
            "Last dialed numbers",
            szFieldName,
            szNumber,            
            gszINIfilename
            );

        }

    DestroyWindow(ghWndMain);
    ghWndMain = NULL;

    DeleteObject(ghFontBtn);
    DeleteObject(ghFontBtnText);
    DeleteObject(ghFontBtnStar);
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
// unregister and line close
VOID CloseTAPI(VOID) 
{

	// unregister as call manager
	lineRegisterRequestRecipient (
									ghLineApp,
									0, // registration instance
									LINEREQUESTMODE_MAKECALL,
									FALSE
								 );

	if ( gCurrentLineInfo.hLine )
	{
		lineClose ( gCurrentLineInfo.hLine );
		gfCurrentLineAvail = FALSE;
		gCurrentLineInfo.hLine = (HLINE)((ULONG_PTR)NULL);
	}

	lineShutdown(ghLineApp);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HICON hIcon;
	HRESULT hr;

        switch (msg)
        {
            case WM_INITDIALOG:
                hIcon = LoadIcon( ghInst, (LPCSTR) MAKEINTRESOURCE( IDI_DIALER ) );
                return TRUE;

            case WM_SYSCOMMAND:
                switch( (DWORD) wParam )
                {
                    case SC_CLOSE:                    
                        PostQuitMessage(0);
                }
                break;

            case WM_INITMENUPOPUP:
                // if edit menu
                if ( LOWORD(lParam) == 1 ) 
                {
                    UINT wEnable;

                    if ( GetParent( GetFocus() ) != GetDlgItem( ghWndMain, IDD_DCOMBO ) )
                    {
                        wEnable = MF_GRAYED;
                    }
                    else
                    {
                        LONG lSelect = (LONG)SendDlgItemMessage (
                            ghWndMain,
                            IDD_DCOMBO,
                            CB_GETEDITSEL,
                            0,
                            0
                            );

                        if ( HIWORD( lSelect ) != LOWORD( lSelect ) )
                            wEnable = MF_ENABLED;
                        else
                            wEnable = MF_GRAYED;
                    }

                    EnableMenuItem((HMENU)wParam, IDM_EDIT_CUT, wEnable);
                    EnableMenuItem((HMENU)wParam, IDM_EDIT_COPY, wEnable);
                    EnableMenuItem((HMENU)wParam, IDM_EDIT_DELETE, wEnable);

                    // enable paste option is there is data 
                    // in the clipboard
                    if ( IsClipboardFormatAvailable( CF_TEXT ) )
                    {
                        if ( GetClipboardData ( CF_TEXT ) )
                        {
                            wEnable = MF_ENABLED;
                        }
                        else
                        {
                            wEnable = MF_GRAYED;
                        }
                    }
                    else
                    {
                        wEnable = MF_GRAYED;	
                    }

                }
                break;


            case WM_COMMAND:
            {
                char szName[TAPIMAXCALLEDPARTYSIZE] = {'\0'};
                char szNumber[TAPIMAXDESTADDRESSSIZE] = {'\0'};

                switch( LOWORD( (DWORD)wParam ) )
                {
                    // FILE menu
                    case IDM_EXIT:
                        PostQuitMessage(0);                    
                        return TRUE;


                        // EDIT menu
                    case IDM_EDIT_CUT:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_CUT, 0, 0);
                        return TRUE;

                    case IDM_EDIT_COPY:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_COPY, 0, 0);
                        return TRUE;

                    case IDM_EDIT_PASTE:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_PASTE, 0, 0);
                        return TRUE;

                    case IDM_EDIT_DELETE:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_CLEAR, 0, 0);
                        return TRUE;

                    case IDM_EDIT_SPEEDDIAL:
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_SD1),
                                        ghWndMain,
                                        (DLGPROC)SpeedDial1Proc,
                                        0
                                       );
                        SetFocus(GetDlgItem(ghWndMain, IDD_DDIAL));
                        return TRUE;

                        // TOOLS menu
                    case IDM_CONNECTUSING:
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_CONNECTUSING),
                                        ghWndMain,
                                        (DLGPROC)ConnectUsingProc,
                                        MENU_CHOICE
                                       );
                        return TRUE;

                    case IDM_LOCATION:
                    {
                        char szCanNumber[ TAPIMAXDESTADDRESSSIZE ] ;

                        // fetch the number to be dialed
                        if ( GetDlgItemText ( 
                                              ghWndMain,
                                              IDD_DCOMBO,
                                              szNumber,
                                              TAPIMAXDESTADDRESSSIZE
                                            )
                           )
                        {
                            // if a number exists, convert it to 
                            // its canonical form.
                            if ( !MakeCanonicalNumber ( szNumber, szCanNumber ) )
                            {
                                hr = StringCbCopyA( szCanNumber, sizeof(szCanNumber),szNumber); 
                                assert_str(hr);
				}
                        }
						

                        lineTranslateDialog (
                                             ghLineApp,
                                             0,
                                             TAPI_CURRENT_VERSION,
                                             ghWndMain,
                                             szCanNumber
                                            );
                        return TRUE;

                    }
                    // HELP menu

                    case IDM_ABOUT:
                        DialogBoxParam(
                                       ghInst,
                                       MAKEINTRESOURCE(IDD_ABOUT),
                                       ghWndMain,
                                       (DLGPROC)AboutProc,
                                       0
                                      );

                        return TRUE;


                        // Accelerator processing
                    case IDM_ACCEL_NUMTODIAL:
                        if(GetActiveWindow() == ghWndMain)
                            SetFocus(GetDlgItem(ghWndMain, IDD_DCOMBO));
                        return TRUE;


                        // Buttons
                    case IDD_DDIAL:

                    {
                        DWORD cSDEntry;
                        char szSDNumber[TAPIMAXDESTADDRESSSIZE];
                        char szFieldName[MAXBUFSIZE];

                        // check if number entered is dialable
                        if ( SendMessage (
                                          GetDlgItem(ghWndMain, IDD_DCOMBO),
                                          WM_GETTEXTLENGTH,
                                          0,
                                          0
                                         ) > 0 
                           )
                        {
                            // get the number to be dialed
                            GetDlgItemText (
                                            ghWndMain,
                                            IDD_DCOMBO,
                                            (LPSTR)szNumber,
                                            TAPIMAXDESTADDRESSSIZE
                                           );

                            // check if it is a speed dial number.  
                            // If so choose the name to be displayed.
                            for( cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
                            {
                                hr = StringCbPrintfA(szFieldName,  sizeof(szFieldName), "Number%d", cSDEntry);
                                assert_str(hr);

                                GetPrivateProfileString (
                                    "Speed Dial Settings",
                                    szFieldName,
                                    gszNULL,
                                    szSDNumber,
                                    TAPIMAXCALLEDPARTYSIZE - 1,
                                    gszINIfilename
                                    );

                                // if the number matches, get the name
                                if ( lstrcmp(szSDNumber, szNumber) == 0 )
                                {
                                    hr = StringCbPrintfA( szFieldName, sizeof(szFieldName),  "Name%d", cSDEntry);
                                    assert_str(hr);
                                    GetPrivateProfileString (
                                        "Speed Dial Settings",
                                        szFieldName,
                                        gszNULL,
                                        szName,
                                        TAPIMAXCALLEDPARTYSIZE - 1,
                                        gszINIfilename
                                        );
                                    break;
                                }
                            }

                            SetFocus( GetDlgItem( ghWndMain, IDD_DDIAL ) );

                            // once the currentline has been set
                            // using the connect proc
                            // the user must hit dial again 
                            if ( giCurrentLine == (DWORD)-1 )
                            {
                                DialogBoxParam (
                                                ghInst,
                                                MAKEINTRESOURCE(IDD_CONNECTUSING),
                                                ghWndMain,
                                                (DLGPROC)ConnectUsingProc,
                                                INVALID_LINE
                                               );
                            }
                            else
                            {
                                AddToRedialList(szNumber);
                                InitiateCall(szNumber, szName);
                            }
                        }
                        return TRUE;
                    }


                    case IDD_DBUTTON1:
                    case IDD_DBUTTON2:
                    case IDD_DBUTTON3:
                    case IDD_DBUTTON4:
                    case IDD_DBUTTON5:
                    case IDD_DBUTTON6:
                    case IDD_DBUTTON7:
                    case IDD_DBUTTON8:
                    case IDD_DBUTTON9:
                    case IDD_DBUTTON0:
                    case IDD_DBUTTONSTAR:
                    case IDD_DBUTTONPOUND:
                    {
                        int     i;
                        TCHAR   szBuffer[TAPIMAXDESTADDRESSSIZE+1];

                        static const char digits[] = { '1', '2', '3', '4',
                        '5', '6', '7', '8',
                        '9', '0', '*', '#' };

                        i = (int)SendDlgItemMessage(ghWndMain,
                                               IDD_DCOMBO,
                                               WM_GETTEXT,
                                               (WPARAM)TAPIMAXDESTADDRESSSIZE+1,
                                               (LPARAM)szBuffer);

                        if (i < TAPIMAXDESTADDRESSSIZE)
                        {
                            MoveMemory(szBuffer+gdwStartSel+1,
                                       szBuffer+gdwEndSel,
                                       i - ( gdwEndSel ) + 1 );

                            szBuffer[gdwStartSel] = digits[LOWORD(wParam) - IDD_DBUTTON1];

                            SendDlgItemMessage(ghWndMain,
                                               IDD_DCOMBO,
                                               WM_SETTEXT,
                                               0,
                                               (LPARAM)szBuffer);

                            gdwStartSel++;
                            gdwEndSel = gdwStartSel;
                        }

                        SetFocus(GetDlgItem(ghWndMain, IDD_DDIAL));
                        EnableWindow(GetDlgItem(ghWndMain, IDD_DDIAL), TRUE);

                        return TRUE;
                    }


                    case IDD_DCOMBO:

                        if (HIWORD(wParam) == CBN_SELENDOK)
                        {
                            EnableWindow( GetDlgItem(ghWndMain, IDD_DDIAL), TRUE );
                        }

                        if ((HIWORD(wParam) == CBN_SELENDOK) ||
                            (HIWORD(wParam) == CBN_SELENDCANCEL))
                        {

                            (DWORD)SendDlgItemMessage(ghWndMain,
                                IDD_DCOMBO,
                                CB_GETEDITSEL,
                                (WPARAM)&gdwStartSel,
                                (LPARAM)&gdwEndSel);
                            return FALSE;
                        }

                        if ( HIWORD( wParam ) == CBN_EDITCHANGE )
                        {
                            EnableWindow (
                                          GetDlgItem( ghWndMain, IDD_DDIAL ),
                                          (BOOL) GetWindowTextLength (
                                GetDlgItem (
                                            ghWndMain,
                                            IDD_DCOMBO
                                           )
                                )
                                         );
                            return TRUE;
                        }

                        break;

                    case IDD_DSPEEDDIAL1:
                    case IDD_DSPEEDDIAL2:
                    case IDD_DSPEEDDIAL3:
                    case IDD_DSPEEDDIAL4:
                    case IDD_DSPEEDDIAL5:
                    case IDD_DSPEEDDIAL6:
                    case IDD_DSPEEDDIAL7:
                    case IDD_DSPEEDDIAL8:
                    {
                        DWORD cSDEntry = LOWORD( (DWORD) wParam)  - IDD_DSPEEDDIAL1 + 1;
                        char szFieldName [MAXBUFSIZE];

                        // get information for the speed dial button
                        // from the INI file
                        hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Name%d", cSDEntry);                    
                        assert_str(hr);

                        GetPrivateProfileString (
                            "Speed Dial Settings",
                            szFieldName,
                            gszNULL,
                            szName,
                            TAPIMAXCALLEDPARTYSIZE - 1,
                            gszINIfilename
                                                );

                        hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "%s%d", "Number", cSDEntry);
                        assert_str(hr);

                        GetPrivateProfileString (
                            "Speed Dial Settings",
                            szFieldName,
                            gszNULL,
                            gszSDNumber[cSDEntry],
                            TAPIMAXDESTADDRESSSIZE - 1,
                            gszINIfilename
                                                );

                        // entry not set yet
                        if( gszSDNumber[cSDEntry][0] == 0 )
                        {
                            DialogBoxParam (
                                            ghInst,
                                            MAKEINTRESOURCE(IDD_SD2),
                                            ghWndMain,
                                            (DLGPROC)SpeedDial2Proc,
                                            MAKELPARAM(wParam,0)
                                           );
                        }
                        
                        // no line open
                        // once the currentline has been set
                        // using the connect proc
                        // the user must hit dial again 
                        else if ( giCurrentLine == (DWORD)-1)
                        {
                            DialogBoxParam (
                                            ghInst,
                                            MAKEINTRESOURCE(IDD_CONNECTUSING),
                                            ghWndMain,
                                            (DLGPROC)ConnectUsingProc,
                                            INVALID_LINE
                                           );
                        }
                        // entry is set and valid voice line is open
                        else
                        {
                            // add number to list box combo.
                            AddToRedialList( gszSDNumber[cSDEntry] );
                            InitiateCall( gszSDNumber[cSDEntry], szName );
                        }
                        break;
                    }
                           } // end switch (LOWORD((DWORD)wParam)) { ... }

                            break; // end case WM_COMMAND
                            }


                                            case WM_PAINT:
                                            {
                                                PAINTSTRUCT ps;


                                                BeginPaint(ghWndMain, &ps);

                                                if(IsIconic(ghWndMain))
                                                    DrawIcon(ps.hdc, 0, 0, hIcon);
                                                else
                                                {
                                                    HBRUSH hBrush;

                                                    hBrush = GetSysColorBrush( COLOR_3DFACE );
                                                    //                FillRect(ps.hdc, &ps.rcPaint, GetStockObject(LTGRAY_BRUSH));
                                                    FillRect(ps.hdc, &ps.rcPaint, hBrush);
                                                }

                                                EndPaint(ghWndMain, &ps);

                                                return TRUE;
                                            }


                                            case WM_DRAWITEM:
                                            {
                                                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
                                                BOOL fHighlighted = (BOOL)(SendDlgItemMessage(
                                                    ghWndMain,
                                                    lpdis->CtlID,
                                                    BM_GETSTATE,
                                                    0,
                                                    0
                                                    ) & 0x0004);

                                                DrawButton(lpdis->hDC, lpdis->rcItem, fHighlighted);
                                                DrawButtonText(
                                                    lpdis->hDC,
                                                    lpdis->rcItem,
                                                    fHighlighted,
                                                    lpdis->CtlID
                                                    );
                                                return TRUE;
                                            }


                                            case WM_CTLCOLORLISTBOX:
                                            case WM_CTLCOLORBTN:
                                            case WM_CTLCOLORSTATIC:
                                                SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
                                                return (BOOL)((ULONG_PTR)GetSysColorBrush( COLOR_3DFACE ));


                                            default:
                                                ;
                                                //            return DefDlgProc( hwnd, msg, wParam, lParam );
                                                //            return DefWindowProc( hwnd, msg, wParam, lParam );


                    } // switch (msg) { ... }

	return FALSE;
	}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID AddToRedialList( LPCSTR szNumber )
{
	// NLASTDIALED == 10
	WORD cNum;
	HWND hWndCombo = GetDlgItem(ghWndMain, IDD_DCOMBO);
	LRESULT nMatch;

	// if valid number
	if ( szNumber[0] ) 
	{
		// if list box has entries, check if this number
		// is already present.  If so delete old entry
		cNum = (WORD) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
		if ( cNum != 0 )
		{
			nMatch = SendMessage ( hWndCombo, CB_FINDSTRING, 0, (LPARAM)szNumber );
			if ( nMatch != CB_ERR )
			{
				SendMessage(hWndCombo, CB_DELETESTRING, nMatch, 0);
			}
			else 
			{
				// if the list is full, remove oldest
				if ( cNum == NLASTDIALED )
				{
					SendMessage( hWndCombo, CB_DELETESTRING, NLASTDIALED - 1, 0 );
				}
			}
		}
		SendMessage(hWndCombo, CB_INSERTSTRING, 0, (LPARAM)szNumber);
		SendMessage(hWndCombo, CB_SETCURSEL, 0, 0L);
		EnableWindow ( GetDlgItem( ghWndMain, IDD_DDIAL ), TRUE ); 
	}
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID InitiateCall ( LPCSTR szNumber, LPCSTR szName )
{
	HLINE hLine = (HLINE)((ULONG_PTR)NULL);
	HRESULT hr;

	DWORD errCode;

	// struct size info
	DWORD dwLTPSize	= sizeof ( LINETRANSLATEOUTPUT );
	DWORD dwNameLen	= lstrlen( szName ) + 1;
	DWORD dwLCPSize	= sizeof( LINECALLPARAMS );

	LPLINETRANSLATEOUTPUT lpTransOut = NULL;
	LPLINECALLPARAMS lpLineCallParams = NULL;

	char szCanNumber[ TAPIMAXDESTADDRESSSIZE ];

	DWORD dwRemainingLength ;

	// Open a line
	errCode = lineOpen (
						ghLineApp,
						giCurrentLine,
						&hLine,
						gCurrentLineInfo.dwAPIVersion,
						0,
						0, 
						LINECALLPRIVILEGE_NONE,
						0,
						NULL
					   );
	if (errCode)
	{
		errString ( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
		goto error;
	}

	
	// call translate address before dialing
	do
	{
		lpTransOut = (LPLINETRANSLATEOUTPUT) DialerAlloc( dwLTPSize );
		if ( !lpTransOut )
		{
			errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
			goto error;
		}
		lpTransOut-> dwTotalSize = dwLTPSize;


		if ( !MakeCanonicalNumber( szNumber, szCanNumber ) )
		{
			hr = StringCbCopyA( szCanNumber, sizeof(szCanNumber),szNumber);
			assert_str(hr);
			
		}
		
		
		errCode = lineTranslateAddress (
										ghLineApp,
										giCurrentLine,
										gCurrentLineInfo.dwAPIVersion,
										szCanNumber,
										0,
										0,
										lpTransOut
									   );
		if ( ((LONG)errCode) < 0 )
		{
			errString( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
			goto error;
		}
		
		if ( lpTransOut-> dwNeededSize <= lpTransOut->dwTotalSize  )
		{
			// ok we are done
			break;
		}
		else
		{
			dwLTPSize = lpTransOut-> dwNeededSize;
			DialerFree ( lpTransOut );
			lpTransOut = NULL;
		}

	} while ( TRUE );

	
	// if number dialed is 911, bring up a warning
	if ( Is911( lpTransOut) )
	{
		INT nRes = errString ( ghWndMain, ERR_911WARN, MB_ICONSTOP | MB_YESNO );
		if ( nRes == IDNO )
		{
			goto error;
		}
	}

	
	// set call parameters
	dwLCPSize += dwNameLen + lpTransOut-> dwDisplayableStringSize;

	lpLineCallParams = (LPLINECALLPARAMS) DialerAlloc( dwLCPSize );
	if ( !lpLineCallParams )
	{
		errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
		goto error;
	}

	lpLineCallParams->dwTotalSize = dwLCPSize;
	lpLineCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
	lpLineCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
	lpLineCallParams->dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
	lpLineCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
	lpLineCallParams->dwAddressID = giCurrentAddress;

	if ( szName[ 0 ] )
	{
		lpLineCallParams->dwCalledPartySize = dwNameLen;
		lpLineCallParams->dwCalledPartyOffset = sizeof( LINECALLPARAMS );

		
		dwRemainingLength = lpLineCallParams->dwTotalSize - sizeof(LINECALLPARAMS) -1 ;

		hr = StringCbCopyA ( 
					(LPSTR) lpLineCallParams + sizeof(LINECALLPARAMS), 
					dwRemainingLength,
					szName
			    );
		assert_str(hr);
	
	}

	lpLineCallParams-> dwDisplayableAddressSize = lpTransOut-> dwDisplayableStringSize;
	lpLineCallParams-> dwDisplayableAddressOffset = sizeof( LINECALLPARAMS ) + dwNameLen;
	dwRemainingLength = lpLineCallParams->dwTotalSize - sizeof(LINECALLPARAMS) - 
						lstrlen((LPSTR) lpLineCallParams + sizeof(LINECALLPARAMS))-1;
	if (dwRemainingLength >0)
	{
		hr = StringCbCopyA (
				(LPSTR) lpLineCallParams + sizeof(LINECALLPARAMS) + dwNameLen,
				dwRemainingLength,
				(LPSTR) lpTransOut + lpTransOut-> dwDisplayableStringOffset);
		assert_str(hr);
	}


	// save dialing information
	// Free old allocs.
	if ( gszCurrentName )
	{
		DialerFree ( gszCurrentName );
	}

	if ( gszCurrentNumber )
	{
		DialerFree ( gszCurrentNumber );
	}

	// save new stuff
	gszCurrentName = (LPSTR) DialerAlloc( dwNameLen );
	if ( !gszCurrentName )
	{
		errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
		goto error;
	}
	hr = StringCbCopyA ( gszCurrentName, dwNameLen, szName );
	assert_str(hr);
	
	gszCurrentNumber = (LPSTR) DialerAlloc( lpTransOut-> dwDisplayableStringSize );
	if ( !gszCurrentNumber )
	{
		errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
		goto error;
	}

	hr = StringCbCopyA ( 
			 gszCurrentNumber, 
			 lpTransOut-> dwDisplayableStringSize,
			 (LPSTR) lpTransOut + lpTransOut-> dwDisplayableStringOffset 
			);
	assert_str(hr);

	gCurrentLineInfo.hLine = hLine;
	ghCall = (HCALL)((ULONG_PTR)NULL);


	// finally make the call.
	gMakeCallRequestID = 0;

	gMakeCallRequestID = lineMakeCall ( 
										hLine, 
										&ghCall, 
										(LPSTR) lpTransOut + lpTransOut-> dwDialableStringOffset, 
										0, 
										lpLineCallParams 
									  );

	// async request ID 
	// - the call is going out
	if ( (LONG) gMakeCallRequestID > 0 ) 
	{
		gfCurrentLineAvail = FALSE;
		gfMakeCallReplyPending = TRUE;
		DialogBoxParam (
						ghInst,
						MAKEINTRESOURCE(IDD_DIALING),
						ghWndMain,
						(DLGPROC)DialingProc,
						0
					   );

	}

	else 
	{
		if ( gMakeCallRequestID == LINEERR_CALLUNAVAIL ) 
		{
			DialogBoxParam (
							ghInst,
							MAKEINTRESOURCE(IDD_CALLFAILED),
							ghWndMain,
							(DLGPROC)LineInUseProc,
							0
						   );
		}

		else
		{
			errString( ghWndMain, gMakeCallRequestID, MB_ICONEXCLAMATION | MB_OK );
		}

		DialerLineClose();
		gfCurrentLineAvail = TRUE;
	}

error :
	if ( lpLineCallParams )
	{
		DialerFree( lpLineCallParams );
	}

	if ( lpTransOut )
	{
		DialerFree( lpTransOut );
	}

	// if makecall did not succeed but line
	// was opened, close it.
	if ( ( gMakeCallRequestID <= 0 ) && ( gCurrentLineInfo.hLine ) )
	{
		DialerLineClose ();
		gfCurrentLineAvail = TRUE;
	}

	SetFocus( GetDlgItem( ghWndMain, IDD_DCOMBO ) );

	return;

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
DWORD GetLineInfo ( DWORD iLine, LPLINEINFO lpLineInfo )
{
	DWORD errCode = 0;
	DWORD dwNeededSize = 0; 
	LINEEXTENSIONID ExtensionID;

	LPSTR pszLineName = NULL; 
	LPLINEDEVCAPS lpDevCaps = NULL;
	HRESULT hr;


	errCode = lineNegotiateAPIVersion (
										ghLineApp,
										iLine,
										TAPI_VERSION_1_0,
										TAPI_CURRENT_VERSION,
										&( lpLineInfo->dwAPIVersion ),
										&ExtensionID
									  );
	if ( errCode )
	{
		GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
		goto error;
	}

	dwNeededSize = sizeof( LINEDEVCAPS );
	do
	{
		lpDevCaps = ( LPLINEDEVCAPS ) DialerAlloc( dwNeededSize );
		if ( !lpDevCaps )
		{
			GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
			errCode = LINEERR_NOMEM;
			goto error;
		}

		lpDevCaps->dwTotalSize = dwNeededSize;
		errCode = lineGetDevCaps (
									ghLineApp,
									iLine,
									lpLineInfo->dwAPIVersion,
									0,
									lpDevCaps
								 );
		if ( errCode )
		{
			GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
			goto error;
		}

		if ( lpDevCaps-> dwNeededSize <= lpDevCaps-> dwTotalSize )
		{
			break;
		}

		dwNeededSize = lpDevCaps->dwNeededSize;
		DialerFree( lpDevCaps );
		lpDevCaps = NULL;

	} while ( TRUE );


	lpLineInfo->nAddr = lpDevCaps->dwNumAddresses;
	lpLineInfo->fVoiceLine =
		( (lpDevCaps->dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE) != 0 );

	pszLineName = (LPSTR) DialerAlloc( MAXBUFSIZE );
	if ( !pszLineName )
	{
		errCode = LINEERR_NOMEM;
		goto error;
	}
	if ( lpDevCaps->dwLineNameSize > 0 )
	{
		hr = StringCbCopyA ( 
				 pszLineName, 
				 MAXBUFSIZE,
				 (LPSTR) lpDevCaps + lpDevCaps->dwLineNameOffset
				);
		assert_str(hr);
	}
	else 
	{
		hr = StringCbPrintfA ( pszLineName,MAXBUFSIZE, "Line %d", iLine );
		assert_str(hr);
	}

	hr = StringCbCopyA( lpLineInfo->szLineName, MAXBUFSIZE, pszLineName);
	assert_str(hr);
	
	lpLineInfo->dwPermanentLineID = lpDevCaps->dwPermanentLineID;

	
error:
	if ( lpDevCaps )
		DialerFree( lpDevCaps );

	if ( pszLineName )
		DialerFree( pszLineName );

	return errCode; 
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID GetLineInfoFailed ( DWORD iLine, LPLINEDEVCAPS lpDevCaps, LPLINEINFO lpLineInfo )
{
	if ( lpDevCaps ) 
		DialerFree(lpDevCaps);

	lpLineInfo->nAddr = 0;
	lpLineInfo->fVoiceLine = FALSE;
	lpLineInfo->dwAPIVersion = 0; 
	lpLineInfo->hLine = (HLINE)0;
	lpLineInfo->dwPermanentLineID = 0;
	lpLineInfo->szLineName[0] = 0;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LPSTR GetAddressName(DWORD iLine, DWORD iAddress)
{
	DWORD errCode = 0;
	DWORD dwNeededSize = 0;       
	LPSTR pszAddressName = NULL;
	LPLINEADDRESSCAPS lpAddressCaps = NULL;
	HRESULT hr;

	// allocate space for lineGetAddressCaps data
	dwNeededSize = sizeof( LINEADDRESSCAPS );
	
	do
	{
		lpAddressCaps = ( LPLINEADDRESSCAPS )DialerAlloc( dwNeededSize );
		if ( !lpAddressCaps )
		{
			goto error;
		}

		lpAddressCaps->dwTotalSize = dwNeededSize;
		errCode = lineGetAddressCaps (
										ghLineApp,
										iLine,
										iAddress,
										gCurrentLineInfo.dwAPIVersion,
										0,
										lpAddressCaps
									 );
		if ( errCode )
		{
			errString( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
			goto error;
		}

		if ( lpAddressCaps-> dwNeededSize <= lpAddressCaps-> dwTotalSize )
		{
			break;
		}

		dwNeededSize = lpAddressCaps->dwNeededSize;
		DialerFree( lpAddressCaps );
		lpAddressCaps = NULL;

	} while( TRUE );


	// get the address name
	pszAddressName = DialerAlloc( MAXBUFSIZE );
	if ( !pszAddressName )
	{
		goto error;
	}

	if ( lpAddressCaps-> dwAddressSize > 0 )
	{
		// keep string length bounded
			hr = StringCbCopyA( 
					pszAddressName, 
					MAXBUFSIZE,
					(LPSTR) lpAddressCaps + lpAddressCaps->dwAddressOffset
				   );
			assert_str(hr);
	}
	else 
	// use default name
	{
		hr = StringCbPrintfA(pszAddressName,MAXBUFSIZE, "Address %d", iAddress);
		assert_str(hr);
	}

error:
	if ( lpAddressCaps )
	{
		DialerFree( lpAddressCaps );
	}

	return pszAddressName;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK DialingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
		char szTemp[ TAPIMAXCALLEDPARTYSIZE ];

		case WM_INITDIALOG:
			// set global handle to window
			ghWndDialing = hwnd;

			AmpersandCompensate( gszCurrentName, szTemp );
            
			SetDlgItemText(hwnd, IDD_DGNUMBERTEXT, gszCurrentNumber);
			SetDlgItemText(hwnd, IDD_DGNAMETEXT, szTemp );
			break;

		case WM_COMMAND:
			switch ( LOWORD( (DWORD)wParam ) )
			{
				// hang up
				case IDCANCEL: 
					//gfDropping = TRUE;
					
					// if lineMakeCall has completed
					// only then drop call.
					if ( !gfMakeCallReplyPending && ghCall )
					{
						if ( ( gDropCallRequestID = lineDrop ( ghCall, NULL, 0 ) ) < 0 )
						{
							errString ( ghWndDialing, gDropCallRequestID, MB_ICONSTOP | MB_OK );
						}
					}
					else
					{
						DialerLineClose();
						gfCurrentLineAvail = TRUE;
						gfMakeCallReplyPending = FALSE;
					}

					ghWndDialing = NULL;
					EndDialog(hwnd, FALSE);

					return TRUE;


				// something else terminated the call
				// all we have to do is terminate this dialog box
				case IDOK: 
					ghWndDialing = NULL;
					EndDialog(hwnd, TRUE);

					return TRUE;
			}
			break;

		default:
		  ;
	}
	return FALSE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            return TRUE;
        }

        case WM_CLOSE:
            EndDialog(hwnd, TRUE);
            return TRUE;

        case WM_COMMAND:
            if(LOWORD((DWORD)wParam) == IDOK)
            {
                EndDialog(hwnd, TRUE);
                return TRUE;
            }

            break;
    }

    return FALSE;
    
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK ConnectUsingProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			BOOL fEnable;
			DWORD dwPriority;

			//
			// Is there any point in even showing this dialog box?
			if ( gnAvailDevices == 0 )
			{
				// Nope.  Let's tell the user what we don't like.
				errString ( ghWndMain, ERR_NOLINES, MB_ICONEXCLAMATION | MB_OK );

				EndDialog(hwnd, FALSE);
				return TRUE;
			}

			// if not brought up by InitializeTAPI()
			if ( lParam != INVALID_LINE ) 
			{
				// hide error text
				EnableWindow( GetDlgItem( hwnd, IDD_CUERRORTEXT ), FALSE );
			}

			// get list of lines into the line list box.
			fEnable = InitializeLineBox( GetDlgItem(hwnd, IDD_CULISTLINE) );
			EnableWindow( GetDlgItem( hwnd, IDD_CULISTLINE ), fEnable);

			// get list of addresses into the address list box.
			fEnable =	fEnable && 
						InitializeAddressBox (
												GetDlgItem(hwnd, IDD_CULISTLINE),
												GetDlgItem(hwnd, IDD_CULISTADDRESS)
											 );
			EnableWindow( GetDlgItem( hwnd, IDD_CULISTADDRESS ), fEnable );
			EnableWindow( GetDlgItem( hwnd, IDOK ), fEnable );

			EnableWindow( GetDlgItem( hwnd, IDD_CUPROPERTIES ), fEnable );

			lineGetAppPriority (
								"DIALER.EXE",
								0, // checking app priority for Assisted Telephony requests
								NULL,
								LINEREQUESTMODE_MAKECALL,
								NULL,
								&dwPriority
							   );
			CheckDlgButton(hwnd, IDD_CUSIMPLETAPICHKBOX, (dwPriority == 1));

			// if dwPriority == 1, we're supporting Assisted Telephony AND
			// have the highest priority.
			EnableWindow (
							GetDlgItem(hwnd, IDD_CUSIMPLETAPICHKBOX),
							gfRegistered
						 );

			return FALSE;
		}

		case WM_COMMAND:
		{
			switch ( LOWORD( (DWORD)wParam ) )
			{
				case IDD_CULISTLINE:
					if ( HIWORD( wParam ) == CBN_SELENDOK )
						// update address box
						InitializeAddressBox (
												GetDlgItem(hwnd, IDD_CULISTLINE),
												GetDlgItem(hwnd, IDD_CULISTADDRESS)
											 ); 
					break;

				case IDD_CUPROPERTIES:
				{
					HWND hW = GetDlgItem(hwnd, IDD_CULISTLINE);

					lineConfigDialog (	
										// device ID
										(DWORD) SendMessage (
																hW,
																CB_GETITEMDATA,
																(WORD) SendMessage(hW, CB_GETCURSEL, 0, 0),
																0
														    ), 
										hwnd,
										NULL
									 );
					break;
				}

				case IDOK:
				{
					HWND hwndBox;
					char szBuffer[MAXBUFSIZE];
					DWORD dwPriority;

					// Update line
					hwndBox = GetDlgItem( hwnd, IDD_CULISTLINE );
					giCurrentLine = (DWORD)SendMessage (
													hwndBox,
													CB_GETITEMDATA,
													SendMessage( hwndBox, CB_GETCURSEL, 0, 0 ),
													0
												);

					// base 10
					_itoa_s( gdwPLID[giCurrentLine], szBuffer,MAXBUFSIZE, 10 ); 
					WritePrivateProfileString (  
												"Preference",
												"Preferred Line",
												szBuffer,
												gszINIfilename
											  );


					// Update address
					hwndBox = GetDlgItem( hwnd, IDD_CULISTADDRESS );
					giCurrentAddress = (DWORD)SendMessage (
													hwndBox,
													CB_GETITEMDATA,
													SendMessage(hwndBox, CB_GETCURSEL, 0, 0),
													0
												   );

					_itoa_s( giCurrentAddress, szBuffer,MAXBUFSIZE, 10 ); 
					WritePrivateProfileString ( 
												"Preference",
												"Preferred Address",
												szBuffer,
												gszINIfilename
											  );


					// Update application priority
					if ( SendDlgItemMessage (
												hwnd,
												IDD_CUSIMPLETAPICHKBOX,
												BM_GETCHECK,
												0,
												0L
											)
						  == 0)
					{
						dwPriority = 0;
					}
					else   
					{
						dwPriority = 1;
					}

					lineSetAppPriority (
										"DIALER.EXE",
										0,
										NULL,
										LINEREQUESTMODE_MAKECALL,
										NULL,
										dwPriority
									   );

					EndDialog(hwnd, TRUE);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd, FALSE);
					return TRUE;
			}
		}

		default:
			  ;

	}

	return FALSE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK LineInUseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    LPARAM lNewParam = lParam;
    PTSTR  ptStr;


    switch(msg)
        {
        case WM_INITDIALOG:
            {
            switch(lParam)
                {
                case LINEDISCONNECTMODE_REJECT:
                    lNewParam = ikszDisconnectedReject;
                    break;

                case LINEDISCONNECTMODE_BUSY:
                    lNewParam = ikszDisconnectedBusy;
                    break;

                case LINEDISCONNECTMODE_NOANSWER:
                    lNewParam = ikszDisconnectedNoAnswer;
                    break;

                case LINEDISCONNECTMODE_CONGESTION:
                    lNewParam = ikszDisconnectedNetwork;
                    break;

                case LINEDISCONNECTMODE_INCOMPATIBLE:
                    lNewParam = ikszDisconnectedIncompatible;
                    break;

                case LINEDISCONNECTMODE_NODIALTONE:
                    lNewParam = ikszDisconnectedNoDialTone;
                    break;

                default:
                    lNewParam = ikszDisconnectedCantDo;
                    break;
                }
            return TRUE;
            }

        case WM_COMMAND:
            if(LOWORD((DWORD)wParam) == IDOK)
                {
                EndDialog(hwnd, TRUE);
                return TRUE;
                }
            break;

        default:
              ;
//            return DefDlgProc( hwnd, msg, wParam, lParam );

        }    


    if (ptStr = DialerAlloc( MAXBUFSIZE )) {

	    LoadString( ghInst, (UINT)lNewParam, ptStr, MAXBUFSIZE );

	    SetDlgItemText(
       	 hwnd,
	        IDD_CFTEXT,
       	 ptStr
	        );
	
	    DialerFree( ptStr );
    }


    return FALSE;
    }



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK SpeedDial1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static DWORD nCurrentSpeedDial;

	// buffer to store speed dial names till they are saved.
	static TCHAR szSDName[NSPEEDDIALS + 1][TAPIMAXDESTADDRESSSIZE];
	HRESULT hr;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			DWORD cSDEntry;
			DWORD idFirstEmpty = (DWORD) -1;

			char szName[TAPIMAXCALLEDPARTYSIZE];
			char szTemp[TAPIMAXCALLEDPARTYSIZE];
			char szFieldName[MAXBUFSIZE];

			// Retrieve speed dial info from INI file
			for(cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
			{
				hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Name%d", cSDEntry);        
				assert_str(hr);

				GetPrivateProfileString (
											"Speed Dial Settings",
											szFieldName,
											gszNULL,
											szSDName[ cSDEntry ],
											TAPIMAXCALLEDPARTYSIZE - 1,
											gszINIfilename
										);

				// set the first empty speed dial button
				if ( idFirstEmpty == -1 && 
					 szSDName[ cSDEntry ][0] == '\0' &&
					 gszSDNumber[ cSDEntry ][ 0 ] == '\0' )
					idFirstEmpty = cSDEntry;

				hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Number%d", cSDEntry);
				assert_str(hr);

				GetPrivateProfileString (
											"Speed Dial Settings",
											szFieldName,
											gszNULL,
											gszSDNumber[cSDEntry],
											MAXBUFSIZE - 1,
											gszINIfilename
										);

				// get a copy of the name for editing
				// if name is empty, use the number as the
				// name.
				if ( lstrcmp( gszNULL, szSDName[ cSDEntry ] ) )
				{			
					hr = StringCbCopyA( szName, sizeof(szName), szSDName[ cSDEntry]);
					assert_str(hr);
				}
				
				else
				{
					hr = StringCbCopyA( szName, sizeof(szName), gszSDNumber[ cSDEntry ]);
					assert_str(hr);
				}

			
				FitTextToButton( hwnd, IDD_SD1SPEEDDIAL1 + cSDEntry - 1, szName );
				AmpersandCompensate( szName, szTemp );

				SetDlgItemText (
								hwnd,
								IDD_SD1SPEEDDIAL1 + cSDEntry - 1,
								(LPCSTR) szTemp
							   ); 

			}

			// for the edit speed dial dialog
			// limit the lengths of text
			SendDlgItemMessage (
								hwnd,
								IDD_SD1EDITNAME,
								EM_LIMITTEXT,
								(WPARAM)(TAPIMAXCALLEDPARTYSIZE - 1),
								0
							   );

			SendDlgItemMessage (
								hwnd,
								IDD_SD1EDITNUMBER,
								EM_LIMITTEXT,
								(WPARAM)(TAPIMAXDESTADDRESSSIZE - 1),
								0
							   );

			// select the first empty button
			// nothing empty, then edit #1
			if ( -1 == idFirstEmpty ) 
			{
				nCurrentSpeedDial = 1;
				SetDlgItemText(
								hwnd,
								IDD_SD1EDITNAME,
								(LPCSTR) szSDName[ 1 ]
							  );

				SetDlgItemText(
								hwnd,
								IDD_SD1EDITNUMBER,
								(LPCSTR) gszSDNumber[ 1 ]
							  );
			}
			else
			{
				nCurrentSpeedDial = idFirstEmpty;
			}

			SetFocus( GetDlgItem( hwnd, IDD_SD1EDITNAME ) );
			return FALSE;
		}

		case WM_COMMAND:
		{
			char szName[TAPIMAXDESTADDRESSSIZE ];
			char szTemp[ TAPIMAXCALLEDPARTYSIZE ];

			switch( LOWORD( (DWORD) wParam ) )
			{
				case IDOK:
				{
					DWORD cSDEntry;
					char szFieldName[MAXBUFSIZE];

					// save new speed dial settings
					for ( cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry )
					{
						hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Name%d", cSDEntry);
						assert_str(hr);

						WritePrivateProfileString ( 
													"Speed Dial Settings",
													szFieldName,
													szSDName [cSDEntry],
													gszINIfilename
												  );

						hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Number%d", cSDEntry);
						assert_str(hr);

						WritePrivateProfileString ( 
													"Speed Dial Settings",
													szFieldName,
													gszSDNumber[cSDEntry],
													gszINIfilename
												  );

						// set the text for the corresponding 
						// main window button
						if ( szSDName[ cSDEntry ][ 0 ] == '\0' )
						{
							hr = StringCbCopyA( szName,  sizeof(szName), gszSDNumber[ cSDEntry ]);
							assert_str(hr);
						}
						else
						{
							hr = StringCbCopyA( szName,  sizeof(szName), szSDName[ cSDEntry ]);
							assert_str(hr);
						}


						FitTextToButton( 
										ghWndMain, 
										IDD_DSPEEDDIAL1 + cSDEntry - 1, 
										szName 
									   );

						AmpersandCompensate( szName, szTemp );
						SetDlgItemText ( 
										ghWndMain,
										IDD_DSPEEDDIAL1 + cSDEntry - 1,
										(LPCSTR) szTemp
									   );
					}

					EndDialog(hwnd, TRUE);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd, FALSE);
					return TRUE;

				case IDD_SD1SPEEDDIAL1:
				case IDD_SD1SPEEDDIAL2:
				case IDD_SD1SPEEDDIAL3:
				case IDD_SD1SPEEDDIAL4:
				case IDD_SD1SPEEDDIAL5:
				case IDD_SD1SPEEDDIAL6:
				case IDD_SD1SPEEDDIAL7:
				case IDD_SD1SPEEDDIAL8:

					nCurrentSpeedDial =	LOWORD( (DWORD) wParam ) - IDD_SD1SPEEDDIAL1 + 1;

					SetDlgItemText (
									hwnd,
									IDD_SD1EDITNAME,
									szSDName [ nCurrentSpeedDial ]
								   );
					SetDlgItemText (
									hwnd,
									IDD_SD1EDITNUMBER,
									gszSDNumber[nCurrentSpeedDial]
								   );

					SetFocus( GetDlgItem( hwnd, IDD_SD1EDITNAME ) );
					SendDlgItemMessage(
										hwnd,
										IDD_SD1EDITNAME,
										EM_SETSEL,
										0,
										MAKELPARAM(0, -1)
									);
					break;

				case IDD_SD1EDITNAME:
					if ( HIWORD( wParam ) == EN_CHANGE )
					{

						GetDlgItemText (
										hwnd,
										IDD_SD1EDITNAME,
										szName,
										TAPIMAXCALLEDPARTYSIZE
									   );

						// if there is no name, label the button with
						// the number
						if ( szName[ 0 ] == '\0' )
						{
							szSDName[ nCurrentSpeedDial ][ 0 ] = '\0';
							hr = StringCbCopyA( szName,  sizeof(szName), gszSDNumber[ nCurrentSpeedDial ]);
							assert_str(hr);

						}
						else
						{
							hr = StringCbCopyA( szSDName[ nCurrentSpeedDial ], sizeof( szSDName[ nCurrentSpeedDial ]), szName);
							assert_str(hr);

						}

						FitTextToButton ( 
											hwnd, 
											IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
											szName
										);
						AmpersandCompensate( szName, szTemp );

						SetDlgItemText (
										hwnd,
										IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
										szTemp
									   );
					}
					break;

				case IDD_SD1EDITNUMBER:
					if ( HIWORD( wParam ) == EN_CHANGE )
					{
						GetDlgItemText (
										hwnd,
										IDD_SD1EDITNUMBER,
										gszSDNumber[nCurrentSpeedDial],
										TAPIMAXDESTADDRESSSIZE
									   );

						if ( gszSDNumber[ nCurrentSpeedDial ][ 0 ] == '\0' )
						{
							GetDlgItemText (
											hwnd,
											IDD_SD1EDITNAME,
											szName,
											TAPIMAXDESTADDRESSSIZE
										   );

							if ( szName[ 0 ] == '\0' )
							{
								SetDlgItemText ( 
												hwnd,
												IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
												szName
											   );

							}
						}
					}
					break;
				} // switch(LOWORD((DWORD)wParam))
			break;

		} // case WM_COMMAND:

		default:
			  ;

	} // switch(msg)

	return FALSE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK SpeedDial2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static DWORD nCurrentSpeedDial;
	HRESULT hr;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			char szFieldName [MAXBUFSIZE];
			char szName [TAPIMAXCALLEDPARTYSIZE];

			nCurrentSpeedDial = LOWORD( lParam ) - IDD_DSPEEDDIAL1 + 1;

			// retrieve speed dial button info
			hr = StringCbPrintfA(szFieldName, sizeof(szFieldName),  "Name%d", nCurrentSpeedDial);
			assert_str(hr);

			GetPrivateProfileString (
										"Speed Dial Settings",
										szFieldName,
										gszNULL,
										szName,
										TAPIMAXCALLEDPARTYSIZE - 1,
										gszINIfilename
									);
			SetDlgItemText (
							hwnd,
							IDD_SD2EDITNAME,
							szName
						   );

			SetDlgItemText (
							hwnd,
							IDD_SD2EDITNUMBER,
							gszSDNumber[nCurrentSpeedDial]
						   );

			// limit the lengths of the texts
			SendDlgItemMessage (
								hwnd,
								IDD_SD2EDITNAME,
								EM_LIMITTEXT,
								(WPARAM)(TAPIMAXCALLEDPARTYSIZE - 1),
								0
							   );

			SendDlgItemMessage (
								hwnd,
								IDD_SD2EDITNUMBER,
								EM_LIMITTEXT,
								(WPARAM)(TAPIMAXDESTADDRESSSIZE - 1),
								0
							   );


			SetFocus( GetDlgItem( hwnd, IDD_SD2EDITNAME ) );
			SendDlgItemMessage (
								hwnd,
								IDD_SD2EDITNAME,
								EM_SETSEL,
								0,
								MAKELPARAM(0, -1)
							   );

			return FALSE;
		}

		case WM_COMMAND:
		{
			char szName[ TAPIMAXCALLEDPARTYSIZE ];
			char szTemp[ TAPIMAXCALLEDPARTYSIZE ];
			char szFieldName[MAXBUFSIZE];

			switch ( LOWORD( (DWORD) wParam ) )
			{
				case IDOK:
				case IDD_SD2SAVEANDDIAL:
				{
					GetDlgItemText (
									hwnd,
									IDD_SD2EDITNAME,
									(LPTSTR) szName,
									TAPIMAXCALLEDPARTYSIZE
								   );

					GetDlgItemText (
									hwnd,
									IDD_SD2EDITNUMBER,
									(LPTSTR) gszSDNumber[nCurrentSpeedDial],
									TAPIMAXCALLEDPARTYSIZE
								   );

					hr = StringCbPrintfA ( szFieldName, sizeof(szFieldName),  "Name%d", nCurrentSpeedDial );
					assert_str(hr);

					WritePrivateProfileString ( 
												"Speed Dial Settings",
												szFieldName,
												szName,
												gszINIfilename
											  );
	
					hr = StringCbPrintfA ( szFieldName, sizeof(szFieldName),  "Number%d", nCurrentSpeedDial );
					assert_str(hr);

					WritePrivateProfileString ( 
												"Speed Dial Settings",
												szFieldName,
												gszSDNumber[nCurrentSpeedDial],
												gszINIfilename
											  );

					// update main window buttons
					// is only number has been entered, label button with it.
					if ( szName[ 0 ] == '\0' )
					{
						hr = StringCbCopyA( szName, sizeof(szName), gszSDNumber[ nCurrentSpeedDial ]);
						assert_str(hr);
					}

					FitTextToButton ( 
										ghWndMain, 
										IDD_DSPEEDDIAL1 + nCurrentSpeedDial - 1, 
										(LPSTR) szName
									);

					AmpersandCompensate( szName, szTemp );
					
					SetDlgItemText ( 
									ghWndMain,
									IDD_DSPEEDDIAL1 + nCurrentSpeedDial - 1,
									(LPCSTR)szTemp
								   );

					// if save and dial, then post dial message to main window
					if ( LOWORD( (DWORD) wParam ) == IDD_SD2SAVEANDDIAL )
					{
						PostMessage (
										ghWndMain,
										WM_COMMAND,
										MAKEWPARAM (
													nCurrentSpeedDial + IDD_DSPEEDDIAL1 - 1,
													BN_CLICKED
												   ),
										(LPARAM) GetDlgItem (
																ghWndMain,
																nCurrentSpeedDial + IDD_DSPEEDDIAL1 - 1
															)
									);
					}
					EndDialog(hwnd, TRUE);
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(hwnd, FALSE);
					return TRUE;

				case IDD_SD2EDITNAME:
				case IDD_SD2EDITNUMBER:
					if ( HIWORD( wParam ) == EN_CHANGE)
					{
						EnableWindow (
										GetDlgItem( hwnd, IDD_SD2SAVEANDDIAL ),
										GetWindowTextLength ( GetDlgItem( hwnd, IDD_SD2EDITNUMBER ) ) > 0
									 );
					}
					break;

			} // switch(LOWORD((DWORD)wParam))
			break;
		}


		default:
			  ;

	} // switch(msg)

	return FALSE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID CALLBACK
tapiCallback (
				DWORD   hDevice,
				DWORD   dwMsg,
				DWORD   dwCBInstance,
				DWORD   dwParam1,
				DWORD   dwParam2,
				DWORD   dwParam3
		    )
{
	switch (dwMsg)
	{
		INT errCode;

		case LINE_ADDRESSSTATE:
			break;

		case LINE_CALLINFO:
			break;

		case LINE_CALLSTATE:
			if ( (HCALL)hDevice != ghCall )
				return;

			switch ( dwParam1 ) // new state
			{
				case LINECALLSTATE_IDLE:

					// tell "Dialing" window to terminate
					if ( ghWndDialing )
					{
						SendMessage ( 
										ghWndDialing,
										WM_COMMAND,
										MAKEWPARAM( IDOK, 0 ),
										0
									);
					}

					// tapi call cleanup
					if ( !gfMakeCallReplyPending && ghCall )
					{
						if ( ( errCode = lineDeallocateCall( ghCall ) ) < 0 )
						{
							errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
						}
						ghCall = (HCALL)((ULONG_PTR)NULL);
					}
					DialerLineClose();
					gfCurrentLineAvail = TRUE;

					// update main window
					DisableDialButtons( FALSE );
					break;

				case LINECALLSTATE_BUSY:
					tapiCallback (
									hDevice,
									dwMsg,
									dwCBInstance,
									LINECALLSTATE_DISCONNECTED,
									LINEDISCONNECTMODE_BUSY,
									dwParam3
								 );
					break;

				case LINECALLSTATE_SPECIALINFO:
					tapiCallback (
									hDevice,
									dwMsg,
									dwCBInstance,
									LINECALLSTATE_DISCONNECTED,
									LINEDISCONNECTMODE_UNREACHABLE,
									dwParam3
								 );
					break;

				case LINECALLSTATE_DISCONNECTED:
				{
					BOOL fCallOK;
					DWORD LineDisconnectMode;


					if ( dwParam2 == 0 )
						LineDisconnectMode = LINEDISCONNECTMODE_NORMAL;
					else
						LineDisconnectMode = dwParam2;

					fCallOK = ( LineDisconnectMode == LINEDISCONNECTMODE_NORMAL		||
								LineDisconnectMode == LINEDISCONNECTMODE_UNKNOWN	||
								LineDisconnectMode == LINEDISCONNECTMODE_PICKUP		||
								LineDisconnectMode == LINEDISCONNECTMODE_FORWARDED	||
								LineDisconnectMode == LINEDISCONNECTMODE_UNAVAIL
							  );

					
					if ( !gfMakeCallReplyPending && ghCall )
					{
						//gfDropping = TRUE;
						if ( ( gDropCallRequestID = lineDrop ( ghCall, NULL, 0 ) ) < 0 )
						{
							errString ( ghWndMain, gDropCallRequestID, MB_ICONSTOP | MB_OK );
						}
					}

					if ( !fCallOK )
						DialogBoxParam (
										ghInst,
										MAKEINTRESOURCE(IDD_CALLFAILED),
										ghWndMain,
										(DLGPROC)LineInUseProc,
										LineDisconnectMode
									   );
					break;
				}
			}
			break;


		case LINE_CLOSE:
			if ( gCurrentLineInfo.hLine == (HLINE)hDevice )
			{
				errString(ghWndMain, ERR_LINECLOSE, MB_ICONEXCLAMATION | MB_OK );
				gCurrentLineInfo.hLine = (HLINE)((ULONG_PTR)NULL);
				gfCurrentLineAvail = FALSE;
				DisableDialButtons(FALSE);                
			}
			break;

		case LINE_CREATE:
			// dwParam1 is the new device's ID
			if ( dwParam1 >= gnAvailDevices ) 
			{
				DWORD* gnAddrTemp;
				DWORD iLine;
				LINEINFO LineInfo;

				// we record new device's address count.

                // we are assuming here that we're just adding a new
                // line and it's sequential and it's the last one

                gnAvailDevices = dwParam1 + 1;
                
				if (!(gnAddrTemp = (DWORD *) DialerAlloc ( sizeof(DWORD) * (int)(gnAvailDevices))))
					break;

				for ( iLine = 0; iLine < (gnAvailDevices-1); ++iLine )
					gnAddrTemp[iLine] = gnAddr[iLine];

				DialerFree( gnAddr );

				// we have effectively added one more
				// space in the gnAddr array
				gnAddr = gnAddrTemp; 

				if ( GetLineInfo( dwParam1, &LineInfo ) != ERR_NONE )
					break;

 				gnAddr[dwParam1] = LineInfo.nAddr;
			}
			break;

		case LINE_DEVSPECIFIC:
			break;

		case LINE_DEVSPECIFICFEATURE:
			break;

		case LINE_GATHERDIGITS:
			break;

		case LINE_GENERATE:
			break;

		case LINE_LINEDEVSTATE:
			if ( dwParam1 & LINEDEVSTATE_REINIT )
			{
				if(dwParam2 != 0) 
				{
					// this is another msg translated into REINIT
					tapiCallback( hDevice, dwParam2, dwCBInstance, dwParam3, 0, 0 );
				}
				else 
				{
					// Re-initialize TAPI
					gfNeedToReinit = TRUE;
				}
			}

			if ( dwParam1 & LINEDEVSTATE_REMOVED )
			{
				DialerLineClose();
				tapiCallback(hDevice, LINE_CLOSE, dwCBInstance, 0, 0, 0); // is this needed?
			}
			break;

		case LINE_MONITORDIGITS:
			break;

		case LINE_MONITORMEDIA:
			break;

		case LINE_MONITORTONE:
			break;

		// async reply from lineMakeCall() or lineDrop()
		case LINE_REPLY:

			// reply for lineMakeCall 
			if ( (LONG) dwParam1 == gMakeCallRequestID )
			{
				// error on make call
				if ( dwParam2 != ERR_NONE )
				{
					// Get rid of the Dialing Dialog box if it's up
					if ( ghWndDialing )
					{
					   SendMessage(
									ghWndDialing,
									WM_COMMAND,
									MAKEWPARAM(IDOK,0),
									0
								  );
					}

					if ( dwParam2 == LINEERR_CALLUNAVAIL )
					{
						DialogBoxParam (
										ghInst,
										MAKEINTRESOURCE(IDD_CALLFAILED),
										ghWndMain,
										(DLGPROC)LineInUseProc,
										0
									   );
					}
					else
					{
						errString ( ghWndMain, dwParam2, MB_ICONEXCLAMATION | MB_OK );
					}

					ghCall = (HCALL)((ULONG_PTR)NULL);
					DialerLineClose();
					gfCurrentLineAvail = TRUE;
				}

				gfMakeCallReplyPending = FALSE;
			}

			// reply from lineDrop()
			if ( (LONG) dwParam1 == gDropCallRequestID )
			{
				//gfDropping = FALSE;

				// tell "Dialing" window to terminate
				if ( ghWndDialing )
				{
					SendMessage ( 
									ghWndDialing,
									WM_COMMAND,
									MAKEWPARAM( IDOK,0 ),
									0
								);
				}
				
				// tapi call cleanup
				if ( dwParam2 == ERR_NONE )
				{
					if ( !gfMakeCallReplyPending && ghCall )
					{
						if ( ( errCode = lineDeallocateCall( ghCall ) ) < 0 )
						{
							errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
						}
						ghCall = (HCALL)((ULONG_PTR)NULL);
					}
				}
				DialerLineClose ();
				gfCurrentLineAvail = TRUE;
			}

			break;

		case LINE_REQUEST:
			// Simple TAPI request
			if ( dwParam1 == LINEREQUESTMODE_MAKECALL )
			{
				gfCallRequest = TRUE;
			}
			break;
	}
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL InitializeLineBox(HWND hwndLineBox)
{

	DWORD iLine, iItem, iItemCurrent = (DWORD)-1;
	DWORD errCode;

	LPLINEINFO lpLineInfo = NULL;

	// allocate buffer for storing LINEINFO for all of 
	// the available lines.  Always allocate space for 
	// at least one line
	if ( gnAvailDevices == 0 )
	{
		lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof(LINEINFO) );
	}
	else
	{
		lpLineInfo = (LPLINEINFO) DialerAlloc ( sizeof(LINEINFO) * (int)gnAvailDevices );
	}

	// if no space was set aside...
	if ( lpLineInfo == NULL ) 
		return LINEERR_NOMEM;

	// fill lpLineInfo[] and open each line
	for ( iLine = 0; iLine < gnAvailDevices; ++iLine )
	{
		// skip remaining processing for this line if it didn't open
		if ( GetLineInfo( iLine, &lpLineInfo[iLine] ) != ERR_NONE )
		{
			continue;
		}

		iItem = (DWORD)SendMessage (
								hwndLineBox,
								CB_ADDSTRING,
								0,
								(LPARAM)(LPCSTR)(lpLineInfo[iLine].szLineName)
							);

		// error, bail out.
		if ( iItem == CB_ERR || iItem == CB_ERRSPACE )
		{
            if (lpLineInfo)
            {
                DialerFree(lpLineInfo);
            }
            
			return FALSE; 
		}

		errCode = (DWORD)SendMessage (
								hwndLineBox,
								CB_SETITEMDATA,
								(WPARAM)iItem,
								(LPARAM)iLine
							  );

		if ( iLine == giCurrentLine )
		{
			iItemCurrent = iItem;
		}
		else if ( iItemCurrent != -1 && iItem <= iItemCurrent )
		{
			// if the item we are putting is before the
			// "current" item, we must increment iItemCurrent
			// to reflect that something is being placed before
			// it, due to sorting
			++iItemCurrent;
		}
	}

	if ( iItemCurrent == (DWORD)-1 )
		iItemCurrent = 0;

	if ( SendMessage( hwndLineBox, CB_GETCOUNT, 0, 0) != 0 )
	{
		SendMessage( hwndLineBox, CB_SETCURSEL, (WPARAM)iItemCurrent, 0 );
		return TRUE;
	}

    DialerFree(lpLineInfo);
	return FALSE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL InitializeAddressBox( HWND hwndLineBox, HWND hwndAddressBox )
{
	DWORD errCode;
	DWORD iAddress, iItem, iItemCurrent = (DWORD)-1;
	DWORD iLineBoxCurrent;
	LPSTR pszAddressName;

	if ( SendMessage( hwndLineBox, CB_GETCOUNT, 0, 0 ) == 0 )
	{
		return FALSE;
	}

	// select current entry in line box
	iLineBoxCurrent = (DWORD)SendMessage ( 
									hwndLineBox,
									CB_GETITEMDATA,
									SendMessage( hwndLineBox, CB_GETCURSEL, 0, 0 ),
									0
								  );
	// empty address list box
	SendMessage ( hwndAddressBox, CB_RESETCONTENT, 0, 0); 

	// get all the address for this line
	for ( iAddress = 0; iAddress < gnAddr[iLineBoxCurrent]; ++iAddress )
	{
		pszAddressName = GetAddressName( iLineBoxCurrent, iAddress );

		// if this address if fails, try the next one
		if ( !pszAddressName )
			continue; 

		iItem = (DWORD)SendMessage (
								hwndAddressBox,
								CB_ADDSTRING,
								0,
								(LPARAM) (LPCSTR) (pszAddressName)
							);

		// error, bail out
		if ( iItem == CB_ERR || iItem == CB_ERRSPACE )
			return FALSE; 

		errCode = (DWORD)SendMessage (
								hwndAddressBox,
								CB_SETITEMDATA,
								(WPARAM) iItem,
								(LPARAM) iAddress
							  );

		if ( iLineBoxCurrent == giCurrentLine )
		{
			if(iAddress == giCurrentAddress)
			{
				iItemCurrent = iItem;
			}
			else 
			{
				// if the item we are putting is before the
				// "current" item, we must increment iItemCur
				// to reflect that something is being placed
				// before it, due to sorting
				if ( iItemCurrent != -1 && iItem <= iItemCurrent )
				{
					++iItemCurrent; 
				}
			}
		}

		DialerFree( pszAddressName );
	}
	
	if ( iLineBoxCurrent != giCurrentLine )
	{	
		// if we're not looking at the current line
		// then highlight address 0
		iItemCurrent = 0;
	}

	SendMessage (
					hwndAddressBox,
					CB_SETCURSEL,
					iItemCurrent,
					0
				);
	return TRUE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID ManageAssistedTelephony(VOID)
{
	DWORD errCode;
	LPLINEREQMAKECALL lpRequestBuffer;

	lpRequestBuffer = (LPLINEREQMAKECALL) DialerAlloc( sizeof( LINEREQMAKECALL ) );
	if ( !lpRequestBuffer ) 
	{
		goto error;
	}

    // bring window to front
    SetForegroundWindow(ghWndMain);
    
	// get next queued request.
	errCode = lineGetRequest (
								ghLineApp,
								LINEREQUESTMODE_MAKECALL,
								lpRequestBuffer
	
							 );
	if ( errCode )
	{
		// if no more call requests pending, reset flag.
		if ( errCode == LINEERR_NOREQUEST )
		{
			gfCallRequest = FALSE;
		}
		else
		{
			errString ( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
		}
		goto error;
	}

	
	// if a line has not been selected
	if ( giCurrentLine == (DWORD)-1 )
	{
		if (!DialogBoxParam (
						ghInst,
						MAKEINTRESOURCE(IDD_CONNECTUSING),
						ghWndMain,
						(DLGPROC) ConnectUsingProc,
						INVALID_LINE
					  ))
        {
            // failed to get a line
            goto error;
        }
	}

	// make the reuested call.
	InitiateCall (
					lpRequestBuffer->szDestAddress,
					lpRequestBuffer->szCalledParty
				 );

error :
	if ( lpRequestBuffer )
	{
		DialerFree( lpRequestBuffer );
	}
	return;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID DialerLineClose()
{
	DWORD errCode;

	if ( gCurrentLineInfo.hLine )
	{
		if ( errCode = lineClose ( gCurrentLineInfo.hLine ) )
		{
			errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
		}
		gCurrentLineInfo.hLine = (HLINE)((ULONG_PTR)NULL);
	}


	// re-initialize TAPI if it needs to be re-initialized
	if ( gfNeedToReinit ) 
	{
		CloseTAPI();

		errCode = InitializeTAPI();
		if(errCode)
		{
			errString(ghWndMain, errCode, MB_APPLMODAL | MB_ICONEXCLAMATION );
			DialerCleanup(); // terminate program if we can't init
			return;
		}

		errCode = lineRegisterRequestRecipient (
												ghLineApp,
												0, 
												LINEREQUESTMODE_MAKECALL,
												TRUE
											   );
		if (errCode)
		{
			errString(ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
		}

		gfNeedToReinit = FALSE;
	}
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
int errString( HWND hWndOwner, UINT errCode, UINT uFlags )
{
	PTSTR ptStrTitle;
	PTSTR ptStrError;
	int   nResult;
    BOOL  bDefault = FALSE;


	ptStrTitle = DialerAlloc( MAXBUFSIZE );
	if ( NULL == ptStrTitle )
	{
	   // Now, _this_ is a problem.
	   return 0;
	}

	ptStrError = DialerAlloc( MAXBUFSIZE );
	if ( NULL == ptStrError )
	{
	   // Now, _this_ is a problem.
	   DialerFree( ptStrTitle);
	   return 0;
	}

	switch(errCode)
	{
		case ERR_NOLINES:
			errCode = ikszErrNoVoiceLine;
			break;

		case ERR_NOVOICELINE:
			errCode = ikszErrNoVoiceLine;
			break;
    
		case ERR_LINECLOSE:
			errCode = ikszErrLineClose;
			break;

		case ERR_911WARN:
			errCode = ikszWarningFor911;
			break;
    
		case ERR_NEWDEFAULT:
			errCode = ikszWarningNewDefault;
			break;

		case LINEERR_NODRIVER:
			errCode = ikszErrLineInitNoDriver;
			break;
    
		case LINEERR_NODEVICE:
			errCode = ikszErrLineInitNoDevice;
			break;
    
		case LINEERR_INIFILECORRUPT:
			errCode = ikszErrLineInitBadIniFile ;
			break;
    
		case LINEERR_NOMEM:
			errCode = ikszErrOOM;
			break;
    
		case LINEERR_INCOMPATIBLEAPIVERSION:
			errCode = ikszErrLineInitWrongDrivers ;
			break;
    
		case LINEERR_OPERATIONFAILED:
			errCode = ikszErrTAPI;
			break;
    
		case LINEERR_INVALADDRESS:
			errCode = ikszErrInvalAddress;
			break;
    
		case LINEERR_ADDRESSBLOCKED:
			errCode = ikszErrAddrBlocked;
			break;
    
		case LINEERR_BILLINGREJECTED:
			errCode = ikszErrBillingRejected;
			break;
    
		case LINEERR_RESOURCEUNAVAIL:
		case LINEERR_ALLOCATED:
		case LINEERR_INUSE:
			errCode = ikszErrResUnavail;
			break;
    
		case LINEERR_NOMULTIPLEINSTANCE:
			errCode = ikszErrNoMultipleInstance;
			break;
    
		case LINEERR_INVALCALLSTATE:
			errCode = ikszErrInvalCallState;
			break;

        case LINEERR_INVALCOUNTRYCODE:
            errCode = ikszErrInvalidCountryRegionCode;
            break;
    
		default:
            bDefault = TRUE;
            break;

	}


    if (bDefault)
    {
        // if using default error, get TAPI's
        // error message from FormatError()
        if (!FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                           (LPCVOID)GetModuleHandle(TEXT("TAPI32.DLL")),
                           (DWORD)TAPIERROR_FORMATMESSAGE(errCode),
                           0,
                           (LPTSTR)ptStrError,
                           MAXBUFSIZE,
                           NULL))
        {
            // if this fails, fall back on default
            LoadString( ghInst, ikszErrDefault, ptStrError, MAXBUFSIZE);
        }

    }
    else    // not the default error message
    {

        if ( 0 == LoadString( ghInst, errCode, ptStrError, MAXBUFSIZE ) )
        {
#if DBG
            TCHAR buf[200];
            hr = StringCbPrintfA( buf, sizeof(buf),  "Cannot load string: hinst:0x%p errcode: %ld", ghInst,(DWORD)errCode);
            assert_str(hr);

            OutputDebugString( buf );
            MessageBox( NULL, buf, "Dialer", MB_OK);
#endif

            LoadString( ghInst, ikszErrDefault, ptStrError, MAXBUFSIZE );
        }
    }

    LoadString( ghInst, ikszWarningTitle, ptStrTitle, MAXBUFSIZE );

	nResult = MessageBox( hWndOwner, ptStrError, ptStrTitle, uFlags );


	DialerFree( ptStrTitle );
	DialerFree( ptStrError );


	return nResult;
}


/*
 *	Name :
 *		FitTextToButton 
 *
 *	Arguements :
 *		hDlg 		handle for the dialog in which this button is embedded
 *		nButtonID 	button id of this button
 *		szName		Name to fit on the button. Max size TAPIMAXCALLEDPARTYSIZE
 *
 *	Return :
 *		None
 *
 *	Comments :
 *		Function first checks to see if the button text specified fits in the
 *		button.  If it does not it truncates it appropriately and adds trailing
 *		ellipses.  
 */
VOID FitTextToButton ( HWND hDlg, INT nButtonID, LPSTR szName )
{

	HDC hDC = 0;
	HFONT hFont = 0, hOldFont = 0;
	HWND hWnd = 0;
	HRESULT hr;

	do
	{
		// calculate number of chars. that can fit on 
		// the button
		int	nLen;
		RECT rect;
		SIZE size;
		POINT pt;
		char buf [TAPIMAXCALLEDPARTYSIZE + 1];

		// get button dimensions
		hWnd = GetDlgItem( hDlg, nButtonID );
		if ( hWnd == NULL )
			break;

		if ( !GetClientRect( hWnd, &rect ) )
			break;
		
		// get character dimensions
		hDC = GetDC( hWnd );
		if ( hDC == NULL )
			break;

		hFont = (HFONT) SendMessage( hWnd, WM_GETFONT, 0, 0 );
		if ( hFont == NULL )
			hOldFont = SelectObject( hDC, GetStockObject( SYSTEM_FONT ) );
		else
			hOldFont = SelectObject( hDC, hFont );

		// add an extra char at the end to compensate for
		// leading space,
		hr = StringCbCopyA ( buf, sizeof(buf),szName );
		assert_str(hr);

		nLen = lstrlen( buf );
		buf [ nLen ] = 'X';
		buf [ nLen + 1 ] = '\0';

		if ( !GetTextExtentPoint32( hDC, buf, nLen + 1, &size ) )
			break;

		pt.x = size.cx;
		if ( !LPtoDP( hDC, &pt, 1 ) )
			break;

		// check if name fits on button
		if (  pt.x > rect.right )
		{
			// find how much of the name fits 
			int i = 0;

			nLen = lstrlen( szName );
			for ( i = 0; i < nLen; i++ )
			{
				buf[ i ] = szName[ i ];
				// an extra char is stuffed to compensate for the
				// leading space left by the left alignment
				buf [ i + 1 ] = 'X';
				buf [ i + 2 ] = '\0';

				// break out in cases of error condition
				if ( !GetTextExtentPoint32( hDC, buf, i + 2, &size ) )
				{
					i = nLen;
					break;
				}

				pt.x = size.cx;
				if ( !LPtoDP( hDC, &pt, 1 ) )
				{
					i = nLen;
					break;
				}
					
				if ( pt.x > rect.right ) 
					break;
			}

            // error
			if ( i >= nLen ) 
				break;
			
			// name is too long. truncate and add ellipses
			if (nLen > 3)
			{
				szName [i - 3] = '\0';
				hr = StringCbCatNA( szName,TAPIMAXCALLEDPARTYSIZE, "...", 3);
				assert_str(hr);
			}
				
			

		
			
		}
			

	} while( FALSE );

	if ( hDC )
	{
		SelectObject( hDC, hOldFont );
		ReleaseDC( hWnd, hDC );
	}

	return;
}



/*
 *	Name :
 *		Is911
 *
 *	Arguements :
 *		lpTransOut -		Translated address contained the dialable string
 *
 *	Returns
 *		TRUE -				If number to be dialed (in the US) is prefixed by 911 
 *		FALSE -				Otherwise
 *
 * Comments
 *
 */
BOOL Is911 ( LPLINETRANSLATEOUTPUT lpTransOut )
{

	DWORD i = 0, j = 0;	
	LPSTR lpDialDigits = (LPSTR)lpTransOut + lpTransOut-> dwDialableStringOffset;
	char sz3Pref [ 4 ] = "";


	// if this is not the US
	if ( lpTransOut-> dwCurrentCountry != 1 )
		return FALSE;

	// skip non digit characters and extract
	// the first 3 digits in the dialable number
	for ( i = 0, j = 0; i < lpTransOut-> dwDialableStringSize ; i++ )
	{
		if ( ISDIGIT( lpDialDigits[i] ) )
		{
			sz3Pref[ j++ ] = lpDialDigits [ i ];
			sz3Pref[ j ] = '\0'; 
			if ( j == 3 )
				break;
		}
	}

	if ( !lstrcmp( sz3Pref, "911" ) )
	{
		return TRUE;
	}

	return FALSE;
}


/*
 *	Name :
 *		MakeCanonicalNumber
 *
 *	Arguements :
 *		szNumber 		Number to convert into canonical form. Max size TAPIMAXDESTADDRESSSIZE
 *		szCanNumber		Canonical representation of number specified in szNumber
 *
 *	Return :
 *		TRUE			If the conversion was successful.
 *		FALSE			otherwise
 *
 *	Comments :
 *		Function first checks if given number is already in canonical form.
 *		If it is, it returns.  If it is not, then it performs the conversion.
 */
 
BOOL MakeCanonicalNumber ( LPCSTR szNumber, LPSTR szCanNumber )
{
	char szDigits [ TAPIMAXDESTADDRESSSIZE ];
	char szPref [ TAPIMAXDESTADDRESSSIZE ];

	BOOL bRes = FALSE;

	INT errCode = -1;
	INT nLenPref, nLenDigits, cPos, i;

	DWORD dwSize = 0;
	DWORD dwInd = 0;

	LPLINETRANSLATEOUTPUT lpTransOut = NULL;
	LPLINETRANSLATECAPS lpTransCaps = NULL;
	HRESULT hr;


	dwSize = sizeof ( LINETRANSLATEOUTPUT );
	do
	{
		lpTransOut = ( LPLINETRANSLATEOUTPUT ) DialerAlloc ( dwSize );
		if ( !lpTransOut )
		{
			errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
			goto error;
		}
			
		lpTransOut-> dwTotalSize = dwSize;
		errCode = lineTranslateAddress (
											ghLineApp,
											giCurrentLine,
											gCurrentLineInfo.dwAPIVersion,
											szNumber,
											0,
											0,
											lpTransOut
									   );
		if ( errCode )
		{
			goto error;
		}

		if ( lpTransOut-> dwNeededSize <= lpTransOut-> dwTotalSize )
			break;

		dwSize = lpTransOut-> dwNeededSize;
		DialerFree( lpTransOut );

	} while( TRUE );

	
	// check if input number is already in 
	// canonical form.
	if ( lpTransOut-> dwTranslateResults & LINETRANSLATERESULT_CANONICAL )
		goto error;

	// ensure country is the USA.
	if ( lpTransOut-> dwCurrentCountry != 1 )
		goto error;


	// Extract the digits from given string
	// allowed formatting characters that are ignored are
	// space, (, ), -, . 
	// presence of other characters will render the string invalid.
	
	// find the prefix of the address upto the | mark.
	// the rest of the string can be ignored
	nLenPref = (int) strcspn ( szNumber, "|" );
	hr = StringCbCopyNA( szPref, sizeof(szPref),szNumber, nLenPref );
	assert_str(hr);
	

	// if string is not composed entirely of digits
	// and allowable formating characters, quit conversion
	if ( strspn( szPref, " 0123456789()-." ) != (size_t) nLenPref )
		goto error;

	// collect digits ignoring formating characters.
	szDigits[ 0 ] = '\0';
	for ( i = 0, nLenDigits = 0; i < nLenPref; i++ )
	{
		if ( ISDIGIT( szNumber[ i ] ) )
		{
			szDigits[ nLenDigits++ ] = szNumber[ i ];
		}
	}
	szDigits[ nLenDigits ] = '\0';

	// if "internal" number
	if ( nLenDigits < LOCAL_NUMBER )
		goto error;

	switch ( nLenDigits )
	{
		// Local number ( 7 digits) preceeded by a 0/1
		// Strip leading 0/1 and treat as a local number
		case EXTENDED_LOCAL_NUMBER:
			if ( szDigits[ 0 ] == '0' || szDigits[ 0 ] == '1' )
			{
				nLenDigits--;
				memmove( szDigits, &(szDigits[1]), nLenDigits );
				szDigits[ nLenDigits ] = '\0';

				cPos = (int) strcspn( szPref, "01" );
				nLenPref--;
				memmove( &(szPref[ cPos ]), &(szPref[ cPos + 1 ]), nLenPref - cPos );
				szPref[ nLenPref ] = '\0';
			}
			else
			{
				goto error;
			}

		case LOCAL_NUMBER :
		{
			LPLINELOCATIONENTRY lpLocLst;

			// if leading digit is 0 or 1, it is 
			// illegal in the US
			if ( szDigits[ 0 ] == '0' || szDigits[ 0 ] == '1' )
			{
				goto error;
			}

			// get area code nformation for local number
			dwSize = sizeof( LINETRANSLATECAPS );
			do
			{
				lpTransCaps = (LPLINETRANSLATECAPS) DialerAlloc( dwSize );
				if ( !lpTransCaps )
				{
					errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
					goto error;
				}

				lpTransCaps-> dwTotalSize = dwSize;
				errCode = lineGetTranslateCaps (
												ghLineApp,
												gCurrentLineInfo.dwAPIVersion,
												lpTransCaps
											   );
				if ( errCode )
				{
					errString( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
					goto error;
				}

				if ( lpTransCaps-> dwNeededSize <= lpTransCaps-> dwTotalSize )
				{
					break;
				}

				dwSize = lpTransCaps-> dwNeededSize;
				DialerFree( lpTransCaps );

			} while ( TRUE );

			// skip entries till you locate information for current location
			dwSize = sizeof( LINELOCATIONENTRY );
			lpLocLst = (LPLINELOCATIONENTRY) ( (LPSTR) lpTransCaps + 
												lpTransCaps-> dwLocationListOffset );

			for ( dwInd = 0; dwInd < lpTransCaps-> dwNumLocations ; dwInd++ )
			{
				if ( lpLocLst[ dwInd ].dwPermanentLocationID == lpTransCaps-> dwCurrentLocationID )
					break;
			}
			
			// current location no found ?????
			// login error
			if ( dwInd == lpTransCaps-> dwNumLocations )
			{
				goto error;
			}

			// construct canonical form as
			szCanNumber[ 0 ]= '\0';
			hr = StringCbCopyA( szCanNumber, TAPIMAXDESTADDRESSSIZE,"+1 (");
			assert_str(hr);
			hr = StringCbCatA( szCanNumber,TAPIMAXDESTADDRESSSIZE, (LPSTR) lpTransCaps + lpLocLst[ dwInd ].dwCityCodeOffset);
			assert_str(hr);
			hr = StringCbCatA( szCanNumber,TAPIMAXDESTADDRESSSIZE, ") ");
			assert_str(hr);
			hr = StringCbCatA( szCanNumber,TAPIMAXDESTADDRESSSIZE, szDigits);
			assert_str(hr);



			cPos = (int) strcspn( szNumber, "|" );
			if ( cPos != lstrlen( szNumber ) )
			{
				hr = StringCbCatA( szCanNumber, TAPIMAXDESTADDRESSSIZE,&(szNumber[ cPos ])); 
				assert_str(hr);
			}

			bRes = TRUE;
			break;
		}

		case EXTENDED_LONG_DISTANCE_NUMBER:
		{
			// Long distance number ( 10 digits) preceeded by a 0/1
			// Strip leading 0/1 and treat as a long distance number
			if ( szDigits[ 0 ] == '0' || szDigits[ 0 ] == '1' )
			{
				nLenDigits--;
				memmove( szDigits, &(szDigits[1]), nLenDigits );
				szDigits[ nLenDigits ] = '\0';

				cPos = (int) strcspn( szPref, "01" );
				nLenPref--;
				memmove( &(szPref[ cPos ]), &(szPref[ cPos + 1 ]), nLenPref - cPos );
				szPref[ nLenPref ] = '\0';
			}
			else
			{
				goto error;
			}

		}

		case LONG_DISTANCE_NUMBER:
		{
			// if first or fourth digit is 0/1, illegal number
			if ( szDigits[ 0 ] == '0' || szDigits[ 0 ] == '1' ||
				 szDigits[ 3 ] == '0' || szDigits[ 3 ] == '1' )
			{
				goto error;
			}
			

			hr = StringCbCopy( szCanNumber, TAPIMAXDESTADDRESSSIZE,"+1 (" );
			assert_str(hr);
			hr = StringCbCatNA( szCanNumber, TAPIMAXDESTADDRESSSIZE,szDigits,3);
			assert_str(hr);
			hr = StringCbCatA( szCanNumber, TAPIMAXDESTADDRESSSIZE,") ");
			assert_str(hr);
			hr = StringCbCatA( szCanNumber,TAPIMAXDESTADDRESSSIZE, &(szDigits[ 3 ]));
			assert_str(hr);
		
			bRes = TRUE;
		}
		break;

		default :
			goto error;
	}

error:
	if ( lpTransOut )
		DialerFree( lpTransOut );

	if ( lpTransCaps )
		DialerFree( lpTransCaps );

	return bRes;
}


/*
 *	Name :
 *		AmpersandCompensate
 *
 *	Arguements :
 *		lpszSrc		:	Src string containing &s
 *		lpszDst		:	Dest string   
 *
 *	Return :
 *
 *	Comments :
 *		Copies string pointed to by lpszSrc to lpszDst character by
 *		character.  If an & is encountered in this process in lpszSrc
 *		it is copied as && into lpszDst.
 *		Assumes lpszDst and lpszSrc are of size TAPIMAXCALLEDPARTYSIZE
 */
VOID AmpersandCompensate ( LPCSTR lpszSrc, LPSTR lpszDst )
{
	// check if the name has an & in it.  If so replace
	// it with &&.
	INT cCnt, cInd;

	for ( cCnt = 0, cInd = 0; 
		  cInd < TAPIMAXCALLEDPARTYSIZE; 
		  cInd++, cCnt++ )
	{
		if ( lpszSrc[ cCnt ] == '&' )
		{
			lpszDst[ cInd++ ] = '&';
		}
		lpszDst[ cInd ] = lpszSrc[ cCnt ];

		if ( lpszSrc[ cCnt ] == '\0' )
			break;
	}

	// make sure string is null terminated.
	lpszDst[ TAPIMAXCALLEDPARTYSIZE - 1 ] = '\0';

	return;
}


 /*
 *	Name :
 *		AmpersandDeCompensate
 *
 *	Arguements :
 *		lpszSrc		:	Src string containing &s
 *		lpszDst		:	Dest string   
 *
 *	Return :
 *
 *	Comments :
 *		Copies string pointed to by lpszSrc to lpszDst character by
 *		character.  If an && is encountered in this process in lpszSrc
 *		it is copied as & into lpszDst.
 *		Assumes lpszDst and lpszSrc are of size TAPIMAXCALLEDPARTYSIZE
 */
 VOID AmpersandDeCompensate ( LPCSTR lpszSrc, LPSTR lpszDst )
 {
	// check if the name has an & in it.  If so replace
	// it with &&.
	INT cCnt, cInd;

	for ( cCnt = 0, cInd = 0; 
		  cInd < TAPIMAXCALLEDPARTYSIZE; 
		  cInd++, cCnt++ )
	{
		if  ( ( lpszSrc[ cInd ] == '&' ) &&
			  ( lpszSrc[ cInd + 1 ] == '&' ) )
		{
			cInd++;
		}
		lpszDst[ cCnt ] = lpszSrc[ cInd ] ;

		if ( lpszSrc [ cInd ] == '\0' )
		{
			break;
		}
	}

	lpszDst[ TAPIMAXCALLEDPARTYSIZE - 1 ] = '\0';

	return;
 }
