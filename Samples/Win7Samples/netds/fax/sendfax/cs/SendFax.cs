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
using System.Runtime.InteropServices;


[assembly: CLSCompliant(true)]
namespace Microsoft.Samples.Fax.SendFax.CS
{
        class SendFax
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
                        System.Console.WriteLine(" /d DocumentPath (can have multiple documents separated by semicolons. test1.txt;test2.doc ");
                        System.Console.WriteLine(" /n Fax Number ");
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
                //  function:   PrintJobStatus
                //
                //  Synopsis:   prints the jobs status
                //
                //  Arguments:  [objFaxOutgoingJob] - FaxOutgoingJob object pointing to the fax that was sent. 
                //
                //  Returns:    bool: true is passed successfully
                //
                //----------------------------------------------------------------------------
                static bool PrintJobStatus(FAXCOMEXLib.IFaxOutgoingJob2 objFaxOutgoingJob)
                {
                        bool bRetVal = false;
                        long lDeviceId = 0;
                        FAX_JOB_STATUS_ENUM faxStatus;
                        FAX_PRIORITY_TYPE_ENUM faxPriority;
                        if (objFaxOutgoingJob != null)
                        {

                                lDeviceId = objFaxOutgoingJob.DeviceId;
                                System.Console.Write("Device Id : ");
                                System.Console.Write(lDeviceId);
                                System.Console.WriteLine();

                                faxStatus = objFaxOutgoingJob.Status;
                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCANCELED) 
                                        System.Console.WriteLine("Status :  Canceled ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCANCELING) 
                                        System.Console.WriteLine("Status :  Canceling ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsCOMPLETED) 
                                        System.Console.WriteLine("Status :  Completed ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsFAILED) 
                                        System.Console.WriteLine("Status :  Failed ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsINPROGRESS) 
                                        System.Console.WriteLine("Status :  In Progress ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsNOLINE) 
                                        System.Console.WriteLine("Status :  No Line ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsPAUSED) 
                                        System.Console.WriteLine("Status :  Paused ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsPENDING) 
                                        System.Console.WriteLine("Status :  Pending ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsRETRIES_EXCEEDED) 
                                        System.Console.WriteLine("Status :  Retries Exceeded ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsRETRYING) 
                                        System.Console.WriteLine("Status :  Retrying ");

                                if (faxStatus == FAXCOMEXLib.FAX_JOB_STATUS_ENUM.fjsROUTING) 
                                        System.Console.WriteLine("Status :  Routing ");              

                                faxPriority = objFaxOutgoingJob.Priority;
                                if (faxPriority == FAX_PRIORITY_TYPE_ENUM.fptLOW) 
                                        System.Console.WriteLine("Priority :  Low ");

                                if (faxPriority == FAX_PRIORITY_TYPE_ENUM.fptNORMAL) 
                                        System.Console.WriteLine("Priority :  Normal ");

                                if (faxPriority == FAX_PRIORITY_TYPE_ENUM.fptHIGH) 
                                        System.Console.WriteLine("Priority :  High ");

                                bRetVal = true;
                                return bRetVal;
                        }
                        System.Console.WriteLine("PrintJobStatus: Parameter is NULL");
                        return false;

                }
                //+---------------------------------------------------------------------------
                //
                //  function:   DecodeToDocArray
                //
                //  Synopsis:   Creates a string Array of Docs from the inputDocListString
                //
                //  Arguments:  [inputDocListString] - The list of documents in string format separated by semicolon
                //              [numDocuments] -    The number of documents to be sent
                //                [bRetVal] - true is passed successfully 
                //
                //  Returns:    string[]: Array of strings each containing a single document
                //
                //----------------------------------------------------------------------------
                static string[] DecodeToDocArray(string inputDocListString, ref int numDocuments, ref bool bRetVal )
                {
                        bRetVal = false;
                        if (String.IsNullOrEmpty(inputDocListString))
                        {
                                return null;
                        }                        
                        string strDelimiter = ";";
                        char[] delimiter = strDelimiter.ToCharArray();                        
                        string[] docStrArray = inputDocListString.Split(delimiter);
                        bRetVal = true;
                        numDocuments = docStrArray.Length;
                        return docStrArray;
                }

                static void Main(string[] args)
                {       
                        FAXCOMEXLib.FaxServerClass objFaxServer = null;
                        FAXCOMEXLib.FaxDocumentClass objFaxDoc = null;
                        FAXCOMEXLib.IFaxOutgoingJob2 objFaxOutgoingJob2;

                        string strServerName = null;
                        string strDocList = null;
                        string strNumber = null;
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
                                                                case 'd':
                                                                        if (strDocList != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strDocList = args[argcount + 1];
                                                                        argcount++;
                                                                        break;
                                                                case 'n':
                                                                        if (strNumber != null)
                                                                        {
                                                                                GiveUsage();
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        strNumber = args[argcount + 1];
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

                                if ((strDocList == null) ||  (strNumber == null))
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

                                bool retVal = false;
                                int numDocs = 0;

                                objFaxDoc = new FaxDocumentClass();
                                string[] strDocArray = DecodeToDocArray(strDocList, ref numDocs, ref retVal);                             
                                objFaxDoc.Bodies = strDocArray;
                                objFaxDoc.Sender.LoadDefaultSender();
                                objFaxDoc.Recipients.Add(strNumber, "TestUser");
                                object strJobIds = null;

                                int iErrorIndex = objFaxDoc.ConnectedSubmit2(objFaxServer, out strJobIds);        
                                if (iErrorIndex != -1)
                                {
                                        System.Console.Write("ConnectedSubmit2 failed ErrorIndex = ");
                                        System.Console.Write(iErrorIndex);
                                        System.Console.WriteLine();
                                        bRetVal = false;
                                        goto Exit;
                                }                                
                                string[] strArrJobIds = (string[]) strJobIds;
                                System.Console.Write("Job Id= ");
                                System.Console.Write(strArrJobIds[0]);
                                System.Console.WriteLine();

                                objFaxOutgoingJob2 = (FAXCOMEXLib.IFaxOutgoingJob2) objFaxServer.CurrentAccount.Folders.OutgoingQueue.GetJob(strArrJobIds[0]);
                                if (PrintJobStatus(objFaxOutgoingJob2) == false)
                                {
                                        System.Console.WriteLine("PrintJobStatus failed.");
                                        bRetVal = false;
                                        goto Exit;
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
