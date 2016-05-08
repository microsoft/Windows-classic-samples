// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This recognizer implementation illustrates essentials of
// loading a recognizer DLL, obtaining ink strokes
// to recognize, and returning a recognition result.

#include "stdafx.h"
#include <assert.h>
#include <Strsafe.h>
#include <Objbase.h>

#include "Limits.h"
#include "RecApis.h"
#include "tpcError.h"

#include "RecoDll.h"


static void DestroyLattice( RECO_LATTICE *pLattice );
static HRESULT GetRecoAttributesHelper(HINSTANCE hDllInstance, RECO_ATTRS* pRecoAttrs);

static const GUID s_guid_x = { 0x598a6a8f, 0x52c0, 0x4ba0, { 0x93, 0xaf, 0xaf, 0x35, 0x74, 0x11, 0xa5, 0x61 } };
static const GUID s_guid_y = { 0xb53f9f75, 0x04e0, 0x4498, { 0xa7, 0xee, 0xc3, 0x0d, 0xbb, 0x5a, 0x90, 0x11 } };
static const PROPERTY_METRICS s_DefaultPropMetrics = { LONG_MIN, LONG_MAX, PROPERTY_UNITS_CENTIMETERS, 1000.0 };


static LPCWSTR TPG_REGPATH =        L"Software\\Microsoft\\TPG";
static LPCWSTR RECOGNIZER_REGPATH = L"Software\\Microsoft\\TPG\\Recognizers";
static LPCWSTR MY_RECO_REGKEY =     L"Software\\Microsoft\\TPG\\Recognizers\\" MY_RECO_GUID;

static LPCWSTR RECO_DLL_REGVALNAME =            L"Recognizer dll";
static LPCWSTR RECO_LANGUAGES_REGVALNAME =      L"Recognized Languages";
static LPCWSTR RECO_CAPABILITIES_REGVALNAME =   L"Recognizer Capability Flags";

static HINSTANCE s_hDllInstance = NULL;

struct MY_RECOGNIZER
{
    // Store runtime data here that your algorithm uses for recognizing ink.
    GUID RecoGuid;
};


#define MAX_RESULT_CHARS 200
#define MAX_STROKES_IN_CONTEXT 50000
#define MAX_POINTS_IN_STROKE  100000

struct MY_STROKE
{
    MY_STROKE           *pNext;
    ULONG               cPoints;   // number of points in the stroke
    __field_ecount_full(cPoints)
    POINT               *aPoints;  // array of x-y points in the stroke
};

struct MY_RECOCONTEXT
{
    MY_RECOGNIZER   *pRecognizer;   // recognizer to use on pStrokeList
    RECO_GUIDE      *pGuide;        // reference boundaries for strokes
    ULONG           uiGuideIndex;   // set but not used (obsolete)
    ULONG           cStrokes;       // Number of ink strokes added
    ULONG           cPointsTot;     // number of x-y points in all strokes
    MY_STROKE       *pStrokeList;   // new strokes are inserted at head of list
    WCHAR           wzBestResult[MAX_RESULT_CHARS]; // reco result based on the stroke list
    RECO_LATTICE    *pLattice;      // mapping of reco result to strokes
};


BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD ul_reason_for_call,
                       void* lpReserved
                     )
{
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            s_hDllInstance = (HINSTANCE) hModule;
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer(void)
{
    LONG        lRes = 0;
    HKEY        hkeyMyReco;
    DWORD       dwLength = 0;
    DWORD       dwDisposition;
    WCHAR       wzRecognizerPath[MAX_PATH];
    HRESULT     hr = S_OK;
    RECO_ATTRS  recoAttr;

    // Write the path to this dll in the registry under
    // the recognizer subkey

    // Wipe out the previous values
    RegDeleteKeyW(HKEY_LOCAL_MACHINE, MY_RECO_REGKEY);
    // Create the new key
    lRes = RegCreateKeyExW(HKEY_LOCAL_MACHINE, MY_RECO_REGKEY, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyMyReco, &dwDisposition);
    assert(lRes == ERROR_SUCCESS && "Can't write registry");
    if (lRes != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyMyReco);
        return E_UNEXPECTED;
    }

    // Get the Reco DLL's full pathname and write it to the registry
    dwLength = GetModuleFileNameW((HMODULE)s_hDllInstance, wzRecognizerPath, MAX_PATH);
    lRes = RegSetValueExW(hkeyMyReco, RECO_DLL_REGVALNAME, NULL, REG_SZ,
        (BYTE*)wzRecognizerPath, sizeof(WCHAR)*dwLength);
    assert(lRes == ERROR_SUCCESS && "Can't write registry");
    if (lRes != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyMyReco);
        return E_UNEXPECTED;
    }

        // Get the Reco attributes from the DLL and write them to registry
    hr = GetRecoAttributesHelper(s_hDllInstance, &recoAttr);
    if (FAILED(hr))
    {
        RegCloseKey(hkeyMyReco);
        return E_UNEXPECTED;
    }
    lRes = RegSetValueExW(hkeyMyReco, RECO_LANGUAGES_REGVALNAME, 0, REG_BINARY,
        (BYTE*)recoAttr.awLanguageId, 64 * sizeof(WORD));
    assert(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyMyReco);
        return E_UNEXPECTED;
    }
    lRes = RegSetValueExW(hkeyMyReco, RECO_CAPABILITIES_REGVALNAME, 0, REG_DWORD,
        (BYTE*)&(recoAttr.dwRecoCapabilityFlags), sizeof(DWORD));
    assert(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyMyReco);
        return E_UNEXPECTED;
    }
    RegCloseKey(hkeyMyReco);

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    LONG        lRes1 = 0;

    // Wipe out the registry information
    lRes1 = RegDeleteKeyW(HKEY_LOCAL_MACHINE, MY_RECO_REGKEY);

    // Delete our parent keys if they contain no remaining subkeys
    // (don't care if these fail)
    RegDeleteKeyW(HKEY_LOCAL_MACHINE, RECOGNIZER_REGPATH);
    RegDeleteKeyW(HKEY_LOCAL_MACHINE, TPG_REGPATH);

    if (lRes1 != ERROR_SUCCESS && lRes1 != ERROR_FILE_NOT_FOUND)
    {
        return E_UNEXPECTED;
    }
    return S_OK ;
}

