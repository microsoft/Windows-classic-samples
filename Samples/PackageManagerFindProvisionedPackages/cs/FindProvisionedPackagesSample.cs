//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Security.Principal;

class FindProvisionedPackagesSample
{
    public static int Main(string[] args)
    {
        Console.WriteLine("Copyright (c) Microsoft Corporation. All rights reserved.");
        Console.WriteLine("FindProvisionedPackages sample");
        Console.WriteLine();

        Windows.Management.Deployment.PackageManager packageManager = new Windows.Management.Deployment.PackageManager();

        try
        {
            var packages = packageManager.FindProvisionedPackages();

            if (packages.Count == 0)
            {
                Console.WriteLine("No packages were found.");
            }
            else
            {
                foreach (var package in packages)
                {
                    DisplayPackageInfo(package);
                    Console.WriteLine();
                }
            }

        }
        catch (UnauthorizedAccessException)
        {
            Console.WriteLine("packageManager.FindProvisionedPackages() failed because access was denied. This program must be run from an elevated command prompt.");
            
            return 1;
        }
        catch (Exception ex)
        {
            Console.WriteLine("packageManager.FindProvisionedPackages() failed, error message: {0}", ex.Message);
            Console.WriteLine("Full Stacktrace: {0}", ex.ToString());

            return 1;
        }

        return 0;
    }

    private static void DisplayPackageInfo(Windows.ApplicationModel.Package package)
    {
        Console.WriteLine("Name: {0}", package.Id.Name);

        Console.WriteLine("FullName: {0}", package.Id.FullName);

        Console.WriteLine("Version: {0}.{1}.{2}.{3}", package.Id.Version.Major, package.Id.Version.Minor,
            package.Id.Version.Build, package.Id.Version.Revision);

        Console.WriteLine("Publisher: {0}", package.Id.Publisher);

        Console.WriteLine("PublisherId: {0}", package.Id.PublisherId);
    }
}