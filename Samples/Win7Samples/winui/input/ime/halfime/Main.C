/************************************************************************
*
* Title:
*
*   Main.C - IME Half-aware Simple Text Editor (DBCS version)
*
* Purpose:
*
*   Sample program for DBCS programming and IME half-aware application.
*
* Synopsis:
*
*   This program is designed as a bare-bone example to demonstrate the
*   basic elements of DBCS-enabling, and how to design an IME half-aware
*   application.
*
*   The data structure is a fixed-size 2-dimensional byte array.  The
*   font is the fixed-pitch system font.  Rudimentary text editing
*   functions, such as the basic cursor movements, insertion, deletion,
*   have been implemented.
*
* DBCS-enabling notes:
*
*   This version of STE is DBCS-enabled with respect to character input,
*   caret shape and movement, and mouse clicking.  It should work on
*   any version of Far East Windows since 4.0.
*
*   As far as source code maintenance goes, there are generally two
*   approaches.
*
*   The first is to add DBCS enabling code under '#ifdef DBCS'.  The
*   advantage of this approach is that it keeps the DBCS enabling code
*   distinct from the SBCS, so it's easier to add them in, and also to
*   remove them later.  (For example, when you want to replace DBCS
*   enabling with Unicode enabling.)  The drawback is that because the
*   DBCS and the SBCS logic are not integrated, they can easily get out
*   of sync (as the SBCS code evolves.)
*
*   The second approach, which is adopted by this sample app, is to
*   integrate DBCS enabling with SBCS.  It takes longer to do, but
*   the resulting source is easier to maintain.  Since IsDBCSLeadByte
*   is at the heart of any DBCS-enabling logic, an additional speed up
*   for generating an SBCS-only version is to define that function as
*   FALSE, and let the compiler optimize the DBCS logic away.
*
* IME Half-Aware notes:
*
*   This version of STE is an IME half-aware application with the 
*   capiabilities to open/close IME conversion engine and control
*   where the IME composition window will be located in. It should
*   work on any version of Far East Windows since 4.0.
*
*   This kind of application typically wants to control the behavior
*   of IME like opening/closing IME, setting where to show composition
*   window, setting where to show candidate lists and so on. It does
*   not display any user interface for IME. 
*
* History:
*
*   17-Aug-1992     created
*   28-Sep-1992     added DBCS-enabling
*   30-Sep-1992     bug fixes
*   25-Mar-1994     added IME half-aware logics
*
************************************************************************/

#include <assert.h>
#include <windows.h>
#include <imm.h>
#include "halfime.h"

//
// Function prototype
//
void    ResetCaret( HWND );
void    SetIMECompFormPos( HWND );
LRESULT CALLBACK SteWndProc( HWND, UINT, WPARAM, LPARAM );


/************************************************************************
*
* Global data
*
************************************************************************/
TCHAR szSteClass[10] = {0};         // window class name
TCHAR szSteTitle[40] = {0};         // window frame title
UINT  cxMetrics,                    // aver. character width
      cxOverTypeCaret,              // caret width for overtype mode
      cyMetrics;                    // character height
int xPos, yPos;                     // caret position
HFONT hfntFixed;                    // fixed-pitch font
HFONT hfntOld;                      // default font holder
BOOL fInsertMode;                   // insert/overtype mode flag
int CaretWidth;                     // insert/overtype mode caret width
BYTE textbuf[MAXROW][MAXCOL];       // text buffer
int DBCSFillChar;                   // 'Pattern/DBCS' fill character

WPARAM CompWindowMode;              // Composition window mode.
COMPOSITIONFORM CompForm = {0};     // Storage for composition window
                                    // structure.
DWORD fdwProperty;                  // the property of current active IME


