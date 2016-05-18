//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#include "DeviceSetting.h"
#include <faxcomex_i.c>

#define ARR_SIZE(a) (sizeof(a)/sizeof(a[0]))


//+---------------------------------------------------------------------------
//
//  function:   GiveUsage
//
//  Synopsis:   prints the usage of the application
//
//  Arguments:  [AppName] - Name of the application whose usage has to be printed
//
//  Returns:    void
//
//----------------------------------------------------------------------------

void GiveUsage(LPTSTR AppName)
{
        _tprintf( TEXT("Usage : %s \n \
            /s Fax Server Name \n \
            /l <list/set> Devices on the server \n \
            /i device id of the device whose property (TSID or CSID) has to be set  \n \
            /c new CSID value for the device \n \
            /t new TSID value for the device \n "),AppName);
        _tprintf( TEXT("Usage : %s /? -- help message\n"),AppName);
}
//+---------------------------------------------------------------------------
//
//  function:   IsOSVersionCompatible
//
//  Synopsis:   finds whether the target OS supports this functionality.
//
//  Arguments:  [dwVersion] - Minimum Version of the OS required for the Sample to run.
//
//  Returns:    bool - true if the Sample can run on this OS
//
//----------------------------------------------------------------------------

bool IsOSVersionCompatible(DWORD dwVersion)
{
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)    ;
        if( !bOsVersionInfoEx  )
        {
                osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
                if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
                        return false;
        }
        bOsVersionInfoEx = (osvi.dwMajorVersion >= dwVersion );
        return (bOsVersionInfoEx == TRUE);
}

