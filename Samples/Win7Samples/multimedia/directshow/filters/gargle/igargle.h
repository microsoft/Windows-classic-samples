//------------------------------------------------------------------------------
// File: IGargle.h
//
// Desc: DirectShow sample code - custom interface that allows the user
//       to adjust the modulation rate.  It defines the interface between
//       the user interface component (the property sheet) and the filter
//       itself.  This interface is exported by the code in Gargle.cpp and
//       is used by the code in GargProp.cpp.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __IGARGLE__
#define __IGARGLE__

#ifdef __cplusplus
extern "C" {
#endif


//
// IGargle's GUID
//
// {d616f352-d622-11ce-aac5-0020af0b99a3}
DEFINE_GUID(IID_IGargle,
0xd616f352, 0xd622, 0x11ce, 0xaa, 0xc5, 0x00, 0x20, 0xaf, 0x0b, 0x99, 0xa3);


//
// IGargle
//
DECLARE_INTERFACE_(IGargle, IUnknown) {

    // Compare these with the functions in class CGargle in gargle.h
    STDMETHOD(get_GargleRate)
        ( THIS_
          int *GargleRate  // [out] the current gargle level
        ) PURE;

    STDMETHOD(put_GargleRate)
        ( THIS_
          int GargleRate   // [in] Change to the level of gargle
        ) PURE;

    STDMETHOD(put_DefaultGargleRate)
        ( THIS
        ) PURE;

    STDMETHOD(put_GargleShape)
        ( THIS_
          int GargleShape // [in] 0=triangle wave, 1 = square wave
        ) PURE;

    STDMETHOD(get_GargleShape)
        ( THIS_
          int *GargleShape  // [out] the current shape, 0=triangle, 1=square
        ) PURE;

};


#ifdef __cplusplus
}
#endif

#endif // __IGARGLE__
