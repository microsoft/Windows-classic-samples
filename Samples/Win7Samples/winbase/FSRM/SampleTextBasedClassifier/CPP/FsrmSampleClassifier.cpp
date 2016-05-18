// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// ContentBasedClassifier.cpp : Implementation of CContentBasedClassifier

#include "stdafx.h"
#include "FsrmSampleClassifier.h"

// Read buffer size for each read on the file
#define ReadBufferSize 1024

/*++

    Routine CFsrmSampleClassifier::get_LastModified

Description:

    This routine is called to determine the last time the classifier's internal rules were modified

Arguments:

    LastModified		- 64-bit FILETIME out parameter

Return value:

    HRESULT

Notes:
    
	For our sample classifier, we specify the rule as never modified

--*/

STDMETHODIMP
CFsrmSampleClassifier::get_LastModified(
	VARIANT * LastModified
	)
{
	// This classifier indicates that rule was never modified
	DECIMAL dec = {0};
	dec.Lo64 = ((ULONGLONG) 0x0000000000000000ui64);
	LastModified->decVal = dec;
	LastModified->vt = VT_DECIMAL;

	return S_OK;
}


/*++

    Routine CFsrmSampleClassifier::UseRulesAndDefinitions

Description:

    This routine is called by the pipeline to specify the rules and property definitions
	this classifier will be called with. A classifier can cache these for future use.

Arguments:

    Rules				- The FSRM rules defined.
	propertyDefinitions - The FSRM classification properties defined.

Return value:

    HRESULT

Notes:
    
	For our sample classifier, we populate our map of <rule-guid,CFsrmSampleClassificationRule>
	for those rules that have the additional classification parameter defined. These are the rules
	this classifier is interested in.

--*/

