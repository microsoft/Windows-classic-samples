//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  MemoryStream.h
//
//          IStream Service functions.
//
//////////////////////////////////////////////////////////////////////

extern IStream* CreateMemoryStream();
extern void ClearStream(IStream *pStream);
extern void AddStringToStream(IStream *pStream, WCHAR *psz);

