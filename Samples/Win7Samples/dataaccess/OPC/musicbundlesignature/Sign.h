//<SnippetMusicBundleSig_hSignWholePage>
/*****************************************************************************
*
* File: Sign.h
*
* Description:
* This sample is a simple application that might be used as a starting-point
* for an application that uses the Packaging API. This sample demonstrates
* signature generation and validation using a sample signing policy described
* in Sign.h
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
* 
****************************************************************************/

#pragma once

//  ============================= Signing Policy ==================================
//
//  A signing policy defines what parts and relationships shall be signed and how they 
//  should be signed for a package so the signature can be considered "compliant" to 
//  a custom file format. OPC itself is a platform for users to build custom file formats, 
//  thus it does not define any signing policy. Users using OPC Digital Signature APIs 
//  shall define the signing policy for their custom file formats. In this example we define
//  the signing policy for the custom file format - a media bundle, as follows: 
//
//      Parts need to be signed:	
//  ---------------------------------------------
//  Part                Canonicalization Method
//  ---------------------------------------------
//  Album art part      OPC_CANONICALIZATION_NONE
//  Tracklist part      OPC_CANONICALIZATION_NONE
//  Track part          OPC_CANONICALIZATION_NONE
//  Lyric part          OPC_CANONICALIZATION_NONE
//
//  Note that since none of our parts are in XML, we must use OPC_CANONICALIZATION_NONE
//  for canonicalization method. If the parts you sign are in XML, then you may consider
//  using OPC_CANONICALIZATION_C14N or OPC_CANONICALIZATION_C14N_WITH_COMMENTS.
//
//      Relationships need to be signed:
//  ---------------------------------------------
//  Relationship Source        Relationship Type
//  ---------------------------------------------
//  Package root            http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail
//  Package root            http://schemas.example.com/package/2008/relationships/media-bundle/album-website
//  Package root            http://schemas.example.com/package/2008/relationships/media-bundle/tracklist
//  Tracklist part          http://schemas.example.com/package/2008/relationships/media-bundle/playlist-song
//  Track part              http://schemas.example.com/package/2008/relationships/media-bundle/song-lryic
//  Signature origin part   http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/signature
//
//  We sign the signature origin part’s signature relationships to prevent any other 
//  signatures to be added to the signed package later (no countersignature is allowed 
//  in this policy). If you want to allow new signature(s) to be added to an already signed
//  package, you should not sign the signature relationships in your policy.
//
//  ==========================================================================================

// The file path of the signed music bundle
extern const WCHAR g_signedFilePath[];

// Performs signing a music bundle task.
HRESULT
SignMusicBundle(
    IOpcFactory* opcFactory
    );

//</SnippetMusicBundleSig_hSignWholePage>