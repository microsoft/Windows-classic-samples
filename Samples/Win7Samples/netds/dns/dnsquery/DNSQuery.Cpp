/************************************************************\
    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

  Copyright © 2000  Microsoft Corporation.  All Rights Reserved.

/***************************************************************/


/*
   FILE:          Dnsquery.cpp
   DESCRIPTION:   This sample illustrates the use of DnsQuery() function to send query to 
                  a DNS server to resolve the host name to an IP address and vice-versa. 
                  
   PLATFORM:      Windows 2000
   WRITTEN BY:    Rashmi Anoop 
   DATE:          3/22/2000
   

*/

/* 
  includes
*/


#include <windows.h>  //windows
#include <windns.h>   //DNS api's
#include <stdio.h>    //standard i/o
#include <winsock.h>  //winsock

#define BUFFER_LEN 255


//Usage of the program
void Usage(char *progname) {
    fprintf(stderr,"Usage\n%s -n [OwnerName] -t [Type]  -s [DnsServerIp]\n",
            progname);
    fprintf(stderr,"Where:\n\t\"OwnerName\" is name of the owner of the record set being queried\n");
    fprintf(stderr,"\t\"Type\" is the type of record set to be queried A or PTR\n");
    fprintf(stderr,"\t\"DnsServerIp\"is the IP address of DNS server (in dotted decimal notation)");
    fprintf(stderr,"to which the query should be sent\n");
    exit(1);
}

//  the main function 
void __cdecl main(int argc, char *argv[])

{

    DNS_STATUS status;               // return value of  DnsQuery_A() function.
    PDNS_RECORD pDnsRecord;          //pointer to DNS_RECORD structure
    PIP4_ARRAY pSrvList = NULL;      //pinter to IP4_ARRAY structure
    LPTSTR pOwnerName = NULL;        //owner name to be queried 
    WORD wType;                      //Type of the record to be queried
    char DnsServIp[BUFFER_LEN];      //DNS server ip address
    DNS_FREE_TYPE freetype ;
    freetype =  DnsFreeRecordListDeep;
    IN_ADDR ipaddr;


    if (argc > 4) {

        for (int i = 1; i < argc ; i++) {

            if ( (argv[i][0] == '-') || (argv[i][0] == '/') ) {

                switch (tolower(argv[i][1])) {
                
                case 'n':
                    pOwnerName = argv[++i];
                    break;

                case 't':
                    if (!_stricmp(argv[i+1], "A") )
                        wType = DNS_TYPE_A; //Query host records to resolve a name
                    else if (!_stricmp(argv[i+1], "PTR") )
                        wType = DNS_TYPE_PTR; //Query PTR records to resovle an IP address
                    else
                        Usage(argv[0]);
                    i++;
                    break;

                case 's':
                    // Allocate memory for IP4_ARRAY structure
                    pSrvList = (PIP4_ARRAY) LocalAlloc(LPTR,sizeof(IP4_ARRAY));
                    if (!pSrvList) {
                        printf("Memory allocation failed \n");
                        exit(1);
                    }
                    if (argv[++i]) {
                        strncpy_s(DnsServIp, _countof(DnsServIp), argv[i], _TRUNCATE);
                        DnsServIp[sizeof(DnsServIp)-1] = '\0';
                        pSrvList->AddrCount = 1;
                        pSrvList->AddrArray[0] = inet_addr(DnsServIp); //DNS server IP address
                        if ( pSrvList->AddrArray[0] == INADDR_NONE ) {
                            printf("Invalid DNS server IP address \n");
                            Usage( argv[0] );
                        }
                        break; 
                    }

                default:
                    Usage(argv[0]);
                    break;
                }
            }
            else
                Usage(argv[0]);     
        }   

    }
    else
        Usage(argv[0]);


    // Calling function DnsQuery_A() to query Host or PTR records    

    status = DnsQuery_A(pOwnerName,                 //pointer to OwnerName 
                        wType,                      //Type of the record to be queried
                        DNS_QUERY_BYPASS_CACHE,     // Bypasses the resolver cache on the lookup. 
                        pSrvList,                   //contains DNS server IP address
                        &pDnsRecord,                //Resource record comprising the response
                        NULL);                     //reserved for future use

    if (status) {
        if (wType == DNS_TYPE_A)
            printf("Failed to query the host record for %s and the error is %d \n", pOwnerName, status);
        else
            printf("Failed to query the PTR record and the error is %d \n", status);
    }
    else {
        if (wType == DNS_TYPE_A) {

            //convert the Internet network address into a string
            //in Internet standard dotted format.
            ipaddr.S_un.S_addr = (pDnsRecord->Data.A.IpAddress);
            printf("The IP address of the host %s is %s \n", pOwnerName,inet_ntoa(ipaddr));

            // Free memory allocated for DNS records 


            DnsRecordListFree(pDnsRecord, freetype);
        }
        else {
            printf("The host name is %s  \n",(pDnsRecord->Data.PTR.pNameHost));

            // Free memory allocated for DNS records 


            DnsRecordListFree(pDnsRecord, freetype);
        }
    }
    LocalFree(pSrvList);

}



