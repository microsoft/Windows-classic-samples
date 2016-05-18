Security Client
===============
       WMI Client Security Sample.  This sample shows how to 
handle various DCOM security issues.  In particular, it shows how to call
CoSetProxyBlanket in order to deal with common situations.  The sample also
shows how to access the security descriptors that wmi uses to control
namespace access. The sample also demonstrates how to use Credential Manager API for
password prompting: this functionality is only implemented on Windows XP and above.

WIN2K LIMITATION
================
Therefore, you will need to remove CredUI code to make the sample run on Win2k - it 
will compile and build on Win2k anyway.

To get the sample working, do the following;

1) Build using NMAKE. 
2) Run it.
