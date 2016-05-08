// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CManipulationEventSink.cpp
//
// Implementation of CManipulationEventSink class.

// Windows header files
#include <windows.h>

// C RunTime header files
#include <assert.h>
#define ASSERT assert

// Application specific header files
#include "CManipulationEventSink.h"

// Constructor.
// Constructs raw, unconnected CManipulationEventSink object.
// in:
//      pcDrawingObject - pointer to the CDrawingObject class (the rectangle)
CManipulationEventSink::CManipulationEventSink(CDrawingObject* pcDrawingObject)
:   m_cRefCount(1),
    m_pConnection(NULL),
    m_dwCookie(0),
    m_pcDrawingObject(pcDrawingObject)
{
    ASSERT((pcDrawingObject != NULL) && L"CManipulationEventSink constructor: incorrect argument");
}

// Connects CManipulationEventSink object to ManipulationProcessor.
// in:
//      pManipulationProcessor - pointer to the IManipulationProcessor to connect to
// returns:
//      success status, true if the connection is established, false on failure
bool CManipulationEventSink::Connect(IManipulationProcessor* pManipulationProcessor)
{
    // Check input arguments
    if (pManipulationProcessor == NULL)
    {
        ASSERT((pManipulationProcessor != NULL) && L"CManipulationEventSink::Create : incorrect arguments");
        return false;
    }

    // Check object state
    if ((m_dwCookie != 0) || (m_pConnection != NULL))
    {
        ASSERT((m_dwCookie == 0) && (m_pConnection == NULL) && L"CManipulationEventSink::Connect : connection already established");
        return false;
    }

    // Get the container with the connection points.
    IConnectionPointContainer* pConnectionContainer = NULL;
    HRESULT hr = pManipulationProcessor->QueryInterface(&pConnectionContainer);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CManipulationEventSink::Connect : failed to get the container with the connection points");
        return false;
    }

    // Get a connection point.
    hr = pConnectionContainer->FindConnectionPoint(__uuidof(_IManipulationEvents), &m_pConnection);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CManipulationEventSink::Connect : failed to get a connection point");
        pConnectionContainer->Release();
        return false;
    }

    // Release the connection container.
    pConnectionContainer->Release();

    // Advise. Establishes an advisory connection between the connection point and the 
    // caller's sink object. 
    hr = m_pConnection->Advise(this, &m_dwCookie);
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"CManipulationEventSink::Connect : failed to Advise");
        m_pConnection->Release();
        m_pConnection = NULL;
        return false;
    }

    return true;
}

// Disconnects CManipulationEventSink object from ManipulationProcessor.
// returns:
//      success status, true if the connection is terminated, false on failure
bool CManipulationEventSink::Disconnect()
{
    // Check object state
    if ((m_dwCookie == 0) || (m_pConnection == NULL))
    {
        ASSERT((m_dwCookie != 0) && (m_pConnection != NULL) && L"CManipulationEventSink::Disconnect : connection does not exist");
        return false;
    }

    // Unadvise. Terminate the connection.
    HRESULT hr = m_pConnection->Unadvise(m_dwCookie);
    ASSERT(SUCCEEDED(hr) && L"CManipulationEventSink::Disconnect : failed to Unadvise");
    UNREFERENCED_PARAMETER(hr);

    m_pConnection->Release();
    m_pConnection = NULL;
    m_dwCookie = 0;

    return true;
}

// Destructor
CManipulationEventSink::~CManipulationEventSink()
{
    ASSERT((m_dwCookie == 0) && (m_pConnection == NULL) && L"CManipulationEventSink destructor : connection is not properly terminated");
}

// IManipulationEvents implementation

// This event is called by the ManipulationProcessor when manipulation 
// is detected (starts). Not used by this application.
// in:
//     x - x coordiante of the initial point of manipulation 
//         (1/100 of pixel)
//     y - y coordiante of the initial point of manipulation 
//         (1/100 of pixel)
HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationStarted(
    FLOAT /* x */,
    FLOAT /* y */)
{
    return S_OK;
}

