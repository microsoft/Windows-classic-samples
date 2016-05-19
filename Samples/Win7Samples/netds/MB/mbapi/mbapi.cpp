/*++

Copyright (c) 2008 Microsoft Corporation

Module Name:

    mbapi.cpp

Abstract:

    Sample code for Mobile Broadband APIs

Date:
    10/20/2008  created

Environment:

   User mode only

--*/

#include <atlbase.h>
// headers needed to use Mobile Broadband APIs 
#include "mbnapi.h"

#define MAX_NOTIFICATION_EVENTS     1

CComPtr<IMbnInterfaceManager>  g_InterfaceMgr = NULL;
CComPtr<IMbnRadio> g_MbnRadio;

VOID DeInitInterfaceMgr();
HRESULT InitApp()
{
    //MB API supports both MTA and STA, this example however exhibits
    //the use of MTA
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    return hr;    
}

HRESULT InitInterfaceMgr()
{
    HRESULT hr = E_FAIL;

    //Initialize Interface Manager
    hr = CoCreateInstance(CLSID_MbnInterfaceManager,
        NULL, 
        CLSCTX_ALL, 
        IID_IMbnInterfaceManager, 
        (void**)&g_InterfaceMgr);

    return hr;
}

VOID DeInitInterfaceMgr()
{
    g_InterfaceMgr = NULL;
}

VOID DeInitApp()
{
    CoUninitialize();
}

HANDLE g_Event = NULL;

// This class is created to implement the event listener notifications. It needs to extend
// all event interfaces for which application wants to register for event notifications e.g.
// it extends IMbnRadioEvents for registering to Radio events notifications. If application
// requires to register for Signal events notifications as well, it should extend IMbnSignalEvents
// too. It then implements all the members of corresponding event interface say OnRadioStateChange, 
// OnSetSoftwareRadioStateComplete for IMbnRadioEvents. Had it extended the IMbnSignalEvents to 
// register for Signal events notifications, it would have implemented the OnSignalStateChange member
// of IMbnSignalEvents too.

class CMbnSinks:
    public IMbnRadioEvents
{
    ULONG m_lRef;
public:
    CMbnSinks();
    ~CMbnSinks();

    //IMbnRadioEvents
    HRESULT STDMETHODCALLTYPE OnRadioStateChange(__in IMbnRadio* Radio);

    HRESULT STDMETHODCALLTYPE OnSetSoftwareRadioStateComplete(__in IMbnRadio* Radio, 
                                    __in ULONG requestID, 
                                    __in HRESULT Status);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();
};

//
// Utility functions
//