static BOOL IsResourceMatchClsId(REFCLSID pCLSID, HINSTANCE hDll)
{
    // Make sure this GUID matches the one the DLL was built with.
    WCHAR               awcRecoGUID[ARRAYSIZE(MY_RECO_GUID)];
    LPOLESTR            pszCLSID = NULL;
    BOOL                bRet = FALSE;

    if (NULL == hDll)
    {
        return FALSE;
    }

    // convert the passed CLSID to a str
    if (!SUCCEEDED(StringFromCLSID(pCLSID, &pszCLSID)))
    {
        goto exit;
    }

    // load the recognizer's GUID string from resources and compare it with the passed argument
    if (!LoadStringW (hDll, RESID_MY_RECO_GUID, awcRecoGUID, ARRAYSIZE(awcRecoGUID)) )
    {
        goto exit;
    }

    if (0 == _wcsicmp (pszCLSID, awcRecoGUID))
    {
        // Clsid matches
        bRet = TRUE;
    }

exit:
    if (NULL != pszCLSID)
    {
        CoTaskMemFree (pszCLSID);
    }

    return bRet;
}


////////////////////////
// IRecognizer
////////////////////////
HRESULT WINAPI CreateRecognizer(CLSID *pCLSID, HRECOGNIZER *phrec)
{
    // Initialize a recognizer. Our example requires no special action here.
    // A more elaborate example might require loading of data to assist
    // with recognition.

    if( !pCLSID )
    {
        return E_POINTER;
    }

    if( !s_hDllInstance )
    {
        return E_FAIL;
    }

    if( !IsResourceMatchClsId((REFCLSID)*pCLSID, s_hDllInstance) )
    {
        return E_FAIL;
    }

    MY_RECOGNIZER *pRec = new MY_RECOGNIZER;
    if( !pRec )
    {
        return E_OUTOFMEMORY;
    }

    pRec->RecoGuid      = *pCLSID;
    *phrec = (HRECOGNIZER) pRec;
    return S_OK;
}


HRESULT WINAPI DestroyRecognizer(HRECOGNIZER hrec)
{
    if( !hrec )
    {
        return E_POINTER;
    }
    delete (MY_RECOGNIZER*) hrec;

    return S_OK;
}

static HRESULT GetRecoAttributesHelper(HINSTANCE hDllInstance, RECO_ATTRS* pRecoAttrs)
{

    HRSRC                   hrsrc = NULL;
    HGLOBAL                 hg = NULL;
    LPBYTE                  pv = NULL;
    DWORD                   dwRecoCapabilityFlags;
    WORD                    wLanguageCount;
    WORD                    iLang;

    if (!pRecoAttrs)
    {
        return E_POINTER;
    }

    if( !hDllInstance )
    {
        return E_POINTER;
    }

    // Initialize the Reco Attribute structure
    ZeroMemory(pRecoAttrs, sizeof(RECO_ATTRS));

    // Load the recognizer friendly name
    if (0 == LoadStringW(s_hDllInstance,                    // handle to resource module
        RESID_MY_FRIENDLYNAME,                              // resource identifier
        pRecoAttrs->awcFriendlyName,                        // resource buffer
        ARRAYSIZE(pRecoAttrs->awcFriendlyName)              // size of buffer
        ))
    {
        return E_FAIL;
    }
    // Load the recognizer vendor name
    if (0 == LoadStringW(s_hDllInstance,                    // handle to resource module
        RESID_MY_VENDORNAME,                                // resource identifier
        pRecoAttrs->awcVendorName,                          // resource buffer
        ARRAYSIZE(pRecoAttrs->awcVendorName)                // size of buffer
        ))
    {
        return E_FAIL;
    }

    // Load the resources
    hrsrc = FindResource(s_hDllInstance,     // module handle
        MAKEINTRESOURCE(RESID_MY_RECO_INFO), // resource name
        RT_RCDATA                            // resource type
        );
    if (NULL == hrsrc)
    {
        // The resource is not found!
        return E_FAIL;
    }
    hg = LoadResource(
        s_hDllInstance, // module handle
        hrsrc           // resource handle
        );
    if (NULL == hg)
    {
        return E_FAIL;
    }
    pv = (LPBYTE)LockResource(
        hg   // handle to resource
        );
    if (NULL == pv)
    {
        return E_FAIL;
    }

    dwRecoCapabilityFlags = *((DWORD*)pv);
    pv += sizeof(dwRecoCapabilityFlags);

    wLanguageCount = *((WORD*)pv);
    pv += sizeof(wLanguageCount);

    // Fill the reco attribute structure for this recognizer

    // Add the languages
    if( wLanguageCount > MAX_LANGUAGES-1 )
    {
        wLanguageCount = MAX_LANGUAGES-1;
    }
    for (iLang = 0; iLang < wLanguageCount; iLang++)
    {
        WORD    iLanguage, iSubLang;

        iLanguage       =   *((WORD *)pv);
        pv              +=  sizeof (iLang);

        iSubLang        =   *((WORD *)pv);
        pv              +=  sizeof (iSubLang);

        pRecoAttrs->awLanguageId[iLang] = MAKELANGID((USHORT)iLanguage, (USHORT)iSubLang);
    }

    // End the list with a NULL
    pRecoAttrs->awLanguageId[wLanguageCount] = 0;

    pRecoAttrs->dwRecoCapabilityFlags = dwRecoCapabilityFlags;
    return S_OK;
}


