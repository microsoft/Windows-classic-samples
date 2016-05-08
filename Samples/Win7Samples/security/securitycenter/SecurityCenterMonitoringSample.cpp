// SecurityCenterMonitoringSample.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "wscapi.h"

DWORD WINAPI OnSecurityCenterHealthChange(LPVOID lpParameter)
{
    HRESULT hr = S_OK;
    WSC_SECURITY_PROVIDER_HEALTH health = WSC_SECURITY_PROVIDER_HEALTH_GOOD;

    if (SUCCEEDED(hr))
    {
        hr = WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_INTERNET_SETTINGS, &health);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Internet Settings are %s, The Security Center service is %s\n", 
                    (WSC_SECURITY_PROVIDER_HEALTH_GOOD == health)?L"OK":L"Not OK",
                    (S_FALSE == hr)?L"Not Running":L"Running");
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_ALL, &health);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Security Center says the machines security health is %s, The Security Center service is %s\n", 
                    (WSC_SECURITY_PROVIDER_HEALTH_GOOD == health)?L"OK":L"Not OK",
                    (S_FALSE == hr)?L"Not Running":L"Running");
        }
    }
    if (FAILED(hr))
    {
        wprintf(L"Failed to get health status from Security Center: Error: 0x%X\n", hr);
    }
    return 0;
}

// Monitor the health of Internet Zone Settings and all Security Center protection technologies as a whole.
int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE hWscCallbackRegistration = NULL;
    HRESULT hr = S_OK;
    
    if (SUCCEEDED(hr))
    {
        hr = WscRegisterForChanges(NULL, &hWscCallbackRegistration, OnSecurityCenterHealthChange, NULL);
        if (FAILED(hr))
        {
            wprintf(L"Failed to Register for Security Center change notifications: Error 0x%X\n", hr);
        }
    }
 
    if (SUCCEEDED(hr))
    {
        wprintf(L"Monitoring Security Center for health changes.  Press Enter to stop...\n");
        getchar();
    }

    if (SUCCEEDED(hr))
    {
        hr = WscUnRegisterChanges(hWscCallbackRegistration);
        if (FAILED(hr))
        {
            wprintf(L"Failed to UnRegister Security Center change notifications: Error 0x%X\n", hr);
        }
    }

    return 0;
}

