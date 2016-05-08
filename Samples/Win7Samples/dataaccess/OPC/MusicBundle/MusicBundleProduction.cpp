//<SnippetMusicBundle_cppProductionWholePage>
/*****************************************************************************
*
* File: MusicBundleProduction.cpp
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
#include "urlmon.h"

//---------------------------------------
// For the sake of simplicity, the production data that appears below is hard
// coded. In a fully-functional application, this data would be retrieved from
// the user of the application.

// Track names.
static LPCWSTR g_trackNames[] = {
    L"\\Tracks\\CrystalFree.wma",
    L"\\Tracks\\Sire.wma",
    L"\\Tracks\\SmallPines.wma",
    L"\\Tracks\\Valparaiso.wma"
};

// Lyric file names. One-to-one mapping with track names.
static LPCWSTR g_lyricsNames[] = {
    L"\\Lyrics\\CrystalFree.txt",
    L"\\Lyrics\\Sire.txt",
    L"\\Lyrics\\SmallPines.txt",
    L"\\Lyrics\\Valparaiso.txt"
};

// Album art.
static const WCHAR g_albumArt[] = L"\\AlbumArt\\jacqui.jpg";

// Track list.
static const WCHAR g_trackList[] = L"\\TrackList.wpl";

// Link to external website for the album.
static const WCHAR g_albumWebsite[] = L"http://www.example.com/Media/Albums/Jacqui%20Kramer/Leap%20Forward";

//-------------------------------------
// Production helper methods.

///////////////////////////////////////////////////////////////////////////////
// Description:
// Creates an empty part, adds the part to the package's part set and writes
// data to the part as part content.
//
// Note: This method does not add Relationships parts to the music bundle.
///////////////////////////////////////////////////////////////////////////////
HRESULT
AddPartToBundle(
    IOpcFactory  *opcFactory,
    LPCWSTR  contentFileName,     // File that contains the content to be stored in the part to add.
    IOpcPartSet  *packagePartSet, // Represents the set of parts (excluding Relationships parts)
                                  // in a package.
    LPCWSTR  contentType,         // The content type of the content to be stored in the part.
    OPC_COMPRESSION_OPTIONS  compressionOptions, // Level of compression to use on the part.
    LPCWSTR  inputDirectory,      // Directory location of the file specified in contentFileName.
    IOpcPartUri  **createdPartUri,// Represents the part name. The caller must release the interface.
    IOpcPart  **createdPart = NULL// Optional. Represents the part to add to the package. The caller must
                                  // release the interface.
    )
{
    HRESULT hr = S_OK;
    IOpcPart * part = NULL;

    if (createdPartUri == NULL)
    {
        // The createdPartUri parameter is required.
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        // Create the part name from the file name.
        hr = opcFactory->CreatePartUri(contentFileName, createdPartUri);
    }

    if (SUCCEEDED(hr))
    {
        // Create the part as an empty part, and add it to the set of parts
        // in the package.
        hr = packagePartSet->CreatePart(
                *createdPartUri,
                contentType,
                compressionOptions,
                &part
                );
    }

    if (SUCCEEDED(hr))
    {
        // Add content to the empty part.
        hr = WriteFileContentToPart(
                opcFactory,
                inputDirectory,
                contentFileName,
                part
                );
    }
    
    if (SUCCEEDED(hr) && createdPart != NULL)
    {
        // The createdPart out parameter was specified.
        // Return the interface pointer to the part created and added to the
        // set of parts in the package.
        *createdPart = part;
        part = NULL;
    }

    // Release resources
    if (part)
    {
        part->Release();
        part = NULL;
    }
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Adds a Track part and a Lyrics part to the set.
// 1. Track part:
//     + Adds a Track part to the music bundle
//     + Creates a relationship where the source is the Track List part and the
//       target is the Tark part by creating the relationship from the Track
//       List part's relationship set.
// 2. Lyrics part:
//     + Adds a Lryics part to the music bundle.
//     + Creates a relationship where the source is the Track part that was 
//       added to the music bundle and the target is the Tark part by creating
//       the relationship from the Track part's relationship set.
///////////////////////////////////////////////////////////////////////////////
HRESULT
AddTrackAndLyricsToBundle(
    IOpcFactory  *opcFactory,
    IOpcPartSet  *packagePartSet, // Represents the set of parts (excluding Relationships parts) in
                                  // a package.
    IOpcRelationshipSet  *trackListRelationshipSet, // Represents the Relationships part containing
                                                    // relationships that have the Track List part
                                                    // as their target.
    LPCWSTR inputDirectory,       // Directory location of the files specified in trackName and 
                                  // lyricsName.
    LPCWSTR trackName,            // Name of file that contains the track data.
    LPCWSTR lyricsName            // Name of tile that contains lyrics data.
    
    )
{
    HRESULT hr = S_OK;
    IOpcPartUri * trackPartUri = NULL;
    IOpcPartUri * lyricsPartUri = NULL;
    IOpcPart * trackPart = NULL;
    IOpcRelationshipSet * trackRelationshipSet = NULL;

    // Add a Track part that contains track data to the package.
    hr = AddPartToBundle(
            opcFactory,
            trackName,
            packagePartSet,
            g_trackContentType,
            OPC_COMPRESSION_NONE,
            inputDirectory,
            &trackPartUri,
            &trackPart
            );

    if (SUCCEEDED(hr))
    {
        // Add a relationship that has the Track part as the target to the
        // relationship set that represents the Relationships part containing
        // relationships that have the Track List part as their source.
        hr = trackListRelationshipSet->CreateRelationship(
                NULL,                         // Use auto-generated relationship ID.
                g_trackRelationshipType,
                trackPartUri,                 // Relationship target's URI.
                OPC_URI_TARGET_MODE_INTERNAL, // Relationship's target is internal.
                NULL                          // No pointer needed since returned relationship not required.
                );
    }

    
    if (SUCCEEDED(hr))
    {
        // Add a Lyrics part that contains lyrics for added track.
        hr = AddPartToBundle(
                opcFactory,
                lyricsName,
                packagePartSet,
                g_lyricsContentType,
                OPC_COMPRESSION_NORMAL,
                inputDirectory,
                &lyricsPartUri
                );
    }

    if (SUCCEEDED(hr))
    {
        // Get relationship set for Track part. The relationship set
        // represents the Relationship part that stores relationships that have
        // the Track part as their source. 
        hr = trackPart->GetRelationshipSet(&trackRelationshipSet);
    }

    if (SUCCEEDED(hr))
    {
        // Add a relationship to the Track part's Relationships part,
        // represented as a relationship object in the relationship object set.
        hr = trackRelationshipSet->CreateRelationship(
                NULL,                         // Use auto-generated relationship ID.
                g_lyricsRelationshipType,
                lyricsPartUri,                // Relationship target's URI.
                OPC_URI_TARGET_MODE_INTERNAL, // Relationship's target is internal.
                NULL                          // No pointer needed since returned relationship not required.
                );
    }
    
    // Release resources
    if (trackPartUri)
    {
        trackPartUri->Release();
        trackPartUri = NULL;
    }

    if (lyricsPartUri)
    {
        lyricsPartUri->Release();
        lyricsPartUri = NULL;
    }

    if (trackPart)
    {
        trackPart->Release();
        trackPart = NULL;
    }

    if (trackRelationshipSet)
    {
        trackRelationshipSet->Release();
        trackRelationshipSet = NULL;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Adds the Album Art part to the package and creates a package relationship to
// that has the Album Art part as its target.
///////////////////////////////////////////////////////////////////////////////
HRESULT
AddAlbumArtToBundle(
    IOpcFactory  *opcFactory,
    IOpcPartSet  *packagePartSet, // Represents the set of parts (excluding Relationships parts)
                                  // in a package.
    IOpcRelationshipSet  *packageRelationshipSet, // Represents the Relationships part that stores
                                                  // package relationships.
    LPCWSTR  inputDirectory       // Directory location of the files specified in trackName and
                                  // lyricsName.
    )
{
    HRESULT hr = S_OK;
    IOpcPartUri * albumArtPartUri = NULL;
    
    // Add Album Art part.
    hr = AddPartToBundle(
            opcFactory,
            g_albumArt,
            packagePartSet,
            g_albumArtContentType,
            OPC_COMPRESSION_NONE,
            inputDirectory,
            &albumArtPartUri
            );
    
    if (SUCCEEDED(hr))
    {
        // Add a package relationship that has the Album Art part as its target.
        hr = packageRelationshipSet->CreateRelationship(
                NULL,                         // Use auto-generated relationship ID.
                g_albumArtRelationshipType,
                albumArtPartUri,              // Relationship target's URI.
                OPC_URI_TARGET_MODE_INTERNAL, // Relationship's target is internal.
                NULL                          // No pointer needed since returned relationship not required.
                );
    }

    // Release resources
    if (albumArtPartUri)
    {
        albumArtPartUri->Release();
        albumArtPartUri = NULL;
    }
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Description:
// Adds the Track List part to the package, and creates and adds a package
// relationship to the package relationship set that has the Track List part as
// its target, and then adds the track and lyrics to the music bundle.
///////////////////////////////////////////////////////////////////////////////
HRESULT
AddTrackListToBundle(
    IOpcFactory  *opcFactory,
    IOpcPartSet  *packagePartSet, // Represents the set of parts (excluding Relationships parts)
                                  // in a package.
    IOpcRelationshipSet  *packageRelationshipSet, // The relationship set that stores package
                                                  // relationships.
    LPCWSTR  inputDirectory       // Directory location of the files specified in trackName and
                                  // lyricsName.
    )
{
    HRESULT hr = S_OK;
    IOpcPartUri * trackListPartUri = NULL;
    IOpcPart * trackListPart = NULL;
    IOpcRelationshipSet * trackListRelationshipSet = NULL;

    // Add Track List part.
    hr = AddPartToBundle(
            opcFactory,
            g_trackList,
            packagePartSet,
            g_trackListContentType,
            OPC_COMPRESSION_NORMAL,
            inputDirectory,
            &trackListPartUri,
            &trackListPart
            );
    
    if (SUCCEEDED(hr))
    {
        // Add a package relationship that has the Track List part as its
        // target.
        hr = packageRelationshipSet->CreateRelationship(
                NULL,                         // Use auto-generated relationship ID.
                g_trackListRelationshipType,
                trackListPartUri,             // Relationship target's URI.
                OPC_URI_TARGET_MODE_INTERNAL, // Relationship's target is internal.
                NULL                          // No pointer needed since returned relationship not required.
                );
    }

    if (SUCCEEDED(hr))
    {
        // Get the relationship set that represents the Relationships part that
        // stores the relationships that have the Track List part as their
        // source.
        hr = trackListPart->GetRelationshipSet(&trackListRelationshipSet);
    }
    
    if(SUCCEEDED(hr))
    {
        // Add all track and lyric files to the music bundle as Track parts
        // and Lyric parts, respectively.
        for (int i = 0; i < countof(g_trackNames) && SUCCEEDED(hr); i++)
        {
            hr = AddTrackAndLyricsToBundle(
                    opcFactory, 
                    packagePartSet,
                    trackListRelationshipSet, 
                    inputDirectory, 
                    g_trackNames[i], 
                    g_lyricsNames[i]
                    );
        }
    }

    // Release resources
    if (trackListPartUri)
    {
        trackListPartUri->Release();
        trackListPartUri = NULL;
    }

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
// Creates a package relationship that targets the album website and adds the
// relationship to the package relationship set. 
///////////////////////////////////////////////////////////////////////////////
HRESULT
AddAlbumWebsiteToBundle(
    IOpcRelationshipSet  *packageRelationshipSet // Relationship set that has all package relationships.
    )
{
    HRESULT hr = S_OK;
    IUri * uri = NULL;

    // Create the URI for the Album Website.
    hr = CreateUri(
            g_albumWebsite,
            Uri_CREATE_CANONICALIZE,
            0,
            &uri
            );

    if (SUCCEEDED(hr))
    {
        // Add Album Website as a package relationship. Note that the target
        // mode of the relationship is "External" because the website is
        // a resource that exists outside of the package.
        hr = packageRelationshipSet->CreateRelationship(
                NULL,                         // Use auto-generated relationship ID.
                g_albumWebsiteRelationshipType,
                uri,                          // Relationship target's URI.
                OPC_URI_TARGET_MODE_EXTERNAL, // Relationship's target is external.
                NULL                          // No pointer needed since returned relationship not required.
                );
    }
    
    // Release resources
    if (uri)
    {
        uri->Release();
        uri = NULL;
    }

    return hr;
}

//---------------------------------
// Function to create the new music bundle.
// Exposed through MusicBundle.h.
//
///////////////////////////////////////////////////////////////////////////////
// Description:
// Creates a package that is a music bundle, in compliance with both the OPC
// specification and the Music Bundle specification, which can be found in 
// MusicBundle.h. Given the directory that contains all the files needed, this
// method creates all parts and relationships for the new music bundle and
// saves resultant package.
//
// Note: Relationships parts are not created explicitly. Relationship sets are
// serialized as Relationships parts when the package is saved.
///////////////////////////////////////////////////////////////////////////////
HRESULT
ProduceMusicBundle(
    LPCWSTR inputDirectory,   // Parent directory that contains files to add to the music bundle.
    LPCWSTR outputPackageName // Name of the music bundle package to create.
    )
{
    HRESULT hr = S_OK;

    IOpcFactory * opcFactory = NULL;
    IOpcPackage * opcPackage = NULL;
    IOpcPartSet * packagePartSet = NULL;
    IOpcRelationshipSet * packageRelationshipSet = NULL;
    IStream * fileStream = NULL;

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
        // Create the new, empty music bundle.
        hr = opcFactory->CreatePackage(&opcPackage);
    }
    
    if (SUCCEEDED(hr))
    {
        // Get the set of parts in the package. Parts (that are not 
        // Relationships parts) to be included in the music bundle to be
        // created will be created in this set.
        hr = opcPackage->GetPartSet(&packagePartSet);
    }

    if (SUCCEEDED(hr))
    {
        // Get the set of package relationships. All package relationships
        // specific to the music bundle to be created will be created in this
        // set.
        hr = opcPackage->GetRelationshipSet(&packageRelationshipSet);
    }
    
    // Populate the music bundle.

    if (SUCCEEDED(hr))
    {
        // Add the Album Art part to the package.
        hr = AddAlbumArtToBundle(
                opcFactory,
                packagePartSet,
                packageRelationshipSet,
                inputDirectory
                );
    }
    if (SUCCEEDED(hr))
    {
        // Add the Track List part, and Track and Lyrics parts to package.
        hr = AddTrackListToBundle(
                opcFactory,
                packagePartSet,
                packageRelationshipSet,
                inputDirectory
                );
    }
    if (SUCCEEDED(hr))
    {
        // Add a package relationship that targets the album website to the
        // package.
        hr = AddAlbumWebsiteToBundle(packageRelationshipSet);
    }

    // Save the music bundle.

    if (SUCCEEDED(hr))
    {
        // Create a writable stream over the name of the package to be created.
        hr = opcFactory->CreateStreamOnFile(
                outputPackageName,
                OPC_STREAM_IO_WRITE,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                &fileStream
                );
    }
    if (SUCCEEDED(hr))
    {
        // Serialize package content to the writable stream.
        hr = opcFactory->WritePackageToStream(
                opcPackage,
                OPC_WRITE_DEFAULT,
                fileStream
                );
    }

    // Release resources
    if (opcFactory)
    {
        opcFactory->Release();
        opcFactory = NULL;
    }

    if (opcPackage)
    {
        opcPackage->Release();
        opcPackage = NULL;
    }

    if (packagePartSet)
    {
        packagePartSet->Release();
        packagePartSet = NULL;
    }

    if (packageRelationshipSet)
    {
        packageRelationshipSet->Release();
        packageRelationshipSet = NULL;
    }

    if (fileStream)
    {
        fileStream->Release();
        fileStream = NULL;
    }

    return hr;
}
//</SnippetMusicBundle_cppProductionWholePage>