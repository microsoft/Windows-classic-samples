#include <stdio.h>
#include <windows.h>
#include <msxml2.h>
#include <comdef.h>
#include <stdlib.h>
#include "wsman.h"

int main()
{
	HRESULT hr = E_FAIL;
	IXMLDOMDocument2 * pXmlDoc = NULL;
	IXMLDOMNode * pXmlNode = NULL;
	BSTR newXml = NULL;
	BSTR currentThreshold = NULL;
	const LPWSTR resourceUri = L"http://schemas.microsoft.com/wbem/wsman/1/wmi/root/hardware/NumericSensor?creationclassname=numericsensor+systemname=ipmi controller 32+deviceid=12.0.32+systemcreationclassname=computersystem";

	VARIANT var;
	VariantInit(&var);

	hr = CoInitialize(0);
	if (FAILED(hr))
	{
		printf("ERROR: CoInit failed: %x\n", hr);
		return hr;
	}

	Wsman wsman;
	LPWSTR xml = NULL;
	try
	{
		wsman.CreateSession();	// optionally pass in connection string
		xml = wsman.Get(resourceUri);
		wprintf(L"Get: %s\n", xml);
	}
	catch (HRESULT hrException)
	{
		return hrException;
	}

	try
	{
		// can use msxml to replace value for put
		hr = CoCreateInstance(CLSID_DOMDocument2, 0, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (LPVOID*)&pXmlDoc);
		if (FAILED(hr))
		{
			printf("ERROR: CoCreate(IXMLDomDoc) failed: %x\n", hr);
			throw hr;
		}

		hr = pXmlDoc->put_async(VARIANT_FALSE);
		if (FAILED(hr))
		{
			printf("ERROR: put_async failed: %x\n", hr);
			throw hr;
		}

		VARIANT_BOOL isSuccessful = VARIANT_FALSE;
		hr = pXmlDoc->loadXML(xml, &isSuccessful);
		if (FAILED(hr) || !isSuccessful)
		{
			printf("ERROR: LoadXML failed: %x\n", hr);
			throw hr;
		}

		var.vt = VT_BSTR;
		var.bstrVal = SysAllocString(L"XPath");
		hr = pXmlDoc->setProperty(_bstr_t(L"SelectionLanguage"), var);
		if (FAILED(hr))
		{
			printf("ERROR: setProperty(selectionLanguage) failed: %x\n", hr);
			throw hr;
		}
		VariantClear(&var);

		var.vt = VT_BSTR;
		var.bstrVal = SysAllocString(L"xmlns:w=\"http://schemas.microsoft.com/wbem/wsman/1/wmi/root/hardware/NumericSensor\"");
		hr = pXmlDoc->setProperty(_bstr_t(L"SelectionNamespaces"), var);
		if (FAILED(hr))
		{
			printf("ERROR: setProperty(selectionNamespaces) failed: %x\n", hr);
			throw hr;
		}

		hr = pXmlDoc->selectSingleNode(_bstr_t(L"/w:NumericSensor/w:LowerThresholdNonCritical"), &pXmlNode);
		if (FAILED(hr))
		{
			printf("ERROR: selectSingleNode failed: %x\n", hr);
			throw hr;
		}

		if (NULL == pXmlNode)
		{
			printf("ERROR: did not find LowerThresholdNonCritical element\n");
			throw E_INVALIDARG;
		}

		hr = pXmlNode->get_text(&currentThreshold);
		if (FAILED(hr))
		{
			printf("ERROR: get_text failed: %x\n", hr);
			throw hr;
		}

		long threshold = _wtol(currentThreshold);
		threshold += 100;
		SysFreeString(currentThreshold);

		const size_t MAX_LONG_STRING_LEN = 33;
		wchar_t thresholdString[MAX_LONG_STRING_LEN] = {0};
		errno_t err = 0;
		if (err = _ltow_s(threshold, thresholdString, MAX_LONG_STRING_LEN, 10))
		{
			printf("ERROR: _ltow_s failed: %d\n", err);
			throw hr;
		}
		hr = pXmlNode->put_text(_bstr_t(thresholdString));
		if (FAILED(hr))
		{
			printf("ERROR: put_text failed: %x\n", hr);
			throw hr;
		}

		hr = pXmlDoc->get_xml(&newXml);
		if (FAILED(hr))
		{
			printf("ERROR: get_xml failed: %x\n", hr);
			throw hr;
		}

		wprintf(L"Put: %s\n", wsman.Put(resourceUri, newXml));
	}
	catch(HRESULT hrException)
	{
		hr = hrException;
	}

	CoUninitialize();
	VariantClear(&var);
	if (pXmlDoc)
	{
		pXmlDoc->Release();
	}
	if (pXmlNode)
	{
		pXmlNode->Release();
	}
	if (newXml)
	{
		SysFreeString(newXml);
	}
	if (currentThreshold)
	{
		SysFreeString(currentThreshold);
	}

	printf("Done\n");
	return FAILED(hr) ? hr : 0;
}