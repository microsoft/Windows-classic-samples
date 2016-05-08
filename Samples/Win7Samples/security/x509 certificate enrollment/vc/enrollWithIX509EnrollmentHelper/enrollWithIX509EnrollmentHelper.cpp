//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------

//This sample demonstrates how to use the Windows 7 new http protocol to 
//enroll a certificate by calling the IX509EnrollmentHelper::AddEnrollmentServer 
//and IX509Enrollment2::Enroll methods. The purpose of the call to the
//IX509EnrollmentHelper::AddEnrollmentServer is to cache the authentication
//credential to enrollment server in Windows vault.

#include <windows.h>

#include <CertEnroll.h>
#include <certcli.h>
#include <certsrv.h>

#include <stdio.h>
#include <stdlib.h>

#if (NTDDI_VERSION < NTDDI_WIN7)
#pragma message("This sample cannot run on preWin7")
#pragma warning(error, E_FAIL)
#endif

void Usage();

HRESULT ParseArguments(
				int argc,
				wchar_t *argv[],
				X509CertificateEnrollmentContext *pcontext,
				LPWSTR *ppwszTemplateName,
				X509EnrollmentAuthFlags *pPolicyServerAuthType,
				LPWSTR *ppwszPolicyServerUrl,
				LPWSTR *ppwszPolicyServerUsername,
				LPWSTR *ppwszPolicyServerPassword,
				X509EnrollmentAuthFlags *pEnrollmentServerAuthType,
				LPWSTR *ppwszEnrollmentServerUrl,
				LPWSTR *ppwszEnrollmentServerUsername,
				LPWSTR *ppwszEnrollmentServerPassword);

HRESULT Hex2BstrByte(
				LPWSTR pwszHex,
				BSTR *pbstrByte);

HRESULT EnrollWithIX509EnrollmentHelper(
			X509CertificateEnrollmentContext context,
			LPWSTR pwszTemplateName,
			X509EnrollmentAuthFlags PolicyServerAuthType,
			LPWSTR pwszPolicyServerUrl,
			LPWSTR pwszPolicyServerUsername,
			LPWSTR pwszPolicyServerPassword,
			X509EnrollmentAuthFlags EnrollmentServerAuthType,
			LPWSTR pwszEnrollmentServerUrl,
			LPWSTR pwszEnrollmentServerUsername,
			LPWSTR pwszEnrollmentServerPassword);


int __cdecl wmain(int argc, wchar_t *argv[])
{
	HRESULT hr = S_OK;

	X509CertificateEnrollmentContext context = ContextUser;
	LPWSTR pwszTemplateName = NULL;
	X509EnrollmentAuthFlags PolicyServerAuthType = X509AuthNone;
	LPWSTR pwszPolicyServerUrl = NULL;
	LPWSTR pwszPolicyServerUsername = NULL;
	LPWSTR pwszPolicyServerPassword = NULL;
	X509EnrollmentAuthFlags EnrollmentServerAuthType = X509AuthNone;
	LPWSTR pwszEnrollmentServerUrl = NULL;
	LPWSTR pwszEnrollmentServerUsername = NULL;
	LPWSTR pwszEnrollmentServerPassword = NULL;

	//Parse command line arguments
	hr = ParseArguments(
				argc,
				argv,
				&context,
				&pwszTemplateName,
				&PolicyServerAuthType,
				&pwszPolicyServerUrl,
				&pwszPolicyServerUsername,
				&pwszPolicyServerPassword,
				&EnrollmentServerAuthType,
				&pwszEnrollmentServerUrl,
				&pwszEnrollmentServerUsername,
				&pwszEnrollmentServerPassword);
	if(FAILED(hr))
	{
		Usage();	//Print usage if invalid input was passed.
		goto error;
	}

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr))
		goto error;

	hr = EnrollWithIX509EnrollmentHelper(
				context,
				pwszTemplateName,
				PolicyServerAuthType,
				pwszPolicyServerUrl,
				pwszPolicyServerUsername,
				pwszPolicyServerPassword,
				EnrollmentServerAuthType,
				pwszEnrollmentServerUrl,
				pwszEnrollmentServerUsername,
				pwszEnrollmentServerPassword);
	if(FAILED(hr))
		goto error;

error:
	CoUninitialize();
	
	return hr;
}

