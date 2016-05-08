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
namespace Microsoft.Samples.Fax.FaxJobOperations.CS
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
                static bool EnumerateFaxOutgoingJobs(IFaxOutgoingJobs objFaxOutgoingJobs, bool bCancelJob, string strJobId)
                {
                        //check for NULL
                        if ((objFaxOutgoingJobs == null) || (bCancelJob == true && strJobId == null))
                        {
                                System.Console.WriteLine("EnumerateFaxOutgoingJobs: Parameter passed is NULL");
                                return false;
                        }                

                        IEnumerator objEnumerator = objFaxOutgoingJobs.GetEnumerator();
                        objEnumerator.Reset();

                        while(objEnumerator.MoveNext())
                        {
                                IFaxOutgoingJob objFaxOutgoingJob = (IFaxOutgoingJob)objEnumerator.Current;
                                if (bCancelJob == true)
                                {
                                        if(String.Compare(objFaxOutgoingJob.Id, strJobId, true, CultureInfo.CurrentCulture) == 0) 
                                                objFaxOutgoingJob.Cancel();
                                        System.Console.WriteLine("Job cancelled successfully");
                                        return true;
                                }
                                else
                                {

                                        System.Console.Write("Outgoing Job Id: " + objFaxOutgoingJob.Id);
                                        System.Console.Write(" Subject: " + objFaxOutgoingJob.Subject);
                                        System.Console.Write(" SenderName: " + objFaxOutgoingJob.Sender.Name);
                                        System.Console.Write(" Submission Id: " + objFaxOutgoingJob.SubmissionId);
                                        System.Console.WriteLine("");
                                }
                        }
                        if (bCancelJob == false)
                                return true;
                        else
                                return false;
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
                static bool EnumOutbox(IFaxOutgoingQueue objFaxOutgoingQueue)
                {
                        //check for NULL
                        if (objFaxOutgoingQueue == null)
                        {
                                System.Console.WriteLine("EnumOutbox: Parameter passed is NULL");
                                return false;
                        }

                        IFaxOutgoingJobs objFaxOutgoingJobs;
                        objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs();
                        if (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs, false, null) == false )
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
                static bool CancelJob(IFaxOutgoingQueue objFaxOutgoingQueue, string strJobId)
                {
                        //check for NULL
                        if (objFaxOutgoingQueue == null)
                        {
                                System.Console.WriteLine("EnumOutbox: Parameter passed is NULL");
                                return false;
                        }

                        IFaxOutgoingJobs objFaxOutgoingJobs;
                        objFaxOutgoingJobs = objFaxOutgoingQueue.GetJobs();
                        if (EnumerateFaxOutgoingJobs(objFaxOutgoingJobs, true, strJobId) == false)
                        {
                                System.Console.WriteLine("Failed to enumerate ");
                                return false;
                        }
                        return true;
                }

                static void Main(string[] args)
                {
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.IFaxOutgoingQueue objFaxOutgoingQueue = null;

                        string strServerName = null;                       
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

                                objFaxOutgoingQueue = objFaxServer.Folders.OutgoingQueue;                                              
                                bool bQuit = false;
                                string strJobId = null;
                                char cOption = 'c';
                                string strChar = null;

                                while (bQuit == false)
                                {
                                        System.Console.WriteLine();
                                        objFaxOutgoingQueue.Blocked = true;                                        
                                        objFaxOutgoingQueue.Paused = true;
                                        objFaxOutgoingQueue.Save();
                                        System.Console.WriteLine("Outgoing Queue is paused. ");
                                        System.Console.WriteLine("Outgoing Queue is blocked. ");
                                        
                                        //Print all outgoing jobs
                                        System.Console.WriteLine("Printing list of Outgoing jobs ...");

                                        if (EnumOutbox(objFaxOutgoingQueue) == false) 
                                        {
                                                System.Console.WriteLine("Failed to enumerate");
                                                bRetVal = false;
                                        }
                                        System.Console.WriteLine("Enter 'c' to cancel a job ");
                                        System.Console.WriteLine("Enter 'q' to quit ");
                                        strChar = System.Console.ReadLine();
                                        strChar.Trim();
                                        cOption = strChar.ToLower(CultureInfo.CurrentCulture)[0];
input:
                                        switch(cOption)
                                        {
                                                case 'c':
                                                        System.Console.WriteLine("Enter 'i' to enter Job id ");
                                                        System.Console.WriteLine("Enter 'q' to quit ");                           
                                                        strChar = System.Console.ReadLine();
                                                        strChar.Trim();
                                                        cOption = strChar.ToLower(CultureInfo.CurrentCulture)[0];

                                                input2:
                                                        switch (cOption)
                                                        {

                                                                case 'i':
                                                                        System.Console.WriteLine("Enter Job id ");
                                                                        strJobId = System.Console.ReadLine();
                                                                        strJobId.Trim();
                                                                        System.Console.Write("Job to be cancelled: ");
                                                                        System.Console.WriteLine(strJobId);
                                                                        CancelJob(objFaxOutgoingQueue, strJobId);
                                                                        break;
                                                                case 'q':
                                                                        goto quit;

                                                                default:
                                                                        System.Console.WriteLine("Invalid Option. Enter cancel option again ");                                                  strChar = System.Console.ReadLine();
                                                                        strChar.Trim();
                                                                        cOption = strChar.ToLower(CultureInfo.CurrentCulture)[0];
                                                                        goto input2;

                                                        }
                                                        break;
                                                case 'q':
quit:                       bQuit = true;
                            break;
                                                default: 
                            System.Console.WriteLine("Invalid Option. Enter again ");
                            System.Console.WriteLine("Invalid Option. Enter cancel option again ");
                            strChar = System.Console.ReadLine();
                            strChar.Trim();
                            cOption = strChar.ToLower(CultureInfo.CurrentCulture)[0];
                            goto input;
                                        }
                                }

                                //unblock queue
                                objFaxOutgoingQueue.Paused = false;                            
                                objFaxOutgoingQueue.Blocked = false;
                                objFaxOutgoingQueue.Save();
                                System.Console.WriteLine("Outgoing Queue is resumed. ");
                                System.Console.WriteLine("Outgoing Queue is unblocked. ");              
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
