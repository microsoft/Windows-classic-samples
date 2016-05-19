//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//

using namespace System;
using namespace System::Collections;

using namespace System::Management::Automation;
using namespace System::Management::Automation::Runspaces;

/// <summary>
/// This sample uses the RunspaceInvoke class to execute
/// a script that retrieves process information for the
/// list of process names passed into the script.
/// It shows how to pass input objects to a script and
/// how to retrieve error objects as well as the output objects.
/// </summary>
/// <remarks>
/// This sample demonstrates the following:
/// 1. Creating an instance of the RunspaceInvoke class.
/// 2. Using this instance to execute a string as a PowerShell script.
/// 3. Passing input objects to the script from the calling program.
/// 4. Using PSObject to extract and display properties from the objects
///    returned by this command.
/// 5. Retrieving and displaying error records that were generated
///    during the execution of that script.
/// </remarks>
int 
__cdecl main()
{
    // Define a list of processes to look for
    array<String^>^ processNames = gcnew array<String^>(4) {
        "lsass", 
            "nosuchprocess", 
            "services", 
            "nosuchprocess2" 
    };

    // The script to run to get these processes. Input passed
    // to the script will be available in the $input variable.
    String^ script = "$input | get-process -name {$_}";

    // Create an instance of the RunspaceInvoke class.
    RunspaceInvoke^ invoker = gcnew RunspaceInvoke();

    Console::WriteLine("Process              HandleCount");
    Console::WriteLine("--------------------------------");

    // Now invoke the runspace and display the objects that are
    // returned...
    IList^ errors = nullptr;
    for each (PSObject^ result in invoker->Invoke(script, processNames, errors))
    {
        Console::WriteLine("{0,-20} {1}",
            result->Members["ProcessName"]->Value,
            result->Members["HandleCount"]->Value);
    }

    // Now process any error records that were generated while running the script.
    Console::WriteLine("\nThe following non-terminating errors occurred:\n");
    if ((errors != nullptr) && (errors->Count > 0))
    {
        for each (PSObject^ err in errors)
        {
            Console::WriteLine("    error: {0}", err->ToString());
        }
    }
    Console::WriteLine("\nHit any key to exit...");
    Console::ReadKey();
}