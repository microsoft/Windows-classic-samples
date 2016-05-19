/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2002 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          ADEvent.h

   Description:   

**************************************************************************/

#ifndef ADEVENT_H
#define ADEVENT_H

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <iads.h>
#include <dsadmin.h>

/**************************************************************************

   CClassFactory class definition

**************************************************************************/

class CClassFactory : public IClassFactory
{
protected:
    DWORD m_ObjRefCount;

public:
    CClassFactory();
    ~CClassFactory();

    //IUnknown methods
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(DWORD) AddRef();
    STDMETHODIMP_(DWORD) Release();

    //IClassFactory methods
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};

/**************************************************************************

   CAdminNotifyHandler class definition

**************************************************************************/

class CAdminNotifyHandler : public IDsAdminNotifyHandler
{
protected:
    DWORD       m_ObjRefCount;
    IDataObject *m_pdoFrom;
    IDataObject *m_pdoTo;
   
public:
    CAdminNotifyHandler();
    ~CAdminNotifyHandler();

    //IUnknown methods
    STDMETHOD(QueryInterface)(REFIID, LPVOID FAR *);
    STDMETHOD_(DWORD, AddRef)();
    STDMETHOD_(DWORD, Release)();

    //IDsAdminNotifyHandler methods
    STDMETHOD(Initialize)(IDataObject* pExtraInfo, ULONG* puEventFlags);
    STDMETHOD(Begin)(ULONG uEvent, IDataObject* pArg1, IDataObject* pArg2, ULONG* puFlags, BSTR* pBstr);
    STDMETHOD(Notify)(ULONG nItem, ULONG uFlags); 
    STDMETHOD(End)(void); 


private:
};

#endif   //ADEVENT_H
