
namespace Microsoft.Samples.ServerManagerDeployment.SMDSampleApp
{
    using System;
    using System.Collections.Generic;
    using System.Threading;
    using Microsoft.Management.Infrastructure;
    using Microsoft.Management.Infrastructure.Options;

    // All possible states of an enumeration or alteration request.
    enum RequestStateEnum : byte
    {
        InProgress = 0,
        Completed = 1,
        Failed = 2,
    };

    // Generates a reuqest guid and converts it to the MSFT_ServerManagerRequestGuid CIM class
    class RequestGuidCreator
    {
        public static CimInstance CreateRequestGuid()
        {
            Guid guid = System.Guid.NewGuid();

            // Converts a Guid to 2 64 bit variables.
            byte[] guidBytes = guid.ToByteArray();
            UInt64 low = 0;
            UInt64 high = 0;
            for (int i = (guidBytes.Length / 2) - 1; i >= 0; i--)
            {
                if (i < 7)
                {
                    low = low << 8;
                    high = high << 8;
                }
                low |= guidBytes[i];
                high |= guidBytes[i + 8];
            }

            // Converts the Guid to the MSFT_ServerManagerRequestGuid CIM class 
            CimInstance guidInstance = new CimInstance("MSFT_ServerManagerRequestGuid", "root\\Microsoft\\Windows\\ServerManager");
            guidInstance.CimInstanceProperties.Add(CimProperty.Create("HighHalf", high, CimType.UInt64, CimFlags.None));
            guidInstance.CimInstanceProperties.Add(CimProperty.Create("LowHalf", low, CimType.UInt64, CimFlags.None));

            return guidInstance;
        }
    }

