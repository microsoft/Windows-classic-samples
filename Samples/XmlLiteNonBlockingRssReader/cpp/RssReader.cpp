#include "RssReader.hpp"

CRssReader::CRssReader()
{
    _ulRef = 1;
    _spXmlReader = NULL;
}

CRssReader::~CRssReader()
{
    ASSERT(_ulRef == 1);
    SAFE_RELEASE(_spXmlReader);
}

HRESULT CRssReader::ReadAsync(LPCWSTR pwszUrl)
{
    HRESULT hr = S_OK;

    IBindCtx* spBindCtx = NULL;
    IMoniker* spMoniker = NULL;
    IStream*  spStream = NULL;

    _bAsync = true;

    CHKHR(::CreateURLMonikerEx(NULL, pwszUrl, &spMoniker, URL_MK_UNIFORM));
    CHKHR(::CreateAsyncBindCtx(0, static_cast<IBindStatusCallback*>(this), NULL, &spBindCtx));
    CHKHR(::CreateXmlReader(IID_IXmlReader, (void**) &_spXmlReader, NULL));

    // Kicks off the binding.
    #pragma warning(suppress: 6011) //spMoniker wouldn't be null if CreateURLMonikerEx() succeeds.
    CHKHR(spMoniker->BindToStorage(spBindCtx, NULL, IID_IStream, (void **)&spStream));

    // Pumps message until the download is finished.
    MSG msg;
    while (_bCompleted == false)
    {
        if (::PeekMessage(&msg, 0, 0 ,0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    CHKHR(::RevokeBindStatusCallback(spBindCtx, static_cast<IBindStatusCallback*>(this)));  

CleanUp:
    SAFE_RELEASE(spBindCtx);
    SAFE_RELEASE(spMoniker);
    SAFE_RELEASE(spStream);
    SAFE_RELEASE(_spXmlReader);
    return hr;
}

HRESULT CRssReader::ReadSync(LPCWSTR pwszUrl)
{
    HRESULT hr = S_OK;

    IBindCtx* spBindCtx = NULL;
    IMoniker* spMoniker = NULL;
    IStream*  spStream = NULL;

    _bAsync = false;

    CHKHR(URLOpenBlockingStream(NULL, pwszUrl, &spStream, NULL, NULL));
    CHKHR(::CreateXmlReader(IID_IXmlReader, (void**) &_spXmlReader, NULL));
    CHKHR(_spXmlReader->SetInput(spStream));
    
    CHKHR(Parse());

CleanUp:
    SAFE_RELEASE(spBindCtx);
    SAFE_RELEASE(spMoniker);
    SAFE_RELEASE(spStream);
    SAFE_RELEASE(_spXmlReader);
    return hr;
}

HRESULT STDMETHODCALLTYPE CRssReader::QueryInterface(REFIID riid, void **ppvObject)
{
    if (ppvObject == NULL)
        return E_INVALIDARG;

    if (riid == IID_IUnknown || riid == IID_IBindStatusCallback)
    {
        *ppvObject = dynamic_cast<IBindStatusCallback*>(this);
        return S_OK;
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE CRssReader::AddRef(void)
{
    _ulRef++;

    return _ulRef;
}

ULONG STDMETHODCALLTYPE CRssReader::Release(void)
{
    _ulRef--;

    return _ulRef;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnStartBinding(DWORD , __RPC__in_opt IBinding*)
{
    _bCompleted = false;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::GetPriority(__RPC__out LONG *pnPriority)
{
    *pnPriority = NORMAL_PRIORITY_CLASS;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnLowResource(DWORD )
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnProgress(ULONG , ULONG , ULONG , __RPC__in_opt LPCWSTR )
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnStopBinding(HRESULT , __RPC__in_opt LPCWSTR )
{
    _bCompleted = true;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo)
{
    // Describes how the bind process is handled.
    *grfBINDF = BINDF_NOWRITECACHE | BINDF_RESYNCHRONIZE;

    if (_bAsync)
        *grfBINDF |= BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE;

    // Describes how we want the binding to occur.
    DWORD cbSize;
    cbSize = pbindinfo->cbSize;
    memset(pbindinfo, 0, cbSize);
    pbindinfo->cbSize = cbSize;
    pbindinfo->dwBindVerb = BINDVERB_GET;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnDataAvailable(DWORD , DWORD , FORMATETC *, STGMEDIUM *pstgmed)
{
    HRESULT hr = S_OK; 
    static BOOL firstDataNotification = TRUE;
    if (firstDataNotification)
    {
        hr = _spXmlReader->SetInput(pstgmed->pstm);
        if (SUCCEEDED(hr))
        {
            firstDataNotification = FALSE;
        
            hr = Parse();
            // Expects only partial data is available.
            if (hr == E_PENDING)
                hr = S_OK;
        }
    }
    else
    {
        hr = Parse();

        // Expects only partial data is available.
        if (hr == E_PENDING)
            hr = S_OK;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CRssReader::OnObjectAvailable(__RPC__in REFIID , __RPC__in_opt IUnknown* )
{
    return S_OK;
}

HRESULT CRssReader::Parse()
{
    HRESULT hr;

    XmlNodeType nodeType;
    const wchar_t* pwszPrefix;
    const wchar_t* pwszLocalName;
    const wchar_t* pwszValue;
    UINT cwchPrefix;

    // Parses the stream with XmlLite when progressing.
    while (S_OK == (hr = _spXmlReader->Read(&nodeType)))
    {
        const UINT buffSize = 8;
        wchar_t buff[buffSize];
        UINT charsRead = 0;

        switch (nodeType)
        {
        case XmlNodeType_XmlDeclaration:
            wprintf(L"XmlDeclaration\n");
            break;
        case XmlNodeType_Element:
            CHKHR(_spXmlReader->GetPrefix(&pwszPrefix, &cwchPrefix));
            CHKHR(_spXmlReader->GetLocalName(&pwszLocalName, NULL));
            if (cwchPrefix > 0)
                wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"Element: %s\n", pwszLocalName);
            if (_spXmlReader->IsEmptyElement() )
                wprintf(L" (empty)");
            break;
        case XmlNodeType_EndElement:
            CHKHR(_spXmlReader->GetPrefix(&pwszPrefix, &cwchPrefix));
            CHKHR(_spXmlReader->GetLocalName(&pwszLocalName, NULL));
            if (cwchPrefix > 0)
                wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"End Element: %s\n", pwszLocalName);
            break;
        case XmlNodeType_Text:
            CHKHR(_spXmlReader->GetValue(&pwszValue, NULL));
            wprintf(L"Text: %s\n", pwszValue);
            break;
        case XmlNodeType_Whitespace:
            wprintf(L"Whitespace: ");
            while (true)
            {
                CHKHR(_spXmlReader->ReadValueChunk(buff, buffSize - 1, &charsRead));
                if (S_FALSE == hr || 0 == charsRead)
                    break;
                buff[charsRead] = NULL;
                wprintf(L"%s", buff);
            }
            wprintf(L"\n");
            break;
        case XmlNodeType_CDATA:
            CHKHR(_spXmlReader->GetValue(&pwszValue, NULL));
            wprintf(L"CDATA: %s\n", pwszValue);
            break;
        case XmlNodeType_ProcessingInstruction:
            CHKHR(_spXmlReader->GetLocalName(&pwszLocalName, NULL));
            CHKHR(_spXmlReader->GetValue(&pwszValue, NULL));
            wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue);
            break;
        case XmlNodeType_Comment:
            CHKHR(_spXmlReader->GetValue(&pwszValue, NULL));
            wprintf(L"Comment: %s\n", pwszValue);
            break;
        case XmlNodeType_DocumentType:
            wprintf(L"DOCTYPE is not printed\n");
            break;
        default:
            wprintf(L"Unknown node type\n");
            break;
        }
    }

CleanUp:
    return hr;
}