//+---------------------------------------------------------------------------
//
//  function:   setTSID
//
//  Synopsis:   sets the value of TSID for a FaxDevice
//
//  Arguments:  [pFaxDevices] - FaxDevices object pointing to the list of devices on the server
//                [lDeviceId] - Device Id of the device to be set
//                [lptstrTSID] -    value of the TSID
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool setTSID(IFaxDevices* pFaxDevices, long lDeviceId, LPTSTR lptstrTSID)
{
        HRESULT hr = S_OK;
        IFaxDevice* pFaxDevice = NULL;
        BSTR bstrTSID = NULL;
        bool bRetVal = false;
        //check for NULL
        if((pFaxDevices == NULL) || (lptstrTSID == NULL))
        { 
                _tprintf(_T("setTSID: Parameter passed is NULL"));
                goto Exit;
        }
        bstrTSID = SysAllocString(lptstrTSID);
        if(bstrTSID == NULL)
        {
                _tprintf(_T("setTSID: bstrTSID is NULL.\n") );
                goto Exit;
        }
        hr = pFaxDevices->get_ItemById(lDeviceId,&pFaxDevice);
        if(FAILED(hr))
        {
                _tprintf(_T("setTSID: get_ItemById Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        //set TSID
        hr = pFaxDevice->put_TSID(bstrTSID);
        if(FAILED(hr))
        {
                _tprintf(_T("setTSID: put_TSID Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }

        //Save it
        hr = pFaxDevice->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setTSID: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        _tprintf(_T("New TSID is set"));
        bRetVal = true;
Exit:
        if(bstrTSID)
            SysFreeString(bstrTSID);
        return bRetVal;
}


//+---------------------------------------------------------------------------
//
//  function:   listDevices
//
//  Synopsis:   lists the set of devices on the server
//
//  Arguments:  [pFaxDevices] - FaxDevices object pointing to the list of devices on the server
//                
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool listDevices(IFaxDevices* pFaxDevices)
{
        HRESULT hr = S_OK;
        BSTR bstrDeviceName = NULL;
        IFaxDevice* pFaxDevice = NULL;
        long count = 0;
        bool bRetVal = false;

        //check for NULL
        if(pFaxDevices == NULL)
        { 
                _tprintf(_T("listDevices: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxDevices->get_Count(&count);
        if(FAILED(hr))
        {
                _tprintf(_T("listDevices: get_Count Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }    
        long lDeviceId = 0;
        for(short i =0; i< count; i++)
        {
                VARIANT varDevice;
                VariantInit(&varDevice);
                varDevice.vt = VT_I2;
                varDevice.iVal = i+1;
                hr = pFaxDevices->get_Item(varDevice,&pFaxDevice);
                if(FAILED(hr))
                {
                        _tprintf(_T("listDevices: get_Item Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }
                hr = pFaxDevice->get_Id(&lDeviceId);
                if(FAILED(hr))
                {
                        _tprintf(_T("listDevices: get_Id Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }
                hr = pFaxDevice->get_DeviceName(&bstrDeviceName);
                if(FAILED(hr))
                {
                        _tprintf(_T("listDevices: get_DeviceName Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }
                _tprintf(_T("Device No: %d Device Id = %d Device Name = %s \n"), i, lDeviceId, bstrDeviceName);        
                if(bstrDeviceName)
                        SysFreeString(bstrDeviceName);
        }    
        bRetVal = true;
Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   setCSID
//
//  Synopsis:   sets the value of CSID for a FaxDevice
//
//  Arguments:  [pFaxDevices] - FaxDevices object pointing to the list of devices on the server
//              [lDeviceId] - Device Id of the device to be set
//              [lptstrCSID] -    value of the CSID
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool setCSID(IFaxDevices* pFaxDevices, long lDeviceId, LPTSTR lptstrCSID)
{
        HRESULT hr = S_OK;
        IFaxDevice* pFaxDevice = NULL;
        BSTR bstrCSID = NULL;
        bool bRetVal = false;

        //check for NULL
        if((pFaxDevices == NULL) || (lptstrCSID == NULL))
        { 
                _tprintf(_T("setCSID: Parameter passed is NULL"));
                goto Exit;
        }

        bstrCSID = SysAllocString(lptstrCSID);
        if(bstrCSID == NULL)
        {
                _tprintf(_T("setCSID: bstrCSID is NULL. \n"));
                goto Exit;
        }
        hr = pFaxDevices->get_ItemById(lDeviceId,&pFaxDevice);
        if(FAILED(hr))
        {
                _tprintf(_T("setCSID: get_ItemById Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        //set CSID
        hr = pFaxDevice->put_CSID(bstrCSID);
        if(FAILED(hr))
        {
                _tprintf(_T("setCSID: put_CSID Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }

        //Save it
        hr = pFaxDevice->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("setCSID: Save Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }
        _tprintf(_T("New CSID is set"));
        bRetVal = true;   
Exit:
        if(bstrCSID)
            SysFreeString(bstrCSID);
        return bRetVal;
}


int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrTSID = NULL;
        LPTSTR lptstrCSID = NULL;
        LPTSTR lptstrDeviceId = NULL;
        LPTSTR lptstrOption = NULL;
        BSTR bstrServerName = NULL;
        bool bConnected = false;
        size_t argSize = 0;
        bool bState = false;
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("This sample is compatible with Windows Vista"));
                bRetVal = false;
                goto Exit1;
        }

        //introducing an artifical scope here so that the COm objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer = NULL;
                IFaxDevices* pFaxDevices = NULL;
                IFaxDevice* pFaxDevice = NULL;

                int argcount = 0;

#ifdef UNICODE
                argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
                argv = argvA;
#endif

                if (argc == 1)
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }


                // check for commandline switches
                for (argcount=1; argcount<argc; argcount++)
                {                  
                        if(argcount + 1 < argc)
                        {
                                hr = StringCbLength(argv[argcount + 1],1024 * sizeof(TCHAR),&argSize);
                                if(!FAILED(hr))
                                {
                                        if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-'))
                                        {
                                                switch (towlower(argv[argcount][1]))
                                                {
                                                        case 's':
                                                                if(lptstrServerName == NULL)
                                                                {
                                                                        lptstrServerName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrServerName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrServerName, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrServerName,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;
                                                        case 'l':
                                                                if(lptstrOption == NULL)
                                                                {
                                                                        lptstrOption = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));                        
                                                                        if(lptstrOption == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrOption: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrOption, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrOption,argSize +1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrOption: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;
                                                        case 'i':
                                                                if(lptstrDeviceId == NULL)
                                                                {
                                                                        lptstrDeviceId = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrDeviceId == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrDeviceId: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrDeviceId, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrDeviceId, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrDeviceId: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;

                                                        case 'c':
                                                                if(lptstrCSID == NULL)
                                                                {
                                                                        lptstrCSID = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrCSID == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrCSID: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrCSID, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrCSID, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrCSID: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break; 
                                                        case 't':
                                                                if(lptstrTSID == NULL)
                                                                {
                                                                        lptstrTSID = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrTSID == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrTSID: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrTSID, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrTSID, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrTSID: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;
                                                        case '?':
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit;                
                                                        default:
                                                                break;
                                                }//switch
                                        }//if
                                }
                        }
                }//for

                if ((lptstrOption == NULL) || (( _tcscmp(_T("set"), CharLower(lptstrOption)) == 0) && ((lptstrDeviceId == NULL) || (lptstrTSID == NULL && lptstrCSID == NULL) )))
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                //initialize COM
                hr = CoInitialize(NULL);
                if(FAILED(hr))
                {
                        //failed to init com
                        _tprintf(_T("Failed to init com. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = CoCreateInstance (CLSID_FaxServer, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxServer), 
                            (void **)&pFaxServer);
                if(FAILED(hr))
                {
                        //CoCreateInstance failed.
                        _tprintf(_T("CoCreateInstance failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                //connect to fax server.
                bstrServerName = SysAllocString(lptstrServerName);
                if( bstrServerName == NULL && lptstrServerName != NULL)
                {
                        _tprintf(_T("bstrServerName is NULL. \n"));
                        bRetVal = false;
                        goto Exit;
                }
                hr = pFaxServer->Connect(bstrServerName);
                if(FAILED(hr))
                {
                        _tprintf(_T("Connect failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                bConnected = true;

                FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
                hr = pFaxServer->get_APIVersion(&enumFaxAPIVersion);
                if(FAILED(hr))
                {
                        //get_APIVersion failed.
                        _tprintf(_T("get_APIVersion failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if (enumFaxAPIVersion < fsAPI_VERSION_3) 
                {
                        bRetVal = false;
                        _tprintf(_T("This sample is compatible with Windows Vista"));
                        goto Exit;
                }         

                hr = pFaxServer->GetDevices(&pFaxDevices);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_Devices failed. Error 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //if list devices option is selected
                if(_tcscmp(_T("list"), CharLower(lptstrOption)) == 0)
                {
                        if(!listDevices(pFaxDevices))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }
                //if set device option is selected
                else 
                {
                        if(_tcscmp(_T("set"), CharLower(lptstrOption)) == 0)
                        {
                                //if set device TSID option is selected
                                long lDeviceId = _ttol((LPCTSTR)lptstrDeviceId);
                                if(lptstrTSID != NULL)          
                                {
                                        if(!setTSID(pFaxDevices, lDeviceId, lptstrTSID))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }

                                //if set device CSID option is selected
                                if(lptstrCSID != NULL)          
                                {
                                        if(!setCSID(pFaxDevices, lDeviceId, lptstrCSID))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                        }
                }


Exit:
                if(bConnected)
                {
                        pFaxServer->Disconnect();
                }
                if(lptstrServerName)
                        free(lptstrServerName);
                if(lptstrOption)
                        free(lptstrOption);
                if(lptstrTSID)
                        free(lptstrTSID);
                if(lptstrCSID)
                        free(lptstrCSID);
                if(lptstrDeviceId)
                        free(lptstrDeviceId);
                if(bstrServerName)
                        SysFreeString(bstrServerName);
        }
        CoUninitialize();
Exit1:
        return bRetVal;
}
