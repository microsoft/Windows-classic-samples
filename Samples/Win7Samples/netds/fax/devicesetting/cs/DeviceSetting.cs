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
namespace Microsoft.Samples.Fax.DeviceSetting.CS
{
        class DeviceSetting
        {
                //+---------------------------------------------------------------------------
                //
                //  function:    GiveUsage
                //
                //  Synopsis:    prints the usage of the application
                //
                //  Arguments:  void
                //
                //  Returns:     void
                //
                //----------------------------------------------------------------------------
                static void GiveUsage()
                {
                        System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName);
                        System.Console.WriteLine(" /s Fax Server Name ");
                        System.Console.WriteLine(" /l <list/set> Devices on the server");
                        System.Console.WriteLine(" /i device id of the device whose property (TSID or CSID) has to be set ");
                        System.Console.WriteLine(" /c new CSID value for the device ");
                        System.Console.WriteLine(" /t new TSID value for the device ");
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
                //  function:    setTSID
                //
                //  Synopsis:    sets the value of TSID for a FaxDevice
                //
                //  Arguments:  [objFaxDevices] - FaxDevices object pointing to the list of devices on the server
                //				[lDeviceId] - Device Id of the device to be set
                //				[strTSID] -	value of the TSID
                //
                //  Returns:     bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setTSID(FAXCOMEXLib.IFaxDevices objFaxDevices, int iDeviceId, string strTSID)
                {
                        FaxDevice objFaxDevice = null;
                        if(objFaxDevices != null && String.IsNullOrEmpty(strTSID) == false)
                        {
                                objFaxDevice = objFaxDevices.get_ItemById(iDeviceId);
                                //set TSID
                                objFaxDevice.TSID = strTSID;
                                //Save it
                                objFaxDevice.Save();
                                System.Console.WriteLine("New TSID is set");
                                return true;
                        }
                        System.Console.WriteLine("setTSID: Parameter is NULL");
                        return false;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:    setCSID
                //
                //  Synopsis:    sets the value of TSID for a FaxDevice
                //
                //  Arguments:  [objFaxDevices] - FaxDevices object pointing to the list of devices on the server
                //				[iDeviceId] - Device Id of the device to be set
                //				[strCSID] -	value of the CSID
                //
                //  Returns:     bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setCSID(FAXCOMEXLib.IFaxDevices objFaxDevices, int iDeviceId, string strCSID)
                {
                        FaxDevice objFaxDevice = null;
                        if (objFaxDevices != null && String.IsNullOrEmpty(strCSID) == false)
                        {
                                objFaxDevice = objFaxDevices.get_ItemById(iDeviceId);
                                //set CSID
                                objFaxDevice.CSID = strCSID;
                                //Save it
                                objFaxDevice.Save();
                                System.Console.WriteLine("New CSID is set");
                                return true;
                        }
                        System.Console.WriteLine("setCSID: Parameter is NULL");
                        return false;
                }

                //+---------------------------------------------------------------------------
                //
                //  function:    listDevices
                //
                //  Synopsis:    sets the value of TSID for a FaxDevice
                //
                //  Arguments:  [pFaxDevices] - FaxDevices object pointing to the list of devices on the server
                //
                //  Returns:     bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool listDevices(IFaxDevices objFaxDevices)
                {
                        FaxDevice objFaxDevice = null;
                        int count = objFaxDevices.Count;

                        if (objFaxDevices != null)
                        {
                                for (int i = 0; i < count; i++)
                                {
                                        objFaxDevice = objFaxDevices[i+1];
                                        System.Console.Write("Device No: ");
                                        System.Console.Write(i);
                                        System.Console.Write("  Device Id = ");
                                        System.Console.Write(objFaxDevice.Id);
                                        System.Console.Write("  Device Name = ");
                                        System.Console.Write(objFaxDevice.DeviceName);
                                        System.Console.WriteLine();              
                                }
                                return true;
                        }
                        System.Console.WriteLine("listDevices: Parameter is NULL");
                        return false;
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxDevices objFaxDevices = null;

                        string strServerName = null;
                        string strCSID = null;
                        string strTSID = null;
                        string strOption = null;
                        string strDeviceId = null;
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
                                                                case 'l':
                                                                        if (strOption != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strOption = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'i':
                                                                        if (strDeviceId != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strDeviceId = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'c':
                                                                        if (strCSID != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strCSID = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 't':
                                                                        if (strTSID != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strTSID = args[argcount + 1];
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

                                if ((strOption == null) || ((String.Compare("set", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0) && ((strDeviceId == null ) || (strCSID == null && strTSID == null))))                          {
                                        System.Console.WriteLine("Missing args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
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

                                objFaxDevices = objFaxServer.GetDevices();

                                //if list devices is selected 
                                if (String.Compare("list", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (listDevices(objFaxDevices) == false)
                                        {
                                                bRetVal = false;
                                        }
                                }
                                else
                                {
                                        //if set device option is selected
                                        if (String.Compare("set", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                        {
                                                int iDeviceId = Int32.Parse(strDeviceId, CultureInfo.CurrentCulture.NumberFormat);
                                                //if set TSID is selected
                                                if (strTSID != null)
                                                {
                                                        if (setTSID(objFaxDevices, iDeviceId, strTSID) == false)
                                                        {
                                                                bRetVal = false;
                                                        }
                                                }
                                                //if set CSID is selected
                                                if (strCSID != null)
                                                {
                                                        if (setCSID(objFaxDevices, iDeviceId, strCSID) == false)
                                                        {
                                                                bRetVal = false;
                                                        }
                                                }
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
