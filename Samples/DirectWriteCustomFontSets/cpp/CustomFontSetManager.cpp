//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


#include "stdafx.h"
#include "BinaryResources.h" // Used in CreateFontSetUsingInMemoryFontData() for loading font data embedded in the app binary.
#include "CustomFontSetManager.h"
#include "Document.h" // Used in CreateFontSetUsingInMemoryFontData() to simulate a document with embedded font data.
#include "FileHelper.h"
#include "FontDownloadListener.h" // Used in GetFontDataDetails() when there are remote fonts (scenario 3)
#include "PackedFontFileLoader.h" // Used in CreateFontSetUsingPackedFontData() for a custom font file loader that handles packed font container formats.


using Microsoft::WRL::ComPtr;


namespace DWriteCustomFontSets
{

    //**********************************************************************
    //
    //   Constructors, destructors
    //
    //**********************************************************************

    CustomFontSetManager::CustomFontSetManager()
    {
        HRESULT hr;

        // IDWriteFactory3 supports APIs available in any Windows 10 version (build 10240 or later).
        DX::ThrowIfFailed(
            DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), &m_dwriteFactory3)
        );

#ifndef FORCE_TH1_IMPLEMENTATION
        // IDWriteFactory5 supports APIs available in Windows 10 Creators Update (preview build 15021 or later).
        hr = m_dwriteFactory3.As(&m_dwriteFactory5);
        if (hr == E_NOINTERFACE)
        {
            // Let this go. Later, if we might use the interface, we'll branch gracefully.
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
#endif // !FORCE_TH1_IMPLEMENTATION

    } // end CustomFontSetManager::CustomFontSetManager()


    CustomFontSetManager::~CustomFontSetManager()
    {
        // Some scenarios register loaders. These need to be unregistered before exiting.

        // This will be relevant in scenario 3, when CreateFontSetUsingKnownRemoteFonts() is called.
        UnregisterFontFileLoader(m_remoteFontFileLoader.Get());

        // This will be relevant in scenario 4, when CreateFontSetUsingInMemoryFontData() is called.
        UnregisterFontFileLoader(m_inMemoryFontFileLoader.Get());

        // This will be relevant in scenario 5, when CreateFontSetUsingPackedFontData() is called.
        UnregisterFontFileLoader(m_packedFontFileLoader.Get());

    } // end CustomFontSetManager::~CustomFontSetManager()




      //**********************************************************************
      //
      //   Method for checking API availability.
      //
      //**********************************************************************

    bool CustomFontSetManager::IDWriteFactory5_IsAvailable()
    {
        return m_dwriteFactory5 != nullptr;
    }




    //**********************************************************************
    //
    //   Methods for the creating a font set under the various scenarios.
    //
    //**********************************************************************

    void CustomFontSetManager::CreateFontSetUsingLocalFontFiles(const std::vector<std::wstring>& selectedFilePathNames)
    {
        // Requires any version of Windows 10.

        // Creates a custom font set for font files at paths in local storage. If a file is
        // an OpenType collection file, which contains multiple fonts, all of the fonts will
        // be added to the collection.
        //
        // If running on Windows 10 Creators Update (preview build 15021 or later), the 
        // IDWriteFontSetBuilder1::AddFontFile method will be used. This method handles all of
        // the fonts in an OpenType collection file in a single call, and it also supports
        // OpenType variable fonts, which can be realized as many different font faces -- all
        // named instances in the variable font will be added in a single call. This method is
        // recommended when available.
        //
        // If running on earlier Windows 10 versions, the method used will be 
        // IDWriteFontSetBuilder::AddFontFaceReference. This does not support OpenType variable
        // fonts, and also requires that a font file first be analyzed to determine whether it
        // is an OpenType collection file, in which case each font must be handled in a separate
        // call.
        //
        // If one of the input path names is not a font file, it will be ignored.
        //
        // When creating a custom font set with font files that are not assumed to be known by
        // the app, DWrite will need to extract some basic font properties, such as names,
        // directly from the font files. This will result in a little extra I/O overhead.

        // Check if IDWriteFontSetBuilder1 will be available (we're running on preview build 15021 or later)
        if (m_dwriteFactory5 != nullptr)
        {
            // We'll need an IDWriteFontFile for each font file to be added to the font set.
            // We won't assume every file is a font file in a supported format; if not, we'll
            // ignore the file.

            // Get the font set builder -- IDWriteFontSetBuilder1.
            ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
            DX::ThrowIfFailed(
                m_dwriteFactory5->CreateFontSetBuilder(&fontSetBuilder)
            );

            // Loop over the file paths.
            for (auto& filePath : selectedFilePathNames)
            {
                ComPtr<IDWriteFontFile> fontFile;
                DX::ThrowIfFailed(
                    m_dwriteFactory5->CreateFontFileReference(filePath.c_str(), /* filetime */ nullptr, &fontFile)
                );

                // Add to the font set builder. If the file is a collection, all of the fonts will
                // get added. If the file is not a supported font file, the call will fail; we'll
                // check for that and ignore.
                HRESULT hr = fontSetBuilder->AddFontFile(fontFile.Get());
                if ((hr != DWRITE_E_FILEFORMAT) && (hr != DWRITE_E_FILENOTFOUND) && (hr != DWRITE_E_FILEACCESS))
                {
                    // Ignore file format or access errors.
                    DX::ThrowIfFailed(hr);
                }
            } // for loop

            // Now create the custom font set.
            DX::ThrowIfFailed(
                fontSetBuilder->CreateFontSet(&m_customFontSet)
            );
        }
        else
        {
            // We're limited to APIs and functionality available on earlier Windows 10 versions, prior
            // to the Windows 10 Creators Update (preview build 15021).
            //
            // Also, we'll need an IDWriteFontFaceReference for each font to be added to the font set.
            // If a file is an OpenType collection, it may contain multiple fonts. For that reason, 
            // we'll need to analyze the file to get the count of fonts, and then provide an index 
            // when creating each font face reference.

            // Get the font set builder - IDWriteFontSetBuilder.
            ComPtr<IDWriteFontSetBuilder> fontSetBuilder;
            DX::ThrowIfFailed(
                m_dwriteFactory3->CreateFontSetBuilder(&fontSetBuilder)
            );

            // Loop over the file paths.
            for (auto& filePath : selectedFilePathNames)
            {
                ComPtr<IDWriteFontFile> fontFile;
                DX::ThrowIfFailed(
                    m_dwriteFactory3->CreateFontFileReference(filePath.c_str(), /* filetime */ nullptr, &fontFile)
                );

                // Confirm the file is a supported font file and get the collection face count.
                BOOL isSupported;
                DWRITE_FONT_FILE_TYPE fileType;
                UINT32 numberOfFonts;
                DX::ThrowIfFailed(
                    fontFile->Analyze(&isSupported, &fileType, /* face type */ nullptr, &numberOfFonts)
                );
                if (!isSupported)
                    continue;

                // For each font within the font file, get a font face reference and add to the builder.
                for (UINT32 fontIndex = 0; fontIndex < numberOfFonts; fontIndex++)
                {
                    ComPtr<IDWriteFontFaceReference> fontFaceReference;
                    DX::ThrowIfFailed(
                        m_dwriteFactory3->CreateFontFaceReference(fontFile.Get(), fontIndex, DWRITE_FONT_SIMULATIONS_NONE, &fontFaceReference)
                    );

                    // If fonts were assumed known, we could set custom properties, and would do that here.
                    // But these are not assumed known, so we'll leave it to DirectWrite to read properties
                    // directly out of the font files.

                    DX::ThrowIfFailed(
                        fontSetBuilder->AddFontFaceReference(fontFaceReference.Get())
                    );
                } // for loop -- over fonts with font file
            } // for loop -- over font files

            // Now create the custom font set
            DX::ThrowIfFailed(
                fontSetBuilder->CreateFontSet(&m_customFontSet)
            );

        } // end if (IDWriteFactory5_IsAvailable())
    } // end CustomFontSetManager::CreateFontSetUsingLocalFontFiles()



    void CustomFontSetManager::CreateFontSetUsingKnownAppFonts()
    {
        // Requires any version of Windows 10.

        // Creates a custom font set using fonts known by and bundled with the app. Since the fonts
        // are known, we can apply custom font properties when the fonts are added to the font set,
        // which will be the properties used within the app to reference the fonts. This saves a
        // bit of file I/O, and makes it easier to change the fonts used in the app since details
        // can be changed in one place.
        //
        // The details for the set of app-provided fonts are defined in the g_appFonts[] array
        // within Statics.cpp.
        //
        // If a file is an OpenType collection file, which contains multiple fonts, we can specify
        // which of the fonts within the file is to be used.
        //
        // As of Windows 10 Creators Update, OpenType variable fonts are not supported in this scenario --
        // specifically, there is no way to add a specific variation instance with custom properties
        // to a custom font set.

        // Get the application install path.
        std::wstring applicationPath;
        if (!FileHelper::GetApplicationPath(applicationPath))
        {
            OutputDebugString(L"\nThere was an unexpected error attempting to get the app install path, so the custom font set with app fonts cannot be created.\n\n");
            return;
        }

        // Get the font set builder.
        ComPtr<IDWriteFontSetBuilder> fontSetBuilder;
        DX::ThrowIfFailed(
            m_dwriteFactory3->CreateFontSetBuilder(&fontSetBuilder)
        );

        // Add the known app fonts to the font set.
        for (uint32_t fontIndex = 0; fontIndex < g_appFontsCount; fontIndex++)
        {
            AppFontInfo const& fontInfo = g_appFonts[fontIndex];
            std::wstring fontFilePath(applicationPath + fontInfo.fontRelativeLocation);

            // Check that the font got deployed with the app. (If not, CreateFontFaceReference would fail.)
            if (!FileHelper::PathExists(fontFilePath))
            {
                std::wstring debugString(L"\nApp font file is missing: " + fontFilePath + L"\n\n");
                OutputDebugString(debugString.c_str());
                continue;
            }

            // Create a font face reference for the specific font (requires file plus collection index).
            ComPtr<IDWriteFontFaceReference> fontFaceReference;
            DX::ThrowIfFailed(
                m_dwriteFactory3->CreateFontFaceReference(fontFilePath.c_str(), /* filetime*/ nullptr, fontInfo.fontIndex, DWRITE_FONT_SIMULATIONS_NONE, &fontFaceReference)
            );

            // Set up custom font properties for app-internal use.
            DWRITE_FONT_PROPERTY props[] =
            {
                // We're only using names to reference fonts programmatically, so won't worry about localized names.
                { DWRITE_FONT_PROPERTY_ID_FAMILY_NAME, fontInfo.familyName, L"en-US" },
                { DWRITE_FONT_PROPERTY_ID_FULL_NAME, fontInfo.fullName, L"en-US" },
                { DWRITE_FONT_PROPERTY_ID_WEIGHT, fontInfo.fontWeight, nullptr }
            };

            // Now add the font to the font set with the custom properties.
            DX::ThrowIfFailed(
                fontSetBuilder->AddFontFaceReference(fontFaceReference.Get(), props, ARRAYSIZE(props))
            );

        } // end for loop

          // Now create the custom font set.
        DX::ThrowIfFailed(
            fontSetBuilder->CreateFontSet(&m_customFontSet)
        );

        return;
    } // CustomFontSetManager::CreateFontSetUsingKnownAppFonts()



    void CustomFontSetManager::CreateFontSetUsingKnownRemoteFonts()
    {
        // Requires Windows 10 Creators Update (preview build 15021 or later).

        // Creates a font set using fonts known by the app, but remote -- located on the Web.
        // Custom font properties will be applied, allowing the font set to be created without
        // needing to download any of the font data.
        //
        // The details for the set of app-specified fonts are defined in the g_remoteFfonts[]
        //  array within Statics.cpp.
        //
        // This uses a system-provided implementation of IDWriteRemoteFontFileLoader. For each
        // remote font file, we create an IDWriteFontFile, and from that create an
        // IDWriteFontFaceReference, and then add that into the font set with pre-defined
        // properties. The font set will be created without needing to download any font data
        // beforehand.
        //
        // Note: In CreateFontSetUsingLocalFontFiles(), we pass the IDWriteFontFile objects into
        // IDWriteFontSetBuilder1::AddFontFile() to add all of the fonts in a collection in one
        // call. If we try to do that with a remote font, the AddFontFile call will fail,
        // returning DWRITE_E_REMOTEFONT. The sequence shown here, using AddFontFaceReference()
        // with custom properties, is required when creating a font set with remote fonts.
        //
        // Before using the IDWriteRemoteFontFileLoader, it must be registered with a DirectWrite
        // factory object. The loader will be needed for as long as the fonts may be used within
        // the app, and so it will be stored as a CustomFontSetManager member. It must be
        // unregistered before it goes out of scope; that will be done in the CustomFontSetManager
        // destructor.

        // Get and register the system-implemented remote font file loader.
        DX::ThrowIfFailed(
            m_dwriteFactory5->CreateHttpFontFileLoader(
                /* referrerURL */ nullptr,
                /* extraHeaders */ nullptr,
                &m_remoteFontFileLoader
            )
        );
        DX::ThrowIfFailed(
            m_dwriteFactory5->RegisterFontFileLoader(m_remoteFontFileLoader.Get())
        );

        // Get a font set builder.
        ComPtr<IDWriteFontSetBuilder> fontSetBuilder;
        DX::ThrowIfFailed(
            m_dwriteFactory5->CreateFontSetBuilder(&fontSetBuilder)
        );

        // Add the remote fonts to the font set.
        for (uint32_t fontIndex = 0; fontIndex < g_remoteFontsCount; fontIndex++)
        {
            AppFontInfo const& fontInfo = g_remoteFonts[fontIndex];

            // Get an IDWriteFontFile.
            ComPtr<IDWriteFontFile> fontFile;
            DX::ThrowIfFailed(
                m_remoteFontFileLoader->CreateFontFileReferenceFromUrl(
                    m_dwriteFactory5.Get(),
                    g_remoteFontBaseUrl,
                    fontInfo.fontRelativeLocation, // Can point to a raw font file (.ttf, .ttc, .otf, .otc), or to a WOFF or WOFF2 packed-format file.
                    &fontFile
                )
            );

            // Get an IDWriteFontFaceReference for a font within the file.
            ComPtr<IDWriteFontFaceReference> fontFaceReference;
            DX::ThrowIfFailed(
                m_dwriteFactory5->CreateFontFaceReference(fontFile.Get(), fontInfo.fontIndex, DWRITE_FONT_SIMULATIONS_NONE, &fontFaceReference)
            );

            // Set up custom font properties for app-internal use.
            DWRITE_FONT_PROPERTY props[] =
            {
                // We're only using names to reference fonts programmatically, so won't worry about localized names.
                { DWRITE_FONT_PROPERTY_ID_FAMILY_NAME, fontInfo.familyName, L"en-US" },
                { DWRITE_FONT_PROPERTY_ID_FULL_NAME, fontInfo.fullName, L"en-US" },
                { DWRITE_FONT_PROPERTY_ID_WEIGHT, fontInfo.fontWeight, nullptr }
            };

            // Now add the font to the font set with the custom properties.
            DX::ThrowIfFailed(
                fontSetBuilder->AddFontFaceReference(fontFaceReference.Get(), props, ARRAYSIZE(props))
            );

        } // end for loop

          // Now create the custom font set.
        DX::ThrowIfFailed(
            fontSetBuilder->CreateFontSet(&m_customFontSet)
        );

        return;
    } // CustomFontSetManager::CreateFontSetUsingKnownRemoteFonts()



    void CustomFontSetManager::CreateFontSetUsingInMemoryFontData()
    {
        // Requires Windows 10 Creators Update (preview build 15021 or later).

        // Creates a custom font set using in-memory font data.
        //
        // The implementation will use in-memory font data from two sources:
        //
        //   - a font embedded within the app binary as a resource; and
        //   - a document with embedded font data.
        //
        // These are two common app scenarios, but the implementation can be adapted
        // to other scenarios in which font data is loaded into memory.
        //
        // The BinaryResources class handles loading of the font embedded in the app
        // as a binary resource.
        //
        // The Document class simulates a document with embedded font data. In a real
        // scenario, the document would be read from a stream. As a simplification,
        // this simulation uses static data.
        //
        // The data in the memory buffer is expected to be raw, OpenType font data,
        // not data in a compressed, packed format such as WOFF2. For support of
        // packed-format font data, see scenario 5.

        // This will use a system implementation of IDWriteInMemoryFontFileLoader.
        // Before a font file loader can be used, it must be registered with a
        // DirectWrite factory object. The loader will be needed for as long as the
        // fonts may be used within the app, and so it will be stored as a
        // CustomFontSetManager member. It must be unregistered before it goes out of
        // scope; that will be done in the CustomFontSetManager destructor.


        // Get and register the system-implemented in-memory font file loader.
        DX::ThrowIfFailed(
            m_dwriteFactory5->CreateInMemoryFontFileLoader(&m_inMemoryFontFileLoader)
        );
        DX::ThrowIfFailed(
            m_dwriteFactory5->RegisterFontFileLoader(m_inMemoryFontFileLoader.Get())
        );

        // Get a font set builder. We're already dependent on Windows 10 Creators Update, 
        // so will use IDWriteFontSetBuilder1, which will save work later (won't need to 
        // check for an OpenType collection and loop over the individual fonts in the
        // collection).
        ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
        DX::ThrowIfFailed(
            m_dwriteFactory5->CreateFontSetBuilder(&fontSetBuilder)
        );


        // Load fonts embedded in the app binary as resources into memory.
        ComPtr<BinaryResources> binaryResources = new BinaryResources();
        std::vector<MemoryFontInfo> appFontResources;
        binaryResources->GetFonts(appFontResources);

        // Add the in-memory fonts to the font set.
        for (uint32_t fontIndex = 0; fontIndex < appFontResources.size(); fontIndex++)
        {
            MemoryFontInfo fontInfo = appFontResources[fontIndex];

            // For each in-memory font, get an IDWriteFontFile using the in-memory font
            // file loader. Then use that to get an IDWriteFontFaceReference, and add
            // the font face reference to the font set.

            ComPtr<IDWriteFontFile> fontFileReference;
            DX::ThrowIfFailed(
                m_inMemoryFontFileLoader->CreateInMemoryFontFileReference(
                    m_dwriteFactory5.Get(),
                    fontInfo.fontData,
                    fontInfo.fontDataSize,
                    binaryResources.Get(), // Passing the binaryResources object as the data owner -- data lifetime is managed by the owner, so DirectWrite won't make a copy.
                    &fontFileReference
                )
            );

            // The data may represent an OpenType collection file, which would include multiple
            // fonts. By using IDWriteFontSetBuilder1::AddFontFile, all of the fonts in a 
            // collection and all of the named instances in variable fonts are added in a single
            // call.

            // We're assuming here that the in-memory data is font data in a supported format.
            // Otherwise, we should check for the AddFontFile call failing with error 
            // DWRITE_E_FILEFORMAT.

            // Since the fonts are embedded in an app binary, they are known in advance, and so
            // custom font properties could be used. In that case, the custom properties would 
            // be specified here, and AddFontFaceReference would be used instead of AddFontFile.
            // See CreateFontSetUsingKnownAppFonts in this file for how that would be done.

            DX::ThrowIfFailed(
                fontSetBuilder->AddFontFile(fontFileReference.Get())
            );
        }

        // Get our simulated document that has embedded font data, and get the document
        // text and a vector of embedded font data.
        ComPtr<Documents::Document> document = new Documents::Document();
        std::wstring text = document->GetText();
        std::vector<MemoryFontInfo> documentFonts;
        document->GetFonts(documentFonts);

        // Add the in-memory fonts to the font set.
        for (uint32_t fontIndex = 0; fontIndex < documentFonts.size(); fontIndex++)
        {
            MemoryFontInfo fontInfo = documentFonts[fontIndex];

            ComPtr<IDWriteFontFile> fontFileReference;
            DX::ThrowIfFailed(
                m_inMemoryFontFileLoader->CreateInMemoryFontFileReference(
                    m_dwriteFactory5.Get(),
                    fontInfo.fontData,
                    fontInfo.fontDataSize,
                    document.Get(), // Passing the document object as the data owner -- data lifetime is managed by the owner, so DirectWrite won't make a copy.
                    &fontFileReference
                )
            );

            DX::ThrowIfFailed(
                fontSetBuilder->AddFontFile(fontFileReference.Get())
            );

        } // end for -- loop over fonts

          // Now create the custom font set.
        DX::ThrowIfFailed(
            fontSetBuilder->CreateFontSet(&m_customFontSet)
        );

        return;
    } // CustomFontSetManager::CreateFontSetUsingInMemoryFontData()



    void CustomFontSetManager::CreateFontSetUsingPackedFontData()
    {
        // Requires Windows 10 Creators Update (preview build 15021 or later).

        // Creates a font set using data in packed, WOFF2 format to demonstrate DirectWrite APIs
        // for unpacking font data in packed WOFF or WOFF2 formats.
        //
        // The font data for this scenario is static data defined in Statics.cpp.
        //
        // This uses a custom implementation of IDWriteFontFileLoader that utilizes the APIs for
        // unpacking the packed font data. The font file loader can also handle font data not in
        // a packed format.
        //
        // An IDWriteFontFileLoader implementation needs to provide access to the data via a
        // callback to an IDWriteFontFileStream object. In the case of packed font data, the
        // method for unpacking returns an IDWriteFontFileStream, making this case simple to
        // handle. If the font data is not contained in a packed format, then a custom
        // implementation of IDWriteFontFileStream would need to be used.

        // Before a font file loader can be used, it must be registered with a
        // DirectWrite factory object. The loader will be needed for as long as the
        // fonts may be used within the app, and so it will be stored as a
        // CustomFontSetManager member. It must be unregistered before it goes out of
        // scope; that will be done in the CustomFontSetManager destructor.

        // Get and register the custom-implementation of IDWriteFontFileLoader that we'll
        // use to handle unpacking of packed font data.
        m_packedFontFileLoader = new PackedFontFileLoader(m_dwriteFactory5.Get());
        DX::ThrowIfFailed(
            m_dwriteFactory5->RegisterFontFileLoader(m_packedFontFileLoader.Get())
        );

        // Get a font set builder. We're already dependent on Windows 10 Creators Update,
        // so will use IDWriteFontSetBuilder1, which will save work later (won't need to
        // check for an OpenType collection and loop over the individual fonts in the
        // collection).
        ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
        DX::ThrowIfFailed(
            m_dwriteFactory5->CreateFontSetBuilder(&fontSetBuilder)
        );

        // For each font to be added to the font set, we need to create an IDWriteFontFile
        // object that carries the custom font file loader that handles the font data.
        for (uint32_t fontIndex = 0; fontIndex < g_packedFontsCount; fontIndex++)
        {
            // For each font, we get an IDWriteFontFile using CreateCustomFontFileReference.
            // This takes a key, which is used by our custom loader implementation as a
            // private ID for each of the fonts managed by the loader. The only requirement
            // on the keys is that they are unique in the context of the loader. The set of
            // fonts managed by the loader are in the g_packedFonts array; we'll use an 
            // index into the array as a key.

            ComPtr<IDWriteFontFile> fontFileReference;
            DX::ThrowIfFailed(
                m_dwriteFactory5->CreateCustomFontFileReference(
                    &fontIndex, // fontFileReferenceKey
                    sizeof(fontIndex), // fontFileReferenceKeySize
                    m_packedFontFileLoader.Get(),
                    &fontFileReference
                )
            );

            // We're assuming here that the font data is a known and supported container format,
            // or is not in a container. And if not in a container, then we assume that it is a
            // supported font format. Otherwise, we could use IDWriteFactory5::AnalyzeContainerType
            // on the raw data to check the container format; and IDWriteFontFile::Analyze to 
            // verify the font is a supported format.

            // In the case of WOFF2 or non-packed font data, the file could be an OpenType 
            // collection, or a variable font with multiple named instances. (The WOFF format
            // does not support OpenType collections, however.) In this case, the file would
            // include multiple font faces. By using IDWriteFontSetBuilder1::AddFontFile, all of
            // the font faces are added in a single call.

            DX::ThrowIfFailed(
                fontSetBuilder->AddFontFile(fontFileReference.Get())
            );

        } // end for -- loop over fonts

          // Now create the custom font set.
        DX::ThrowIfFailed(
            fontSetBuilder->CreateFontSet(&m_customFontSet)
        );

        return;
    } // CustomFontSetManager::CreateFontSetUsingPackedFontData()




    //***********************************************************
    //
    //   Other public methods
    //
    //***********************************************************

    uint32_t CustomFontSetManager::GetFontCount() const
    {
        if (m_customFontSet == nullptr)
            return 0;

        return m_customFontSet->GetFontCount();
    }


    std::vector<std::wstring> CustomFontSetManager::GetFullFontNames() const
    {
        // Call GetPropertyValuesFromFontSet to get an IDWriteStringList with en-US (or default) full
        // names. IDWriteStringList is a dictionary with entries that include a language tag and the
        // property value. We only care about the latter.

        ComPtr<IDWriteStringList> fullNamePropertyValues = GetPropertyValuesFromFontSet(DWRITE_FONT_PROPERTY_ID_FULL_NAME);

        std::vector<std::wstring> fullNames;
        for (UINT32 i = 0; i < fullNamePropertyValues->GetCount(); i++)
        {
            std::wstring propertyValueString;
            UINT32 length;
            DX::ThrowIfFailed(
                fullNamePropertyValues->GetStringLength(i, &length)
            );
            propertyValueString.resize(length);
            DX::ThrowIfFailed(
                fullNamePropertyValues->GetString(i, &propertyValueString[0], length + 1)
            );
            fullNames.push_back(std::move(propertyValueString)); // Use move to avoid a copy.
        }

        return fullNames;
    } // end CustomFontSetManager::GetFullFontNames()


    std::vector<std::wstring> CustomFontSetManager::GetFontDataDetails(HANDLE cancellationHandle) const
    {
        // Report some representative details that require actual font data. If fonts are remote, a
        // download will be required. Since latency or success are uncertain, we'll give some ways to
        // interrupt the operation. The cancellationHandle parameter is for a caller-determined object
        // that we can wait on. We'll also set a 15-second timeout. In either case, we'll return an
        // empty vector.

        std::vector<std::wstring> resultVector;

        // We'll enqueue a download request for data from each font in the font set. If the font is
        // already local, this will be a no-op.
        //
        // Note that, depending on actual app scenarios, direct enqueueing may not be a typical usage
        // pattern. For instance, in apps that display text using IDWriteTextLayout, the layout will
        // automatically enqueue download requests as needed when measuring or drawing actions are
        // done, using fallback fonts in the meantime. After drawing or getting metrics, the app can
        // then check the font download queue to see if its non-empty, and initiate a download if
        // needed.

        for (uint32_t fontIndex = 0; fontIndex < m_customFontSet->GetFontCount(); fontIndex++)
        {
            ComPtr<IDWriteFontFaceReference> fontFaceReference;
            DX::ThrowIfFailed(
                m_customFontSet->GetFontFaceReference(fontIndex, &fontFaceReference)
            );
            DX::ThrowIfFailed(
                fontFaceReference->EnqueueFontDownloadRequest()
            );
        } // end for loop

          // Check the font download queue to see if we have remote fonts to download.
        ComPtr<IDWriteFontDownloadQueue> fontDownloadQueue;
        DX::ThrowIfFailed(
            m_dwriteFactory3->GetFontDownloadQueue(&fontDownloadQueue)
        );
        if (!fontDownloadQueue->IsEmpty())
        {
            // We need a download listener. It will set an event when the download task is 
            // completed; we need to get the event handle and wait on it after initiating
            // the download.
            ComPtr<FontDownloadListener> fontDownloadListener = new FontDownloadListener();
            HANDLE downloadCompletedHandle = fontDownloadListener->GetDownloadCompletedEventHandle();

            // Now begin the download, and then wait on our primary or alternate event.
            DX::ThrowIfFailed(
                fontDownloadQueue->BeginDownload(fontDownloadListener.Get())
            );
            const HANDLE handles[] = { downloadCompletedHandle, cancellationHandle };
            DWORD waitResult = WaitForMultipleObjects(ARRAYSIZE(handles), handles, FALSE, 15000);

            // Check the wait result. If the alternate event fired, the user wants to exit early
            // so just return the empty result vector. If download success, we'll build up results
            // using the font data.
            switch (waitResult)
            {
            case WAIT_OBJECT_0:
                // DownloadCompleted was called on our listener; check for errors.
                if (fontDownloadListener->GetDownloadResult() != S_OK)
                {
                    // Download failed for some reason; return empty result vector.
                    OutputDebugString(L"\nUnexpected error within GetFontDataDetails: downloading of fonts was initiated, but failed.\n\n");
                    return resultVector;
                }
                // Download succeeded; break out of switch.
                break;

                // Remaining cases are all failure/abort, so just return empty result vector.

            case WAIT_OBJECT_0 + 1: // User interrupted flow.
                std::wcout << L"You've aborted the process of getting font details.\n";
                return resultVector;

            case WAIT_TIMEOUT:
                std::wcout << L"Downloading of fonts was initiated, but has timed out.\n";
                return resultVector;

            default: // Shouldn't ever go here.
                return resultVector;

            } // end switch
        } // end if (!fontDownloadQueue->IsEmpty())

        // Get representative data for each font: list the full name to identify the font (this will come directly 
        // from the font data, not from any custom font set properties), and give the font's x-height.
        for (uint32_t fontIndex = 0; fontIndex < m_customFontSet->GetFontCount(); fontIndex++)
        {
            std::wstring fontDetailReport;

            // Get IDWriteFontFace3
            ComPtr<IDWriteFontFaceReference> fontFaceReference;
            DX::ThrowIfFailed(
                m_customFontSet->GetFontFaceReference(fontIndex, &fontFaceReference)
            );
            ComPtr<IDWriteFontFace3> fontFace; // IDWriteFontFace3 or later is needed for the GetInformationalStrings() method.
            DX::ThrowIfFailed(
                fontFaceReference->CreateFontFace(&fontFace)
            );

            // Font detail report string begins with full name to identify the font.
            ComPtr<IDWriteLocalizedStrings> localizedStrings;
            BOOL exists;
            DX::ThrowIfFailed(
                fontFace->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_FULL_NAME, &localizedStrings, &exists)
            );
            if (exists) // should always be the case
            {
                uint32_t stringIndex;
                DX::ThrowIfFailed(
                    localizedStrings->FindLocaleName(L"en-US", &stringIndex, &exists)
                );
                if (!exists)
                {
                    stringIndex = 0;
                }
                uint32_t stringLength;
                DX::ThrowIfFailed(
                    localizedStrings->GetStringLength(stringIndex, &stringLength)
                );
                fontDetailReport.resize(stringLength);
                DX::ThrowIfFailed(
                    localizedStrings->GetString(stringIndex, &fontDetailReport[0], static_cast<UINT32>(fontDetailReport.size() + 1))
                );
                fontDetailReport.append(L": ");
            }
            else // In case we didn't get the full name, just give the font set index.
            {
                fontDetailReport.assign(L"Font ");
                fontDetailReport.append(std::to_wstring(fontIndex));
                fontDetailReport.append(L": ");
            }

            // Add to the font detail report the font's x-height.
            DWRITE_FONT_METRICS1 fontMetrics;
            fontFace->GetMetrics(&fontMetrics);
            fontDetailReport.append(L"x-height = ");
            fontDetailReport.append(std::to_wstring(fontMetrics.xHeight));

            // Add font detail report string to the result vector.
            resultVector.push_back(fontDetailReport);

        } // end for loop

        return resultVector;
    } // end CustomFontSetManager::GetFontDataDetails()


    bool CustomFontSetManager::CustomFontSetHasRemoteFonts() const
    {
        // Used to determine if there are any fonts in the font set for which data is currently
        // remote, thus requiring a download before it can be used. If all the data is already
        // local (was always local or has already been downloaded), then this will return FALSE.

        for (uint32_t fontIndex = 0; fontIndex < m_customFontSet->GetFontCount(); fontIndex++)
        {
            ComPtr<IDWriteFontFaceReference> fontFaceReference;
            DX::ThrowIfFailed(
                m_customFontSet->GetFontFaceReference(fontIndex, &fontFaceReference)
            );
            if (fontFaceReference->GetLocality() != DWRITE_LOCALITY_LOCAL)
                return true;
        }
        return false;
    } // end CustomFontSetHasRemoteFonts



    //***********************************************************
    //
    //   Private helper methods
    //
    //***********************************************************

    void CustomFontSetManager::UnregisterFontFileLoader(IDWriteFontFileLoader* fontFileLoader)
    {
        if (fontFileLoader != nullptr)
        {
            // Will be call from destructor. Ignore any errors.
            m_dwriteFactory3->UnregisterFontFileLoader(fontFileLoader);
        }
    }


    ComPtr<IDWriteStringList> CustomFontSetManager::GetPropertyValuesFromFontSet(DWRITE_FONT_PROPERTY_ID propertyId) const
    {
        // We can iterate over the font faces within the font set, but IDWriteFontSet has a convenient
        // GetPropertyValues method that allows us to get a list of information-string property values
        // from all of the fonts in the font set in one call. 
        //
        // A font can have multiple localized variants for a given informational string. If we call 
        // using the overload that doesn't include a parameter for language preference, the returned 
        // list will include all localized variants from all fonts. But if we indicate a language
        // preference, the list will include only the best language match from a font, with en-US as 
        // a fallback.
        //
        // The list includes unique values from across the set, so in general isn't guaranteed to have
        // as many values as there are fonts. Certain properties -- full name or Postscript name --
        // are likely to be unique to each font, however.

        ComPtr<IDWriteStringList> propertyValues;
        DX::ThrowIfFailed(
            m_customFontSet->GetPropertyValues(propertyId, L"en-US", &propertyValues)
        );

        return propertyValues;

    } // end CustomFontSetManager::GetPropertyValuesFromFontSet()


} // namespace DWriteCustomFontSets