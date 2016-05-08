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


[assembly:CLSCompliant(true)]
namespace Microsoft.Samples.Fax.FaxAccounts.CS
{   
        class FaxReassign
        {
                static string SENDER_NAME = "ReassignAdmin";
                static string SENDER_FAXNUMBER = "1234";
                static string SUBJECT = "Reassigned Fax";
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
                        System.Console.WriteLine(" /o <List/Reassign> Message option ");
                        System.Console.WriteLine(" /i Message Id. Used if Reassign option ");
                        System.Console.WriteLine(" /r Recipients in the form \"domain1\\user1;domain1\\user2\" ");
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
                //  function:   IFaxIncomingMessageIterator
                //
                //  Synopsis:   Get the incoming archive folder of the current account
                //
                //  Arguments:  [objFaxFolders] - List of folders for the current account
                //
                //  Returns:    IFaxIncomingMessageIterator: Iterator to the messages in inbox folder
                //
                //----------------------------------------------------------------------------
                static FAXCOMEXLib.IFaxIncomingMessageIterator FaxAccountIncomingArchive(FAXCOMEXLib.IFaxAccountFolders objFaxFolders)
                {
                        int NUM_MSGS = 100;
                        FAXCOMEXLib.IFaxAccountIncomingArchive objFaxInbox;
                        FAXCOMEXLib.IFaxIncomingMessageIterator objIncomingMsgIterator;

                        if (objFaxFolders != null)
                        {
                                //Initialize MsgArchive Object
                                objFaxInbox = objFaxFolders.IncomingArchive;
                                //Initialize Msg Iterator
                                objIncomingMsgIterator = objFaxInbox.GetMessages(NUM_MSGS);
                                return objIncomingMsgIterator;
                        }
                        System.Console.WriteLine("FaxAccountIncomingArchive: Parameter is NULL");                      
                        return null;
                }

                //+---------------------------------------------------------------------------
                //
                //  function:   hasReassignPermission
                //
                //  Synopsis:   Check if the current user has ReAssign Permission
                //
                //  Arguments:  [objFaxServer] - Fax Server object
                //
                //  Returns:    bool: true if it has reassign permissions
                //
                //----------------------------------------------------------------------------
                static bool hasReassignPermission(FAXCOMEXLib.FaxServerClass objFaxServer)
                {            
                        FAXCOMEXLib.IFaxSecurity2 objFaxSecurity2;
                        if (objFaxServer != null)
                        {
                                //Get the Security Object
                                objFaxSecurity2 = objFaxServer.Security2;
                                FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2 enumFaxRights;
                                //Get the Access Rights of the user
                                enumFaxRights = objFaxSecurity2.GrantedRights;                
                                if ((enumFaxRights & FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2.far2MANAGE_RECEIVE_FOLDER) == FAXCOMEXLib.FAX_ACCESS_RIGHTS_ENUM_2.far2MANAGE_RECEIVE_FOLDER) 
                                {                    
                                        return true;
                                }
                                else
                                {	                
                                        return false;
                                }
                        }
                        System.Console.WriteLine("hasReassignPermission: Parameter is NULL");
                        return false;
                }        
                //+---------------------------------------------------------------------------
                //
                //  function:   getUnassignedMsg
                //
                //  Synopsis:   Get unassigned msgs
                //
                //  Arguments:  [objIncomingMsgIterator] - Iterator to the messages in inbox folder
                //                [pCount] - Referenced variable containing the number of reassignable faxes.
                //
                //  Returns:    ArrayList: Array of strings containing the mesg ids of reassignable faxes
                //
                //----------------------------------------------------------------------------
                static ArrayList getUnassignedMsg(FAXCOMEXLib.IFaxIncomingMessageIterator objIncomingMsgIterator, ref int pCount)
                {            
                        //Get the number of reassignable messages    
                        ArrayList arrMsgIds = new ArrayList();
                        if (objIncomingMsgIterator != null)
                        {

                                //Goto first Msg
                                objIncomingMsgIterator.MoveFirst();

                                //Loop thru all msgs
                                int i = 0;
                                while (true)
                                {
                                        FAXCOMEXLib.IFaxIncomingMessage objIncomingMessage;
                                        if (objIncomingMsgIterator.AtEOF)
                                        {                        
                                                break;
                                        }
                                        objIncomingMessage = objIncomingMsgIterator.Message;
                                        FAXCOMEXLib.IFaxIncomingMessage2 objIncomingMessage2 = (FAXCOMEXLib.IFaxIncomingMessage2)(objIncomingMessage);
                                        //if not reassigned
                                        if (!objIncomingMessage2.WasReAssigned)
                                        {
                                                arrMsgIds.Add(objIncomingMessage2.Id);                        
                                                i++;
                                        }                    
                                        objIncomingMsgIterator.MoveNext();
                                }                
                                pCount = i;        

                                return arrMsgIds;
                        }
                        System.Console.WriteLine("getUnassignedMsg: Parameter is NULL");
                        return null;
                }

