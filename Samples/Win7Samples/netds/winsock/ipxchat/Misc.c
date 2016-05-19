// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   misc.c
//
//  PURPOSE:  Contains all helper functions "global" to the application.
//
//  FUNCTIONS:
//    CenterWindow - Center one window over another.
//    ReceiveInBox - Reads incoming socket data.
//    SendOutBox   - Writes outgoing socket data.
//    AtoH         - Converts ascii string to network order hex
//    BtoH         - Converts ascii byte to hex
//    CleanUp      - closes sockets and detaches winsock dll
//    GetAddrString - Puts IPX address into <network>.<node>.<socket> string format
//    HtoA          - Converts network order hex to ascii string
//    HtoB          - Converts hex byte to asci string
//
//  COMMENTS:
//
//
#include "globals.h"            // prototypes specific to this application
#include <windows.h>            // required for all Windows applications
#include <windowsx.h>



//
//  FUNCTION: CenterWindow(HWND, HWND)
//
//  PURPOSE:  Center one window over another.
//
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//
//  RETURN VALUE:
//
//    TRUE  - Success
//    FALSE - Failure
//
//  COMMENTS:
//
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
//

BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

    // Get the Height and Width of the child window
    if(!GetWindowRect(hwndChild, &rcChild))
        return FALSE;
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the Height and Width of the parent window
    if(!GetWindowRect(hwndParent, &rcParent))
        return FALSE;
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

    // Get the display limits
    if(NULL == (hdc = GetDC(hwndChild)))
        return FALSE;
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    if(0 == ReleaseDC(hwndChild, hdc))
        return FALSE;

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
        xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
}

//
//  FUNCTION: ReceiveInBox(HWND, WPARAM, LPARAM, char *, int)
//
//  PURPOSE:  Reads incoming data from socket
//
//  PARAMETERS:
//    hWnd      - Handle to current window
//    uParam    - WPARAM (unused)
//    lParam    - LPARAM contains event (FD_READ or FD_CLOSE).
//    szRBuf    - Receive Buffer
//    cRBufLen  - size of Receive Buffer
//
//  RETURN VALUE:
//
//    TRUE  - Data Read
//    FALSE - If FD_CLOSE message
//
//  COMMENTS:
//
//    Called if socket has data OR if it is closed.  If closed post
//    WM_DISCONNECTED message.  Else read data and make sure it is
//    NULL terminated.
//

BOOL ReceiveInBox(HWND hWnd, WPARAM uParam, LPARAM lParam, char * szRBuf, int cRBufLen)
{
    char * pRBuf;     // temp buf pointer
    int cBytesRead;   // count of bytes actually read

    uParam;

    
    if (LOWORD(lParam) == FD_CLOSE)                   // Is this a FD_CLOSE event?
    {
        SendMessage(hWnd, MW_DISCONNECTED, 0, 0);     // Yes, post message
        return(FALSE);
    }

    pRBuf = szRBuf;   // Set temp pointer
    cRBufLen--;       // Save room for null terminator
          
    // read socket
    if((cBytesRead = recv(sock, pRBuf, cRBufLen, 0)) != SOCKET_ERROR)
        pRBuf += cBytesRead;    // Move temp pointer to end of buffer
              
    *pRBuf = 0;   // Null terminate - if recv() failed, then prBuf will 
                  // point to first byte of the buffer
    
    return (TRUE);   // We've got a buffer to display
}

//
//  FUNCTION: SendOutBox(char *, int)
//
//  PURPOSE:  Reads incoming data from socket
//
//  PARAMETERS:
//    szSBuf    - Send Buffer
//    cSBufLen  - size of Send Buffer
//
//  COMMENTS:
//
//    Writes send buffer to socket -- repeats until all data is sent.
//

void SendOutBox(char * szSBuf, int cSBufLen)
{
    char * pSBuf;
    int cBytesSent;

    pSBuf = szSBuf; // Set temp pointer
    
    while((cBytesSent = send(sock,
                             pSBuf,
                             cSBufLen,
                             0)) != SOCKET_ERROR)
    {
        pSBuf += cBytesSent;
        cSBufLen -= cBytesSent;
        if(!cSBufLen) return;
    }
}

//
//  FUNCTION: AtoH(char *, char *, int)
//
//  PURPOSE:  Converts ascii string to network order hex
//
//  PARAMETERS:
//    src    - pointer to input ascii string
//    dest   - pointer to output hex
//    destlen - size of dest
//
//  COMMENTS:
//
//    2 ascii bytes make a hex byte so must put 1st ascii byte of pair
//    into upper nibble and 2nd ascii byte of pair into lower nibble.
//

void AtoH(char * src, char * dest, int destlen)
{
    char * srcptr;

    srcptr = src;

    while(destlen--)
    {
        *dest = BtoH(*srcptr++) << 4;    // Put 1st ascii byte in upper nibble.
        *dest = BtoH(*srcptr++);         // Add 2nd ascii byte to above.
        dest++;
    }
}

