// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "Subscription.h"


//Handling Different Options available
typedef enum _ACTION
{
    ActionCreate = 0,
    ActionDelete,
    ActionEnumerate,
    ActionGetRuntimeStatus,
    ActionGetSubscription,
    ActionRetry,
    ActionHelp,
    
} ACTION;

//Command Line Arguments
const LPCWSTR CREATE_SUBSCRIPTION   = L"CS";
const LPCWSTR DELETE_SUBSCRIPTION   = L"DS";
const LPCWSTR ENUM_SUBSCRIPTION     = L"ES";
const LPCWSTR GET_RUNTIME_STATUS   = L"GR";
const LPCWSTR GET_SUBSCRIPTION       = L"GS";
const LPCWSTR RETRY_SUBSCRIPTION = L"RS";
const LPCWSTR HELP = L"?";


const std::wstring DEFAULT_URI = L"http://schemas.microsoft.com/wbem/wsman/1/windows/EventLog";
const std::wstring DEFAULT_QUERY = L"<QueryList><Query Path=\"Application\"><Select>*</Select></Query></QueryList>";
const std::wstring DEFAULT_LOG = L"ForwardedEvents";
const std::wstring DEFAULT_EVENTSOURCE = L"Localhost";
const std::wstring DEFAULT_USERNAME = L"TestUser";
const std::wstring DEFAULT_PASSWORD = L"MyTestPassword";


// The Default Query translates to Select any event that is raised to the 
// Application log on the event source machine (localhost in this sample)
/*
            <Query>
                 <![CDATA[<QueryList>
                                    <Query Path="Application">
                                            <Select>*</Select>
                                    </Query>
                                </QueryList>
                            ]]>
            </Query>

*/

//Display the Usage
void DisplayUsage(void)
{
   wprintf(L"Usage:  "
 		L"Subscription.exe <option> [Subscription Name]\n\n"
		L"Options:\n\n"
		L"     /CS - Creates a Subscription\n"
		L"     /DS - Deletes a Subscription\n"
		L"     /ES - Enumerates subscriptions on the machine\n"
		L"     /GR - Gets the runtime status of each event source  in the subscription\n"
		L"     /GS - Prints the Properties of a subscription\n"
		L"     /RS - Retries the subscription for all the event sources\n"
		L"     /?  - Print this help message\n");
                

    wprintf(L" \nE.g:\n\n");

    wprintf(L"     Create a Subscription:\n"
		 L"         subscription.exe /cs MyTestSubscription\n\n");

    wprintf(L"     Delete a Subscription:\n"
		 L"         subscription.exe /ds MyTestSubscription\n\n");

    wprintf(L"     Enumerate Subscriptions on the machine:\n"
		 L"         subscription.exe /es \n\n");

    wprintf(L"     Get Runtime Status of a Subscription:\n"
		 L"         subscription.exe /gr MyTestSubscription\n\n");

    wprintf(L"     Get Subscription Properties:\n"
		 L"         subscription.exe /gs MyTestSubscription\n\n");

    wprintf(L"     Retry Subscription:\n"
		 L"         subscription.exe /rs MyTestSubscription\n\n");

    wprintf(L"     Print This Help Message:\n"
		 L"         subscription.exe /? \n\n");


    return;

}


//Display Formatted Error Message 
void ErrorExit(DWORD errorCode)
{

    LPVOID lpwszBuffer;

    if (ERROR_SUCCESS == errorCode)
        return;

    FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           errorCode,
                           0,
                           (LPWSTR) &lpwszBuffer,
                           0,
                           NULL);

    
    if (!lpwszBuffer)
    {
        wprintf(L"Failed to FormatMessage.  Operation Error Code: %u. Error Code from FormatMessage: %u\n", errorCode, GetLastError());
        return;
    }

    wprintf(L"\nFailed to Perform Operation.\nError Code: %u\nError Message: %s\n", errorCode, lpwszBuffer);

    LocalFree(lpwszBuffer);

    return;

}

