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
using System.Runtime.InteropServices;
using System.Text;


[assembly: CLSCompliant(true)]
namespace Microsoft.Samples.Fax.SendToFax.CS
{
        public enum SendToMode { SendToFaxRecipientAttachment };
        class SendToFax
        {
                //functions exported by fxsutility.dll
                [DllImport("fxsutility.dll",  CharSet=CharSet.Unicode)]
                        public static extern bool CanSendToFaxRecipient();
                [DllImport("fxsutility.dll",  CharSet=CharSet.Unicode)]
                        public static extern UInt32 SendToFaxRecipient(SendToMode enumType, char[] strDoc);  
            
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
                //  Modifies:
                //
                //----------------------------------------------------------------------------
                static void GiveUsage()
                {
                        System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName);
                        System.Console.WriteLine(" /o <cansendtofax> or <sendtofax> ");
                        System.Console.WriteLine(" /d Document that is to be sent as Fax ");
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

                static void Main(string[] args)
                {
                        string strDoc = null;
                        string strOption = null;
                        bool bRetVal = true;                        
                        int iVista = 6;
                        UInt32 lRet = 0;
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
                                                                case 'd':
                                                                        if (strDoc != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strDoc = args[argcount + 1];
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

                                if ((strOption == null) || ((strDoc == null) && (String.Compare("sendtofax", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)))
                                {
                                        System.Console.WriteLine("Missing args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }

                                //if sendtofax option is selected
                                if (String.Compare("sendtofax", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        strDoc = strDoc.Replace(";", " ");
                                        lRet = SendToFaxRecipient(SendToMode.SendToFaxRecipientAttachment,strDoc.ToCharArray());  
                                        if(lRet != 0)
                                        {
                                                System.Console.Write("SendToFaxRecipient: failed. Error ");
                                                System.Console.Write(lRet);
                                                System.Console.WriteLine();
                                                bRetVal = false;
                                                goto Exit;
                                        }
                                        System.Console.WriteLine("SendToFaxRecipient was successful");
                                }

                                //if cansendtofax option is selected
                                if (String.Compare("cansendtofax", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if(CanSendToFaxRecipient() == false)
                                        {
                                                System.Console.WriteLine("CanSendToFaxRecipient: failed. ");
                                                bRetVal = false;
                                        }
                                        System.Console.WriteLine("CanSendToFaxRecipient was successful");
                                }
                        }
                        catch (Exception excep)
                        {
                                System.Console.WriteLine("Exception Occured");
                                System.Console.WriteLine(excep.Message);
                        }
Exit:           
                        if (bRetVal == false)
                                System.Console.WriteLine("Function Failed");
                }
        }
}