// This function takes the safearray of IMbnInterface objects as input and copies
// the interface guids to the pointer to the list of guids.  
HRESULT CopyArrayOfInterfaces(SAFEARRAY* psaObjects, UINT* pCount, GUID** ppGuidList)
{
    if (NULL == psaObjects || NULL == ppGuidList || NULL == pCount)
    {
        return E_POINTER;
    }
    
    *ppGuidList = NULL;
    *pCount = 0;

    HRESULT hr = S_OK;

    LONG lLower;
    LONG lUpper;

    hr = SafeArrayGetLBound(psaObjects, 1, &lLower);
    if(FAILED(hr))
    {
        return hr;
    }
    
    hr = SafeArrayGetUBound(psaObjects, 1, &lUpper);
    if(FAILED(hr))
    {
        return hr;
    }

    LONG Num = lUpper - lLower + 1; 

    if(Num > 0)
    {
        *ppGuidList = (GUID*)malloc(sizeof(GUID)* Num);
        if(*ppGuidList == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

    BSTR GuidName;
    GUID Guid;

    CComPtr<IMbnInterface> pMbnInterface = NULL;
    UINT nActualInterfaces = 0;
    for (LONG l = lLower; l <= lUpper; l++)
    {
        hr = SafeArrayGetElement(psaObjects, &l, &pMbnInterface);
        if (SUCCEEDED(hr))
        {
            hr = pMbnInterface->get_InterfaceID(&GuidName);
            if(SUCCEEDED(hr))
            {
                hr = IIDFromString(GuidName, &Guid);
                if(SUCCEEDED(hr))
                {
                    (*ppGuidList)[nActualInterfaces] = Guid;
                    nActualInterfaces ++;
                }

                SysFreeString(GuidName);
                GuidName = NULL;
            }
        }
        pMbnInterface = NULL;
    }

    *pCount = nActualInterfaces;
    if(nActualInterfaces > 0)
    {
        hr = S_OK;
    }

    return hr;
}

// print the error message
VOID
PrintStatusMsg(
    __in LPWSTR strCommand,
    __in HRESULT hr
)
{
    if (strCommand != NULL)
    {
        if (hr == S_OK)
        {
            wprintf(L"Command \"%s\" completed successfully.\n", strCommand);
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER))
        {
            wprintf(L"The parameter for \"%s\" is not correct.\n", strCommand);
            wprintf(L"Please use \"help %s\" to check the usage of the command.\n", strCommand);
        }
        else
        {	
            wprintf(L"Got error 0x%x  for command \"%s\"\n", hr, strCommand);
        }
    }
}

// enumerates mobile broadband interfaces
VOID 
EnumInterface(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    HRESULT hr = S_OK;
    SAFEARRAY *psa = NULL;
    GUID *pList = NULL;
    BOOL isInitAppSuccess = FALSE;
    BOOL isModeSet = TRUE;
    
    do
    {
        if (argc != 1)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }

        hr = InitApp();
        if(FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            break;            
        }
        isInitAppSuccess = TRUE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            isModeSet = FALSE;          
        }
        
        hr = InitInterfaceMgr();
        if(FAILED(hr))
        {
            break;
        }
        
        hr = g_InterfaceMgr->GetInterfaces(&psa);
        if(FAILED(hr))
        {
            break;
        }

        UINT size = 0;
        hr = CopyArrayOfInterfaces(psa, &size, &pList);
        if(FAILED(hr))
        {
            break;
        }

        if (pList)
        {
            for (ULONG l = 0; l < size; l++)
            {
                LPOLESTR pStr = NULL;
                hr = StringFromIID(pList[l], &pStr);
                if(FAILED(hr))
                {
                    break;
                }

                wprintf(L"Interface [%d] = %s\n", l, pStr);
                CoTaskMemFree(pStr);
            }
        }

    } while(FALSE);

    //cleanup
    if(psa)
    {
        SafeArrayDestroy(psa);
    }
    
    if(pList)
    {
        free(pList);
    }
    
    if(g_InterfaceMgr)
    {
        DeInitInterfaceMgr();
    }
    
    if(isInitAppSuccess && isModeSet)
    {
        DeInitApp();
    }
    
    PrintStatusMsg(argv[0], hr);
}

// get interface capability
VOID 
GetInterfaceCapability(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    HRESULT hr = S_OK;
    BOOL isInitAppSuccess = FALSE;
    BOOL isModeSet = TRUE;
    
    do
    {
        if (argc != 2)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }
        
        hr = InitApp();
        if(FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            break;            
        }
        isInitAppSuccess = TRUE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            isModeSet = FALSE;          
        }        

        hr = InitInterfaceMgr();
        if(FAILED(hr))
        {
            break;
        }

        MBN_INTERFACE_CAPS InterfaceCaps;
        CComPtr<IMbnInterface> pMbnInterface;
        hr = g_InterfaceMgr->GetInterface(argv[1], &pMbnInterface);
        if(FAILED(hr))
        {
            break;
        }

        hr = pMbnInterface->GetInterfaceCapability(&InterfaceCaps);
        if(FAILED(hr))
        {
            break;
        }

        wprintf(L"\t DeviceId \t\t%s\n", InterfaceCaps.deviceID);
        wprintf(L"\t Manufacturer \t\t%s\n", InterfaceCaps.manufacturer);
        wprintf(L"\t Model \t\t\t%s\n", InterfaceCaps.model);

        SysFreeString(InterfaceCaps.customDataClass);
        SysFreeString(InterfaceCaps.customBandClass);
        SysFreeString(InterfaceCaps.deviceID);
        SysFreeString(InterfaceCaps.manufacturer);
        SysFreeString(InterfaceCaps.model);
        SysFreeString(InterfaceCaps.firmwareInfo);

    } while(FALSE);

    //cleanup
    if(g_InterfaceMgr)
    {
        DeInitInterfaceMgr();
    }
    
    if(isInitAppSuccess && isModeSet)
    {
        DeInitApp();
    }


    PrintStatusMsg(argv[0], hr);
}

