//<SnippetMusicBundle_hMusicBundleWholePage>
//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Abstract:
//      Sample application for producing and consuming a music bundle package.
//----------------------------------------------------------------------------

#pragma once

//============================================================
//                    Format specification
//
// The Music Bundle format used in this sample is defined as
// follows:
//
// Parts
// =====
// Track List:
//    Part - Media Player Playlist file. Although OPC does not
//           know how to parse this particular file format, the
//           relationships can be used to find all the tracks
//           pointed to by this playlist.
//    Relationships - Internal; one targeting each Track in
//                    the bundle
// Track:
//    Part - Windows Media Audio file.
//    Relationships - Internal; one targeting the Lyrics for
//                    this track
// Lyrics:
//    Part - text file containing Lyrics for the music in the
//           corresponding Track
//    Relationships - none
// Album Art:
//    Part - image media file for album cover
//    Relationships - none
//
// Root Relationships
// ==================
// Track List:
//    Internal relationship targeting the Track List part
// Album Website:
//    External relationship targeting a website URL
// Album Art:
//    Internal relationship targeting the Album Art part
//
//============================================================

//============================================================
//                   Format specific data
//
// The format has well-defined Content Types for parts and
// Relationship Types for relationships as defined below:
//============================================================

// Content types for parts in music bundle
extern const WCHAR g_trackContentType[];
extern const WCHAR g_lyricsContentType[];
extern const WCHAR g_albumArtContentType[];
extern const WCHAR g_trackListContentType[];

// Relationship types for relationships in music bundle
extern const WCHAR g_trackRelationshipType[];
extern const WCHAR g_lyricsRelationshipType[];
extern const WCHAR g_albumArtRelationshipType[];
extern const WCHAR g_trackListRelationshipType[];
extern const WCHAR g_albumWebsiteRelationshipType[];

//============================================================
//                     Production Methods
//============================================================
//
// Creates an OPC package representing a Music Bundle
// It adds all parts and relationships as specified
// by the Music Bundle format
//
HRESULT
ProduceMusicBundle(
    LPCWSTR     inputDirectory,
    LPCWSTR     outputPackageName
    );

//============================================================
//                     Consumption Methods
//============================================================
//
// Consumes a Music Bundle package. It writes the parts to files in
// output directory. This directory can then be used to round-trip
// and recreate the package. The Track List and Lyrics parts and
// the Album Website are also displayed to the console.
//
HRESULT
ConsumeMusicBundle(
    LPCWSTR     inputPackageName,
    LPCWSTR     outputDirectory
    );
//</SnippetMusicBundle_hMusicBundleWholePage>
