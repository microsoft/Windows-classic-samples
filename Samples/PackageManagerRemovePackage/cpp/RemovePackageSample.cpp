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
    wcout << L"RemovePackage sample" << endl << endl;

    if (args->Length < 2)
    {
        wcout << L"Usage: RemovePackageSample.exe packageFullname" << endl;
        return 1;
    }

    HANDLE completedEvent = nullptr;
    int returnValue = 0;
    String^ inputPackageFullName = args[1];

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
            auto packageManager = ref new PackageManager();
            auto deploymentOperation = packageManager->RemovePackageAsync(inputPackageFullName);

            deploymentOperation->Completed =
                ref new AsyncOperationWithProgressCompletedHandler<DeploymentResult^, DeploymentProgress>(
                [&completedEvent](IAsyncOperationWithProgress<DeploymentResult^, DeploymentProgress>^ operation, AsyncStatus)
            {
                SetEvent(completedEvent);
            });

            wcout << L"Removing package " << inputPackageFullName->Data() << endl;

            wcout << L"Waiting for removal to complete..." << endl;
            WaitForSingleObject(completedEvent, INFINITE);

            if (deploymentOperation->Status == AsyncStatus::Error)
            {
                auto deploymentResult = deploymentOperation->GetResults();
                wcout << L"Removal Error: " << deploymentOperation->ErrorCode.Value << endl;
                wcout << L"Detailed Error Text: " << deploymentResult->ErrorText->Data() << endl;
                returnValue = 1;
            }
            else if (deploymentOperation->Status == AsyncStatus::Canceled)
            {
                wcout << L"Removal Canceled" << endl;
            }
            else if (deploymentOperation->Status == AsyncStatus::Completed)
            {
                wcout << L"Removal succeeded!" << endl;
            }
        }
    }
    catch (Exception^ ex)
    {
        wcout << L"RemovePackageSample failed, error message: " << ex->ToString()->Data() << endl;
        returnValue = 1;
    }

    if (completedEvent != nullptr)
        CloseHandle(completedEvent);

    return returnValue;
}