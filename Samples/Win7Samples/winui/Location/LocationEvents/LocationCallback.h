// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This is the header for the declaration of our callback object

#pragma once


#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h>


class CLocationEvents :
    public CComObjectRoot,
    public ILocationEvents // We must include this interface so the Location API knows how to talk to our object
{
public:
    CLocationEvents() : m_previousTime(0) {}
    virtual ~CLocationEvents(){}

    DECLARE_NOT_AGGREGATABLE(CLocationEvents)

    BEGIN_COM_MAP(CLocationEvents)
        COM_INTERFACE_ENTRY(ILocationEvents)
    END_COM_MAP()

    // ILocationEvents

    // This is called when there is a new location report
    STDMETHOD(OnLocationChanged)(REFIID reportType, ILocationReport* pLocationReport);

    // This is called when the status of a report type changes.
    // The LOCATION_REPORT_STATUS enumeration is defined in LocApi.h in the SDK
    STDMETHOD(OnStatusChanged)(REFIID reportType, LOCATION_REPORT_STATUS status);

private:
    // This is a private helper that allows us to print a GUID to the console in a friendly format
    void CLocationEvents::PrintGUID(const GUID guid);

    // We store the previous report timestamp here, and use this when computing the time between the current and previous report
    ULONGLONG m_previousTime;
};