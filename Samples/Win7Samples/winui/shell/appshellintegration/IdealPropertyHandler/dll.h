// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <windows.h>
#include <new> // std::nothrow
#include "resource.h"

// stuff from the Windows SDK that we will use
class DECLSPEC_UUID("8d80504a-0826-40c5-97e1-ebc68f953792") DocFilePropertyHandler;
class DECLSPEC_UUID("97e467b4-98c6-4f19-9588-161b7773d6f6") OfficeDocFilePropertyHandler;
class DECLSPEC_UUID("45670FA8-ED97-4F44-BC93-305082590BFB") XPSPropertyHandler;
class DECLSPEC_UUID("c73f6f30-97a0-4ad1-a08f-540d4e9bc7b9") XMLPropertyHandler;
class DECLSPEC_UUID("993BE281-6695-4BA5-8A2A-7AACBFAAB69E") OfficeOPCPropertyHandler;
class DECLSPEC_UUID("C41662BB-1FA0-4CE0-8DC5-9B7F8279FF97") OfficeOPCExtractImageHandler;
class DECLSPEC_UUID("9DBD2C50-62AD-11D0-B806-00C04FD706EC") PropertyThumbnailHandler;

const WCHAR c_szDocFullDetails[] = L"prop:System.PropGroup.Description;System.Title;System.Subject;System.Keywords;System.Category;System.Comment;System.Rating;System.PropGroup.Origin;System.Author;System.Document.LastAuthor;System.Document.RevisionNumber;System.Document.Version;System.ApplicationName;System.Company;System.Document.Manager;System.Document.DateCreated;System.Document.DateSaved;System.Document.DatePrinted;System.Document.TotalEditingTime;System.PropGroup.Content;System.ContentStatus;System.ContentType;System.Document.PageCount;System.Document.WordCount;System.Document.CharacterCount;System.Document.LineCount;System.Document.ParagraphCount;System.Document.Template;System.Document.Scale;System.Document.LinksDirty;System.Language;System.PropGroup.FileSystem;System.ItemNameDisplay;System.ItemType;System.ItemFolderPathDisplay;System.DateCreated;System.DateModified;System.Size;System.FileAttributes;System.OfflineAvailability;System.OfflineStatus;System.SharedWith;System.FileOwner;System.ComputerName";
const WCHAR c_szDocInfoTip[] = L"prop:System.ItemType;System.Author;System.Size;System.DateModified;System.Document.PageCount";
const WCHAR c_szDocPreviewDetails[] = L"prop:*System.DateModified;System.Author;System.Keywords;System.Rating;*System.Size;System.Title;System.Comment;System.Category;*System.Document.PageCount;System.ContentStatus;System.ContentType;*System.OfflineAvailability;*System.OfflineStatus;System.Subject;*System.DateCreated;*System.SharedWith";

class DECLSPEC_UUID("E1A6729E-D430-46a4-A45D-CC042D7F7B36") COpenMetadataHandler;
HRESULT COpenMetadataHandler_CreateInstance(REFIID riid, void **ppv);

void DllAddRef();
void DllRelease();

HRESULT RegisterDocFile();
HRESULT UnregisterDocFile();
HRESULT RegisterOpenMetadata();
HRESULT UnregisterOpenMetadata();
