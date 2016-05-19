    This module illustrates the Win32 Winsock and Mailslot APIs to do a generic
	broadcast over IPX, UDP and Mailslot protocols.

    This example implements a client and a server. The example has a number
	command line options. For example,

    -s To run the example as a server(default role).
	
    -c To run the example as a client.
   
	-p <i or m or u> To specify the protocol to be used.
	 i - IPX.
	 m - Mailslots.
	 u - UDP(default protocol).
	
    -e <Endpoint> To specify an end point of your choice. This is a mandatory
	parameter. Servers create this endpoint and read broadcast messages. An 
	endpoint in case Mailslot protocol is a Mailslot name.(default is 5005). 
	
    -d <DomainName> - To specify a domain name or a workgroup name. This is 
	useful for Mailslot clients, only.

	To run the application as a server, the following command lines can be 
	specified:
    
	bcast -s -e 8000 -p u
	bcast -s -e 8000 -p i
	bcast -s -e MAILSLOT1 -p m

	To run the application as a client, the following command lines can be
	specified:
	
	bcast -c -e 8000 -p u
	bcast -c -e 8000 -p i
	bcast -c -e MAILSLOT1 -p m -d DOMAIN1
	bcast -c -e MAILSLOT1 -p m -d WORKGROUP1
