//------------------------------------------------------------------------------
// File: GargUIDs.h
//
// Desc: DirectShow sample code - definition of CLSIDs.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __GARGUIDS__
#define __GARGUIDS__

#ifdef __cplusplus
extern "C" {
#endif


//
// Gargle filter object
// {d616f350-d622-11ce-aac5-0020af0b99a3}
DEFINE_GUID(CLSID_Gargle,
0xd616f350, 0xd622, 0x11ce, 0xaa, 0xc5, 0x00, 0x20, 0xaf, 0x0b, 0x99, 0xa3);


//
// Gargle filter property page
// {d616f351-d622-11ce-aac5-0020af0b99a3}
DEFINE_GUID(CLSID_GargProp,
0xd616f351, 0xd622, 0x11ce, 0xaa, 0xc5, 0x00, 0x20, 0xaf, 0x0b, 0x99, 0xa3);

//
//  Note: IGargle's uuid is defined with the interface (see igargle.h)
//  IGargle is a private interface created by the filter.
//  The filter object and the property page defined here are public interfaces
//  that can be called by an application or another filter.


#ifdef __cplusplus
}
#endif

#endif // __GARGUIDS__
