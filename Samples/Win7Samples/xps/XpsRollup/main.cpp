//+----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      A sample demonstrating use of the XPS Object Model to combine XPS
//      documents.
//-----------------------------------------------------------------------------

// Compile with /EHsc
#include <stdio.h>
#define NOMINMAX
#include <windows.h>
#include <comdef.h>
#include <XpsObjectModel.h>
#include <set>
#include <limits>

// This struct is used by std::set to compare PartUri instances.  It calls ComparePartUri() to
// compare by value, not by pointer.
struct part_uri_less
{
    bool operator()(
        IOpcPartUri* uri1,
        IOpcPartUri* uri2
        ) const
    {
        HRESULT hr;
        int result;
        if (FAILED(hr = uri1->ComparePartUri(uri2, &result)))
        {
            // Since this struct is used by an STL class, we throw an exception here.  It will
            // be caught in one of the PageReferenceAdder methods.
            throw std::runtime_error("ERROR: ComparePartUri failed");
        }

        return result < 0;
    }
};

// This struct is used by std::set to compare PartResource instances.  In this case we are
// unconcerned with the value of the resource - we only want to know whether the pointers we have
// are to the same COM object.  _com_ptr defines an operator< which performs a proper comparison by
// performing a QueryInterface() to IUnknown and comparing the returned pointer values.
struct part_less
{
    bool operator()(
        IXpsOMResource* part1,
        IXpsOMResource* part2
        ) const
    {
        return part1 < static_cast<IXpsOMResource*>(part2);
    }
};

// This class does the heavy lifting.  Its constructor takes an XpsOMObjectFactory (for creating
// PartUris) and a PackageWriter.  We then call AddPageReference(), which calls
// CollectPartResources() to check for part name conflicts, renames pages and resources as
// necessary, and adds the page to the PackageWriter.
class PageReferenceAdder
{
public:
    PageReferenceAdder(
        IXpsOMObjectFactory* xpsFactory,
        IXpsOMPackageWriter* packageWriter) :
        m_xpsFactory(xpsFactory),
        m_packageWriter(packageWriter),
        m_pageCounter(1),
        m_partNames(NULL),
        m_resources(NULL)
    {
        // We want to avoid throwing from the constructor so that clients
        // of this class do not need to be exception-safe.
        try
        {
            m_partNames = new PartNameCollection_t;
            m_resources = new ResourceCollection_t;
        }
        catch (std::bad_alloc)
        {
            // We could get bad_alloc two ways - either the allocation
            // for the collection itself could throw, or the constructor
            // of the collection could throw when it attempts to allocate
            // a sentinel node.  In either case we do nothing - we check
            // whether the pointer is NULL in AddPageReference.
        }
    }

    ~PageReferenceAdder()
    {
        if (m_partNames)
        {
            delete m_partNames;
        }

        if (m_resources)
        {
            delete m_resources;
        }
    }

    HRESULT
    AddPageReference(
        IXpsOMPageReference* pageReference
        )
    {
        HRESULT hr = S_OK;
        IXpsOMPartResources* partResources = NULL;
        IXpsOMPage* page = NULL;
        IXpsOMStoryFragmentsResource* storyFragments = NULL;
        IXpsOMPrintTicketResource* printTicket = NULL;
        IXpsOMImageResource* thumbnail = NULL;
        XPS_SIZE size;

        // If we failed to initialize, report this to the client now.
        if (!m_partNames || !m_resources)
        {
            return E_OUTOFMEMORY;
        }

        if (FAILED(hr = pageReference->CollectPartResources(&partResources)))
        {
            fwprintf(stderr, L"ERROR: Could not collect part resources: 0x%X\n", hr);
        }

        // Now we call AddResourceCollection, a templated function, for each of the resource
        // collections.  This function will loop through the collection and call RenameResource,
        // passing the pointer to the resource as well as a prefix string.  The prefix string allows
        // us to create part names in accordance with the naming recommendations in the XPS spec.
        // RenameResource does the appropriate renaming, then adds the resource to a collection so
        // that we only process it once.

        {
            IXpsOMFontResourceCollection* fonts = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = partResources->GetFontResources(&fonts)))
                {
                    fwprintf(stderr, L"ERROR: Could not get font resource collection: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                // We can't use the FAILED macro inline here due to the template parameter list.
                hr = AddResourceCollection<
                        IXpsOMFontResource*,
                        IXpsOMFontResourceCollection*>(fonts, L"Resources/Fonts/");
                if (FAILED(hr))
                {
                    // Error already reported.
                }
            }

            if (fonts)
            {
                fonts->Release();
                fonts = NULL;
            }
        }

