#include "CHWMFT_DebugLogger.h"
#ifdef  _HMFTLOGGING_
#include <evntrace.h>
#include <strsafe.h>

REGHANDLE    g_ETWHandle    = NULL;
BOOL         g_bEnabled     = FALSE;
UCHAR        g_ucLevel      = 0;

void NTAPI EventRegisterCallback(
    LPCGUID                     lpcguidSourceId,
    ULONG                       ulIsEnabled,
    UCHAR                       ucLevel,
    ULONGLONG                   ullMatchAnyKeyword,
    ULONGLONG                   ullMatchAllKeywords,
    PEVENT_FILTER_DESCRIPTOR    pefdFilterData,
    PVOID                       pvCallbackContext)
{
    switch(ulIsEnabled)
    {
    case EVENT_CONTROL_CODE_ENABLE_PROVIDER:  
        g_bEnabled  = TRUE;
        g_ucLevel   = ucLevel;
        break;
    case EVENT_CONTROL_CODE_DISABLE_PROVIDER:  
        g_bEnabled  = FALSE;
        g_ucLevel   = 0;
        break;
    default:
        // Nothing to do
        break;
    };
}
#endif

void TraceInitialize(void)
{
#ifdef  _HMFTLOGGING_
    if(g_bEnabled == FALSE)
    {
        // Provider ID: {54E23341-C608-4161-97F1-653A9B6FFFF1}
        // TODO: Generate a new unique provider ID. Do not reuse this GUID.
        static const GUID guidTrace = 
            { 0x54e23341, 0xc608, 0x4161, { 0x97, 0xf1, 0x65, 0x3a, 0x9b, 0x6f, 0xff, 0xf1 } };

        EventRegister(
            &guidTrace,
            &EventRegisterCallback,
            NULL,
            &g_ETWHandle
            );
    }
#endif
}

void TraceUninitialize(void)
{
#ifdef  _HMFTLOGGING_
    if(g_ETWHandle != NULL)
    {
        EventUnregister(g_ETWHandle);
    }

    g_ETWHandle = NULL;
    g_bEnabled  = FALSE;
    g_ucLevel   = 0;
#endif
}


void TraceString(
    const UCHAR ucLevel,
    const WCHAR* pwszFormat, 
    ...)
{
#ifdef  _HMFTLOGGING_
    do
    {
        if(g_bEnabled == FALSE)
        {
            // Do not trace
            break;
        }

        if( (ucLevel > g_ucLevel)   &&
            (g_ucLevel != 0)        )
        {
            // Do not trace
            break;
        }

        WCHAR   pwszTrace[1024] = {0};

        va_list args;
        va_start(args, pwszFormat);

        StringCchVPrintfW(
            pwszTrace,
            ARRAYSIZE(pwszTrace),
            pwszFormat,
            args
            );

        va_end(args);

        EventWriteString(g_ETWHandle, ucLevel, 0, pwszTrace);
    }while(false);
#endif
}