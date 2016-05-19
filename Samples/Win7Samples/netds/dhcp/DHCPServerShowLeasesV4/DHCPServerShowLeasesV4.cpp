/*
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Copyright ©  Microsoft Corporation.  All Rights Reserved.
 *  
 * Author: Yashlaxmi Gupta
 * Abstract:
 *    Dump IPv4 leases across all the scopes configured on a DHCP server.
 *
 */

/*
 * Includes
 */
#include <stdio.h> 
#include <tchar.h> 
#include <windows.h>
#include <strsafe.h>
#include <intsafe.h>
#include <dhcpsapi.h>

#define SERVERNAME_BUF_SIZE 255
#define IPADDR_BUF_SIZE 36
#define MAX_DATE_TIME_BUF_SIZE 64

#define DHCP_DATE_TIME_INFINITE_LOW 0xFFFFFFFF
#define DHCP_DATE_TIME_INFINITE_HIGH 0x7FFFFFFF
#define DHCP_DATE_TIME_ZERO_LOW 0
#define DHCP_DATE_TIME_ZERO_HIGH 0

WCHAR *ConvertIpAddtoWstr(
    __in DHCP_IP_ADDRESS addr
);
WCHAR *GetHardwareAddress(
    __in DHCP_CLIENT_UID phyAdd
);
WCHAR *GetDateTimeString(
    __in FILETIME ftTime
);

