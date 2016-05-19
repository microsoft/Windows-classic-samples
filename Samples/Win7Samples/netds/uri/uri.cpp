#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <urlmon.h>

const WCHAR * GetPropertyName(Uri_PROPERTY property);
void DisplayHelp(WCHAR *pwzApplicationName);
void DisplayIUri(IUri *pIUri);

int wmain(int argc, WCHAR* argv[])
{
	IUri * pBaseIUri = NULL;
	IUri * pRelativeIUri = NULL;
	IUri * pCombinedIUri = NULL;

	if (2 <= argc)
	{
		HRESULT hr = CreateUri(argv[1], Uri_CREATE_ALLOW_RELATIVE, 0, &pBaseIUri);

		if (SUCCEEDED(hr))
		{
			if (2 == argc)
			{
				wprintf(L"The result of creating \"%s\": \n", argv[1]);
				DisplayIUri(pBaseIUri);
			}
			else
			{
				hr = CreateUri(argv[2], Uri_CREATE_ALLOW_RELATIVE, 0, &pRelativeIUri);

				if (SUCCEEDED(hr))
				{
					hr = CoInternetCombineIUri(pBaseIUri, pRelativeIUri, 0, &pCombinedIUri, 0);

					if (SUCCEEDED(hr))
					{
						wprintf(L"The result of combining \"%s\" and \"%s\": \n", argv[1], argv[2]);
						DisplayIUri(pCombinedIUri);
					}
					else
					{
						wprintf(L"CoInternetCombineIUri failed with %X\n", hr);
					}
				}
				else
				{
					wprintf(L"CreateUri of Relative URI failed with %X\n", hr);
				}
			}
		}
		else
		{
			wprintf(L"CreateUri of Base URI failed with %X\n", hr);
		}
	}
	else
	{
		DisplayHelp(argc > 0 ? argv[0] : NULL);
	}

	if (NULL != pBaseIUri)
	{
		pBaseIUri->Release();
		pBaseIUri = NULL;
	}

	if (NULL != pRelativeIUri)
	{
		pRelativeIUri->Release();
		pRelativeIUri = NULL;
	}

	if (NULL != pCombinedIUri)
	{
		pCombinedIUri->Release();
		pCombinedIUri = NULL;
	}

	return 0;
}

void DisplayIUri(IUri *pUri)
{
	HRESULT hr = S_OK;
    for (DWORD dwProperty = Uri_PROPERTY_STRING_START; dwProperty <= Uri_PROPERTY_STRING_LAST; ++dwProperty)
    {
        BSTR bstrProperty = NULL;
        hr = pUri->GetPropertyBSTR((Uri_PROPERTY)dwProperty, &bstrProperty, 0);

        if (S_OK == hr)
        {
            wprintf(L"\t%-27s == \"%s\"\n", GetPropertyName((Uri_PROPERTY)dwProperty), bstrProperty);
        }
        else if (S_FALSE == hr)
        {
            wprintf(L"\t%-27s not set\n", GetPropertyName((Uri_PROPERTY)dwProperty));
        }
        else
        {
            wprintf(L"\t%-27s GetPropertyBSTR failed with %X\n", GetPropertyName((Uri_PROPERTY)dwProperty), hr);
        }

        SysFreeString(bstrProperty);
    }

    for (DWORD dwProperty = Uri_PROPERTY_DWORD_START; dwProperty <= Uri_PROPERTY_DWORD_LAST; ++dwProperty)
    {
        DWORD dwValue;
        hr = pUri->GetPropertyDWORD((Uri_PROPERTY)dwProperty, &dwValue, 0);

        if (S_OK == hr)
        {
            wprintf(L"\t%-27s == %d\n", GetPropertyName((Uri_PROPERTY)dwProperty), dwValue);
        }
        else if (S_FALSE == hr)
        {
            wprintf(L"\t%-27s not set\n", GetPropertyName((Uri_PROPERTY)dwProperty));
        }
        else
        {
            wprintf(L"\t%s GetPropertyDWORD failed with %X\n", GetPropertyName((Uri_PROPERTY)dwProperty), hr);
        }
    }
}

const WCHAR* GetPropertyName(Uri_PROPERTY property)
{
	switch (property)
	{
		case Uri_PROPERTY_ABSOLUTE_URI:		return L"Uri_PROPERTY_ABSOLUTE_URI";
		case Uri_PROPERTY_AUTHORITY:		return L"Uri_PROPERTY_AUTHORITY";
		case Uri_PROPERTY_DISPLAY_URI:		return L"Uri_PROPERTY_DISPLAY_URI";
		case Uri_PROPERTY_DOMAIN:			return L"Uri_PROPERTY_DOMAIN";
		case Uri_PROPERTY_EXTENSION:		return L"Uri_PROPERTY_EXTENSION";
		case Uri_PROPERTY_FRAGMENT:			return L"Uri_PROPERTY_FRAGMENT";
		case Uri_PROPERTY_HOST:				return L"Uri_PROPERTY_HOST";
		case Uri_PROPERTY_PASSWORD:			return L"Uri_PROPERTY_PASSWORD";
		case Uri_PROPERTY_PATH:				return L"Uri_PROPERTY_PATH";
		case Uri_PROPERTY_PATH_AND_QUERY:	return L"Uri_PROPERTY_PATH_AND_QUERY";
		case Uri_PROPERTY_QUERY:			return L"Uri_PROPERTY_QUERY";
		case Uri_PROPERTY_RAW_URI:			return L"Uri_PROPERTY_RAW_URI";
		case Uri_PROPERTY_SCHEME_NAME:		return L"Uri_PROPERTY_SCHEME_NAME";
		case Uri_PROPERTY_USER_INFO:		return L"Uri_PROPERTY_USER_INFO";
		case Uri_PROPERTY_USER_NAME:		return L"Uri_PROPERTY_USER_NAME";
		case Uri_PROPERTY_HOST_TYPE:		return L"Uri_PROPERTY_HOST_TYPE";
		case Uri_PROPERTY_PORT:				return L"Uri_PROPERTY_PORT";
		case Uri_PROPERTY_SCHEME:			return L"Uri_PROPERTY_SCHEME";
		case Uri_PROPERTY_ZONE:				return L"Uri_PROPERTY_ZONE";
		default:							return L"ERROR: Unknown Uri_PROPERTY value.";
	}
}

void DisplayHelp(WCHAR *pwzApplicationName)
{
	wprintf(L"Displays the properties of an IUri object.\n"
		L"\n"
		L"%s uri1 [uri2]\n"
		L"\n"
		L"When one URI is specified the displayed result is the URI.\n"
		L"When two URIs are specified the displayed result is the two URIs combined.\n"
		L"\n",
		pwzApplicationName == NULL ? L"iuri" : pwzApplicationName);
}

