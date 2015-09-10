//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using Microsoft.Management.Infrastructure;
using Microsoft.Management.Infrastructure.Generic;
using Microsoft.Management.Infrastructure.Options;

namespace SampleDotNetClient
{
    internal static class SampleCimOperation
    {
        static SampleCimOperation() { }
        #region Helpers

        internal static CimOperationOptions GetOperationOptions()
        {
            CimOperationOptions options = new CimOperationOptions();
            options.Timeout = new TimeSpan(
                                            0, // Hours
                                            0, // Minutes 
                                            30  // Seconds
                                            );
            options.WriteMessage = CimExtension.WriteMessage;
            options.WriteProgress = CimExtension.WriteProgress;
            options.WriteError = CimExtension.WriteError;
            options.PromptUser = CimExtension.PromptUser;
            return options;
        }

        internal static Dictionary<string, object> GetKeyValues(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            Dictionary<string, object> propertyValues = new Dictionary<string, object>();
            try
            {
                CimClass cimClass = cimSession.GetClass(cimNamespace, cimClassName);
                var keyProperties = from p in cimClass.CimClassProperties where ((p.Flags & CimFlags.Key) == CimFlags.Key) select p;
                foreach (CimPropertyDeclaration keyProperty in keyProperties)
                {
                    Console.Write("Please type Key value for Property '" + keyProperty.Name + "' of Type:({0}) ", keyProperty.CimType);
                    string propertyValue = Console.ReadLine();
                    propertyValues.Add(keyProperty.Name, propertyValue);
                }
            }
            catch (CimException exception)
            {
                Console.WriteLine("Unable to get schema for class " + cimClassName + " in namespace " + cimNamespace);
                PrintCimException(exception);
                return null;
            }

            return propertyValues;
        }

        internal static CimInstance GetAllValues(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            CimInstance instance;
            try
            {
                CimClass cimClass = cimSession.GetClass(cimNamespace, cimClassName);
                instance = new CimInstance(cimClass);
                var cimProperties = from p in cimClass.CimClassProperties select p;
                foreach (CimPropertyDeclaration property in cimProperties)
                {
                    Console.Write("Please type value for Property '" + property.Name + "' of Type:({0}) ", property.CimType);
                    string propertyValue = Console.ReadLine();
                    if (propertyValue != null)
                    {
                        instance.CimInstanceProperties[property.Name].Value = propertyValue;
                    }
                }
            }
            catch (CimException exception)
            {
                Console.WriteLine("Unable to get schema for class " + cimClassName + " in namespace " + cimNamespace);
                PrintCimException(exception);
                return null;
            }

            return instance;
        }

        internal static List<CimInstance> GetAndPrintInstances(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            IEnumerable<CimInstance> enumeratedInstances;
            try
            {
                enumeratedInstances = cimSession.EnumerateInstances(cimNamespace, cimClassName);
            }
            catch (CimException exception)
            {
                Console.WriteLine("Unable to get instances of class {0} in namespace {1}", cimClassName, cimNamespace);
                PrintCimException(exception);
                return null;
            }

            List<CimInstance> list = new List<CimInstance>();
            int count = 0;
            foreach (var enumeratedInstance in enumeratedInstances)
            {
                list.Add(enumeratedInstance);
                bool bSingleton = true;
                var keyProperties = from p in enumeratedInstance.CimInstanceProperties where ((p.Flags & CimFlags.Key) == CimFlags.Key) select p;
                foreach (var enumeratedProperty in keyProperties)
                {
                    if (bSingleton) 
                        bSingleton = false;
                    Console.WriteLine(
                        "Id: {0} : Name: {1}, Type: {2}, Value: {3}",
                        count,
                        enumeratedProperty.Name,
                        enumeratedProperty.CimType,
                        enumeratedProperty.Value);
                }
                if (bSingleton)
                {
                    Console.WriteLine("Id: {0}", count);
                }

                count++;
            }

            return list;
        }

        public static void PrintCimException(CimException exception)
        {
            Console.WriteLine("Error Code = " + exception.NativeErrorCode);
            Console.WriteLine("MessageId = " + exception.MessageId);
            Console.WriteLine("ErrorSource = " + exception.ErrorSource);
            Console.WriteLine("ErrorType = " + exception.ErrorType);
            Console.WriteLine("Status Code = " + exception.StatusCode);
        }