//This function has 3 parts. 
//The part1 gets policy server and template with the calls from IX509EnrollmentServer.
//The part2 caches credential in Windows vault with the calls form IX509EnrollmentHelper
//if the authentication type is Certificate or UsernamePassword
//The part3 enrolls a certificate with the calls from IX509Enrollment2
HRESULT EnrollWithIX509EnrollmentHelper(
			X509CertificateEnrollmentContext context,
			LPWSTR pwszTemplateName,
			X509EnrollmentAuthFlags PolicyServerAuthType,
			LPWSTR pwszPolicyServerUrl,
			LPWSTR pwszPolicyServerUsername,
			LPWSTR pwszPolicyServerPassword,
			X509EnrollmentAuthFlags EnrollmentServerAuthType,
			LPWSTR pwszEnrollmentServerUrl,
			LPWSTR pwszEnrollmentServerUsername,
			LPWSTR pwszEnrollmentServerPassword)
{
	HRESULT hr = S_OK;

	BSTR bstrTemplateName = NULL;
	BSTR bstrPolicyServerUrl = NULL;
	BSTR bstrPolicyServerUsername = NULL;
	BSTR bstrPolicyServerPassword = NULL;

	IX509EnrollmentPolicyServer *pPolicyServer = NULL;
	IX509CertificateTemplates *pTemplates = NULL;
	IX509CertificateTemplate *pTemplate = NULL;

	IX509Enrollment2 *pEnroll2 = NULL;

	IX509EnrollmentHelper *pEnrollHelper = NULL;
	
	BSTR bstrEnrollmentServerUrl = NULL;
	BSTR bstrEnrollmentServerUsername = NULL;
	BSTR bstrEnrollmentServerPassword = NULL;

	bstrTemplateName = SysAllocString(pwszTemplateName);
	bstrPolicyServerUrl = SysAllocString(pwszPolicyServerUrl);
	if(!bstrTemplateName && pwszTemplateName ||
	   !bstrPolicyServerUrl && pwszPolicyServerUrl)
	{
		hr = E_OUTOFMEMORY;
		goto error;
	}

	if(PolicyServerAuthType == X509AuthUsername)
	{
		bstrPolicyServerUsername = SysAllocString(pwszPolicyServerUsername);
		bstrPolicyServerPassword = SysAllocString(pwszPolicyServerPassword);

		if(!bstrPolicyServerUsername && pwszPolicyServerUsername ||
		   !bstrPolicyServerPassword && pwszPolicyServerPassword)
		{
			hr = E_OUTOFMEMORY;
			goto error;
		}
	}
	else if(PolicyServerAuthType == X509AuthCertificate)
	{	
		//This call is preparation of the call to pPolicyServer->SetCredential.
		//For certificate authentication, bstrPolicyServerUsername should be a 
		//pointer to a memory blob in which certificate hash value is stored.
		hr = Hex2BstrByte(pwszPolicyServerUsername, &bstrPolicyServerUsername);
		if(FAILED(hr))
			goto error;
	}

	hr = CoCreateInstance(
				__uuidof(CX509EnrollmentPolicyWebService),
				NULL,
				CLSCTX_INPROC_SERVER,
				__uuidof(IX509EnrollmentPolicyServer),
				(void **)&pPolicyServer);
	if(FAILED(hr))
		goto error;

	//The bstrPolicyServerId is optional
	hr = pPolicyServer->Initialize(
							bstrPolicyServerUrl,	//[in] BSTR bstrPolicyServerUrl
							NULL,					//[in] BSTR bstrPolicyServerId
							PolicyServerAuthType,	//[in] X509EnrollmentAuthFlags authFlags
							false,					//[in] VARIANT_BOOL fIsUnTrusted
							context);				//[in] X509CertificateEnrollmentContext context
	if(FAILED(hr))
		goto error;

	//This call sets authentication type and authentication credential
	//to policy server to the object pointed by pPolicyServer.
	//This call is necessary even for Kerberos authentication type.
	hr = pPolicyServer->SetCredential(
								NULL,						//[in] LONG hWndParent
								PolicyServerAuthType,		//[in] X509EnrollmentAuthFalgs flag
								bstrPolicyServerUsername,	//[in] BSTR strCredential,
								bstrPolicyServerPassword);	//[in] BSTR strPassword
	if(FAILED(hr))
		goto error;

	//The flag LoadOptionDefault means enrollment process reads
	//policies (templates) from local cache in file system if it exists.
	//Otherwise, it queries policies (templates) from policy server through
	//http protocol, then caches them in local file system.
	//The flag LoadOptionReload queries policies (templates) directly from
	//policy server regardless of if cached policies (templates) exist.
	hr = pPolicyServer->LoadPolicy(LoadOptionDefault);	//[in] X509EnrollmentPolicyLoadOption option
	if(FAILED(hr))
		goto error;

	//pTemplates points to collection of policies (templates)
	hr = pPolicyServer->GetTemplates(&pTemplates);
	if(FAILED(hr))
		goto error;

	//pTemplate points to the policy (template) specified by a template name
	hr = pTemplates->get_ItemByName(bstrTemplateName, &pTemplate);
	if(FAILED(hr))
		goto error;

	bstrEnrollmentServerUrl = SysAllocString(pwszEnrollmentServerUrl);
	if(!bstrEnrollmentServerUrl && pwszEnrollmentServerUrl)
	{
		hr = E_OUTOFMEMORY;
		goto error;
	}

	if(EnrollmentServerAuthType == X509AuthUsername)
	{
		bstrEnrollmentServerUsername = SysAllocString(pwszEnrollmentServerUsername);
		bstrEnrollmentServerPassword = SysAllocString(pwszEnrollmentServerPassword);

		if(!bstrEnrollmentServerUsername && pwszEnrollmentServerUsername ||
		   !bstrEnrollmentServerPassword && pwszEnrollmentServerPassword)
		{
			hr = E_OUTOFMEMORY;
			goto error;
		}
	}
	else if(EnrollmentServerAuthType == X509AuthCertificate)
	{	
		//This call is preparation of the call to pEnrollHelper->AddEnrollmentServer.
		//For certificate authentication, bstrPolicyServerUsername should be a 
		//pointer to a memory blob in which certificate hash value is stored.
		hr = Hex2BstrByte(pwszEnrollmentServerUsername, &bstrEnrollmentServerUsername);
		if(FAILED(hr))
			goto error;
	}

	if(EnrollmentServerAuthType == X509AuthCertificate ||
	   EnrollmentServerAuthType == X509AuthUsername)
	{
		hr = CoCreateInstance(
					__uuidof(CX509EnrollmentHelper),
					NULL,
					CLSCTX_ALL,
					__uuidof(IX509EnrollmentHelper),
					(void **) &pEnrollHelper);
		if(FAILED(hr))
			goto error;

		hr = pEnrollHelper->Initialize(context);
		if(FAILED(hr))
			goto error;

		//This call caches the authentication credential to
		//enrollment server in Windows vault
		hr = pEnrollHelper->AddEnrollmentServer(
					bstrEnrollmentServerUrl,		//[in] BSTR strEnrollmentServerURI
					EnrollmentServerAuthType,		//[in] X509EnrollmentAuthFlags authFlags
					bstrEnrollmentServerUsername,	//[in] BSTR strCredential
					bstrEnrollmentServerPassword);	//[in] BSTR strPassword
		if(FAILED(hr))
			goto error;
	}

	hr = CoCreateInstance(
				__uuidof(CX509Enrollment),
				NULL,
				CLSCTX_INPROC_SERVER,
				__uuidof(IX509Enrollment2),
				(void **) &pEnroll2);
	if(FAILED(hr))
		goto error;

	//This is a new API in Windows 7 to support http protocol
	hr = pEnroll2->InitializeFromTemplate(
								context,		//[in] X509CertificateEnrollmentContext context
								pPolicyServer,	//[in] IX509EnrollmentPolicyServer *pPolicyServer
								pTemplate);		//[in] IX509CertificateTemplate *pTemplate
	if(FAILED(hr))
		goto error;

	//This call reads authentication cache to
	//enrollment server from Windows vault
	hr = pEnroll2->Enroll();
	if(FAILED(hr))
		goto error;

error:
	if(SUCCEEDED(hr))
		wprintf(L"\nCertificate enrollment succeeded. \n");
	else
		wprintf(L"\nCertificate enrollment failed. \n");

	SysFreeString(bstrTemplateName);
	SysFreeString(bstrPolicyServerUrl);
	SysFreeString(bstrPolicyServerUsername);
	SysFreeString(bstrPolicyServerPassword);
	
	SysFreeString(bstrEnrollmentServerUrl);
	SysFreeString(bstrEnrollmentServerPassword);
	SysFreeString(bstrEnrollmentServerUsername);

	if(pEnrollHelper)
		pEnrollHelper->Release();

	if(pPolicyServer)
		pPolicyServer->Release();

	if(pTemplates)
		pTemplates->Release();

	if(pTemplate)
		pTemplate->Release();

	if(pEnroll2)
		pEnroll2->Release();
	
	return hr;
}


