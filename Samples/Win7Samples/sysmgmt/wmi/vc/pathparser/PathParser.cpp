// **************************************************************************
//
// Copyright (c) Microsoft Corporation, All Rights Reserved
//
// File:  PathParser.cpp 
//
// Description:
//      WMI Path Parser Sample.
//      This sample shows how to use the Path Parser.  It takes a path as a command
//      line argument and passes it off to the parser to be parsed and then uses various
//      functions to get pieces of the path.
//
// History:
//
// **************************************************************************

#include <objbase.h>
#include <windows.h>                                     
#include <stdio.h>
#include <wbemidl.h> 
#include <wmiutils.h>

//***************************************************************************
//
// DumpServerName
//
// Purpose: Dumps the server name
//
//***************************************************************************

void DumpServerName(IWbemPath * pParser)
{
    HRESULT hr;

    // One could safely assume a maximum server name, but for the sake of example,
    // the code will call GetServer in order to determine how long of name is needed, then
    // do the allocation, then get the name and finally dump it.
    
    DWORD dwSize = 0;
    hr = pParser->GetServer(&dwSize, NULL);
    if(FAILED(hr) || dwSize == 0)
        return;                                 // nothing here, just return

    WCHAR * pServer = new WCHAR[dwSize];
    if(pServer)
    {
        hr = pParser->GetServer(&dwSize,  pServer);
        if(FAILED(hr))
            printf("\nUnexpected failure, hr = 0x%x", hr);
        else
            printf("\nThe server name is <%S>", pServer);
        delete [] pServer;
    }
}

//***************************************************************************
//
// DumpNamespaces
//
// Purpose: Dumps the name spaces
//
//***************************************************************************

void DumpNamespaces(IWbemPath * pParser)
{
    HRESULT hr;

    // first get the number of namespaces

    ULONG lCnt;
    hr  = pParser->GetNamespaceCount(&lCnt);
    if(FAILED(hr))
    {
        printf("\nGetNamespaceCount failed, hr = 0x%x", hr);
        return;
    }
    
    for(ULONG dwCnt = 0; dwCnt < lCnt; dwCnt++)
    {
        if(dwCnt == 0)
            printf("\nNamespaces=");
        else
            printf(",");

        // first determine the necessary buffer size
        
        DWORD dwSize = 0;
        hr = pParser->GetNamespaceAt(dwCnt, &dwSize, NULL);
        if(FAILED(hr) || dwSize == 0)
        {
            printf("\nGetNamespaceAt failed, hr = 0x%x", hr);
            return;
        }

        WCHAR * pNamespace = new WCHAR[dwSize];
        if(pNamespace)
        {
            hr = pParser->GetNamespaceAt(dwCnt, &dwSize,  pNamespace);
            if(FAILED(hr))
                printf("\nGetNamespaceAt failed for index %d, sc = 0x%x", dwCnt, hr);
            else
                printf("%S", pNamespace);
            delete [] pNamespace;
        }
    }
}

//***************************************************************************
//
// DumpClassName
//
// Purpose: Dumps the class name
//
//***************************************************************************

void DumpClassName(IWbemPath * pParser)
{
    HRESULT hr;

    // One could safely assume a maximum class name, but for the sake of example,
    // the code will call GetClassName in order to determine how long of name is needed, then
    // do the allocation, then get the name and finally dump it.
    
    DWORD dwSize = 0;
    hr = pParser->GetClassName(&dwSize, NULL);
    if(FAILED(hr) || dwSize == 0)
        return;                                 // nothing here, just return

    WCHAR * pClassName = new WCHAR[dwSize];
    if(pClassName)
    {
        hr = pParser->GetClassName(&dwSize,  pClassName);
        if(FAILED(hr))
            printf("\nUnexpected failure, hr = 0x%x", hr);
        else
            printf("\nThe class name is <%S>", pClassName);
        delete [] pClassName;
    }
}

//***************************************************************************
//
// DumpKeyList
//
// Purpose: Dumps out the keys
//
//***************************************************************************