//
//  FUNCTION: BtoH(char *, char *, int)
//
//  PURPOSE:  Converts ascii byte to numeric
//
//  PARAMETERS:
//    ch - ascii byte to convert
//
//  RETURNS:
//    associated numeric value
//
//  COMMENTS:
//
//    Will convert any hex ascii digit to its numeric counterpart.
//    Puts in 0xff if not a valid hex digit.
//

unsigned char BtoH(char ch)
{
    if (ch >= '0' && ch <= '9') return (ch - '0');        // Handle numerals
    if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  // Handle capitol hex digits
    if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  // Handle small hex digits
    return(255);
}

//
//  FUNCTION: CleanUp(void)
//
//  PURPOSE:  Protocol specific cleanup function
//
//  COMMENTS:
//
//    Deletes our two possible sockets (they might not exist which
//    just means closesocket might return an error -- we don't care).
//

void CleanUp(void)
{
    if(INVALID_SOCKET != SrvSock)
    {
        closesocket(SrvSock);  // Close our server side socket
        SrvSock = INVALID_SOCKET;
    }
    if (INVALID_SOCKET != sock)
    {
        closesocket(sock);     // Close our connection specific socket
        sock = INVALID_SOCKET;
    }
    WSACleanup();          // Nix the DLL
}

//
//  FUNCTION: GetAddrString(PSOCKADDR_IPX, char *)
//
//  PURPOSE:  Converts IPX address to ascii string for displaying
//
//  PARAMETERS:
//    pSAddr - pointer to socket address struc
//    dest   - pointer to destination string
//
//  COMMENTS:
//
//    Address is in network order to use HtoA to convert to ascii.
//    
//    Final format is 
//      <8 char network address>.<12 char node address>.<4 char sock address>
//

void GetAddrString(PSOCKADDR_IPX pSAddr, char * dest)
{
    char abuf[15];                                // temp buffer
    char * currptr;                               // temp destination pointer
    HRESULT hRet;

    currptr = dest; // initialize destination pointer

    HtoA((char *)&pSAddr->sa_netnum, abuf, 4);    // convert network number
    hRet = StringCchCopy(currptr,15,abuf);
    currptr += 8;
    hRet = StringCchCat(currptr,18,".");                   // don't forget seperator
    currptr++;
    
    HtoA((char *)&pSAddr->sa_nodenum, abuf, 6);   // convert node number
    hRet = StringCchCat(currptr,18,abuf);
    currptr += 12;
    hRet = StringCchCat(currptr,18,".");                          // seperator
    currptr++;

    HtoA((char *)&pSAddr->sa_socket, abuf, 2);    // convert socket number
    hRet = StringCchCat(currptr,18,abuf);
   
}

//
//  FUNCTION: HtoA(char *, char *, int)
//
//  PURPOSE:  Converts network ordered hex to ascii string
//
//  PARAMETERS:
//    src     - pointer to network ordered hex
//    dest    - pointer to ascii string
//    srclen  - size of hex number in bytes
//
//  COMMENTS:
//
//    1 byte hex = 2 bytes ascii so convert high order nibble with HtoB()
//    then convert low order nibble. dest buffer better be 2*srclen + 1.
//

void HtoA(char * src, char * dest, int srclen)
{
    char * destptr; // temp pointers
    UCHAR * srcptr;
        
    srcptr = (UCHAR *)src;
    destptr = dest;

    while(srclen--)
    {
    *destptr++ = HtoB((UCHAR)(*srcptr >> 4));      // Convert high order nibble
    *destptr++ = HtoB((UCHAR)(*srcptr++ & 0x0F));  // Convert low order nibble
    }
    *destptr = 0;  // Null terminator
}

//
//  FUNCTION: HtoB(UCHAR)
//
//  PURPOSE:  Converts hex byte to ascii byte
//
//  PARAMETERS:
//    ch - Hex byte
//                 
//  RETURNS:
//    ascii byte
//
//  COMMENTS:
//
//    We actually only convert a nibble since 1 byte hex = 2 bytes ascii.
//    So if ch > 0xf we just return 'X'.
//

char HtoB(UCHAR ch)
{
    if (ch <= 9) return ('0' + ch);             // handle decimal values
    if (ch <= 0xf) return ('A' + ch - 10);      // handle hexidecimal specific values
    return('X');                                // Someone screwed up
}


//---------------------------------------------------------------------------
//
// FUNCTION:    GetStringRes (int id INPUT ONLY)
//
// COMMENTS:    Load the resource string with the ID given, and return a
//              pointer to it.  Notice that the buffer is common memory so
//              the string must be used before this call is made a second time.
//
//---------------------------------------------------------------------------

LPTSTR GetStringRes (int id)
{
  static TCHAR buffer[MAX_PATH];

  buffer[0]=0;
  LoadString (GetModuleHandle (NULL), id, buffer, MAX_PATH);
  return buffer;
}
