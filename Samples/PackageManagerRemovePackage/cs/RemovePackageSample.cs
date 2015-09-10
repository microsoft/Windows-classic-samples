//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Security.Principal;
using System.Threading;
using Windows.Management.Deployment;

class RemovePackageSample
{
    public static int Main(string[] args)
    {
        Console.WriteLine("Copyright (c) Microsoft Corporation. All rights reserved.");
        Console.WriteLine("RemovePackage sample");
        Console.WriteLine();

        if (args.Length < 1)
        {
            Console.WriteLine("Usage: RemovePackageSample.exe packageFullname");
            return 1;
        }

        string inputPackageFullName = args[0];
        int returnValue = 0;

        try
        {
            Windows.Management.Deployment.PackageManager packageManager = new Windows.Management.Deployment.PackageManager();

            Windows.Foundation.IAsyncOperationWithProgress<DeploymentResult, DeploymentProgress> deploymentOperation = packageManager.RemovePackageAsync(inputPackageFullName);

            ManualResetEvent opCompletedEvent = new ManualResetEvent(false); // this event will be signaled when the deployment operation has completed.

            deploymentOperation.Completed = (depProgress, status) => { opCompletedEvent.Set(); };

            Console.WriteLine("Removing package {0}", inputPackageFullName);

            Console.WriteLine("Waiting for removal to complete...");

            opCompletedEvent.WaitOne();

            if (deploymentOperation.Status == Windows.Foundation.AsyncStatus.Error)
            {
                Windows.Management.Deployment.DeploymentResult deploymentResult = deploymentOperation.GetResults();
                Console.WriteLine("Removal Error: {0}", deploymentOperation.ErrorCode);
                Console.WriteLine("Detailed Error Text: {0}", deploymentResult.ErrorText);
                returnValue = 1;
            }
            else if (deploymentOperation.Status == Windows.Foundation.AsyncStatus.Canceled)
            {
                Console.WriteLine("Removal Canceled");
            }
            else if (deploymentOperation.Status == Windows.Foundation.AsyncStatus.Completed)
            {
                Console.WriteLine("Removal succeeded!");
            }
            else
            {
                returnValue = 1;
                Console.WriteLine("Removal status unknown");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("RemovePackageSample failed, error message: {0}", ex.Message);
            Console.WriteLine("Full Stacktrace: {0}", ex.ToString());

            return 1;
        }

        return returnValue;
    }
}