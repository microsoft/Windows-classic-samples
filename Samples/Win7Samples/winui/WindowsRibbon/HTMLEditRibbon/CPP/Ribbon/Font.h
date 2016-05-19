// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(FONT_H__6CEBA2A3_A812_4E4A_BACF_05E6672C9DB3__INCLUDED_)
#define FONT_H__6CEBA2A3_A812_4E4A_BACF_05E6672C9DB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Set FontName and FontSize to the HTML Editor.
HRESULT SetFont(CMainFrame* pFrame, IPropertyStore* pStore);
// Update FontName and FontSize controls in the ribbon.
HRESULT UpdateFont(CMainFrame* pFrame, IPropertyStore* pStore);

#endif // !defined(FONT_H__6CEBA2A3_A812_4E4A_BACF_05E6672C9DB3__INCLUDED_)
