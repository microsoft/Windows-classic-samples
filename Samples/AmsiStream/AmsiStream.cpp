#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

constexpr wchar_t AppName[] = L"Contoso Script Engine v3.4.9999.0";

class CAmsiStreamBase
{
protected:
    ~CAmsiStreamBase()
    {
        free(m_contentName);
    }

    HRESULT SetContentName(_In_ PCWSTR name)
    {
        m_contentName = _wcsdup(name);
        return m_contentName ? S_OK : E_OUTOFMEMORY;
    }

    HRESULT BaseGetAttribute(
        _In_ AMSI_ATTRIBUTE attribute,
        _In_ ULONG bufferSize,
        _Out_writes_bytes_to_(bufferSize, *actualSize) PBYTE buffer,
        _Out_ ULONG* actualSize)
        //
        // Return Values:
        //   S_OK: SUCCESS
        //   E_NOTIMPL: attribute not supported
        //   E_NOT_SUFFICIENT_BUFFER: need a larger buffer, required size in *retSize
        //   E_INVALIDARG: bad arguments
        //   E_NOT_VALID_STATE: object not initialized
        //
    {
        wprintf(L"GetAttribute() called with: attribute = %u, bufferSize = %u\n", attribute, bufferSize);

        if (actualSize == nullptr || (buffer == nullptr && bufferSize > 0)) {
            return E_INVALIDARG;
        }

        *actualSize = 0;

        switch (attribute)
        {
        case AMSI_ATTRIBUTE_CONTENT_SIZE:
            return CopyAttribute(&m_contentSize, sizeof(m_contentSize), bufferSize, buffer, actualSize);

        case AMSI_ATTRIBUTE_CONTENT_NAME:
            return CopyAttribute(m_contentName, (wcslen(m_contentName) + 1) * sizeof(WCHAR), bufferSize, buffer, actualSize);

        case AMSI_ATTRIBUTE_APP_NAME:
            return CopyAttribute(AppName, sizeof(AppName), bufferSize, buffer, actualSize);

        case AMSI_ATTRIBUTE_SESSION:
            constexpr HAMSISESSION session = nullptr; // no session for file stream
            return CopyAttribute(&session, sizeof(session), bufferSize, buffer, actualSize);
        }

        return E_NOTIMPL; // unsupport attribute
    }

    HRESULT CopyAttribute(
        _In_ const void* resultData,
        _In_ size_t resultSize,
        _In_ ULONG bufferSize,
        _Out_writes_bytes_to_(bufferSize, *actualSize) PBYTE buffer,
        _Out_ ULONG* actualSize)
    {
        *actualSize = (ULONG)resultSize;
        if (bufferSize < resultSize)
        {
            return E_NOT_SUFFICIENT_BUFFER;
        }
        memcpy_s(buffer, bufferSize, resultData, resultSize);
        return S_OK;
    }

    ULONGLONG m_contentSize = 0;
    PWSTR m_contentName = nullptr;
};


class CAmsiFileStream : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAmsiStream>, CAmsiStreamBase
{
public:
	HRESULT RuntimeClassInitialize(_In_ LPCWSTR fileName)
	{
		HRESULT hr = S_OK;

        hr = SetContentName(fileName);
        if (FAILED(hr))
        {
            return hr;
        }

        m_fileHandle.Attach(CreateFileW(fileName,
			GENERIC_READ,             // dwDesiredAccess
			0,                        // dwShareMode
			nullptr,                  // lpSecurityAttributes
			OPEN_EXISTING,            // dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL,    // dwFlagsAndAttributes
			nullptr));                // hTemplateFile

        if (!m_fileHandle.IsValid())
        {
			hr = HRESULT_FROM_WIN32(GetLastError());
			wprintf(L"Unable to open file %s, hr = 0x%x\n", fileName, hr);
			return hr;
		}

        LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(m_fileHandle.Get(), &fileSize))
        {
			hr = HRESULT_FROM_WIN32(GetLastError());
			wprintf(L"GetFileSizeEx failed with 0x%x\n", hr);
            return hr;
		}
        m_contentSize = (ULONGLONG)fileSize.QuadPart;

        return S_OK;
	}

	// IAmsiStream

	STDMETHOD(GetAttribute)(
        _In_ AMSI_ATTRIBUTE attribute,
		_In_ ULONG bufferSize,
		_Out_writes_bytes_to_(bufferSize, *actualSize) PBYTE buffer,
		_Out_ ULONG* actualSize)
	{
        return BaseGetAttribute(attribute, bufferSize, buffer, actualSize);
	}

    STDMETHOD(Read)(
	    _In_ ULONGLONG position,
		_In_ ULONG size,
		_Out_writes_bytes_to_(size, *readSize) PBYTE buffer,
		_Out_ ULONG* readSize)
    {
		wprintf(L"Read() called with: position = %I64u, size = %u\n", position, size);

        OVERLAPPED o = {};
        o.Offset = LODWORD(position);
        o.OffsetHigh = HIDWORD(position);

		if (!ReadFile(m_fileHandle.Get(), buffer, size, readSize, &o))
		{
			HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
			wprintf(L"ReadFile failed with 0x%x\n", hr);
			return hr;
		}

		return S_OK;
	}