                //+---------------------------------------------------------------------------
                //
                //  function:   Reassign
                //
                //  Synopsis:   Reassign the Msg
                //
                //  Arguments:  [objIncomingMsgIterator] - Iterator to the messages in inbox folder
                //                [strMsgId] - Id of the message to be reassigned
                //                [strRecipients] - Recipients to whom the message is to be assigned.            
                //
                //  Returns:    bool : true if reassign was successful
                //
                //----------------------------------------------------------------------------
                static bool Reassign(FAXCOMEXLib.IFaxIncomingMessageIterator objIncomingMsgIterator, string strMsgId, string strRecipients)
                {
                        bool bRetVal = false;
                        if ((objIncomingMsgIterator != null) && (String.IsNullOrEmpty(strMsgId) != true) && (String.IsNullOrEmpty(strRecipients) != true))
                        {
                                //Goto first Msg
                                objIncomingMsgIterator.MoveFirst();
                                while (true)
                                {
                                        FAXCOMEXLib.IFaxIncomingMessage objIncomingMessage;
                                        if (objIncomingMsgIterator.AtEOF)
                                        {                        
                                                System.Console.WriteLine("Reassign Message Id not found");
                                                break;
                                        }
                                        //Get current Msg
                                        objIncomingMessage = objIncomingMsgIterator.Message;
                                        FAXCOMEXLib.IFaxIncomingMessage2 objIncomingMessage2 = (FAXCOMEXLib.IFaxIncomingMessage2)(objIncomingMessage);                  

                                        if (String.Compare(objIncomingMessage2.Id, strMsgId, true, CultureInfo.CurrentCulture) == 0)
                                        {
                                                //Set the Msg Parameters
                                                objIncomingMessage2.Subject = SUBJECT;
                                                objIncomingMessage2.SenderName = SENDER_NAME;
                                                objIncomingMessage2.Recipients = strRecipients;
                                                objIncomingMessage2.SenderFaxNumber = SENDER_FAXNUMBER;
                                                //Reassign
                                                objIncomingMessage2.ReAssign();
                                                System.Console.WriteLine("Reassign was successful");
                                                bRetVal = true;
                                                break;
                                        }
                                        //Next Msg
                                        objIncomingMsgIterator.MoveNext();
                                }                
                                return bRetVal;
                        }
                        System.Console.WriteLine("Reassign: Parameter is NULL");
                        return false;
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxAccount objFaxAccount;
                        FAXCOMEXLib.IFaxAccountFolders objFaxFolders;
                        FAXCOMEXLib.IFaxIncomingMessageIterator objIncomingMessageIterator;

                        string strServerName = null;
                        string strMsgId = null;            
                        string strRecipient = null;            
                        string strOption = null;            
                        bool bConnected = false;
                        bool bRetVal = true;
                        ArrayList arrFaxMsgIds = null;
                        int count = 0;

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
                                                                case 'r':
                                                                        if (strRecipient != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strRecipient = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'i':
                                                                        if (strMsgId != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strMsgId = args[argcount + 1];
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

                                if ((strOption == null) || ((String.Compare("reassign", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0) && ((strRecipient == null )|| (strMsgId == null))))
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
                                        System.Console.WriteLine("The Fax Server API version does not support this feature");
                                        goto Exit;
                                }               
                                objFaxAccount = objFaxServer.CurrentAccount;               
                                //Now that we have got the account object lets get the folders object
                                objFaxFolders = objFaxAccount.Folders;               

                                //if reassign message option is selected
                                if (String.Compare("reassign", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {                    
                                        if (hasReassignPermission(objFaxServer))
                                        {
                                                objIncomingMessageIterator = FaxAccountIncomingArchive(objFaxFolders);
                                                if (objIncomingMessageIterator != null)
                                                {
                                                        if (!Reassign(objIncomingMessageIterator, strMsgId, strRecipient))
                                                        {
                                                                //we dont want to log any error here as the error will be logged in the function itself
                                                                bRetVal = false;
                                                        }
                                                        else
                                                        {
                                                                //we dont want to log any error here as the error will be logged in the function itself
                                                                bRetVal = true;
                                                        }
                                                }
                                        }
                                        else
                                        {
                                                System.Console.WriteLine("User doesn't have reassign permission");
                                                bRetVal = false;

                                        }
                                }

                                //if list message ids option is selected
                                if (String.Compare("list", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {           
                                        if (hasReassignPermission(objFaxServer))
                                        {
                                                objIncomingMessageIterator = FaxAccountIncomingArchive(objFaxFolders);                 
                                                if (objIncomingMessageIterator != null)
                                                {                        
                                                        arrFaxMsgIds = getUnassignedMsg(objIncomingMessageIterator, ref count);
                                                        if (arrFaxMsgIds == null)
                                                        {
                                                                System.Console.WriteLine("No reassignable faxes present");
                                                        }
                                                        else
                                                        {
                                                                System.Console.WriteLine("Printing Msg Ids of reassignable faxes");
                                                                for(int i = 0; i<count; i++)
                                                                {
                                                                        System.Console.WriteLine("Msg Id of Message Number " + i.ToString(CultureInfo.CurrentCulture.NumberFormat) + " is " + arrFaxMsgIds[i]);
                                                                }
                                                                bRetVal = true;
                                                        }
                                                }
                                                else
                                                {
                                                        //we dont want to log any error here as the error will be logged in the function itself
                                                        bRetVal = false;
                                                }
                                        }
                                        else
                                        {
                                                System.Console.WriteLine("User doesn't have reassign permission");
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
                        if(bConnected)
                        {
                                objFaxServer.Disconnect();
                        }
                        if (bRetVal == false)
                                System.Console.WriteLine("Function Failed");
                }        
        }
}