/************************************************************************
*
*   SteRegister - standard class registration routine
*
************************************************************************/
int SteRegister( 
    HANDLE hInstance )
{
    WNDCLASS wc = {0};

    wc.hCursor       = LoadCursor( NULL, IDC_IBEAM );
    wc.hIcon         = LoadIcon( hInstance, MAKEINTRESOURCE(ID_ICON));
    wc.lpszMenuName  = MAKEINTRESOURCE(ID_MENU);
    wc.hInstance     = hInstance;
    wc.lpszClassName = szSteClass;
    wc.lpfnWndProc   = (WNDPROC)SteWndProc;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style         = CS_BYTEALIGNCLIENT | CS_CLASSDC;

    if ( !RegisterClass( &wc ) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/************************************************************************
*
*   WinMain
*
************************************************************************/
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszCmdLine,
    int nCmdShow )
{
    MSG msg = {0};
    HWND hWnd = NULL;
    BOOL bSuccess = TRUE;
    BOOL bRet;

    LoadString( hInstance, IDS_CLASS, szSteClass, 10 );
    LoadString( hInstance, IDS_TITLE, szSteTitle, 40 );

	if ( !SteRegister( hInstance ) )
	{
	    bSuccess = FALSE;
	    goto exit_func;
	}

    if ( !(hWnd = CreateWindow( szSteClass, szSteTitle,
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
                                NULL, NULL, hInstance, NULL)) )
    {
	    bSuccess = FALSE;
	    goto exit_func;
    }

    //
    // Create window with just enough client space for the text buffer
    //
    SetWindowPos( hWnd, 0, 0, 0,
                  MAXCOL*cxMetrics + GetSystemMetrics(SM_CXBORDER)*2,
                  MAXROW*cyMetrics + GetSystemMetrics(SM_CYBORDER)*2
                    + GetSystemMetrics(SM_CYCAPTION)
                    + GetSystemMetrics(SM_CYMENU),
                  SWP_NOZORDER );

    ShowWindow( hWnd, nCmdShow );


    //
    // Any IME half-aware application should call TranslateMessage API to
    // translate messages. This is because Windows95 IME architecture 
    // will call ImmToAsciiEx API, if necessary, to make IME conversion
    // engine work, and generate some IME messages during TranslateMessage
    // session. Otherwise, IME will not work.
    //
    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // possibly handle the error 
            bSuccess = FALSE;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    

exit_func:
    if (bSuccess)
    {
        return (int) msg.wParam;
    }
    else
    {
        return 0;
    }
}

/**********************************************************************
*
* SteIMEOpenClose()
*
* This routines calls IMM API to open or close IME.
*
***********************************************************************/
void SteImeOpenClose( 
    HWND hWnd, 
    BOOL fFlag )
{
    HIMC hIMC = NULL;    // Input context handle.

    //
    // If fFlag is true then open IME; otherwise close it.
    //
    if ( !( hIMC = ImmGetContext( hWnd ) ) )
    {
        return;
    }

    //
    // When an application wants to open/close IME for some
    // reasons, it should call TMMSetOpenStatus() to do that.
    //
    ImmSetOpenStatus( hIMC, fFlag );
    ImmReleaseContext( hWnd, hIMC );
}


/**********************************************************************
*
* SteIMECompWindowPos()
*
* This routines inform IME where to display composition window.
* Here we control IME composition window with the following cases:
* 1: DEFAULT:   Display composition window in IME default position.
* 2: NEARCARET: Display composition window in near caret position.
* 3: PreFer:    Display composition window in application prefer position.
*
***********************************************************************/
void SteIMECompWindowPos(
    HWND hWnd, 
    WPARAM cmd )
{
    RECT rect = {0};  // Storage for client rect.
    HIMC hImc = NULL;

	
    //
    // Set current composition mode.
    //
    CompWindowMode = cmd;

    switch( cmd )
    {
        case IDC_DEFAULT_POS:

            //
            // Put composition window in default position
            // being deterimined by IME UI. Applications 
            // have to set composition form style with 
            // CFS_DEFAULT to imform IME UI.
            // 

            CompForm.dwStyle = CFS_DEFAULT;
            break;

        case IDC_NEARCARET_POS:

            //
            // Put composition window in near caret 
            // position. If IME UI receives this control
            // message, it will put the composition window
            // near to the position deesignated by ptCurrentPos.
            // So for this operation, applications have to set
            // composition form style with CFS_POINT and store
            // caret position in ptCurrentPos.
            //

            CompForm.dwStyle = CFS_POINT;
            GetCaretPos( (PPOINT) &CompForm.ptCurrentPos );
            break;

        case IDC_PREFER_POS:

            // Put composition window in application
            // prefer position. Applications have to set
            // composition form sytle with  CFS_POINT |
            // CFS_FORCE_POSITION and store the prefer
            // position in ptCurrentPos.
            //
            // The difference between CFS_POINT and CFS_POINT
            // with CFS_FORCE_POSITION is if setting
            // CFS_FORCE_POSITION style, IME will actually
            // display its composition window in the position
            // specified by ptCurrent without any adjustment;  
            // otherwise IME will shPosow its composition window 
            // by adjusting the position specified by 
            // ptCurrentPos.
            //

            CompForm.dwStyle = CFS_POINT | CFS_FORCE_POSITION;

            GetClientRect( hWnd, (LPRECT) &rect);

            CompForm.ptCurrentPos.x = 0;
            CompForm.ptCurrentPos.y = rect.bottom;
            break;
    }

    hImc = ImmGetContext( hWnd );
    ImmSetCompositionWindow( hImc, (COMPOSITIONFORM FAR*)&CompForm );
    ImmReleaseContext( hWnd, hImc );
}


/***********************************************************************
*
* ResetIMECompWin()
*
* This routine informs IME the fact that caret position has been changed.
*
************************************************************************/
void ResetIMECompWin( 
    HWND hWnd )
{
    if ( CompWindowMode == IDC_NEARCARET_POS ||
         CompWindowMode == IDC_PREFER_POS )
    {
        HIMC hIMC = ImmGetContext(hWnd);
        if ( hIMC )
        {
            ImmSetCompositionWindow(hIMC,&CompForm);
        }
        ImmReleaseContext( hWnd , hIMC);
    }
}


/***********************************************************************
*
* SetIMECompFormPos( HWND )
*
************************************************************************/
void SetIMECompFormPos( 
    HWND hWnd )
{

    // 
    // If current composition form mode is near caret operation, 
    // application should inform IME UI the caret position has been
    // changed. IME UI will make decision whether it has to adjust
    // composition window position.
    // 
    //
    if ( CompWindowMode == IDC_NEARCARET_POS )
    {
        HIMC hIMC = ImmGetContext(hWnd);
        POINT point = {0};

        GetCaretPos( &point );

        CompForm.dwStyle = CFS_POINT;

        CompForm.ptCurrentPos.x = (long) point.x;
        CompForm.ptCurrentPos.y = (long) point.y;

        if ( hIMC )
        {
            ImmSetCompositionWindow(hIMC,&CompForm);
        }
        ImmReleaseContext( hWnd , hIMC);
    }

}


/************************************************************************
*
*   SteCreate - WM_CREATE message handler
*
************************************************************************/
void SteCreate( 
    HWND hWnd )
{
    HDC        hdc = NULL;
    TEXTMETRIC tm = {0};
    LOGFONT    lFont = {0};
    HFONT      hFont = NULL;

    //
    // Note that this window has a class DC
    //
    hdc = GetDC( hWnd );

    //
    // Select fixed pitch system font and get its text metrics
    //
    hfntFixed = GetStockObject( SYSTEM_FIXED_FONT );
    hfntOld = SelectObject( hdc, hfntFixed );
    GetTextMetrics( hdc, &tm );
    ReleaseDC( hWnd, hdc );
    cxMetrics = tm.tmAveCharWidth;
    cyMetrics = tm.tmHeight;

    //
    // Determine the version of DBCS Windows from system font charset ID.
    // Then hardcode a DBCS character value for the 'Pattern/DBCS' command.
    // The value is the Han character for 'door' or 'gate', which is
    // left-right symmetrical.
    //
    switch( tm.tmCharSet )
    {
        case SHIFTJIS_CHARSET:
            DBCSFillChar = 0x96e5;
            break;

        case HANGEUL_CHARSET:
            DBCSFillChar = 0xdaa6;
            break;

        case CHINESEBIG5_CHARSET:
            DBCSFillChar = 0xaaf9;
            break;

        default:
            DBCSFillChar = 0x7071;  // 'pq'
            break;
    }

    //                   
    // Initialize caret width.  Fat in INSERT mode, skinny in OVERTYPE mode.
    //
    fInsertMode = FALSE;
    CaretWidth = cxOverTypeCaret = GetSystemMetrics( SM_CXBORDER );

    //
    // Sets the logical font to be used to display characters in the 
    // composition window. Especially for at caret or near caret operation, 
    // application should set composition font.
    //
    if ( ( hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0L ) ) != NULL )
    {
        if ( GetObject( hFont, sizeof(LOGFONT), (LPVOID)&lFont ) )
        {
            HIMC hImc = NULL;
            if ( (  hImc = ImmGetContext( hWnd ) ) )
            {
                ImmSetCompositionFont( hImc, &lFont );
                ImmReleaseContext( hWnd, hImc );
            }
        }
    }

    PostMessage( hWnd, WM_COMMAND, IDC_CLEAR, 0L );

    fdwProperty = ImmGetProperty( GetKeyboardLayout(0), IGP_PROPERTY );
}


