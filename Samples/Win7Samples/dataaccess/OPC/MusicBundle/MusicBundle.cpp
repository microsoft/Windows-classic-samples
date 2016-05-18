//<SnippetMusicBundle_cppMusicBundleWholePage>
/*****************************************************************************
*
* File: MusicBundle.cpp
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

// Content types for parts in a music bundle, as defined in the music bundle
// specification, which is in the MusicBundle.h header file.
const WCHAR g_trackContentType[] = L"audio/x-ms-wma";
const WCHAR g_lyricsContentType[] = L"text/plain";
const WCHAR g_albumArtContentType[] = L"image/jpeg";
const WCHAR g_trackListContentType[] = L"application/vnd.ms-wpl";

// Relationship types for relationships in music bundle, as defined in the
// music bundle specification, which is in the MusicBundle.h header file.
// As described in the OPC, the format creator can create custom relationship
// types that identify what kinds of resources that are the targets of 
// relationships in the custom package format.
const WCHAR g_trackRelationshipType[] = L"http://schemas.example.com/package/2008/relationships/media-bundle/playlist-song";
const WCHAR g_lyricsRelationshipType[] = L"http://schemas.example.com/package/2008/relationships/media-bundle/song-lryic";
const WCHAR g_trackListRelationshipType[] = L"http://schemas.example.com/package/2008/relationships/media-bundle/tracklist";
const WCHAR g_albumWebsiteRelationshipType[] = L"http://schemas.example.com/package/2008/relationships/media-bundle/album-website";

// The thumbnails relationship type is defined in the OPC specification.
const WCHAR g_albumArtRelationshipType[] = L"http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail";

//============================================================
// Main entry point of the sample.
//============================================================
int
wmain(
    int         argc,
    wchar_t *   argv[]
    )
{
    bool bShowHelp = false; // Indicates whether parameter help should be shown.

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Check if consuming or producing Music Bundle.
        if (argc == 4)
        {
            if(lstrcmpi(L"-p", argv[1]) == 0)
            {
                // Produce Music Bundle.
                hr = ProduceMusicBundle(
                        argv[2], // Input directory.
                        argv[3]  // Output package name.
                        );

                if(FAILED(hr))
                {
                    fwprintf(
                        stderr,
                        L"Production failed with error : 0x%x\n",
                        hr
                        );
                }
            }
            else if(lstrcmpi(L"-c", argv[1]) == 0)
            {
                // Consume Music Bundle.
                hr = ConsumeMusicBundle(
                        argv[2], // Name of package to consume.
                        argv[3]  // Output directory.
                        );

                if(FAILED(hr))
                {
                    fwprintf(
                        stderr,
                        L"Consumption failed with error : 0x%x\n",
                        hr
                        );
                }
            }
            else
            {
                // Neither production or consumption were indicated.
                bShowHelp = true;
            }
        }
        else
        {
            // Wrong number of aruguments.
            bShowHelp = true;
        }
        
        if(bShowHelp)
        {
            // Input arguments are invalid; show help text.
            wprintf(L"Music Bundle Sample:\n");
            wprintf(L"To Produce Bundle : MusicBundle.exe -p <Input Directory> <Output Package Path>\n"); 
            wprintf(L"To Consume Bundle : MusicBundle.exe -c <Input Package Path> <Output Directoy>\n");
        }
        
        CoUninitialize();
    }
    else
    {
        fwprintf(stderr, L"CoInitializeEx() failed with error: 0x%x\n", hr);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}
//</SnippetMusicBundle_cppMusicBundleWholePage>