private:
    FileHandle m_fileHandle;
};

const char SampleStream[] = "Hello, world";

class CAmsiMemoryStream : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAmsiStream>, CAmsiStreamBase
{
public:
    HRESULT RuntimeClassInitialize()
    {
        m_contentSize = sizeof(SampleStream);
        return SetContentName(L"Sample content.txt");
    }

    // IAmsiStream

    STDMETHOD(GetAttribute)(
        _In_ AMSI_ATTRIBUTE attribute,
        _In_ ULONG bufferSize,
        _Out_writes_bytes_to_(bufferSize, *actualSize) PBYTE buffer,
        _Out_ ULONG* actualSize)
    {
        HRESULT hr = BaseGetAttribute(attribute, bufferSize, buffer, actualSize);
        if (hr == E_NOTIMPL)
        {
            switch (attribute)
            {
            case AMSI_ATTRIBUTE_CONTENT_ADDRESS:
                const void* contentAddress = SampleStream;
                hr = CopyAttribute(&contentAddress, sizeof(contentAddress), bufferSize, buffer, actualSize);
            }
        }
        return hr;
    }

    STDMETHOD(Read)(
        _In_ ULONGLONG position,
        _In_ ULONG size,
        _Out_writes_bytes_to_(size, *readSize) PBYTE buffer,
        _Out_ ULONG* readSize)
    {
        wprintf(L"Read() called with: position = %I64u, size = %u\n", position, size);

        *readSize = 0;
        if (position >= m_contentSize)
        {
            wprintf(L"Reading beyond end of stream\n");
            return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        }

        if (size > m_contentSize - position) {
            size = static_cast<ULONG>(m_contentSize - position);
        }

        *readSize = size;
        memcpy_s(buffer, size, SampleStream + position, size);
        return S_OK;
    }
};

class CStreamScanner
{
public:
    HRESULT Initialize()
    {
        return CoCreateInstance(
            __uuidof(CAntimalware),
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_antimalware));
    }

    HRESULT ScanStream(_In_ IAmsiStream* stream)
    {
        wprintf(L"Calling antimalware->Scan() ...\n");
        ComPtr<IAntimalwareProvider> provider;
        AMSI_RESULT r;
        HRESULT hr = m_antimalware->Scan(stream, &r, &provider);
        if (FAILED(hr)) {
            return hr;
        }

        wprintf(L"Scan result is %u. IsMalware: %d\n", r, AmsiResultIsMalware(r));

        if (provider) {
            PWSTR name;
            hr = provider->DisplayName(&name);
            if (SUCCEEDED(hr)) {
                wprintf(L"Provider display name: %s\n", name);
                CoTaskMemFree(name);
            }
            else
            {
                wprintf(L"DisplayName failed with 0x%x", hr);
            }
        }

        return S_OK;
    }

private:
    ComPtr<IAntimalware> m_antimalware;
};

HRESULT ScanArguments(_In_ int argc, _In_reads_(argc) wchar_t** argv)
{
    CStreamScanner scanner;
    HRESULT hr = scanner.Initialize();
    if (FAILED(hr))
    {
        return hr;
    }

    if (argc < 2)
    {
        // Scan a single memory stream.
        wprintf(L"Creating memory stream object\n");

        ComPtr<IAmsiStream> stream;
        hr = MakeAndInitialize<CAmsiMemoryStream>(&stream);
        if (FAILED(hr)) {
            return hr;
        }

        hr = scanner.ScanStream(stream.Get());
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        // Scan the files passed on the command line.
        for (int i = 1; i < argc; i++)
        {
            LPWSTR fileName = argv[i];

            wprintf(L"Creating stream object with file name: %s\n", fileName);
            ComPtr<IAmsiStream> stream;
            hr = MakeAndInitialize<CAmsiFileStream>(&stream, fileName);
            if (FAILED(hr)) {
                return hr;
            }

            hr = scanner.ScanStream(stream.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }

    return S_OK;
}

int __cdecl wmain(_In_ int argc, _In_reads_(argc) WCHAR **argv)
{

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)) {
        hr = ScanArguments(argc, argv);
        CoUninitialize();
	}

	wprintf(L"Leaving with hr = 0x%x\n", hr);

    return 0;
}
