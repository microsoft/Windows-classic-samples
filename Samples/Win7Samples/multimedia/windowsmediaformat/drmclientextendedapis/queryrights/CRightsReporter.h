///////////////////////////////////////////////////////////////////////////////
//
// CRightsReporter.h : Contains declarations for the CRightsReporter class.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <wmdrmsdk.h>
#include "DRMSampleUtils.h"
#include <strsafe.h>

///////////////////////////////////////////////////////////////////////////////
//
// Class: CRightsReporter
// Description: Encapsulates the process of querying for the rights granted by
//  licenses on a local machine for a list of Key IDs.
// Notes: The usage of this class follows a simple, predictable pattern. Before
//  using any other methods, the application must call Initialize, which 
//  allocates the rights lists used by the querying methods. Then the 
//  application calls SetReportFile and SetKIDStrings (in either order) to set
//  up the input and output needed for generating the report. Next, the 
//  application calls CreateRightsReport to generate the report. Finally, the 
//  application calls ShutDown to release all allocated resources.
//
// In practical use, this class reports on a single list of KIDs per instance.
//  However, additional calls to SetKIDStrings and SetReportFile can be made
//  to accomodate reports for multiple lists.
// 
///////////////////////////////////////////////////////////////////////////////
class CRightsReporter
{
public:
    CRightsReporter(); 
    ~CRightsReporter();
    HRESULT Initialize();
    HRESULT SetReportFile(const WCHAR* pwszFilename);
    HRESULT SetKIDStrings(WCHAR** ppKIDStrings, int NumKIDs);
    HRESULT CreateRightsReport();
    void    ShutDown();

private:

    HRESULT WriteReportHeader();
    HRESULT GetRightsForKID(BSTR KID);
    HRESULT GetDetailedRightsForKID(BSTR KID);

    // Interfaces.
    IWMDRMProvider*     m_pProvider;
    IWMDRMLicenseQuery* m_pLicenseQuery;

    // KID string array.
    BSTR* m_KIDStrings;
    int   m_NumKIDs;

    // Rights string arrays (for the two types of query).
    BSTR* m_IsAllowedRights;
    BSTR* m_LicenseStateRights;
    int   m_cIsAllowedRights;
    int   m_cLicenseStateRights;

    // Query result arrays (for the two types of query).
    DWORD*                  m_IsAllowedResults;
    DRM_LICENSE_STATE_DATA* m_LicenseStateResults;

    // Output file.
    FILE* m_pFile;
};