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
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using FAXCOMEXLib;

[assembly: CLSCompliant(true)]
namespace Microsoft.Samples.Fax.FaxAccounts.CS
{    
        class FaxAccount
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
                        System.Console.WriteLine(" /o <Add/Delete/Validate/Enum> Account option  ");
                        System.Console.WriteLine(" /a Account Name Only if the option is Add/Delete or Validate ");
                        System.Console.WriteLine("Usage : " + System.Diagnostics.Process.GetCurrentProcess().ProcessName + " /? -- help message");
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   IsOSVersionCompatible
                //
                //  Synopsis:   finds whether the target OS supports this functionality.
                //
                //  Arguments:  [iVersion] - Minimum Version of the OS required for the Sample to run.
                //
                //  Returns:    bool - true if the Sample can run on this OS
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
                //  function:   DisplayFaxAccount
                //
                //  Synopsis:   prints the display name of a Fax Account
                //
                //  Arguments:  [objFaxAcc] - FaxAccount object whose display name is to be printed
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool DisplayFaxAccount(FAXCOMEXLib.IFaxAccount objFaxAcc)
                {
                        if(objFaxAcc != null)
                        {
                                //print the accountname
                                System.Console.WriteLine("Fax Account Name: " + objFaxAcc.AccountName);
                                return true;
                        }
                        System.Console.WriteLine("DisplayFaxAccount: Parameter is NULL");
                        return false;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   FaxEnumAccounts
                //
                //  Synopsis:   Enumerates the list of accounts
                //
                //  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
                //                [bCheck] - if set to true then is verifies if the account with name strAccName is present.
                //                [strAccName] - name of the account that is to be verified.
                //                [pbFound] - used to return the result of whether the account is present or not
                //
                //  Returns:    bool: true if passed successfully
                //
                //  Modifies:    pbFound : if the account with the name strAccName is found then pbFound is set to true.
                //
                //----------------------------------------------------------------------------
                static bool FaxEnumAccounts(FAXCOMEXLib.IFaxAccountSet objFaxAccSet, bool bCheck, string strAccName, ref bool pbFound)
                {
                        if(objFaxAccSet !=null) 
                        {
                                //Get the FaxAccounts object
                                FAXCOMEXLib.IFaxAccounts objFaxAccounts = objFaxAccSet.GetAccounts();
                                //Print the number of FaxAccounts
                                System.Console.WriteLine("Number of accounts: " + objFaxAccounts.Count);

                                //start enumerating each account.
                                System.Collections.IEnumerator objFaxEnum = objFaxAccounts.GetEnumerator();

                                while (true)
                                {
                                        bool bLast = objFaxEnum.MoveNext();
                                        if (bLast == false)
                                        {
                                                //enumeration is done
                                                System.Console.WriteLine("Enumeration of accounts done.");
                                                break;
                                        }

                                        FAXCOMEXLib.IFaxAccount objFaxAccount = (IFaxAccount)objFaxEnum.Current;
                                        //if check that a account is present.
                                        if (bCheck)
                                        {
                                                if (String.IsNullOrEmpty(strAccName) == false)
                                                {
                                                        if (String.Compare(objFaxAccount.AccountName.ToLower(CultureInfo.CurrentCulture), strAccName.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                                        {
                                                                pbFound = true;
                                                        }                        
                                                }
                                                else
                                                {
                                                        System.Console.WriteLine("FaxEnumAccounts: strAccName Parameter is NULL");
                                                        return false;
                                                }

                                        }
                                        //Display the current account info.
                                        DisplayFaxAccount(objFaxAccount);
                                }
                                return true;
                        }        
                        System.Console.WriteLine("FaxEnumAccounts: Parameter is NULL");
                        return false;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   AddAccount
                //
                //  Synopsis:   Adds a Fax Account
                //
                //  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
                //                [strAccName] - name of the account to be added. Must be a valid NT/Domain user.
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool AddAccount(FAXCOMEXLib.IFaxAccountSet objFaxAccSet, string strAccName)
                {
                        if ((objFaxAccSet != null) && (String.IsNullOrEmpty(strAccName) == false))
                        {
                                bool bFound = true;
                                //first enum the existing accounts
                                if (!FaxEnumAccounts(objFaxAccSet, false, null, ref bFound))
                                {
                                        //enum failed 
                                        System.Console.WriteLine("FaxEnumAccounts failed");
                                }
                                //now add the account
                                FAXCOMEXLib.IFaxAccount objFaxAccount = objFaxAccSet.AddAccount(strAccName);
                                //Display Info on added account.
                                DisplayFaxAccount(objFaxAccount);
                                return true;
                        }
                        System.Console.WriteLine("AddAccount: Parameter is NULL");
                        return false;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   DeleteAccount
                //
                //  Synopsis:   Deletes a Fax Account
                //
                //  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
                //                [strAccountName] - name of the account to be deleted. 
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool DeleteAccount(FAXCOMEXLib.IFaxAccountSet objFaxAccSet, string strAccountName)
                {
                        if ((objFaxAccSet != null) && (String.IsNullOrEmpty(strAccountName) == false))
                        {
                                bool bFound = false;
                                //now lets enumerate the existing accounts first.
                                if (!FaxEnumAccounts(objFaxAccSet, false, null, ref bFound))
                                {
                                        //FaxEnumAccounts failed
                                        System.Console.WriteLine("FaxEnumAccounts failed before deleting");
                                }

                                objFaxAccSet.RemoveAccount(strAccountName);
                                //now enumerate to see if account exists            
                                if (!FaxEnumAccounts(objFaxAccSet, true, strAccountName, ref bFound))
                                {
                                        //we can properly validate if this call fails, hence log an error
                                        System.Console.WriteLine("FaxEnumAccounts failed during validation");
                                        return false;
                                }
                                if (bFound)
                                {
                                        //we just deleted the account but still the enumeration shows the account. hecen log error
                                        System.Console.WriteLine("Account exists after deleteion");
                                        return false;
                                }
                                return true;
                        }
                        System.Console.WriteLine("DeleteAccount: Parameter is NULL");
                        return false;

                }
                //+---------------------------------------------------------------------------
                //
                //  function:   GetAccountInfo
                //
                //  Synopsis:   Get the account object.
                //
                //  Arguments:  [objFaxAccSet] - FaxAccountSet object having the list of all Fax Accounts
                //                [strAccountName] - Account whose info is to be printed.
                //
                //  Returns:    bool: true if passed successfully
                //
                //----------------------------------------------------------------------------
                static bool GetAccountInfo(FAXCOMEXLib.IFaxAccountSet objFaxAccSet, string strAccountName)
                {
                        if ((objFaxAccSet != null) && (String.IsNullOrEmpty(strAccountName) != false))
                        {
                                FAXCOMEXLib.IFaxAccount objFaxAccount;
                                //Get the account with the name lptstrAccountName
                                objFaxAccount = objFaxAccSet.GetAccount(strAccountName);
                                DisplayFaxAccount(objFaxAccount);
                                return true;
                        }
                        System.Console.WriteLine("GetAccountInfo: Parameter is NULL");
                        return false;        
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxAccountSet objFaxAccountSet;
                        string strServerName = null;
                        string strAccountName = null;
                        string strOption = null;
                        bool bConnected = false;
                        bool bFound = true;
                        bool bRetVal = true;

                        int iVista = 6;
                        bool bVersion = IsOSVersionCompatible(iVista);

                        if (bVersion == false)
                        {
                                System.Console.WriteLine("OS Version does not support this feature");
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
                                //FaxAccount objFax = new FaxAccount();
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
                                                                case 'a':
                                                                        if (strAccountName != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strAccountName = args[argcount + 1];
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

                                if (strOption == null || (String.Compare("enum", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) != 0) && strAccountName == null)
                                {
                                        System.Console.WriteLine("Missing args.");
                                        GiveUsage();
                                        bRetVal = false;
                                        goto Exit;
                                }
                                //Connect to Fax Server               
                                objFaxServer = new FaxServerClass();
                                objFaxServer.Connect(strServerName);
                                bConnected = true;                

                                if (objFaxServer.APIVersion < FAX_SERVER_APIVERSION_ENUM.fsAPI_VERSION_3)
                                {
                                        bRetVal = false;
                                        System.Console.WriteLine("Feature not available on this version of the Fax API");
                                        goto Exit;
                                }

                                //lets also get the account set since that is the basis for all account relates operations
                                objFaxAccountSet = objFaxServer.FaxAccountSet;

                                //if Enum Account option is selected
                                if (String.Compare("enum", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!FaxEnumAccounts(objFaxAccountSet, false, null, ref bFound))
                                        {
                                                //we dont want to log any error here as the error will be logged in the function itself
                                                bRetVal = false;
                                        }
                                }

                                //if Add Account option is selected
                                if (String.Compare("add", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!AddAccount(objFaxAccountSet, strAccountName))
                                        {
                                                bRetVal = false;
                                        }
                                }

                                //if Delete Account option is selected
                                if (String.Compare("delete", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!DeleteAccount(objFaxAccountSet, strAccountName))
                                        {
                                                bRetVal = false;
                                        }
                                }

                                //if validate account option is selected
                                if (String.Compare("validate", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (!GetAccountInfo(objFaxAccountSet, strAccountName))
                                        {
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