/************************************************************************
*
*   ResetCaret - Reset caret shape to match input mode (overtype/insert)
*
************************************************************************/
void ResetCaret( 
    HWND hWnd )
{

    HideCaret( hWnd );
    DestroyCaret();
    CreateCaret( hWnd,
                 NULL,
                 (fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ?
                    CaretWidth*2 : CaretWidth,
                 cyMetrics );
    SetCaretPos( xPos * cxMetrics, yPos * cyMetrics );
    ShowCaret( hWnd );

    SetIMECompFormPos( hWnd );

}



/************************************************************************
*
*   SteCommand - WM_COMMAND handler
*
************************************************************************/
void SteCommand(
    HWND hWnd, 
    WPARAM cmd )
{
    switch( cmd )
    {
        case IDC_CLEAR:
            //
            // Blank out text buffer.  Return caret to home position
            //

            for ( yPos = FIRSTROW; yPos <= LASTROW; yPos++ )
            {
                for ( xPos = FIRSTCOL; xPos <= LASTCOL; xPos++ )
                {
                    textbuf[yPos][xPos] = ' ';
                }
            }
            break;

        case IDC_ANSIFILL: /* fall-through */
        case IDC_DBCSFILL:

            //
            // Fill text buffer with ANSI or DBCS pattern
            //

            for ( yPos = FIRSTROW; yPos <= LASTROW; yPos++ )
            {
                for ( xPos = FIRSTCOL; xPos <= LASTCOL; xPos++ )
                {
                    if ( cmd == IDC_ANSIFILL )
                    {
                        textbuf[yPos][xPos] = 'a';
                    }
                    else 
                    {
                        textbuf[yPos][xPos]   = HIBYTE(DBCSFillChar);
                        textbuf[yPos][++xPos] = LOBYTE(DBCSFillChar);
                    }
                }
            }
            break;
    

        //
        // The following messages are to control IME.
        //

        case IDC_OPENIME:

            SteImeOpenClose( hWnd, TRUE );
            goto RETURN;
            break;

        case IDC_CLOSEIME:

            SteImeOpenClose( hWnd, FALSE );
            goto RETURN;
            break;

        case IDC_DEFAULT_POS:    /* fall-through */
        case IDC_NEARCARET_POS:  /* fall-through */
        case IDC_PREFER_POS:

            SteIMECompWindowPos( hWnd, cmd );
            goto RETURN;
            break;

    }

    yPos = FIRSTROW;
    xPos = FIRSTCOL;

    InvalidateRect( hWnd, (LPRECT)NULL, TRUE );
    ResetCaret(hWnd);

RETURN:

    return;    
}


/************************************************************************
*
*   IsDBCSTrailByte - returns TRUE if the given byte is a DBCS trail byte
*
*                     The algorithm searchs backward in the string, to some
*                     known character boundary, counting consecutive bytes
*                     in the lead byte range. An odd number indicates the
*                     current byte is part of a two byte character code.
*
*   INPUT: PCHAR  - pointer to a preceding known character boundary.
*          PCHAR  - pointer to the character to test.
*
*   OUTPUT:BOOL   - indicating truth of p==trailbyte.
*
************************************************************************/
BOOL IsDBCSTrailByte( 
    char *base, 
    char *p )
{
    int lbc = 0;    // lead byte count

    assert(base <= p);

    while ( p > base ) 
    {
    	if ( !IsDBCSLeadByte(*(--p)) )
    	{
    	    break;
    	}
    	lbc++;
    }

    return (lbc & 1);
}




/************************************************************************
*
*   VirtualKeyHandler - WM_KEYDOWN handler
*
*
*   INPUT:  HWND - handle to the window for repainting output.
*           WPARAM - virtual key code.
*
************************************************************************/
void VirtualKeyHandler( 
    HWND hWnd, 
    WPARAM wParam )
{
    int i;
    HDC hdc;
    static int delta = 1;

    switch( wParam )
    {
        case VK_HOME:   // beginning of line
            xPos = FIRSTCOL;
            break;

        case VK_END:    // end of line
            xPos = LASTCOL;
            goto check_for_trailbyte;

        case VK_RIGHT:
            if ( IsDBCSLeadByte( textbuf[yPos][xPos] ) )
            {
                if (xPos==LASTCOL - 1) 
                {
                    break;   // last character don't move
                }
                xPos += 2;   // skip 2 for DB Character
            }
            else
            {
                xPos = min( xPos+1, LASTCOL );
            }
            break;

        case VK_LEFT:
            xPos = max( xPos-1, FIRSTCOL );

check_for_trailbyte:

            if ( IsDBCSTrailByte( (CHAR *)textbuf[yPos], (CHAR *)&(textbuf[yPos][xPos]) ) )
            {
                xPos--;
            }
            break;

        case VK_UP:
            yPos = max( yPos-1, FIRSTROW );
            goto Virtical_Check_Trail;

        case VK_DOWN:
            yPos = min( yPos+1, LASTROW );

Virtical_Check_Trail:

            if ( IsDBCSTrailByte( (CHAR *)textbuf[yPos], (CHAR *)&(textbuf[yPos][xPos]) ) )
            {
                if (xPos<LASTCOL)
                {
                    xPos+=delta;
                    delta *= -1;
                }
                else
                {
                    xPos--;
                }
            }
            break;


        case VK_INSERT:

            //
            // Change caret shape to indicate insert/overtype mode
            //

            fInsertMode = !fInsertMode;
            CaretWidth = fInsertMode ? cxMetrics : cxOverTypeCaret;
            break;

        case VK_BACK:   // backspace
            if ( xPos > FIRSTCOL ) 
            {
                xPos--;

                //
                // DB Character so backup one more to allign on boundary
                //
                if ( IsDBCSTrailByte( (CHAR *)textbuf[yPos], (CHAR *)&(textbuf[yPos][xPos]) ) )
                {
            	    xPos--;
                }
                //
                // Fall Through to VK_DELETE to adjust row
                //
            }
            else     //FIRST COLUMN  don't backup -- this would change for wrapping
            {
               break;
            }
            
        case VK_DELETE:

            if ( !IsDBCSLeadByte( textbuf[yPos][xPos] ) ) 
            {
                //
                // Move rest of line left by one, then blank out last character
                //
                for ( i = xPos; i < LASTCOL; i++ )
            	textbuf[yPos][i] = textbuf[yPos][i+1];
                textbuf[yPos][LASTCOL] = ' ';
            } 
            else 
            {
                //
                // Move line left by two bytes, blank out last two bytes
                //

                for ( i = xPos; i < LASTCOL-1; i++ )
                {
                	textbuf[yPos][i] = textbuf[yPos][i+2];
                }
                textbuf[yPos][LASTCOL-1] = ' ';
                textbuf[yPos][LASTCOL]   = ' ';
            }

            //
            // Repaint the entire line
            //

            hdc = GetDC( hWnd );
            HideCaret( hWnd );
            TextOutA( hdc, 0, yPos*cyMetrics, (LPCSTR)textbuf[yPos], MAXCOL );
            ReleaseDC( hWnd, hdc );
            break;

        case VK_TAB:    // tab  -- tabs are column allignment not character
            {
                 int xTabMax = xPos + TABSTOP;
                 int xPosPrev;

                 do 
                 {
                     xPosPrev = xPos;
                     SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L );
                 } 
                 while ((xPos % TABSTOP) &&
                        (xPos < xTabMax) &&
                        (xPos != xPosPrev));

            }
            break;

        case VK_RETURN: // linefeed
            yPos = min( yPos+1, LASTROW );
            xPos = FIRSTCOL;
            break;
    }

    ResetCaret( hWnd );
}


