// This is the main function of the executable hosting the service.
// It parses the command line and passes the parameters to the service.
// From a Sapphire perspective there is not much to see here.

#include "Service.h"
#include "string.h"
#include "wtypes.h"
#include "stdlib.h"
#include "intsafe.h"

HRESULT ParseCommandLine(
    __in int argc, 
    __in_ecount(argc) wchar_t** argv,
    __out MESSAGE_ENCODING* messageEncoding,
    __out DWORD* chunkSize, 
    __out long* maxConnections,
    __out REPORTING_LEVEL* reportingLevel, 
    __in bool server)
{
    *messageEncoding = DEFAULT_ENCODING;
    *chunkSize = 32768;
    *maxConnections = 100;
    *reportingLevel = REPORT_ERROR;
    bool reportingSet = false;

    // Parse the optional parameters.
    for (int i = 0; i < argc; i++)
    {
        WCHAR* arg = argv[i];
        if (!_wcsicmp(arg, L"-reporting:error") || !_wcsicmp(arg, L"/reporting:error"))
        {
            if (reportingSet)
            {
                wprintf(L"Error: More than one reporting level specified.\n");
                return E_FAIL;
            }

            *reportingLevel = REPORT_ERROR;
            reportingSet = true;
        }
        else if (!_wcsicmp(arg, L"-reporting:info") || !_wcsicmp(arg, L"/reporting:info"))
        {
            if (reportingSet)
            {
                wprintf(L"Error: More than one reporting level specified.\n");
                return E_FAIL;
            }

            reportingSet = true;
            *reportingLevel = REPORT_INFO;
        }
        else if (!_wcsicmp(arg, L"-reporting:verbose") || !_wcsicmp(arg, L"/reporting:verbose"))
        {
            if (reportingSet)
            {
                wprintf(L"Error: More than one reporting level specified.\n");
                return E_FAIL;
            }

            reportingSet = true;
            *reportingLevel = REPORT_VERBOSE;
        }
        else if (wcsstr(arg, L"-reporting:") || wcsstr(arg, L"-reporting:"))
        {
            wprintf(L"Error: Illegal reporting level specified.\n");
            return E_FAIL;
        }
        else if (!_wcsicmp(arg, L"-encoding:binary") || !_wcsicmp(arg, L"/encoding:binary"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = BINARY_ENCODING;
        }
        else if (!_wcsicmp(arg, L"-encoding:text") || !_wcsicmp(arg, L"/encoding:text"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = TEXT_ENCODING;
        }
        else if (!_wcsicmp(arg, L"-encoding:MTOM") || !_wcsicmp(arg, L"/encoding:MTOM"))
        {
            if (DEFAULT_ENCODING != *messageEncoding)
            {
                wprintf(L"Error: More than one encoding specified.\n");
                return E_FAIL;
            }

            *messageEncoding = MTOM_ENCODING;
        }
        else if (!_wcsnicmp(arg, L"-encoding:", 10) || !_wcsnicmp(arg, L"/encoding:", 10))
        {
            wprintf(L"Error: Illegal encoding specified.\n");
            return E_FAIL;
        }
        else if (!_wcsnicmp(arg, L"-chunk:", 7) || !_wcsnicmp(arg, L"/chunk:", 7))
        {
            if (!server)
            {
                wprintf(L"Chunk is not a legal setting on the client side.\n");
                return E_FAIL;
            }

            *chunkSize = wcstoul(&arg[7], NULL, 10);    
        }
        else if (!_wcsnicmp(arg, L"-connections:", 13) || !_wcsnicmp(arg, L"/connections:", 13))
        {
            *maxConnections = wcstol(&arg[13], NULL, 10);    
        }
        else
        {
            wprintf(L"Unrecognized parameter: %s.\n", arg);
            return E_FAIL;
        }
    }

    return S_OK;
}

int __cdecl wmain(
    __in int argc, 
    __in_ecount(argc) wchar_t** argv)
{
    bool server = true;
    TRANSPORT_MODE transport = TCP_TRANSPORT;
    SECURITY_MODE securityMode = NO_SECURITY;

    if (argc < 3)
    {
        wprintf(L"Usage:\n FileRepService.exe <server/client> <Service Url> [/reporting:<error/verbose>] [/encoding:<text/binary/MTOM>]");
        wprintf(L" [/connections:<number of connections>] [/chunk:<size of a the payload per message>]\n");

        return -1;
    }

    if (!_wcsicmp(argv[1], L"client"))
    {
        server = false;
    }
    else if (_wcsicmp(argv[1], L"server"))
    {
        wprintf(L"Must specify server or client\n");
        return -1;
    }

    LPWSTR url = argv[2];

    // Default settings of optional parameters.
    MESSAGE_ENCODING messageEncoding = DEFAULT_ENCODING;
    DWORD chunkSize = 32768;
    long maxConnections = 100;
    REPORTING_LEVEL reportingLevel = REPORT_ERROR;

    if (argc > 3)
    {        
        if (FAILED(ParseCommandLine(argc - 3, &argv[3], &messageEncoding, &chunkSize, &maxConnections, &reportingLevel, server)))
        {
            return -1;
        }
    }  
    
    if (FAILED(ParseTransport( url, &transport, &securityMode)))
    {
        wprintf(L"Illegal protocol.\n");
        return -1;
    }

    if (chunkSize > MAXMESSAGESIZE -1024) // 1024 for overhead
    {
        wprintf(L"The chunk size specified exceeded the allowed maximum of %d.\n", MAXMESSAGESIZE);
        return -1;
    }
    else if (0 == chunkSize)
    {
        wprintf(L"The chunk size must be greater than zero.\n");
        return -1;
    }

    if (maxConnections < 1) 
    {
        wprintf(L"You must allow at least one connection.\n");
        return -1;
    }

    CFileRep* fileServer = NULL;

    if (server)
    {
        fileServer = new CFileRepServer(reportingLevel, maxConnections, transport, securityMode, messageEncoding, chunkSize); 
    }
    else
    {
        fileServer = new CFileRepClient(reportingLevel, maxConnections, transport, securityMode, messageEncoding); 
    }

    if (!fileServer)
    {
        wprintf(L"Unable to create service.\n");
        return -1;
    }

    ULONG urlSize;
    if (FAILED(SizeTToULong(::wcslen(url),&urlSize)))
    {
        wprintf(L"url string too long. Exiting.\n");
        return -1;
    }
    
    if (FAILED(fileServer->Start(url, urlSize)))
    {
        wprintf(L"Service startup failed. Exiting.\n");
        return -1;
    }
    
    wprintf(L"Startup complete. Press any key to exit.\n");
    
    (void)getchar();

    fileServer->Stop();
    delete fileServer;
    wprintf(L"Shutdown complete.\n");

    return 0;
}

