// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once

class ResourceFontContext
{
public:
    ResourceFontContext();
    ~ResourceFontContext();

    HRESULT Initialize();

    HRESULT CreateFontCollection(
        UINT const* fontCollectionKey,  // [keySize] in bytes
        UINT32 keySize,
        OUT IDWriteFontCollection** result
        );

private:
    // Not copyable or assignable.
    ResourceFontContext(ResourceFontContext const&);
    void operator=(ResourceFontContext const&);

    HRESULT InitializeInternal();

    // Error code from Initialize().
    HRESULT hr_;
};
