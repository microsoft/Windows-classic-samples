Sample TCP client and server that connect securely using the secure socket extension of Winsock. The sample code is split into separate directories as described below:

Item		Description
------------	---------------------------------------------------------------
stcpclient      Folder containing the secure TCP client side code
stcpcommon      Folder containing the common library code that is shared between the secure TCP client and server
stcpserver      Folder containing the secure TCP server side code

Notes for running the sample:
It should be noted that the samples are meant to be run on 2 machines running Windows Vista (or future versions). 
Additionally, IPsec credentials must be provisioned on both the machines for the connection to succeed, because the sample uses IPsec for securing its traffic. Please refer to the MSDN documentaion and www.microsoft.com for more information on setting up IPsec. Building the sample will generate 2 executable files: stcpclient.exe and stcpserver.exe. Copy stcpclient.exe to machineA and stcpserver.exe to machineB. On machineB, start the TCP server by running stcpserver.exe. Run "stcpserver.exe /?" for more usage options. Then on machineA, start the TCP client by running "stcpclient.exe <fully-qualified-DNS-name-for-machine-B>". At this point the connection should get established securely.
Run "stcpclient.exe /?" for more usage options.

This works best when you put the two machines in same domain since the default IPsec policy uses Secure Socket and it would be
able to use Kerb to authenticate.
Many sure your environment does not have other overlapped IPsec policies that would interfere with the default or the advanced policy as specified in the sample.

