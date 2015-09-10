// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DimmerService.h


#ifndef __UPNPDimmerSERVICE_H_
#define __UPNPDimmerSERVICE_H_


class UPNPDimmerService;

class ATL_NO_VTABLE UPNPDimmerService :
   public CComObjectRootEx <CComMultiThreadModel>,
   public IDispatchImpl<IUPnPService_DimmingService_SCPD, &IID_IUPnPService_DimmingService_SCPD, &LIBID_SDKDimmerServiceLib>,
   public IUPnPEventSource
{

public:

   UPNPDimmerService();
   ~UPNPDimmerService();

   DECLARE_NOT_AGGREGATABLE(UPNPDimmerService)

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_COM_MAP(UPNPDimmerService)
      COM_INTERFACE_ENTRY(IUPnPService_DimmingService_SCPD)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IUPnPEventSource)
   END_COM_MAP()

public:

   // IUPnPService_DimmingService_SCPD methods (i.e. Dimming Service Methods)
   STDMETHOD (get_Power) (_Out_ VARIANT_BOOL *pval);
   STDMETHOD (get_dimLevel) (_Out_ LONG *dLevel);

   STDMETHOD (PowerOn) ();
   STDMETHOD (PowerOff) ();

   STDMETHOD (GetPowerValue) (_Out_ VARIANT_BOOL *powerVal);

   STDMETHOD (SetDimLevel) (_In_ LONG dLevel);
   STDMETHOD (GetDimLevel) (_Out_ LONG *dLevel);
   STDMETHOD (GetConfigDetails) (_Out_ VARIANT_BOOL *powerVal, _Out_ LONG *dLevel);

   // IUPnPEventSource methods
   STDMETHOD (Advise) (_In_ IUPnPEventSink *punkSubscriber);
   STDMETHOD (Unadvise) (_In_ IUPnPEventSink *punkSubscriber);

private:
   VARIANT_BOOL power;
   LONG dimnessLevel;
   IUPnPEventSink *esEventingManager;

   //to synchronise between multiple requests for the same action
   CRITICAL_SECTION csSync;
};

#endif //__DimmerSERVICE_H_
