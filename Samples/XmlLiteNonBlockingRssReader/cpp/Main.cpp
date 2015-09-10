#include "RssReader.hpp"

const LPWSTR defaultRssSite = L"http://social.msdn.microsoft.com/Forums/en-US/xmlandnetfx/threads?outputAs=rss";

void PrintUsage()
{
    wprintf(L"RssReader [-options]\n");
    wprintf(L"Options:\n");
    wprintf(L"    -s:Rss_Website        The rss website. If the argument doesn't exist, then use the default rss site:\n    'http://social.msdn.microsoft.com/Forums/en-US/xmlandnetfx/threads?outputAs=rss' \n");
    wprintf(L"    -sync                 Get Rss content synchronously. If the argument doesn't exist, then get rss content asynchronously.\n");
    wprintf(L"    -h                    Print this message.\n");
    wprintf(L"An example:\n");
    wprintf(L"    RssReader -s:http://social.msdn.microsoft.com/Forums/en-US/xmlandnetfx/threads?outputAs=rss \n");
}
/**
 * Notice that this works for XmlLite with non-blocking feature from Win8.
 *
 */
int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    // parse command line arguments.
    LPWSTR rssSite = defaultRssSite;
    BOOL isAsync = TRUE;
    if (argc > 3)
    {
        PrintUsage();
        return -1;
    }
    for (int i = 1 ; i < argc; i++)
    {
        LPWSTR option = argv[i];
        if (0 == wcscmp(option, L"-h"))
        {
            PrintUsage();
            return 0;
        }
        else if (0 == wcscmp(option, L"-sync"))
        {
             if (FALSE == isAsync)
             {
                 PrintUsage();
                 return -1;
             }
             isAsync = FALSE;
        }
        else
        {
            wchar_t* delimit = wcschr(option, L':');
            if (NULL == delimit)
            {
                PrintUsage();
                return -1;
            }
            (*delimit) = L'\0';
            if (0 != wcscmp(option, L"-s"))
            {
                PrintUsage();
                return -1;
            }
            (*delimit) = L':';
            rssSite = delimit + 1;
            if (L'\0' == *rssSite)
            {
                PrintUsage();
                return -1;
            }
        }

    }

    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    CRssReader reader;
    if (isAsync)
    {
        reader.ReadAsync(rssSite);
    }
    else
    {
        reader.ReadSync(rssSite);
    }

    ::CoUninitialize();

    return 0;
}