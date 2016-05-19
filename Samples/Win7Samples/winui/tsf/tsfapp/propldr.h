/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          PropLdr.h

   Description:   CTSFPersistentPropertyLoader definition

**************************************************************************/

#ifndef PROPLOADER_H
#define PROPLOADER_H

/**************************************************************************
	#include statements
**************************************************************************/

#include <windows.h>
#include <msctf.h>

/**************************************************************************
	class definitions
**************************************************************************/

/**************************************************************************

	CTSFPersistentPropertyLoader

**************************************************************************/

class CTSFPersistentPropertyLoader : public ITfPersistentPropertyLoaderACP
{
public:
    CTSFPersistentPropertyLoader(TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream);
    ~CTSFPersistentPropertyLoader();

    // IUnknown methods
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

    // ITfPersistentPropertyLoaderACP methods
    STDMETHODIMP LoadProperty(const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream **ppStream);


private:
    int m_ObjRefCount;
    TF_PERSISTENT_PROPERTY_HEADER_ACP m_hdr;
    BYTE *m_pb;
};


#endif //PROPLOADER_H