STDMETHODIMP
CFsrmSampleClassifier::UseRulesAndDefinitions(
		IFsrmCollection * Rules, 
		IFsrmCollection * propertyDefinitions
		)
{
	HRESULT hr = S_OK;
	_variant_t vRule;
	CComPtr<IFsrmRule> pFsrmRule;
	CComPtr<IFsrmClassificationRule> pFsrmClassificationRule;
	CComBSTR		strRuleName;
	wstring			strRuleNameLower;
	CComBSTR		strPropertyName;
	CComBSTR		strPropertyValue;
	CComBSTR		strFileNameContains;
	CComBSTR		strFileContentContains;
	SAFEARRAY *psaParams = NULL;
	LONG			cRuleCount;
	LONG lBoundParams;
	LONG uBoundParams;
	GUID idRule;

	m_mapFsrmClassificationRules.clear();

	// Get the count of rules
	hr = Rules->get_Count( &cRuleCount );

	if (hr != S_OK) {
		goto exit;
	}

	//iterate over the rules collection and build a map to store 
	//this map will be used while processing records for matching rules
	for (LONG iRule = 1; iRule <= cRuleCount; iRule++) {

		vRule.Clear( );

		// Get the next rule
		hr = Rules->get_Item( iRule, &vRule );
		if (hr != S_OK) {
			goto exit;
		}
	
		// Query for the IFsrmRule interface
		pFsrmRule.Release( );
		hr = vRule.pdispVal->QueryInterface( _uuidof( IFsrmRule ), (void **)&pFsrmRule );
		if (hr != S_OK) {
			goto exit;
		}

		// Query for the IFsrmClassificationRule interface
		pFsrmClassificationRule.Release( );
		hr = pFsrmRule->QueryInterface( _uuidof( IFsrmClassificationRule ), (void **)&pFsrmClassificationRule );
		if (hr != S_OK) {
			goto exit;
		}

		// Get the rule's name
		strRuleName.Empty( );
		hr = pFsrmRule->get_Name( &strRuleName );
		if (hr != S_OK) {
			goto exit;
		}		

		// Get the property name for this rule
		strPropertyName.Empty( );
		hr = pFsrmClassificationRule->get_PropertyAffected( &strPropertyName );
		if (hr != S_OK) {
			goto exit;
		}

		// Get the property value that will be applied by this rule
		strPropertyValue.Empty( );
		hr = pFsrmClassificationRule->get_Value( &strPropertyValue );
		if (hr != S_OK) {
			goto exit;
		}

		// Get the rule id guid
		::memset(&idRule, 0, sizeof(GUID));
		hr = pFsrmRule->get_Id(&idRule);
		if (FAILED(hr))
		{
			goto exit;
		}

		// Get the additional parameters specified for this rule
		// This classifier needs filenamecontains and filecontentcontains
		psaParams = NULL;
		hr = pFsrmRule->get_Parameters(&psaParams);
		if (FAILED(hr))
		{
			goto exit;
		}

		if (psaParams)
		{
			lBoundParams = 0;
			uBoundParams = 0;

			hr = SafeArrayGetLBound(psaParams, 1, &lBoundParams);
			if (FAILED(hr))
			{
				goto exit;
			}

			hr = SafeArrayGetUBound(psaParams, 1, &uBoundParams);
			if (FAILED(hr))
			{
				goto exit;
			}

			strFileNameContains.Empty();
			strFileContentContains.Empty();

			for (LONG iParam = lBoundParams; iParam <= uBoundParams; iParam++)
			{
				_variant_t vParam;				

				hr = SafeArrayGetElement(psaParams, &iParam, &vParam);
				if (FAILED(hr))
				{
					goto exit;
				}

				CComBSTR strParam = vParam.bstrVal;
				LPWSTR pwszParamKey = (LPWSTR)strParam;
				LPWSTR pwszParamVal = ::wcschr(pwszParamKey, L'=');
				*pwszParamVal = 0;
				pwszParamVal++;

				CComBSTR strKey = pwszParamKey;
				hr = strKey.ToLower();
				if (FAILED(hr))
				{
					goto exit;
				}

				if (wcscmp(strKey, g_wszFileNameContainsKey) == 0)
				{
					strFileNameContains = SysAllocString(pwszParamVal);
					hr = strFileNameContains.ToLower();
					if (FAILED(hr))
					{
						goto exit;
					}
				}
				else if (wcscmp(strKey, g_wszFileContentContainsKey) == 0)
				{
					strFileContentContains = SysAllocString(pwszParamVal);
					hr = strFileContentContains.ToLower();
					if (FAILED(hr))
					{
						goto exit;
					}
				}
			}

			if (strFileNameContains.Length() != 0 || strFileContentContains.Length() != 0)
			{
				//create a new property/value pair to store
				CFsrmSampleClassificationRule newRule( 
					strPropertyName, 
					strPropertyValue,
					strFileNameContains,
					strFileContentContains
					);

				//make sure rule name is lower case for ease in matching
				strRuleNameLower = strRuleName;
				std::transform( strRuleNameLower.begin( ), strRuleNameLower.end( ), strRuleNameLower.begin( ), towlower );

				//add rule with property/value to map
				m_mapFsrmClassificationRules[idRule] = newRule;	
			}

		}


	
	}

exit:

	return hr;

}

/*++

    Routine CFsrmSampleClassifier::OnBeginFile

Description:

    This routine is called by the pipeline before processing each file in the scope.

Arguments:

    propertyBag				- The FSRM property bag for this file.
	
Return value:

    HRESULT

Notes:
    
	We cache this property bag for future access to the streaming interface on the file, 
	and to access other properties of the file e.g. the file's name

--*/

STDMETHODIMP
CFsrmSampleClassifier::OnBeginFile(
		IFsrmPropertyBag * propertyBag,
                SAFEARRAY           *psaRuleIds
		)
{
	HRESULT hr = S_OK;
	
	m_spCurrentPropertyBag = propertyBag;	

	return hr;
}

/*++

    Routine CFsrmSampleClassifier::GetPropertyValueToApply

Description:

    This routine is called by the pipeline to determine what value to apply for a property.

Arguments:

    propertyBag				- The FSRM property bag for this file.
	
Return value:

    HRESULT

Notes:
    
	This classifier uses the predefined values in the classification UI.

--*/

STDMETHODIMP
CFsrmSampleClassifier::GetPropertyValueToApply(
		BSTR property, 
		BSTR * Value, 
		GUID idRule, 
		GUID idPropDef
		)
{
	HRESULT hr = S_OK;
	return hr;
}

