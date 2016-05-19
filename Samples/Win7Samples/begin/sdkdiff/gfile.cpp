// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * File Open/Create dialogs
 *
 */

/*
 * these dialog functions exist because they were written and
 * used before the commmon dialogs existed.
 *
 * they have now been reduced to just calls to the common file dialog
 * functions
 */



/*---includes-----------------------------------------------------------*/
#include "precomp.h"
#include "gutilsrc.h"

/*--functions----------------------------------------------------------*/

/*
 * gfile_open
 *     	dialog asking the user to select an existing file to open.
 *
 * parameters
 *
 *	prompt - message to user indicating purpose of file
 *		 (to be displayed somewhere in dialog box.
 *
 *	ext    - default file extension if user enters file without
 *		 extension.
 *
 *	spec   - default file spec (eg *.*)
 *
 *	pszFull - buffer where full filename (including path) is returned.
 *
 *	cchMax - size of pszFull buffer.
 *
 *	fn     - buffer where filename (just final element) is returned.
 *
 * returns - true if file selected and exists (tested with OF_EXIST).
 *	     FALSE if dialog cancelled. If user selects a file that we cannot
 *	     open, we complain and restart the dialog.
 *
 *	     if TRUE is returned, the file will have been successfully opened,
 *	     for reading and then closed again.
 */

BOOL 
FAR 
PASCAL
gfile_open(
    HWND hwnd, 
    LPSTR prompt, 
    LPSTR ext, 
    LPSTR spec, 
    LPSTR pszFull, 
    int cchMax, 
    LPSTR fn
    )
{
    OPENFILENAME ofn;
    char achFilters[MAX_PATH];
    char szTmp[MAX_PATH * 2] = {0};
    HANDLE fh;
	HRESULT hr;

    if (!pszFull)
    {
        pszFull = szTmp;
        cchMax = sizeof(szTmp) / sizeof(szTmp[0]);
    }

    if (cchMax < 1)
        return FALSE;

    /* build filter-pair buffer to contain one pair - the spec filter,
     * twice (one of the pair should be the filter, the second should be
     * the title of the filter - we don't have a title so we use the
     * filter both times.
     */
    hr = StringCchPrintf(achFilters, (sizeof(achFilters)/sizeof(achFilters[0])) - 1, "%s%c%s", spec, 0, spec);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_PRINTF);
    /*
     * initialise arguments to dialog proc
     */
    memset(&ofn, 0, sizeof(ofn));
    // GetOpenFileName ang GetSaveFileName unfortunately
    // validate the size of the structue.  So we need to lie to
    // the function if we were built for >=Win2000 and
    // running on an earlier OS
#if (_WIN32_WINNT >= 0x0500) 
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    ofn.lStructSize = sizeof(OPENFILENAME);
#endif
    ofn.hwndOwner = hwnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = achFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;       // first filter pair in list
    pszFull[0] = '\0';
    ofn.lpstrFile = pszFull;        // we need to get the full path to open
    ofn.nMaxFile = cchMax;
    ofn.lpstrFileTitle = fn;        // return final elem of name here
    ofn.nMaxFileTitle = 13;     // assume just big enough for 8.3+null
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = prompt;        // dialog title is good place for prompt text
    ofn.Flags = OFN_FILEMUSTEXIST |
                OFN_HIDEREADONLY |
                OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = ext;

    /*
     * loop until the user cancels, or selects a file that we can open
     */
    do {
        if (!GetOpenFileName(&ofn)) {
            return(FALSE);
        }

        fh = CreateFile(pszFull, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            if (MessageBox(NULL, "File Could Not Be Opened", "File Open",
                           MB_OKCANCEL|MB_ICONSTOP) == IDCANCEL) {
                return(FALSE);
            }
        }
    } while (fh == INVALID_HANDLE_VALUE);

    CloseHandle(fh);

    return(TRUE);
}





/*
 * gfile_new
 *     	dialog asking the user to name a file for writing to.
 *
 * parameters
 *
 *	prompt - message to user indicating purpose of file
 *		 (to be displayed somewhere in dialog box.
 *
 *	ext    - default file extension if user enters file without
 *		 extension.
 *
 *	spec   - default file spec (eg *.*)
 *
 *	pszFull - buffer where full filename (including path) is returned.
 *
 *	cchMax - size of pszFull buffer.
 *
 *	fn     - buffer where filename (just final element) is returned.
 *
 * returns - true if file selected and exists (tested with OF_EXIST).
 *	     FALSE if dialog cancelled. If user selects a file that we cannot
 *	     open, we complain and restart the dialog.
 *
 *	     if TRUE is returned, the file will have been successfully
 *	     created and opened for writing and then closed again.
 */

BOOL 
FAR 
PASCAL
gfile_new(
    LPSTR prompt, 
    LPSTR ext, 
    LPSTR spec, 
    LPSTR pszFull, 
    int cchMax, 
    LPSTR fn
    )
{
    OPENFILENAME ofn;
    char achFilters[MAX_PATH] = {0};
    char szTmp[MAX_PATH * 2];
    HANDLE fh;
	HRESULT hr;

    if (!pszFull)
    {
        pszFull = szTmp;
        cchMax = sizeof(szTmp) / sizeof(szTmp[0]);
    }

    if (cchMax < 1)
        return FALSE;

    /* build filter-pair buffer to contain one pair - the spec filter,
     * twice (one of the pair should be the filter, the second should be
     * the title of the filter - we don't have a title so we use the
     * filter both times. remember double null at end of list of strings.
     */
    hr = StringCchPrintf(achFilters,(sizeof(achFilters)/sizeof(achFilters[0])) - 1, "%s%c%s", spec, 0, spec);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_PRINTF);

    /*
     * initialise arguments to dialog proc
     */
    memset(&ofn, 0, sizeof(ofn));
    // GetOpenFileName ang GetSaveFileName unfortunately
    // validate the size of the structue.  So we need to lie to
    // the function if we were built for >=Win2000 and
    // running on an earlier OS
#if (_WIN32_WINNT >= 0x0500) 
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    ofn.lStructSize = sizeof(OPENFILENAME);
#endif
    ofn.hwndOwner = NULL;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = achFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;       // first filter pair in list
    pszFull[0] = '\0';
    ofn.lpstrFile = pszFull;        // we need to get the full path to open
    ofn.nMaxFile = cchMax;
    ofn.lpstrFileTitle = fn;        // return final elem of name here
    ofn.nMaxFileTitle = 13;     // assume just big enough for 8.3+null
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = prompt;        // dialog title is good place for prompt text
    ofn.Flags = OFN_HIDEREADONLY;
    ofn.lpstrDefExt = ext;

    /*
     * loop until the user cancels, or selects a file that we can create/write
     */
    do {
        if (!GetSaveFileName(&ofn)) {
            return(FALSE);
        }

        fh = CreateFile(pszFull, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            if (MessageBox(NULL, "File Could Not Be Created", "File Open",
                           MB_OKCANCEL|MB_ICONSTOP) == IDCANCEL) {
                return(FALSE);
            }
        }
    } while (fh == INVALID_HANDLE_VALUE);

    CloseHandle(fh);

    return(TRUE);
}
