// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// template class that encapsulates a local server class factory to be declared on the stack
// that will factory an already existing object provided to the constructor
// usually that object is the application or a sub object within the
// application.
//
// class __declspec(uuid("<object CLSID>")) CMyClass : public IUnknown
// {
// public:
//     IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
//
// as follows
//
// CStaticClassFactory<CMyClass> classFactory(this);
// hr = classFactory.Register(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE);
// if (SUCCEEDED(classFactory.Register(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE)))
// {
//     classFactory.MessageLoop()
// }

template <class TObjectToFactory> class CStaticClassFactory : public IClassFactory
{
public:
    CStaticClassFactory(IUnknown *punkObject) : _dwRegisterClass(0), _punkObject(punkObject)
    {
        _punkObject->AddRef();
    }

    ~CStaticClassFactory()
    {
        if (_dwRegisterClass)
        {
            CoRevokeClassObject(_dwRegisterClass);
        }
        _punkObject->Release();
    }

    HRESULT Register(CLSCTX classContent, REGCLS classUse)
    {
        return CoRegisterClassObject(__uuidof(TObjectToFactory), static_cast<IClassFactory *>(this),
            classContent, classUse, &_dwRegisterClass);
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = { QITABENT(CStaticClassFactory, IClassFactory), { 0 }, };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        return 1;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        if (punkOuter)
        {
            *ppv = NULL;
            return CLASS_E_NOAGGREGATION;
        }
        return _punkObject->QueryInterface(riid, ppv); //  : TObjectToFactory::CreateInstance(riid, ppv);
    }

    IFACEMETHODIMP LockServer(BOOL /* fLock */)
    {
        return S_OK;
    }

private:
    DWORD _dwRegisterClass;
    IUnknown *_punkObject;
};


// this class encapsulates the management of a message loop for an
// application. it supports queing a callback to the application via the message
// loop to enable the app to return from a call and continue processing that call
// later. this behavior is needed when implementing a shell verb as verbs
// must not block the caller.  to use this class:
//
// 1) inherit from this class
//
// class CApplication : CAppMessageLoop
//
// 2) in the invoke function, for example IExecuteCommand::Execute() or IDropTarget::Drop()
// queue a callback by callong QueueAppCallback();
//
//   IFACEMETHODIMP CExecuteCommandVerb::Execute()
//   {
//       QueueAppCallback();
//   }
//
// 3) implement OnAppCallback, this is the code that will execute the queued callback
//    void OnAppCallback()
//    {
//        // do the work here
//    }

class CAppMessageLoop
{
protected:
    // this timer is used to exit the message loop if the the application is
    // activated but not invoked. this is needed if there is a failure when the
    // verb is being invoked due to low resources or some other uncommon reason.
    // without this the app would not exit in this case. this timer needs to be
    // canceled once the app learns that it has should remain running.
    static UINT const uTimeout = 30 * 1000;    // 30 seconds
    CAppMessageLoop() : _uTimeoutTimerId(SetTimer(NULL, 0, uTimeout, NULL)), _uPostTimerId(0)
    {
    }

    void QueueAppCallback()
    {
        // queue a callback on OnAppCallback() by setting a timer of zero seconds
        _uPostTimerId = SetTimer(NULL, 0, 0, NULL);
        if (_uPostTimerId)
        {
            CancelTimeout();
        }
    }

    // cancel the timeout timer, this should be called when the appliation
    // knows that it wants to keep running, for example when it recieves the
    // incomming call to invoke the verb, this is done implictly when the
    // app queues a callback

    void CancelTimeout()
    {
        if (_uTimeoutTimerId)
        {
            KillTimer(NULL, _uTimeoutTimerId);
            _uTimeoutTimerId = 0;
        }
    }

    void MessageLoop()
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_TIMER)
            {
                KillTimer(NULL, msg.wParam);    // all are one shot timers

                if (msg.wParam == _uTimeoutTimerId)
                {
                    _uTimeoutTimerId = 0;
                }
                else if (msg.wParam == _uPostTimerId)
                {
                    _uPostTimerId = 0;
                    OnAppCallback();
                }
                PostQuitMessage(0);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void virtual OnAppCallback() = 0;  // app must implement

private:
    UINT_PTR _uTimeoutTimerId;      // timer id used to exit the app if the app is not called back within a certain time
    UINT_PTR _uPostTimerId;         // timer id used to to queue a callback to the app
};

