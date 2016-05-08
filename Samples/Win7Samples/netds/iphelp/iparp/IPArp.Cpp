/*---------------------------------------------------------------------------
    Copyright (c) 1998 - 2000  Microsoft Corporation
    Module Name: IpArp.exe    
    File       : IpArp.cpp
    Description: This file demonstrates the use of IP Helper APIs to
                 manipulate ARP cache.
    Author:
    Frank Li           April 16, 1998

    
    Revision History:
    Who         Wen        What
    --------    --------    ----------------------------------
    Frank Li    04-16-98    created   
---------------------------------------------------------------------------*/

#include "IpArp.h"

#define IPADDR_BUF_SIZE 128
#define IPTYPE_BUF_SIZE 128
#define PHYSADDR_BUF_SIZE 256

void Usage(char * pszProgramName)
{
    printf("%s -s inet_addr eth_addr [if_addr]\n", pszProgramName);
    printf("%s -d inet_addr [if_addr]\n", pszProgramName);
    printf("%s -a\n", pszProgramName);

    printf("-a            Displays current ARP entries by interrogating the current\n");
    printf("              protocol data.\n");
    printf("-d            Deletes the host specified by inet_addr.\n");
    printf("-s            Adds the host and associates the Internet address inet_addr\n");
    printf("              with the Physical address eth_addr.  The Physical address is\n");
    printf("              given as 6 hexadecimal bytes separated by hyphens. The entry\n");
    printf("              is permanent.\n");
    printf("eth_addr      Specifies a physical address.\n");
    printf("if_addr       If present, this specifies the Internet address of the\n");
    printf("              interface whose address translation table should be modified.\n");
    printf("              If not present, the first applicable interface will be used.\n");
    printf("Example:\n");
    printf("   >IpArp -s 157.55.85.212   00-aa-bb-cc-dd-ee 0x2000003 .... Add a static\n");
    printf("                                  arp entry on interface number 0x2000003.\n");
    printf("   >IpArp -a                                    ....Displays the arp table.\n");
    printf("   >IpArp -d 157.55.85.212                      ....Delete an entry.\n");
    WSACleanup();
    exit(1);
}


void _cdecl main(int argc, char **argv)
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    int nRet;

    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {    
        fprintf(stderr,"\n Wrong version\n");
        return;
    }


    if ((argc < 2) || (argv[1][0] != '-'))        
        Usage("IpArp");
    if (strlen(argv[1]) > 2)     
        Usage("IpArp");  

    switch(argv[1][1]) 
    {    
    case 'a':
        // Print arp table
        DoGetIpNetTable();
        break;     
    case 's':      
        //Update or add an ARP Internet/Ethernet Address entry 
        if (argc == 4)
            DoSetIpNetEntry(argv[2], argv[3]);
        else if (argc == 5)
            DoSetIpNetEntry(argv[2], argv[3], argv[4]);
        else
            Usage("IpArp");
        break;    
    case 'd':     
        //Delete an Internet/Ethernet Address pair from the ARP table 
        if (argc == 3)
            DoDeleteIpNetEntry(argv[2]);
        else if (argc == 4)
            DoDeleteIpNetEntry(argv[2], argv[3]);
        else
            Usage("IpArp");                
        break;  
    default:    
        // help
        Usage("IpArp");        
        break;    
    }
    WSACleanup();
}


void DoGetIpNetTable()
{
    DWORD dwStatus;
    PMIB_IPNETTABLE pIpArpTab = NULL;


    if ( (dwStatus = MyGetIpNetTable(pIpArpTab, TRUE)) == NO_ERROR)
    {
        PrintIpNetTable(pIpArpTab);
        free(pIpArpTab);
        return;
    }
    else if ( dwStatus == ERROR_NO_DATA)
    {
        printf("No entries in arp cache.\n");
        if (pIpArpTab)
            free (pIpArpTab);
        return;
    }
    else
    {
        if (pIpArpTab)
            free (pIpArpTab);
        printf("IpArp returned 0x%x\n", dwStatus);
        return;
    }

}

