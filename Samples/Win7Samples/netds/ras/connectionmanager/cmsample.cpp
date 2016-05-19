//+----------------------------------------------------------------------------
//
// File:     cmsample.cpp 
//      
// Module:   CMSAMPLE.DLL
//
// Synopsis: Main source for changing proxy file setting using a Tunnel Address
//           with Connection Manager
//
// Copyright (c) 2001 - 2002 Microsoft Corporation. All rights reserved.
//
//+----------------------------------------------------------------------------
#include <windows.h>
#include <strsafe.h>

// Function prototypes

// Main Entry point
HRESULT WINAPI SetProxyUsingTunnelAddress(HWND hWnd, HINSTANCE hInst, 
                                          LPSTR pszArgs, DWORD nShow);
LPSTR *GetArgV(LPSTR pszCmdLine);
BOOL WriteProxyServerSettingToFile(LPCSTR pszSourceFile, LPSTR pszProxyServer);
BOOL ReadProxyServerByTunnelAddressFromFile(LPCSTR pszSourceFile, 
                                            LPSTR pszTunnelAddress, 
											LPSTR *ppszProxyServer);

#define CMSAMPLE_STARTING_BUF_SIZE  256	// Starting size of the string buffer
#define MAX_CMD_ARGS                15	// Maximum number of arguments expected

// Enumerations to keep pointer state for parsing command line arguments
typedef enum _CMDLN_STATE
{
    CS_END_SPACE,   // handling end space
    CS_BEGIN_QUOTE, // we've encountered a begin quote
    CS_END_QUOTE,   // we've encountered an end quote
    CS_CHAR,        // we're scanning chars
    CS_DONE
} CMDLN_STATE;




extern "C" BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,	    // handle to DLL module 
    DWORD       fdwReason,		// reason for calling function 
    LPVOID      lpvReserved 	// reserved 
)
{
    return TRUE;
}


