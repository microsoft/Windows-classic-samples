// DynamDOM.cpp : Defines the entry point for the console application.
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

// Helper that allocates the BSTR param for the caller.
HRESULT CreateElement(IXMLDOMDocument *pXMLDom, PCWSTR wszName, IXMLDOMElement **ppElement)
{
    HRESULT hr = S_OK;
    *ppElement = NULL;

    BSTR bstrName = SysAllocString(wszName);
    CHK_ALLOC(bstrName);
    CHK_HR(pXMLDom->createElement(bstrName, ppElement));

CleanUp:
    SysFreeString(bstrName);
    return hr;
}

// Helper function to append a child to a parent node.
HRESULT AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMNode *pChildOut = NULL;
    CHK_HR(pParent->appendChild(pChild, &pChildOut));

CleanUp:
    SAFE_RELEASE(pChildOut);
    return hr;
}

// Helper function to create and add a processing instruction to a document node.
HRESULT CreateAndAddPINode(IXMLDOMDocument *pDom, PCWSTR wszTarget, PCWSTR wszData)
{
    HRESULT hr = S_OK;
    IXMLDOMProcessingInstruction *pPI = NULL;

    BSTR bstrTarget = SysAllocString(wszTarget);
    BSTR bstrData = SysAllocString(wszData);
    CHK_ALLOC(bstrTarget && bstrData);

    CHK_HR(pDom->createProcessingInstruction(bstrTarget, bstrData, &pPI));
    CHK_HR(AppendChildToParent(pPI, pDom));

CleanUp:
    SAFE_RELEASE(pPI);
    SysFreeString(bstrTarget);
    SysFreeString(bstrData);
    return hr;
}

// Helper function to create and add a comment to a document node.
HRESULT CreateAndAddCommentNode(IXMLDOMDocument *pDom, PCWSTR wszComment)
{
    HRESULT hr = S_OK;
    IXMLDOMComment *pComment = NULL;

    BSTR bstrComment = SysAllocString(wszComment);
    CHK_ALLOC(bstrComment);

    CHK_HR(pDom->createComment(bstrComment, &pComment));
    CHK_HR(AppendChildToParent(pComment, pDom));

CleanUp:
    SAFE_RELEASE(pComment);
    SysFreeString(bstrComment);
    return hr;
}

// Helper function to create and add an attribute to a parent node.
HRESULT CreateAndAddAttributeNode(IXMLDOMDocument *pDom, PCWSTR wszName, PCWSTR wszValue, IXMLDOMElement *pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMAttribute *pAttribute = NULL;
    IXMLDOMAttribute *pAttributeOut = NULL; // Out param that is not used

    BSTR bstrName = NULL;
    VARIANT varValue;
    VariantInit(&varValue);

    bstrName = SysAllocString(wszName);
    CHK_ALLOC(bstrName);
    CHK_HR(VariantFromString(wszValue, varValue));

    CHK_HR(pDom->createAttribute(bstrName, &pAttribute));
    CHK_HR(pAttribute->put_value(varValue));
    CHK_HR(pParent->setAttributeNode(pAttribute, &pAttributeOut));

CleanUp:
    SAFE_RELEASE(pAttribute);
    SAFE_RELEASE(pAttributeOut);
    SysFreeString(bstrName);
    VariantClear(&varValue);
    return hr;
}

// Helper function to create and append a text node to a parent node.
HRESULT CreateAndAddTextNode(IXMLDOMDocument *pDom, PCWSTR wszText, IXMLDOMNode *pParent)
{
    HRESULT hr = S_OK;    
    IXMLDOMText *pText = NULL;

    BSTR bstrText = SysAllocString(wszText);
    CHK_ALLOC(bstrText);

    CHK_HR(pDom->createTextNode(bstrText, &pText));
    CHK_HR(AppendChildToParent(pText, pParent));

CleanUp:
    SAFE_RELEASE(pText);
    SysFreeString(bstrText);
    return hr;
}

