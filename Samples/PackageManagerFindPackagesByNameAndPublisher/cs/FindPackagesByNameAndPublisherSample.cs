//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Security.Principal;

class FindPackagesSample
{
    public static int Main(string[] args)
    {
        Console.WriteLine("Copyright (c) Microsoft Corporation. All rights reserved.");
        Console.WriteLine("FindPackagesByNameAndPublisher sample");
        Console.WriteLine();

        if (args.Length < 2)
        {
            Console.WriteLine("Usage: FindPackagesByNameAndPublisherSample.exe packageName publisherName");
            Console.WriteLine();

            return 1;
        }

        string inputPackageName = args[0];
        string inputPublisherName = args[1];

        Windows.Management.Deployment.PackageManager packageManager = new Windows.Management.Deployment.PackageManager();

        try
        {
            IEnumerable<Windows.ApplicationModel.Package> packages = (IEnumerable<Windows.ApplicationModel.Package>)packageManager.FindPackages(inputPackageName, inputPublisherName);

            int packageCount = 0;
            foreach (var package in packages)
            {
                DisplayPackageInfo(package);
                DisplayPackageUsers(packageManager, package);

                Console.WriteLine();
                packageCount += 1;
            }

            if (packageCount < 1)
            {
                Console.WriteLine("No packages were found.");
            }

        }
        catch (UnauthorizedAccessException)
        {
            Console.WriteLine("packageManager.FindPackages() failed because access was denied. This program must be run from an elevated command prompt.");

            return 1;
        }
        catch (Exception ex)
        {
            Console.WriteLine("packageManager.FindPackages() failed, error message: {0}", ex.Message);
            Console.WriteLine("Full Stacktrace: {0}", ex.ToString());

            return 1;
        }

        return 0;
    }

    private static void DisplayPackageUsers(Windows.Management.Deployment.PackageManager packageManager, Windows.ApplicationModel.Package package)
    {
        IEnumerable<Windows.Management.Deployment.PackageUserInformation> packageUsers = packageManager.FindUsers(package.Id.FullName);

        Console.Write("Users: ");

        foreach (var packageUser in packageUsers)
        {
            Console.Write("{0} ", SidToAccountName(packageUser.UserSecurityId));
        }

        Console.WriteLine();
    }

    private static string SidToAccountName(string sidString)
    {
        SecurityIdentifier sid = new SecurityIdentifier(sidString);
        try
        {
            NTAccount account = (NTAccount)sid.Translate(typeof(NTAccount));
            return account.ToString();
        }
        catch (IdentityNotMappedException)
        {
            return sidString;
        }
    }


    private static void DisplayPackageInfo(Windows.ApplicationModel.Package package)
    {
        Console.WriteLine("Name: {0}", package.Id.Name);

        Console.WriteLine("FullName: {0}", package.Id.FullName);

        Console.WriteLine("Version: {0}.{1}.{2}.{3}", package.Id.Version.Major, package.Id.Version.Minor,
            package.Id.Version.Build, package.Id.Version.Revision);

        Console.WriteLine("Publisher: {0}", package.Id.Publisher);

        Console.WriteLine("PublisherId: {0}", package.Id.PublisherId);

        Console.WriteLine("Installed Location: {0}", package.InstalledLocation.Path);

        Console.WriteLine("IsFramework: {0}", package.IsFramework);
    }
}