//+----------------------------------------------------------------------------
//
// Function:  SetProxyUsingTunnelAddress
//
// Synopsis:  Entry point for changing the proxy file settings using a tunnel
//            address. The parameters to the dll are passed via a string which 
//			  contains parameters.
//
// Arguments: HWND hWnd         - Window handle of caller
//            HINSTANCE hInst   - Instance handle of caller
//            LPSTR pszArgs     - Argument string
//            DWORD nShow       - Reserved for future use
//
// Returns:   HRESULT           - S_OK or S_FALSE
//
//+----------------------------------------------------------------------------
HRESULT WINAPI SetProxyUsingTunnelAddress(HWND hWnd, HINSTANCE hInst, 
                                          LPSTR pszArgs, DWORD nShow)
{
    // If we wanted to fail the connection we would return E_FAIL, since we  
    // still want to connect even if this custom action fails, we just return S_FALSE (by default)
    // or S_OK in a successful case
    HRESULT hr = S_FALSE;		
    LPSTR* ArgV = NULL;
    LPSTR pszServiceDir = NULL;
    LPSTR pszTunnelAddress = NULL;
    LPSTR pszProxyFile = NULL;
    LPSTR pszTunnelFile = NULL;
    LPSTR pszTunnelSettingFilePath = NULL;
    LPSTR pszProxyFilePath = NULL;
    LPSTR pszProxyServer = NULL;
    DWORD dwTunnelPathLen = 0;
    DWORD dwProxyPathLen = 0;
    HANDLE hCurrentHeap = GetProcessHeap();
    int i = 0;
    DWORD lcid = 0;

	//  Parse out the command line parameters
	//  command line is of the form: 
    //      /ServiceDir %SERVICEDIR% /TunnelAddress %TUNNELADDRESS% /ProxyFile 
    //          <PROXYFILE> /TunnelFile <TUNNELFILE>

	// Check if we have any arguments
    if (!pszArgs)
    {
		goto exit;
    }

	// Separate each argument in the string by '\0' and return a list of 
    // pointers to each argument
    ArgV = GetArgV(pszArgs);
    
	// Check if we have any valid parsed arguments
    if (!ArgV)
    {
        goto exit;
    }

	// Search the command line arguments for the following switches and their
	// corresponding values. Since actual paramter value is at i+1, make sure i 
    // is less than the maximum allowed parameters - 1. 
    
    
    lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    while (ArgV[i] && ((MAX_CMD_ARGS-1) > i))
    {
        if ((CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE,  ArgV[i], -1, "/ServiceDir", -1)) && ArgV[i+1])
        {
			pszServiceDir = (ArgV[i+1]);
			i = i+2;
        }
		else if ((CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE,  ArgV[i], -1, "/TunnelAddress", -1)) && ArgV[i+1])
        {
			pszTunnelAddress = ArgV[i+1];
			i = i+2;            
        }
		else if ((CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE,  ArgV[i], -1, "/ProxyFile", -1)) && ArgV[i+1])
        {
			pszProxyFile = ArgV[i+1];
			i = i+2;            
        }
		else if ((CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE,  ArgV[i], -1, "/TunnelFile", -1)) && ArgV[i+1])
        {
			pszTunnelFile = ArgV[i+1];
			i = i+2;            
        }
        else
        {
            //  Unknown option.  
            i++;
        }
    }	

	// Make sure we have values for the arguments
	if (!pszServiceDir || !pszTunnelAddress || !pszProxyFile || !pszTunnelFile)
	{
        goto exit;
	}

	// Check to see if we got zero length string values from the command line
    // arguments. Exit if that is the case.
	if (!(*pszServiceDir) || !(*pszTunnelAddress) ||	
		!(*pszProxyFile) || !(*pszTunnelFile))
	{
        goto exit;
	}

	// Calculate the string size for the two paths that need to be created
	// Add 2 for the NULL and backslash
    dwTunnelPathLen = lstrlen(pszServiceDir) +  lstrlen(pszTunnelFile) + 2; 
	dwProxyPathLen = lstrlen(pszServiceDir) +  lstrlen(pszProxyFile) + 2;	

	// Allocate the memory
	pszTunnelSettingFilePath = (LPSTR)HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY,
                                dwTunnelPathLen);	// ANSI - char == byte
	if (!pszTunnelSettingFilePath)
	{
        goto exit;
	}

	pszProxyFilePath = (LPSTR)HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY, 
                                dwProxyPathLen);	// ANSI - char == byte
	if (!pszProxyFilePath)
	{
        goto exit;
	}
	
	// Since we allocated pszTunnelSettingFilePath, we know that it has enough
    // room to fit the path. Otherwise we would have to use _snprintf or wnsprintf.
    // Copy the Service directory to create the full path.
	StringCchPrintf(pszTunnelSettingFilePath, dwTunnelPathLen, "%s\\%s", pszServiceDir, pszTunnelFile);
    
    // Since we allocated pszTunnelSettingFilePath, we know that it has enough
    // room to fit the path. Otherwise we would have to use _snprintf or wnsprintf.
    //Create the full path to the Proxy file.
	StringCchPrintf(pszProxyFilePath, dwProxyPathLen, "%s\\%s", pszServiceDir, pszProxyFile);
	
	if (ReadProxyServerByTunnelAddressFromFile(pszTunnelSettingFilePath, 
                                               pszTunnelAddress, 
                                               &pszProxyServer))
	{
		// Call WriteProxyServerSettingToFile - the function checks for empty 
        // strings
		if(WriteProxyServerSettingToFile(pszProxyFilePath, pszProxyServer))
		{
			hr = S_OK;
		}
	}
	
exit:
	// Clean up allocated memory
	// Delete the argument pointers, Tunnel Server path, Proxy file path and 
    // ProxyServer name pointers
    if (ArgV)
	    HeapFree(hCurrentHeap, 0, (LPVOID)ArgV);	
    
    if (pszTunnelSettingFilePath)
	    HeapFree(hCurrentHeap, 0, (LPVOID)pszTunnelSettingFilePath);
	
    if (pszProxyFilePath)
        HeapFree(hCurrentHeap, 0, (LPVOID)pszProxyFilePath);
	
    if (pszProxyServer)
        HeapFree(hCurrentHeap, 0, (LPVOID)pszProxyServer);

	return hr;
}



