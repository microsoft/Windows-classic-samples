
namespace Microsoft.Samples.ServerManagerDeployment.SMDSampleApp
{
    using System;
    using System.Collections.Generic;
    using System.Threading;
    using Microsoft.Management.Infrastructure;
    using Microsoft.Management.Infrastructure.Options;

    // ---------------------------------------------------------------------------------------------------
    // The sample class to remove a list of server components from a VHD image.
    // The method must be executed with elevated rights to successfully perform the RemoveRole operation.
    // ----------------------------------------------------------------------------------------------------
    public class RemoveRoleSample
    {
        // Method: RemoveRoleVhd - Removing a list of server components from a VHD image.
        // Parameters: componentUniqueNames - The list of unique names of the server components to remove.
        //             vhdPath - The path of the VHD image.
        // Returns: The list of server components that are removed.
        public List<CimInstance> RemoveRoleVhd(List<string> componentUniqueNames, string vhdPath)
        {
            CimInstance guidInstance = RequestGuidCreator.CreateRequestGuid();
            RequestStateEnum removeRoleRequestState = RequestStateEnum.Failed;
            List<CimInstance> serverComponentInstances = new List<CimInstance>();
            List<CimInstance> componentDescriptors = new List<CimInstance>();

            Console.WriteLine("Getting Components information...");

            // First performs a GetRole operation to get the MSFT_ServerManagerServerComponent CIM classes on the VHD image
            GetRoleSample getRoleSample = new GetRoleSample();
            List<CimInstance> serverComponents = getRoleSample.GetRoleVhd(vhdPath);

            // Retrieves the list of MSFT_ServerManagerServerComponentDescriptor CIM classes for the roles to remove
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

            Console.Write("Start uninstalling components.");

            // Create a CIM session to the local computer and invoke the RemoveServerComponentsVhdAsync CIM method
            using (CimSession cimSession = CimSession.Create(null))
            {
                CimOperationOptions operationOptions = new CimOperationOptions() { EnableMethodResultStreaming = true };
                CimMethodParametersCollection methodParameters = new CimMethodParametersCollection();
                methodParameters.Add(CimMethodParameter.Create("RequestGuid", guidInstance, CimType.Instance, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("DeleteComponents", false, CimType.Boolean, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("ServerComponentDescriptors", componentDescriptors.ToArray(), CimType.InstanceArray, CimFlags.In));
                methodParameters.Add(CimMethodParameter.Create("VhdPath", vhdPath, CimType.String, CimFlags.In));

                IObservable<CimMethodResultBase> observable = cimSession.InvokeMethodAsync("root\\Microsoft\\Windows\\ServerManager",
                                                                                           "MSFT_ServerManagerDeploymentTasks",
                                                                                           "RemoveServerComponentVhdAsync",
                                                                                           methodParameters,
                                                                                           operationOptions);
                DeploymentObserver observer = new DeploymentObserver();
                using (IDisposable cancellationDisposable = observable.Subscribe(observer))
                {
                    observer.GetResults(out serverComponentInstances, out removeRoleRequestState);
                }
            }

            int timeout = 1200000; // timeout in 20 minutes for VHD Remove
            int startTime = Environment.TickCount;

            // Executes the Loop to query the method invocation results until the RequestState is Completed or Failed
            while (removeRoleRequestState == RequestStateEnum.InProgress && Environment.TickCount < startTime + timeout)
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
                        observer.GetResults(out serverComponentInstances, out removeRoleRequestState);
                    }
                }
                Console.Write(".");
                Thread.Sleep(1000);
            }

            Console.WriteLine();

            if (removeRoleRequestState == RequestStateEnum.Completed)
            {
                Console.WriteLine("Components successfully uninstalled!");
            }
            else if (removeRoleRequestState == RequestStateEnum.Failed)
            {
                Console.WriteLine("RemoveServerComponentVhdAsync request failed!");
            }

            return serverComponentInstances;
        }
    }
}
