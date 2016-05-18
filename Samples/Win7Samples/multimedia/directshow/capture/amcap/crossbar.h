//------------------------------------------------------------------------------
// File: Crossbar.h
//
// Desc: DirectShow sample code - definition of class for controlling
//       video crossbars.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __Crossbar_h__
#define __Crossbar_h__


//------------------------------------------------------------------------------
// Name: class CClass
// Desc: This class contains routing information for the capture data
//------------------------------------------------------------------------------
class CRouting {
public:
    class CRouting       *pLeftRouting;
    class CRouting       *pRightRouting;
    LONG                 VideoInputIndex;
    LONG                 VideoOutputIndex;
    LONG                 AudioInputIndex;
    LONG                 AudioOutputIndex;
    IAMCrossbar         *pXbar;
    LONG                 InputPhysicalType;
    LONG                 OutputPhysicalType;
    LONG                 Depth;

    CRouting () {};
    ~CRouting () {};
};

typedef CGenericList<CRouting> CRoutingList;




//------------------------------------------------------------------------------
// Name: class CCrossbar
// Desc: The actual helper class for Crossbars
//------------------------------------------------------------------------------
class CCrossbar
{
private:
    IPin                    *m_pStartingPin;
    CRouting                 m_RoutingRoot;
    CRoutingList            *m_RoutingList;
    int                      m_CurrentRoutingIndex;

    HRESULT BuildRoutingList (
                IPin     *pStartingInputPin,
                CRouting *pCRouting,
                int       Depth);
    HRESULT SaveRouting (CRouting *pRoutingNew);
    HRESULT DestroyRoutingList();
    BOOL    StringFromPinType (TCHAR *pc, int nSize, long lType);
    
    HRESULT GetCrossbarIPinAtIndex(
                IAMCrossbar *pXbar,
                LONG PinIndex,
                BOOL IsInputPin,
                IPin ** ppPin);
    HRESULT GetCrossbarIndexFromIPin (
                IAMCrossbar * pXbar,
                LONG * PinIndex,
                BOOL IsInputPin,
                IPin * pPin);

public:

    CCrossbar (IPin *pPin, HRESULT *phr);
    ~CCrossbar();

    HRESULT GetInputCount (LONG *pCount);
    HRESULT GetInputType  (LONG Index, LONG * PhysicalType);
    HRESULT GetInputName  (LONG Index, TCHAR * pName, LONG NameSize);
    HRESULT SetInputIndex (LONG Index);
    HRESULT GetInputIndex (LONG *Index);

};

#endif  // __Crossbar_h__
