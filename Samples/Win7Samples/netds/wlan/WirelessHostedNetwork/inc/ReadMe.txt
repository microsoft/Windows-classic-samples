========================================================================
    INC : Common Include Files Overview
========================================================================

This file contains a summary of what you will find in each of the files that
make up the common include files.


common.h
    This is the header file that contains the common classes. It declares the 
    CRefObject class that supports reference and dereference. 

icsconn.h
    This is the header file used to enable and disable Internet Connection Sharing (ICS).
    It declares the CIcsConnection class 
    
icsmgr.h
    This is the header file used to interface between the HostedNetwork application and ICS.
    It declares the CIcsManager and CIcsConnectionInfo classes. 
    
WlanMgr.h
    This is the header file used to start and stop the HostedNetworkinterface and
    set parameters for the HostedNetwork. It declares the CWlanManager, CWlanStation,
    and CHostedNetworkNotificationSink classes. 
    


