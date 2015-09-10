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
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Management::Deployment;
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
    wcout << L"FindPackagesWithPackageTypes sample" << endl << endl;

    try
    {
        PackageTypes type = PackageTypes::Main | PackageTypes::Framework;

        auto packageManager = ref new PackageManager();
        auto packages = packageManager->FindPackagesWithPackageTypes(type);

        int packageCount = 0;
        std::for_each(begin(packages), end(packages),
            [&packageManager, &packageCount](Package^ package) 
        { 
            DisplayPackageInfo(package);
            wcout << endl;
            packageCount += 1; 
        });
    }
    catch (AccessDeniedException^)
    {
        wcout << L"FindPackagesWithPackageTypesSample failed because access was denied. This program must be run from an elevated command prompt." << endl;
        return 1;
    }
    catch (Exception^ ex)
    {
        wcout << L"FindPackagesWithPackageTypesSample failed, error message: " << ex->ToString()->Data() << endl;
        return 1;
    }

    return 0;
}