// This event is called by the ManipulationProcessor during the movement of 
// the fingers.
// in:
//                      x - x coordiante of the initial point of manipulation 
//                          (1/100 of pixel)
//                      y - y coordiante of the initial point of manipulation 
//                          (1/100 of pixel)
//      translationDeltaX - shift of the x-coordinate (1/100 of pixel)
//      translationDeltaY - shift of the y-coordinate (1/100 of pixel)
//             scaleDelta - scale factor (zoom in/out)
//         expansionDelta - the current rate of scale change
//          rotationDelta - rotation angle in radians
// cumulativeTranslationX - cumulative shift of x-coordinate (1/100 of pixel)
// cumulativeTranslationY - cumulative shift of y-coordinate (1/100 of pixel)
//        cumulativeScale - cumulative scale factor (zoom in/out)
//    cumulativeExpansion - cumulative rate of scale change
//     cumulativeRotation - cumulative rotation angle in radians
HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationDelta(
    FLOAT /* x */,
    FLOAT /* y */,
    FLOAT translationDeltaX,
    FLOAT translationDeltaY,
    FLOAT scaleDelta,
    FLOAT /* expansionDelta */,
    FLOAT rotationDelta,
    FLOAT /* cumulativeTranslationX */,
    FLOAT /* cumulativeTranslationY */,
    FLOAT /* cumulativeScale */,
    FLOAT /* cumulativeExpansion */,
    FLOAT /* cumulativeRotation */)
{
    m_pcDrawingObject->ApplyManipulationDelta(translationDeltaX, translationDeltaY, scaleDelta, rotationDelta);

    return S_OK;
}

// This event is called by the ManipulationProcessor when manipulation is 
// completed. Not used by this application. 
// in:
//                      x - x coordinate of the initial point of manipulation 
//                          (1/100 of pixel)
//                      y - y coordinate of the initial point of manipulation 
//                          (1/100 of pixel)
// cumulativeTranslationX - cumulative shift of x-coordinate (1/100 of pixel)
// cumulativeTranslationY - cumulative shift of y-coordinate (1/100 of pixel)
//        cumulativeScale - cumulative scale factor (zoom in/out)
//    cumulativeExpansion - cumulative rate of scale change
//     cumulativeRotation - cumulative rotation angle in radians
HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationCompleted(
    FLOAT /* x */,
    FLOAT /* y */,
    FLOAT /* cumulativeTranslationX */,
    FLOAT /* cumulativeTranslationY */,
    FLOAT /* cumulativeScale */,
    FLOAT /* cumulativeExpansion */,
    FLOAT /* cumulativeRotation */)
{
    return S_OK;
}

// IUnknown implementation

// The IUnknown interface lets clients get pointers to other interfaces on a 
// given object through the QueryInterface method, and manage the existence of
// the object through the IUnknown::AddRef and IUnknown::Release methods. All 
// other COM interfaces are inherited, directly or indirectly, from IUnknown. 
// Therefore, the three methods in IUnknown are the first entries in the VTable 
// for every interface. 

// The IUnknown::AddRef method increments the reference count for an interface 
// on an object. It should be called for every new copy of a pointer to an 
// interface on a given object. 
ULONG CManipulationEventSink::AddRef(void)
{
    return InterlockedIncrement(&m_cRefCount);
}

// Decrements the reference count for the calling interface on a object. 
// If the reference count on the object falls to 0, the object is freed 
// from memory.
ULONG CManipulationEventSink::Release(void)
{
    ULONG cNewRefCount = InterlockedDecrement(&m_cRefCount);
    if (cNewRefCount == 0)
    {
        delete this;
    }
    return cNewRefCount;
}

// Returns a pointer to a specified interface on an object to which a client 
// currently holds an interface pointer. This function must call IUnknown::AddRef 
// on the pointer it returns. 
// in: 
//    riid - Identifier of the interface being requested.
// out:
//  ppvObj - Address of pointer variable that receives the interface pointer 
//           requested in riid. Upon successful return, *ppvObject contains the 
//           requested interface pointer to the object. If the object does not 
//           support the interface specified in iid, *ppvObject is set to NULL.
HRESULT CManipulationEventSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if ((riid == __uuidof(_IManipulationEvents)) || (riid == IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}
