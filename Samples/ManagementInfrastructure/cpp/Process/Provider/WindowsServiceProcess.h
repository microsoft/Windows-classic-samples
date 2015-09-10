//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//


#ifndef __WINDOWSSERVICEPROCESS_H
#define __WINDOWSSERVICEPROCESS_H

#include <MI.h>
#include <Windows.h>
#include <strsafe.h>
#include "MSFT_WindowsServiceProcess.h"
#include "helper.h"


void GetServiceInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace, 
                            _In_ MI_Uint32 handleID, _In_opt_ const MI_Instance *existingEndPoint);

void GetProcessInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace,
                             _In_z_ const MI_Char* queryString, _In_opt_ const MI_Instance *existingEndPoint);

void GetInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace,
                    _In_z_ const MI_Char* queryString, _In_opt_ const MI_Instance *existingEndPoint,
                    _In_ MI_Boolean bExistingEndPointIsProcessObject);

MI_Result PostInstance( _In_ MI_Context* context, _In_opt_ const MI_Instance *miInstance, _In_opt_ const MI_Instance *existingEndPoint,
                          _In_ MI_Boolean bExistingEndPointIsProcessObject);



#endif 