        public static void PrintCimInstance(CimInstance cimInstance)
        {
            //Console.ForegroundColor = ConsoleColor.Blue;
            Console.WriteLine("Printing non null properties for class {0} ...", cimInstance.CimSystemProperties.ClassName);
            //Console.ResetColor();
            foreach (var enumeratedProperty in cimInstance.CimInstanceProperties)
            {
                bool isKey = false;
                if ((enumeratedProperty.Flags & CimFlags.Key) == CimFlags.Key)
                {
                    isKey = true;
                }

                if (enumeratedProperty.Value != null)
                {
                    if (isKey)
                    {
                        //Console.ForegroundColor = ConsoleColor.Red;
                    }

                    Console.WriteLine(
                        "Name: {0}, Type: {1}, Key: {2}, Value: {3}", 
                        enumeratedProperty.Name, 
                        enumeratedProperty.CimType,
                        isKey, 
                        enumeratedProperty.Value);
                    //Console.ResetColor();
                }
            }
        }

        public static void PrintCimMethodResult(CimMethodResult methodResult)
        {
            //Console.ForegroundColor = ConsoleColor.Blue;
            Console.WriteLine("Printing method output ...");
            //Console.ResetColor();
            foreach (CimMethodParameter methodParam in methodResult.OutParameters)
            {
                Console.WriteLine(
                    "Name: {0}, Type: {1}, Flags: {2}, Value: {3}", 
                    methodParam.Name, 
                    methodParam.CimType,
                    methodParam.Flags, 
                    methodParam.Value);
            }

            Console.WriteLine(methodResult.ReturnValue);
        }

        public static void PrintCimMethodStreamResult(CimMethodStreamedResult methodStreamResult)
        {
            //Console.ForegroundColor = ConsoleColor.Blue;
            Console.WriteLine("Printing method stream output ...");
            //Console.ResetColor();
            Console.WriteLine(
                "ParameterName : {0} , ParameterType : {1} , ParameterValue : {2}",
                methodStreamResult.ParameterName,
                methodStreamResult.ItemType,
                methodStreamResult.ItemValue);
            CimInstance cimInstance = methodStreamResult.ItemValue as CimInstance;
            if (cimInstance != null)
            {
                PrintCimInstance(cimInstance);
            }
        }

        #endregion Helpers
        #region Core

        internal static CimInstance GetInstanceCore(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            CimInstance getInstance = new CimInstance(cimClassName);
            Dictionary<string, object> propertyValues = GetKeyValues(cimSession, cimNamespace, cimClassName);
            if (propertyValues == null || propertyValues.Count == 0)
            {
                return getInstance;
            }

            foreach (var property in propertyValues)
            {
                getInstance.CimInstanceProperties.Add(CimProperty.Create(property.Key, property.Value, CimFlags.Key));
            }

            return getInstance;
        }

        internal static string QueryInstanceCore()
        {
            Console.Write("Please type the WQL Query :");
            return Console.ReadLine();
        }

        internal static CimInstance DeleteInstanceCore(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            CimInstance deleteInstance = null;
            List<CimInstance> list = GetAndPrintInstances( cimSession, cimNamespace, cimClassName );
            if (list == null)
            {
                return deleteInstance;
            }

            while (true)
            {
                Console.WriteLine("Which instance do you want to delete");
                string instanceId = Console.ReadLine();
                int result;
                if (String.IsNullOrEmpty(instanceId) || int.TryParse(instanceId, out result) == false || result >= list.Count)
                {
                    Console.WriteLine("Please type the instance Id in the range {0} to {1}", 0, list.Count - 1);
                }
                else
                {
                    deleteInstance = list[result];
                    break;
                }
            }

            return deleteInstance;
        }