// get radio state
VOID 
GetRadioState(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    HRESULT hr = S_OK;
    BOOL isInitAppSuccess = FALSE;
    BOOL isModeSet = TRUE;
    
    do
    {
        if (argc != 2)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }

        hr = InitApp();
        if(FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            break;            
        }
        isInitAppSuccess = TRUE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            isModeSet = FALSE;          
        }        

        hr = InitInterfaceMgr();
        if(FAILED(hr))
        {
            break;
        }

        MBN_RADIO SoftwareRadioState;
        MBN_RADIO HardwareRadioState;
        CComPtr<IMbnInterface> pMbnInterface;
        CComPtr<IMbnRadio> pMbnRadio;
        hr = g_InterfaceMgr->GetInterface(argv[1], &pMbnInterface);
        if(FAILED(hr))
        {
            break;
        }

        hr = pMbnInterface->QueryInterface(__uuidof(IMbnRadio), reinterpret_cast<void**>(&pMbnRadio));
        if(FAILED(hr))
        {
            break;
        }

        hr = pMbnRadio->get_SoftwareRadioState(&SoftwareRadioState);
        if(FAILED(hr))
        {
            break;
        }

        if (MBN_RADIO_OFF == SoftwareRadioState)
        {
            wprintf(L"\t SoftwareRadioState \tOff\n");
        }
        else if (MBN_RADIO_ON == SoftwareRadioState)
        {
            wprintf(L"\t SoftwareRadioState \tOn\n");
        }    

        hr = pMbnRadio->get_HardwareRadioState(&HardwareRadioState);
        if(FAILED(hr))
        {
            break;
        }

        if (MBN_RADIO_OFF == HardwareRadioState)
        {
            wprintf(L"\t HardwareRadioState \tOff\n");
        }
        else if (MBN_RADIO_ON == HardwareRadioState)
        {
            wprintf(L"\t HardwareRadioState \tOn\n");
        }
    
    }
    while(FALSE);

    //cleanup
    if(g_InterfaceMgr)
    {
        DeInitInterfaceMgr();
    }
    
    if(isInitAppSuccess && isModeSet)
    {
        DeInitApp();
    }


    PrintStatusMsg(argv[0], hr);
}

// set the radio state
VOID 
SetRadioState(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    HRESULT hr = S_OK;    
    CMbnSinks *pCMbnSinks          = NULL;
    BOOL isInitAppSuccess = FALSE;
    BOOL isModeSet = TRUE;
    BOOL isAdviseDone = FALSE;    
    IConnectionPoint *pcp;
    DWORD dwCookie = 0;
    
    do
    {
        if (argc != 3)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }

        MBN_RADIO softwareRadioState;
        if (_wcsicmp(argv[2], L"on") == 0)
        {
            softwareRadioState = MBN_RADIO_ON;
        }
        else if (_wcsicmp(argv[2], L"off") == 0)
        {
            softwareRadioState = MBN_RADIO_OFF;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }

        hr = InitApp();
        if(FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            break;            
        }
        isInitAppSuccess = TRUE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            isModeSet = FALSE;          
        }        

        hr = InitInterfaceMgr();
        if(FAILED(hr))
        {
            break;
        }

        //CreateEvent
        g_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(!g_Event)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        
        // registering for notifications, following steps need to be performed
        IUnknown* pUnkSink;
        CComPtr<IConnectionPointContainer>  pcpc;

        // 1.Get an IConnectionPointContainer interface by calling QueryInterface 
        //   on corresponding IMbnXXXManager object say IMbnInterfaceManager for IMbnRadioEvents.  
        hr = g_InterfaceMgr->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<VOID**>(&pcpc));
        if(FAILED(hr))
        {
            break;
        }

        // 2.Call FindConnectionPoint on the returned interface and pass corresponding 
        //   IID_IMbnXXXEvents to riid say IID_IMbnRadioEvents.
        hr = pcpc->FindConnectionPoint(IID_IMbnRadioEvents, &pcp);
        if(FAILED(hr))
        {
            break;
        }

        // 3.Call Advise on the returned connection point and pass a pointer to an IUnknown 
        //   interface on an object that implements IMbnXXXEvents to pUnk.
        pCMbnSinks = new CMbnSinks;
        if (pCMbnSinks)
        {
            //QI will do AddRef on pCMbnSinks   
            pCMbnSinks->QueryInterface(IID_IUnknown, (LPVOID *)&pUnkSink ); 
            hr = pcp->Advise(pUnkSink, &dwCookie);
        }
        if(FAILED(hr))
        {
            break;
        }
        isAdviseDone = TRUE;
        
        //QI MbnRadio object to perform the SetSoftwareRadioState operation
        CComPtr<IMbnInterface> pMbnInterface;
        hr = g_InterfaceMgr->GetInterface(argv[1], &pMbnInterface);
        if(FAILED(hr))
        {
            break;
        }

        CComPtr<IMbnRadio> pMbnRadio;
        hr = pMbnInterface->QueryInterface(__uuidof(IMbnRadio), reinterpret_cast<void**>(&pMbnRadio));
        if(FAILED(hr))
        {
            break;
        }

        ULONG requestID = 0;
        hr = pMbnRadio->SetSoftwareRadioState(softwareRadioState, &requestID);
        if(FAILED(hr))
        {
            break;
        }
        wprintf(L"\t SetSoftwareRadioState call successful requestID = %d\n", requestID);

        //Now waiting for set request to get completed		
        DWORD waitObject;

        waitObject = WaitForSingleObject(
                        g_Event,
                        INFINITE);
                
        if (WAIT_OBJECT_0 == waitObject)
        {
            // g_Event is triggerred
            // for getting the status of the software radio state now
            pMbnRadio = g_MbnRadio;
            MBN_RADIO NewSoftwareRadioState;
            hr = pMbnRadio->get_SoftwareRadioState(&NewSoftwareRadioState);
            if(FAILED(hr))
            {
                break;
            }
            
            if (MBN_RADIO_OFF == NewSoftwareRadioState)
            {
                wprintf(L"\t New SoftwareRadioState \tOff\n");
            }
            else if (MBN_RADIO_ON == NewSoftwareRadioState)
            {
                wprintf(L"\t New SoftwareRadioState \tOn\n");
            }      
            
        }
    
    } while(FALSE);

    //cleanup
    if(isAdviseDone)
    {
        pcp->Unadvise(dwCookie);
        pcp->Release();
    }
    
    if (g_Event)
    {
        CloseHandle(g_Event);
        g_Event = NULL;
    }
    
    if (pCMbnSinks)
    {
        pCMbnSinks->Release();
    }
    
    g_MbnRadio = NULL;
    
    if(g_InterfaceMgr)
    {
        DeInitInterfaceMgr();
    }
    
    if(isInitAppSuccess && isModeSet)
    {
        DeInitApp();
    }

    PrintStatusMsg(argv[0], hr);
}

