//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

using System;
using Microsoft.Management.Infrastructure;
using Microsoft.Management.Infrastructure.Options;

namespace SampleDotNetClient
{
    class DotNetSample
    {
        public static void Main()
        {
            bool hasComputerNameChanged = true;
            CimSession cimSession = null;
            CimSessionOptions sessionOptions = null;
            
            string className = null;
            string computerName = @"."; //GetName("ComputerName");
            if (String.IsNullOrEmpty(computerName))
            {
                computerName = null;
            }

            string namespaceName = @"root\cimv2"; //GetName("Namespace");
            CurrentOperation currentOperation = GetCurrentOption(true);
            while (true)
            {
                if (currentOperation == CurrentOperation.OperationQuit)
                {
                    if (cimSession != null)
                    {
                        cimSession.Close();
                        cimSession = null;
                    }

                    return;
                }

                if (ClassNeeded(currentOperation))
                {
                    className = GetName("ClassName");
                }

                try
                {
                    // Create local CIM session
                    if (hasComputerNameChanged)
                    {
                        if (cimSession != null)
                        {
                            cimSession.Close();
                        }

                        sessionOptions = new DComSessionOptions();
                        sessionOptions.Timeout = new TimeSpan(
                                                            0, // Hours
                                                            2, // Minutes 
                                                            0  // Seconds
                                                            );
                        cimSession = CimSession.Create(computerName, sessionOptions);
                        
                        hasComputerNameChanged = false;
                    }

                    switch (currentOperation)
                    {
                        case CurrentOperation.EnumerateAsync:
                            SampleCimOperation.EnumerateASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.EnumerateSync:
                            SampleCimOperation.EnumerateSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.GetInstanceSync:
                            SampleCimOperation.GetInstanceSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.GetInstanceAsync:
                            SampleCimOperation.GetInstanceASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.CreateInstanceAsync:
                            SampleCimOperation.CreateInstanceASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.CreateInstanceSync:
                            SampleCimOperation.CreateInstanceSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.DeleteInstanceAsync:
                            SampleCimOperation.DeleteInstanceASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.DeleteInstanceSync:
                            SampleCimOperation.DeleteInstanceSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.ModifyInstanceAsync:
                            SampleCimOperation.ModifyInstanceASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.ModifyInstanceSync:
                            SampleCimOperation.ModifyInstanceSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.QueryInstanceAsync:
                            SampleCimOperation.QueryInstanceASync(cimSession, namespaceName);
                            break;
                        case CurrentOperation.QueryInstanceSync:
                            SampleCimOperation.QueryInstanceSync(cimSession, namespaceName);
                            break;
                        case CurrentOperation.QueryAssociationSync:
                            SampleCimOperation.EnumerateAssociatedInstanceSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.QueryAssociationAsync:
                            SampleCimOperation.EnumerateAssociatedInstanceASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.InvokeMethodSync:
                            SampleCimOperation.InvokeMethodSync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.InvokeMethodAsync:
                            SampleCimOperation.InvokeMethodASync(cimSession, namespaceName, className);
                            break;
                        case CurrentOperation.SubscribeSync:
                            SampleCimOperation.SubscribeSync(cimSession, namespaceName);
                            break;
                        case CurrentOperation.SubscribeAsync:
                            SampleCimOperation.SubscribeASync(cimSession, namespaceName);
                            break;
                        case CurrentOperation.OperationComputerName:
                            computerName = GetName("ComputerName");
                            if (String.IsNullOrEmpty(computerName))
                            {
                                computerName = null;
                            }

                            hasComputerNameChanged = true;
                            break;
                        case CurrentOperation.OperationNamespaceName:
                            namespaceName = GetName("Namespace");
                            break;
                        default:
                            break;
                    }
                }
                catch (CimException ex)
                {
                    Console.WriteLine(ex.Message);
                }

                currentOperation = GetCurrentOption(false);
            }
        }

