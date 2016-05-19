/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "DiscMaster2Event.h"

// IDiscMaster2Events methods
STDMETHODIMP_(VOID)
CTestDiscMaster2Event::NotifyDeviceAdded(
    IDispatch* object,
    BSTR uniqueId
    )
{
    UNREFERENCED_PARAMETER (object);
    printf("Device   Added: %ws\n", uniqueId);
    return;
}
STDMETHODIMP_(VOID)
CTestDiscMaster2Event::NotifyDeviceRemoved(
    IDispatch* object,
    BSTR uniqueId
    )
{
    UNREFERENCED_PARAMETER (object);
    printf("Device Removed: %ws\n", uniqueId);
    return;
}
