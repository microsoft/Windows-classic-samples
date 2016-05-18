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
 *    Compares scope-level options for V4 scopes with the same scope address
 *    on 2 DHCP servers (e.g. when configured in an 80:20 configuration) and 
 *    highlights the differences.
 */

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <strsafe.h>
#include <dhcpsapi.h>

#define SERVERNAME_BUF_SIZE 255

DHCP_IP_ADDRESS GetIpV4Address(
    __in char *ipAddStr
);

DWORD __cdecl main(int argc, char* argv[])
{
	DWORD error = ERROR_SUCCESS;
    
    WCHAR server1[SERVERNAME_BUF_SIZE]={0}, server2[SERVERNAME_BUF_SIZE]={0}; 
    bool mismatch = FALSE;
    size_t opt1Len = 0, opt2Len = 0;
    HRESULT Hres = S_OK;
	
    DHCP_OPTION_SCOPE_INFO ScopeInfo;
    LPDHCP_ALL_OPTION_VALUES Values1 = NULL, Values2 = NULL;
    LPDHCP_OPTION_VALUE Server2OptionValue = NULL, Server1OptionValue = NULL;
    
    if (4 != argc)
    {
        wprintf(L"Usage: DhcpServerCompareScopesV4.exe <Server1 IpAdd/Name> <Server2 IpAdd/Name> <ScopeIp>");
        return ERROR_INVALID_PARAMETER;
    }
    MultiByteToWideChar(0, 0, argv[1], (int)strlen(argv[1]), server1, SERVERNAME_BUF_SIZE);      
    MultiByteToWideChar(0, 0, argv[2], (int)strlen(argv[2]), server2, SERVERNAME_BUF_SIZE);      
    
    DHCP_IP_ADDRESS scopeAddress = GetIpV4Address(argv[3]);
    
    ScopeInfo.ScopeType = DhcpSubnetOptions;
	ScopeInfo.ScopeInfo.SubnetScopeInfo = scopeAddress;
    // getting all the options configured on server 1 for scope specified.
    error = DhcpGetAllOptionValues(server1,
                                   DHCP_OPT_ENUM_IGNORE_VENDOR,
                                   &ScopeInfo,
                                   &Values1);
    if(ERROR_SUCCESS != error)
	{
		wprintf(L"DhcpServerCompareScopesV4 returned with error: %d\n",error);
		return error;
	}  
    if(NULL != Values1)
    {
        // iterating over all the options and option value arrays.
        for(unsigned int count = 0; count < Values1->NumElements; count++)
	    {
            if( NULL != Values1->Options[count].OptionsArray )
		    {
                // iterating over all the values of a option.
    		    for(unsigned int optcount = 0; optcount < Values1->Options[count].OptionsArray->NumElements; optcount++)
			    {
                    // get the option values configured for the specific option ID on second server.
                    // assuming that the scope is already configured on the secondary server.
                    error = DhcpGetOptionValueV5(server2,
                                                  0,
                                                  Values1->Options[count].OptionsArray->Values[optcount].OptionID,
                                                  NULL,
                                                  Values1->Options[count].VendorName,
                                                  &ScopeInfo,
                                                  &Server2OptionValue);
                    // the option is not configured on the secondary server.
                    if (ERROR_FILE_NOT_FOUND == error)
                    {
                        // there is a mismatch in the scope configurations.
                        wprintf(L"\nMismatch in Option Id %d\n",Values1->Options[count].OptionsArray->Values[optcount].OptionID);
                        wprintf(L"Option Missing in secondary server scope\n");
                    }
                    // if the option is configured on the secondary server.
                    else if (ERROR_SUCCESS != error)
                    {
                        wprintf(L"DhcpServerCompareScopesV4 returned with error : %d\n",error);
		        return error;
                    }
                    else
                    {
                        // if the number of option values on both servers is same. If not then there is a mismatch.
                        if (Values1->Options[count].OptionsArray->Values[optcount].Value.NumElements != Server2OptionValue->Value.NumElements)
                        {
                            wprintf(L"\nMismatch in Option Id %d\n",Values1->Options[count].OptionsArray->Values[optcount].OptionID);
                            wprintf(L"Number of option values configured on both server scopes differ\n");
                        }
                        // if the number of options is same, checking the value of options. 
                        else
                        {
                            mismatch = FALSE;
                            // iterating over all the values of the option configured
                            for (unsigned int i=0; i < Values1->Options[count].OptionsArray->Values[optcount].Value.NumElements; i++)
                            {
                                // comparing on the basis of option type.
                                switch (Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->OptionType)
                                {
                                    case DhcpBinaryDataOption:
                                       if(!((Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.BinaryDataOption.DataLength == Server2OptionValue->Value.Elements->Element.BinaryDataOption.DataLength) &&
                                            (0 == memcmp(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.BinaryDataOption.Data,Server2OptionValue->Value.Elements->Element.BinaryDataOption.Data,Server2OptionValue->Value.Elements->Element.BinaryDataOption.DataLength))))
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpByteOption:
                                        if(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.ByteOption != Server2OptionValue->Value.Elements->Element.ByteOption)
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpDWordOption:
                                        if(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.DWordOption != Server2OptionValue->Value.Elements->Element.DWordOption)
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpDWordDWordOption:
                                        if ((Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.DWordDWordOption.DWord1 != Server2OptionValue->Value.Elements->Element.DWordDWordOption.DWord1 ) ||
                                            (Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.DWordDWordOption.DWord2 != Server2OptionValue->Value.Elements->Element.DWordDWordOption.DWord2 ) )
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpEncapsulatedDataOption:
                                        if (!((Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.EncapsulatedDataOption.DataLength == Server2OptionValue->Value.Elements->Element.EncapsulatedDataOption.DataLength) &&
                                             (0 == memcmp(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.EncapsulatedDataOption.Data,Server2OptionValue->Value.Elements->Element.EncapsulatedDataOption.Data,Server2OptionValue->Value.Elements->Element.EncapsulatedDataOption.DataLength))))
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpIpAddressOption:
                                        if (Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.IpAddressOption != Server2OptionValue->Value.Elements->Element.IpAddressOption)
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpStringDataOption:
                                        Hres = StringCchLengthW(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.StringDataOption,STRSAFE_MAX_CCH, &opt1Len);
                                        if (FAILED(Hres))
                                        {
                                            return HRESULT_CODE(Hres);
                                        }
                                        Hres = StringCchLengthW(Server2OptionValue->Value.Elements->Element.StringDataOption,STRSAFE_MAX_CCH, &opt2Len);
                                        if (FAILED(Hres))
                                        {
                                            return HRESULT_CODE(Hres);
                                        }
                                        if (!((opt1Len == opt2Len)&&(0 == wmemcmp(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.StringDataOption,Server2OptionValue->Value.Elements->Element.StringDataOption,opt1Len))))
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpWordOption:
                                        if (Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.WordOption != Server2OptionValue->Value.Elements->Element.WordOption)
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                    case DhcpIpv6AddressOption:
                                        Hres = StringCchLengthW(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.Ipv6AddressDataOption,STRSAFE_MAX_CCH, &opt1Len);
                                        if (FAILED(Hres))
                                        {
                                            return HRESULT_CODE(Hres);
                                        }
                                        Hres = StringCchLengthW(Server2OptionValue->Value.Elements->Element.Ipv6AddressDataOption,STRSAFE_MAX_CCH, &opt2Len);
                                        if (FAILED(Hres))
                                        {
                                            return HRESULT_CODE(Hres);
                                        }
                                        if (!((opt1Len == opt2Len)&&(0 == wmemcmp(Values1->Options[count].OptionsArray->Values[optcount].Value.Elements->Element.Ipv6AddressDataOption,Server2OptionValue->Value.Elements->Element.Ipv6AddressDataOption,opt1Len))))
                                        {
                                            mismatch = TRUE;
                                        }
                                        break;
                                }
                                // if there is a mismatch
                                if (mismatch)
                                {
                                    wprintf(L"\nMismatch in Option Id : %d\n",Values1->Options[count].OptionsArray->Values[optcount].OptionID);
                                    wprintf(L"Option values on both server scopes differ\n");
                                    break;
                                }
                            } 
                        }
                    }
                    if (NULL != Server2OptionValue)
                    {
                        DhcpRpcFreeMemory(Server2OptionValue);
                        Server2OptionValue = NULL;
                    }
			    }
		    }
	    }
        DhcpRpcFreeMemory(Values1);
        Values1 = NULL;
    }
    // getting all the options configured on the seconday server.
    // this is to check if there are any options that are configured
    // on secondary server and missing from the primary server scope.
    error = DhcpGetAllOptionValues(server2,
                                    1,
                                    &ScopeInfo,
                                    &Values2);
    if(ERROR_SUCCESS != error)
	{
        wprintf(L"DhcpServerCompareScopesV4 returned with error : %d\n",error);
		return error;
	}  
    if(Values2 != NULL)
    {
        // iterating over all the options configured.
        for(unsigned int count = 0; count < Values2->NumElements; count++)
	    {
            if( NULL != Values2->Options[count].OptionsArray )
		    {
                // for each option ID checking if the same is configured on the primary server also.
    		    for(unsigned int optcount = 0; optcount < Values2->Options[count].OptionsArray->NumElements; optcount++)
			    {
                    error = DhcpGetOptionValueV5(server1,
                                                  0,
                                                  Values2->Options[count].OptionsArray->Values[optcount].OptionID,
                                                  NULL,
                                                  Values2->Options[count].VendorName,
                                                  &ScopeInfo,
                                                  &Server1OptionValue );
                    // if not configured then there is a mismatch.
                    if (ERROR_FILE_NOT_FOUND == error)
                    {
                        wprintf(L"\nMismatch in Option Id %d\n",Values2->Options[count].OptionsArray->Values[optcount].OptionID);
                        wprintf(L"Option Missing in primary server scope\n");
                    }
                    else if(ERROR_SUCCESS != error)
                    {
                        wprintf(L"DhcpServerCompareScopesV4 returned with error : %d\n",error);
		        return error;
                    }
                    // if the option is configured on primary server, comparing the values would have been done already above.
                    if (NULL != Server1OptionValue)
                    {
                        DhcpRpcFreeMemory(Server1OptionValue);
                        Server1OptionValue = NULL;
                    }
			    }
		    }
	    }
        DhcpRpcFreeMemory(Values2);
        Values2 = NULL;
    }
    return 0;
}
DHCP_IP_ADDRESS GetIpV4Address(
    __in char *ipAddStr
)
{
    DWORD ipAdd = 0, num = 0;
    size_t Size = 0;
    int count = 0, base[3] = {1,10,100}, j=0, shift[4]={0,8,16,24};
    // find the length of the ip address string.
    
    Size = strlen(ipAddStr);
    for (int i=(int)Size-1; i>=0; i--)
    { 
        //adding each byte to the ipAdd DWORD 
        if (ipAddStr[i] == '.')
        {
            ipAdd = ipAdd + (num << shift[j++]);
            count = 0;
            num = 0;
            i--;
        }
        //creating numeric value for each byte.
        num = num + (ipAddStr[i] - 48) * base[count];   
        count++;
    }
    ipAdd = ipAdd + (num << shift[j]);
    return ipAdd;
}