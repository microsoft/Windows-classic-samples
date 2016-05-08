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
 * Dump IPv6 leases across all the scopes configured on a DHCP server.
 *
 */
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <intsafe.h>
#include <strsafe.h>
#include <winsock.h>
#include <dhcpsapi.h>

#define SERVERNAME_BUF_SIZE 255
#define IPADDRV6_BUF_SIZE 128

#define MAX_DATE_TIME_BUF_SIZE 64

#define DHCP_DATE_TIME_INFINITE_LOW 0xFFFFFFFF
#define DHCP_DATE_TIME_INFINITE_HIGH 0x7FFFFFFF
#define DHCP_DATE_TIME_ZERO_LOW 0
#define DHCP_DATE_TIME_ZERO_HIGH 0

WCHAR *ConvertIpV6AddtoWstr(
    __in DHCP_IPV6_ADDRESS addr
);
WCHAR *GetHardwareAddress(
    __in DHCP_CLIENT_UID phyAdd
);
WCHAR *GetDateTimeString(
    __in FILETIME ftTime
);

DWORD __cdecl main(int argc, char* argv[])
{
    DHCP_RESUME_HANDLE ResumeHandle = 0;
	LPDHCPV6_IP_ARRAY EnumScopesV6 = NULL;
	DWORD nReadV6 = 0, nTotalV6 = 0;
	DHCP_RESUME_IPV6_HANDLE ResumeHandleV6 = {0,0};
	LPDHCP_CLIENT_INFO_ARRAY_V6 ClientsV6 = NULL;
    DWORD  nClientsReadV6 = 0, nClientsTotalV6 = 0;
	DWORD error1 = ERROR_SUCCESS, error2 = ERROR_SUCCESS;
    WCHAR *szDateTimeStr = NULL, *szDuid = NULL, *szClientIp = NULL, *szScopeIp = NULL;

    WCHAR szServer[SERVERNAME_BUF_SIZE] = {0};
    if (2 != argc)
    {
        wprintf(L"Usage: DhcpServerShowLeasesV6.exe <Server IpAdd/Name>");
        return ERROR_INVALID_PARAMETER;
    }
    MultiByteToWideChar(0, 0, argv[1], (int)strlen(argv[1]), szServer, SERVERNAME_BUF_SIZE);      
    do {
        // enumerate all the IpV6 scopes on the server.
        error1 = DhcpEnumSubnetsV6(szServer, &ResumeHandle, (DWORD)~0, &EnumScopesV6, &nReadV6, &nTotalV6);
        if (0 == nTotalV6)
        {
            wprintf(L"No scopes on this server.\n");
        }
        if (ERROR_NO_MORE_ITEMS == error1) // there are no IpV6 scopes
        {
            break;
        }
	    if (ERROR_SUCCESS != error1 && ERROR_MORE_DATA != error1)
	    {
            wprintf(L"DhcpServerShowLeasesV6 returned with error: %d\n",error1); 
            return error1;
	    }
        // iterating over all the scopes one by one to get the clients for each scope.
	    for (unsigned int count=0; count < EnumScopesV6->NumElements; count++)
	    {
            // converting scope IP address from DHCP_IPV6_ADDRESS to LPWSTR
            szScopeIp = ConvertIpV6AddtoWstr(EnumScopesV6->Elements[count]);
            wprintf(L"\nScope : %s\n\n", szScopeIp ? szScopeIp : L" ");
 		    do {
                // enumerating the clients on a specific IpV6 scope
                //the leases shown include all the leases present and can be filtered out on the basic of Address State attribute of client.
		        error2 = DhcpEnumSubnetClientsV6(szServer, EnumScopesV6->Elements[count], &ResumeHandleV6, (DWORD)~0, &ClientsV6, &nClientsReadV6, &nClientsTotalV6);
                if (0 == nClientsTotalV6)
                {
                    wprintf(L"No clients on this scope.\n");
                }
                if (ERROR_NO_MORE_ITEMS == error2)
                {
                    break;
                }
		        if (ERROR_SUCCESS != error2 && ERROR_MORE_DATA != error2)
		        {
                    wprintf(L"DhcpServerShowLeasesV6 returned with error: %d\n",error2); 
                    return error2;
		        }
                //iterating over all the clients on a speicified scope.
                for (unsigned int count = 0; count < ClientsV6->NumElements; count++)
  	            {
                    //converting client Ip from DHCP_IPV6_ADDRESS to LPWSTR
                    szClientIp = ConvertIpV6AddtoWstr(ClientsV6->Clients[count]->ClientIpAddress);
                    wprintf(L"ClientAddress   : %s\n",szClientIp ? szClientIp : L" ");
                    //converting client DUID from byte * to LPWSTR
                    szDuid = GetHardwareAddress(ClientsV6->Clients[count]->ClientDUID);
                    wprintf(L"DUID            : %s\n",szDuid ? szDuid : L" ");
                    wprintf(L"IAID            : %d\n",ClientsV6->Clients[count]->IAID);
                    // if lease duration is infinite then lease never expires. 
                    if (DHCP_DATE_TIME_INFINITE_LOW == ClientsV6->Clients[count]->ClientValidLeaseExpires.dwLowDateTime && 
                        DHCP_DATE_TIME_INFINITE_HIGH == ClientsV6->Clients[count]->ClientValidLeaseExpires.dwHighDateTime)
                    {
                        wprintf(L"Lease Expires   : Never\n\n");
                    }
                    // if lease duration is 0 the client is inactive.
                    else if (DHCP_DATE_TIME_ZERO_LOW == ClientsV6->Clients[count]->ClientValidLeaseExpires.dwLowDateTime && 
                             DHCP_DATE_TIME_ZERO_HIGH == ClientsV6->Clients[count]->ClientValidLeaseExpires.dwHighDateTime)
                    {
                        wprintf(L"Lease Expires   : Inactive\n\n");
                    }
                    else
                    {
                        // converting the date time information from FILETIME * to LPWSTR.
                        szDateTimeStr = GetDateTimeString(*(FILETIME *)(&ClientsV6->Clients[count]->ClientValidLeaseExpires));
                        wprintf(L"Lease Expires   : %s\n\n", szDateTimeStr ? szDateTimeStr : L" ");
                    }
                    if (NULL != szClientIp)
                    {
                        free(szClientIp);
                        szClientIp = NULL;
                    }
                    if (NULL != szDuid)
                    {
                        free(szDuid);
                        szDuid = NULL;
                    }
                    if (NULL != szDateTimeStr)
                    {
                        free(szDateTimeStr);
                        szDateTimeStr = NULL;
                    }
	            }
                if (NULL != ClientsV6)
                {
                    DhcpRpcFreeMemory(ClientsV6);
                    ClientsV6 = NULL;
                }
                nClientsReadV6 = 0;
		        nClientsTotalV6 = 0;
            } while (ERROR_MORE_DATA == error2);
            if (NULL != szScopeIp)
            {
                free(szScopeIp);
                szScopeIp = NULL;
            }
	    }
        nReadV6 = 0;
        nTotalV6 = 0;
        if (NULL != EnumScopesV6)
        {
            DhcpRpcFreeMemory(EnumScopesV6);    
            EnumScopesV6 = NULL;
        }
    } while (ERROR_MORE_DATA == error1);
	return 0;
}

