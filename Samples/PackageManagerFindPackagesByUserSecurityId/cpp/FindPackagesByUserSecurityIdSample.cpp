//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <sddl.h>

#include <algorithm>
#include <collection.h>
#include <iostream>
#include <string>

#using <Windows.winmd>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace std;

void DisplayPackageInfo(Windows::ApplicationModel::Package^ package)
{
    wcout << L"Name: " << package->Id->Name->Data() << endl;
    wcout << L"FullName: " << package->Id->FullName->Data() << endl;
    wcout << L"Version: " << package->Id->Version.Major << "." << 
        package->Id->Version.Minor << "." << package->Id->Version.Build << 
        "." << package->Id->Version.Revision << endl;
    wcout << L"Publisher: " << package->Id->Publisher->Data() << endl;
    wcout << L"PublisherId: " << package->Id->PublisherId->Data() << endl;
    wcout << L"Installed Location: " << package->InstalledLocation->Path->Data() << endl;
    wcout << L"IsFramework: " << (package->IsFramework ? L"True" : L"False") << endl;
}

[STAThread]
int __cdecl main(Platform::Array<String^>^ args)
{
    wcout << L"Copyright (c) Microsoft Corporation. All rights reserved." << endl;
    wcout << L"FindPackagesBySecurityId sample" << endl << endl;

    if (args->Length < 2)
    {
        wcout << L"Usage: FindPackagesBySecurityIdSample.exe userSecurityID";
        return 1;
    }

    String^ inputUserSecurityId = args[1];

    try
    {
        auto packageManager = ref new Windows::Management::Deployment::PackageManager();
        auto packages = packageManager->FindPackagesForUser(inputUserSecurityId);

        int packageCount = 0;
        std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
            [&packageManager, &packageCount](Windows::ApplicationModel::Package^ package) 
        { 
            DisplayPackageInfo(package);
            wcout << endl;
            packageCount += 1; 
        });

        if (packageCount < 1)
        {
            wcout << L"No packages were found." << endl;
        }
    }
    catch (AccessDeniedException^)
    {
        wcout << L"FindPackagesBySecurityIdSample failed because access was denied. This program must be run from an elevated command prompt." << endl;
        return 1;
    }
    catch (Exception^ ex)
    {
        wcout << L"FindPackagesBySecurityIdSample failed, error message: " << ex->ToString()->Data() << endl;
        return 1;
    }

    return 0;
}