//----------------------------------------------------------------------------
// Add an arp entry with ip dotted decimal address of "pszDottedInetAddr" and
// physical address of "pszPhysAddr" in 00-aa-bb-cc-dd-ee format on interface 
// index of "pszInterface" in hex form.
//----------------------------------------------------------------------------
void DoSetIpNetEntry(char* pszDottedInetAddr, char* pszPhysAddr, char* pszInterface)
{
    DWORD dwInetAddr = 0; // ip address
    DWORD dwStatus;
    BYTE bPhysAddr[MAXLEN_PHYSADDR]; 

    if (pszDottedInetAddr == NULL || pszPhysAddr == NULL)
    {
        printf("IpArp: Bad Argument\n");
        return;
    }

    dwInetAddr = inet_addr(pszDottedInetAddr); // convert dotted ip addr. to ip addr.
    if (dwInetAddr == INADDR_NONE)
    {
        printf("IpArp: Bad Argument %s\n", pszDottedInetAddr);
        return;
    }

    if (StringToPhysAddr(pszPhysAddr, (char*)bPhysAddr ) != 0)
    {
        printf("IpArp: Bad Argument %s\n", pszPhysAddr);
        return;
    }
    MIB_IPNETROW arpEntry; // an arp entry
    if (pszInterface)
    {
        // User provides a interface index number
        sscanf_s(pszInterface, "%X",&(arpEntry.dwIndex));
    }
    else
    {
        // add this to the first available interface
        PMIB_IPADDRTABLE pIpAddrTable = NULL;
        if ( (dwStatus = MyGetIpAddrTable(pIpAddrTable)) != NO_ERROR)
        {
            printf("IpArp: Couldn't find a interface number to add your arp entry\n");
            return;
        }
        arpEntry.dwIndex = pIpAddrTable->table[0].dwIndex;
        free(pIpAddrTable);
    }
    arpEntry.dwPhysAddrLen = 6;
    memcpy(arpEntry.bPhysAddr, bPhysAddr, 6);
    arpEntry.dwAddr = dwInetAddr;
    arpEntry.dwType = MIB_IPNET_TYPE_STATIC; //static arp entry
    dwStatus = SetIpNetEntry(&arpEntry);
    if (dwStatus != NO_ERROR)
    {
        printf("IpArp: couldn't add (%s, %s), dwStatus = %lu.\n",
                    pszDottedInetAddr, pszPhysAddr, dwStatus);
    }
}
//----------------------------------------------------------------------------
// Delete an arp entry with ip dotted decimal address of "pszDottedInetAddr" 
// and interface index of "pszInterface" in hex form.
//----------------------------------------------------------------------------
void DoDeleteIpNetEntry(char* pszDottedInetAddr, char* pszInterface)
{
    DWORD dwInetAddr = 0; // ip address
    DWORD dwStatus;

    if (pszDottedInetAddr == NULL)
    {
        printf("IpArp: Bad Argument\n");
        return;
    }
    dwInetAddr = inet_addr(pszDottedInetAddr); // convert dotted ip addr. to ip addr.
    if (dwInetAddr == INADDR_NONE)
    {
        printf("IpArp: Bad Argument %s\n", pszDottedInetAddr);
        return;
    }

    MIB_IPNETROW arpEntry; // an arp entry
    if (pszInterface)
    {
        // User provides a interface index number
        sscanf_s(pszInterface, "%X",&(arpEntry.dwIndex));
    }
    else
    {
        // try to delete this from first available interface
        PMIB_IPADDRTABLE pIpAddrTable = NULL;
        if ( (dwStatus = MyGetIpAddrTable(pIpAddrTable)) != NO_ERROR)
        {
            printf("IpArp: Couldn't find a interface number to add your arp entry\n");
            return;
        }
        arpEntry.dwIndex = pIpAddrTable->table[0].dwIndex;
        free(pIpAddrTable);
    }
    
    arpEntry.dwAddr = dwInetAddr;
    dwStatus = DeleteIpNetEntry(&arpEntry);
    if (dwStatus != NO_ERROR)
    {
        printf("IpArp: couldn't delete (%s), dwStatus = %lu.\n",
                    pszDottedInetAddr, dwStatus);
    }


}
//----------------------------------------------------------------------------
// Inputs: pIpAddrTable is the IP address table
//         dwIndex is the Interface Number
// Output: If it returns TRUE, str contains the ip address of the interface
//----------------------------------------------------------------------------
bool InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIndex, char str[])
{
    struct in_addr inadTmp;
    char* szIpAddr;

    if (pIpAddrTable == NULL ||  str == NULL)
        return FALSE;
    str[0] = '\0';
    for (DWORD dwIdx = 0; dwIdx < pIpAddrTable->dwNumEntries; dwIdx++)
    {
        if (dwIndex == pIpAddrTable->table[dwIdx].dwIndex)
        {
            inadTmp.s_addr = pIpAddrTable->table[dwIdx].dwAddr;
            szIpAddr = inet_ntoa(inadTmp);
            if (szIpAddr)
            {
                strcpy_s(str, IPADDR_BUF_SIZE, szIpAddr);
                return TRUE;
            }
            else
                return FALSE;
        }
    }
    return FALSE;

}

//-------------------------------------------------------------------------
// Input: str points to an ethernet address of the form 00-aa-bb-cc-dd-ee
// If it returns 0, ret contains the 6 bytes ethernet address.
//-------------------------------------------------------------------------

