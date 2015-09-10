//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <objbase.h>
#include <iostream>
#include <string>

#using <Windows.winmd>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Management::Deployment;
using namespace std;

[MTAThread]
int __cdecl main(Platform::Array<String^>^ args)
{
    wcout << L"Copyright (c) Microsoft Corporation. All rights reserved." << endl;
    wcout << L"AddPackage sample" << endl << endl;

    if (args->Length < 2)
    {
        wcout << "Usage: AddPackageSample.exe packageUri" << endl;
        return 1;
    }

    HANDLE completedEvent = nullptr;
    int returnValue = 0;
    String^ inputPackageUri = args[1];

    try
    {
        completedEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        if (completedEvent == nullptr)
        {
            wcout << L"CreateEvent Failed, error code=" << GetLastError() << endl;
            returnValue = 1;
        }
        else
        {
            auto packageUri = ref new Uri(inputPackageUri);

            auto packageManager = ref new PackageManager();
            auto deploymentOperation = packageManager->AddPackageAsync(packageUri, nullptr, DeploymentOptions::None);

            deploymentOperation->Completed =
                ref new AsyncOperationWithProgressCompletedHandler<DeploymentResult^, DeploymentProgress>(
                [&completedEvent](IAsyncOperationWithProgress<DeploymentResult^, DeploymentProgress>^ operation, AsyncStatus)
            {
                SetEvent(completedEvent);
            });

            wcout << L"Installing package " << inputPackageUri->Data() << endl;

            wcout << L"Waiting for installation to complete..." << endl;

            WaitForSingleObject(completedEvent, INFINITE);

            if (deploymentOperation->Status == AsyncStatus::Error)
            {
                auto deploymentResult = deploymentOperation->GetResults();
                wcout << L"Installation Error: " << deploymentOperation->ErrorCode.Value << endl;
                wcout << L"Detailed Error Text: " << deploymentResult->ErrorText->Data() << endl;
            }
            else if (deploymentOperation->Status == AsyncStatus::Canceled)
            {
                wcout << L"Installation Canceled" << endl;
            }
            else if (deploymentOperation->Status == AsyncStatus::Completed)
            {
                wcout << L"Installation succeeded!" << endl;
            }
        }
    }
    catch (Exception^ ex)
    {
        wcout << L"AddPackageSample failed, error message: " << ex->ToString()->Data() << endl;
        returnValue = 1;
    }

    if (completedEvent != nullptr)
        CloseHandle(completedEvent);

    return returnValue;
}