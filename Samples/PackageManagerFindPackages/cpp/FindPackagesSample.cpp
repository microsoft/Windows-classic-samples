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

void SidToAccountName(wstring const &sidString, wstring& stringSid)
{
    PSID sid = nullptr;
    if (ConvertStringSidToSid(sidString.c_str(), &sid))
    {
        DWORD nameCharCount = 0;
        DWORD domainNameCharCount = 0;
        SID_NAME_USE sidType;

        // determine how much space is required to store the name and domainName
        LookupAccountSid(nullptr, sid, nullptr, &nameCharCount, nullptr, &domainNameCharCount, &sidType);

        wchar_t *name = new wchar_t[nameCharCount + 1]; // leave space for terminating null
        wchar_t *domainName = new wchar_t[domainNameCharCount + 1];

        ZeroMemory(name, (nameCharCount + 1) * sizeof(wchar_t));
        ZeroMemory(domainName, (domainNameCharCount + 1) * sizeof(wchar_t));

        if (LookupAccountSid(nullptr, sid, name, &nameCharCount, domainName, &domainNameCharCount, &sidType))
        {
            stringSid = domainName;
            stringSid = stringSid + L"\\" + name;
        }

        delete [] domainName;
        delete [] name;
        LocalFree(sid);
    }

    if (stringSid.length() == 0)
        stringSid = sidString;
}

void DisplayPackageUsers(Windows::Management::Deployment::PackageManager^ packageManager, Windows::ApplicationModel::Package^ package)
{
    auto packageUsers = packageManager->FindUsers(package->Id->FullName);

    wcout << L"Users: ";

    std::for_each(begin(packageUsers), end(packageUsers),
        [](Windows::Management::Deployment::PackageUserInformation^ packageUser)
    {
        wstring stringSid;
        SidToAccountName(packageUser->UserSecurityId->Data(), stringSid);
        wcout << stringSid << L" ";
    });

    wcout << endl;
}

[STAThread]
int __cdecl main(Platform::Array<String^>^ args)
{
    wcout << L"Copyright (c) Microsoft Corporation. All rights reserved." << endl;
    wcout << L"FindPackages sample" << endl << endl;

    try
    {
        auto packageManager = ref new Windows::Management::Deployment::PackageManager();
        auto packages = packageManager->FindPackages();

        int packageCount = 0;
        std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
            [&packageManager, &packageCount](Windows::ApplicationModel::Package^ package) 
        { 
            DisplayPackageInfo(package);
            DisplayPackageUsers(packageManager, package);
            wcout << endl;
            packageCount += 1; 
        });
    }
    catch (AccessDeniedException^)
    {
        wcout << L"FindPackagesSample failed because access was denied. This program must be run from an elevated command prompt." << endl;
        return 1;
    }
    catch (Exception^ ex)
    {
        wcout << L"FindPackagesSample failed, error message: " << ex->ToString()->Data() << endl;
        return 1;
    }

    return 0;
}