int StringToPhysAddr(char* szInEther,  char* szOutEther)
{
    const char DASH = '-';
    register char c;    
    register int val;    

    // check szInEther for the correct format
    if (strlen(szInEther) != 17)
        return (-1);
    if (szInEther[2] != DASH || szInEther[5] != DASH || szInEther[8] != DASH ||
        szInEther[8] != DASH || szInEther[14] != DASH)
        return (-1);
    if (!isxdigit(szInEther[0]) || !isxdigit(szInEther[1]) ||
        !isxdigit(szInEther[3]) || !isxdigit(szInEther[4]) ||
        !isxdigit(szInEther[6]) || !isxdigit(szInEther[7]) ||
        !isxdigit(szInEther[9]) || !isxdigit(szInEther[10]) ||
        !isxdigit(szInEther[12]) || !isxdigit(szInEther[13]) ||
        !isxdigit(szInEther[15]) || !isxdigit(szInEther[16]))
        return (-1);
    // convert the 12 hex decimals back to 6 digit decimals
    for (int i = 0; i < 6; i++)
    {
        val = 0;
        c = toupper(szInEther[i*3]);
        c = c - (isdigit(c) ? '0' : ('A' - 10)); //offset adjustment
        val += c;
        val = (val << 4); // val * 16 
        c = toupper(szInEther[i*3 + 1]);
        c = c - (isdigit(c) ? '0' : ('A' - 10)); // offset adjustement
        val += c;
        szOutEther[i] = val;
    }
    return (0);
}

//----------------------------------------------------------------------------
// Inputs: PhysAddr is the hardware address in bytes
//         PhysAddrLen is the length of the PhysAddr
// Outputs: if it returns TRUE, str is the hex formated string of the hardware
//          address.
// NOTE: make sure str is TRIPLE as big as PhysAddrLen
//----------------------------------------------------------------------------
bool PhysAddrToString(BYTE PhysAddr[], DWORD PhysAddrLen, char str[])
{
    if (PhysAddr == NULL || PhysAddrLen == 0 || str == NULL)
        return FALSE;
    str[0] = '\0';
    for (DWORD dwIdx = 0; dwIdx < PhysAddrLen; dwIdx++)
    {
        if (dwIdx == PhysAddrLen-1)
            sprintf_s(str+(dwIdx*3), IPADDR_BUF_SIZE-(dwIdx*3), "%02X", ((int)PhysAddr[dwIdx])&0xff);
        else
            sprintf_s(str+(dwIdx*3), IPADDR_BUF_SIZE-(dwIdx*3), "%02X-", ((int)PhysAddr[dwIdx])&0xff);
        
    }
    return TRUE;
}
//----------------------------------------------------------------------------
//   arp table format to be printed:
// Interface: 157.61.239.34 on Interface 2
//   Internet Address      Physical Address      Type
//   159.61.230.39         00-aa-00-61-5d-a4     dynamic
//
// Interface: 157.54.178.219 on Interface 3
//   Internet Address      Physical Address      Type
//   159.54.170.1          00-10-54-42-c0-88     dynamic
//   159.54.170.113        00-aa-00-c0-80-2e     dynamic
//----------------------------------------------------------------------------
void PrintIpNetTable(PMIB_IPNETTABLE pIpNetTable)
{
    DWORD i, dwStatus, dwCurrIndex;
    struct in_addr inadTmp;
    char szPrintablePhysAddr[PHYSADDR_BUF_SIZE];
    char szType[IPTYPE_BUF_SIZE];
    char szIpAddr[IPADDR_BUF_SIZE];
    PMIB_IPADDRTABLE pIpAddrTable = NULL;

    if (pIpNetTable == NULL)
    {
        printf( "pIpNetTable == NULL in line %d\n", __LINE__);
        return;
    }
    // get IP Address Table for mapping interface index number to ip address
    if ( (dwStatus = MyGetIpAddrTable(pIpAddrTable)) != NO_ERROR)
    {
        printf("GetIpAddrTable returned 0x%x\n", dwStatus);
        if (pIpAddrTable)
            free(pIpAddrTable);
        return;
    }
    assert(pIpAddrTable);
    // Note: the ARP table should be sorted in interface index
    dwCurrIndex = pIpNetTable->table[0].dwIndex;
    if (InterfaceIdxToInterfaceIp(pIpAddrTable, dwCurrIndex, szIpAddr))
    {
        printf("\nInterface: %s on Interface 0x%X\n", szIpAddr, dwCurrIndex);
        printf("  Internet Address      Physical Address      Type\n");
    }
    else
    {
        printf("Error: Could not convert Interface number 0x%X to IP address.\n",
                    pIpNetTable->table[0].dwIndex);
        return;
    }
    
    
    for (i = 0; i < pIpNetTable->dwNumEntries; ++i)
    {
        if (pIpNetTable->table[i].dwIndex != dwCurrIndex)
        {
            dwCurrIndex = pIpNetTable->table[i].dwIndex;
            if (InterfaceIdxToInterfaceIp(pIpAddrTable, dwCurrIndex, szIpAddr))
            {
                printf("Interface: %s on Interface 0x%X\n", szIpAddr, dwCurrIndex);
                printf("  Internet Address      Physical Address      Type\n");
            }
            else
            {
                printf("Error: Could not convert Interface number 0x%X to IP address.\n",
                    pIpNetTable->table[0].dwIndex);
                return;
            }
        }
        PhysAddrToString(pIpNetTable->table[i].bPhysAddr, pIpNetTable->table[i].dwPhysAddrLen, szPrintablePhysAddr);
        inadTmp.s_addr = pIpNetTable->table[i].dwAddr;
        switch (pIpNetTable->table[i].dwType)
        {
        case 1:
            strcpy_s(szType, IPTYPE_BUF_SIZE, "other");
            break;
        case 2:
            strcpy_s(szType, IPTYPE_BUF_SIZE, "invalidated");
            break;
        case 3:
            strcpy_s(szType, IPTYPE_BUF_SIZE, "dynamic");
            break;
        case 4: 
            strcpy_s(szType, IPTYPE_BUF_SIZE, "static");
            break;
        default:
            strcpy_s(szType, IPTYPE_BUF_SIZE, "invalidType");
        }
        printf("  %-16s      %-17s     %-11s\n", inet_ntoa(inadTmp), szPrintablePhysAddr, szType);
        
    }
    if (pIpAddrTable)
        free(pIpAddrTable);
}


