//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module GUIDS.H | Internal GUIDS
//
//
#ifndef _GUIDS_H_
#define _GUIDS_H_



// @msg CLSID_SampProv | {E8CCCB79-7C36-101B-AC3A-00AA0044773D} 
// Provider Class Id
DEFINE_GUID(CLSID_SampProv,	 0xE8CCCB79L,0x7C36,0x101B,0xAC,0x3A,0x00,0xAA,0x00,0x44,0x77,0x3D);

// @msg DBGUID_SAMPLEDIALECT | {E9FBAF50-D402-11ce-BEDC-00AA00A14D7D}
//	The dialect GUID for the Sample Provider
DEFINE_GUID(DBGUID_SAMPLEDIALECT,	0xe9fbaf50, 0xd402, 0x11ce, 0xbe, 0xdc, 0x0, 0xaa, 0x0, 0xa1, 0x4d, 0x7d);

// {119C8711-905B-11d2-AF65-00C04F6F8697}
DEFINE_GUID(CLSID_SampProvConnectionPage, 
0x119c8711, 0x905b, 0x11d2, 0xaf, 0x65, 0x0, 0xc0, 0x4f, 0x6f, 0x86, 0x97);

// {119C8712-905B-11d2-AF65-00C04F6F8697}
DEFINE_GUID(CLSID_SampProvAdvancedPage, 
0x119c8712, 0x905b, 0x11d2, 0xaf, 0x65, 0x0, 0xc0, 0x4f, 0x6f, 0x86, 0x97);

// {245E7460-B577-11D2-AF53-00C04F782926}
DEFINE_GUID(CLSID_SampProvBinder, 
0x245e7460, 0xb577, 0x11d2, 0xaf, 0x53, 0x0, 0xc0, 0x4f, 0x78, 0x29, 0x26);

#endif