DWORD __cdecl main(int argc, char* argv[])
{
    DHCP_RESUME_HANDLE ResumeHandleScope = 0;
    LPDHCP_IP_ARRAY EnumScopes = NULL;
    DWORD nRead = 0;
    DWORD nTotal = 0; 
    
    DHCP_RESUME_HANDLE ResumeHandleClient = 0;
    LPDHCP_CLIENT_INFO_ARRAY_V5 Clients = NULL;
    DWORD nClientsRead = 0;
    DWORD nClientsTotal = 0;

	DWORD error1 = ERROR_SUCCESS, error2 = ERROR_SUCCESS;
	WCHAR *szDateTimeStr = NULL, *szClientIp = NULL, *szSubnetMask = NULL, *szHardwareAdd = NULL, *szScopeIp = NULL;
    WCHAR szServer[SERVERNAME_BUF_SIZE]={0};
    if (2 != argc)
    {
        wprintf(L"Usage: DhcpServerShowLeasesV4.exe <Server IpAdd/Name>");
        return ERROR_INVALID_PARAMETER;
    }
    MultiByteToWideChar(0, 0, argv[1], (int)strlen(argv[1]), szServer, SERVERNAME_BUF_SIZE);      
	do {
        // enumerate all the IpV4 scopes on the server.
        error1 = DhcpEnumSubnets(szServer, &ResumeHandleScope, (DWORD)~0, &EnumScopes, &nRead, &nTotal);
        if (0 == nTotal)
        {
            wprintf(L"No scopes on this server.\n\n");
        }
        if (ERROR_NO_MORE_ITEMS == error1)
        {
            break;
        }
	    if (ERROR_SUCCESS != error1 && ERROR_MORE_DATA != error1)
	    {
            wprintf(L"DhcpServerShowLeasesV4 returned with error:  %d\n",error1); 
            return error1;
	    }
        // iterating over all the scopes one by one to get the clients for each scope.
        for (unsigned int count=0; count < EnumScopes->NumElements; count++)
	    {
            // converting scope IP address from DWORD to LPWSTR
            szScopeIp = ConvertIpAddtoWstr(EnumScopes->Elements[count]);
            wprintf(L"\nScope : %s\n\n",szScopeIp ? szScopeIp : L" ");
		    do {
                // enumerating the clients on a specific IpV4 scope 
                error2 = DhcpEnumSubnetClientsV5(szServer, EnumScopes->Elements[count], &ResumeHandleClient, (DWORD)~0, &Clients, &nClientsRead, &nClientsTotal);
                if (0 == nClientsTotal)
                {
                    wprintf(L"No clients on this scope.\n\n");
                }
                if (ERROR_NO_MORE_ITEMS == error2)
                {
                    break;
                }
                if (ERROR_SUCCESS != error2 && ERROR_MORE_DATA != error2)
		        {
			        wprintf(L"DhcpServerShowLeasesV4 returned with error:  %d\n",error2); 
                    return error2;
		        }
                //iterating over all the clients leases on a specified scope.
                //the leases shown include all the leases present and can be filtered out on the basic of Address State attribute of client.
                for (unsigned int count = 0; count < Clients->NumElements; count++)
  	            {
                    //converting client Ip from DWORD to LPWSTR
                    szClientIp = ConvertIpAddtoWstr(Clients->Clients[count]->ClientIpAddress);
                    wprintf(L"Client Address   : %s\n",szClientIp ? szClientIp : L" ");
                    //converting scope subnet mask from DWORD to LPWSTR
                    szSubnetMask = ConvertIpAddtoWstr(Clients->Clients[count]->SubnetMask);
                    wprintf(L"Subnet Mask      : %s\n",szSubnetMask ? szSubnetMask : L" ");
                    //converting client hardware address from byte * to LPWSTR
                    szHardwareAdd = GetHardwareAddress(Clients->Clients[count]->ClientHardwareAddress);
                    wprintf(L"Hardware Address : %s\n",szHardwareAdd ? szHardwareAdd : L" ");
                    // if lease duration is infinite then lease never expires.
                    if (DHCP_DATE_TIME_INFINITE_LOW == Clients->Clients[count]->ClientLeaseExpires.dwLowDateTime && 
                        DHCP_DATE_TIME_INFINITE_HIGH == Clients->Clients[count]->ClientLeaseExpires.dwHighDateTime)
                    {
                        wprintf(L"Lease Expires    : Never\n");
                    }
                    // if lease duration is 0 the client is inactive.
                    else if (DHCP_DATE_TIME_ZERO_LOW == Clients->Clients[count]->ClientLeaseExpires.dwLowDateTime && 
                             DHCP_DATE_TIME_ZERO_HIGH == Clients->Clients[count]->ClientLeaseExpires.dwHighDateTime)
                    {
                        wprintf(L"Lease Expires    : Inactive\n");
                    }
                    else
                    {
                        // converting the date time information from FILETIME * to LPWSTR.
                        szDateTimeStr = GetDateTimeString(*(FILETIME *)(&Clients->Clients[count]->ClientLeaseExpires));
                        wprintf(L"Lease Expires    : %s\n", szDateTimeStr ? szDateTimeStr : L" ");
                    }
                    wprintf(L"Client Name      : %s\n",Clients->Clients[count]->ClientName ? Clients->Clients[count]->ClientName : L" ");
                    wprintf(L"Client Comment   : %s\n\n",Clients->Clients[count]->ClientComment ? Clients->Clients[count]->ClientComment : L" ");
                    if (NULL != szClientIp)
                    {
                        free(szClientIp);
                        szClientIp = NULL;
                    }
                    if (NULL != szSubnetMask)
                    {
                        free(szSubnetMask);
                        szSubnetMask = NULL;
                    }
                    if (NULL != szHardwareAdd)
                    {
                        free(szHardwareAdd);
                        szHardwareAdd = NULL;
                    }
                    if (NULL != szDateTimeStr)
                    {
                        free(szDateTimeStr);
                        szDateTimeStr = NULL;
                    }
   	            }
                if (NULL != Clients)
                {
                    DhcpRpcFreeMemory(Clients);
                    Clients = NULL;
                }
                nClientsRead = 0;
		        nClientsTotal = 0;
                ResumeHandleClient = 0;

            } while (ERROR_MORE_DATA == error2);
            if (NULL != szScopeIp)
            {
                free(szScopeIp);
                szScopeIp = NULL;
            }
        }
        if (NULL != EnumScopes)
        {
            DhcpRpcFreeMemory(EnumScopes);
            EnumScopes = NULL;
        }
        nRead = 0;
        nTotal = 0;
        ResumeHandleScope = 0;
        
    } while (ERROR_MORE_DATA == error1);
	return 0;
}