// show help messages
VOID 
Help(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
);

typedef VOID (*WLSAMPLE_FUNCTION) (int argc, LPWSTR argv[]);
typedef struct _WLSAMPLE_COMMAND {
    LPWSTR strCommandName;           // command name
    LPWSTR strShortHand;             // a shorthand for the command
    WLSAMPLE_FUNCTION Func;         // pointer to the function
    LPWSTR strHelpMessage;          // help message
    LPWSTR strParameters;           // parameters for the command
    BOOL bRemarks;                  // whether have remarks for the command
    LPWSTR strRemarks;              // remarks
} WLSAMPLE_COMMAND, *PWLSAMPLE_COMMAND;

WLSAMPLE_COMMAND g_Commands[] = {
    // interface related commands
    {
        L"EnumInterface",
        L"ei",
        EnumInterface,
        L"Enumerate wireless interfaces and print the basic interface information.",
        L"",
        FALSE,
        L""
    },
    {
        L"GetInterfaceCapability",
        L"gic",
        GetInterfaceCapability,
        L"Get the capability of an interface.",
        L"<interface GUID>",
        TRUE,
        L"Use EnumInterface (ei) command to get the GUID of an interface."
    },
    {
        L"GetRadioState",
        L"grs",
        GetRadioState,
        L"Get the software radio state.",
        L"<interface GUID>",
        TRUE,
        L"Use EnumInterface (ei) command to get the GUID of an interface."
    },
    {
        L"SetRadioState",
        L"srs",
        SetRadioState,
        L"Set the software radio state.",
        L"<interface GUID> <on|off>",
        TRUE,
        L"Use EnumInterface (ei) command to get the GUID of an interface."
    },
    {
        L"help",
        L"?",
        Help,
        L"Print this help message.",
        L"[<command>]",
        FALSE,
        L""
    }
};

