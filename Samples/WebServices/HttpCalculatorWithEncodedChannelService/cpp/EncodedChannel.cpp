// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "new"

static const WS_STRING encodedContentType = WS_STRING_VALUE(L"myContentType");

struct Encoder
{
    BYTE* buffer;
    ULONG bufferLength;
    WCHAR contentType[256];
    ULONG contentTypeLength;
    WS_WRITE_CALLBACK writeCallback;
    void* writeContext;
};

HRESULT CALLBACK CreateEncoder(
    _In_ void* createContext,
    _In_ WS_WRITE_CALLBACK writeCallback,
    _In_ void* writeContext,
    _Outptr_ void** encoderContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(createContext);
    UNREFERENCED_PARAMETER(error);

    Encoder* encoder = new(std::nothrow) Encoder;
    if (encoder == NULL)
    {
        return E_OUTOFMEMORY;
    }
    encoder->bufferLength = 1024;
    encoder->buffer = new(std::nothrow) BYTE[encoder->bufferLength];
    if (encoder->buffer == NULL)
    {
        delete encoder;
        return E_OUTOFMEMORY;
    }
    encoder->writeCallback = writeCallback;
    encoder->writeContext = writeContext;
    (*encoderContext) = encoder;
    return S_OK;
}

HRESULT CALLBACK EncoderGetContentType(
    _In_ void* encoderContext,
    _In_ const WS_STRING* contentType,
    _Out_ WS_STRING* newContentType,
    _Out_ WS_STRING* contentEncoding,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(error);

    Encoder* encoder = (Encoder*)encoderContext;
    if (contentType->length > WsCountOf(encoder->contentType))
    {
        return E_FAIL;
    }
    ::memcpy(encoder->contentType, contentType->chars, contentType->length * sizeof(WCHAR));
    encoder->contentTypeLength = contentType->length;
    (*newContentType) = encodedContentType;
    contentEncoding->length = 0;
    contentEncoding->chars = NULL;
    return S_OK;
}

HRESULT CALLBACK EncoderStart(
    _In_ void* encoderContext,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    Encoder* encoder = (Encoder*)encoderContext;
    WS_BYTES bytes1 = { sizeof(encoder->contentTypeLength), (BYTE*)&encoder->contentTypeLength };
    hr = encoder->writeCallback(encoder->writeContext, &bytes1, 1, NULL, error);
    if (FAILED(hr))
    {
        return hr;
    }
    WS_BYTES bytes2 = { encoder->contentTypeLength * sizeof(WCHAR), (BYTE*)encoder->contentType };
    hr = encoder->writeCallback(encoder->writeContext, &bytes2, 1, NULL, error);
    if (FAILED(hr))
    {
        return hr;
    }
    return S_OK;
}

HRESULT EncoderEncode(
    _In_ Encoder* encoder,
    _In_reads_(bufferLength) const BYTE* buffer,
    _In_ ULONG bufferLength,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    while (bufferLength > encoder->bufferLength)
    {
        memcpy(encoder->buffer, buffer, encoder->bufferLength);
        for (ULONG i = 0; i < encoder->bufferLength; i++)
        {
            encoder->buffer[i]++;
        }
        WS_BYTES newBytes = { encoder->bufferLength, encoder->buffer };
        hr = encoder->writeCallback(encoder->writeContext, &newBytes, 1, NULL, error);
        if (FAILED(hr))
        {
            return hr;
        }
        buffer += encoder->bufferLength;
        bufferLength -= encoder->bufferLength;
    }
    memcpy(encoder->buffer, buffer, bufferLength);
    for (ULONG i = 0; i < bufferLength; i++)
    {
        encoder->buffer[i]++;
    }
    WS_BYTES newBytes = { bufferLength, encoder->buffer };
    return encoder->writeCallback(encoder->writeContext, &newBytes, 1, NULL, error);
}

HRESULT CALLBACK EncoderEncode(
    _In_ void* encoderContext,
    _In_reads_(count) const WS_BYTES* buffers,
    _In_ ULONG count,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    Encoder* encoder = (Encoder*)encoderContext;
    for (ULONG i = 0; i < count; i++)
    {
        hr = EncoderEncode(encoder, buffers[i].bytes, buffers[i].length, NULL, error);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    return S_OK;
}

HRESULT CALLBACK EncoderEnd(
    _In_ void* encoderContext,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(encoderContext);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);
    return S_OK;
}

void CALLBACK FreeEncoder(
    _In_ void* encoderContext)
{
    Encoder* encoder = (Encoder*)encoderContext;
    delete encoder;
}