        internal static CimInstance ModifyInstanceCore(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            CimInstance modifiedInstance = null;
            List<CimInstance> list = GetAndPrintInstances(cimSession, cimNamespace, cimClassName);
            if (list == null)
            {
                return modifiedInstance;
            }

            while (true)
            {
                Console.Write("Which instance do you want to modify : ");
                string instanceId = Console.ReadLine();
                int result;
                if (String.IsNullOrEmpty(instanceId) || int.TryParse(instanceId, out result) == false || result >= list.Count)
                {
                    Console.WriteLine("Please type the instance Id in the range {0} to {1}", 0, list.Count - 1);
                }
                else
                {
                    // Modify properties
                    Console.WriteLine("Printing non-key properties");
                    var keyProperties = from p in (list[result]).CimInstanceProperties 
                                        where ((p.Flags & CimFlags.Key) != CimFlags.Key) 
                                        select p;

                    foreach (var enumeratedProperty in keyProperties)
                    {
                        Console.WriteLine(
                            "Name: {0}, Type: {1}, Value: {2}", 
                            enumeratedProperty.Name, 
                            enumeratedProperty.CimType,
                            enumeratedProperty.Value);
                    }

                    string propertyName;
                    Console.WriteLine("Please type <Name> and <Value> for the properties to be modified. When done type '<EOL>' or press enter");

                    while (true)
                    {
                        Console.Write("Please type Property Name to modify: ");
                        propertyName = Console.ReadLine();

                        if (String.IsNullOrEmpty(propertyName) || propertyName.Equals("<EOL>", StringComparison.OrdinalIgnoreCase))
                        {
                            break;
                        }

                        Console.Write("Please type value for Property " + propertyName + " : ");
                        string propertyValue = Console.ReadLine();
                        (list[result]).CimInstanceProperties[propertyName].Value = propertyValue;
                    }

                    modifiedInstance = list[result];
                    break;
                }
            }

            return modifiedInstance;
        }

        internal static CimInstance CreateInstanceCore(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            CimInstance createInstance = GetAllValues(cimSession, cimNamespace, cimClassName);
            return createInstance;
        }

        internal static CimInstance EnumerateAssociatedInstanceCore(
            CimSession cimSession, 
            string cimNamespace, 
            string cimClassName,
            out string associationClassName, 
            out string resultClassName)
        {
            associationClassName = null;
            resultClassName = null;
            CimInstance associatedInstance = null;

            List<CimInstance> list = GetAndPrintInstances( cimSession, cimNamespace, cimClassName );
            if (list == null)
            {
                Console.WriteLine("EnumerateAssociatedInstance operation not performed");
                return associatedInstance;
            }

            while (true)
            {
                Console.WriteLine("On which instance do you want to perform association");
                string instanceId = Console.ReadLine();
                int result;
                if (String.IsNullOrEmpty(instanceId) || int.TryParse(instanceId, out result) == false || result >= list.Count)
                {
                    Console.WriteLine("Please type the instance Id in the range {0} to {1}", 0, list.Count - 1);
                }
                else
                {
                    associatedInstance = list[result];
                    break;
                }
            }

            associationClassName = DotNetSample.GetName("Association Class Name");
            if (string.IsNullOrEmpty(associationClassName))
            {
                associationClassName = null;
            }

            resultClassName = DotNetSample.GetName("Result Class Name");
            if (string.IsNullOrEmpty(resultClassName))
            {
                resultClassName = null;
            }

            return associatedInstance;
        }

        internal static CimMethodParametersCollection InvokeMethodCore(
            CimSession cimSession, 
            string cimNamespace, 
            string cimClassName, 
            out string cimMethodName, 
            out CimInstance inputInstance)
        {
            CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
            inputInstance = null;
            cimMethodName = null;
            bool isStaticMethod = false;
            try
            {
                CimClass cimClass = cimSession.GetClass(cimNamespace, cimClassName);

                // Print Methods
                foreach (CimMethodDeclaration methodDecl in cimClass.CimClassMethods)
                {
                    Console.WriteLine("Method Name = " + methodDecl.Name);
                }

                cimMethodName = DotNetSample.GetName("Method Name");
                if (cimClass.CimClassMethods[cimMethodName].Qualifiers["static"] != null)
                {
                    isStaticMethod = true;
                }

                foreach (CimMethodParameterDeclaration methodParameter in cimClass.CimClassMethods[cimMethodName].Parameters)
                {
                    bool bInQualifier = (methodParameter.Qualifiers["In"] != null);
                    if (bInQualifier)
                    {
                        Console.Write("Please type value for Parameter '" + methodParameter.Name + "' of Type:({0}) ", methodParameter.CimType);
                        string parameterValue = Console.ReadLine();
                        if (!String.IsNullOrEmpty(parameterValue))
                        {
                            methodParameters.Add(CimMethodParameter.Create(methodParameter.Name, parameterValue, methodParameter.CimType, 0));
                        }
                    }
                }

                // Get the instance if method is not static
                if (!isStaticMethod)
                {
                    // Get the instances for this class.
                    List<CimInstance> list = GetAndPrintInstances(cimSession, cimNamespace, cimClassName);
                    if (list == null || list.Count == 0)
                    {
                        Console.WriteLine("InvokeMethodCore operation not performed");
                        return null;
                    }

                    while (true)
                    {
                        Console.WriteLine("On which instance do you want to invoke the method");
                        string instanceId = Console.ReadLine();
                        int result;
                        if (String.IsNullOrEmpty(instanceId) || int.TryParse(instanceId, out result) == false || result >= list.Count)
                        {
                            Console.WriteLine("Please type the instance Id in the range {0} to {1}", 0, list.Count - 1);
                        }
                        else
                        {
                            inputInstance = (CimInstance)list[result];
                            break;
                        }
                    }
                }
            }
            catch (CimException exception)
            {
                Console.WriteLine("Unable to get schema for class '" + cimClassName + "' in namespace " + cimNamespace);
                PrintCimException(exception);
                return null;
            }

            return methodParameters;
        }
        #endregion Core
        #region Synchronous
        #region Enumerate
        public static void EnumerateSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            // Check Arguments
            if (cimNamespace == null)
            {
                throw new ArgumentNullException("cimNamespace");
            }

