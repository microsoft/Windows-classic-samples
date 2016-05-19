===============================================================
EVENT LOG CONSUMER SAMPLES
===============================================================


DESCRIPTION
===============================
These samples demonstrate Event Log consumption concepts using the Evt APIs.

The EventsQuery sample demonstrates the following concepts:
    1.  Enumeration of channels
    2.  Querying of channels
    3.  Filtering on channels

The GetEventRawDescription sample demonstrates the following concepts:
    1.  Retrieving raw descriptions from events
    2.  Querying provider configuration

The ChangeChannelConfig sample demonstrates the following concepts:
    1.  Configuring the maximum file size for a channel

The PullSubscription sample demonstrates the following concepts:
    1. Subscribing to events through a pull-based model
    2. Using bookmarks to pause and resume consumption

The PushSubscription sample demonstrates the following concepts:
    1. Subscribing to events through a push-based model


SAMPLE LANGUAGE IMPLEMENTATIONS
===============================
     This sample is available in the following language implementations:
     C/C++


FILES
===============================
ChangeMaxSize.cpp
        Configures the maximum file size for a channel.

Events.c
        Queries, enumerates, and filters channels.

GetEventRawDescription.cpp 
        Retrieves raw message strings from events and queries provider configuration.

PullSubscription.cpp
        Subscribes to a channel and query.  When events arrive that match the query, the current event is bookmarked and the user is presented with details of the event.  When the user resumes, the next event is retrieved from the bookmarked location.

PushSubscription.cpp
        Subscribes to a channel and query.  When events arrive that match query, the user is presented with details of event.  Note that because this sample demonstrates EvtRender usage, it will have different output from PullSubscription.cpp.
 

BUILD
===============================

To build the samples using msbuild:
=============================================
1.  Open a Command Prompt window and navigate to the following directory:
        Samples\WinBase\Eventing\EventLogConsumer
2. Type msbuild EventLogSamples.sln.

To build the samples using Visual Studio:
================================================
1.  Open Windows Explorer and navigate to the following directory:
        Samples\WinBase\Eventing\EventLogConsumer
2. Double-click the icon for the EventLogSamples.sln solution file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. 

To build an individual sample from its makefile:
================================================
1.  Open a Command Prompt window and navigate to a subdirectory of the following directory:
        Samples\WinBase\Eventing\EventLogConsumer
2. Type "nmake" to build the sample.


RUN
===============================

To run the sample:
=================
1. Navigate to the directory that contains the executable for the desired sample using the command prompt.
2. Type the name of the executable at the command line to view usage details.

