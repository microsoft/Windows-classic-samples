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
namespace Microsoft.Samples.Fax.ServerConfig.CS
{
        class ServerConfig
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
                        System.Console.WriteLine(" /o <PersonalCoverPage/Branding/IncomingFaxesPublic/AutoCreateAccount option ");
                        System.Console.WriteLine(" /v value to be set \"0\" or \"1\" ");
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
                //  function:   PrintGeneralConfig
                //
                //  Synopsis:   prints the Server Configuration (PersonalCoverPages, Branding, IncomingFaxPublic and AutoCreateOnConnect)
                //
                //  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool PrintGeneralConfig(IFaxConfiguration objFaxConfiguration)
                {
                        bool bRetVal = true;
                        bool bValue = false;

                        //check for NULL
                        if (objFaxConfiguration == null)
                        {
                                System.Console.WriteLine("PrintGeneralConfig: Parameter passed is NULL");
                                return false;
                        }
                        System.Console.WriteLine();
                        System.Console.WriteLine("Logging Gerneral Config details....");
                        System.Console.WriteLine();

                        objFaxConfiguration.Refresh();
                        bValue = objFaxConfiguration.AllowPersonalCoverPages;

                        if(bValue)
                                System.Console.WriteLine("AllowPersonalCoverPages = true");
                        else
                                System.Console.WriteLine("AllowPersonalCoverPages = false");

                        bValue = objFaxConfiguration.AutoCreateAccountOnConnect;
                        if(bValue)
                                System.Console.WriteLine("AutoCreateOnConnect = true");
                        else
                                System.Console.WriteLine("AutoCreateOnConnect = false");

                        bValue = objFaxConfiguration.Branding;

                        if(bValue)
                                System.Console.WriteLine("Branding = true");
                        else
                                System.Console.WriteLine("Branding = false");

                        bValue = objFaxConfiguration.IncomingFaxesArePublic;

                        if(bValue)
                                System.Console.WriteLine("IncomingFaxesArePublic = true");
                        else
                                System.Console.WriteLine("IncomingFaxesArePublic = false");

                        return bRetVal;
                }

                //+---------------------------------------------------------------------------
                //
                //  function:   setIncomingFaxesArePublic
                //
                //  Synopsis:   sets the valus of IncomingFaxArePublic according to bState value
                //
                //  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
                //                [bState] -    bool value set to true or false
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setIncomingFaxesArePublic(IFaxConfiguration objFaxConfiguration, bool bState )
                {
                        //check for NULL
                        if (objFaxConfiguration == null)
                        {
                                System.Console.WriteLine("setIncomingFaxesArePublic: Parameter passed is NULL");
                                return false;
                        }
                        objFaxConfiguration.Refresh();   
                        //Set the configuration object
                        objFaxConfiguration.IncomingFaxesArePublic = bState;
                        //Save it
                        objFaxConfiguration.Save();    
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   setAllowPersonalCoverPages
                //
                //  Synopsis:   sets the valus of AllowPersonalCoverPages according to bState value
                //
                //  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
                //                [bState] -    bool value set to true or false
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setAllowPersonalCoverPages(IFaxConfiguration objFaxConfiguration, bool bState )
                {
                        //check for NULL
                        if (objFaxConfiguration == null)
                        {
                                System.Console.WriteLine("setAllowPersonalCoverPages: Parameter passed is NULL");
                                return false;
                        }
                        objFaxConfiguration.Refresh();
                        //Set the configuration object
                        objFaxConfiguration.AllowPersonalCoverPages = bState;
                        //Save it
                        objFaxConfiguration.Save();    
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   setBranding
                //
                //  Synopsis:   sets the valus of Branding according to bState value
                //
                //  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
                //                [bState] -    bool value set to true or false
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setBranding(IFaxConfiguration objFaxConfiguration, bool bState )
                {
                        //check for NULL
                        if (objFaxConfiguration == null)
                        {
                                System.Console.WriteLine("setBranding: Parameter passed is NULL");
                                return false;
                        }
                        objFaxConfiguration.Refresh();
                        //Set the configuration object
                        objFaxConfiguration.Branding = bState;     
                        //Save it
                        objFaxConfiguration.Save();
                        return true;
                }


                //+---------------------------------------------------------------------------
                //
                //  function:   setAutoCreateAccountOnConnect
                //
                //  Synopsis:   sets the valus of AutoCreateAccountonConnect according to bState value
                //
                //  Arguments:  [pFaxConfiguration] - FaxConfiguration object pointing to the configuration of the current server
                //                [bState] -    bool value set to true or false
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool setAutoCreateAccountOnConnect(IFaxConfiguration objFaxConfiguration, bool bState )
                {
                        //check for NULL
                        if (objFaxConfiguration == null)
                        {
                                System.Console.WriteLine("setAutoCreateAccountOnConnect: Parameter passed is NULL");
                                return false;
                        }
                        objFaxConfiguration.Refresh();
                        //Set the configuration object
                        objFaxConfiguration.AutoCreateAccountOnConnect = bState;
                        //Save it
                        objFaxConfiguration.Save();    
                        return true;
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxConfiguration objFaxConfiguration;        
                        string strServerName = null;
                        string strValue = null;
                        string strOption = null;
                        bool bConnected = false;
                        bool bState = false;            
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
                                                                case 'v':
                                                                        if (strValue != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strValue = args[argcount + 1];
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

                                if ((strOption == null) || (strValue == null) || ((String.Compare("0", strValue, true, CultureInfo.CurrentCulture) != 0) && (String.Compare("1", strValue, true, CultureInfo.CurrentCulture) != 0)))
                                {
                                        System.Console.WriteLine("Missing/Invalid args.");
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

                                objFaxConfiguration = objFaxServer.Configuration;                                
                                if (String.Compare("0", strValue, true, CultureInfo.CurrentCulture) == 0)
                                {
                                        bState = false;
                                }
                                if (String.Compare("1", strValue, true, CultureInfo.CurrentCulture) == 0)
                                {
                                        bState = true;
                                }
                                System.Console.WriteLine();
                                System.Console.WriteLine("Current Configuration.");
                                System.Console.WriteLine();
                                if (!PrintGeneralConfig(objFaxConfiguration))
                                {
                                        //we dont want to log any error here as the error will be logged in the function itself
                                        bRetVal = false;
                                }

                                //if PersonalCoverPages option is selected
                                if(String.Compare("personalcoverpage", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) ==0)
                                {
                                        if(!setAllowPersonalCoverPages(objFaxConfiguration, bState))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //if Branding option is selected
                                if(String.Compare("branding", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) ==0)              
                                {
                                        if(!setBranding(objFaxConfiguration, bState))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //if IncomingFaxArePublic option is selected
                                if(String.Compare("incomingfaxespublic", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) ==0)                 
                                {
                                        if(!setIncomingFaxesArePublic(objFaxConfiguration, bState))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }
                                //if AutoCreateAccount option is selected
                                if(String.Compare("autocreateaccount", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) ==0) 
                                {
                                        if(!setAutoCreateAccountOnConnect(objFaxConfiguration, bState))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }

                                System.Console.WriteLine("Current Server settings after the changes... ");
                                if (!PrintGeneralConfig(objFaxConfiguration))
                                {
                                        //we dont want to log any error here as the error will be logged in the function itself
                                        bRetVal = false;
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


