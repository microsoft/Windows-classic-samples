#include "Subscription.h"

std::wstring ConvertEcConfigurationMode( DWORD code )
{

    if ( EcConfigurationModeCustom == (EC_SUBSCRIPTION_CONFIGURATION_MODE) code )
        return L"Custom";
    else if ( EcConfigurationModeMinLatency == (EC_SUBSCRIPTION_CONFIGURATION_MODE) code )
        return L"MaxLatency";
    else if ( EcConfigurationModeMinBandwidth == (EC_SUBSCRIPTION_CONFIGURATION_MODE) code )
        return L"MinBandwidth";
    else if ( EcConfigurationModeNormal == (EC_SUBSCRIPTION_CONFIGURATION_MODE) code )
        return L"Normal";
    else
        return L"Unknown";
}

std::wstring ConvertEcDeliveryMode( DWORD code )
{
    if ( EcDeliveryModePull == (EC_SUBSCRIPTION_DELIVERY_MODE) code )
        return L"Pull";
    else if ( EcDeliveryModePush == (EC_SUBSCRIPTION_DELIVERY_MODE) code )
        return L"Push";
    else
        return L"Unknown";
}

std::wstring ConvertEcContentFormat( DWORD code )
{
    if ( EcContentFormatEvents == code )
        return L"FormatEvents";
    else if ( EcContentFormatRenderedText == code )
        return L"RenderedText";
    else
        return L"Unknown";
}

std::wstring ConvertEcCredentialsType( DWORD code )
{
    if ( EcSubscriptionCredDefault == code )
        return L"Default";
    else if ( EcSubscriptionCredNegotiate == code )
        return L"Negotiate";
    else if ( EcSubscriptionCredDigest == code )
        return L"Digest";
    else if ( EcSubscriptionCredBasic == code )
        return L"Basic";
    else
        return L"Unknown";
}


std::wstring ConvertEcDateTime( ULONGLONG code )
{
    FILETIME ft;
    SYSTEMTIME utcTime;
    SYSTEMTIME localTime; 
    std::wstring timeString;
    std::vector<WCHAR> buffer(30);

    timeString = L"Error- Failed to Convert Date Time to String";

    ft.dwHighDateTime = (DWORD)((code >> 32) & 0xFFFFFFFF);
    ft.dwLowDateTime = (DWORD)(code & 0xFFFFFFFF);

    if( !FileTimeToSystemTime( &ft, &utcTime) )
    {
        return timeString;
    }

    if(!SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &localTime))
    {
		return timeString;
    }

    HRESULT hr = StringCchPrintfW( (LPWSTR) &buffer[0], 
                  buffer.size(), 
                  L"%4.4hd-%2.2hd-%2.2hdT%2.2hd:%2.2hd:%2.2hd.%3.3hdZ",
                  localTime.wYear,
                  localTime.wMonth,
                  localTime.wDay,
                  localTime.wHour,
                  localTime.wMinute,
                  localTime.wSecond,
                  localTime.wMilliseconds );

    if(FAILED(hr)) 
    {
        return timeString;
    }

    timeString = (LPWSTR) &buffer[0];

    return timeString;
}

