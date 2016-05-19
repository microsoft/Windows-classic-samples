// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DimmerDeviceDCO.h 

#ifndef __DimmerDevice_H_
#define __DimmerDevice_H_

// This is equal to the number of devices (root or embedded) in the description doc
#define NUM_TEMPLATE_UDNS_IN_DESCDOC 1	

// This  is equal to the maximum number of services in any device in the description doc
#define MAX_NOSERVICE_PER_DEVICE 1		



////////////////////////////////////////////////////////////////
// UPNPDimmerDevice : class definition for the Device Control 
//                    Implementation for the Dimmer Device
////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE UPNPDimmerDevice:
   public CComObjectRootEx<CComMultiThreadModel>,
   public CComCoClass<UPNPDimmerDevice, &CLSID_UPNPSampleDimmerDevice>,
   public IUPnPDeviceControl
{
private:
   // variables used internally are declared

   // String that stores the description doc that is passed by the 
   // Device Host during Intialize
   BSTR descriptionDoc;

   // table to hold mapping from template UDN's to the UDN's actually 
   // used by the DeviceHost when publishing the device
   BSTR UDNmapping[NUM_TEMPLATE_UDNS_IN_DESCDOC][2];

   // now for each of the Template UDN's we maintain a list of services 
   // in it and the IDispatch interface pointers the serviceIDDeviceMap 
   // table maps between devices (root or embedded) and the service Id's in the device
   BSTR serviceIDDeviceMap[NUM_TEMPLATE_UDNS_IN_DESCDOC][MAX_NOSERVICE_PER_DEVICE];

   // this table maps between the services in the device (root or embedded) 
   // and the IDispatch pointers
   IDispatch* pDisPtrs[NUM_TEMPLATE_UDNS_IN_DESCDOC][MAX_NOSERVICE_PER_DEVICE];


public:
   UPNPDimmerDevice();
   ~UPNPDimmerDevice();
   STDMETHODIMP UPNPDimmerDevice::Initialize(BSTR bstrXMLDesc,BSTR deviceID,BSTR bstrInitString);
   STDMETHODIMP UPNPDimmerDevice::GetServiceObject(BSTR bstrUDN,BSTR bstrServiceId,IDispatch **ppdispService);

   DECLARE_REGISTRY_RESOURCEID(IDR_UPNPDIMMERDEVICE)

   DECLARE_PROTECT_FINAL_CONSTRUCT()

   BEGIN_COM_MAP(UPNPDimmerDevice)
      COM_INTERFACE_ENTRY(IUPnPDeviceControl)
   END_COM_MAP()

};

#endif //__DimmerDevice_H_
