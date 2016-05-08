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

#include "OutboundRouting.h"
#include <faxcomex_i.c>

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
                                /o <listGroups/ListRules/RemoveGroup/AddGroup/RemoveRule/AddRule> \n \
                                /i index of the group or rule to be removed. valid values 0 to n \n \
                                /n name of the new routing rule or group \n \
                                /c country code for the new routing rule \n \
                                /a area code for the new routing rule \n \
                                /b 1 to use the device id for the new routing rule else 0 \n \
                                /d device id for the routing rule and list of device ids separated by \";\" for routing group \n" ),AppName);
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
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
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
//  function:   listDeviceIds
//
//  Synopsis:   lists the set of devices on for a routing group
//
//  Arguments:  [IFaxDeviceIds] - FaxDevIds object pointing to the list of device ids are part of the routing group
//                
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool listDeviceIds(IFaxDeviceIds* pFaxDevIds)
{
        HRESULT hr = S_OK;        
        long lDevId = 0;
        long count = 0;
        bool bRetVal = false;

        //check for NULL
        if(pFaxDevIds == NULL)
        { 
                _tprintf(_T("listDevices: Parameter passed is NULL"));
                goto Exit;
        }
        hr = pFaxDevIds->get_Count(&count);
        if(FAILED(hr))
        {
                _tprintf(_T("listDevices: get_Count Failed. Error. 0x%x \n"), hr);
                goto Exit;
        }    
        long lDeviceId = 0;
        for(int i =1; i<= count; i++)
        {                
                hr = pFaxDevIds->get_Item(i,&lDevId);
                if(FAILED(hr))
                {
                        _tprintf(_T("listDevices: get_Item Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }                
                _tprintf(_T("Device No: %d Device Id = %d \n"), i, lDevId);                       
        }    
        bRetVal = true;
Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   listGroups
//
//  Synopsis:   list of Routing Groups on the Server
//
//  Arguments:  [pFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool listGroups(IFaxOutboundRoutingGroups* pFaxOutRoutGrps)
{
        HRESULT  hr = S_OK;
        bool bRetVal = true;    
        long lCount =0; 
        IFaxOutboundRoutingGroup* pFaxOutRoutGrp = NULL;
        BSTR bstrGrpName = NULL;

        //check for NULL
        if (pFaxOutRoutGrps == NULL) 
        {
                _tprintf(_T("listGroups: Parameter passed is NULL"));
                goto Exit;
        }


        _tprintf(_T("\n Listing Routing Group details....\n \n"));

        hr = pFaxOutRoutGrps->get_Count(&lCount);
        if(FAILED(hr))
        {
                _tprintf(_T("Count failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        //enumerate
        for(int i =1; i<= lCount; i++)
        {
                VARIANT varDevice;
                IFaxDeviceIds* pFaxDevIds = NULL;
                FAX_GROUP_STATUS_ENUM enumStatus;
                VariantInit(&varDevice);
                varDevice.vt = VT_I4;
                varDevice.lVal = i;
                hr = pFaxOutRoutGrps->get_Item(varDevice,&pFaxOutRoutGrp);
                if(FAILED(hr))
                {
                        _tprintf(_T("listGroups: get_Item Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }
                hr = pFaxOutRoutGrp->get_DeviceIds(&pFaxDevIds);
                if(FAILED(hr))
                {
                        _tprintf(_T("listGroups: get_DeviceIds Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }

                hr = pFaxOutRoutGrp->get_Name(&bstrGrpName);
                if(FAILED(hr))
                {
                        _tprintf(_T("listGroups: get_Name Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }

                hr = pFaxOutRoutGrp->get_Status(&enumStatus);
                if(FAILED(hr))
                {
                        _tprintf(_T("listGroups: get_Status Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }

                //print all the details
                _tprintf(_T(" ===================================================\n"));  
                _tprintf(_T("Group No: %d Group Name = %s \n"), i, bstrGrpName);        
                if(enumStatus == fgsALL_DEV_VALID )
                        _tprintf(TEXT("Status : All the devices in the routing group are valid and available for sending outgoing faxes. \n") );
                if(enumStatus == fgsEMPTY )
                        _tprintf(TEXT("Status : The routing group does not contain any devices. \n") );
                if(enumStatus == fgsALL_DEV_NOT_VALID)
                        _tprintf(TEXT("Status : The routing group does not contain any available devices for sending faxes. (Devices can be \"unavailable\" when they are offline and when they do not exist.) \n") );
                if(enumStatus == fgsSOME_DEV_NOT_VALID)
                        _tprintf(TEXT("Status : The routing group contains some devices that are unavailable for sending faxes. (Devices can be \"unavailable\" when they are offline and when they do not exist.) \n") );

                if(!listDeviceIds(pFaxDevIds))
                {
                        //we dont want to log any error here as the error will be logged in the function itself
                        bRetVal = false;
                }
                if(bstrGrpName)
                        SysFreeString(bstrGrpName);
        }    

Exit: 
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   removeGroup
//
//  Synopsis:   removes a routing group from FaxOutboundRoutingGroups based on index
//
//  Arguments:  [pFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
//                [index] - index of the group to be removed
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool removeGroup(IFaxOutboundRoutingGroups* pFaxOutRoutGrps, int index)
{
        HRESULT  hr = S_OK;
        bool bRetVal = false;    
        long lCount =0; 
        //check for NULL
        if (pFaxOutRoutGrps == NULL) 
        {
                _tprintf(_T("removeGroup: Parameter passed is NULL"));
                bRetVal = false;
                goto Exit;
        }  

        //get count of groups
        hr = pFaxOutRoutGrps->get_Count(&lCount);
        if(FAILED(hr))
        {
                _tprintf(_T("removeGroup: Count failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        //invalid index
        if(index > lCount || index <=0)
        {
                _tprintf(_T("removeGroup: Invalid index value. It can be from 1 to %d.\n" ), lCount);
                bRetVal = false;
                goto Exit;
        }

        VARIANT varDevice;
        VariantInit(&varDevice);
        varDevice.vt = VT_I4;
        varDevice.lVal = index;

        //remove group
        hr = pFaxOutRoutGrps->Remove(varDevice);
        if(FAILED(hr))
        {
                _tprintf(_T("removeGroup: Remove Failed. Error. 0x%x \n"), hr);
                bRetVal = false;
                goto Exit;
        }
        _tprintf(_T("Group removed successfully. \n"));
        bRetVal = true;     

Exit: 
        return bRetVal;
}


//+---------------------------------------------------------------------------
//
//  function:   SplitDevIds
//
//  Synopsis:   Splits the sting of ";" separated Device Ids in to String array
//
//  Arguments:   [inputDevIdList] - The list of device ids in string format separated by semicolon
//               [numDevIds] -    The number of device ids 
//               [lptstrDevIdArr] - Array of strings each containing a single device id
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool SplitDevIds(const LPTSTR inputDevIdList, int* numDevIds, LPTSTR** lptstrDevIdArr)
{
        bool bRetVal = false;
        if(NULL == inputDevIdList || NULL == numDevIds || NULL == lptstrDevIdArr )
        {
                return false;
        }

        HRESULT hr = S_OK;      
        int numDevs = 0;
        int count =0;

        size_t iStrLen = 0;
        hr = StringCchLength( inputDevIdList,  2056 , &iStrLen);

        if(FAILED(hr))
        {
                _tprintf(_T("StringCchLength of inputDevIdList failed.  Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        LPTSTR devListString = (TCHAR*) malloc(sizeof(TCHAR) * (iStrLen+1));
        if(devListString == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc of devListString failed. Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        memset(devListString, 0, iStrLen+1);
        hr = StringCchCopy(devListString, (iStrLen+1),inputDevIdList );
        if(FAILED(hr))
        {
                _tprintf(_T("StringCchCopy of devListString failed. Error 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }
        //Get the number of DevIds first.
        size_t i = 0;
        while(i < iStrLen)
        {
                if(devListString[i] == ';')
                {
                        numDevs= numDevs + 1;
                }
                i++;
        }
        numDevs = numDevs+1;
        *lptstrDevIdArr = new LPTSTR [numDevs];
        if(*lptstrDevIdArr == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("New of lptstrDevIdArr failed. Error 0x%x"),hr);
                bRetVal = false;
                goto Exit;
        }   
        memset(*lptstrDevIdArr, 0, numDevs);

        i = 0;
        size_t end = 0;
        size_t start = 0;
        //get the array
        while(i <= iStrLen)
        {
                if(devListString[i] == ';' || i == iStrLen)
                {
                        end = i;
                        size_t len = end - start ;
                        if(len == 0)
                        {                            
                                _tprintf(_T("Input string is not properly separated \n"));
                                bRetVal = false;
                                goto Exit1;
                        }

                        TCHAR* strTemp = &devListString[start];
                        (*lptstrDevIdArr)[count] = (TCHAR*)malloc((len+1) * sizeof(TCHAR));
                        if((*lptstrDevIdArr)[count] == NULL)
                        {
                                hr = E_OUTOFMEMORY;
                                _tprintf(_T("malloc of (*lptstrDevIdArr)[count] failed. Error 0x%x"), hr);
                                bRetVal = false;
                                goto Exit1;
                        }
                        memset((*lptstrDevIdArr)[count], 0, (len+1));
                        hr = StringCchCopyN((*lptstrDevIdArr)[count], len+1, strTemp, len);
                        if(FAILED(hr))
                        {
                                _tprintf(_T("StringCchCopyN of lptstrDevIdArr on count %i failed. Error 0x%x "), count, hr);
                                bRetVal = false;
                                goto Exit1;
                        }
                        start = i+1;
                        count++;
                }            
                i++;
        }
        *numDevIds = numDevs;
        bRetVal = true;
        goto Exit;
Exit1:

        for(int j =0;j < count; j++)
        {
                if((*lptstrDevIdArr) != NULL)
                {
                        if((*lptstrDevIdArr)[j])
                                free((*lptstrDevIdArr)[j]);
                }
        }

        if((*lptstrDevIdArr) != NULL)
                delete[] (*lptstrDevIdArr); 

Exit:
        return bRetVal;
}
//+---------------------------------------------------------------------------
//
//  function:   addGroup
//
//  Synopsis:   adds a routing group to FaxOutboundRoutingGroups
//
//  Arguments:  [pFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
//                [lptstrGrpName] - Routing Group Name
//                [lptstrDevIds] - device ids for the new group
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool addGroup(IFaxOutboundRoutingGroups* pFaxOutRoutGrps, LPTSTR lptstrGrpName, LPTSTR lptstrDevIds)
{
        HRESULT  hr = S_OK;
        bool bRetVal = false;    
        long lCount =0; 
        int iDevCount=0;
        LPTSTR* devArr = NULL;
        BSTR bstrGrpName = SysAllocString(lptstrGrpName);
        IFaxOutboundRoutingGroup* pFaxOutRoutGrp = NULL;
        IFaxDeviceIds* pFaxDevIds = NULL;
        //check for NULL
        if (pFaxOutRoutGrps == NULL || lptstrGrpName == NULL || lptstrDevIds == NULL) 
        {
                _tprintf(_T("addGroup: Parameter passed is NULL"));
                bRetVal = false;
                goto Exit;
        }  

        //check for NULL
        if (bstrGrpName == NULL ) 
        {
                _tprintf(_T("addRule: bstrGrpName is NULL"));
                bRetVal = false;
                goto Exit;
        }  

        hr = pFaxOutRoutGrps->Add(bstrGrpName, &pFaxOutRoutGrp);
        if(FAILED(hr))
        {
                _tprintf(_T("addGroup: Add failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        SplitDevIds(lptstrDevIds, &iDevCount, &devArr);
        hr = pFaxOutRoutGrp->get_DeviceIds(&pFaxDevIds);
        if(FAILED(hr))
        {
                _tprintf(_T("addGroup: get_DeviceIds Failed. Error. 0x%x \n"), hr);
                bRetVal = false;
                goto Exit;
        }
        for(int i = 0; i < iDevCount ; i++)
        {
                hr = pFaxDevIds->Add(_ttoi(devArr[i]));                
                if(FAILED(hr))
                {
                        _tprintf(_T("addGroup: Add failed. Error = 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }
        }
        _tprintf(_T("Group added successfully. \n"));
        bRetVal = true;     

Exit: 
        for(int i =0;i < iDevCount; i++)
        {
                if((devArr)[i])
                        free((devArr)[i]);
        }

        if(devArr)
                delete devArr; 
        return bRetVal;

}
//+---------------------------------------------------------------------------
//
//  function:   listRules
//
//  Synopsis:   list of Routing Rules on the Server
//
//  Arguments:  [pFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool listRules(IFaxOutboundRoutingRules* pFaxOutRoutRules)
{
        HRESULT  hr = S_OK;
        bool bRetVal = true;    
        long lCount =0; 
        IFaxOutboundRoutingRule* pFaxOutRoutRule = NULL;

        //check for NULL
        if (pFaxOutRoutRules == NULL) 
        {
                _tprintf(_T("listRules: Parameter passed is NULL"));
                goto Exit;
        }

        _tprintf(_T("\n Listing Routing Rule details....\n \n"));

        hr = pFaxOutRoutRules->get_Count(&lCount);
        if(FAILED(hr))
        {
                _tprintf(_T("Count failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        for(int i =1; i<= lCount; i++)
        {                
                long lDeviceId = 0;
                long lAreaCode = 0;
                long lCountryCode = 0;
                VARIANT_BOOL bUseDevice = VARIANT_FALSE;
                FAX_RULE_STATUS_ENUM enumStatus;
                BSTR bstrGrpName = NULL;
                hr = pFaxOutRoutRules->get_Item(i,&pFaxOutRoutRule);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_Item Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }
                hr = pFaxOutRoutRule->get_AreaCode(&lAreaCode);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_AreaCode Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }

                hr = pFaxOutRoutRule->get_CountryCode(&lCountryCode);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_CountryCode Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }

                hr = pFaxOutRoutRule->get_DeviceId(&lDeviceId);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_DeviceId Failed. Error. 0x%x \n"), hr);
                        goto Exit;
                }


                hr = pFaxOutRoutRule->get_GroupName(&bstrGrpName);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_GroupName Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }

                hr = pFaxOutRoutRule->get_UseDevice(&bUseDevice);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_UseDevice Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }

                hr = pFaxOutRoutRule->get_Status(&enumStatus);
                if(FAILED(hr))
                {
                        _tprintf(_T("listRules: get_Status Failed. Error. 0x%x \n"), hr);
                        goto Exit;            
                }

                _tprintf(_T(" ===================================================\n"));  

                _tprintf(_T("Rule No: %d Group Name = %s \n"), i, bstrGrpName);  

                if(lAreaCode ==  frrcANY_CODE)
                        _tprintf(_T("Area Code: The outbound routing rule applies to all area codes. \n"));
                else
                        _tprintf(_T("Area Code: = %d \n"),  lAreaCode);  

                if(lCountryCode ==  frrcANY_CODE)
                        _tprintf(_T("Country Code: The outbound routing rule applies to all area codes. \n"));  
                else
                        _tprintf(_T("Country Code: = %d \n"), lCountryCode);  

                _tprintf(_T("Associated Device Id: = %d \n"), lDeviceId);  

                if(bUseDevice == VARIANT_TRUE)
                        _tprintf(_T("Applies to single device \n"));  
                else 
                        _tprintf(_T("Applies to group of devices. \n"));  


                if(enumStatus ==  frsVALID )
                        _tprintf(TEXT("Status : The routing rule is valid and can be applied to outbound faxes. \n"));
                if(enumStatus == frsEMPTY_GROUP )
                        _tprintf(TEXT("Status : The routing rule cannot be applied because the rule uses an outbound routing group for its destination and the group is empty. \n") );
                if(enumStatus ==  frsALL_GROUP_DEV_NOT_VALID)
                        _tprintf(TEXT("Status : The routing rule cannot be applied because the rule uses an existing outbound routing group for its destination and the group does not contain devices that are valid for sending faxes. \n"));
                if(enumStatus == frsSOME_GROUP_DEV_NOT_VALID)
                        _tprintf(TEXT("Status : The routing rule uses an existing outbound routing group for its destination but the group contains devices that are not valid for sending faxes. \n") );
                if(enumStatus == frsBAD_DEVICE)
                        _tprintf(TEXT("Status : The routing rule cannot be applied because the rule uses a single device for its destination and that device is not valid for sending faxes. \n") );

                if(bstrGrpName)
                        SysFreeString(bstrGrpName);
        }    

Exit: 
        return bRetVal;
}




//+---------------------------------------------------------------------------
//
//  function:   removeRule
//
//  Synopsis:   removes a routing rule from FaxOutboundRoutingRules based on index
//
//  Arguments:  [pFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
//                [index] - index of the group to be removed
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool removeRule(IFaxOutboundRoutingRules* pFaxOutRoutRules, int index)
{
        HRESULT  hr = S_OK;
        bool bRetVal = false;    
        long lCount =0; 
        //check for NULL
        if (pFaxOutRoutRules == NULL) 
        {
                _tprintf(_T("removeRule: Parameter passed is NULL"));
                bRetVal = false;
                goto Exit;
        }  

        //get the count
        hr = pFaxOutRoutRules->get_Count(&lCount);
        if(FAILED(hr))
        {
                _tprintf(_T("removeRule: Count failed. Error = 0x%x"), hr);
                bRetVal = false;
                goto Exit;
        }

        //check if valid
        if(index > lCount || index <=0)
        {
                _tprintf(_T("removeRule: Invalid index value. It can be from 1 to %d.\n" ), lCount);
                bRetVal = false;
                goto Exit;
        }

        //remove rule
        hr = pFaxOutRoutRules->Remove(index);
        if(FAILED(hr))
        {
                _tprintf(_T("removeRule: Remove Failed. Error. 0x%x \n"), hr);
                bRetVal = false;
                goto Exit;
        }

        _tprintf(_T("Rule removed successfully. \n"));
        bRetVal = true;     

Exit: 
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   addRule
//
//  Synopsis:   adds a routing rule to FaxOutboundRoutingRules
//
//  Arguments:  [pFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
//                [lptstrGrpName] - Routing Group Name
//                [lptstrDevIds] - device ids for the new group
//
//  Returns:    bool: true if passed successfully
//
//----------------------------------------------------------------------------
bool addRule(IFaxOutboundRoutingRules* pFaxOutRoutRules, LPTSTR lptstrGrpName, LPTSTR lptstrDevId, LPTSTR lptstrCountryCode, LPTSTR lptstrAreaCode, VARIANT_BOOL bUseDevice )
{
        HRESULT  hr = S_OK;
        bool bRetVal = false;    
        long lCount =0; 
        int iDevCount=0;
        int iDevId = -1;
        LPTSTR* devArr = NULL;
        BSTR bstrGrpName = SysAllocString(lptstrGrpName);
        IFaxOutboundRoutingRule* pFaxOutRoutRule = NULL;
        IFaxDeviceIds* pFaxDevIds = NULL;
        //check for NULL
        if (pFaxOutRoutRules == NULL || lptstrCountryCode == NULL || lptstrAreaCode == NULL || (lptstrGrpName == NULL && lptstrDevId == NULL ) ) 
        {
                _tprintf(_T("addRule: Parameter passed is NULL"));
                bRetVal = false;
                goto Exit;
        }  

        //check for NULL
        if ((bstrGrpName != NULL && !bUseDevice)  || ( lptstrDevId != NULL && bUseDevice) ) 
        {


                if(lptstrDevId != NULL)
                        iDevId = _ttoi(lptstrDevId);
                //Set Area Code and Country Code for all codes
                hr = pFaxOutRoutRules->Add(_ttoi(lptstrCountryCode),_ttoi(lptstrAreaCode),bUseDevice,bstrGrpName, iDevId, &pFaxOutRoutRule);
                if(FAILED(hr))
                {
                        _tprintf(_T("addRule: Add failed. Error = 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }    
        }
        else
        {
                _tprintf(_T("addRule: Parameter is NULL"));
                bRetVal = false;
                goto Exit;
        }  
        _tprintf(_T("Rule added successfully. \n"));
        bRetVal = true;     

Exit: 
        return bRetVal;

}
int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;        
        LPTSTR lptstrIndex = NULL;        
        LPTSTR lptstrName = NULL;
        LPTSTR lptstrCountryCode = NULL;        
        LPTSTR lptstrAreaCode = NULL;
        LPTSTR lptstrIds = NULL;
        LPTSTR lptstrOption = NULL;
        BSTR bstrServerName = NULL;
        LPTSTR lptstrUseDev = NULL;
        bool bConnected = false;
        size_t argSize = 0;
        VARIANT_BOOL vbState = VARIANT_FALSE;
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("OS Version does not support this feature"));
                bRetVal = false;
                goto Exit1;
        }


        //introducing an artifical scope here so that the COm objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer = NULL;
                IFaxOutboundRouting* pFaxOutRout = NULL;
                IFaxOutboundRoutingGroups* pFaxOutRoutGrps =NULL;
                IFaxOutboundRoutingRules* pFaxOutRoutRules = NULL;

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
                                                        case 'o':
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

                                                        case 'n':
                                                                if(lptstrName == NULL)
                                                                {
                                                                        lptstrName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrName, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrName, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrName: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                        case 'd':
                                                                if(lptstrIds == NULL)
                                                                {
                                                                        lptstrIds = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrIds == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrIds: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrIds, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrIds, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrIds: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                                if(lptstrIndex == NULL)
                                                                {
                                                                        lptstrIndex = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrIndex == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrIndex: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrIndex, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrIndex, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrIndex: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                                if(lptstrCountryCode == NULL)
                                                                {
                                                                        lptstrCountryCode = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrCountryCode == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrCountryCode: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrCountryCode, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrCountryCode, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrCountryCode: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                        case 'a':
                                                                if(lptstrAreaCode == NULL)
                                                                {
                                                                        lptstrAreaCode = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrAreaCode == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrAreaCode: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrAreaCode, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrAreaCode, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrAreaCode: StringCchCopyN failed. Error 0x%x \n"), hr);
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
                                                        case 'b':
                                                                if(lptstrUseDev == NULL)
                                                                {
                                                                        lptstrUseDev = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrUseDev == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrUseDev: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrUseDev, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrUseDev, argSize + 1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrUseDev: StringCchCopyN failed. Error 0x%x \n"), hr);
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

                if (lptstrOption == NULL) 
                {
                        _tprintf( TEXT("Missing/Invalid Value.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                if ((lptstrIndex == NULL ) && ((_tcscmp(_T("removegroup"), CharLower(lptstrOption)) == 0) || ((_tcscmp(_T("removerule"), CharLower(lptstrOption)) == 0))))
                {
                        _tprintf( TEXT("Missing/Invalid Value.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                if (((lptstrName == NULL ) || (lptstrIds == NULL ) ) && ((_tcscmp(_T("addgroup"), CharLower(lptstrOption)) == 0)))
                {
                        _tprintf( TEXT("Missing/Invalid Value.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                //if addrule and lptstrUseDev is not set
                if (((lptstrUseDev == NULL ) || ((_tcscmp(_T("1"), lptstrUseDev) != 0) && (_tcscmp(_T("0"), lptstrUseDev) != 0)) ) && (((_tcscmp(_T("addrule"), CharLower(lptstrOption)) == 0))))
                {
                        _tprintf( TEXT("Set /b tag to 0 or 1.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                //if UseDev = 1 then set lptstrIds
                //if UseDev = 0 then set lptstrName
                if(lptstrUseDev != NULL )
                {
                        if ((((_tcscmp(_T("0"), lptstrUseDev) == 0) && lptstrName == NULL ) || ((_tcscmp(_T("1"), lptstrUseDev) == 0)  && (lptstrIds == NULL)) || (lptstrAreaCode == NULL ) || (lptstrCountryCode == NULL ) ) && (((_tcscmp(_T("addrule"), CharLower(lptstrOption)) == 0))))
                        {
                                _tprintf( TEXT("Missing/Invalid Value.\n") );
                                GiveUsage(argv[0]);
                                bRetVal = false;
                                goto Exit;
                        }
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
                        _tprintf(_T("OS Version does not support this feature"));
                        goto Exit;
                }         

                //Get Routing object
                hr = pFaxServer->get_OutboundRouting(&pFaxOutRout);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_OutboundRouting failed. Error 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //Get Groups object
                hr = pFaxOutRout->GetGroups(&pFaxOutRoutGrps);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetGroups failed. Error 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //Get Rules object
                hr = pFaxOutRout->GetRules(&pFaxOutRoutRules);
                if(FAILED(hr))
                {
                        _tprintf(_T("GetRules failed. Error 0x%x"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                //list groups
                if(_tcscmp(_T("listgroups"), CharLower(lptstrOption)) == 0)
                {
                        if(!listGroups(pFaxOutRoutGrps))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }

                //list rules
                if(_tcscmp(_T("listrules"), CharLower(lptstrOption)) == 0)
                {
                        if(!listRules(pFaxOutRoutRules))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }    
                //remove group
                if(_tcscmp(_T("removegroup"), CharLower(lptstrOption)) == 0)
                {
                        if(!removeGroup(pFaxOutRoutGrps,_ttoi(lptstrIndex)))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }

                //remove rules
                if(_tcscmp(_T("removerule"), CharLower(lptstrOption)) == 0)
                {
                        if(!removeRule(pFaxOutRoutRules, _ttoi(lptstrIndex)))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }        

                //add groups
                if(_tcscmp(_T("addgroup"), CharLower(lptstrOption)) == 0)
                {
                        if(!addGroup(pFaxOutRoutGrps, lptstrName, lptstrIds))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
                        }
                }    

                //add rules
                VARIANT_BOOL bUseDevice;
                if(_tcscmp(_T("addrule"), CharLower(lptstrOption)) == 0)
                {
                        if(_tcscmp(_T("0"), lptstrUseDev) == 0)                
                        {
                                bUseDevice = VARIANT_FALSE;
                        }
                        else
                        {
                                bUseDevice = VARIANT_TRUE;
                        }
                        if(!addRule(pFaxOutRoutRules, lptstrName, lptstrIds, lptstrCountryCode, lptstrAreaCode, bUseDevice))
                        {
                                //we dont want to log any error here as the error will be logged in the function itself
                                bRetVal = false;
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
                if(lptstrName)
                        free(lptstrName);
                if(lptstrIds)
                        free(lptstrIds);
                if(lptstrIndex)
                        free(lptstrIndex);
                if(lptstrUseDev)
                        free(lptstrUseDev );
                if(lptstrAreaCode )
                        free(lptstrAreaCode);
                if(lptstrCountryCode)
                        free(lptstrCountryCode);
                if(bstrServerName)
                        SysFreeString(bstrServerName);
        }
        CoUninitialize();
Exit1:
        return bRetVal;

}
