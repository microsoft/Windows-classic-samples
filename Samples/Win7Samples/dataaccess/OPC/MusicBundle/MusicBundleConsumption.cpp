//<SnippetMusicBundle_cppConsumptionWholePage>
/*****************************************************************************
*
* File: MusicBundleConsumption.cpp
*
* Description:
* This sample is a simple application that might be used as a starting-point
* for an application that uses the Packaging API. This sample demonstrates
* the production and consumption of a music bundle package, which is a custom
* package file-format.
*
* The music bundle format is an example of a custom package file format and
* was designed specifically for this sample. See the MusicBundle.h header file
* for the music bundle specification.
*
* For the sake of simplicity, production data used for the sample is hard
* coded. In a fully-functional application, production data would be retrieved
* from the user of the application.
*
* ------------------------------------
*
*  This file is part of the Microsoft Windows SDK Code Samples.
* 
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>

#include <msopc.h>  // For Packaging APIs

#include <strsafe.h>

#include "util.h"
#include "MusicBundle.h"

//-------------------------------------
// Consumption helper methods.

///////////////////////////////////////////////////////////////////////////////
// Description:
// Reads a part from the Music Bundle, displays content to the console (if
// required), and writes the content to a file in the output directory.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ReadPartFromBundle(
    IOpcFactory  *opcFactory,
    IOpcRelationshipSet  *relationshipSet,// Track relationship set.
    IOpcPartSet  *packagePartSet,         // Set of all parts in the package.
    LPCWSTR  relationshipType,            // Relationship type of relationship targeting the part.
    LPCWSTR  contentType,                 // Content type of the part.
    LPCWSTR  outputDirectory,             // Part content is serialized as the content of a file in
                                          // this directory.
    LPCWSTR  displayTitle = NULL,         // Title to display with file contents. Optional; set to
                                          // NULL if displaying content is not required.
    IOpcPart **partRead = NULL            // The part to be read. Optional; caller must release the
                                          // object.
    )
{
    HRESULT hr = S_OK;
    IOpcPart * part = NULL;
    IOpcRelationship * relationship = NULL;
    IStream * stream = NULL;

    // Get relationships of the specified type. 
    hr = GetRelationshipByType(
            relationshipSet,
            relationshipType,
            &relationship
            );

    if (SUCCEEDED(hr))
    {
        // Get the part targetted by the relationship.
        hr = GetRelationshipTargetPart(
                packagePartSet,
                relationship,
                contentType,
                &part
                );
    }
    
    if (SUCCEEDED(hr) && displayTitle)
    {
        // Get part content stream.
        hr = part->GetContentStream(&stream);
    
        
        if (SUCCEEDED(hr))
        {
            // Display the content to the console.
            hr = DisplayStreamContent(displayTitle, stream);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Write the part content to a file in the output directory.
        hr = WritePartContentToFile(opcFactory, outputDirectory, part);
    }

    if (SUCCEEDED(hr) && partRead != NULL)
    {
        // Return a point to the part that was read and written.
        *partRead = part;
        part = NULL;
    }

    // Release resources
    if (part)
    {
        part->Release();
        part = NULL;
    }

    if (relationship)
    {
        relationship->Release();
        relationship = NULL;
    }

    if (stream)
    {
        stream->Release();
        stream = NULL;
    }
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Method does the following:
// 1. Reads a specified Track part in the Music Bundle and writes the part to
//    a file in the output directory. 
// 2. Reads the Lyrics part for the Track part.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ReadTrackAndLyricsFromBundle(
    IOpcFactory  *opcFactory,
    IOpcPartSet  *packagePartSet,// Set of all parts in the package.
    IOpcPart  *trackPart,        // Track part to read.
    LPCWSTR  outputDirectory     // Write part content to a file in this directory.
    
    )
{
    HRESULT hr = S_OK;
    IOpcRelationshipSet * trackRelationshipSet = NULL;

    // Write part content to a file in the output directory.
    hr = WritePartContentToFile(opcFactory, outputDirectory, trackPart);

    // Get the relationship set for the Track part.
    if (SUCCEEDED(hr))
    {
        hr = trackPart->GetRelationshipSet(&trackRelationshipSet);
    }

    // Get Lyrics for track.
    if (SUCCEEDED(hr))
    {
        hr = ReadPartFromBundle(
                opcFactory,
                trackRelationshipSet,
                packagePartSet,
                g_lyricsRelationshipType,
                g_lyricsContentType,
                outputDirectory,
                L"Lyrics"
                );
    }

    // Release resources
    if (trackRelationshipSet)
    {
        trackRelationshipSet->Release();
        trackRelationshipSet = NULL;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Enumerates and reads all the Track parts in the Track List
// relationship set by the relationship type
///////////////////////////////////////////////////////////////////////////////
HRESULT
EnumerateTracksFromBundle(
    IOpcFactory  *opcFactory,
    IOpcRelationshipSet  *trackListRelationshipSet,// Set of all relationships whose source
                                                   // is the Track List part.
    IOpcPartSet *packagePartSet,                   // Set of all parts in the package.
    LPCWSTR outputDirectory                        // Write part content to a file in this directory.
    )
{
    HRESULT hr = S_OK;
    BOOL bNext = FALSE;
    IOpcRelationshipEnumerator * relationshipEnumerator = NULL;
    
    // Get enumerator of relationships in the set that are the track
    // relationship type.
    hr = trackListRelationshipSet->GetEnumeratorForType(
            g_trackRelationshipType,
            &relationshipEnumerator
            );

    // For each relationship in the enumerator, get the targetted Track part,
    // read the part and read the Lyrics part linked to the current Track part.
    if (SUCCEEDED(hr))
    {
        while (SUCCEEDED(hr = relationshipEnumerator->MoveNext(&bNext)) && bNext)
        {
            IOpcRelationship * trackRelationship = NULL;
            IOpcPart * trackPart = NULL;
            
            // Get current enumerator relationship.
            hr = relationshipEnumerator->GetCurrent(&trackRelationship);
        
            // Get the Track part targetted by the relationship.
            if (SUCCEEDED(hr))
            {
                hr = GetRelationshipTargetPart(
                        packagePartSet,
                        trackRelationship,
                        g_trackContentType,
                        &trackPart
                        );
            }
            
            if (SUCCEEDED(hr))
            {
                // Read contents of the Track part and of the Lyrics part that
                // is linked to the current Track part.
                hr = ReadTrackAndLyricsFromBundle(
                        opcFactory,
                        packagePartSet,
                        trackPart,
                        outputDirectory
                        );
            }

            // Release resources
            if (trackRelationship)
            {
                trackRelationship->Release();
                trackRelationship = NULL;
            }

            if (trackPart)
            {
                trackPart->Release();
                trackPart = NULL;
            }
        }
    }

    // Release resources
    if (relationshipEnumerator)
    {
        relationshipEnumerator->Release();
        relationshipEnumerator = NULL;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Method does the following:
// 1. Reads the Track List in the Music Bundle, displays it to
//    the console.
// 2. Writes the part to a file in the output directory. 
// 3. Enumerates the Track List relationships and reads all the
//    Tracks in the Music Bundle.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ReadTrackListFromBundle(
    IOpcFactory  *opcFactory,
    IOpcRelationshipSet  *packageRelationshipSet,// Package relationship set.
    IOpcPartSet  *packagePartSet,                // Set of all parts in the package.
    LPCWSTR  outputDirectory                     // Write part content to a file in this directory.
    )
{
    HRESULT hr = S_OK;
    IOpcPart * trackListPart = NULL;
    IOpcRelationshipSet * trackListRelationshipSet = NULL;
    
    // Find the Track List part by using the track list relationship type.
    hr = ReadPartFromBundle(
            opcFactory,
            packageRelationshipSet,
            packagePartSet,
            g_trackListRelationshipType,
            g_trackListContentType,
            outputDirectory,
            L"TrackList",
            &trackListPart
            );

    if (SUCCEEDED(hr))
    {
        // Get the set of relationships whose source is the Track List part.
        hr = trackListPart->GetRelationshipSet(&trackListRelationshipSet);
    }

    
    if (SUCCEEDED(hr))
    {
        // Enumerate tracks in the bundle.
        hr = EnumerateTracksFromBundle(
                opcFactory,
                trackListRelationshipSet,
                packagePartSet,
                outputDirectory
                );
    }

    // Release resources
    if (trackListPart)
    {
        trackListPart->Release();
        trackListPart = NULL;
    }

    if (trackListRelationshipSet)
    {
        trackListRelationshipSet->Release();
        trackListRelationshipSet = NULL;
    }
        
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Reads the Album Art in the Music Bundle and writes the part
// to a file in the output directory
///////////////////////////////////////////////////////////////////////////////
HRESULT
ReadAlbumArtFromBundle(
    IOpcFactory  *opcFactory,
    IOpcRelationshipSet  *packageRelationshipSet,// Package relationship set.
    IOpcPartSet  *packagePartSet,                // Set of all parts in the package.
    LPCWSTR  outputDirectory                     // Write part content to a file in this directory.
    )
{
    HRESULT hr = S_OK;
    
    // Find the Album Art part by using the album art relationship type.
    hr = ReadPartFromBundle(
            opcFactory,
            packageRelationshipSet,
            packagePartSet,
            g_albumArtRelationshipType,
            g_albumArtContentType,
            outputDirectory
            );

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Reads the Album Website part displays its content in the console.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ReadAlbumWebsiteFromBundle(
    IOpcRelationshipSet  *packageRelationshipSet // Package relationship set.
    )
{
    HRESULT hr = S_OK;
    IOpcRelationship * opcRelationship = NULL;
    OPC_URI_TARGET_MODE targetMode = OPC_URI_TARGET_MODE_INTERNAL;
    IUri * targetUri = NULL;
    BSTR targetUriString = NULL;

    // Find the Album Website part by using the album website relationship type.
    hr = GetRelationshipByType(
            packageRelationshipSet,
            g_albumWebsiteRelationshipType,
            &opcRelationship
            );

    if (SUCCEEDED(hr))
    {
        // Get the target mode of the relationship; teh mode must be 'External'
        // for the relationship to target an external absolute URL.
        hr = opcRelationship->GetTargetMode(&targetMode);
    }

    if (SUCCEEDED(hr))
    {
        if (targetMode != OPC_URI_TARGET_MODE_EXTERNAL)
        {
            // The target mode was 'Internal'.
            fwprintf(
                stderr,
                L"Invalid music bundle package: relationship with type %s must have External target mode.\n",
                g_albumWebsiteRelationshipType
                );

            // Set the return code to an error.
            hr = E_FAIL;
        }
    }

    
    if (SUCCEEDED(hr))
    {
        // Get the target URI, which is the album website URL.
        hr = opcRelationship->GetTargetUri(&targetUri);
    }

    if (SUCCEEDED(hr))
    {
        // Get the album website URL as a string.
        hr = targetUri->GetAbsoluteUri(&targetUriString);
    }

    if (SUCCEEDED(hr))
    {
        // Display the album website URL.
        wprintf(L"\n++++++ Album Website ++++++\n%s\n\n", targetUriString);
    }

    // Release resources
    if (opcRelationship)
    {
        opcRelationship->Release();
        opcRelationship = NULL;
    }

    if (targetUri)
    {
        targetUri->Release();
        targetUri = NULL;
    }

    SysFreeString(targetUriString);

    return hr;
}

//---------------------------------
// Function to consume the new music bundle.
// Exposed through MusicBundle.h.
//
///////////////////////////////////////////////////////////////////////////////
// Description:
// Deserializes the contents of a music bundle package, displays text from
// the Track List, Lyrics, Album Website parts. The method then serializes the
// contents of the parts to files in specified output directory.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ConsumeMusicBundle(
    LPCWSTR  inputPackageName,// Name of music bundle.
    LPCWSTR  outputDirectory  // Directory into which music bundle parts are written as files.
    )
{
    HRESULT hr = S_OK;
    IOpcFactory * opcFactory = NULL;
    IStream * packageStream = NULL;
    IOpcPackage * opcPackage = NULL;
    IOpcRelationshipSet * packageRelationshipSet = NULL;
    IOpcPartSet * packagePartSet = NULL;

    // Create a new factory.
    hr = CoCreateInstance(
            __uuidof(OpcFactory),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IOpcFactory),
            (LPVOID*)&opcFactory
            );

    if (SUCCEEDED(hr))
    {
        // Open a read-only stream over the input package.
        hr = opcFactory->CreateStreamOnFile(
                inputPackageName,
                OPC_STREAM_IO_READ, // Read-only.
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                &packageStream
                );
    }

    if (SUCCEEDED(hr))
    {
        // Create a package object to represent the pacakge being read, allowing 
        // package components to be accessed through Packaging API objects.
        hr = opcFactory->ReadPackageFromStream(
                packageStream,
                OPC_READ_DEFAULT, // Validate package component when it is accessed.
                &opcPackage
                );
    }

    if (SUCCEEDED(hr))
    {
        // Get relationship set of package relationships.
        hr = opcPackage->GetRelationshipSet(&packageRelationshipSet);
    }

    if (SUCCEEDED(hr))
    {
        // Get the set of parts in the package that are not Relationships parts.
        hr = opcPackage->GetPartSet(&packagePartSet);
    }

    // Read, display and unpack the music bundle.

    if (SUCCEEDED(hr))
    {
        // Read and display album art.
        hr = ReadAlbumArtFromBundle(
                opcFactory,
                packageRelationshipSet,
                packagePartSet,
                outputDirectory
                );
    }
    if (SUCCEEDED(hr))
    {
        // Read and display album website.
        hr = ReadAlbumWebsiteFromBundle(packageRelationshipSet);
    }
    if (SUCCEEDED(hr))
    {
        // Read and unpack as files in the output directory: the track list,
        // tracks and lyrics. Display the track list and corresponding lyrics.
        hr = ReadTrackListFromBundle(
                opcFactory,
                packageRelationshipSet,
                packagePartSet,
                outputDirectory
                );
    }

    // Release resources
    if (opcFactory)
    {
        opcFactory->Release();
        opcFactory = NULL;
    }

    if (packageStream)
    {
        packageStream->Release();
        packageStream = NULL;
    }

    if (opcPackage)
    {
        opcPackage->Release();
        opcPackage = NULL;
    }

    if (packageRelationshipSet)
    {
        packageRelationshipSet->Release();
        packageRelationshipSet = NULL;
    }

    if (packagePartSet)
    {
        packagePartSet->Release();
        packagePartSet = NULL;
    }

    return hr;
}
//</SnippetMusicBundle_cppConsumptionWholePage>