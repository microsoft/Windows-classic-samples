// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "prntvpt.h"


/*++
    These are defines for Command Line Arguments
--*/
#define DISPLAY_USAGE                   1
#define PRINT_JOB_SIMPLE                2
#define PRINT_JOB_MULTIPLEPRINTTICKET   4
#define PRINT_JOB_IMAGEPASSTHROUGH      8


#define MAX_PRINTER_NAME    256
#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

const   WCHAR gszPrinterName[] = L"Microsoft XPS Document Writer";

/*++
    The PTPC_STATE_INFO structure is used to cache state information.
    For example, the hPrinter and hProvider may need to be used repeatedly
    so we cache them. Caching hProvider is also efficient if calling the PT/PC
    API's multiple times
--*/
typedef struct _PTPC_STATE_INFO
{
    HANDLE          hPrinter;
    HPTPROVIDER     hProvider;
    WCHAR           szPrinterName[MAX_PRINTER_NAME];
} PTPC_STATE_INFO, *PPTPC_STATE_INFO;


HRESULT
CreatePrintTicketJobSimple(
    VOID
    );

HRESULT
CreatePrintJobMultiplePrintTicket(
    VOID
    );

HRESULT
CreatePrintJobWithImagePassThrough(
    VOID
    );