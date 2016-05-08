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

using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using FAXCOMEXLib;


[assembly: CLSCompliant(true)]
namespace Microsoft.Samples.Fax.OutboundRouting.CS
{
        class OutboundRouting
        {
                //+---------------------------------------------------------------------------
                //
                //  function:   GiveUsage
                //
                //  Synopsis:   prints the usage of the application
                //
                //  Arguments:  void
                //
                //  Returns:    void
                //
                //----------------------------------------------------------------------------
                static void GiveUsage()
                {
                        System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName);
                        System.Console.WriteLine(" /s Fax Server Name ");
                        System.Console.WriteLine(" /o <listGroups/ListRules/RemoveGroup/AddGroup/RemoveRule/AddRule> ");
                        System.Console.WriteLine(" /i index of the group or rule to be removed. valid values 0 to n ");
                        System.Console.WriteLine(" /n name of the new routing rule or group  ");
                        System.Console.WriteLine(" /c country code for the new routing rule");
                        System.Console.WriteLine(" /a area code for the new routing rule ");
                        System.Console.WriteLine(" /b 1 to use the device id for the new routing rule else 0 ");
                        System.Console.WriteLine(" /d device id for the routing rule and list of device ids separated by \";\" for routing group ");                        
                        System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName + " /? -- help message");
                }
                //+---------------------------------------------------------------------------
                //
                //  function:    IsOSVersionCompatible
                //
                //  Synopsis:    finds whether the target OS supports this functionality.
                //
                //  Arguments:  [iVersion] - Minimum Version of the OS required for the Sample to run.
                //
                //  Returns:     bool - true if the Sample can run on this OS
                //
                //----------------------------------------------------------------------------
                static bool IsOSVersionCompatible(int iVersion)
                {
                        OperatingSystem os = Environment.OSVersion;
                        Version osVersion = os.Version;
                        if (osVersion.Major >= iVersion)
                                return true;
                        else
                                return false;
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
                static bool listDeviceIds(IFaxDeviceIds objFaxDevIds)
                {
                        long lDevId = 0;
                        long count = 0;
                        bool bRetVal = false;

                        //check for null
                        if (objFaxDevIds == null)
                        {
                                System.Console.WriteLine("listDevices: Parameter passed is null");
                                goto Exit;
                        }

                        count = objFaxDevIds.Count;                                         
                        for (int i = 0; i < count; i++)
                        {
                                lDevId = objFaxDevIds[i+1];
                                System.Console.Write("Device No: ");
                                System.Console.Write(i);
                                System.Console.Write(" Device Id: ");
                                System.Console.Write(lDevId);
                                System.Console.WriteLine();
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
                //  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool listGroups(IFaxOutboundRoutingGroups objFaxOutRoutGrps)
                {        
                        bool bRetVal = true;    
                        long lCount =0; 
                        IFaxOutboundRoutingGroup objFaxOutRoutGrp = null;
                        string strGrpName = null;

                        //check for null
                        if (objFaxOutRoutGrps == null) 
                        {
                                System.Console.WriteLine("listGroups: Parameter passed is null");
                                goto Exit;
                        }

                        System.Console.WriteLine(" Listing Routing Group details....");

                        lCount = objFaxOutRoutGrps.Count;

                        //enumerate
                        for(int i =0; i< lCount; i++)
                        {                
                                IFaxDeviceIds objFaxDevIds = null;
                                FAX_GROUP_STATUS_ENUM enumStatus;
                                objFaxOutRoutGrp = objFaxOutRoutGrps[i+1];
                                objFaxDevIds = objFaxOutRoutGrp.DeviceIds; 
                                strGrpName = objFaxOutRoutGrp.Name;                
                                enumStatus = objFaxOutRoutGrp.Status;

                                //print all the details
                                System.Console.WriteLine(" ===================================================");  
                                System.Console.Write("Group No: ");
                                System.Console.Write(i+1);                    

                                System.Console.WriteLine("Group Name = ");
                                System.Console.WriteLine(strGrpName);  

                                if(enumStatus == FAX_GROUP_STATUS_ENUM.fgsALL_DEV_VALID )
                                        System.Console.WriteLine("Status : All the devices in the routing group are valid and available for sending outgoing faxes. " );
                                if (enumStatus == FAX_GROUP_STATUS_ENUM.fgsEMPTY)
                                        System.Console.WriteLine("Status : The routing group does not contain any devices. " );
                                if (enumStatus == FAX_GROUP_STATUS_ENUM.fgsALL_DEV_NOT_VALID)
                                        System.Console.WriteLine("Status : The routing group does not contain any available devices for sending faxes. (Devices can be \"unavailable\" when they are offline and when they do not exist.) " );
                                if (enumStatus == FAX_GROUP_STATUS_ENUM.fgsSOME_DEV_NOT_VALID)
                                        System.Console.WriteLine("Status : The routing group contains some devices that are unavailable for sending faxes. (Devices can be \"unavailable\" when they are offline and when they do not exist.) " );

                                if(!listDeviceIds(objFaxDevIds))
                                {
                                        //we dont want to log any error here as the error will be logged in the function itself
                                        bRetVal = false;
                                }
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
                //  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
                //                [index] - index of the group to be removed
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool removeGroup(IFaxOutboundRoutingGroups objFaxOutRoutGrps, int index)
                {
                        bool bRetVal = false;    
                        long lCount =0; 
                        //check for null
                        if (objFaxOutRoutGrps == null) 
                        {
                                System.Console.WriteLine("removeGroup: Parameter passed is null");
                                bRetVal = false;
                                goto Exit;
                        }  

                        //get count of groups
                        lCount = objFaxOutRoutGrps.Count;

                        //invalid index
                        if(index > lCount || index <=0)
                        {
                                System.Console.Write("removeGroup: Invalid index value. It can be from 1 to ");
                                System.Console.WriteLine(lCount);
                                bRetVal = false;
                                goto Exit;
                        }

                        //remove group
                        objFaxOutRoutGrps.Remove(index);
                        System.Console.WriteLine("Group removed successfully. ");
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
                //               [bRetVal] - Array of strings each containing a single device id
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static string[] SplitDevIds(string inputDevIdList, ref int numDevIds, ref bool bRetVal )
                {
                        bRetVal = false;
                        if (String.IsNullOrEmpty(inputDevIdList))
                        {
                                return null;
                        }                        
                        string strDelimiter = ";";
                        char[] delimiter = strDelimiter.ToCharArray();
                        string[] devIdStrArray = inputDevIdList.Split(delimiter);
                        numDevIds = devIdStrArray.Length;
                        bRetVal = true;
                        return devIdStrArray;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   addGroup
                //
                //  Synopsis:   adds a routing group to FaxOutboundRoutingGroups
                //
                //  Arguments:  [objFaxOutRoutGrps] - FaxOutboundRoutingGroups object pointing to the Routing Groups of the current server
                //                [strGrpName] - Routing Group Name
                //                [strDevIds] - device ids for the new group
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static   bool addGroup(IFaxOutboundRoutingGroups objFaxOutRoutGrps, string strGrpName, string strDevIds)
                {        
                        bool bRetVal = false;
                        bool bRet = false;                        
                        int iDevCount=0;
                        IFaxOutboundRoutingGroup objFaxOutRoutGrp = null;
                        IFaxDeviceIds objFaxDevIds = null;
                        //check for NULL
                        if (objFaxOutRoutGrps == null || strGrpName == null || strDevIds == null)
                        {
                                System.Console.WriteLine("addGroup: Parameter passed is null");
                                bRetVal = false;
                                goto Exit;
                        }

                        objFaxOutRoutGrp = objFaxOutRoutGrps.Add(strGrpName);
                        string[] devArr = SplitDevIds(strDevIds, ref iDevCount, ref bRet);
                        objFaxDevIds = objFaxOutRoutGrp.DeviceIds;
                        for(int i = 0; i < iDevCount ; i++)
                        {
                                int iVal = Int32.Parse(devArr[i], CultureInfo.CurrentCulture.NumberFormat);                            
                                objFaxDevIds.Add(iVal);
                        }
                        System.Console.WriteLine("Group added successfully. ");
                        bRetVal = true;     
Exit: 
                        return bRetVal;

                }
                //+---------------------------------------------------------------------------
                //
                //  function:   listRules
                //
                //  Synopsis:   list of Routing Rules on the Server
                //
                //  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static  bool listRules(IFaxOutboundRoutingRules objFaxOutRoutRules)
                {
                        bool bRetVal = true;    
                        long lCount =0; 
                        IFaxOutboundRoutingRule objFaxOutRoutRule = null;

                        //check for null
                        if (objFaxOutRoutRules == null) 
                        {
                                System.Console.WriteLine("listRules: Parameter passed is null");
                                goto Exit;
                        }

                        System.Console.WriteLine(" Listing Routing Rule details.... ");

                        lCount = objFaxOutRoutRules.Count;

                        for(int i =0; i< lCount; i++)
                        {                
                                long lDeviceId = 0;
                                long lAreaCode = 0;
                                long lCountryCode = 0;
                                bool bUseDevice = false;
                                FAX_RULE_STATUS_ENUM enumStatus;
                                string  strGrpName = null;

                                objFaxOutRoutRule = objFaxOutRoutRules[i+1] ; 
                                lAreaCode = objFaxOutRoutRule.AreaCode; 
                                lCountryCode = objFaxOutRoutRule.CountryCode;               
                                lDeviceId = objFaxOutRoutRule.DeviceId;
                                strGrpName = objFaxOutRoutRule.GroupName;
                                bUseDevice = objFaxOutRoutRule.UseDevice;                
                                enumStatus = objFaxOutRoutRule.Status;

                                System.Console.WriteLine(" ===================================================");  
                                System.Console.Write("Rule No: ");
                                System.Console.Write(i+1);
                                System.Console.Write("Group Name = ");
                                System.Console.WriteLine(strGrpName);

                                if(lAreaCode == (long) FAX_ROUTING_RULE_CODE_ENUM.frrcANY_CODE)
                                        System.Console.WriteLine("Area Code: The outbound routing rule applies to all area codes. ");
                                else
                                {
                                        System.Console.Write("Area Code: ");
                                        System.Console.WriteLine(lAreaCode);
                                }


                                if (lCountryCode == (long) FAX_ROUTING_RULE_CODE_ENUM.frrcANY_CODE)
                                        System.Console.WriteLine("Country Code: The outbound routing rule applies to all area codes. ");  
                                else
                                {
                                        System.Console.Write("Country Code: ");
                                        System.Console.WriteLine(lCountryCode);
                                }

                                System.Console.Write("Associated Device Id: = ");
                                System.Console.WriteLine(lDeviceId);  

                                if(bUseDevice == true)
                                        System.Console.WriteLine("Applies to single device ");  
                                else 
                                        System.Console.WriteLine("Applies to group of devices. ");


                                if (enumStatus == FAX_RULE_STATUS_ENUM.frsVALID)
                                        System.Console.WriteLine("Status : The routing rule is valid and can be applied to outbound faxes. ");
                                if(enumStatus ==  FAX_RULE_STATUS_ENUM.frsEMPTY_GROUP )
                                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses an outbound routing group for its destination and the group is empty. ") ;
                                if(enumStatus ==   FAX_RULE_STATUS_ENUM.frsALL_GROUP_DEV_NOT_VALID)
                                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses an existing outbound routing group for its destination and the group does not contain devices that are valid for sending faxes. ");
                                if (enumStatus == FAX_RULE_STATUS_ENUM.frsSOME_GROUP_DEV_NOT_VALID)
                                        System.Console.WriteLine("Status : The routing rule uses an existing outbound routing group for its destination but the group contains devices that are not valid for sending faxes. ") ;
                                if (enumStatus == FAX_RULE_STATUS_ENUM.frsBAD_DEVICE)
                                        System.Console.WriteLine("Status : The routing rule cannot be applied because the rule uses a single device for its destination and that device is not valid for sending faxes. ") ;
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
                //  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
                //                [index] - index of the group to be removed
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool removeRule(IFaxOutboundRoutingRules objFaxOutRoutRules, int index)
                {                        
                        bool bRetVal = false;    
                        long lCount =0; 
                        //check for null
                        if (objFaxOutRoutRules == null) 
                        {
                                System.Console.WriteLine("removeRule: Parameter passed is null");
                                bRetVal = false;
                                goto Exit;
                        }  

                        //get the count
                        lCount = objFaxOutRoutRules.Count;
                        //check if valid
                        if(index > lCount || index <=0)
                        {
                                System.Console.Write("removeRule: Invalid index value. It can be from 1 to ");
                                System.Console.WriteLine(lCount);
                                bRetVal = false;
                                goto Exit;
                        }

                        //remove rule
                        objFaxOutRoutRules.Remove(index);
                        System.Console.WriteLine("Rule removed successfully. ");
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
                //  Arguments:  [objFaxOutRoutRules] - FaxOutboundRoutingRules object pointing to the Routing Rules of the current server
                //                [strGrpName] - Routing Group Name
                //                [strDevId] - device ids for the new group
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static   bool addRule(IFaxOutboundRoutingRules objFaxOutRoutRules, string strGrpName, string strDevId, string strCountryCode , string strAreaCode, bool bUseDevice)
                {                        
                        bool bRetVal = false;                                                   
                        IFaxOutboundRoutingRule objFaxOutRoutRule = null;

                        //check for null
                        if (objFaxOutRoutRules == null || (strGrpName == null && bUseDevice == false) || (strDevId == null && bUseDevice == true ) || strCountryCode == null || strAreaCode == null) 
                        {
                                System.Console.WriteLine("addRule: Parameter passed is null");
                                bRetVal = false;
                                goto Exit;
                        } 
                        int iDevId = -1;
                        if(strDevId != null)
                            iDevId = Int32.Parse(strDevId, CultureInfo.CurrentCulture.NumberFormat);
                            
                        //Set Area Code and Country Code for all codes
                        objFaxOutRoutRule =  objFaxOutRoutRules.Add(Int32.Parse(strCountryCode, CultureInfo.CurrentCulture.NumberFormat), Int32.Parse(strAreaCode, CultureInfo.CurrentCulture.NumberFormat), bUseDevice, strGrpName, iDevId);                 
                        System.Console.WriteLine("Rule added successfully. ");
                        bRetVal = true;     
Exit: 
                        return bRetVal;

                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxOutboundRoutingGroups objFaxOutRoutGrps = null;
                        FAXCOMEXLib.IFaxOutboundRoutingRules objFaxOutRoutRules = null;
                        string strServerName = null;
                        string strName = null;
                        string strIndex = null;
                        string strIds = null;
                        string strOption = null;
                        string strCountryCode = null;
                        string strAreaCode = null;
                        string strUseDev = null;
                        bool bConnected = false;                        
                        bool bRetVal = true;

                        int iVista = 6;
                        bool bVersion = IsOSVersionCompatible(iVista);

                        if (bVersion == false)
                        {
                                System.Console.WriteLine("This sample is compatible with Windows Vista");
                                bRetVal = false;
                                goto Exit;
                        }

                        try
                        {
                                if ((args.Length == 0))
                                {
                                        System.Console.WriteLine("Missing args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }

                                // check for commandline switches
                                for (int argcount = 0; argcount < args.Length; argcount++)
                                {
                                        if (argcount + 1 < args.Length)
                                        {
                                                if ((args[argcount][0] == '/') || (args[argcount][0] == '-'))
                                                {
                                                        switch (((args[argcount].ToLower(CultureInfo.CurrentCulture))[1]))
                                                        {
                                                                case 's':
                                                                        if (strServerName != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strServerName = args[argcount + 1];                                     
                                                                        argcount++;
                                                                        break;
                                                                case 'o':
                                                                        if (strOption != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strOption = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'n':
                                                                        if (strName != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strName = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'd':
                                                                        if (strIds != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strIds = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'i':
                                                                        if (strIndex != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strIndex = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'a':
                                                                        if (strAreaCode != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strAreaCode = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'c':
                                                                        if (strCountryCode != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strCountryCode = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'b':
                                                                        if (strUseDev != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strUseDev = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case '?':
                                                                        GiveUsage();
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                default:
                                                                        break;
                                                        }//switch
                                                }//if 
                                        }//if (argcount + 1 < argc)
                                }//for

                                if (strOption == null)
                                {
                                        System.Console.WriteLine("Missing/Invalid args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }  

                                if(((strName == null) || (strIds == null)) &&  ((String.Compare("addgroup", strOption, true, CultureInfo.CurrentCulture) == 0)))
                                {
                                        System.Console.WriteLine("Missing/Invalid args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }

                                //if addrule and strUseDev is not set
                                if (((strUseDev == null) || ((String.Compare("1", strUseDev,true,CultureInfo.CurrentCulture) != 0) && (String.Compare("0", strUseDev,true,CultureInfo.CurrentCulture) != 0))) && (((String.Compare("addrule", strOption,true,CultureInfo.CurrentCulture) == 0))))
                                {
                                        System.Console.WriteLine("Set /b tag to 0 or 1.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }



                                if ((strIndex == null) && ((String.Compare("removegroup", strOption, true, CultureInfo.CurrentCulture) == 0) && (String.Compare("removerule", strOption, true, CultureInfo.CurrentCulture) == 0)))
                                {
                                        System.Console.WriteLine("Missing/Invalid args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }

                                //if UseDev = 1 then set lptstrIds
                                //if UseDev = 0 then set lptstrName
                                if (strUseDev != null)
                                {
                                        if ((((String.Compare(strUseDev, "0", true, CultureInfo.CurrentCulture) == 0 ) && (strName == null)) || ((String.Compare(strUseDev, "1", true, CultureInfo.CurrentCulture) == 0 ) && (strIds == null)) || (strCountryCode == null) || (strAreaCode == null)) && ((String.Compare("addrule", strOption, true, CultureInfo.CurrentCulture) == 0)))
                                        {
                                                System.Console.WriteLine("Missing/Invalid args.");
                                                GiveUsage();
                                                bRetVal = false;
                                                goto Exit;
                                        }
                                }

                                //Connect to Fax Server               
                                objFaxServer = new FaxServerClass();
                                objFaxServer.Connect(strServerName);
                                bConnected = true;

                                //Check the API version
                                if (objFaxServer.APIVersion < FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3)
                                {
                                        bRetVal = false;
                                        System.Console.WriteLine("This sample is compatible with Windows Vista");
                                        goto Exit;
                                }

                                objFaxOutRoutGrps = objFaxServer.OutboundRouting.GetGroups();
                                objFaxOutRoutRules = objFaxServer.OutboundRouting.GetRules();                                 

                                //list groups
                                if (String.Compare("listgroups", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!listGroups(objFaxOutRoutGrps))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //list rules
                                if (String.Compare("listrules", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!listRules(objFaxOutRoutRules))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //remove group
                                if (String.Compare("removegroup", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!removeGroup(objFaxOutRoutGrps, Int32.Parse(strIndex, CultureInfo.CurrentCulture.NumberFormat)))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //remove rule
                                if (String.Compare("removerule", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!removeRule(objFaxOutRoutRules, Int32.Parse(strIndex, CultureInfo.CurrentCulture.NumberFormat)))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //add group
                                if (String.Compare("addgroup", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!addGroup(objFaxOutRoutGrps,strName,strIds))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //add rule
                                if (String.Compare("addrule", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        bool bUseDevice = false;
                                        if (String.Compare("0", strUseDev,true,CultureInfo.CurrentCulture) == 0)
                                        {
                                                bUseDevice = false;
                                        }
                                        else
                                        {
                                                bUseDevice = true;
                                        }
                                        if (!addRule(objFaxOutRoutRules, strName, strIds, strCountryCode, strAreaCode, bUseDevice))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }

                        }
                        catch (Exception excep)
                        {
                                System.Console.WriteLine("Exception Occured");
                                System.Console.WriteLine(excep.Message);
                        }
Exit:
                        if (bConnected)
                        {
                                objFaxServer.Disconnect();
                        }
                        if (bRetVal == false)
                                System.Console.WriteLine("Function Failed");
                }
        }
}