//Funtion to Parse Command Line Arguments
ACTION ParseArguments(int argc, __in_ecount(argc)wchar_t* argv[])
{

    size_t optionLength = 0;
    LPCWSTR optionDelimiter;
    LPCWSTR optionName ;
    
    ACTION action = ActionHelp;

    if (argc < 2) return action;

    if (*argv[1] != '/' && *argv[1] !='-') return action;


    optionDelimiter = argv[1] + 1;

    while( *optionDelimiter !='\0' )
    {
        optionDelimiter ++;
    }

    if ( optionDelimiter == argv[1] + 1)
    {
        return ActionHelp;
    }

    
    optionLength = wcslen(argv[1]+1);
    optionName = new wchar_t[optionLength + 1];
    wcsncpy_s((wchar_t*) optionName, optionLength+1, argv[1]+1, optionLength+1);
    
    BOOL subscriptionNameReqd = true;
 
    if ( 0 == _wcsicmp(optionName, ENUM_SUBSCRIPTION ))
    {
        action = ActionEnumerate;        
        subscriptionNameReqd = false;
    }
    else if (0 == _wcsicmp(optionName, HELP))
    {
        action = ActionHelp;
        subscriptionNameReqd = false;
    }
    else if ( 0 == _wcsicmp(optionName, DELETE_SUBSCRIPTION))
    {
        action = ActionDelete;
    }
    else if ( 0 == _wcsicmp(optionName, CREATE_SUBSCRIPTION))
    {
        action = ActionCreate;
    }
    else if ( 0 == _wcsicmp(optionName, GET_RUNTIME_STATUS))
    {
        action = ActionGetRuntimeStatus;
    }
    else if ( 0 == _wcsicmp(optionName, RETRY_SUBSCRIPTION))
    {
        action = ActionRetry;
    }
    else if(0 ==_wcsicmp(optionName, GET_SUBSCRIPTION))
    {
        action = ActionGetSubscription;
    }
    
    if ( subscriptionNameReqd && !argv[2])
    {
        action = ActionHelp;
    }

    delete[] optionName;
    
    return action;
}




int  __cdecl wmain(int argc, __in_ecount(argc) wchar_t* argv[])
{

        DWORD dwRetVal = ERROR_SUCCESS;
        ACTION action;

        action = ParseArguments(argc, argv);
        switch (action)
        {
            case ActionCreate:
                {
                    //Create a default event collector Subscription
                    SUBSCRIPTION sub;

                    sub.Name = argv[2];
                    sub.Description = L"This is a test subscription to the local machine. This subscription collects events from the Application \nLog and forwards them to the ForwardedEvents log. This subscription uses Custom Configuration Mode";
                    sub.URI = DEFAULT_URI;
                    sub.Query = DEFAULT_QUERY;
                    sub.DestinationLog = DEFAULT_LOG;
                    sub.ConfigMode = EcConfigurationModeCustom;
                    sub.MaxItems  = 5;
                    sub.MaxLatencyTime = 10000;
                    sub.HeartbeatInerval = 10000;
                    sub.DeliveryMode = EcDeliveryModePull;
                    sub.EventSource = DEFAULT_EVENTSOURCE;
                    sub.EventSourceStatus = true;
                    sub.CommonUserName = DEFAULT_USERNAME;
                    sub.CommonPassword = DEFAULT_PASSWORD;
                    sub.ContentFormat = EcContentFormatRenderedText;
                    sub.CredentialsType = EcSubscriptionCredDefault;

                    sub.SubscriptionStatus = true;
                    sub.ReadExistingEvents = false;

                    dwRetVal = CreateSubscription(sub);
                    break;
                }
            case ActionDelete:
                    
                   //Delete the specified event collector subscription
                    if ( !EcDeleteSubscription(argv[2], 0))
                    {
                        dwRetVal = GetLastError();
                    }
                    break;
    
            case ActionEnumerate:

                    //Enumerate the event collector subscriptions available on the machine
                    // Return if there are additional (un-required) arguments
                    if (argc>2)
                    {
                        DisplayUsage();
                        return 0;
                    }
                    dwRetVal = EnumerateSubscriptions();
                    break;
            
            case ActionGetRuntimeStatus:

                    //Get the Runtime Status for the event sources in 
                    // the specified event collector subscription
                    dwRetVal = GetRuntimeStatus(argv[2]);
                    break;
            
            case ActionGetSubscription:
                    //Get the properties associated with the event collector subscription
                    dwRetVal = GetSubscription(argv[2]);
                    break;
            case ActionRetry:
                    //Retry the event collector subscription for all the event sources
                    dwRetVal = RetrySubscription(argv[2]);
                    break;

            case ActionHelp:
            default:
                DisplayUsage();
                break;
        }

        //Exit with the appropriate error message
        ErrorExit(dwRetVal);

        
    return 0;
}