/*++

    Routine CFsrmSampleClassifier::DoesPropertyValueApply

Description:

    This routine is called by the FSRM pipeline to determine if 
	a rule is applicable to the current file being processed.	

Arguments:

    property				- The FSRM property being applied
	Value					- The value for this property that will be applied
	applyValue				- A boolean for the classifier to indicate that this property is to be applied
	idRule					- The rule guid 
	idPropDef				- The property's definition guid
	
Return value:

    HRESULT

Notes:
    
	This classifier looks up the rule guid against the cache'd map to see if it should be interested
	in this rule.
	If so, the classifier finds the rule definition in its map. It then uses the rule's parameters
	to search in the file's name and content.

--*/

STDMETHODIMP
CFsrmSampleClassifier::DoesPropertyValueApply(
		BSTR property, 
		BSTR Value, 
		VARIANT_BOOL * applyValue, 
		GUID idRule, 
		GUID idPropDef
		)
{
	HRESULT hr = S_OK;
	wstring strRuleNameLower;	
	FsrmClassificationRulesMap::iterator iterRules;		
	VARIANT_BOOL bResult = VARIANT_FALSE;

	try
	{
		iterRules = m_mapFsrmClassificationRules.find(idRule);
		if (iterRules == m_mapFsrmClassificationRules.end())
		{
			goto exit;
		}
		else
		{
			CFsrmSampleClassificationRule &matchingRule = iterRules->second;			
			hr = NameOrContentContains(
				matchingRule.m_strFileNameContains, 
				matchingRule.m_strFileContentContains, 
				&bResult
				);
			if (FAILED(hr))
			{
				goto exit;
			}
		}
	}
	catch( _com_error& err ) {
		hr = err.Error( );
	}
	catch( const std::bad_alloc& ) {
		hr = E_OUTOFMEMORY;
	}
	catch( const std::exception& ) {
		hr = E_UNEXPECTED;
	}
	

	*applyValue = bResult;

exit:
	return hr;	

}
	
/*++

    Routine CFsrmSampleClassifier::OnEndFile

Description:

    This routine is called at the end of processing a file

Arguments:

	None

Return value:

    HRESULT

Notes:

	Nothing to cleanup

--*/

STDMETHODIMP
CFsrmSampleClassifier::OnEndFile(
		)
{
	HRESULT hr = S_OK;
	return hr;

}

/*++

    Routine CFsrmSampleClassifier::OnLoad

Description:

    This routine performs the initialization phase of the classifier module.

Arguments:

    moduleDefinition	- this instance of the pipeline module
	moduleConnector		- out parameter, FsrmPipelineModuleConnector created and bound to during
					      the initialization phase

Return value:

    HRESULT

Notes:
    
	Performs initialialization by creating a connector 
	and binding this instance of the module to it.

--*/
STDMETHODIMP
CFsrmSampleClassifier::OnLoad(
		IFsrmPipelineModuleDefinition * moduleDefinition, 
		IFsrmPipelineModuleConnector * * moduleConnector
		)
{
	HRESULT hr = S_OK;
	CComPtr<IFsrmPipelineModuleConnector> pConnector;
	CComQIPtr<IFsrmPipelineModuleImplementation> pModuleImpl;

	try {

		if (moduleDefinition == NULL) {
			hr = E_INVALIDARG;
			goto exit;
		}
		
		(*moduleConnector) = NULL;

		m_spDefinition = moduleDefinition;		

		// create the FsrmPipelineModuleConnector object reference
		hr = ::CoCreateInstance(
			__uuidof(FsrmPipelineModuleConnector),
			NULL,			
			CLSCTX_ALL,
			__uuidof(IFsrmPipelineModuleConnector),
			(void **)&pConnector);
		
		if (hr != S_OK) {
			goto exit;
		}

		pModuleImpl = GetControllingUnknown( );

		if (pModuleImpl == NULL) {
			hr = E_NOINTERFACE;
			goto exit;
		}

		//bind the connector to this instance
		hr = pConnector->Bind( moduleDefinition, pModuleImpl );

		(*moduleConnector) = pConnector.Detach( );

	}
	catch( _com_error& err ) {
		hr = err.Error( );
	}
	catch( const std::bad_alloc& ) {
		hr = E_OUTOFMEMORY;
	}
	catch( const std::exception& ) {
		hr = E_UNEXPECTED;
	}

exit:
	return hr;
}
	
