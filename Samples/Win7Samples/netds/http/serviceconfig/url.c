/*++
 Copyright (c) 2002 - 2002 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT. 

--*/

#include "precomp.h"
#pragma hdrstop

//
// Private functions.
//

/***************************************************************************++

Routine Description:
    Prints a record in the URL ACL store.

Arguments:
    pOutput - A pointer to HTTP_SERVICE_CONFIG_URLACL_SET

Return Value:
    None.

--***************************************************************************/
void
PrintUrlAclRecord(
    IN PUCHAR pOutput
    )
{
    PHTTP_SERVICE_CONFIG_URLACL_SET pSetParam;

    pSetParam = (PHTTP_SERVICE_CONFIG_URLACL_SET) pOutput;

    wprintf(L"URL : %s", pSetParam->KeyDesc.pUrlPrefix);
	
    wprintf(L"ACL : %s", pSetParam->ParamDesc.pStringSecurityDescriptor);

    wprintf(L"------------------------------------------------------------------------------");

}

/***************************************************************************++

Routine Description:
    Sets an URL ACL entry.

Arguments:
    pUrl - The URL
    pAcl - The ACL specified as a SDDL string.

Return Value:
    Success/Failure.

--***************************************************************************/
int 
DoUrlAclSet(
    __in_opt PWSTR pUrl,
    __in_opt PWSTR pAcl
    )
{
    HTTP_SERVICE_CONFIG_URLACL_SET SetParam;
    DWORD                          Status;

    ZeroMemory(&SetParam, sizeof(SetParam));

    SetParam.KeyDesc.pUrlPrefix                  = pUrl;
    SetParam.ParamDesc.pStringSecurityDescriptor = pAcl;

    Status = HttpSetServiceConfiguration(
                NULL,
                HttpServiceConfigUrlAclInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );

    wprintf(L"HttpSetServiceConfiguration completed with %d", Status);

                
    return Status;
}


/***************************************************************************++

Routine Description:
    Queries for a URL ACL entry.

Arguments:
    pUrl - The URL (if NULL, then enumerate the store).

Return Value:
    Success/Failure.

--***************************************************************************/
int DoUrlAclQuery(
    __in_opt PWSTR pUrl
    )
{
    DWORD                             Status;
    PUCHAR                            pOutput = NULL;
    DWORD                             OutputLength = 0;
    DWORD                             ReturnLength = 0;
    HTTP_SERVICE_CONFIG_URLACL_QUERY  QueryParam;

    ZeroMemory(&QueryParam, sizeof(QueryParam));

    if(pUrl)
    {
        // If a URL is specified, we'll Query for an exact entry.
        // 
        QueryParam.QueryDesc = HttpServiceConfigQueryExact;
        QueryParam.KeyDesc.pUrlPrefix = pUrl;
    }
    else
    {
        //
        // No URL is specified, so enumerate the entire store. 
        // 
        QueryParam.QueryDesc = HttpServiceConfigQueryNext;
    }

    for(;;)
    {
        // 
        // First, compute bytes required for querying the first entry.
        //
        Status = HttpQueryServiceConfiguration(
                    NULL,
                    HttpServiceConfigUrlAclInfo,
                    &QueryParam,
                    sizeof(QueryParam),
                    pOutput,
                    OutputLength,
                    &ReturnLength,
                    NULL
                    );

        if(Status == ERROR_INSUFFICIENT_BUFFER)
        {
            // If the API completes with ERROR_INSUFFICIENT_BUFFER, we'll
            // allocate memory for it & continue with the loop where we'll
            // call it again.
            
            if(pOutput)
            {
                // If there was an existing buffer, free it.
                LocalFree(pOutput);
            }

            // Allocate a new buffer
            pOutput = LocalAlloc(LMEM_FIXED, ReturnLength);
            if(!pOutput)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            OutputLength = ReturnLength;
        }
        else if(Status == NO_ERROR)
        {

            // The query succeeded! We'll print the record that we just
            // queried.
            
            PrintUrlAclRecord(pOutput);

            if(pUrl)
            {
                //
                // If we are not enumerating, we are done.
                //
                break;
            }
            else
            {
                //
                // Since we are enumerating, we'll move on to the next
                // record. This is done by incrementing the cursor, till 
                // we get ERROR_NO_MORE_ITEMS.
                //
                QueryParam.dwToken ++;
            }
        }
        else if(ERROR_NO_MORE_ITEMS == Status && pUrl == NULL)
        {
            // We are enumerating and we have reached the end. This is 
            // indicated by a ERROR_NO_MORE_ITEMS error code. 
            
            // This is not a real error, since it is used to indicate that
            // we've finished enumeration.
            
            Status = NO_ERROR;
            break;
        } 
        else
        {
            //
            // Some other error, so we are done
            //
            wprintf(L"HttpQueryServiceConfiguration completed with %d", Status);

            break;
        }
    } 
    
    if(pOutput)
    {
        LocalFree(pOutput);
    }

    return Status;
}

/***************************************************************************++

Routine Description:
    Deletes an URL ACL entry.

Arguments:
    pUrl - The URL

Return Value:
    Success/Failure.

--***************************************************************************/
int DoUrlAclDelete(
    __in_opt PWSTR pUrl
    )
{
    HTTP_SERVICE_CONFIG_URLACL_SET SetParam;
    DWORD                          Status;

    SetParam.KeyDesc.pUrlPrefix = pUrl;
    Status = HttpDeleteServiceConfiguration(
                NULL,
                HttpServiceConfigUrlAclInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );
                
    wprintf(L"HttpDeleteServiceConfiguration completed with %d", Status);
    return Status;
}

//
// Public function.
//

/***************************************************************************++

Routine Description:
    The function that parses parameters specific to URL ACL & 
    calls Set, Query or Delete.

Arguments:
    argc - Count of arguments.
    argv - Pointer to command line arguments.
    Type - Type of operation to be performed.

Return Value:
    Success/Failure.

--***************************************************************************/
int 
DoUrlAcl(
    int          argc, 
    __in_ecount(argc) WCHAR      **argv, 
    HTTPCFG_TYPE Type
    )
{
    PWSTR   pUrl             = NULL;
    PWSTR   pAcl             = NULL;
    WCHAR   **argvSaved      = argv;

    while(argc>=2 && (argv[0][0] == '-' || argv[0][0]== '/'))
    {
        switch(toupper(argv[0][1]))
        {
            case 'U':
                pUrl = argv[1];
                break;
    
            case 'A':
                pAcl = argv[1];
                break;
        
            default:
	    	  wprintf(L"%s is not a valid command.", argv[0]);

                return ERROR_INVALID_PARAMETER;
        }

        argc -=2;
        argv +=2;
    }

    switch(Type)
    {
        case HttpCfgTypeSet:
            return DoUrlAclSet(pUrl, pAcl);

        case HttpCfgTypeQuery:
            return DoUrlAclQuery(pUrl);

        case HttpCfgTypeDelete:
            return DoUrlAclDelete(pUrl);

        default: 
	     wprintf(L"%s is not a valid command.", argvSaved[0]);

            return ERROR_INVALID_PARAMETER;
    }
}