            if (cimClassName == null)
            {
                throw new ArgumentNullException("cimClassName");
            }

            try
            {
                IEnumerable<CimInstance> enumeratedInstances = cimSession.EnumerateInstances(cimNamespace, cimClassName);
                foreach (CimInstance cimInstance in enumeratedInstances)
                {
                    // Use the instance
                    PrintCimInstance(cimInstance);
                }
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
        #endregion Enumerate
        #region GetInstance
        public static void GetInstanceSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            // Check Arguments
            if (cimNamespace == null)
            {
                throw new ArgumentNullException("cimNamespace");
            }

            if (cimClassName == null)
            {
                throw new ArgumentNullException("cimClassName");
            }

            try
            {
                CimInstance getInstance = GetInstanceCore(cimSession, cimNamespace, cimClassName);
                if (getInstance == null)
                {
                    Console.WriteLine("Operation GetInstance not performed");
                    return;
                }

                CimInstance cimInstance = cimSession.GetInstance(cimNamespace, getInstance);

                // Use the instance
                PrintCimInstance(cimInstance);
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }

        #endregion GetInstance

        #region QueryInstance

        public static void QueryInstanceSync(CimSession cimSession, string cimNamespace)
        {
            try
            {
                string query = QueryInstanceCore();
                IEnumerable<CimInstance> queryInstances = cimSession.QueryInstances(cimNamespace, "WQL", query);
                foreach (CimInstance cimInstance in queryInstances)
                {
                    // Use the instance
                    PrintCimInstance(cimInstance);
                }
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        
        #endregion QueryInstance

        #region DeleteInstance

        public static void DeleteInstanceSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance deleteInstance = DeleteInstanceCore(cimSession, cimNamespace, cimClassName);
                if (deleteInstance == null)
                {
                    Console.WriteLine("DeleteInstance operation not performed");
                    return;
                }

                cimSession.DeleteInstance(cimNamespace, deleteInstance);
                Console.WriteLine("Instance Deleted Successfully");
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }  
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }

        #endregion DeleteInstance

        #region ModifyInstance

        public static void ModifyInstanceSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance modifiedInstance = ModifyInstanceCore(cimSession, cimNamespace, cimClassName);
                if (modifiedInstance == null)
                {
                    Console.WriteLine("ModifyInstance operation not performed");
                    return;
                }

                CimInstance cimInstance = cimSession.ModifyInstance(cimNamespace, modifiedInstance);
                Console.WriteLine("Instance Modified Successfully");
                PrintCimInstance(cimInstance);
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion ModifyInstance
        #region AssociatedInstance
        public static void EnumerateAssociatedInstanceSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                string associationClassName;
                string resultClassName;
                CimInstance associatedInputInstance = EnumerateAssociatedInstanceCore(
                    cimSession, 
                    cimNamespace, 
                    cimClassName,
                    out associationClassName, 
                    out resultClassName);
                if (associatedInputInstance == null)
                {
                    Console.WriteLine("EnumerateAssociatedInstanceSync operation not performed");
                    return;
                }

                IEnumerable<CimInstance> enumeratedInstances = cimSession.EnumerateAssociatedInstances(
                    cimNamespace, 
                    associatedInputInstance,
                    associationClassName, 
                    resultClassName, 
                    null, 
                    null);
                foreach (CimInstance cimInstance in enumeratedInstances)
                {
                    // Use the instance
                    PrintCimInstance(cimInstance);
                }
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion AssociatedInstance
        #region CreateInstance

        public static void CreateInstanceSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance createInstance = CreateInstanceCore(cimSession, cimNamespace, cimClassName);
                if (createInstance == null)
                {
                    Console.WriteLine("CreateInstance operation not performed");
                    return;
                }

                CimInstance cimInstance = cimSession.CreateInstance(cimNamespace, createInstance);
                Console.WriteLine("Instance Created Successfully");
                PrintCimInstance(cimInstance);
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion CreateInstance
        #region InvokeMethod
        public static void InvokeMethodSync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                string methodName;
                CimInstance inputInstance;
                CimMethodParametersCollection methodParameters = InvokeMethodCore(cimSession, cimNamespace, cimClassName, out methodName, out inputInstance);
                if (methodParameters == null)
                {
                    Console.WriteLine("Operation InvokeMethod not performed");
                    return;
                }

                CimMethodResult methodResult;
                if (inputInstance == null)
                {
                    methodResult = cimSession.InvokeMethod(cimNamespace, cimClassName, methodName, methodParameters);
                }
                else
                {
                    methodResult = cimSession.InvokeMethod(cimNamespace, inputInstance, methodName, methodParameters);
                }

                // Use the method result
                PrintCimMethodResult(methodResult);
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        
        #endregion InvokeMethod

        #region Subscribe

        public static void SubscribeSync(CimSession cimSession, string cimNamespace)
        {
            try
            {
                string query = QueryInstanceCore();
                //Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Press <Enter> to abort the subscription");
                //Console.ResetColor();
                CancellationTokenSource cts = new CancellationTokenSource();
                CimOperationOptions cimOperationOptions = new CimOperationOptions { CancellationToken = cts.Token };
                IEnumerable<CimSubscriptionResult> queryInstances = cimSession.Subscribe(cimNamespace, "WQL", query, cimOperationOptions);
                IEnumerator<CimSubscriptionResult> queryEnumerator = queryInstances.GetEnumerator();

                Thread waitUserInputThread = new Thread(SubscribeSyncReal);
                waitUserInputThread.Start(queryEnumerator);
                Console.ReadLine();
                cts.Cancel();
                queryEnumerator.Dispose();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }

        public static void SubscribeSyncReal(object inputParam)
        {
            try
            {
                IEnumerator<CimSubscriptionResult> queryEnumerator = inputParam as IEnumerator<CimSubscriptionResult>;
                if (queryEnumerator == null)
                {
                    Console.WriteLine("Can't perform Subscription operation");
                    return;
                }

                while (queryEnumerator.MoveNext())
                {
                    // Use the instance
                    PrintCimInstance(queryEnumerator.Current.Instance);
                }
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch (OperationCanceledException ex1)
            {
                Console.WriteLine("Indication Operation Cancelled: " + ex1.Message);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion Subscribe
        #endregion Synchronous
        #region Asynchoronous
        #region Enumerate
        public static void EnumerateASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            // Check Arguments
            if (cimNamespace == null)
            {
                throw new ArgumentNullException("cimNamespace");
            }

            if (cimClassName == null)
            {
                throw new ArgumentNullException("cimClassName");
            }

            try
            {
                IObservable<CimInstance> enumeratedInstances = cimSession.EnumerateInstancesAsync(cimNamespace, cimClassName, GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion Enumerate
        #region GetInstance
        public static void GetInstanceASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            // Check Arguments
            if (cimNamespace == null)
            {
                throw new ArgumentNullException("cimNamespace");
            }

            if (cimClassName == null)
            {
                throw new ArgumentNullException("cimClassName");
            }

            try
            {
                CimInstance inputInstance = GetInstanceCore(cimSession, cimNamespace, cimClassName);
                IObservable<CimInstance> enumeratedInstances = cimSession.GetInstanceAsync(cimNamespace, inputInstance, GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion GetInstance
        #region QueryInstance
        public static void QueryInstanceASync(CimSession cimSession, string cimNamespace)
        {
            try
            {
                string query = QueryInstanceCore();
                IObservable<CimInstance> queryInstances = cimSession.QueryInstancesAsync(cimNamespace, "WQL", query, GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = queryInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion QueryInstance
        #region DeleteInstance
        public static void DeleteInstanceASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance deleteInstance = DeleteInstanceCore(cimSession, cimNamespace, cimClassName);
                if (deleteInstance == null)
                {
                    Console.WriteLine("DeleteInstance operation not performed");
                    return;
                }

                CimAsyncStatus enumeratedInstances = cimSession.DeleteInstanceAsync(cimNamespace, deleteInstance, GetOperationOptions());
                TestObserver<object> observer = new TestObserver<object>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion DeleteInstance
        #region ModifyInstance
        public static void ModifyInstanceASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance modifiedInstance = ModifyInstanceCore(cimSession, cimNamespace, cimClassName);
                if (modifiedInstance == null)
                {
                    Console.WriteLine("ModifyInstance operation not performed");
                    return;
                }

                CimAsyncResult<CimInstance> enumeratedInstances = cimSession.ModifyInstanceAsync(cimNamespace, modifiedInstance, GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion ModifyInstance
        #region CreateInstance
        public static void CreateInstanceASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                CimInstance createInstance = CreateInstanceCore(cimSession, cimNamespace, cimClassName);
                if (createInstance == null)
                {
                    Console.WriteLine("CreateInstance operation not performed");
                    return;
                }

                CimAsyncResult<CimInstance> enumeratedInstances = cimSession.CreateInstanceAsync(cimNamespace, createInstance, GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion CreateInstance
        #region AssociatedInstance
        public static void EnumerateAssociatedInstanceASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                string resultClassName;
                string associationClassName;
                CimInstance associatedInputInstance = EnumerateAssociatedInstanceCore(
                    cimSession, 
                    cimNamespace, 
                    cimClassName,
                    out associationClassName, 
                    out resultClassName);
                if (associatedInputInstance == null)
                {
                    Console.WriteLine("EnumerateAssociatedInstanceSync operation not performed");
                    return;
                }

                IObservable<CimInstance> enumeratedInstances = cimSession.EnumerateAssociatedInstancesAsync(
                    cimNamespace, 
                    associatedInputInstance,
                    associationClassName, 
                    resultClassName, 
                    null, 
                    null,
                    GetOperationOptions());
                TestObserver<CimInstance> observer = new TestObserver<CimInstance>();
                IDisposable disposeable = enumeratedInstances.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion AssociatedInstance
        #region InvokeMethod
        public static void InvokeMethodASync(CimSession cimSession, string cimNamespace, string cimClassName)
        {
            try
            {
                string methodName;
                CimInstance inputInstance;
                CimMethodParametersCollection methodParameters = InvokeMethodCore(cimSession, cimNamespace, cimClassName, out methodName, out inputInstance);
                if (methodParameters == null)
                {
                    Console.WriteLine("Operation InvokeMethod not performed");
                    return;
                }

                CimAsyncMultipleResults<CimMethodResultBase> invokeParams;
                CimOperationOptions options = GetOperationOptions();
                options.EnableMethodResultStreaming = true;
                if (inputInstance == null)
                {
                    invokeParams = cimSession.InvokeMethodAsync(cimNamespace, cimClassName, methodName, methodParameters, options);
                }
                else
                {
                    invokeParams = cimSession.InvokeMethodAsync(cimNamespace, inputInstance, methodName, methodParameters, options);
                }

                TestObserver<CimMethodResultBase> observer = new TestObserver<CimMethodResultBase>();
                IDisposable disposeable = invokeParams.Subscribe(observer);
                observer.WaitForCompletion();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion InvokeMethod
        #region Subscribe
        public static void SubscribeASync(CimSession cimSession, string cimNamespace)
        {
            try
            {
                string query = QueryInstanceCore();

                //Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Press <Enter> to abort the subscription");
                //Console.ResetColor();

                IObservable<CimSubscriptionResult> queryInstances = cimSession.SubscribeAsync(cimNamespace, "WQL", query, GetOperationOptions());
                TestObserver<CimSubscriptionResult> observer = new TestObserver<CimSubscriptionResult>();
                IDisposable disposeAble = queryInstances.Subscribe(observer);

                Console.ReadLine();

                disposeAble.Dispose();
            }
            catch (CimException ex)
            {
                PrintCimException(ex);
            }
            catch( Exception ex)
            {
                Console.WriteLine(ex.Message);
            }            
        }
        #endregion Subscribe
        #endregion Asynchoronous
    }
}