        #region Helpers

        public static string GetName(string name)
        {
            Console.Write("Type {0} : ", name);
            return Console.ReadLine();
        }

        internal static bool ClassNeeded(CurrentOperation currentOperation)
        {
            if (currentOperation == CurrentOperation.QueryInstanceSync || currentOperation == CurrentOperation.QueryInstanceAsync ||
                currentOperation == CurrentOperation.SubscribeAsync || currentOperation == CurrentOperation.SubscribeSync ||
                currentOperation == CurrentOperation.OperationNamespaceName || currentOperation == CurrentOperation.OperationComputerName)
            {
                return false;
            }

            return true;
        }

        internal enum CurrentOperation : uint
        {
            OperationQuit = 0,
            EnumerateSync = 1,
            EnumerateAsync = 2,
            GetInstanceSync = 3,
            GetInstanceAsync = 4,
            CreateInstanceSync = 5,
            CreateInstanceAsync = 6,
            DeleteInstanceSync = 7,
            DeleteInstanceAsync = 8,
            ModifyInstanceSync = 9,
            ModifyInstanceAsync = 10,
            QueryInstanceSync = 11,
            QueryInstanceAsync = 12,
            QueryAssociationSync = 13,
            QueryAssociationAsync = 14,
            InvokeMethodSync = 15,
            InvokeMethodAsync = 16,
            SubscribeSync = 17,
            SubscribeAsync = 18,
            OperationMax = 19,
            OperationComputerName = 50,
            OperationNamespaceName = 100
        }

        internal static bool IsCorrectChoice(uint result, bool isFirst)
        {
            if (isFirst)
            {
                if (result >= (uint)CurrentOperation.OperationMax)
                {
                    return false;
                }
            }
            else
            {
                if (result >= (uint)CurrentOperation.OperationMax &&
                    result != (uint)CurrentOperation.OperationComputerName &&
                    result != (uint)CurrentOperation.OperationNamespaceName)
                {
                    return false;
                }
            }

            return true;
        }

        internal static CurrentOperation GetCurrentOption(bool isFirst)
        {
            Console.WriteLine(" Please choose from the following operations ");
            Console.WriteLine("(0) To quit");
            Console.WriteLine("(1) To Enumerate Synchoronously ");
            Console.WriteLine("(2) To Enumerate ASynchoronously");
            Console.WriteLine("(3) To Get Synchoronously");
            Console.WriteLine("(4) To Get ASynchoronously");
            Console.WriteLine("(5) To Create Synchoronously");
            Console.WriteLine("(6) To Create ASynchoronously");
            Console.WriteLine("(7) To Delete Synchoronously");
            Console.WriteLine("(8) To Delete ASynchoronously");
            Console.WriteLine("(9) To Modify Synchoronously");
            Console.WriteLine("(10) To Modify ASynchoronously");
            Console.WriteLine("(11) To Query Synchoronously");
            Console.WriteLine("(12) To Query ASynchoronously");
            Console.WriteLine("(13) To Association Synchoronously");
            Console.WriteLine("(14) To Association ASynchoronously");
            Console.WriteLine("(15) To Invoke Method Synchoronously");
            Console.WriteLine("(16) To Invoke Method ASynchoronously");
            Console.WriteLine("(17) To Subscribe Synchoronously");
            Console.WriteLine("(18) To Subscribe ASynchoronously");
            if (!isFirst)
            {
                Console.WriteLine("(50) To Change ComputerName");
                Console.WriteLine("(100) To Change Namespace");
            }

            Console.Write("Input > ");
            string inputValue = Console.ReadLine();
            uint result;
            if (UInt32.TryParse(inputValue, out result) == false || IsCorrectChoice(result, isFirst) == false)
            {
                Console.WriteLine("Invalid Selection. Please try again");
                return GetCurrentOption(isFirst);
            }
            else
            {
                return (CurrentOperation)result;
            }
        }

        #endregion Helpers
    }
}