/************************************************************************
*
*   StoreChar - Stores one SBCS character into text buffer and advances
*               cursor
*
************************************************************************/
void StoreChar( 
    HWND hWnd, 
    BYTE ch )
{
    int i;
    HDC hdc = NULL;

    //
    // If insert mode, move rest of line to the right by one
    //

    if ( fInsertMode ) 
    {
        for ( i = LASTCOL; i > xPos; i-- )
        {
            textbuf[yPos][i] = textbuf[yPos][i-1];
        }

        //
        // If the row ends on a lead byte, blank it out
        // To do this we must first traverse the string
        // starting from a known character boundry until
        // we reach the last column. If the last column
        // is a character boundry then the last character
        // is either a single byte or a lead byte
        //

        for ( i = xPos+1; i < LASTCOL; ) 
        {
            if ( IsDBCSLeadByte( textbuf[yPos][i] ) )
            {
                i++;
            }
            i++;
        }
        
        if (i==LASTCOL)
        {
           if ( IsDBCSLeadByte( textbuf[yPos][LASTCOL] ) )
           {
               textbuf[yPos][LASTCOL] = ' ';
           }
        }
    } 
    else 
    {  // overtype mode
    	if ( IsDBCSLeadByte( textbuf[yPos][xPos] ) )
    	{
            //
            // Blank out trail byte
            //
            textbuf[yPos][xPos+1] = ' ';

            //
            // or shift line left on character and blank last column
            //
            // for ( i = xPos+1; i < LASTCOL; i++ )
            //     textbuf[yPos][i] = textbuf[yPos][i+1];
            // textbuf[yPos][LASTCOL] = ' ';
        }
    }

    //
    // Store input character at current caret position
    //
    textbuf[yPos][xPos] = ch;

    //
    // Display input character.
    //
    hdc = GetDC( hWnd );
    HideCaret( hWnd );
    TextOutA( hdc, xPos*cxMetrics, yPos*cyMetrics,
              (LPCSTR)&(textbuf[yPos][xPos]), MAXCOL-xPos );
    ShowCaret( hWnd );
    ReleaseDC( hWnd, hdc );

    SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L );
}