HRESULT WINAPI GetRecoAttributes(HRECOGNIZER hrec, RECO_ATTRS* pRecoAttrs)
{

    if (NULL == hrec)
    {
        return E_POINTER;
    }

    return GetRecoAttributesHelper( s_hDllInstance, pRecoAttrs);
}


HRESULT WINAPI CreateContext(HRECOGNIZER hrec, HRECOCONTEXT *phrc)
{
    if( !hrec || !phrc )
    {
        return E_POINTER;
    }

    MY_RECOCONTEXT *pRC = new MY_RECOCONTEXT;
    if( !pRC )
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory( pRC, sizeof(MY_RECOCONTEXT) );
    pRC->pRecognizer = (MY_RECOGNIZER*) hrec;

    *phrc = (HRECOCONTEXT) pRC;

    return S_OK;
}

static void DestroyStrokeList( MY_STROKE *pStrokeList )
{
    while( pStrokeList )
    {
        MY_STROKE *pStrokeToDelete = pStrokeList;
        pStrokeList = pStrokeList->pNext;
        if( pStrokeToDelete->aPoints )
        {
            delete [] pStrokeToDelete->aPoints;
        }
        delete pStrokeToDelete;
    }
}


HRESULT WINAPI DestroyContext(HRECOCONTEXT hrc)
{
    if( !hrc )
    {
        return E_POINTER;
    }

    MY_RECOCONTEXT    *pRC = (MY_RECOCONTEXT *) hrc;
    if( pRC->pGuide )
    {
        delete pRC->pGuide;
    }
    DestroyStrokeList( pRC->pStrokeList );
    if( pRC->pLattice ) // The lattice contains dynamic data to free
    {
        DestroyLattice( pRC->pLattice );
    }
    ZeroMemory(pRC, sizeof(MY_RECOCONTEXT));
    delete (MY_RECOCONTEXT *) hrc;
    return S_OK;
}

HRESULT WINAPI GetResultPropertyList(HRECOGNIZER hrec, ULONG* pPropertyCount, GUID*pPropertyGuid)
{
    UNREFERENCED_PARAMETER(hrec);
    UNREFERENCED_PARAMETER(pPropertyCount);
    UNREFERENCED_PARAMETER(pPropertyGuid);
    return E_NOTIMPL;
}
HRESULT WINAPI GetPreferredPacketDescription(HRECOGNIZER hrec, PACKET_DESCRIPTION* pPacketDescription)
{
    if ( !pPacketDescription )
    {
        return E_POINTER;
    }

    //
    // We can be called with pPacketProperties
    // equal to NULL, just to get the size of the buffers

    if (pPacketDescription->pPacketProperties)
    {
        // Set the packet size to the size of x and y
        pPacketDescription->cbPacketSize = 2 * sizeof(LONG);

        // We are only setting 2 properties (X and Y)
        if (pPacketDescription->cPacketProperties < 2)
        {
            return TPC_E_INSUFFICIENT_BUFFER;
        }
        pPacketDescription->cPacketProperties = 2;

        // We are not setting buttons
        pPacketDescription->cButtons = 0;

        // Make sure that the pPacketProperties is of a valid size
        if (!pPacketDescription->pPacketProperties )
        {
            return E_POINTER;
        }

        // Fill in pPacketProperties
        // Add the GUID_X
        pPacketDescription->pPacketProperties[0].guid = s_guid_x;
        pPacketDescription->pPacketProperties[0].PropertyMetrics = s_DefaultPropMetrics;

        // Add the GUID_Y
        pPacketDescription->pPacketProperties[1].guid = s_guid_y;
        pPacketDescription->pPacketProperties[1].PropertyMetrics = s_DefaultPropMetrics;
    }
    else
    {
        // Just fill in the PacketDescription structure leaving NULL
        // pointers for the pguidButtons and pPacketProperies

        // Set the packet size to the size of x and y
        pPacketDescription->cbPacketSize = 2 * sizeof(LONG);

        // We are only setting 2 properties (X and Y)
        pPacketDescription->cPacketProperties = 2;

        // We are not setting buttons
        pPacketDescription->cButtons = 0;

        // There are no guid buttons
        pPacketDescription->pguidButtons = NULL;
    }

    return S_OK;
}

