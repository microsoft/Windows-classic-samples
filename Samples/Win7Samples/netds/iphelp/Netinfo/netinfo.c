#include "netinfo.h"

VOID
PrintIpAddress(
    IN CONST SOCKADDR_INET *IpAddress
    )  
/*++

Routine Description:

    Print an IP address.

Arguments:

    IpAddress - Supplies IP address to be converted.
    
    AddressString - Returns the string.

    BufferLength - Supplies and returns the string length.

Returns:

    NONE.
    
--*/
{
    ULONG i;

    if (IpAddress->si_family == AF_INET6) {
		ULONG CurrentRunStart = 0, CurrentRunLength = 0;
		ULONG MaxRunStart = 0, MaxRunLength = 0;
		//
		// Search for the longest run of consecutive zeros.
		//
        for (i = 0; i < 7; i++) {
			if (IpAddress->Ipv6.sin6_addr.u.Word[i] == 0) {
				CurrentRunLength++;
				if (CurrentRunLength > MaxRunLength) {
					MaxRunStart = CurrentRunStart;
					MaxRunLength = CurrentRunLength;
				}
			} else {
				CurrentRunStart = i + 1;
				CurrentRunLength = 0;
			}
		}
			
		if (MaxRunLength > 1) {
			for (i = 0; i < MaxRunStart; i++) {
				printf("%x:", ntohs(IpAddress->Ipv6.sin6_addr.u.Word[i]));
			}
			if (MaxRunStart == 0) {
				printf(":");
			}
			for (i = MaxRunStart + MaxRunLength; i < 8; i++) {
				printf(":%x", ntohs(IpAddress->Ipv6.sin6_addr.u.Word[i]));
			}
			if (MaxRunStart + MaxRunLength >= 8) {
				printf(":");
			}
		} else {
			for (i = 0; i < 7; i++) {
				printf("%x:",ntohs(IpAddress->Ipv6.sin6_addr.u.Word[i]));
			}
			printf("%x",ntohs(IpAddress->Ipv6.sin6_addr.u.Word[i]));  
		}

    } else {
        printf(
            "%d.%d.%d.%d",
            IpAddress->Ipv4.sin_addr.s_net,
            IpAddress->Ipv4.sin_addr.s_host,
            IpAddress->Ipv4.sin_addr.s_lh,
            IpAddress->Ipv4.sin_addr.s_impno);  
    }
}

VOID
__stdcall AddressCallbackDemo(
    IN PVOID CallerContext,
    IN PMIB_UNICASTIPADDRESS_ROW Address OPTIONAL,
    IN MIB_NOTIFICATION_TYPE NotificationType
    )
{
    UNREFERENCED_PARAMETER(CallerContext);
    printf("Address Change Notification Received.\n");
    printf("Notification Type is %d.\n", NotificationType);   

    if (Address != NULL) {
        printf("Interface Index %d.\n", Address->InterfaceIndex);
        printf("Address:  ");
        PrintIpAddress(&Address->Address);
        printf("\n");
    }
}

VOID 
InterfaceCallbackDemo(
    IN PVOID CallerContext,
    IN PMIB_IPINTERFACE_ROW InterfaceRow,
    IN MIB_NOTIFICATION_TYPE NotificationType 
    )
{
    UNREFERENCED_PARAMETER(CallerContext);
    
    printf("Interface Change Notification Received.\n");
    printf("Notification Type is %d.\n", NotificationType);
    
    if (InterfaceRow != NULL) {
        printf("Interface Index: %d.\n", InterfaceRow->InterfaceIndex);  
    }
}

