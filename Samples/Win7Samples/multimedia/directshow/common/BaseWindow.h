//////////////////////////////////////////////////////////////////////////
// BaseWindow.h: Abstract window class.
//
// Note: This class is designed to be as minimal as possible for
// purposes of a sample Win32 application. It is not meant to be
// a complete solution along the lines of MFC or ATL.
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


class BaseWindow
{
public:

	BaseWindow();
	virtual ~BaseWindow() {}

	static LRESULT CALLBACK	WindowProc(HWND, UINT, WPARAM, LPARAM);

	HRESULT Create(HINSTANCE hInstance);
	HRESULT Show(int nCmdShow);

	virtual LRESULT OnReceiveMessage(UINT msg, WPARAM wparam, LPARAM lparam);

protected:

	HRESULT Register();

	virtual LPCTSTR ClassName() const = 0;
	virtual LPCTSTR MenuName() const { return NULL; }
	virtual LPCTSTR WindowName() const = 0;

	virtual void OnPaint() = 0;


	HWND		m_hwnd;
	HINSTANCE	m_hInstance;

};