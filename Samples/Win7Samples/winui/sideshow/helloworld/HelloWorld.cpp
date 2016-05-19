// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// HelloWorld.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HelloWorldClient.h"

using namespace std;

int __cdecl main(int /*argc*/, char* /*argv*/[])
{
    //
    // Initialize COM; we use the Apartment threading model
    // because our implemented COM objects are not thread-safe
    //
    cout << "Initializing COM" << endl;
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    CHelloWorldClient client;
    
    //
    // Register this client application with the Windows SideShow
    // platform
    //
    cout << "Registering this client application" << endl;
    client.Register();

    //
    // Add content to the display
    //
    cout << "Add content to the display" << endl;
    client.AddContent();

    //
    // Wait for the user to input something, so we can
    // handle events from the platform if necessary
    //
    cout << "Press enter key to finish";
    getchar();

    //
    // Remove all of the content from the display
    // so it's no longer available once this application
    // closes
    //
    cout << "Removing all content" << endl;
    client.RemoveAllContent();

    //
    // Unregister this client application from the platform
    //
    cout << "Unregistering this client application" << endl;
    client.Unregister();

    //
    // Finally, uninitialize COM
    //
    cout << "Uninitializing COM" << endl;
    ::CoUninitialize();
    return 0;
}

