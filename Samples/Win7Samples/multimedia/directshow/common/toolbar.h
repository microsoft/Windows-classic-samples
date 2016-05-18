//////////////////////////////////////////////////////////////////////////
// Toolbar.h: Toolbar and rebar control classes.
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


class Toolbar : public Control
{
public:

    // Thin wrapper for TBBUTTON structure, which
    // holds information about a toolbar button.
    struct Button : TBBUTTON
    {
        Button(int bitmap, int command);
    };

	enum ButtonState
	{
		Normal, Hot, Disabled
	};

    Toolbar();
	~Toolbar();
    HRESULT Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle = 0);
    HRESULT AddButton(const Button& button);  
    HRESULT Check(int id, BOOL fCheck);
    HRESULT Enable(int id, BOOL fCheck);

	HRESULT SetImageList(
		ButtonState state,		// Button state associated with this image list (normal, disabled, hot)
		UINT nBitmapID,			// Bitmap resource ID. Bitmap must be 24 bpp
		const Size& buttonSize, // Size of each button in pixels
		DWORD numButtons,		// Number of buttons
		COLORREF mask			// Color mask
		);

	HRESULT SetButtonImage(int command, int bitmap);

    HRESULT ShowToolTip(NMTTDISPINFO *pDispInfo);

private:

	HIMAGELIST	m_hImageListNormal;
	HIMAGELIST	m_hImageListHot;
	HIMAGELIST	m_hImageListDisabled;
};



class Rebar : public Control
{
public:
    Rebar();
    HRESULT Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle = 0);
    HRESULT AddBand(HWND hBand, UINT id);
    HRESULT ShowBand(UINT id, BOOL bShow);

private:
    HRESULT BandIdToIndex(UINT id, UINT *pIndex);
};