/************************************************************************
*
*   StoreDBCSChar - Stores one DBCS character into text buffer and
*                   advances cursor
*
************************************************************************/
void StoreDBCSChar( 
    HWND hWnd, 
    WORD ch )
{
    int i;
    HDC hdc;

    //
    // If there is no room for a DBCS character, discard it
    //
    if ( xPos == LASTCOL )
    {
        return;
    }

    //
    // If insert mode, move rest of line to the right by two
    //
    if ( fInsertMode ) 
    {
        for ( i = LASTCOL; i > xPos+1; i-- )
        {
            textbuf[yPos][i] = textbuf[yPos][i-2];
        }

        //
        // If the row ends on a lead byte, blank it out
        // To do this we must first traverse the string
        // starting from a known charcter boundry until
        // we reach the last column. If the last column
        // is not a trail byte then it is a single byte
        // or a lead byte
        //
        for ( i = xPos+2; i < LASTCOL; ) 
        {
            if ( IsDBCSLeadByte( textbuf[yPos][i] ) )
            {
                i++;
            }
            i++;
        }
        if (i==LASTCOL)
        {
           if (IsDBCSLeadByte( textbuf[yPos][LASTCOL] ) )
           {
                textbuf[yPos][LASTCOL] = ' ';
           }
        }
    }
    else 
    {  // overtype mode
        if ( !IsDBCSLeadByte( textbuf[yPos][xPos] ) )
        {
            //
            // Overtyping the current byte, plus the following byte,
            // which could be a lead byte.
            //
            if ( IsDBCSLeadByte( textbuf[yPos][xPos+1] ) )
            {
                textbuf[yPos][xPos+2] = ' ';
            }
        }
    }

    //
    // Store input character at current caret position
    //
    textbuf[yPos][xPos]   = LOBYTE(ch);     // lead byte
    textbuf[yPos][xPos+1] = HIBYTE(ch);     // trail byte

    //
    // Display input character.
    //
    hdc = GetDC( hWnd );
    HideCaret( hWnd );
    TextOutA( hdc, xPos*cxMetrics, yPos*cyMetrics,
             (LPCSTR)&(textbuf[yPos][xPos]), MAXCOL-xPos );
    ShowCaret( hWnd );
    ReleaseDC( hWnd, hdc );

    SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L );
}


