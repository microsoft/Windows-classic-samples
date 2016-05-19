//------------------------------------------------------------------------------
// File: IEZ.h
//
// Desc: DirectShow sample code - custom interface to allow the user to
//       perform image special effects.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __IEZ__
#define __IEZ__

#ifdef __cplusplus
extern "C" {
#endif

    // { fd5010a3-8ebe-11ce-8183-00aa00577da1 }
    DEFINE_GUID(IID_IIPEffect,
    0xfd5010a3, 0x8ebe, 0x11ce, 0x81, 0x83, 0x00, 0xaa, 0x00, 0x57, 0x7d, 0xa1);

    DECLARE_INTERFACE_(IIPEffect, IUnknown)
    {
        STDMETHOD(get_IPEffect) (THIS_
                    int *effectNum,         // The current effect
                    REFTIME *StartTime,     // Start time of effect
                    REFTIME *Length         // length of effect
                 ) PURE;

        STDMETHOD(put_IPEffect) (THIS_
                    int effectNum,          // Change to this effect
                    REFTIME StartTime,      // Start time of effect
                    REFTIME Length          // Length of effect
                 ) PURE;
    };

#ifdef __cplusplus
}
#endif

#endif // __IEZ__

