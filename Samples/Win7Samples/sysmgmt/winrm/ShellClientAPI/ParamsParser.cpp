/******************************************************************************
 * <copyright file="ParamsParser.cpp" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#include "ParamsParser.h"

// Constructor.
CParamsParser::CParamsParser()
{
	this->connection = NULL;
	this->authentication = NULL;
	this->username = NULL;
	this->password = NULL;
	this->resourceUri = NULL;
	this->commandLine = NULL;
	this->sendData = NULL;
	this->countStr = NULL;

    authenticationMechanism = WSMAN_FLAG_AUTH_KERBEROS;
    count = 1;
}

// Destructor.
CParamsParser::~CParamsParser()
{
	delete [] this->connection;
	delete [] this->authentication;
	delete [] this->username;
	delete [] this->password;
	delete [] this->resourceUri;
	delete [] this->commandLine;
	delete [] this->sendData;
	delete [] this->countStr;
}

bool CParamsParser::ParseCommandLine(int argc, __in_ecount(argc) wchar_t * argv[])
{
	// parse commandline args
	for (int i = 1; i < argc; i++)
	{
		PWSTR argName = new wchar_t[wcslen(argv[i])+1];
		if (NULL == argName)
		{
			wprintf(L"Could not allocate argName\n");
			return false;
		}

		ZeroMemory(argName, (wcslen(argv[i])+1)*sizeof(wchar_t));
		StringCchCopy(argName, wcslen(argv[i])+1, argv[i]);

		PWSTR argValue = NULL;
		if (argValue = wcsstr(argName, L":"))
		{
			// put terminator where colon is
			*argValue = L'\0';
			argValue++;	// move past colon 
		}
		if (!argValue)
		{
			wprintf(L"Value missing for switch: %s\n", argName);
			PrintUsage(argv[0]);
            delete [] argName;
			return 1;
		}
		
		if (0 == _wcsicmp(argName,L"/remote") ||
            0 == _wcsicmp(argName,L"-remote"))
		{
			if (NULL == this->connection)
			{
				this->connection = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->connection)
				{
					wprintf(L"Could not allocate connection\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->connection, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->connection, wcslen(argValue)+1, argValue);
			}
			else
			{
				wprintf(L"Error: more than one remote specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/authentication") ||
                 0 == _wcsicmp(argName,L"-authentication"))
		{
			if (NULL == this->authentication)
			{
				this->authentication = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->authentication)
				{
					wprintf(L"Could not allocate authentication\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->authentication, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->authentication, wcslen(argValue)+1, argValue);
                if (0 == _wcsicmp(this->authentication,L"Negotiate"))
                {
                    authenticationMechanism = WSMAN_FLAG_AUTH_NEGOTIATE;
                }
                else if (0 == _wcsicmp(this->authentication,L"Basic"))
                {
                    authenticationMechanism = WSMAN_FLAG_AUTH_BASIC;
                }
                else if (0 == _wcsicmp(this->authentication,L"Kerberos"))
                {
                    authenticationMechanism = WSMAN_FLAG_AUTH_KERBEROS;
                }
                else
                {
                    // you can add more, here only allows the above for short demo
				    wprintf(L"Error: unsupported authentication mechanism specified\n");
				    CParamsParser::PrintUsage(argv[0]);
                    delete [] argName;
				    return false;
                }
			}
			else
			{
				wprintf(L"Error: more than one authentication specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/username") ||
                 0 == _wcsicmp(argName,L"-username"))
		{
			if (NULL == this->username)
			{
				this->username = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->username)
				{
					wprintf(L"Could not allocate username\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->username, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->username, wcslen(argValue)+1, argValue);
			}
			else
			{
				wprintf(L"Error: more than one username specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/password") ||
                 0 == _wcsicmp(argName,L"-password"))
		{
			if (NULL == this->password)
			{
				this->password = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->password)
				{
					wprintf(L"Could not allocate password\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->password, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->password, wcslen(argValue)+1, argValue);
			}
			else
			{
				wprintf(L"Error: more than one password specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/resourceUri") ||
                 0 == _wcsicmp(argName,L"-resourceUri"))
		{
			if (NULL == this->resourceUri)
			{
				this->resourceUri = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->resourceUri)
				{
					wprintf(L"Could not allocate resourceUri\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->resourceUri, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->resourceUri, wcslen(argValue)+1, argValue);
			}
			else
			{
				wprintf(L"Error: more than one resourceUri specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/commandLine") ||
                 0 == _wcsicmp(argName,L"-commandLine"))
		{
			if (NULL == this->commandLine)
			{
				this->commandLine = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->commandLine)
				{
					wprintf(L"Could not allocate commandLine\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->commandLine, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->commandLine, wcslen(argValue)+1, argValue);
			}
			else
			{
				wprintf(L"Error: more than one commandLine specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else if (0 == _wcsicmp(argName,L"/sendData") ||
                 0 == _wcsicmp(argName,L"-sendData"))
		{
			if (NULL == this->sendData)
			{
				this->sendData = new char[wcslen(argValue)+1];
				if (NULL == this->sendData)
				{
					wprintf(L"Could not allocate sendData\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->sendData, (wcslen(argValue)+1)*sizeof(char));
				
				if (!WideCharToMultiByte(
					CP_ACP,
					WC_NO_BEST_FIT_CHARS,
					argValue,
					-1,
					sendData,
					wcslen(argValue)+1,
					NULL,
					NULL))
				{
					wprintf(L"Failed to convert %s to ansi\n", argValue);
                    delete [] argName;
					return false;
				}
			}
			else
			{
				wprintf(L"Error: more than one sendData specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}		
		}
		else if (0 == _wcsicmp(argName,L"/count") ||
                 0 == _wcsicmp(argName,L"-count"))
		{
			if (NULL == this->countStr)
			{
				this->countStr = new wchar_t[wcslen(argValue)+1];
				if (NULL == this->countStr)
				{
					wprintf(L"Could not allocate countStr\n");
                    delete [] argName;
					return false;
				}
				ZeroMemory(this->countStr, (wcslen(argValue)+1)*sizeof(wchar_t));
				StringCchCopy(this->countStr, wcslen(argValue)+1, argValue);

                count = _wtoi(this->countStr);
                if (count < 1)
                {
					wprintf(L"count should not be less than 1\n");
                    delete [] argName;
					return false;
                }
			}
			else
			{
				wprintf(L"Error: more than one count specified\n");
				CParamsParser::PrintUsage(argv[0]);
                delete [] argName;
				return false;
			}
		}
		else
		{
			wprintf(L"Unknown switch: %s\n", argv[i]);
			CParamsParser::PrintUsage(argv[0]);
            delete [] argName;
			return false;
		}

        delete [] argName;
	}

	if (!this->connection || !this->authentication || 
        !this->username || !this->password || !this->resourceUri ||
        !this->commandLine || !this->sendData || !this->countStr)
	{
		printf("Error: missing required parameter\n");
		CParamsParser::PrintUsage(argv[0]);
		return false;
	}

	return true;
}

void CParamsParser::PrintUsage(PCWSTR program)
{
    wprintf(L"Usage: %s /<SWITCH>:<VALUE> ... \n\n", program);
	wprintf(L"Valid switches: \n");
	wprintf(L"\tremote\\ttSpecifies identifier of remote system.\n");
	wprintf(L"\tauthentication\tSpecifies authentication basic, negotiate, kerberos.\n");
	wprintf(L"\tusername\tSpecifies username on remote machine.\n");
    wprintf(L"\tpassword\tSpecifies password.\n");
	wprintf(L"\tresourceUri\tspecify management resources to be used for operations.\n");
	wprintf(L"\tcommandLine\tSpecifies command string.\n");
    wprintf(L"\tsendData\tSpecifies data to be sent.\n");
    wprintf(L"\tcount\\ttSpecifies how many times data will be sent.\n\n");	
    wprintf(L"For example: \n\n");
    wprintf(L"1) execute 'cmd dir': \n");
    wprintf(L"\t%s -remote:http://fqdn.domain.com "
            L"-authentication:negotiate -username:domain\\username -password:dummyPassword "
            L"-resourceUri:http://schemas.microsoft.com/wbem/wsman/1/windows/shell/cmd "
            L"-commandLine:dir -sendData: /count:1 \n\n", program);
    wprintf(L"2) receive back whatever user has sent: \n");
    wprintf(L"\t%s -remote:https://fqdn.domain.com:444/WSManSDKSamples "
            L"-authentication:negotiate -username:domain\\username -password:dummyPassword "
            L"-resourceUri:http://microsoft.wsman.sdksample/shellresource "
            L"-commandLine:dir -sendData:dummy /count:3 \n\n", program);
    wprintf(L"3) operation failure 'exceeding the authz quota' due to many send operations: \n");
    wprintf(L"\t%s -remote:https://fqdn.domain.com:444/WSManSDKSamples "
            L"-authentication:negotiate -username:domain\\username -password:dummyPassword "
            L"-resourceUri:http://microsoft.wsman.sdksample/shellresource "
            L"-commandLine:dir -sendData:dummy /count:10 \n", program);
}