        {
            IXpsOMImageResourceCollection* images = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = partResources->GetImageResources(&images)))
                {
                    fwprintf(stderr, L"ERROR: Could not get image resource collection: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                // We can't use the FAILED macro inline here due to the template parameter list.
                hr = AddResourceCollection<
                        IXpsOMImageResource*,
                        IXpsOMImageResourceCollection*
                        >(images, L"Resources/Images/");
                if (FAILED(hr))
                {
                    // Error already reported.
                }
            }

            if (images)
            {
                images->Release();
                images = NULL;
            }
        }

        {
            IXpsOMColorProfileResourceCollection* colorProfiles = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = partResources->GetColorProfileResources(&colorProfiles)))
                {
                    fwprintf(stderr, L"ERROR: Could not get color profile resource collection: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                // We can't use the FAILED macro inline here due to the template parameter list.
                hr = AddResourceCollection<
                        IXpsOMColorProfileResource*,
                        IXpsOMColorProfileResourceCollection*
                        >(colorProfiles, L"Metadata/");
                if (FAILED(hr))
                {
                    // Error already reported.
                }
            }

            if (colorProfiles)
            {
                colorProfiles->Release();
                colorProfiles = NULL;
            }
        }

        {
            IXpsOMRemoteDictionaryResourceCollection* remoteDictionaries = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = partResources->GetRemoteDictionaryResources(&remoteDictionaries)))
                {
                    fwprintf(stderr, L"ERROR: Could not get remote dictionary resource collection: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                // We can't use the FAILED macro inline here due to the template parameter list.
                hr = AddResourceCollection<
                        IXpsOMRemoteDictionaryResource*,
                        IXpsOMRemoteDictionaryResourceCollection*
                        >(remoteDictionaries, L"Resources/Dictionaries/");
                if (FAILED(hr))
                {
                    // Error already reported.
                }
            }

            if (remoteDictionaries)
            {
                remoteDictionaries->Release();
                remoteDictionaries = NULL;
            }
        }

        if (partResources)
        {
            partResources->Release();
            partResources = NULL;
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = pageReference->GetPage(&page)))
            {
                fwprintf(stderr, L"ERROR: Could not get page: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            // If a PageReference does not have a page attached, GetPage() returns S_OK and a NULL
            // page pointer.  For package-bound PageReferences, this should never be the case, but
            // we make the check to be thorough.
            if (!page)
            {
                fwprintf(stderr, L"ERROR: PageReference has no Page\n");
                hr = E_UNEXPECTED;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = RenamePage(page)))
            {
                // Error already reported.
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = page->GetPageDimensions(&size)))
            {
                fwprintf(stderr, L"ERROR: Could not get page dimensions: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = pageReference->GetStoryFragmentsResource(&storyFragments)))
            {
                fwprintf(stderr, L"ERROR: Could not get story fragments: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = pageReference->GetPrintTicketResource(&printTicket)))
            {
                fwprintf(stderr, L"ERROR: Could not get print ticket: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = pageReference->GetThumbnailResource(&thumbnail)))
            {
                fwprintf(stderr, L"ERROR: Could not get thumbnail: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = m_packageWriter->AddPage(page, &size, NULL, storyFragments, printTicket, thumbnail)))
            {
                fwprintf(stderr, L"ERROR: Could not add page: 0x%X\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Now that we have added the page to the PackageWriter, we can discard it and reclaim
            // the memory used.
            if (FAILED(hr = pageReference->DiscardPage()))
            {
                fwprintf(stderr, L"ERROR: Could not discard page: 0x%X\n", hr);
            }
        }

        if (thumbnail)
        {
            thumbnail->Release();
            thumbnail = NULL;
        }

        if (printTicket)
        {
            printTicket->Release();
            printTicket = NULL;
        }

        if (storyFragments)
        {
            storyFragments->Release();
            storyFragments = NULL;
        }

        if (page)
        {
            page->Release();
            page = NULL;
        }

        if (partResources)
        {
            partResources->Release();
            partResources = NULL;
        }

        return hr;
    }

private:
    HRESULT
    RenamePage(IXpsOMPage* page)
    {
        HRESULT hr = S_OK;

        try
        {
            std::wstring newUri(L"Documents/1/Pages/");

            // maximum value of an int in decimal is 10 characters long, plus the trailing NULL
            wchar_t buffer[11];
            if (_itow_s(m_pageCounter++, buffer, /* radix */ 10) != 0)
            {
                fwprintf(stderr, L"ERROR: _itow_s failed trying to convert %d\n", m_pageCounter - 1);
                return E_UNEXPECTED;
            }

            newUri += buffer;
            newUri += L".fpage";

            IOpcPartUri* partName = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = m_xpsFactory->CreatePartUri(newUri.c_str(), &partName)))
                {
                    fwprintf(stderr, L"ERROR: CreatePartUri failed: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = page->SetPartName(partName)))
                {
                    fwprintf(stderr, L"ERROR: Could not set part name: 0x%X\n", hr);
                }
            }

            if (partName)
            {
                partName->Release();
                partName = NULL;
            }

            wprintf(L"Adding page %s\n", newUri.c_str());
        }
        catch (std::exception &e)
        {
            fwprintf(stderr, L"ERROR: Caught exception in RenamePage: %S\n", e.what());
            return E_FAIL;
        }

        return hr;
    }

    template <typename Resource_t, typename ResourceCollection_t>
    HRESULT
    AddResourceCollection(ResourceCollection_t collection, LPCWSTR prefix)
    {
        HRESULT hr = S_OK;

        UINT32 count = 0;
        if (SUCCEEDED(hr))
        {
            if (FAILED(hr = collection->GetCount(&count)))
            {
                fwprintf(stderr, L"ERROR: Could not get count: 0x%X\n", hr);
            }
        }

        for (UINT32 i = 0; i < count; i++)
        {
            Resource_t resource = NULL;
            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = collection->GetAt(i, &resource)))
                {
                    fwprintf(stderr, L"ERROR: Could not get resource: 0x%X\n", hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = RenameResource(resource, prefix)))
                {
                    // Error already reported.
                }
            }

            if (resource)
            {
                resource->Release();
                resource = NULL;
            }
        }

        return hr;
    }

    HRESULT
    RenameResource(IXpsOMResource* partResource, LPCWSTR prefix)
    {
        HRESULT hr = S_OK;
        IOpcPartUri* partName = NULL;
        BSTR absoluteUri = NULL;

        try
        {
            // Only continue if we have not seen this COM object before.
            if (m_resources->find(partResource) == m_resources->end())
            {
                if (SUCCEEDED(hr))
                {
                    if (FAILED(hr = partResource->GetPartName(&partName)))
                    {
                        fwprintf(stderr, L"ERROR: could not get part name for page: 0x%X\n", hr);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if (FAILED(hr = partName->GetAbsoluteUri(&absoluteUri)))
                    {
                        fwprintf(stderr, L"ERROR: could not get absolute URI: 0x%X\n", hr);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    std::wstring absolute(absoluteUri);

                    size_t slash = absolute.find_last_of(L"/");
                    if (slash == std::wstring::npos)
                    {
                        slash = 0;
                    }

                    int counter = 0;

                    do
                    {
                        if (partName)
                        {
                            partName->Release();
                            partName = NULL;
                        }

                        std::wstring newUri(L"Documents/1/");
                        newUri += prefix;

                        if (counter == std::numeric_limits<int>::max())
                        {
                            fwprintf(stderr, L"ERROR: Too many conflicts on part name %s - giving up\n",
                                absolute.substr(slash).c_str());
                            hr = E_FAIL;
                        }

                        // maximum value of an int in decimal is 10 characters long, plus the trailing NULL
                        wchar_t buffer[11];
                        if (SUCCEEDED(hr))
                        {
                            if (_itow_s(counter++, buffer, /* radix */ 10) != 0)
                            {
                                fwprintf(stderr, L"ERROR: _itow_s failed trying to convert %d\n", counter);
                                hr = E_UNEXPECTED;
                            }
                        }

                        if (SUCCEEDED(hr))
                        {
                            if (counter != 1)
                            {
                                newUri += buffer;
                            }

                            newUri += absolute.substr(slash);

                            if (FAILED(hr = m_xpsFactory->CreatePartUri(newUri.c_str(), &partName)))
                            {
                                fwprintf(stderr, L"ERROR: CreatePartUri failed: 0x%X\n", hr);
                            }
                        }
                    } while (SUCCEEDED(hr) && (m_partNames->find(partName) != m_partNames->end()));
                }

                if (SUCCEEDED(hr))
                {
                    if (FAILED(hr = partResource->SetPartName(partName)))
                    {
                        fwprintf(stderr, L"ERROR: could not set part name for page: 0x%X\n", hr);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    m_partNames->insert(partName);
                    m_resources->insert(partResource);
                }
            }
        }
        catch (std::exception &e)
        {
            fwprintf(stderr, L"ERROR: Caught exception in RenameResource: %S\n", e.what());
            hr = E_FAIL;
        }

        if (absoluteUri)
        {
            SysFreeString(absoluteUri);
            absoluteUri = NULL;
        }

        if (partName)
        {
            partName->Release();
            partName = NULL;
        }

        return hr;
    }

    // Declare these private so the compiler will not generate default implementations.
    // It does not make sense for PageReferenceAdder to be copied or assigned.
    PageReferenceAdder(const PageReferenceAdder&);
    PageReferenceAdder& operator=(const PageReferenceAdder&);

private:
    typedef std::set<IXpsOMResource*, part_less> ResourceCollection_t;
    typedef std::set<IOpcPartUri*, part_uri_less> PartNameCollection_t;

    PartNameCollection_t* m_partNames;
    ResourceCollection_t* m_resources;

    IXpsOMObjectFactory* m_xpsFactory;
    IXpsOMPackageWriter* m_packageWriter;
    int m_pageCounter;
};

void Usage(wchar_t *argv0)
{
    fwprintf(stderr, L"XPS Object Model Document Rollup Sample\n\n");
    fwprintf(stderr, L"\tUsage: %s <output filename> <input filename>...\n", argv0);
}

int
wmain(int argc, wchar_t* argv[])
{
    HRESULT hr = S_OK;
    IXpsOMObjectFactory* xpsFactory = NULL;
    IOpcPartUri* partUri = NULL;
    IXpsOMPackageWriter* packageWriter = NULL;

    if (argc < 3)
    {
        Usage(argv[0]);
        return 1;
    }

    if (FAILED(hr = CoInitializeEx(0, COINIT_MULTITHREADED)))
    {
        fwprintf(stderr, L"ERROR: CoInitializeEx failed with HRESULT 0x%X\n", hr);
        return 1;
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = CoCreateInstance(
                    __uuidof(XpsOMObjectFactory),
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IXpsOMObjectFactory),
                    reinterpret_cast<void**>(&xpsFactory)
                    )
                )
           )
        {
            fwprintf(stderr, L"ERROR: Could not create XPS OM Object Factory: %08X\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(L"/FixedDocumentSequence.fdseq", &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %x\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePackageWriterOnFile(
                    argv[1],
                    NULL,
                    0,
                    TRUE,
                    XPS_INTERLEAVING_OFF,
                    partUri,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &packageWriter
                    )
                )
           )
        {
            fwprintf(stderr, L"ERROR: Could not create package writer: 0x%X\n", hr);
        }
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = xpsFactory->CreatePartUri(L"/Documents/1/FixedDocument.fdoc", &partUri)))
        {
            fwprintf(stderr, L"ERROR: Could not create part URI: %x\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = packageWriter->StartNewDocument(partUri, NULL, NULL, NULL, NULL)))
        {
            fwprintf(stderr, L"ERROR: Could not start new document: 0x%X\n", hr);
        }
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    PageReferenceAdder pageReferenceAdder(xpsFactory, packageWriter);

    for (int file = 2; file < argc; file++)
    {
        if (SUCCEEDED(hr))
        {
            wprintf(L"Opening %s...\n", argv[file]);
            IXpsOMPackage* inputPackage = NULL;
            IXpsOMDocumentSequence* inputDocSeq = NULL;
            IXpsOMDocumentCollection* documents = NULL;
            UINT32 docCount = 0;

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = xpsFactory->CreatePackageFromFile(
                            argv[file],
                            TRUE,
                            &inputPackage
                            )
                        )
                   )
                {
                    fwprintf(stderr, L"ERROR: Could not open %s: %x\n", argv[file], hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = inputPackage->GetDocumentSequence(&inputDocSeq)))
                {
                    fwprintf(stderr, L"ERROR: Could not get document sequence for %s: %x\n", argv[file],
                        hr);
                }
            }

            if (inputPackage)
            {
                inputPackage->Release();
                inputPackage = NULL;
            }

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = inputDocSeq->GetDocuments(&documents)))
                {
                    fwprintf(stderr, L"ERROR: Could not get documents for %s: %x\n", argv[file],
                        hr);
                }
            }

            if (inputDocSeq)
            {
                inputDocSeq->Release();
                inputDocSeq = NULL;
            }

            if (SUCCEEDED(hr))
            {
                if (FAILED(hr = documents->GetCount(&docCount)))
                {
                    fwprintf(stderr, L"ERROR: Could not get document count for %s: %x\n", argv[file],
                        hr);
                }
            }

            if (SUCCEEDED(hr))
            {
                for (UINT32 doc = 0; doc < docCount; doc++)
                {
                    IXpsOMDocument* document = NULL;
                    IXpsOMPageReferenceCollection* pageReferences = NULL;
                    UINT32 pageCount = 0;

                    if (SUCCEEDED(hr))
                    {
                        if (FAILED(hr = documents->GetAt(doc, &document)))
                        {
                            fwprintf(stderr, L"ERROR: Could not get document %d for %s: %x\n", doc, argv[file],
                                hr);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        if (FAILED(hr = document->GetPageReferences(&pageReferences)))
                        {
                            fwprintf(stderr, L"ERROR: Could not get page references for %s: %x\n", argv[file],
                                hr);
                        }
                    }

                    if (document)
                    {
                        document->Release();
                        document = NULL;
                    }

                    if (SUCCEEDED(hr))
                    {
                        if (FAILED(hr = pageReferences->GetCount(&pageCount)))
                        {
                            fwprintf(stderr, L"ERROR: Could not get page count for document %d of %s: %x\n",
                                doc, argv[file], hr);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        for (UINT32 page = 0; page < pageCount; page++)
                        {
                            IXpsOMPageReference* pageReference = NULL;
                            if (SUCCEEDED(hr))
                            {
                                if (FAILED(hr = pageReferences->GetAt(page, &pageReference)))
                                {
                                    fwprintf(stderr, L"ERROR: Could not get page reference %d of document %d of %s: %x\n",
                                        page, doc, argv[file], hr);
                                }
                            }

                            if (SUCCEEDED(hr))
                            {
                                if (FAILED(hr = pageReferenceAdder.AddPageReference(pageReference)))
                                {
                                    // Error reported by PageReferenceAdder
                                }
                            }

                            if (pageReference)
                            {
                                pageReference->Release();
                                pageReference = NULL;
                            }
                        }
                    }

                    if (pageReferences)
                    {
                        pageReferences->Release();
                        pageReferences = NULL;
                    }
                }
            }

            if (documents)
            {
                documents->Release();
                documents = NULL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = packageWriter->Close()))
        {
            fwprintf(stderr, L"ERROR: Could not close package writer: %x\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"Done!\n");
    }

    if (packageWriter)
    {
        packageWriter->Release();
        packageWriter = NULL;
    }

    if (partUri)
    {
        partUri->Release();
        partUri = NULL;
    }

    if (xpsFactory)
    {
        xpsFactory->Release();
        xpsFactory = NULL;
    }

    CoUninitialize();

    return SUCCEEDED(hr) ? 0 : 1;
}