VOID
AddressDemo(
    VOID
    )
{
    NETIO_STATUS Status;
    PMIB_UNICASTIPADDRESS_TABLE UnicastTable = NULL;
    MIB_UNICASTIPADDRESS_ROW Row;
    ULONG i;
    IN6_ADDR Ipv6Address = {0xfe,0x3f,0,0,0,0,0,0,0,0,0,0,0,0,0x20,0x00};
    HANDLE Handle;
    NET_IFINDEX InterfaceIndex = 0;

    //
    // Retrieve all the IP addresses.
    //
    
    Status = 
        GetUnicastIpAddressTable(
            AF_UNSPEC, &UnicastTable);
    if (!WIN_SUCCESS(Status)) {
        printf(
            "GetUnicastAddressTable Failed. Error %d\n", 
            Status);
    } else {
        printf(
            "GetUnicastAddressTable Succeeded.\n");
        printf(
            "Total Number of all IP Address Entries: %d.\n",
            UnicastTable->NumEntries);
        for (i = 0; i < UnicastTable->NumEntries; i++) {
            printf("Address %2d:  ", i);
            PrintIpAddress(&UnicastTable->Table[i].Address);
			      printf("\n");
        }
        FreeMibTable(UnicastTable); 
        printf("\n\n");
    }   
    
    //
    // Retrieve IPv6 Only Addresses.
    //

    Status = 
        GetUnicastIpAddressTable(
            AF_INET6, &UnicastTable);
    if (!WIN_SUCCESS(Status)) {
        printf(
            "GetUnicastAddressTable Failed. Error %d\n", 
            Status);
    } else {
        printf(
            "GetUnicastAddressTable Succeeded.\n");
        printf(
            "Total Number of IPv6 Address Entries: %d.\n",
            UnicastTable->NumEntries);
        for (i = 0; i < UnicastTable->NumEntries; i++) {
            printf("Address %2d:  ", i);
            PrintIpAddress(&UnicastTable->Table[i].Address);
            printf("\n");
        }
        InterfaceIndex = UnicastTable->Table[i - 1].InterfaceIndex;
        FreeMibTable(UnicastTable);        
    }

    Status = 
        NotifyUnicastIpAddressChange(
            AF_UNSPEC, 
            //(PUNICAST_IPADDRESS_CHANGE_CALLBACK)AddressCallbackDemo,
            &AddressCallbackDemo,
            NULL,
            FALSE, 
            &Handle);

    if (!WIN_SUCCESS(Status)) {
        printf(
            "\nRegister address change failed. Error %d\n", 
            Status);
    } else {
        printf(
            "\nRegister address change succeeded.\n");     
    } 
    InitializeUnicastIpAddressEntry(&Row);
    Row.Address.si_family = AF_INET6;
    INETADDR_SET_ADDRESS((PSOCKADDR)&Row.Address, (PUCHAR)&Ipv6Address);
    Row.InterfaceIndex = InterfaceIndex;
    Status = 
        CreateUnicastIpAddressEntry(&Row);
    if (!WIN_SUCCESS(Status)) {
        printf("Create IPv6 unicast address entry failed. Error %d\n", 
            Status);
    } else {
        printf("Create IPv6 unicast address entry succeeded.\n"); 
    }

    Status = 
        GetUnicastIpAddressEntry(&Row);
    if (!WIN_SUCCESS(Status)) {
        printf("Get IPv6 unicast address failed. Error %d\n", Status);
    } else {
        printf("Get IPv6 unicast address entry succeeded.\n"); 
    }

    Row.PreferredLifetime = 500000;
    Status = 
        SetUnicastIpAddressEntry(&Row);
    if (!WIN_SUCCESS(Status)) {
        printf("Set IPv6 unicast address entry failed. Error %d\n", 
            Status);
    } else {
        printf("Set IPv6 unicast address entry succeeded.\n"); 
    }
    
    Status = 
        DeleteUnicastIpAddressEntry(&Row);
    if (!WIN_SUCCESS(Status)) {
        printf("Delete Ipv6 Unicast Address Failed. Error %d\n", 
            Status);
    } else {
        printf("Delete Ipv6 Unicast Address Succeeded.\n"); 
    }

    Sleep(2000);
    Status = CancelMibChangeNotify2(Handle);
    if (!WIN_SUCCESS(Status)) {
        printf(
            "Deregister address change failed. Error %d\n\n", 
            Status);
    } else {
        printf(
            "Deregister address change succeeded.\n\n");     
    }
}

VOID
IpInterfaceDemo(
    VOID
    )
{
    ULONG InterfaceCount = 0, Index = 0, i;
    ULONG Status;
    
    PMIB_IPINTERFACE_TABLE InterfaceTable = NULL;   
  
    Status = GetIpInterfaceTable(AF_UNSPEC, &InterfaceTable);
            
    if (!WIN_SUCCESS(Status)) {
        printf("Get IpInterfaceTable failed. Error %d\n", Status);
    } else {
        printf("Get IpInterfaceTable succeeded.\n");
        InterfaceCount = InterfaceTable->NumEntries;
        printf(
            "Total number of Interfaces: %d\n", 
            InterfaceTable->NumEntries);
        for (i = 0; i < InterfaceCount; i++) {
            printf("\n");
            printf(
                "Interface Index: %d\n", 
                InterfaceTable->Table[i].InterfaceIndex);
            printf(
                "Address Family: %ws\n",
                (InterfaceTable->Table[i].Family == AF_INET) ? 
                    L"IPv4" : L"IPv6");
            printf(
                "AdvertisingEnabled: %ws\n",
                InterfaceTable->Table[i].AdvertisingEnabled ?
                    L"TRUE" : L"FALSE");
            printf(
                "UseAutomaticMetric: %ws\n",
                InterfaceTable->Table[i].UseAutomaticMetric ?
                    L"TRUE" : L"FALSE");          
            printf(
                "ForwardingEnabled: %ws\n",
                InterfaceTable->Table[i].ForwardingEnabled ?
                    L"TRUE" : L"FALSE");
            printf(
                "WeakHostSend: %ws\n",
                InterfaceTable->Table[i].WeakHostSend ?
                    L"TRUE" : L"FALSE");
            printf(
                "WeakHostReceive: %ws\n",
                InterfaceTable->Table[i].WeakHostReceive ?
                    L"TRUE" : L"FALSE");
            printf(
                "UseNeighborUnreachabilityDetection: %ws\n",
                InterfaceTable->Table[i].UseNeighborUnreachabilityDetection ?
                L"TRUE" : L"FALSE");
            printf(
                "AdvertiseDefaultRoute: %ws\n",
                InterfaceTable->Table[i].AdvertiseDefaultRoute ?
                L"TRUE" : L"FALSE");            
        }
        FreeMibTable(InterfaceTable);
    }
}

int __cdecl
main(
    __in int argc, 
    __in_ecount(argc) char **argv
    )
{
    AddressDemo();
    IpInterfaceDemo();
}
