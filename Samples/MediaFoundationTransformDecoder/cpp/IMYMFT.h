#pragma once

#include <Mfidl.h>

// {BC36CB81-C1FA-4B92-B12D-FCBB8DB3FDF6}
DEFINE_GUID(IID_IMYMFT, 
0xbc36cb81, 0xc1fa, 0x4b92, 0xb1, 0x2d, 0xfc, 0xbb, 0x8d, 0xb3, 0xfd, 0xf6);

class IMYMFT:
    public IUnknown
{
public:
    virtual HRESULT __stdcall   DecodeInputFrame(
                                            IMFSample*  pInputSample
                                            )  = 0;
};