HRESULT WINAPI GetUnicodeRanges(HRECOGNIZER hrec, ULONG *pcRanges, CHARACTER_RANGE *pcr)
{
    UNREFERENCED_PARAMETER(hrec);
    UNREFERENCED_PARAMETER(pcRanges);
    UNREFERENCED_PARAMETER(pcr);
    return E_NOTIMPL;
}

////////////////////////
// IRecoContext
////////////////////////


static int RealToInt( double dVal )
{
    return dVal<0.0 ? (int)(dVal-0.5) : (int)(dVal+0.5);
}

static void Transform(const XFORM *pXf, POINT * aPoints, ULONG cPoints)
{
    // Transform POINT array in place
    ULONG iPoint = 0;
    LONG xp = 0;

    if(NULL != pXf)
    {
        assert((cPoints == 0) || aPoints);
        for(iPoint = 0; iPoint < cPoints; ++iPoint)
        {
            xp =  RealToInt(aPoints[iPoint].x * pXf->eM11 + aPoints[iPoint].y * pXf->eM21 + pXf->eDx);
            aPoints[iPoint].y = RealToInt(aPoints[iPoint].x * pXf->eM12 + aPoints[iPoint].y * pXf->eM22 + pXf->eDy);
            aPoints[iPoint].x = xp;
        }
    }
}

