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
//enroll a certificate by calling the IX509Enrollment2:CreateRequest, 
//ICertRequest3::SetCredential, ICertRequest3::Submit and 
//IX509Enrollment2::InstallResponse2 methods. The purpose of the call to
//the ICertRequest3::SetCredential is to set the authentication credential
//to enrollment server in the object pointed by the interface ICertRequest3.

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

HRESULT EnrollWithICertRequest3(
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
		Usage();	//Print usage if invalid input was passed
		goto error;
	}

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr))
		goto error;

	hr = EnrollWithICertRequest3(
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

//This function demonstrates how to enroll a certificate
//through Windows 7 new http protocol. Both policy server
//and enrollment server require authentication. The credentials
//to the authentication can be set by calling the method
//IX509EnrollmentPolicyServer::SetCredential and 
//ICertRequest3::SetCredential.
HRESULT EnrollWithICertRequest3(
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
	BSTR bstrPolicyServerId = NULL;
	BSTR bstrPolicyServerUsername = NULL;
	BSTR bstrPolicyServerPassword = NULL;

	IX509EnrollmentPolicyServer *pPolicyServer = NULL;
	IX509CertificateTemplates *pTemplates = NULL;
	IX509CertificateTemplate *pTemplate = NULL;

	IX509Enrollment2 *pEnroll2 = NULL;
	BSTR bstrRequest = NULL;
	
	BSTR bstrEnrollmentServerUrl = NULL;
	BSTR bstrEnrollmentServerUsername = NULL;
	BSTR bstrEnrollmentServerPassword = NULL;

	ICertRequest3 *pCertRequest3 = NULL;
	LONG lDisposition = 0;
	BSTR bstrDisposition = NULL;
	VARIANT varFullResponse;

	VariantInit(&varFullResponse);
	varFullResponse.vt = VT_BSTR;
	varFullResponse.bstrVal = NULL;

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
								PolicyServerAuthType,		//[in] X509EnrollmentAuthFlags flag
								bstrPolicyServerUsername,	//[in] BSTR strCredential
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

	//This call is preparation of the call IX509Enrollment2::InstallResponse2
	hr = pPolicyServer->GetPolicyServerId(&bstrPolicyServerId);
	if(FAILED(hr))
		goto error;

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

	//Get request blob for the call ICertRequest3::Submit
	hr = pEnroll2->CreateRequest(
						XCN_CRYPT_STRING_BASE64,
						&bstrRequest);
	if(FAILED(hr))
		goto error;

	hr = CoCreateInstance(
					__uuidof(CCertRequest),
					NULL,
					CLSCTX_INPROC_SERVER,
					__uuidof(ICertRequest3),
					(void **) &pCertRequest3);
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
		//This call is preparation of the call to pCertRequest3->SetCredential.
		//For certificate authentication, bstrPolicyServerUsername should be a 
		//pointer to a memory blob in which certificate hash value is stored.
		hr = Hex2BstrByte(pwszEnrollmentServerUsername, &bstrEnrollmentServerUsername);
		if(FAILED(hr))
			goto error;
	}

	//This call sets authentication type and authentication credential
	//to enrollment server to the object pointed by pCertRequest3.
	//This call is necessary even for Kerberos authentication type.
	hr = pCertRequest3->SetCredential(
						NULL,							//[in] LONG hWnd
						EnrollmentServerAuthType,		//[in] X509EnrollmentAuthFlags AuthType
						bstrEnrollmentServerUsername,	//[in] BSTR strCredential
						bstrEnrollmentServerPassword);	//[in] BSTR strPassword
	if(FAILED(hr))
		goto error;

	//Submit request and get response
	//For ICertRequest3::Submit, bstrConfig can be
	//enrollment server URL
	hr = pCertRequest3->Submit(
							CR_IN_BASE64 | CR_IN_FORMATANY,	//[in] LONG Flags
							bstrRequest,					//[in] BSTR const strRequest
							NULL,							//[in] BSTR const strAttributes
							bstrEnrollmentServerUrl,		//[in] BSTR const strConfig
							&lDisposition);					//[out, retval] LONG *pDisposition
	if(FAILED(hr))
		goto error;

	//Check the submission status
	if(lDisposition != CR_DISP_ISSUED) //Not enrolled
	{
		hr = pCertRequest3->GetDispositionMessage(&bstrDisposition);
		if(FAILED(hr))
			goto error;

		pCertRequest3->GetLastStatus(&hr);

		if(lDisposition == CR_DISP_UNDER_SUBMISSION) //Pending
		{
			wprintf(L"The submission is in pending status: %s \n", bstrDisposition);
			goto error;
		}
		else //Failed
		{
			wprintf(L"The submission failed: %s \n", bstrDisposition);
			goto error;
		}
	}

	//Get full response for installation
	hr = pCertRequest3->GetFullResponseProperty(
				FR_PROP_FULLRESPONSENOPKCS7,	//[in] LONG PropId (FR_PROP_*)
				0,								//[in] LONG PropIndex
				PROPTYPE_BINARY,				//[in] LONG PropType (PROPTYPE_*
				CR_OUT_BASE64,					//[in] LONG Flags (CR_OUT_*)
				&varFullResponse);				//[out, retval] VARIANT *pvarPropertyValue
	if(FAILED(hr))
		goto error;

	//Install the response
	hr = pEnroll2->InstallResponse2(
				AllowNone,					//[in] InstallResponseRestrictionFlags Restrictions
				varFullResponse.bstrVal,	//[in] BSTR strResponse
				XCN_CRYPT_STRING_BASE64,	//[in] EnrodingType Encoding
				bstrPolicyServerPassword,	//[in] BSTR strPassword
				bstrPolicyServerUrl,		//[in] BSTR strEnrollmentPolicyServerUrl
				bstrPolicyServerId,			//[in] BSTR strEnrollmentPolicyServerID
				PsfNone,					//[in] PolicyServerUrlFlags EnrollmentPolicyServerFlags
				PolicyServerAuthType);		//[in] X509EnrollmentAuthFlags authFlags
	if(FAILED(hr))
		goto error;
				
error:
	if(SUCCEEDED(hr))
		wprintf(L"\nCertificate enrollment succeeded. \n");
	else
		wprintf(L"\nCertificate enrollment failed. \n");

	SysFreeString(bstrTemplateName);
	SysFreeString(bstrPolicyServerUrl);
	SysFreeString(bstrPolicyServerId);
	SysFreeString(bstrPolicyServerUsername);
	SysFreeString(bstrPolicyServerPassword);
	
	SysFreeString(bstrDisposition);
	SysFreeString(bstrEnrollmentServerUrl);
	SysFreeString(bstrEnrollmentServerPassword);
	SysFreeString(bstrEnrollmentServerUsername);
	SysFreeString(bstrRequest);

	if(!varFullResponse.bstrVal)
		VariantClear(&varFullResponse);

	if(pPolicyServer)
		pPolicyServer->Release();

	if(pTemplates)
		pTemplates->Release();

	if(pCertRequest3)
		pCertRequest3->Release();

	if(pTemplate)
		pTemplate->Release();

	if(pEnroll2)
		pEnroll2->Release();
	
	return hr;
}


void Usage()
{
	wprintf(L"Usage: \n\n");
	
	wprintf(L"enrollWithICertRequest3.exe <-Param> <Value> \n\n");
	
	wprintf(L"-Param                       Value \n");
	wprintf(L"-Context                     User | Machine \n");
	wprintf(L"-TemplateName                Certificate template name \n");
	wprintf(L"-PolicyServerAuthType        Kerberos | UsernamePassword | Certificate \n");
	wprintf(L"-PolicyServerUrl             Policy server URL \n");
	wprintf(L"-PolicyServerUsername        Username or auth cert hash for policy server authentication \n");
	wprintf(L"-PolicyServerPassword        Password for policyserver authentication \n");
	wprintf(L"-EnrollmentServerAuthType    Kerberos | UsernamePassword | Certificate \n");
	wprintf(L"-EnrollmentServerUrl         Enrollment server URL \n");
	wprintf(L"-EnrollmentServerUsername    Username or auth cert hash for enrollment server authentication \n");
	wprintf(L"-EnrollmentServerPassword    Password for enrollment server authentication \n\n");

	wprintf(L"Example: \n");
	wprintf(L"enrollWithICertRequest3.exe ");
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