/************************************************************************
*
*   CharHandler - WM_CHAR handler
*
************************************************************************/
void CharHandler( 
    HWND hWnd, 
    WPARAM wParam )
{
    unsigned char ch = (unsigned char)wParam;

    //
    // Because DBCS characters are usually generated by IMEs (as two
    // PostMessages), if a lead byte comes in, the trail byte should
    // arrive very soon after.  We wait here for the trail byte and
    // store them into the text buffer together.
    if ( IsDBCSLeadByte( ch ) ) 
    {
        //
        // Wait an arbitrary amount of time for the trail byte to
        // arrive.  If it doesn't, then discard the lead byte.
        //
        // This could happen if the IME screwed up.  Or, more likely,
        // the user generated the lead byte through ALT-numpad.
        //
        MSG msg = {0};
        int i = 10;

        while (!PeekMessage((LPMSG)&msg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE)) 
        {
            if ( --i == 0 )
            {
                return;
            }
            Yield();
        }

        StoreDBCSChar( hWnd, (WORD)(((unsigned)(msg.wParam)<<8) | (unsigned)ch ));
    } 
    else 
    {
        switch( ch )
        {
            case '\r': /* fall-through */
            case '\t': /* fall-through */
            case '\b':
                //
                // Throw away.  Already handled at WM_KEYDOWN time.
                //
                break;

            default:
                StoreChar( hWnd, ch );
                break;
        }
    }
}


