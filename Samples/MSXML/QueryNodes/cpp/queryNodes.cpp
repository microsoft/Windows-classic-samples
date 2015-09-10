// QueryNodes.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <msxml6.h>

#pragma warning(disable : 4127)  // conditional expression is constant
// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
    HRESULT hr = S_OK;
    BSTR bstr = SysAllocString(wszValue);
    CHK_ALLOC(bstr);

    V_VT(&Variant)   = VT_BSTR;
    V_BSTR(&Variant) = bstr;

CleanUp:
    return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc)
{
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
    if (SUCCEEDED(hr))
    {
        // these methods should not fail so don't inspect result
        (*ppDoc)->put_async(VARIANT_FALSE);  
        (*ppDoc)->put_validateOnParse(VARIANT_FALSE);
        (*ppDoc)->put_resolveExternals(VARIANT_FALSE);
        (*ppDoc)->put_preserveWhiteSpace(VARIANT_TRUE);
    }
    return hr;
}

// Helper function to display parse error.
// It returns error code of the parse error.
HRESULT ReportParseError(IXMLDOMDocument *pDoc, _Null_terminated_ const WCHAR* pwszDesc)
{
    HRESULT hr = S_OK;
    HRESULT hrRet = E_FAIL; // Default error code if failed to get from parse error.
    IXMLDOMParseError *pXMLErr = NULL;
    BSTR bstrReason = NULL;

    CHK_HR(pDoc->get_parseError(&pXMLErr));
    CHK_HR(pXMLErr->get_errorCode(&hrRet));
    CHK_HR(pXMLErr->get_reason(&bstrReason));
    wprintf(L"%s\n%s\n", pwszDesc, bstrReason);

CleanUp:
    SAFE_RELEASE(pXMLErr);
    SysFreeString(bstrReason);
    return hrRet;
}

HRESULT queryNodes()
{
    HRESULT hr = S_OK;
    IXMLDOMDocument *pXMLDom = NULL;
    IXMLDOMNodeList *pNodes = NULL;
    IXMLDOMNode *pNode = NULL;

    BSTR bstrQuery1 = NULL;
    BSTR bstrQuery2 = NULL;
    BSTR bstrNodeName = NULL;
    BSTR bstrNodeValue = NULL;
    VARIANT_BOOL varStatus;
    VARIANT varFileName;
    VariantInit(&varFileName);

    CHK_HR(CreateAndInitDOM(&pXMLDom));

    CHK_HR(VariantFromString(L"stocks.xml", varFileName));
    CHK_HR(pXMLDom->load(varFileName, &varStatus));
    if (varStatus != VARIANT_TRUE)
    {
        CHK_HR(ReportParseError(pXMLDom, L"Failed to load DOM from stocks.xml."));
    }

    // Query a single node.
    bstrQuery1 = SysAllocString(L"//stock[1]/*");
    CHK_ALLOC(bstrQuery1);
    CHK_HR(pXMLDom->selectSingleNode(bstrQuery1, &pNode));
    if (pNode)
    {
        wprintf(L"Result from selectSingleNode:\n");
        CHK_HR(pNode->get_nodeName(&bstrNodeName));
        wprintf(L"Node, <%s>:\n", bstrNodeName);
        SysFreeString(bstrNodeName);

        CHK_HR(pNode->get_xml(&bstrNodeValue));
        wprintf(L"\t%s\n\n", bstrNodeValue);
        SysFreeString(bstrNodeValue);
        SAFE_RELEASE(pNode);
    }
    else
    {
        CHK_HR(ReportParseError(pXMLDom, L"Error while calling selectSingleNode."));
    }

    // Query a node-set.
    bstrQuery2 = SysAllocString(L"//stock[1]/*");
    CHK_ALLOC(bstrQuery2);
    CHK_HR(pXMLDom->selectNodes(bstrQuery2, &pNodes));
    if(pNodes)
    {
        wprintf(L"Results from selectNodes:\n");
        //get the length of node-set
        long length;
        CHK_HR(pNodes->get_length(&length));
        for (long i = 0; i < length; i++)
        {
            CHK_HR(pNodes->get_item(i, &pNode));
            CHK_HR(pNode->get_nodeName(&bstrNodeName));
            wprintf(L"Node (%d), <%s>:\n", i, bstrNodeName);
            SysFreeString(bstrNodeName);

            CHK_HR(pNode->get_xml(&bstrNodeValue));
            wprintf(L"\t%s\n", bstrNodeValue);
            SysFreeString(bstrNodeValue);
            SAFE_RELEASE(pNode);
        }
    }
    else
    {
        CHK_HR(ReportParseError(pXMLDom, L"Error while calling selectNodes."));
    }

CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pNodes);
    SAFE_RELEASE(pNode);
    SysFreeString(bstrQuery1);
    SysFreeString(bstrQuery2);
    SysFreeString(bstrNodeName);
    SysFreeString(bstrNodeValue);
    VariantClear(&varFileName);
    
    return hr;
}

int __cdecl wmain()
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        hr = queryNodes();
        if(FAILED(hr))
        {
            wprintf(L"Failed.\n");
        }
        CoUninitialize();
    }
    return 0;

}