// Helper function to create and append a CDATA node to a parent node.
HRESULT CreateAndAddCDATANode(IXMLDOMDocument *pDom, PCWSTR wszCDATA, IXMLDOMNode *pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMCDATASection *pCDATA = NULL;

    BSTR bstrCDATA = SysAllocString(wszCDATA);
    CHK_ALLOC(bstrCDATA);

    CHK_HR(pDom->createCDATASection(bstrCDATA, &pCDATA));
    CHK_HR(AppendChildToParent(pCDATA, pParent));

CleanUp:
    SAFE_RELEASE(pCDATA);
    SysFreeString(bstrCDATA);
    return hr;
}

// Helper function to create and append an element node to a parent node, and pass the newly created
// element node to caller if it wants.
HRESULT CreateAndAddElementNode(IXMLDOMDocument *pDom, PCWSTR wszName, PCWSTR wszNewline, IXMLDOMNode *pParent, IXMLDOMElement **ppElement = NULL)
{
    HRESULT hr = S_OK;
    IXMLDOMElement* pElement = NULL;

    CHK_HR(CreateElement(pDom, wszName, &pElement));
    // Add NEWLINE+TAB for identation before this element.
    CHK_HR(CreateAndAddTextNode(pDom, wszNewline, pParent));
    // Append this element to parent.
    CHK_HR(AppendChildToParent(pElement, pParent));

CleanUp:
    if (ppElement)
        *ppElement = pElement;  // Caller is repsonsible to release this element.
    else
        SAFE_RELEASE(pElement); // Caller is not interested on this element, so release it.

    return hr;
}

void dynamDOM()
{
    HRESULT hr = S_OK;
    IXMLDOMDocument *pXMLDom = NULL;
    IXMLDOMElement *pRoot = NULL;
    IXMLDOMElement *pNode = NULL;
    IXMLDOMElement *pSubNode = NULL;
    IXMLDOMDocumentFragment *pDF = NULL;

    BSTR bstrXML = NULL;
    VARIANT varFileName;
    VariantInit(&varFileName);

    CHK_HR(CreateAndInitDOM(&pXMLDom));

    // Create a processing instruction element.
    CHK_HR(CreateAndAddPINode(pXMLDom, L"xml", L"version='1.0'"));

    // Create a comment element.
    CHK_HR(CreateAndAddCommentNode(pXMLDom, L"sample xml file created using XML DOM object."));

    // Create the root element.
    CHK_HR(CreateElement(pXMLDom, L"root", &pRoot));

    // Create an attribute for the <root> element, with name "created" and value "using dom".
    CHK_HR(CreateAndAddAttributeNode(pXMLDom, L"created", L"using dom", pRoot));

    // Next, we will create and add three nodes to the <root> element.
    // Create a <node1> to hold text content.
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"node1", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"some character data", pNode));
    SAFE_RELEASE(pNode);

    // Create a <node2> to hold a CDATA section.
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"node2", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddCDATANode(pXMLDom, L"<some mark-up text>", pNode));
    SAFE_RELEASE(pNode);

    // Create <node3> to hold a doc fragment with three sub-elements.
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"node3", L"\n\t", pRoot, &pNode));

    // Create a document fragment to hold three sub-elements.
    CHK_HR(pXMLDom->createDocumentFragment(&pDF));

    // Create 3 subnodes.
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"subNode1", L"\n\t\t", pDF));
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"subNode2", L"\n\t\t", pDF));
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"subNode3", L"\n\t\t", pDF));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"\n\t", pDF));

    // Append pDF to <node3>.
    CHK_HR(AppendChildToParent(pDF, pNode));
    SAFE_RELEASE(pNode);

    // Add NEWLINE for identation before </root>.
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"\n", pRoot));
    // add <root> to document
    CHK_HR(AppendChildToParent(pRoot, pXMLDom));

    CHK_HR(pXMLDom->get_xml(&bstrXML));
    wprintf(L"Dynamically created DOM:\n%s\n", bstrXML);

    CHK_HR(VariantFromString(L"dynamDOM.xml", varFileName));
    CHK_HR(pXMLDom->save(varFileName));
    wprintf(L"DOM saved to dynamDOM.xml\n");

CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pRoot);
    SAFE_RELEASE(pNode);
    SAFE_RELEASE(pDF);
    SAFE_RELEASE(pSubNode);
    SysFreeString(bstrXML);
    VariantClear(&varFileName);
}

int __cdecl wmain()
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        dynamDOM();
        CoUninitialize();
    }
    return 0;
}
