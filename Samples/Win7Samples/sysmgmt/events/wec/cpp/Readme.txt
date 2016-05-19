Subscription.EXE -- Copyright (c) Microsoft Corporation.  All rights reserved.

Description:
This sample illustrates use of the Event Collector APIs to 

    a. Create a Subscription (to the local machine)
    b. Get Properties of a Subscription
    c. Enumerate Subscriptions available on the machine
    d. Retry Subscription
    e. Get Runtime Status of the Subscription
    f. Add an Event Source to the subscription
    g. Delete a subscription


Pre-Requisites:

1. Microsoft Windows Vista(TM) or later
2. Winrm Service is started and configured
3. Wecsvc Service is started and configured
4. One must be an administrator to create a subscription or retry a subscription
5. Testuser defined in the sample is created and part of the administrators group (of the event source)

Notes:

1. A setup.cmd is provided that will help with the configuration of Winrm and Event Collector services. 
2. If you want to retry the entire subscription, EcSaveSubscription() without any value changes will do the same work.


Limitations:

1. Sample does not make use of Credential Manager UI APIs to manage credentials for creating 
subscriptions or when adding event sources to a subscritption
2. Creates a default subscription everytime, unless the default values are changed and the solution rebuilt.
3. Does not allow setting of individual properties through commandline. However, the basic concept is illustrated.
4. Does not provide option to add/remove an event source through commandline.
5. Does not use HTTPS transport option for subscription

=============================================================================

 BUILD Instructions

The sample can be built by running the following command from a Platform SDK build environment window: 

    nmake

In order to build the sample clean, the following commands should be used: 
    nmake clean
    nmake
=============================================================================

Usage:

Subscription.exe /? will display the command line options availabe.  A summary of the options is provided below.


               CS - Creates a Subscription [Requires Admin Privileges]
               DS - Deletes a Subscription [Requires Admin Privileges]
               ES - Enumerates subscriptions on the machine
               GR - Gets the runtime status of each event source  in the subscrption
               GS - Prints the Properties of a subscription 
               RS - Retries the subscription for all the event sources [Requires Admin Privileges]
               ?   - Print this help message


E.g:
    Create Subscription:
              subscription.exe /cs MyTestSubscription

    Delete Subscription:
              subscription.exe /ds MyTestSubscription

    Enumerate Subscription:
              subscription.exe /es

    Get Runtime Status of  Subscription:
              subscription.exe /gr MyTestSubscription

    Get Subscription:
              subscription.exe /gs MyTestSubscription

    Retry Subscription:
              subscription.exe /rs MyTestSubscription

    Print This Help Message:
              subscription.exe /?


=============================================================================

Important Information:

1.  Please use Credential Manager APIs to manage Credentials for the subscription. For more information on credential 
    manager APIs please review the Credential Manager UI section

     http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/authentication_functions.asp




=============================================================================
KNOWN ISSUES