/************************************************************************
*
*   MouseHandler - WM_BUTTONDOWN handler
*
************************************************************************/
void MouseHandler( 
    HWND hWnd, 
    LPARAM lParam )
{
    HideCaret( hWnd );

    //
    // Calculate caret position based on fixed pitched font
    //
    yPos = MAKEPOINTS(lParam).y / cyMetrics;
    xPos = MAKEPOINTS(lParam).x / cxMetrics;

    //
    // Adjust caret position if click landed on a trail byte
    //
    if ( IsDBCSTrailByte( (CHAR *)textbuf[yPos], (CHAR *)&(textbuf[yPos][xPos]) ) )
    {
        //
        // If click landed on the last quarter of the DBCS character,
        // assume the user was aiming at the next character.
        //
        if ( (MAKEPOINTS(lParam).x - xPos * cxMetrics) > (cxMetrics / 2) )
        {
            xPos++;
        }
        else
        {
            xPos--;
        }
    }

    DestroyCaret();
    CreateCaret(hWnd,
                NULL,
                (fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ?
                    CaretWidth*2 : CaretWidth,
                cyMetrics );
    SetCaretPos( xPos * cxMetrics, yPos * cyMetrics );
    ShowCaret( hWnd );

    SetIMECompFormPos( hWnd );
}


/************************************************************************
*
*   InputChangeHandler - WM_INPUTLANGCHANGE handler
*
************************************************************************/
void InputChangeHandler( 
    HWND hWnd )
{
    HIMC hIMC;

    fdwProperty = ImmGetProperty( GetKeyboardLayout(0), IGP_PROPERTY );

    // if this application set the candidate position, it need to set
    // it to default for the near caret IME
    if ( hIMC = ImmGetContext( hWnd ) ) 
    {
        UINT i;

        for (i = 0; i < 4; i++) 
        {
            CANDIDATEFORM CandForm = {0};

            if ( fdwProperty & IME_PROP_AT_CARET ) 
            {
                CandForm.dwIndex = i;
                CandForm.dwStyle = CFS_CANDIDATEPOS;
#if 0
                // This application do not want to set candidate window to
                // any position. Anyway, if an application need to set the
                // candiadet position, it should remove the if 0 code

                // the position you want to set
                CandForm.ptCurrentPos.x = ptAppWantPosition[i].x;
                CandForm.ptCurrentPos.y = ptAppWantPosition[i].y;

                ImmSetCandidateWindow( hIMC, &CandForm );
#endif
            }
            else 
            {
                if ( !ImmGetCandidateWindow( hIMC, i, &CandForm ) ) 
                {
                    continue;
                }

                if ( CandForm.dwStyle == CFS_DEFAULT ) 
                {
                    continue;
                }

                CandForm.dwStyle = CFS_DEFAULT;

                ImmSetCandidateWindow( hIMC, &CandForm );
            }
        }

        ImmReleaseContext( hWnd, hIMC );
    }

    return;
}


/************************************************************************
*
*   SteWndProc - STE class window procedure
*
************************************************************************/
LRESULT CALLBACK SteWndProc( 
    HWND hWnd, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam )
{
    int i = 0;
    HDC hdc = NULL;
    PAINTSTRUCT ps = {0};
    BOOL bRetDWP = FALSE;

    switch( msg ) 
    {
        case WM_CREATE:
            SteCreate( hWnd );
            break;

        case WM_MOVE:
            ResetIMECompWin( hWnd );
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            DestroyWindow( hWnd );
            break;

        case WM_SETFOCUS:
            CreateCaret(hWnd,
                        NULL,
                        (fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ?
                            CaretWidth*2 : CaretWidth,
                        cyMetrics );
            SetCaretPos( xPos * cxMetrics, yPos * cyMetrics );
            ShowCaret( hWnd );
            break;

        case WM_KILLFOCUS:
            HideCaret( hWnd );
            DestroyCaret();
            break;

        case WM_KEYDOWN:
            VirtualKeyHandler( hWnd, wParam );
            break;

        case WM_CHAR:
            CharHandler( hWnd, wParam );
            break;

        case WM_LBUTTONDOWN:
            MouseHandler( hWnd, lParam );
            break;

        case WM_COMMAND:
            SteCommand( hWnd, wParam );
            break;

        case WM_PAINT:
            InvalidateRect(hWnd,NULL,FALSE);  
            hdc = BeginPaint( hWnd, &ps );

            //
            // Refresh display from text buffer
            //
            for ( i = FIRSTROW; i <= LASTROW; i++ )
            {
                TextOutA( hdc, 0, i*cyMetrics, (LPCSTR)textbuf[i], MAXCOL );
            }
            EndPaint( hWnd, &ps );
            break;

        case WM_INPUTLANGCHANGE:
            InputChangeHandler( hWnd );
            bRetDWP = TRUE;
            break;

        default:
            bRetDWP = TRUE;
            break;
    }

    if (bRetDWP)
    {
        return DefWindowProc( hWnd, msg, wParam, lParam );
    }
    else
    {
        return 0;
    }
}
