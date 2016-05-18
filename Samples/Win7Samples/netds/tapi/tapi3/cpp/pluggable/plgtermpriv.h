
#ifndef __PLG_PRIV__
#define __PLG_PRIV__

#include <objbase.h>
#include <initguid.H>


//{26b65f6d-f315-4162-95a0-637b21c0cb6f}
DEFINE_GUID(IID_ITPlgPrivEventSink, 
	0x26b65f6d, 0xf315, 0x4162, 0x95, 0xa0, 0x63, 0x7b, 0x21, 0xc0, 0xcb, 0x6f);

interface __declspec(uuid("26b65f6d-f315-4162-95a0-637b21c0cb6f")) ITPlgPrivEventSink : public IUnknown
//
// ITPlgPrivEventSink
//
//interface DECLSPEC_NOVTABLE ITPlgPrivEventSink : IUnknown
//DECLARE_INTERFACE_(
//    ITPlgPrivEventSink, IUnknown)
{
    STDMETHOD (FireEvent)(long lEventCode) = 0;
};
#endif //__PLG_PRIV__