void Usage()
{
	wprintf(L"Usage: \n\n");
	
	wprintf(L"enrollWithIX509EnrollmentHelper.exe <-Param> <Value> \n\n");
	
	wprintf(L"-Param                       Value \n");
	wprintf(L"-Context                     User | Machine \n");
	wprintf(L"-TemplateName                Certificate template name \n");
	wprintf(L"-PolicyServerAuthType        Kerberos | UsernamePassword | Certificate \n");
	wprintf(L"-PolicyServerUrl             Policy server URL \n");
	wprintf(L"-PolicyServerUsername        Username or auth cert hash for policy server authentication \n");
	wprintf(L"-PolicyServerPassword        Password for policy server authentication \n");
	wprintf(L"-EnrollmentServerAuthType    Kerberos | UsernamePassword | Certificate \n");
	wprintf(L"-EnrollmentServerUrl         Enrollment server URL \n");
	wprintf(L"-EnrollmentServerUsername    Username or auth cert hash for enrollment server authentication \n");
	wprintf(L"-EnrollmentServerPassword    Password for enrollment server authentication \n\n");

	wprintf(L"Example: \n");
	wprintf(L"enrollWithIX509EnrollmentHelper.exe ");
	wprintf(L"-Context User ");
	wprintf(L"-TemplateName User ");
	wprintf(L"-PolicyServerAuthType Certificate ");
	wprintf(L"-PolicyServerUrl https://policyservermachinename.sampledomain.sample.com/ADPolicyProvider_CEP_Certificate/service.svc/CEP ");
	wprintf(L"-PolicyServerUsername 02aea105e66a8a2d41a7f630517db0d2c0de625b ");
	wprintf(L"-EnrollmentServerAuthType UsernamePassword ");
	wprintf(L"-EnrollmentServerUrl https://enrollmentservermachinename.sampledomain.sample.com/CaName_CES_UsernamePassword/service.svc/CES ");
	wprintf(L"-EnrollmentServerUsername sampledomain\\sampleuser ");
	wprintf(L"-EnrollmentServerPassword samplepassword \n");	
}

