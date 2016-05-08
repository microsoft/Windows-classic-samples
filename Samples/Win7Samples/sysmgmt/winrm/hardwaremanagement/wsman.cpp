#include <stdio.h>
#include <windows.h>
#include "wsman.h"

Wsman::Wsman(void)
{
	HRESULT hr = E_FAIL;
	_pWsmanSessionDispatch = NULL;
	_pWsmanDispatch = NULL;
	_getDispid = 0;
	_putDispid = 0;
	_createSessionDispid = 0;
	_tmpBuf = NULL;

	// get wsman.automation object
	hr = CLSIDFromProgID(L"wsman.automation", &_wsmanOleClsid);
	if (FAILED(hr))
	{
		printf("ERROR: Failed to get 'wsman.automation': %x\n", hr);
		throw hr;
	}

	hr = CoCreateInstance(_wsmanOleClsid, 0, CLSCTX_INPROC_SERVER, IID_IDispatch, (LPVOID *)&_pWsmanDispatch);
	if (FAILED(hr))
	{
		printf("ERROR: CoCreateInstance() failed: %x\n", hr);
		throw hr;
	}
}

Wsman::~Wsman()
{
	if (_pWsmanSessionDispatch)
	{
		_pWsmanSessionDispatch->Release();
		_pWsmanSessionDispatch = NULL;
	}

	if (_tmpBuf)
	{
		SysFreeString(_tmpBuf);
	}
}

void Wsman::CreateSession(const LPWSTR connectionString)
{
	HRESULT hr = E_FAIL;

	if (_createSessionDispid == 0)
	{
		// get dispid for createsession
		OLECHAR FAR* createSessionName = L"createsession";
		hr = _pWsmanDispatch->GetIDsOfNames(IID_NULL, &createSessionName, 1, LOCALE_USER_DEFAULT, &_createSessionDispid);
		if (FAILED(hr))
		{
			printf("ERROR: GetIDsOfNames(createSession) failed: %x\n", hr);
			throw hr;
		}
	}

	DISPPARAMS connectArgs = {0};
	connectArgs.cArgs = 3;
	connectArgs.rgvarg = new VARIANTARG[connectArgs.cArgs];	// parameters are passed in reverse order
	if (NULL == connectArgs.rgvarg)
	{
		printf("ERROR: Out of memory\n");
		throw E_OUTOFMEMORY;
	}
	connectArgs.rgvarg[0].vt = VT_ERROR;	// skip optional param
	connectArgs.rgvarg[0].scode = DISP_E_PARAMNOTFOUND;
	connectArgs.rgvarg[1].vt = VT_ERROR;	// skip optional param
	connectArgs.rgvarg[1].scode = DISP_E_PARAMNOTFOUND;
	connectArgs.rgvarg[2].vt = VT_BSTR;
	connectArgs.rgvarg[2].bstrVal = SysAllocString(connectionString);
	VARIANT var;
	VariantInit(&var);
	hr = _pWsmanDispatch->Invoke(_createSessionDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &connectArgs, &var, NULL, NULL);
	if (FAILED(hr))
	{
		printf("ERROR: Invoke(createSession) failed: %x\n", hr);
		throw hr;
	}

	VariantClear(&(connectArgs.rgvarg[2]));
	delete [] connectArgs.rgvarg;

	hr = var.punkVal->QueryInterface(IID_IDispatch, (LPVOID *)&_pWsmanSessionDispatch);
	if (FAILED(hr))
	{
		printf("ERROR: QueryInterface(IDispatch) failed: %x\n", hr);
		throw hr;
	}
}