WCHAR *ConvertIpAddtoWstr(
    __in DHCP_IP_ADDRESS addr
)
{   
    // getting the individual bytes of the IP Address.
    PUCHAR pAddress = (PUCHAR)&addr;
    // allocating memory to IP Address string.
    WCHAR *szIpAddress = (WCHAR *)malloc(IPADDR_BUF_SIZE);
    if (NULL == szIpAddress)
    {
        return NULL;
    }
    memset(szIpAddress, 0, IPADDR_BUF_SIZE);
    // creating the IP Address string
    HRESULT HRes = StringCchPrintfW(szIpAddress, IPADDR_BUF_SIZE, L"%ld.%ld.%ld.%ld",pAddress[3],pAddress[2],pAddress[1],pAddress[0]);
    if (FAILED(HRes))
    {
        return NULL;
    }
    return szIpAddress;
}
WCHAR *GetHardwareAddress(
    __in DHCP_CLIENT_UID phyAdd
)
{
    HRESULT HRes = S_OK;
    WCHAR *szPhysicalAddress = NULL;
    DWORD Size = 0;
    DWORD i=0, j=0, num0=0, num1=0, num2=0;
    if (NULL != phyAdd.Data && 0 < phyAdd.DataLength)
    {
        // calculating the ecount of the hardware address string.
        // corressponding to each byte in the original string, there would be a hyphen and a '\0' character for the last one.
        HRes = DWordMult(3, phyAdd.DataLength, &Size);
        if (FAILED(HRes))
        {    
            return NULL;
        }
        // calculating the bcount of the hardware address string.
        HRes = DWordMult(Size, sizeof(WCHAR), &Size);
        if (FAILED(HRes))
        {
            return NULL;
        }
        // allocating the memory for the hardware address string. 
        szPhysicalAddress = (WCHAR *)malloc(Size);
        if (NULL == szPhysicalAddress)
        {
            return NULL;
        }
        memset(szPhysicalAddress, 0, Size);
        for (i=0; i<phyAdd.DataLength; i++)
        {
            num0 = phyAdd.Data[i];
            // getting the first hexadecimal character in the byte.
            num1 = num0 / 16;
            // getting the second hexadecimal character in the byte.
            num2 = num0 % 16;
            szPhysicalAddress[j++] = "0123456789ABCDEF"[num1];
            szPhysicalAddress[j++] = "0123456789ABCDEF"[num2];
            if (i < phyAdd.DataLength - 1)
            { 
                // adding the separator hyphen.
                szPhysicalAddress[j++] = L'-';
            }
        }
    }
    // adding the terminating null character.
    szPhysicalAddress[j] = L'\0';
    return szPhysicalAddress;
}
WCHAR *GetDateTimeString(
    __in FILETIME ftTime
)
{
    int cchFormat = 0, cchData = 0;
    DWORD error = NO_ERROR;
    HRESULT HRes = S_OK;
    FILETIME ftLocalTime = {0};
    SYSTEMTIME stTime = {0};
    LPWSTR pwszFormat = NULL;
    LPWSTR pwszDatetimestring = NULL;
    pwszDatetimestring = (LPWSTR)malloc(MAX_DATE_TIME_BUF_SIZE * sizeof(WCHAR));
    if (NULL == pwszDatetimestring)
    {
        return NULL;
    }
    // convert file time based on UTC to a local file time.
    if (!FileTimeToLocalFileTime(&ftTime, &ftLocalTime))
    {
        error = GetLastError();
        return NULL;
    }
    // convert a 64bit file time to system time format.
    if (!FileTimeToSystemTime(&ftLocalTime, &stTime))
    {
        error = GetLastError();
        return NULL;
    }
    // get the string length for the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_SSHORTDATE,
                              NULL,
                              0);
    if (0 == cchFormat)
    {
        error = GetLastError();
        return NULL;
    }
    // allocate buffer for locale info string using cchFormat(buffer length required) from above.
    pwszFormat = (LPWSTR)malloc(cchFormat * sizeof(WCHAR));
    if (NULL == pwszFormat)
    {
        error = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }
    // get the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_SSHORTDATE,
                              pwszFormat,
                              cchFormat);
    if (0 == cchFormat)
    {
        error = GetLastError();
        return NULL;
    }
    // format the date as a date string for the specified(retieved) locale.
    cchData = GetDateFormat(LOCALE_USER_DEFAULT,
                            0,
                            &stTime,
                            pwszFormat,
                            pwszDatetimestring,
                            MAX_DATE_TIME_BUF_SIZE);
    if (0 == cchData)
    {
        error = GetLastError();
        return NULL;
    }
    // add the space between date and time.
    HRes = StringCchCatW(pwszDatetimestring,(MAX_DATE_TIME_BUF_SIZE-cchData),L" ");
    if (FAILED(HRes))
    {
        return NULL;
    }
    // get the string length for the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_STIMEFORMAT,
                              NULL,
                              0);
    if (0 == cchFormat)
    {
        error = GetLastError();
        return NULL;
    }
    // allocate buffer for locale info string using cchFormat(buffer length required) from above.
    pwszFormat = (LPWSTR)malloc(cchFormat * sizeof(WCHAR));
    if (NULL == pwszFormat)
    {
        error = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }
    // get the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_STIMEFORMAT,
                              pwszFormat,
                              cchFormat);
    if (0 == cchFormat)
    {
        error = GetLastError();
        return NULL;
    }
    // format time as a time string for the specified locale.
    cchData = GetTimeFormat(LOCALE_USER_DEFAULT,
                            0,
                            &stTime,
                            pwszFormat,
                            pwszDatetimestring + cchData,
                            MAX_DATE_TIME_BUF_SIZE-cchData-1);
    if (0 == cchData)
    {
        error = GetLastError();
        return NULL;
    }
    // return formatted date and time string.
    return pwszDatetimestring;
}