WS_CHANNEL_ENCODER channelEncoder =
{
    NULL,
    CreateEncoder,
    EncoderGetContentType,
    EncoderStart,
    EncoderEncode,
    EncoderEnd,
    FreeEncoder
};

struct Decoder
{
    WCHAR contentType[256];
    ULONG contentTypeLength;
    WS_READ_CALLBACK readCallback;
    void* readContext;
};

HRESULT ReadBlock(
    _In_ Decoder* decoder,
    _Out_writes_bytes_to_(maxLength, *outLength) void* buffer,
    _In_ ULONG maxLength,
    _Out_opt_ ULONG* outLength,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    ULONG length = 0;
    while (length < maxLength)
    {
        ULONG actualLength;
        hr = decoder->readCallback(decoder->readContext, &((BYTE*)buffer)[length], maxLength - length, &actualLength, NULL, error);
        if (FAILED(hr))
        {
            return hr;
        }
        if (actualLength == 0)
        {
            break;
        }
        length += actualLength;
    }
    if (outLength == NULL)
    {
        if (length != maxLength)
        {
            return E_FAIL;
        }
    }
    else
    {
        (*outLength) = length;
    }
    return S_OK;
}

HRESULT CALLBACK CreateDecoder(
    _In_ void* createContext,
    _In_ WS_READ_CALLBACK readCallback,
    _In_ void* readContext,
    _Outptr_ void** decoderContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(createContext);
    UNREFERENCED_PARAMETER(error);

    Decoder* decoder = new(std::nothrow) Decoder;
    if (decoder == NULL)
    {
        return E_OUTOFMEMORY;
    }
    decoder->readCallback = readCallback;
    decoder->readContext = readContext;
    (*decoderContext) = decoder;
    return S_OK;
}

HRESULT CALLBACK DecoderGetContentType(
    _In_ void* decoderContext,
    _In_ const WS_STRING* contentType,
    _In_opt_ const WS_STRING* contentEncoding,
    _Out_ WS_STRING* newContentType,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(error);

    Decoder* decoder = (Decoder*)decoderContext;
    if (contentEncoding != NULL)
    {
        return E_FAIL;
    }
    if (contentType->length != encodedContentType.length)
    {
        return E_FAIL;
    }
    if (::memcmp(contentType->chars, encodedContentType.chars, encodedContentType.length * sizeof(WCHAR)) != 0)
    {
        return E_FAIL;
    }
    newContentType->chars = decoder->contentType;
    newContentType->length = decoder->contentTypeLength;
    return S_OK;
}

HRESULT CALLBACK DecoderStart(
    _In_ void* decoderContext,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    Decoder* decoder = (Decoder*)decoderContext;

    hr = ReadBlock(decoder, &decoder->contentTypeLength, sizeof(ULONG), NULL, NULL, error);
    if (FAILED(hr))
    {
        return hr;
    }
    if (decoder->contentTypeLength > WsCountOf(decoder->contentType))
    {
        return E_FAIL;
    }
    hr = ReadBlock(decoder, &decoder->contentType, decoder->contentTypeLength * sizeof(WCHAR), NULL, NULL, error);
    if (FAILED(hr))
    {
        return hr;
    }
    return S_OK;
}

HRESULT CALLBACK DecoderDecode(
    _In_ void* decoderContext,
    _Out_writes_bytes_to_(maxLength, *outLength) void* buffer,
    _In_ ULONG maxLength,
    _Out_ ULONG* outLength,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    HRESULT hr;
    Decoder* decoder = (Decoder*)decoderContext;
    ULONG length;
    hr = ReadBlock(decoder, buffer, maxLength, &length, NULL, error);
    if (FAILED(hr))
    {
        return hr;
    }
    for (ULONG i = 0; i < length; i++)
    {
        ((BYTE*)buffer)[i]--;
    }
    (*outLength) = length;
    return hr;
}

HRESULT CALLBACK DecoderEnd(
    _In_ void* decoderContext,
    _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
    _In_opt_ WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(decoderContext);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);
    return S_OK;
}

void CALLBACK FreeDecoder(
    _In_ void* decoderContext)
{
    Decoder* decoder = (Decoder*)decoderContext;
    delete decoder;
}

WS_CHANNEL_DECODER channelDecoder =
{
    NULL,
    CreateDecoder,
    DecoderGetContentType,
    DecoderStart,
    DecoderDecode,
    DecoderEnd,
    FreeDecoder
};