HRESULT WINAPI AddStroke(HRECOCONTEXT hrc,
          const PACKET_DESCRIPTION *pPacketDesc,
          ULONG cbPacket,                                       // I: Size of packet array (in BYTEs)
          const BYTE *pPacket,                                  // I: Array of packets
          const XFORM *pXForm)                                  // I: Transform to apply to each point
{
    // Add a copy of the given stroke points to the reco context.

    ULONG ulPacketSize;        // Size of one packet (in LONGs)
    ULONG cPoints;            // number of points in the stroke

    MY_RECOCONTEXT    *pRC = (MY_RECOCONTEXT *) hrc;

    if (NULL == pRC )
    {
        return E_POINTER;
    }
    if ( !pPacket || !cbPacket )
    {
        return E_POINTER;
    }

    if( MAX_STROKES_IN_CONTEXT <= pRC->cStrokes )
    {
        return E_FAIL;
    }

    //
    // Get the number of packets (== number of points)

    if (pPacketDesc)
    {
        if ( !pPacketDesc->cbPacketSize )
        {
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
        if (pPacketDesc->cbPacketSize % sizeof(LONG) != 0)
        {
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
        if (cbPacket % pPacketDesc->cbPacketSize != 0)
        {
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
        ulPacketSize = pPacketDesc->cbPacketSize / sizeof(LONG);
        cPoints = cbPacket / pPacketDesc->cbPacketSize;
        if (ulPacketSize < 2)  // Need 2+ values in a packet; i.e. X and Y
        {
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
    }
    else  // Preferred packet description
    {
        if (cbPacket % (2 * sizeof(LONG)) != 0)
        {
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
        ulPacketSize = 2;
        cPoints = cbPacket / (2 * sizeof(LONG));
    }

    if (cPoints == 0)  // Don't add strokes with 0 points
    {
        return E_FAIL;
    }

    if( MAX_POINTS_IN_STROKE < cPoints )
    {
        return E_FAIL;
    }

    //
    // Find the index (offset) in packet of GUID_X and GUID_Y

    ULONG ulXIndex, ulYIndex;
    if (pPacketDesc)
    {
        if (pPacketDesc->cPacketProperties < 2 ||      // Need 2+ properties
            NULL == pPacketDesc->pPacketProperties)     // Corrupted structure
        {
            assert(0);      // This should never happen!
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
        BOOL bXFound = FALSE;
        BOOL bYFound = FALSE;
        for (ULONG ulIndex = 0; ulIndex < pPacketDesc->cPacketProperties; ulIndex++)
        {
            if (IsEqualGUID(pPacketDesc->pPacketProperties[ulIndex].guid, s_guid_x))
            {
                bXFound = TRUE;
                ulXIndex = ulIndex;
            }
            else if (IsEqualGUID(pPacketDesc->pPacketProperties[ulIndex].guid, s_guid_y))
            {
                bYFound = TRUE;
                ulYIndex = ulIndex;
            }
            if (bXFound && bYFound)
                break;
        }
        if (!bXFound || !bYFound)               // X- or Y-coordinates are
        {                                       // not part of the packet!
            return TPC_E_INVALID_PACKET_DESCRIPTION;
        }
    }
    else    // Preferred packet description
    {
        ulXIndex = 0;
        ulYIndex = 1;
    }

        // Get ready to copy the ink points to MY_STROKE struct
    MY_STROKE *pStroke = new MY_STROKE;
    if( !pStroke )
    {
        goto OutOfMemory;
    }

    pStroke->aPoints = new POINT[cPoints];
    if( !pStroke->aPoints )
    {
        goto OutOfMemory;
    }

        // Copy the points
    POINT *pPoint = pStroke->aPoints;
    const LONG  *pLongs = (const LONG *)pPacket;
    for( ULONG iPoint=0; iPoint<cPoints; iPoint++ )
    {
        pPoint->x = *(pLongs+ulXIndex);
        pPoint->y = *(pLongs+ulYIndex);
        pPoint++;
        pLongs += ulPacketSize;
    }
    pStroke->cPoints = cPoints;

        // Transform points from Tablet space to ink space
    Transform(pXForm, pStroke->aPoints, cPoints);

        // Insert the new stroke at front of the list
    pStroke->pNext = pRC->pStrokeList;
    pRC->pStrokeList = pStroke;

    pRC->cStrokes++;              // Update number of strokes

        // Update number of points in all strokes.
        // (We track this for our example. It is not essential.)
    pRC->cPointsTot += cPoints;

    return S_OK;

OutOfMemory:
    if( pStroke )
    {
        if( pStroke->aPoints )
        {
            delete [] pStroke->aPoints;
        }
        delete pStroke;
    }
    return E_OUTOFMEMORY;
}


HRESULT WINAPI GetBestResultString(HRECOCONTEXT hrc, __inout ULONG *pcTCH, __out_ecount_opt(*pcTCH) PTCH* pwcBestResult)
{
    // Return the recognition result string that was computed in Process().
    // Note: pwcBestResult does not include a NULL-terminator, nor does the
    // buffer length count a NULL-terminator.

    HRESULT hr = S_OK;
    if( !hrc || !pcTCH )
    {
        return E_POINTER;
    }
    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;

    if( !pRC->wzBestResult[0] ) // No reco result available
    {
        return TPC_E_NOT_RELEVANT;
    }

    ULONG cWChar = wcsnlen( pRC->wzBestResult, ARRAYSIZE(pRC->wzBestResult) );
    if( !pwcBestResult )  // Caller is querying for buffer size
    {
        *pcTCH = cWChar;
        return S_OK;
    }
    memcpy_s( pwcBestResult, *pcTCH*sizeof(WCHAR), pRC->wzBestResult, cWChar*sizeof(WCHAR) );
    if( cWChar > *pcTCH )
    {
        hr = TPC_S_TRUNCATED;
    }
    *pcTCH = cWChar;
    return hr;
}

HRESULT WINAPI DestroyAlternate(HRECOALT hrcalt)
{
    UNREFERENCED_PARAMETER(hrcalt);
    return E_NOTIMPL;
}

HRESULT WINAPI SetGuide(HRECOCONTEXT hrc, const RECO_GUIDE* pGuide, ULONG iIndex)
{
    if( !hrc )
    {
        return E_POINTER;
    }
    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;

    if( pGuide )  // pGuide not NULL means lined or boxed mode
    {
        if ((pGuide->cHorzBox < 0 || pGuide->cVertBox < 0) ||    // invalid
            (pGuide->cHorzBox > 0 && pGuide->cVertBox == 0))     // vertical lined mode
        {
            return E_INVALIDARG;
        }
    }

    // If there is no guide already present, allocate one
    if (!pRC->pGuide)
    {
        pRC->pGuide = new RECO_GUIDE;
    }
    if (!pRC->pGuide)
    {
        return E_OUTOFMEMORY;
    }

    if (pGuide)
        *(pRC->pGuide) = *pGuide;
    else
    {
        ZeroMemory(pRC->pGuide, sizeof(RECO_GUIDE));  // free mode
    }
    pRC->uiGuideIndex = iIndex;

    return S_OK;
}

HRESULT WINAPI GetGuide(HRECOCONTEXT hrc, RECO_GUIDE* pGuide, ULONG *piIndex)
{
    if( !hrc || !pGuide || !piIndex )
    {
        return E_POINTER;
    }
    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;

    if( !pRC->pGuide )
    {
        return S_FALSE;
    }

    *pGuide = *(pRC->pGuide);
    *piIndex = pRC->uiGuideIndex;

    return S_OK;
}


HRESULT WINAPI AdviseInkChange(HRECOCONTEXT hrc, BOOL bNewStroke)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(bNewStroke);
    return S_OK;
}
HRESULT WINAPI SetCACMode(HRECOCONTEXT hrc, int iMode)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(iMode);
    return E_NOTIMPL;
}
HRESULT WINAPI CloneContext(HRECOCONTEXT hrc, HRECOCONTEXT* pCloneHrc)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pCloneHrc);
    return E_NOTIMPL;
}

HRESULT WINAPI ResetContext(HRECOCONTEXT hrc)
{
    // Deletes the current ink and recognition results from the context.
    if( !hrc )
    {
        return E_POINTER;
    }
    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;
    DestroyStrokeList( pRC->pStrokeList ); pRC->pStrokeList = NULL;
    pRC->cStrokes = 0;
    pRC->cPointsTot = 0;
    pRC->wzBestResult[0] = 0;
    if( pRC->pLattice )
    {
        DestroyLattice( pRC->pLattice ); pRC->pLattice = NULL;
    }
    return S_OK;
}


HRESULT WINAPI Process(HRECOCONTEXT hrc, BOOL *pbPartialProcessing)
{
    // Recognize the strokes in the given context.
    // We don't actually recognize anything. Instead we illustrate how
    // to process the ink and guide information so that you can provide
    // your own recognition algorithm.

    int iRet = S_OK;
    BYTE *abIsBoxDirty = NULL;

    if( !hrc || !pbPartialProcessing )
    {
        return E_POINTER;
    }
    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;

    if( !pRC->cStrokes )
    {
        iRet = S_OK;
        goto Exit;
    }
    if( !pRC->pStrokeList )
    {
        iRet = E_FAIL;
        goto Exit;
    }

    HRESULT hr = StringCchPrintfW( pRC->wzBestResult, ARRAYSIZE(pRC->wzBestResult), L"Strokes=%d  Points=%d",
        pRC->cStrokes, pRC->cPointsTot );
    if( FAILED(hr) )
    {
        iRet = E_FAIL;
        goto Exit;
    }

        // Find number of rows and columns in guide.
        // Determine if lined mode.
    ULONG cRow = 0;
    ULONG cCol = 0;
    bool bIsLined = false;
    if( pRC->pGuide )
    {
        cRow = pRC->pGuide->cVertBox;
        cCol = pRC->pGuide->cHorzBox;
        if( cRow ) // lined or boxed mode
        {
            if( !cCol ) // lined mode
            {
                cCol = 1;
                bIsLined = true;
            }
        }
    }

    if( !cRow ) // free mode.
    {
        goto Exit;
    }

        // At this point we are in either lined mode or boxed mode but
        // not free mode. The remainder of this routine enumerates the
        // lines or boxes containing ink.

        // Allocate 2D array of booleans to keep track of which boxes contain ink points.
        // For lined mode, the array has a single column.
    if( ULONG_MAX / cRow < cCol )
    {
        assert(0 && "Overflow");
        iRet = E_FAIL;
        goto Exit;
    }
    abIsBoxDirty = new BYTE[cRow*cCol];
    if( !abIsBoxDirty )
    {
        iRet = E_FAIL;
        goto Exit;
    }
    ZeroMemory( abIsBoxDirty, cRow*cCol*sizeof(BYTE) );

        // Scan through the ink to mark boxes containing ink points
    MY_STROKE *pStroke = pRC->pStrokeList;
    for( ;pStroke; pStroke = pStroke->pNext )
    {
        POINT *pPoint = pStroke->aPoints;
        for( ULONG iPoint=0; iPoint < pStroke->cPoints; iPoint++, pPoint++ )
        {
            ULONG iRow = (pPoint->y - pRC->pGuide->yOrigin) / pRC->pGuide->cyBox;
            ULONG iCol = bIsLined ? 0 : (pPoint->x - pRC->pGuide->xOrigin) / pRC->pGuide->cxBox;
            if( iRow >= cRow || iCol >= cCol )
            {
                assert(0 && "Buffer overrun");
                iRet = E_FAIL;
                goto Exit;
            }
            abIsBoxDirty[iRow*cCol + iCol] = 1;
        }
    }

        // Prepare to concatenate line/box numbers to the result
    hr = StringCchCatW(
        pRC->wzBestResult,
        ARRAYSIZE(pRC->wzBestResult),
        bIsLined ? L"  Lines used=" : L"  Boxes used=" );
    if( FAILED(hr) )
    {
        iRet = E_FAIL;
        goto Exit;
    }

        // Concatenate the numbers iteratively to the end of the result string.
        // For simplicity, we opt for a less efficient implementation which
        // scans for the end of the result string for each concatenation.

    bool bIsTruncated = false;  // true if result string runs out of space
    for( ULONG iRow=0; iRow < cRow && !bIsTruncated; iRow++ )
    {
        for( ULONG iCol=0; iCol < cCol && !bIsTruncated; iCol++ )
        {
            if( abIsBoxDirty[iRow*cCol + iCol] )
            {
                    // Get the next item to concatenate
                WCHAR wzTmp[20];
                if( bIsLined )
                {
                    hr = StringCchPrintfW( wzTmp, ARRAYSIZE(wzTmp), L"%d ", iRow );
                    if( FAILED(hr) )
                    {
                        iRet = E_FAIL;
                        goto Exit;
                    }
                }
                else
                {
                    hr = StringCchPrintfW( wzTmp, ARRAYSIZE(wzTmp), L"(%d,%d)", iRow, iCol );
                    if( FAILED(hr) )
                    {
                        iRet = E_FAIL;
                        goto Exit;
                    }
                }
                    // Concatenate the item to the end of the result
                hr = StringCchCatW( pRC->wzBestResult, ARRAYSIZE(pRC->wzBestResult), wzTmp );
                if( FAILED(hr) )
                {
                        // overwrite tail of wzBestResult with "<truncated>"
                    const WCHAR wzTrunc[] = L"<truncated>";
                    const size_t len = ARRAYSIZE(wzTrunc);
                    hr = StringCchCopyW(
                        pRC->wzBestResult+ARRAYSIZE(pRC->wzBestResult)-len,
                        len,
                        wzTrunc);
                    if( FAILED(hr) )
                    {
                        iRet = E_FAIL;
                        goto Exit;
                    }
                    bIsTruncated = true;
                }
            }
        }
    }
Exit:
    *pbPartialProcessing = FALSE; // No more ink to process
    if( abIsBoxDirty )
    {
        delete [] abIsBoxDirty;
    }
    return iRet;
}

HRESULT WINAPI SetFactoid(HRECOCONTEXT hrc, ULONG cwcFactoid, const WCHAR *pwcFactoid)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(cwcFactoid);
    UNREFERENCED_PARAMETER(pwcFactoid);
    return S_OK; // ignore factoid
}
HRESULT WINAPI SetFlags(HRECOCONTEXT hrc, DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(dwFlags);
    return S_OK; // ignore flags
}

static void DestroyLattice( RECO_LATTICE *pLattice )
{
    if( !pLattice )
    {
        return;
    }

    if( pLattice->pLatticeColumns )
    {
        if( pLattice->pLatticeColumns->pStrokes )
        {
            delete [] pLattice->pLatticeColumns->pStrokes;
        }
        if( pLattice->pLatticeColumns->pLatticeElements )
        {
            delete [] pLattice->pLatticeColumns->pLatticeElements;
        }
        delete [] pLattice->pLatticeColumns;
    }
    if( pLattice->pulBestResultColumns )
    {
        delete [] pLattice->pulBestResultColumns;
    }
    if( pLattice->pulBestResultIndexes )
    {
        delete [] pLattice->pulBestResultIndexes;
    }
    delete pLattice;
}



HRESULT WINAPI GetLatticePtr(HRECOCONTEXT hrc, RECO_LATTICE **ppLattice)
{
    // We trivialize the lattice by providing just
    // one segmentation using all the strokes, and just one
    // recognition result with no alternates.

    if( !hrc )
    {
        return E_POINTER;
    }
    if( !ppLattice )
    {
        return E_POINTER;
    }

    MY_RECOCONTEXT *pRC = (MY_RECOCONTEXT*) hrc;
    if( !pRC->wzBestResult[0] ) // No reco result available
    {
        return TPC_E_NOT_RELEVANT;
    }

    if( pRC->pLattice ) // free existing lattice data
    {
        DestroyLattice( pRC->pLattice ); pRC->pLattice = NULL;
    }

        // RECO_LATTICE initialization
    RECO_LATTICE *pLattice = pRC->pLattice = new RECO_LATTICE;
    if( !pLattice )
    {
        return E_OUTOFMEMORY;
    }
    ZeroMemory( pLattice, sizeof(RECO_LATTICE) );
    pLattice->ulColumnCount = 1;
    pLattice->pLatticeColumns = new RECO_LATTICE_COLUMN[pLattice->ulColumnCount];
    if( !pLattice->pLatticeColumns )
    {
        goto OutOfMemory;
    }
    ZeroMemory( pLattice->pLatticeColumns, sizeof(RECO_LATTICE_COLUMN) );
    pLattice->ulPropertyCount = 0;
    pLattice->pGuidProperties = NULL;
    pLattice->ulBestResultColumnCount = 1;
    pLattice->pulBestResultColumns = new ULONG[pLattice->ulBestResultColumnCount];
    if( !pLattice->pulBestResultColumns )
    {
        goto OutOfMemory;
    }
    pLattice->pulBestResultColumns[0] = 0;
    pLattice->pulBestResultIndexes = new ULONG[1];
    if( !pLattice->pulBestResultIndexes )
    {
        goto OutOfMemory;
    }
    pLattice->pulBestResultIndexes[0] = 0;

        // RECO_LATTICE_COLUMN initialization
        // We have just one Lattice Column to initialize
    RECO_LATTICE_COLUMN *pLatticeColumn = pLattice->pLatticeColumns;
    pLatticeColumn->key = 0;
    pLatticeColumn->cpProp.cProperties = 0;
    pLatticeColumn->cpProp.apProps = NULL;
        // fake the stroke mapping:
    pLatticeColumn->cStrokes = pRC->cStrokes;
    pLatticeColumn->pStrokes = new ULONG[pRC->cStrokes];
    if( !pLatticeColumn->pStrokes )
    {
        goto OutOfMemory;
    }
    for( ULONG ii=0; ii<pRC->cStrokes; ii++ )
    {
        pLatticeColumn->pStrokes[ii] = ii;
    }
    pLatticeColumn->cLatticeElements = 1;
    pLatticeColumn->pLatticeElements = new RECO_LATTICE_ELEMENT[pLatticeColumn->cLatticeElements];
    if( !pLatticeColumn->pLatticeElements )
    {
        goto OutOfMemory;
    }

        // RECO_LATTICE_ELEMENT initialization
        // We have just one Column Element to initialize
    RECO_LATTICE_ELEMENT *pLatticeElement = pLatticeColumn->pLatticeElements;
    pLatticeElement->score = 0;
    pLatticeElement->type = RECO_TYPE_WSTRING;
    pLatticeElement->pData = (BYTE*) pRC->wzBestResult;
    pLatticeElement->ulNextColumn = 1;
    pLatticeElement->ulStrokeNumber = pRC->cStrokes;
    pLatticeElement->epProp.cProperties = 0;
    pLatticeElement->epProp.apProps = NULL;

    *ppLattice = pLattice;
    return S_OK;

    OutOfMemory:
    if( pLattice->pLatticeColumns )
    {
        if( pLattice->pLatticeColumns->pStrokes )
        {
            delete [] pLattice->pLatticeColumns->pStrokes;
        }
        if( pLattice->pLatticeColumns->pLatticeElements )
        {
            delete [] pLattice->pLatticeColumns->pLatticeElements;
        }
        delete [] pLattice->pLatticeColumns;
    }
    if( pLattice->pulBestResultColumns )
    {
        delete [] pLattice->pulBestResultColumns;
    }
    if( pLattice->pulBestResultIndexes )
    {
        delete [] pLattice->pulBestResultIndexes;
    }
    if( pLattice )
    {
        delete pLattice;
    }
    return E_OUTOFMEMORY;
}

HRESULT WINAPI SetTextContext(HRECOCONTEXT hrc, ULONG cwcBefore, const WCHAR *pwcBefore, ULONG cwcAfter, const WCHAR *pwcAfter)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(cwcBefore);
    UNREFERENCED_PARAMETER(pwcBefore);
    UNREFERENCED_PARAMETER(cwcAfter);
    UNREFERENCED_PARAMETER(pwcAfter);
    return E_NOTIMPL;
}
HRESULT WINAPI GetEnabledUnicodeRanges(HRECOCONTEXT hrc, ULONG *pcRanges, CHARACTER_RANGE *pcr)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pcRanges);
    UNREFERENCED_PARAMETER(pcr);
    return E_NOTIMPL;
}
HRESULT WINAPI SetEnabledUnicodeRanges(HRECOCONTEXT hrc, ULONG cRanges, CHARACTER_RANGE *pcr)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(cRanges);
    UNREFERENCED_PARAMETER(pcr);
    return E_NOTIMPL;
}
HRESULT WINAPI GetContextPropertyList(HRECOCONTEXT hrc, ULONG *pcProperties, GUID *pPropertyGUIDS)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pcProperties);
    UNREFERENCED_PARAMETER(pPropertyGUIDS);
    return E_NOTIMPL;
}
HRESULT WINAPI GetContextPropertyValue(HRECOCONTEXT hrc, GUID *pGuid, ULONG *pcbSize, BYTE *pProperty)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pGuid);
    UNREFERENCED_PARAMETER(pcbSize);
    UNREFERENCED_PARAMETER(pProperty);
    return E_NOTIMPL;
}
HRESULT WINAPI SetContextPropertyValue(HRECOCONTEXT hrc, GUID *pGuid, ULONG cbSize, BYTE *pProperty)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pGuid);
    UNREFERENCED_PARAMETER(cbSize);
    UNREFERENCED_PARAMETER(pProperty);
    return E_NOTIMPL;
}
HRESULT WINAPI IsStringSupported(HRECOCONTEXT hrc, ULONG wcString, const WCHAR *pwcString)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(wcString);
    UNREFERENCED_PARAMETER(pwcString);
    return E_NOTIMPL;
}
HRESULT WINAPI SetWordList(HRECOCONTEXT hrc, HRECOWORDLIST hwl)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(hwl);
    return E_NOTIMPL;
}
HRESULT WINAPI GetContextPreferenceFlags(HRECOCONTEXT hrc, DWORD *pdwContextPreferenceFlags)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pdwContextPreferenceFlags);
    return E_NOTIMPL;
}
HRESULT WINAPI GetRightSeparator(HRECOCONTEXT hrc, __inout ULONG *pcSize, __out_ecount(*pcSize) OPTIONAL WCHAR* pwcRightSeparator)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pcSize);
    UNREFERENCED_PARAMETER(pwcRightSeparator);
    return E_NOTIMPL;
}
HRESULT WINAPI GetLeftSeparator(HRECOCONTEXT hrc, __inout ULONG *pcSize, __out_ecount(*pcSize) OPTIONAL WCHAR* pwcLeftSeparator)
{
    UNREFERENCED_PARAMETER(hrc);
    UNREFERENCED_PARAMETER(pcSize);
    UNREFERENCED_PARAMETER(pwcLeftSeparator);
    return E_NOTIMPL;
}

////////////////////////
// IRecoWordList
////////////////////////
HRESULT WINAPI DestroyWordList(HRECOWORDLIST hwl)
{
    UNREFERENCED_PARAMETER(hwl);
    return E_NOTIMPL;
}
HRESULT WINAPI AddWordsToWordList(HRECOWORDLIST hwl, __in WCHAR *pwcWords)
{
    UNREFERENCED_PARAMETER(hwl);
    UNREFERENCED_PARAMETER(pwcWords);
    return E_NOTIMPL;
}
HRESULT WINAPI MakeWordList(HRECOGNIZER hrec, __in WCHAR *pBuffer, HRECOWORDLIST *phwl)
{
    UNREFERENCED_PARAMETER(hrec);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(phwl);
    return E_NOTIMPL;
}