//----------------------------------------------------------------------------
//  Format of IP Address Table:
//  ipAdEntAddr   ifAdEntIfIndex  ipAdEntNetMask   ipAdEntBcastAddr  ipAdEntReasmMaxSize
//----------------------------------------------------------------------------
void PrintIpAddrTable(PMIB_IPADDRTABLE pIpAddrTable)
{
    DWORD i;
    struct in_addr inadTmp1;
    struct in_addr inadTmp2;
    char szAddr[IPADDR_BUF_SIZE];
    char szMask[IPADDR_BUF_SIZE];

    if (pIpAddrTable == NULL)
    {
        printf( "pIpAddrTable == NULL in line %d\n", __LINE__);
        return;
    }
    printf("ipAdEntAddr\t ifAdEntIfIndex\t ipAdEntNetMask\t ipAdEntBcastAddr\t ipAdEntReasmMaxSize\n");
    for (i = 0; i < pIpAddrTable->dwNumEntries; ++i)
    {
        inadTmp1.s_addr = pIpAddrTable->table[i].dwAddr;
        strcpy_s(szAddr, IPADDR_BUF_SIZE, inet_ntoa(inadTmp1));
        inadTmp2.s_addr = pIpAddrTable->table[i].dwMask;
        strcpy_s(szMask, IPADDR_BUF_SIZE, inet_ntoa(inadTmp2));
        printf("  %s\t 0x%X\t %s\t %s\t %u\n",
                szAddr, 
                pIpAddrTable->table[i].dwIndex,
                szMask,
                (pIpAddrTable->table[i].dwBCastAddr ? "255.255.255.255" : "0.0.0.0"),
                pIpAddrTable->table[i].dwReasmSize);

    }
}

//----------------------------------------------------------------------------
// Input : fOrder -- sorts the output IP Addr Table
// Output: If it returns NO_ERROR, pIpAddrTable points to the IP Addr Table
//----------------------------------------------------------------------------
DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, bool fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;


    // query for buffer size needed
    status = GetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        printf("No error\n");
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        pIpAddrTable = (PMIB_IPADDRTABLE) malloc(dwActualSize);
        assert(pIpAddrTable);
        
        statusRetry = GetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

//----------------------------------------------------------------------------
// Input : fOrder -- sorts the output IP Net Table
// Output: If it returns NO_ERROR, pIpNetTable points to the IP Net Table
//----------------------------------------------------------------------------
DWORD MyGetIpNetTable(PMIB_IPNETTABLE& pIpNetTable, bool fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;
    
    // query for buffer size needed
    dwActualSize = 0;
    status = GetIpNetTable(pIpNetTable, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        pIpNetTable = (PMIB_IPNETTABLE) malloc(dwActualSize);
        assert(pIpNetTable);
        
        statusRetry = GetIpNetTable(pIpNetTable, &dwActualSize, fOrder);

        if (statusRetry != NO_ERROR)
        {
#ifdef _DEBUG
            printf("Retry failed.\n");
#endif
            return statusRetry;
        }
        else
        {
            return statusRetry;
        }
    }
    else
    {
#ifdef _DEBUG
        printf("first getipnettable call failed\n");
#endif
        return status;
    }
}



