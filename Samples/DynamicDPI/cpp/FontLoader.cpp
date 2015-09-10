//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "FontLoader.h"
#include "FontFileStream.h"
#include "DirectXHelper.h"                   // For ThrowIfFailed

using namespace Microsoft::WRL;

FontLoader::FontLoader(
    _In_ std::wstring location,
    _In_ IDWriteFactory* dwriteFactory
    ) :
    m_refCount(),
    m_location(location),
    m_fontFileCount(),
    m_fontFileStreams(),
    m_fontFileStreamIndex(),
    m_currentFontFile(),
    m_dwriteFactory(dwriteFactory)
{
}

HRESULT FontLoader::Load()
{
    HRESULT hr = S_OK;
    // Prepend \\?\ to allow the path to be longer than MAX_PATH.
    std::wstring fontPath = L"\\\\?\\";
    fontPath.append(m_location);

    PCWSTR uri = fontPath.c_str();

    auto attr = GetFileAttributes(uri);
    if (0xFFFFFFFF != attr)
    {
        // Directory exists.
        fontPath.append(L"\\*.ttf");

        WIN32_FIND_DATAW fileNameData;
        HANDLE findHandle = FindFirstFileExW(fontPath.c_str(), FindExInfoBasic, &fileNameData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

        if (findHandle == INVALID_HANDLE_VALUE)
        {
            hr = GetLastError();
        }
        else
        {
            do
            {
                // Open file and read data
                auto fileHandle = CreateFileW(fileNameData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (fileHandle == INVALID_HANDLE_VALUE)
                {
                    hr = GetLastError();
                    break;
                }

                FILE_STANDARD_INFO fileInfo = { 0 };
                if (!GetFileInformationByHandleEx(
                    fileHandle,
                    FileStandardInfo,
                    &fileInfo,
                    sizeof(fileInfo)
                    ))
                {
                    hr = GetLastError();
                    CloseHandle(fileHandle);
                    // No error checking on CloseHandle, since we are already in an error condition.
                    break;
                }

                if (fileInfo.EndOfFile.HighPart != 0)
                {
                    hr = E_OUTOFMEMORY;
                    CloseHandle(fileHandle);
                    // No error checking on CloseHandle, since we are already in an error condition.
                    break;
                }

                auto fileData = new std::vector<BYTE>(fileInfo.EndOfFile.LowPart);

                DWORD actualRead;
                if (!ReadFile(fileHandle, fileData->data(), fileInfo.EndOfFile.LowPart, &actualRead, NULL))
                {
                    hr = GetLastError();
                    CloseHandle(fileHandle);
                    // No error checking on CloseHandle, since we are already in an error condition.
                    break;
                }
                if (!CloseHandle(fileHandle))
                {
                    hr = GetLastError();
                    break;
                }

                ComPtr<FontFileStream> fontFileStream(new FontFileStream(fileData));
                m_fontFileStreams.push_back(fontFileStream);
                m_fontFileCount++;
            } while (FindNextFileW(findHandle, &fileNameData));
            FindClose(findHandle);
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE FontLoader::QueryInterface(
    REFIID uuid,
    _Outptr_ void** object
    )
{
    if (    uuid == IID_IUnknown
        ||  uuid == __uuidof(IDWriteFontCollectionLoader)
        ||  uuid == __uuidof(IDWriteFontFileEnumerator)
        ||  uuid == __uuidof(IDWriteFontFileLoader)
        )
    {
        *object = this;
        AddRef();
        return S_OK;
    }
    else
    {
        *object = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE FontLoader::AddRef()
{
    return static_cast<ULONG>(InterlockedIncrement(&m_refCount));
}

ULONG STDMETHODCALLTYPE FontLoader::Release()
{
    ULONG newCount = static_cast<ULONG>(InterlockedDecrement(&m_refCount));

    if (newCount == 0)
        delete this;

    return newCount;
}

//  Called by DirectWrite to create an enumerator for the fonts in the font collection.
//  The font collection key being passed in is the same key the application passes to
//  DirectWrite when calling CreateCustomFontCollection API.
//
HRESULT STDMETHODCALLTYPE FontLoader::CreateEnumeratorFromKey(
    _In_ IDWriteFactory* factory,
    _In_reads_bytes_(fontCollectionKeySize) void const* fontCollectionKey,
    UINT32 fontCollectionKeySize,
    _Outptr_ IDWriteFontFileEnumerator** fontFileEnumerator
    )
{
    UNREFERENCED_PARAMETER(factory);
    UNREFERENCED_PARAMETER(fontCollectionKey);
    UNREFERENCED_PARAMETER(fontCollectionKeySize);
    *fontFileEnumerator = ComPtr<IDWriteFontFileEnumerator>(this).Detach();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FontLoader::MoveNext(OUT BOOL* hasCurrentFile)
{
    *hasCurrentFile = FALSE;

    if (m_fontFileStreamIndex < m_fontFileCount)
    {
        DX::ThrowIfFailed(
            m_dwriteFactory->CreateCustomFontFileReference(
                &m_fontFileStreamIndex,
                sizeof(size_t),
                this,
                &m_currentFontFile
                )
            );

        *hasCurrentFile = TRUE;
        ++m_fontFileStreamIndex;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE FontLoader::GetCurrentFontFile(OUT IDWriteFontFile** currentFontFile)
{
    *currentFontFile = ComPtr<IDWriteFontFile>(m_currentFontFile.Get()).Detach();
    return S_OK;
}

//  Called by DirectWrite to create a font file stream. The font file reference
//  key being passed in is the same key the application passes to DirectWrite
//  when calling CreateCustomFontFileReference.
//
HRESULT STDMETHODCALLTYPE FontLoader::CreateStreamFromKey(
    _In_reads_bytes_(fontFileReferenceKeySize) void const* fontFileReferenceKey,
    UINT32 fontFileReferenceKeySize,
    _Outptr_ IDWriteFontFileStream** fontFileStream
    )
{
    if (fontFileReferenceKeySize != sizeof(size_t))
    {
        return E_INVALIDARG;
    }

    size_t fontFileStreamIndex = *(static_cast<size_t const*>(fontFileReferenceKey));

    *fontFileStream = ComPtr<IDWriteFontFileStream>(m_fontFileStreams.at(fontFileStreamIndex).Get()).Detach();

    return S_OK;
}