HRESULT ParseArguments(
				int argc,
				wchar_t *argv[],
				X509CertificateEnrollmentContext *pcontext,
				LPWSTR *ppwszTemplateName,
				X509EnrollmentAuthFlags *pPolicyServerAuthType,
				LPWSTR *ppwszPolicyServerUrl,
				LPWSTR *ppwszPolicyServerUsername,
				LPWSTR *ppwszPolicyServerPassword,
				X509EnrollmentAuthFlags *pEnrollmentServerAuthType,
				LPWSTR *ppwszEnrollmentServerUrl,
				LPWSTR *ppwszEnrollmentServerUsername,
				LPWSTR *ppwszEnrollmentServerPassword)
{
	HRESULT hr = E_INVALIDARG;

	LPWSTR pwszParam = NULL;
	LPWSTR pwszValue = NULL;
	
	//Error-check arguments
	if(!pcontext ||
	   !ppwszTemplateName             ||
	   !pPolicyServerAuthType         ||
	   !ppwszPolicyServerUrl          ||
	   !ppwszPolicyServerUsername     ||
	   !ppwszPolicyServerPassword     ||
	   !pEnrollmentServerAuthType     ||
	   !ppwszEnrollmentServerUrl      ||
	   !ppwszEnrollmentServerUsername ||
	   !ppwszEnrollmentServerPassword)
	{
		goto error;
	}

	*ppwszPolicyServerUrl = NULL;

	*ppwszEnrollmentServerUrl = NULL;
	*ppwszEnrollmentServerUsername = NULL;
	*ppwszEnrollmentServerPassword = NULL;

	//Parse command line arguments
	for(int i = 1; i < argc; i+=2)
	{
		pwszParam = argv[i];
		pwszValue = argv[i+1];

		if(*pwszParam == L'-' && pwszValue == NULL)
			goto error;

		if(_wcsicmp(pwszParam, L"-Context") == 0)
		{
			if(_wcsicmp(pwszValue, L"User") == 0)
				*pcontext = ContextUser;
			else if(_wcsicmp(pwszValue, L"Machine") == 0)
				*pcontext = ContextAdministratorForceMachine;
			else
				goto error;
		}
		else if(_wcsicmp(pwszParam, L"-TemplateName") == 0)
			*ppwszTemplateName = argv[i+1];
		else if(_wcsicmp(pwszParam, L"-PolicyServerAuthType") == 0)
		{
			if(_wcsicmp(pwszValue, L"Kerberos") == 0)
				*pPolicyServerAuthType = X509AuthKerberos;
			else if(_wcsicmp(pwszValue, L"UsernamePassword") == 0)
				*pPolicyServerAuthType = X509AuthUsername;
			else if(_wcsicmp(pwszValue, L"Certificate") == 0)
				*pPolicyServerAuthType = X509AuthCertificate;
			else
				goto error;
		}
		else if(_wcsicmp(pwszParam, L"-PolicyServerUrl") == 0)
			*ppwszPolicyServerUrl = pwszValue;
		else if(_wcsicmp(pwszParam, L"-PolicyServerUsername") == 0)
			*ppwszPolicyServerUsername = pwszValue;
		else if(_wcsicmp(pwszParam, L"-PolicyServerPassword") == 0)
			*ppwszPolicyServerPassword = pwszValue;
		else if(_wcsicmp(pwszParam, L"-EnrollmentServerAuthType") == 0)
		{
			if(_wcsicmp(pwszValue, L"Kerberos") == 0)
				*pEnrollmentServerAuthType = X509AuthKerberos;
			else if(_wcsicmp(pwszValue, L"UsernamePassword") == 0)
				*pEnrollmentServerAuthType = X509AuthUsername;
			else if(_wcsicmp(pwszValue, L"Certificate") == 0)
				*pEnrollmentServerAuthType = X509AuthCertificate;
			else
				goto error;
		}
		else if(_wcsicmp(pwszParam, L"-EnrollmentServerUrl") == 0)
			*ppwszEnrollmentServerUrl = pwszValue;
		else if(_wcsicmp(pwszParam, L"-EnrollmentServerUsername") == 0)
			*ppwszEnrollmentServerUsername = pwszValue;
		else if(_wcsicmp(pwszParam, L"-EnrollmentServerPassword") == 0)
			*ppwszEnrollmentServerPassword = pwszValue;
		else
			goto error;
	}

	//Check if necessary arguments were set
	if(!*ppwszTemplateName                    ||
	   *pPolicyServerAuthType == X509AuthNone || 
	   !*ppwszPolicyServerUrl                 || 
	   !*ppwszEnrollmentServerUrl)
	{
		goto error;
	}

	//Set enrollment server to the same auth type as policy server
	//if enrollment server auth type was not passed.
	if(*pEnrollmentServerAuthType == X509AuthNone)
	{
		*pEnrollmentServerAuthType = *pPolicyServerAuthType;
		*ppwszEnrollmentServerUsername = *ppwszPolicyServerUsername;
		*ppwszEnrollmentServerPassword = *ppwszPolicyServerPassword;
	}

	hr = S_OK;
	
error:
	return hr;
}