//+----------------------------------------------------------------------------
//
// Function:  ReadProxyServerByTunnelAddressFromFile
//
// Synopsis:  Reads the proxy settings from the given proxy file and stores 
//            them in the provided pointer.  Please note that the buffers 
//            allocated here and stored in ppszProxyServer must be freed by   
//			  the caller. If the TunnelAddress doesn't exist in the 
//			  pszSourceFile this function still allocates memory and returns 
//            an empty string.
//
// Arguments: LPCSTR pszSourceFile    - file from which to read the proxy 
//                                      settings
//            LPSTR  pszTunnelAddress - string containing the TunnelAddress 
//										used to look up the ProxyServer value
//            OUT LPSTR  *ppszProxyServer - string pointer that will have the Proxy
//                                      server value (in server:port format)
//
// Returns:   BOOL - TRUE if the settings were successfully read
//
//+----------------------------------------------------------------------------
BOOL ReadProxyServerByTunnelAddressFromFile(LPCSTR pszSourceFile, 
                                            LPSTR pszTunnelAddress, 
											LPSTR *ppszProxyServer)
{
	BOOL bReturn = FALSE;
	BOOL bExit = FALSE;
	DWORD dwReturnedSize = 0;
	DWORD dwSize = CMSAMPLE_STARTING_BUF_SIZE;		
    HANDLE hCurrentHeap = GetProcessHeap();

    // Section in Tunnel to read
    const CHAR* const c_pszTunnelAddressSection = "Tunnel Address";	

    //  Check parameters
	if ((NULL == ppszProxyServer) || (NULL == pszSourceFile) || 
        (NULL == pszTunnelAddress))
	{
		return FALSE;
	}

	// Check for empty strings
	if (!(*pszSourceFile) || !(*pszTunnelAddress))
	{
		return FALSE;
	}

	// Set the incoming pointer to NULL
	*ppszProxyServer = NULL;


	// In case the original buffer size is too small, the loop will try to 
    // allocate more buffer space and try to read the value until. The 
	// loop will exist if the value properly fits into the buffer or the size
	// exceeds 2048. 
    do
    {
        if (*ppszProxyServer)
        {
		    // Free allocated memory
		    HeapFree(hCurrentHeap, 0, (LPVOID)*ppszProxyServer);
		    *ppszProxyServer = NULL;
        }

		// Allocate space for the ProxyServer name
        *ppszProxyServer = (LPSTR)HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY,
                                            dwSize);	//ANSI - char == byte

        if (*ppszProxyServer)
        {
			// Since memory allocation succeeded, read the value from the 
            // settings file
            dwReturnedSize = GetPrivateProfileString(c_pszTunnelAddressSection,
                                            pszTunnelAddress,  "", 
                                            *ppszProxyServer, dwSize, 
                                            pszSourceFile);
            
			// Check if the value fits into the buffer
            if ((dwReturnedSize == (dwSize - 2))  || 
                (dwReturnedSize == (dwSize - 1)))
            {
                //  The buffer is too small, lets allocate a bigger one
                dwSize = 2*dwSize;
                if (dwSize > 2048)
                {
                    // Allocation above 2K, need to exit
					HeapFree(hCurrentHeap, 0, (LPVOID)*ppszProxyServer);
					*ppszProxyServer = NULL;
					goto exit;
                }
            }
            else if (0 == dwReturnedSize)
            {
                //  Either we got an error, or more likely there was no data
                //  to get
				HeapFree(hCurrentHeap, 0, (LPVOID)*ppszProxyServer);
                *ppszProxyServer = NULL;
                goto exit;
            }
            else
            {
				// The function read in the data correctly. Just return.
                return TRUE;
            }
        }
        else
        {
            bExit = TRUE;
        }

    } while (!bExit);

exit:
	return bReturn;
}