BSTR Wsman::Get(const LPWSTR resourceUri)
{
	HRESULT hr = E_FAIL;

	if (_getDispid == 0)
	{
		// get dispids related to get
		OLECHAR FAR* getName = L"get";
		hr = _pWsmanSessionDispatch->GetIDsOfNames(IID_NULL, &getName, 1, LOCALE_USER_DEFAULT, &_getDispid);
		if (FAILED(hr))
		{
			printf("ERROR: GetIDsOfNames(get) failed: %x\n", hr);
			throw hr;
		}
	}
	
	DISPPARAMS getArgs = {0};
	getArgs.cArgs = 2;
	getArgs.rgvarg = new VARIANTARG[getArgs.cArgs];	// parameters are passed in reverse order
	if (NULL == getArgs.rgvarg)
	{
		printf("ERROR: Out of memory\n");
		throw E_OUTOFMEMORY;
	}
	getArgs.rgvarg[0].vt = VT_ERROR;	// skip optional param
	getArgs.rgvarg[0].scode = DISP_E_PARAMNOTFOUND;
	getArgs.rgvarg[1].vt = VT_BSTR;
	getArgs.rgvarg[1].bstrVal = SysAllocString(resourceUri);
	unsigned int argNum = 0;

	VARIANT var;
	VariantInit(&var);
	EXCEPINFO excepinfo = {0};
	hr = _pWsmanSessionDispatch->Invoke(_getDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &getArgs, &var, &excepinfo, &argNum);
	if (FAILED(hr))
	{
		printf("ERROR: Invoke(get) failed: %x\nargNum = %d\n", hr, argNum);
		wprintf(L"Description: %s\n", excepinfo.bstrDescription);
		throw hr;
	}
	VariantClear(&(getArgs.rgvarg[1]));
	delete [] getArgs.rgvarg;

	if (_tmpBuf)
	{
		SysFreeString(_tmpBuf);
	}
	_tmpBuf = SysAllocString(var.bstrVal);
	if (NULL == _tmpBuf)
	{
		printf("ERROR: Out of memory");
		throw E_OUTOFMEMORY;
	}
	VariantClear(&var);

	return _tmpBuf;
}

BSTR Wsman::Put(const LPWSTR resourceUri, const LPWSTR resource)
{
	HRESULT hr = E_FAIL;

	if (_putDispid == 0)
	{
		// get dispids related to put
		OLECHAR FAR* putName = L"put";
		hr = _pWsmanSessionDispatch->GetIDsOfNames(IID_NULL, &putName, 1, LOCALE_USER_DEFAULT, &_putDispid);
		if (FAILED(hr))
		{
			printf("ERROR: GetIDsOfNames(put) failed: %x\n", hr);
			throw hr;
		}
	}
	
	DISPPARAMS putArgs = {0};
	putArgs.cArgs = 3;
	putArgs.rgvarg = new VARIANTARG[putArgs.cArgs];	// parameters are passed in reverse order
	if (NULL == putArgs.rgvarg)
	{
		printf("ERROR: Out of memory\n");
		throw E_OUTOFMEMORY;
	}
	putArgs.rgvarg[0].vt = VT_ERROR;	// skip optional param
	putArgs.rgvarg[0].scode = DISP_E_PARAMNOTFOUND;
	putArgs.rgvarg[1].vt = VT_BSTR;
	putArgs.rgvarg[1].bstrVal = SysAllocString(resource);
	putArgs.rgvarg[2].vt = VT_BSTR;
	putArgs.rgvarg[2].bstrVal = SysAllocString(resourceUri);
	unsigned int argNum = 0;

	VARIANT var;
	VariantInit(&var);
	EXCEPINFO excepinfo = {0};
	hr = _pWsmanSessionDispatch->Invoke(_putDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &putArgs, &var, &excepinfo, &argNum);
	if (FAILED(hr))
	{
		printf("ERROR: Invoke(put) failed: %x\nargNum = %d\n", hr, argNum);
		wprintf(L"Description: %s\n", excepinfo.bstrDescription);
		throw hr;
	}
	VariantClear(&(putArgs.rgvarg[1]));
	VariantClear(&(putArgs.rgvarg[2]));
	delete [] putArgs.rgvarg;

	if (_tmpBuf)
	{
		SysFreeString(_tmpBuf);
	}
	_tmpBuf = SysAllocString(var.bstrVal);
	if (NULL == _tmpBuf)
	{
		printf("ERROR: Out of memory");
		throw E_OUTOFMEMORY;
	}
	VariantClear(&var);

	return _tmpBuf;
}