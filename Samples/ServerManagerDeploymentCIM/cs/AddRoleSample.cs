
namespace Microsoft.Samples.ServerManagerDeployment.SMDSampleApp
{
    using System;
    using System.Collections.Generic;
    using System.Threading;
    using Microsoft.Management.Infrastructure;
    using Microsoft.Management.Infrastructure.Options;

    // ---------------------------------------------------------------------------------------------------
    // The sample class to add a list of server components to a live computer.
    // The method must be executed with elevated rights to successfully perform the AddRole operation.
    // ---------------------------------------------------------------------------------------------------
    public class AddRoleSample
    {
        // Method: AddRole - Adding a list of server components to a live computer
        // Parameters: componentUniqueNames - The list of unique names of the server components to install.
        // Returns: The list of server components that are installed.
        public List<CimInstance> AddRole(List<string> componentUniqueNames)
        {
            CimInstance guidInstance = RequestGuidCreator.CreateRequestGuid();
            RequestStateEnum addRoleRequestState = RequestStateEnum.Failed;
            List<CimInstance> serverComponentInstances = new List<CimInstance>();
            List<CimInstance> componentDescriptors = new List<CimInstance>();

            Console.WriteLine("Getting Components information...");

            // First performs a GetRole operation to get the MSFT_ServerManagerServerComponent CIM classes on the computer
            GetRoleSample getRoleSample = new GetRoleSample();
            List<CimInstance> serverComponents = getRoleSample.GetRole();

            // Retrieves the list of MSFT_ServerManagerServerComponentDescriptor CIM classes for the roles to add
            foreach (CimInstance cimInstance in serverComponents)
            {
                CimProperty uniqueNameProperty = cimInstance.CimInstanceProperties["UniqueName"];
                if (uniqueNameProperty != null && componentUniqueNames.Contains((string)uniqueNameProperty.Value))
                {
                    CimProperty descriptorProperty = cimInstance.CimInstanceProperties["Descriptor"];
                    if (descriptorProperty != null)
                    {
                        componentDescriptors.Add((CimInstance)descriptorProperty.Value);
                    }
                }
            }

            Console.Write("Start installing components.");

            // Creates a CIM session to the local computer and invoke the AddServerComponentAsync CIM method
            using (CimSession cimSession = CimSession.Create(null))
            {
                CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("Source", null, CimType.StringArray, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("ScanForUpdates", false, CimType.Boolean, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("ServerComponentDescriptors", componentDescriptors.ToArray(), CimType.InstanceArray, CimFlags.In));

                IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                           "MSFT_ServerManagerDeploymentTasks",
                                                                                           "AddServerComponentAsync",
                                                                                           methodParameters,
                                                                                           operationOptions);
                DeploymentObserver observer = new DeploymentObserver();
                using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                {
                    observer.GetResults(out serverComponentInstances, out addRoleRequestState);
                }
            }

            int timeout = 600000; // timeout in 10 minutes
            int startTime = Environment.TickCount;

            // Executes the Loop to query the method invocation results until the RequestState is Completed or Failed
            while (addRoleRequestState == RequestStateEnum.InProgress && Environment.TickCount < startTime + timeout)
            {
                using (CimSession cimSession = CimSession.Create(null))
                {
                    CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                    CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                    methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));
                    methodParameters.Add(CimMethodParameter.Create("KeepAlterationStateOnRestartRequired", false, CimType.Boolean, CimFlags.In));

                    IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                               "MSFT_ServerManagerDeploymentTasks",
                                                                                               "GetAlterationRequestState",
                                                                                               methodParameters,
                                                                                               operationOptions);
                    DeploymentObserver observer = new DeploymentObserver();
                    using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                    {
                        observer.GetResults(out serverComponentInstances, out addRoleRequestState);
                    }
                }
                Console.Write(".");
                Thread.Sleep(1000);
            }

            Console.WriteLine();

            if (addRoleRequestState == RequestStateEnum.Completed)
            {
                Console.WriteLine("Components successfully installed!");
            }
            else if (addRoleRequestState == RequestStateEnum.Failed)
            {
                Console.WriteLine("AddServerComponentAsync request failed!");
            }

            return serverComponentInstances;
        }
    }
}
