////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 1998-2001  Microsoft Corporation
//
//
// TAPIEventNotification.h : Declaration of the CTAPIEventNotification object
//
////////////////////////////////////////////////////////////////////////////
#ifndef __TAPIEventNotification_H__
#define __TAPIEventNotification_H__


#define WM_PRIVATETAPIEVENT		WM_USER+101
#define MAX_REC_TIME			60000
#define TIMER_ID				1

#define PLAY_FILENAME			_T("Welcome.wav")
#define REC_FILENAME			_T("Message")
#define REC_FILEEXT				_T(".avi")

HRESULT
OnTapiEvent(
				IN TAPI_EVENT TapiEvent,
				IN IDispatch * pEvent
				);

/////////////////////////////////////////////////////////////////////////////
// CTAPIEventNotification
class CTAPIEventNotification : public ITTAPIEventNotification
{
private:

	LONG       m_dwRefCount;

public:

	// CTAPIEventNotification implements ITTAPIEventNotification
	//  Declare ITTAPIEventNotification methods here
	HRESULT STDMETHODCALLTYPE Event(
                                    IN TAPI_EVENT TapiEvent,
                                    IN IDispatch * pEvent
                                   );
    
// other COM stuff:
public:

	// constructor
	CTAPIEventNotification() { m_dwRefCount = 1;}

	// IUnknown implementation
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
	{
		if (iid == IID_ITTAPIEventNotification)
		{
			AddRef();
			*ppvObject = (void *)this;
			return S_OK;
		}

		if (iid == IID_IUnknown)
		{
			AddRef();
			*ppvObject = (void *)this;
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	//
	// reference counting needs to be thread safe
	//

	ULONG STDMETHODCALLTYPE AddRef()
	{
		ULONG l = InterlockedIncrement(&m_dwRefCount);
		return l;
	}
    
	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG l = InterlockedDecrement(&m_dwRefCount);

		if ( 0 == l)
		{
			delete this;
		}

		return l;
	}

};

//
//helper functions
//
HRESULT 
CreateAndSelectFileRecordTerminal();

bool 
SameCall(
		IN ITCallStateEvent* pCallStateEvent
		);

HRESULT 
DoCallNotification(
		IN IDispatch * pEvent
		);

HRESULT 
DoCallState(
		IN IDispatch * pEvent
		);

HRESULT 
DoCallMedia(
		IN IDispatch * pEvent
		);

HRESULT 
DoFileEvent(
		IN IDispatch * pEvent
		);


#endif //__TAPIEventNotification_H__