void DumpKeyList(IWbemPath * pParser)
{

    HRESULT hr;
    IWbemPathKeyList * pKeyList = NULL;
    hr = pParser->GetKeyList(&pKeyList);
    if(FAILED(hr) || pKeyList == NULL)
    {
        printf("\nNULL KEY LIST *****************");
        return;
    }

    unsigned long uNumKey;
    hr = pKeyList->GetCount(&uNumKey);
    if(FAILED(hr))
    {
        printf("\nGetCount failed, hr = 0x%x", hr);
        return;
    }
    printf("\nThe key count is %d", uNumKey);

    // check for the special case of a singleton.  In this case, the path does not have a named key and the
    // value is "@"

    if(uNumKey == 0)
    {
        ULONGLONG uResponse;
        hr = pKeyList->GetInfo(0, &uResponse);
        if(SUCCEEDED(hr) && (uResponse & WBEMPATH_INFO_CONTAINS_SINGLETON))
        {
            printf("\nPath is a singleton");
            return;
        }
    }

    // for the sake of clarity, this example assumes a maximum name size of 256.  Normally, an application would
    // take one of two approaches;  First, it could just allocate a buffer the size of the text used in the SetText 
    // call and assume that is sufficient for the largest piece.  Second, it can call GetKey2 will a NULL pointer
    // to the name buffer and find out how large it needs to be, as is done in the DumpClassName and 
    // DumpServer routines.
    
     WCHAR wName[256];
     for(DWORD uKeyIx = 0; uKeyIx < uNumKey ; uKeyIx++)
     {
        ULONG uKeyType; 
        DWORD dwSize = 256;

        VARIANT var, var2;
        VariantInit(&var);
        VariantInit(&var2);
        hr = pKeyList->GetKey2(uKeyIx, 0, &dwSize, wName, &var, &uKeyType);
        if(FAILED(hr))
        {
            printf("\nGetKey2 failed, index = %d, hr = 0x%x", uKeyIx, hr);
        }
        else
        {
            // here we use Variant change type as a convenient string conversion routine.
            
            hr = VariantChangeType(&var2, &var, 0, VT_BSTR);
            if(SUCCEEDED(hr))
            {
                printf("\nKey %d, has name <%S>, type %d, value <%S>", uKeyIx, wName, uKeyType, var2.bstrVal);
                VariantClear(&var2);
            }
            else
                printf("\nVariantChangeType failed, hr = 0x%x", hr);
        VariantClear(&var);
        }
     }
}

//***************************************************************************
//
// ParseAndDump
//
// Purpose: Does the actual parse, and dumps information
//
//***************************************************************************

void ParseAndDump(IWbemPath * pParser, WCHAR * pwcPath)
{
    HRESULT hr;
    printf("\nAttempting to parse string <%S>", pwcPath);
    hr = pParser->SetText(WBEMPATH_CREATE_ACCEPT_ALL, pwcPath);
    if(FAILED(hr))
    {
        printf("\nParse failed, return code is 0x%x", hr);
        return;
    }
    DumpServerName(pParser);
    DumpNamespaces(pParser);
    DumpClassName(pParser);
    DumpKeyList(pParser);
}

//***************************************************************************
//
// main
//
// Purpose: Program entry point.  It checks the argument, converts it to unicode,
//               initializes Com, and creates the parser.
//
//***************************************************************************

int main(int iArgCnt, char ** argv)
{

    // First argument is the is the path.  If not present, dump out a usage message.

    if(iArgCnt < 2)
    {
        printf("\nUsage:  PathParser <PathToBeParsed>"
                 "\nExample:  PathParser \\\\MyServer\\root\\default:stdregprov=@"
                 "\nor        PathParser \\\\MyServer\\root\\default:stdregprov.key=\\\"hello\\\""
                 "\nor        PathParser \\\\MyServer\\root\\default:stdregprov.key=23"
                 );
        return 1;
    }

    // Convert to unicode

    int iLen = (int)strlen(argv[1]);
    WCHAR * pwcPath = new WCHAR[iLen+1];
    if(pwcPath == NULL)
    {
        printf("\nFailed due to lack of memory");
        return 1;
    }
	//mbstowcs(wchar_t *, const char *, size_t);
	//mbstowcs(pwcPath, argv[1], iLen + 1);	
	//errno_t mbstowcs_s(size_t *pReturnValue, wchar_t *wcstr, size_t sizeInWords, const char *mbstr, size_t count);
	size_t * intReturnValue = 0;
	mbstowcs_s(intReturnValue, pwcPath, iLen+1, argv[1], iLen + 1);
 
    // Initialize COM and create the path parser object

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if(SUCCEEDED(hr))
    {
        IWbemPath *pParser = NULL;
        hr = CoCreateInstance(CLSID_WbemDefPath, 0, CLSCTX_INPROC_SERVER,
                                                                     IID_IWbemPath, (LPVOID *) &pParser);
        if(SUCCEEDED(hr))
        {
            ParseAndDump(pParser, pwcPath);
            pParser->Release();
        }
        else
            printf("\nCoCreateInstance of CLSID_WbemDefPath, return is 0x%x", hr);
        CoUninitialize();
    }
    delete [] pwcPath;
    printf("\nTerminating normally");
    return 0;
}