WCHAR *ConvertIpV6AddtoWstr(
    __in DHCP_IPV6_ADDRESS addr
)
{
    HRESULT Hres = S_OK;
    WCHAR *szIpv6Address = NULL;
    WCHAR szIpv6LowOrderBits[IPADDRV6_BUF_SIZE/2-1];
    unsigned short octet[4];
    BYTE *IpByte = NULL;
    IpByte = (BYTE *)malloc(16);
    if (NULL == IpByte)
    {
        return NULL;
    }
    // converting high and low order bits into a byte array.
    for (unsigned int i=0;i<8;i++)
	{
		IpByte[i]=(BYTE)(addr.HighOrderBits>>((7-i)*8));
	}
	for (unsigned int i=0;i<8;i++)
	{
		IpByte[8+i]=(BYTE)(addr.LowOrderBits>>((7-i)*8));
    }
    szIpv6Address = (WCHAR *)malloc(IPADDRV6_BUF_SIZE);
    if (NULL == szIpv6Address)
    {
        return NULL;
    }
    memset(szIpv6Address, 0, IPADDRV6_BUF_SIZE);
    // converting high order bytes into haxadecimal string form. 
    for (unsigned int i=0; i<8; i=i+2)
    {
        octet[i/2] = htons(*((unsigned short *)IpByte));
        IpByte = IpByte+2;
    }
    if (0 == octet[1] && 0 == octet[2] && 0 == octet[3])
    {
        Hres = StringCchPrintfW(szIpv6Address,IPADDRV6_BUF_SIZE,L"%x::",octet[0]);    
        if (FAILED(Hres))
            return NULL;
    }
    else if (0 == octet[2] && 0 == octet[3])
    {
        Hres = StringCchPrintfW(szIpv6Address,IPADDRV6_BUF_SIZE,L"%x:%x::",octet[0],octet[1]);
        if (FAILED(Hres))
            return NULL;
    }
    else if(0 == octet[3])
    {
        Hres = StringCchPrintfW(szIpv6Address,IPADDRV6_BUF_SIZE,L"%x:%x:%x::",octet[0],octet[1],octet[2]);
        if (FAILED(Hres))
            return NULL;
    }
    else
    {
        Hres = StringCchPrintfW(szIpv6Address,IPADDRV6_BUF_SIZE,L"%x:%x:%x:%x:",octet[0],octet[1],octet[2],octet[3]);
        if (FAILED(Hres))
            return NULL;
    }
    // converting high order bytes into haxadecimal string form.
    for (unsigned int i=0; i<8; i=i+2)
    {
        octet[i/2] = htons(*((unsigned short *)IpByte));
        IpByte = IpByte+2;
    }
    if (0 == octet[0] && 0 == octet[1] && 0 == octet[2])
    {
        Hres = StringCchPrintfW(szIpv6LowOrderBits,IPADDRV6_BUF_SIZE/2-1,L"%x",octet[3]);    
        if (FAILED(Hres))
            return NULL;
    }
    else if (0 == octet[0] && 0 == octet[1])
    {
        Hres = StringCchPrintfW(szIpv6LowOrderBits,IPADDRV6_BUF_SIZE/2-1,L"%x:%x",octet[2],octet[3]);
        if (FAILED(Hres))
            return NULL;
    }
    else if (0 == octet[0])
    {   
        Hres = StringCchPrintfW(szIpv6LowOrderBits,IPADDRV6_BUF_SIZE/2-1,L"%x:%x:%x",octet[1],octet[2],octet[3]);
        if (FAILED(Hres))
            return NULL;
    }
    else
    { 
        Hres = StringCchPrintfW(szIpv6LowOrderBits,IPADDRV6_BUF_SIZE/2-1,L"%x:%x:%x:%x",octet[0],octet[1],octet[2],octet[3]);
        if (FAILED(Hres))
            return NULL;
    }
    // combining the two string for high and low order bytes.
    Hres = StringCchCatW(szIpv6Address,IPADDRV6_BUF_SIZE,szIpv6LowOrderBits);
    if (FAILED(Hres))
        return NULL;
    return szIpv6Address;
}
WCHAR *GetHardwareAddress(
    __in DHCP_CLIENT_UID phyAdd
)
{
    HRESULT HRes = S_OK;
    WCHAR *szPhysicalAddress = NULL;
    DWORD Size = 0;
    DWORD i=0, j=0, num0=0,num1=0,num2=0;

    if(phyAdd.Data != NULL && phyAdd.DataLength > 0)
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
            szPhysicalAddress[j++] = "0123456789ABCDEF"[num1];
            // getting the second hexadecimal character in the byte.
            num2 = num0 % 16;
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
    HRESULT HRes = S_OK;
    FILETIME ftLocalTime = {0};
    SYSTEMTIME stTime = {0};
    LPWSTR pwszFormat = NULL;
    LPWSTR pwszDatetimestring = NULL;
    pwszDatetimestring = (LPWSTR)malloc(MAX_DATE_TIME_BUF_SIZE * sizeof(WCHAR));

    // convert file time based on UTC to a local file time.
    if (!FileTimeToLocalFileTime(&ftTime, &ftLocalTime))
    {
        return NULL;
    }
    // convert a 64bit file time to system time format.
    if (!FileTimeToSystemTime(&ftLocalTime, &stTime))
    {
        return NULL;
    }
    // get the string length for the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_SSHORTDATE,
                              NULL,
                              0);
    if (0 == cchFormat)
    {
        return NULL;
    }
    // allocate buffer for locale info string using cchFormat(buffer length required) from above.
    pwszFormat = (LPWSTR)malloc(cchFormat * sizeof(WCHAR));
    if (NULL == pwszFormat)
    {
        return NULL;
    }
    // get the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_SSHORTDATE,
                              pwszFormat,
                              cchFormat);
    if (0 == cchFormat)
    {
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
        return NULL;
    }
    // allocate buffer for locale info string using cchFormat(buffer length required) from above.
    pwszFormat = (LPWSTR)malloc(cchFormat * sizeof(WCHAR));
    if (NULL == pwszFormat)
    {
        return NULL;
    }
    // get the locale info.
    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              LOCALE_STIMEFORMAT,
                              pwszFormat,
                              cchFormat);
    if (0 == cchFormat)
    {
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
        return NULL;
    }
    // return formatted date and time string.
    return pwszDatetimestring;
}
