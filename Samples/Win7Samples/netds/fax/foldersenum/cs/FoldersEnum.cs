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
namespace Microsoft.Samples.Fax.FoldersEnum.CS
{
        class DeviceSetting
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
                        System.Console.WriteLine(" /o <EnumInbox>/<EnumOutbox>/<EnumSentItems>/<EnumIncoming> ");
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
                //  function:   EnumerateFaxOutgoingJobs
                //
                //  Synopsis:   Enumerates the Fax Jobs in the Outbox folder
                //
                //  Arguments:  [objFaxOutgoingJobs] - Fax Outgoing Jobs Object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumerateFaxOutgoingJobs(IFaxOutgoingJobs objFaxOutgoingJobs)
                {
                        //check for NULL
                        if (objFaxOutgoingJobs == null)
                        {
                                System.Console.WriteLine("EnumerateFaxOutgoingJobs: Parameter passed is NULL");
                                return false;
                        }
                        System.Console.WriteLine("Enumerating Outgoing Jobs ...");

                        IEnumerator objEnumerator = objFaxOutgoingJobs.GetEnumerator();
                        objEnumerator.Reset();

                        while(objEnumerator.MoveNext())
                        {
                                IFaxOutgoingJob objFaxOutgoingJob = (IFaxOutgoingJob)objEnumerator.Current;
                                System.Console.WriteLine("Outgoing Job Id: " + objFaxOutgoingJob.Id);
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumerateFaxIncomingJobs
                //
                //  Synopsis:   Enumerates the Fax Jobs in the Incoming folder
                //
                //  Arguments:  [objFaxIncomingJobs] - Fax Incoming Jobs Object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumerateFaxIncomingJobs(IFaxIncomingJobs objFaxIncomingJobs)
                {
                        //check for NULL
                        if (objFaxIncomingJobs == null)
                        {
                                System.Console.WriteLine("EnumerateFaxIncomingJobs: Parameter passed is NULL");
                                return false;
                        }
                        IEnumerator objEnumerator = objFaxIncomingJobs.GetEnumerator();
                        objEnumerator.Reset();
                        System.Console.WriteLine("Enumerating Incoming Jobs ...");
                        while(objEnumerator.MoveNext())
                        {
                                IFaxIncomingJob objFaxIncomingJob = (IFaxIncomingJob) objEnumerator.Current;
                                System.Console.WriteLine("Incoming Job Id: " + objFaxIncomingJob.Id);
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumerateFaxInboxMessages
                //
                //  Synopsis:   Enumerates the Fax Messages in the Inbox folder
                //
                //  Arguments:  [objIncomingMsgIterator] - Fax Incoming Message Iterator Object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumerateFaxInboxMessages(IFaxIncomingMessageIterator objIncomingMsgIterator)
                {
                        //check for NULL
                        if (objIncomingMsgIterator == null)
                        {
                                System.Console.WriteLine("EnumerateFaxInboxMessages: Parameter passed is NULL");
                                return false;
                        }
                        System.Console.WriteLine("Enumerating Inbox Messages ...");
                        objIncomingMsgIterator.MoveFirst();
                        while (objIncomingMsgIterator.AtEOF == false)
                        {
                                IFaxIncomingMessage2 objFaxIncomingMsg = (IFaxIncomingMessage2) objIncomingMsgIterator.Message;
                                System.Console.WriteLine("Inbox Msg Id: " + objFaxIncomingMsg.Id);
                                objIncomingMsgIterator.MoveNext();
                        }
                        return true;
                }

                //+---------------------------------------------------------------------------
                //
                //  function:   EnumerateFaxSentItemMessages
                //
                //  Synopsis:   Enumerates the Fax Messages in the Sent Items folder
                //
                //  Arguments:  [objOutgoingMsgIterator] - Fax SentItems Message Iterator Object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumerateFaxSentItemMessages(IFaxOutgoingMessageIterator objOutgoingMsgIterator)
                {
                        //check for NULL
                        if (objOutgoingMsgIterator == null)
                        {
                                System.Console.WriteLine("EnumerateFaxSentItemMessages: Parameter passed is NULL");
                                return false;
                        }
                        System.Console.WriteLine("Enumerating SentItems Messages ...");
                        objOutgoingMsgIterator.MoveFirst();
                        while (objOutgoingMsgIterator.AtEOF == false)
                        {
                                IFaxOutgoingMessage2 objFaxOutgoingMsg = (IFaxOutgoingMessage2) objOutgoingMsgIterator.Message;
                                System.Console.WriteLine("SentItems Msg Id: " + objFaxOutgoingMsg.Id);
                                objOutgoingMsgIterator.MoveNext();
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumInbox
                //
                //  Synopsis:   Displays the messages present in the inbox
                //
                //  Arguments:  [objFaxFolders] - Fax Folders object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumInbox(IFaxAccountFolders objFaxFolders)
                {
                        //check for NULL
                        if (objFaxFolders == null)
                        {
                                System.Console.WriteLine("EnumInbox: Parameter passed is NULL");
                                return false;
                        }
                        IFaxAccountIncomingArchive objFaxInbox;
                        objFaxInbox = objFaxFolders.IncomingArchive;

                        IFaxIncomingMessageIterator objIncomingMsgIterator;
                        objIncomingMsgIterator = objFaxInbox.GetMessages(100);
                        if (EnumerateFaxInboxMessages(objIncomingMsgIterator) == false)
                        {
                                System.Console.WriteLine("Failed to enumerate ");
                                return false;
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumSentItems
                //
                //  Synopsis:   Displays the messages present in the Sent Items
                //
                //  Arguments:  [objFaxFolders] - Fax Folders object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------

                static bool EnumSentItems(IFaxAccountFolders objFaxFolders)
                {
                        //check for NULL
                        if (objFaxFolders == null)
                        {
                                System.Console.WriteLine("EnumSentItems: Parameter passed is NULL");
                                return false;
                        }
                        IFaxAccountOutgoingArchive objFaxOutbox;
                        objFaxOutbox = objFaxFolders.OutgoingArchive;

                        IFaxOutgoingMessageIterator objOutgoingMsgIterator;
                        objOutgoingMsgIterator = objFaxOutbox.GetMessages(100);
                        if (EnumerateFaxSentItemMessages(objOutgoingMsgIterator) == false)
                        {
                                System.Console.WriteLine("Failed to enumerate ");
                                return false;
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumIncoming
                //
                //  Synopsis:   Displays the jobs present in the Incoming Folder
                //
                //  Arguments:  [objFaxFolders] - Fax Folders object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------

                static bool EnumIncoming(IFaxAccountFolders objFaxFolders)
                {
                        //check for NULL
                        if (objFaxFolders == null)
                        {
                                System.Console.WriteLine("EnumIncoming: Parameter passed is NULL");
                                return false;
                        }
                        IFaxAccountIncomingQueue objFaxIncomingQueue;
                        objFaxIncomingQueue = objFaxFolders.IncomingQueue;

                        IFaxIncomingJobs objFaxIncomingJobs;
                        objFaxIncomingJobs = objFaxIncomingQueue.GetJobs();
                        if (EnumerateFaxIncomingJobs(objFaxIncomingJobs) == false)
                        {
                                System.Console.WriteLine("Failed to enumerate ");
                                return false;
                        }
                        return true;
                }
                //+---------------------------------------------------------------------------
                //
                //  function:   EnumOutbox
                //
                //  Synopsis:   Displays the jobs present in the Outbox Folder
                //
                //  Arguments:  [objFaxFolders] - Fax Folders object
                //
                //  Returns:    bool - true if the function was successful
                //
                //----------------------------------------------------------------------------
                static bool EnumOutbox(IFaxAccountFolders objFaxFolders)
                {
                        //check for NULL
                        if (objFaxFolders == null)
                        {
                                System.Console.WriteLine("EnumOutbox: Parameter passed is NULL");
                                return false;
                        }
                        IFaxAccountOutgoingQueue objFaxOutgoingQueue;
                        objFaxOutgoingQueue = objFaxFolders.OutgoingQueue;

                        IFaxOutgoingJobs objFaxOutgoingJobs;
                        objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs();
                        if (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs) == false )
                        {
                                System.Console.WriteLine("Failed to enumerate ");
                                return false;
                        }
                        return true;
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxAccount objFaxAccount;
                        FAXCOMEXLib.IFaxAccountFolders objFaxAccFolders;


                        string strServerName = null;
                        string strOption = null;
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

                                objFaxAccount = objFaxServer.CurrentAccount;                                              
                                objFaxAccFolders = objFaxAccount.Folders;

                                if (String.Compare("enuminbox", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (EnumInbox(objFaxAccFolders) == false)
                                        {
                                                System.Console.WriteLine("EnumInbox Failed");
                                                bRetVal = false;
                                        }
                                }

                                if (String.Compare("enumoutbox", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (EnumOutbox(objFaxAccFolders) == false)
                                        {
                                                System.Console.WriteLine("EnumOutbox Failed");
                                                bRetVal = false;
                                        }
                                }

                                if (String.Compare("enumincoming", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (EnumIncoming(objFaxAccFolders) == false)
                                        {
                                                System.Console.WriteLine("EnumIncoming Failed");
                                                bRetVal = false;
                                        }
                                }

                                if (String.Compare("enumsentitems", strOption.ToLower(CultureInfo.CurrentCulture), true, CultureInfo.CurrentCulture) == 0)
                                {
                                        if (EnumSentItems(objFaxAccFolders) == false)
                                        {
                                                System.Console.WriteLine("EnumSentItems Failed");
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