HRESULT Hex2BstrByte(
				LPWSTR pwszHex,
				BSTR *pbstrByte)
{

	HRESULT hr = S_OK;

	BSTR bstrByte = NULL;

	BYTE *pbData = NULL;
	DWORD cbData = 0;

	if(!pwszHex || !*pwszHex)
	{
		hr = E_INVALIDARG;
		goto error;
	}

	//Get buffer size first.
	if(!CryptStringToBinary(
					pwszHex,
					0,
					CRYPT_STRING_HEX,
					NULL,
					&cbData,
					NULL,
					NULL))
	{
		hr = E_FAIL;
		goto error;
	}

	pbData = (BYTE*)malloc(cbData);
	if(!pbData)
	{
		hr = E_OUTOFMEMORY;
		goto error;
	}

	//pbData points to the memory blob in which
	//the converted hex values are stored.
	if(!CryptStringToBinary(
					pwszHex,
					0,
					CRYPT_STRING_HEX,
					pbData,
					&cbData,
					NULL,
					NULL))
	{
		hr = E_FAIL;
		goto error;
	}
	
	bstrByte = SysAllocStringByteLen((LPCSTR)pbData, cbData);
	if(!bstrByte)
	{
		hr = E_OUTOFMEMORY;
		goto error;
	}

	*pbstrByte = bstrByte;

error:

	free(pbData);

	return hr;
}
