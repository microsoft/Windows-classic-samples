//-----------------------------------------------------------------------------
// File: CreateWavSink.h
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once


//-------------------------------------------------------------------
// Name: CreateWavSink 
// Description: Creates an instance of the WavSink object. 
//
// To use the WavSink, include this header file in an application
// and link to the library file created by this project.
//
// pStream: Pointer to a bytestream where the .wav file will be
//          written. The bystream must support writing and seeking.
//
// ppSink:  Receives a pointer to the IMFMediaSink interface. The
//          caller must release the interface.
//-------------------------------------------------------------------

HRESULT CreateWavSink(IMFByteStream *pStream, IMFMediaSink **ppSink);