// show help messages
VOID 
Help(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    UINT i;

    if (argc == 1)
    {
        // show all commands
        wprintf(L"This is a sample showing how to use Mobile Broadband APIs to manager wireless networks.\n");
        wprintf(L"The following commands are available. Use \"help xyz\" to show the description of command xyz.\n");
        for (i=0; i < ARRAYSIZE(g_Commands); i++)
        {
                wprintf(L"\t %s", g_Commands[i].strCommandName);
                wprintf(L"(%s)\n", g_Commands[i].strShortHand);
        }
    }
    else if (argc == 2)
    {
        // show the description of a command
        for (i=0; i < ARRAYSIZE(g_Commands); i++)
        {
            if (_wcsicmp(argv[1], g_Commands[i].strCommandName) == 0 ||
                    _wcsicmp(argv[1], g_Commands[i].strShortHand) == 0)
            {
                wprintf(L"Command: %s", g_Commands[i].strCommandName);
                wprintf(L"(%s)\n", g_Commands[i].strShortHand);
                wprintf(L"Description: %s\n", g_Commands[i].strHelpMessage);
                wprintf(L"Usage: %s", g_Commands[i].strCommandName);
                wprintf(L"(%s)\n", g_Commands[i].strShortHand);
                wprintf(g_Commands[i].strParameters);
                if (g_Commands[i].bRemarks)
                {
                    wprintf(L"Remarks: %s\n", g_Commands[i].strRemarks);
                }
                break;
            }
        }
    }
    else
    {
        wprintf(L"Invalid Parameter\n");
    }
}

// command is stored in the global variable
void 
ExecuteCommand(
    __in int argc, 
    __in_ecount(argc) LPWSTR argv[]
)
{
    UINT i = 0;

    for (i=0; i < ARRAYSIZE(g_Commands); i++)
    {
        // find the command and call the function
        if (_wcsicmp(argv[0], g_Commands[i].strCommandName) == 0 ||
            _wcsicmp(argv[0], g_Commands[i].strShortHand) == 0)
        {
            g_Commands[i].Func(argc, argv);
            break;
        }
    }

    if (i == ARRAYSIZE(g_Commands))
    {
        wprintf(L"Invalid command %s !\n", argv[0]);
        
    }
}

// the main program
int _cdecl
wmain(__in int argc, __in_ecount(argc) LPWSTR argv[])
{
    DWORD dwRetCode = ERROR_SUCCESS;
    
    if (argc == 1)
    {
        wprintf(L"Please type \"%s ?\" for help.\n", argv[0]);
        dwRetCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        // don't pass in the first parameter
        ExecuteCommand(argc-1, argv+1);
    }

    return dwRetCode;
}

CMbnSinks::CMbnSinks()
{
    m_lRef = 0;    
}

CMbnSinks::~CMbnSinks()
{
}

// This method is used to QueryInterface the required riid. As the class
// CMbnSinks is updated to extend and implement required event interfaces
// say IMbnSignalEvents, this method should be updated accordingly to QI
// the additional riids say IID_IMbnSignalEvents.

HRESULT STDMETHODCALLTYPE CMbnSinks::QueryInterface(REFIID riid, void **ppv)
{
        if ( !ppv )
        {
            return E_POINTER;
        }

        HRESULT hr = E_NOINTERFACE;

        *ppv = NULL;

        if (riid == IID_IUnknown)
        {
            *ppv = this;
        }
        else if(riid == IID_IMbnRadioEvents)
        {
            *ppv = (IMbnRadioEvents *)this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        if (*ppv)
        {
            reinterpret_cast<IUnknown *>(*ppv)->AddRef();
            hr = S_OK;
        }

        return hr;
}

ULONG STDMETHODCALLTYPE CMbnSinks::AddRef()
{
      return InterlockedIncrement( (LONG *)&m_lRef );
}

ULONG STDMETHODCALLTYPE CMbnSinks::Release()
{
    ULONG ulNewRef = (ULONG)InterlockedDecrement( (LONG *)&m_lRef );
    if( ulNewRef == 0 )
    {
        delete this;
    }

    return ulNewRef;
}

HRESULT STDMETHODCALLTYPE CMbnSinks::OnRadioStateChange(__in IMbnRadio* Radio)
{
    wprintf(L"*****Received OnRadioStateChange Event\n");
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CMbnSinks::OnSetSoftwareRadioStateComplete(__in IMbnRadio* Radio, 
                                             __in ULONG requestID, 
                                             __in HRESULT Status)
{
    
    wprintf(L"***Received OnSetSoftwareRadioStateComplete Event for requestID = %d\n", requestID);
    g_MbnRadio = Radio;     
    SetEvent(g_Event);
    return(S_OK);
}

