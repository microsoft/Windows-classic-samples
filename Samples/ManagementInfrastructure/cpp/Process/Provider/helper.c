//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "helper.h"

// Converting the win32 failure code to suitable MI_Result code.
MI_Result ResultFromWin32Error(
    DWORD error)
{
    MI_Result result = MI_RESULT_FAILED;
    switch(error)
    {
    case ERROR_FILE_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case ERROR_PATH_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case ERROR_ACCESS_DENIED: 
        result = MI_RESULT_ACCESS_DENIED;
        break;
    case ERROR_INVALID_HANDLE : 
        result = MI_RESULT_INVALID_PARAMETER;
        break; 
    case ERROR_NOT_ENOUGH_MEMORY : 
        result = MI_RESULT_SERVER_LIMITS_EXCEEDED;
        break;     
    case ERROR_INVALID_DATA : 
        result = MI_RESULT_INVALID_PARAMETER;
        break; 
    case ERROR_NOT_SUPPORTED : 
        result = MI_RESULT_NOT_SUPPORTED;
        break;
    case ERROR_INVALID_PARAMETER : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;     
    case ERROR_INSUFFICIENT_BUFFER : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;    
    case ERROR_PROC_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_BAD_PATHNAME : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;        
    case ERROR_ALREADY_EXISTS : 
        result = MI_RESULT_ALREADY_EXISTS;
        break;    
    case ERROR_NO_DATA : 
        result = MI_RESULT_NOT_FOUND;
        break;  
    case ERROR_NOINTERFACE : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_OBJECT_NAME_EXISTS : 
        result = MI_RESULT_ALREADY_EXISTS;
        break; 
    case ERROR_SERVICE_DOES_NOT_EXIST : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;      
    case ERROR_NO_SUCH_USER : 
        result = MI_RESULT_NOT_FOUND;
        break;       
    case ERROR_NO_SUCH_GROUP : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case DNS_ERROR_RCODE_NAME_ERROR : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case DNS_INFO_NO_RECORDS : 
        result = MI_RESULT_NOT_FOUND;
        break; 
    default : 
        result = MI_RESULT_FAILED;
        break;        
    }
    return result;
}