/*++

    Routine CFsrmSampleClassifier::OnUnload

Description:

    This routine is called when the classifier is unloaded

Arguments:

	None

Return value:

    HRESULT

Notes:

	This classifier does nothing on unload.

--*/

STDMETHODIMP
CFsrmSampleClassifier::OnUnload(
		)
{
	HRESULT hr = S_OK;
	return hr;
}



/*++

    Routine CFsrmSampleClassifier::NameOrContentContains

Description:

    This routine is private to the classifier.
	It performs the search in the file's name and content

Arguments:

    szFileNameContains		- The string to search for in the file's name
	szFileContentContains	- The string to search for in the file's contents
	bResult					- The result if any of the above was found

Return value:

    HRESULT

Notes:

    This classifier first looks at the file's name for a match of the filename search-string.
	If the filename does not contain the search-string, it obtains a stream on the file
	and searches the stream for the file content search-string.

--*/

HRESULT 
CFsrmSampleClassifier::NameOrContentContains(
	LPCWSTR szFileNameContains,
	LPCWSTR szFileContentContains,
	VARIANT_BOOL * bResult
	)

{
	HRESULT hr = S_OK;
	CComVariant var;
	CComBSTR strFileName;
	CComBSTR strFromBytesRead;
	BYTE bytesToRead[ReadBufferSize];
	ULONG bytesRead;
	ULARGE_INTEGER readOffset;	
	wstring strRuleNameLower;	
	wstring strMatch;
	size_t pos = 0;	

	*bResult = VARIANT_FALSE;

	try {

		if (szFileNameContains != NULL)
		{

			// get the file name and lower case it for lookup
			m_spCurrentPropertyBag->get_Name( &strFileName );
			hr = strFileName.ToLower( );
			if (FAILED(hr))
			{
				goto exit;
			}
			strMatch = strFileName;

			pos = strMatch.find(szFileNameContains);

			if (std::wstring::npos != pos)
			{
				*bResult = VARIANT_TRUE;
				goto exit;
			}
		}
		if (szFileContentContains != NULL)
		{
			// get the file's streaming interface
			hr = m_spCurrentPropertyBag->GetFileStreamInterface(
				FsrmFileStreamingMode_Read,
				FsrmFileStreamingInterfaceType_ILockBytes,
				&var);

			if (hr != S_OK) {
				goto exit;
			}

			if (var.vt == VT_UNKNOWN && var.punkVal != NULL) {

				CComQIPtr<ILockBytes> pLockBytes = var.punkVal;
				if (pLockBytes == NULL) {
					hr = S_FALSE;
					goto exit;
				}

				readOffset.QuadPart = 0;

				do
				{

					// read the first ReadBufferSize bytes from the file
					hr = pLockBytes->ReadAt( 
						readOffset, 
						bytesToRead, 
						sizeof(bytesToRead) - 1, 
						&bytesRead
						); 

					if (hr != S_OK) {
						hr = S_FALSE;
						goto exit;
					}
					
					if (bytesRead > 0)
					{

						bytesToRead[bytesRead] = 0;
						readOffset.QuadPart += bytesRead;
							
						strFromBytesRead.Empty();
						hr = strFromBytesRead.Append((char *)bytesToRead);
						if (FAILED(hr))
						{
							goto exit;
						}
						hr = strFromBytesRead.ToLower( );
						if (FAILED(hr))
						{
							goto exit;
						}
						strMatch += strFromBytesRead;

						pos = strMatch.find(szFileContentContains);

						if (std::wstring::npos != pos)
						{
							*bResult = VARIANT_TRUE;	
							goto exit;
						}

						strMatch.empty();			
						strMatch += strFromBytesRead;
					}
				} while(bytesRead > 0);

			}
		}

			
	}
	catch( _com_error& err ) {
		hr = err.Error( );
	}
	catch( const std::bad_alloc& ) {
		hr = E_OUTOFMEMORY;
	}
	catch( const std::exception& ) {
		hr = E_UNEXPECTED;
	}
	

	
exit:
	return hr;
}