//+----------------------------------------------------------------------------
//
// Function:  WriteProxyServerSettingToFile
//
// Synopsis:  Writes the specified settings to the given backup proxy filename.
//            Please see the above format guide for specifics.
//
// Arguments: LPCSTR pszSourceFile - file to write the current settings to
//            LPSTR pszProxyServer - proxy server string in server:port format
//
// Returns:   BOOL - TRUE if the values were written successfully
//
//+----------------------------------------------------------------------------
BOOL WriteProxyServerSettingToFile(LPCSTR pszSourceFile, LPSTR pszProxyServer)
{
	BOOL bReturn = FALSE;
    
    // Section in Proxy file to update
    const CHAR* const c_pszManualProxySection = "Manual Proxy";	

    // Key in Proxy file to update
    const CHAR* const c_pszProxyServer = "ProxyServer";		

    //  Check input params
	if ( (NULL == pszSourceFile) || (NULL == pszProxyServer))
	{
		return bReturn;
	}

	// Check for empty strings
	if (!(*pszSourceFile) || !(*pszProxyServer))
	{
		return bReturn;
	}

    //  Save the Proxy Server name to the Proxy setting file
    if (WritePrivateProfileString(c_pszManualProxySection, c_pszProxyServer, 
                                  pszProxyServer, pszSourceFile))
    {
        bReturn = TRUE;
    }

	return bReturn;
}



//+----------------------------------------------------------------------------
//
// Function:  GetArgV
//
// Synopsis:  Simulates ArgV using GetCommandLine to parse the parameters
//
// Arguments: LPSTR pszCmdLine - Ptr to a copy of the command line to be 
//                               processed
//
// Returns:   LPSTR * - Ptr to a ptr array containing the arguments. Caller is
//                       responsible for releasing memory.
//
//				
//+----------------------------------------------------------------------------
LPSTR *GetArgV(LPSTR pszCmdLine)
{   
    HANDLE hCurrentHeap = GetProcessHeap();

    if (NULL == pszCmdLine || NULL == pszCmdLine[0])
    {
        return NULL;
    }

    // Allocate Ptr array, up to MAX_CMD_ARGS ptrs
	LPSTR *ppArgV = (LPSTR *) HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY, 
                                        sizeof(LPSTR) * MAX_CMD_ARGS);

    if (NULL == ppArgV)
    {
        return NULL;
    }

    // Declare locals
    LPSTR pszCurr = NULL;
    LPSTR pszNext = NULL;
    LPSTR pszToken = NULL;
    CMDLN_STATE state;
    state = CS_CHAR;
    int ndx = 0;  

    // Parse out pszCmdLine and store pointers in ppArgV
    pszCurr = pszToken = pszCmdLine;

    do
    {
        switch (*pszCurr)
        {
            case ' ':
                if (state == CS_CHAR)
                {
                    // We found a token                
                    pszNext = CharNext(pszCurr);
                    *pszCurr = '\0';

                    ppArgV[ndx] = pszToken;
                    ndx++;

                    pszCurr = pszToken = pszNext;
                    state = CS_END_SPACE;
                    continue;
                }
				else 
                {
                    if (state == CS_END_SPACE || state == CS_END_QUOTE)
				    {
					    pszToken = CharNext(pszToken);
				    }
                }
                
                break;

            case '\"':
                if (state == CS_BEGIN_QUOTE)
                {
                    // We found a token
                    pszNext = CharNext(pszCurr);
                    *pszCurr = '\0';

                    // skip the opening quote
                    pszToken = CharNext(pszToken);
                    
                    ppArgV[ndx] = pszToken;
                    ndx++;
                    
                    pszCurr = pszToken = pszNext;
                    
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case '\0':
                if (state != CS_END_QUOTE)
                {
                    // End of the line, set last token
                    ppArgV[ndx] = pszToken;
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }

        pszCurr = CharNext(pszCurr);
    } while ((state != CS_DONE) && (MAX_CMD_ARGS > ndx));

    return ppArgV;
}
