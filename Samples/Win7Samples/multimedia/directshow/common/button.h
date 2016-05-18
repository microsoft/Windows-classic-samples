//////////////////////////////////////////////////////////////////////////
// Button.h: Button control class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once


// Simple text or bitmap button
class Button : public Control
{
public:
	HRESULT CreateText(HWND hParent, const TCHAR *szCaption, int nID, const Rect& rcBound);
	HRESULT CreateBitmap(HWND hParent, int nImgID, int nID, const Rect& rcBound);
	BOOL SetImage(WORD nImgId);

	void SetCheck(int nCheck)
	{
		SendMessage(BM_SETCHECK, (WPARAM)nCheck, 0L);
	}

	BOOL IsChecked()
	{
		return SendMessage(BM_GETCHECK, 0, 0 ) == BST_CHECKED;
	}
};