    // Displays the information of the Server Components stored in MSFT_ServerManagerServerComponent CIM classes
    class ServerComponentsPrinter
    {
        public static void DisplayResult(List<CimInstance> serverComponentInstances)
        {
            foreach (CimInstance cimInstance in serverComponentInstances)
            {
                string displayName = null;
                byte installed = 0;

                // Retrieves properties in the CIM class: the dispay name and the installation state. 
                CimProperty displayNameProperty = cimInstance.CimInstanceProperties["DisplayName"];
                if (displayNameProperty != null)
                {
                    displayName = (string)displayNameProperty.Value;
                }

                CimProperty installedProperty = cimInstance.CimInstanceProperties["Installed"];
                if (installedProperty != null)
                {
                    installed = (byte)installedProperty.Value;
                }

                // Displays "[x] Name" for installed components and "[ ] Name" for not installed components
                if (displayName != null)
                {
                    if (installed == 1)
                    {
                        Console.Write("[x] ");
                    }
                    else
                    {
                        Console.Write("[ ] ");
                    }

                    Console.WriteLine("{0}", displayName);
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // The class implementing the IObserver interface for handling the CIM method invocation results
    // ---------------------------------------------------------------------------------------------------
    class DeploymentObserver : IObserver<CimMethodResultBase>, IDisposable
    {
        private readonly ManualResetEventSlim completeEvent = new ManualResetEventSlim(false);
        private readonly List<CimInstance> serverComponents = new List<CimInstance>();
        private RequestStateEnum requestState = RequestStateEnum.Failed;
        private bool disposed = false;

        // Sets the completion event when method invocation completed
        public virtual void OnCompleted()
        {
            this.completeEvent.Set();
        }

        // Displays error message when error occurs
        public virtual void OnError(Exception error)
        {
            Console.WriteLine("Error in DeploymentObserver: {0}", error.Message);
            this.completeEvent.Set();
        }

        // Processes the results from CIM method invocation
        public virtual void OnNext(CimMethodResultBase result)
        {
            // Retrieves and stores the streamed results of MSFT_ServerManagerServerComponent CIM classes
            var streamedResult = result as CimMethodStreamedResult;
            if (streamedResult != null)
            {
                if (streamedResult.ParameterName.Equals("ServerComponents", StringComparison.OrdinalIgnoreCase))
                {
                    serverComponents.AddRange((CimInstance[])streamedResult.ItemValue);
                }
            }

            // Retrieves the posted result of MSFT_ServerManagerRequestState CIM class and stores the RequestState property
            var postedResult = result as CimMethodResult;
            if (postedResult != null && postedResult.OutParameters != null)
            {
                CimInstance requestStateInstance = null;
                CimMethodParameter requestStateParameter = null;

                // The output parameter is named as "EnumerationState" when invoking the GetServerComponentsAsync method,
                // and it's named as "AlterationState" when invoking the Add/RemoveServerComponentAsync methods
                requestStateParameter = postedResult.OutParameters["EnumerationState"];
                if (requestStateParameter != null)
                {
                    requestStateInstance = (CimInstance)requestStateParameter.Value;
                }

                requestStateParameter = postedResult.OutParameters["AlterationState"];
                if (requestStateParameter != null)
                {
                    requestStateInstance = (CimInstance)requestStateParameter.Value;
                }

                if (requestStateInstance != null)
                {
                    CimProperty property = requestStateInstance.CimInstanceProperties["RequestState"];
                    if (property != null)
                    {
                        requestState = (RequestStateEnum)property.Value;
                    }
                }
            }
        }

        // Waits for the completion of the CIM method invocation and retrieves the results
        public void GetResults(out List<CimInstance> serverComponentsResult, out RequestStateEnum requestStateResult)
        {
            this.completeEvent.Wait();
            serverComponentsResult = serverComponents;
            requestStateResult = requestState;
        }

        public void Dispose()
        {
            if (!disposed)
            {
                this.completeEvent.Dispose();
                this.disposed = true;
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // The sample class to get the list of server components from a live computer or a VHD image
    // ---------------------------------------------------------------------------------------------------
    class GetRoleSample
    {
        // Method: GetRole - Getting the list of server components from a live computer.
        // Returns: The list of retrieved server components on the system.
        public List<CimInstance> GetRole()
        {
            CimInstance guidInstance = RequestGuidCreator.CreateRequestGuid();
            List<CimInstance> serverComponentInstances = new List<CimInstance>();
            RequestStateEnum getRoleRequestState = RequestStateEnum.Failed;

            // Creates a CIM session to the local computer and invoke the GetServerComponentsAsync CIM method
            using (CimSession cimSession = CimSession.Create(null))
            {
                CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));

                IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                           "MSFT_ServerManagerDeploymentTasks",
                                                                                           "GetServerComponentsAsync",
                                                                                           methodParameters,
                                                                                           operationOptions);
                DeploymentObserver observer = new DeploymentObserver();
                using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                {
                    observer.GetResults(out serverComponentInstances, out getRoleRequestState);
                }
            }

            int timeout = 60000; // timeout in 1 minute
            int startTime = Environment.TickCount;

            // If the call to GetServerComponentsAsync completed with RequestState of InProgress, constructs a loop to invoke the 
            // GetEnumerationRequestState CIM method to query the RequestState until it's Completed or Failed.
            // The list of the retrieved server components is stored in serverComponentInstances.
            while (getRoleRequestState == RequestStateEnum.InProgress && Environment.TickCount < startTime + timeout)
            {
                using (CimSession cimSession = CimSession.Create(null))
                {
                    CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                    CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                    methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));

                    IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                               "MSFT_ServerManagerDeploymentTasks",
                                                                                               "GetEnumerationRequestState",
                                                                                               methodParameters,
                                                                                               operationOptions);
                    DeploymentObserver observer = new DeploymentObserver();
                    using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                    {
                        observer.GetResults(out serverComponentInstances, out getRoleRequestState);
                    }
                }
                Thread.Sleep(1000);
            }

            if (getRoleRequestState == RequestStateEnum.Failed)
            {
                Console.WriteLine("GetServerComponentsAsync request failed!");
            }

            return serverComponentInstances;
        }

        // Method: GetRole - Getting the list of server components from a VHD Image.
        // Returns: The list of retrieved server components on the system.
        public List<CimInstance> GetRoleVhd(string vhdPath)
        {
            CimInstance guidInstance = RequestGuidCreator.CreateRequestGuid();
            List<CimInstance> serverComponentInstances = new List<CimInstance>();
            RequestStateEnum getRoleRequestState = RequestStateEnum.Failed;

            // Create a CIM session to the local computer and invoke the GetServerComponentsVhdAsync CIM method
            using (CimSession cimSession = CimSession.Create(null))
            {
                CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("VhdPath", vhdPath, CimType.String, CimFlags.In));

                IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                           "MSFT_ServerManagerDeploymentTasks",
                                                                                           "GetServerComponentsVhdAsync",
                                                                                           methodParameters,
                                                                                           operationOptions);
                DeploymentObserver observer = new DeploymentObserver();
                using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                {
                    observer.GetResults(out serverComponentInstances, out getRoleRequestState);
                }
            }

            int timeout = 600000; // timeout in 10 minutes for VHD Get
            int startTime = Environment.TickCount;

            // Executes the Loop to query the method invocation results until the RequestState is Completed or Failed
            while (getRoleRequestState == RequestStateEnum.InProgress && Environment.TickCount < startTime + timeout)
            {
                using (CimSession cimSession = CimSession.Create(null))
                {
                    CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                    CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                    methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));

                    IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                               "MSFT_ServerManagerDeploymentTasks",
                                                                                               "GetEnumerationRequestState",
                                                                                               methodParameters,
                                                                                               operationOptions);
                    DeploymentObserver observer = new DeploymentObserver();
                    using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                    {
                        observer.GetResults(out serverComponentInstances, out getRoleRequestState);
                    }
                }
                Thread.Sleep(1000);
            }

            if (getRoleRequestState == RequestStateEnum.Failed)
            {
                Console.WriteLine("GetServerComponentsVhdAsync request failed!");
            }

            return serverComponentInstances;
        }
    }
}
