// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// ContentBasedClassifier.cpp : Implementation of CContentBasedClassifier

#include "stdafx.h"
#include "ContentBasedClassifier.h"


/*++

    Routine CContentBasedClassifier::get_LastModified

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
CContentBasedClassifier::get_LastModified(
	VARIANT * LastModified
	)
{
	DECIMAL dec = {0};
	dec.Lo64 = ((ULONGLONG) 0x0000000000000000ui64);
	LastModified->decVal = dec;
	LastModified->vt = VT_DECIMAL;

	return S_OK;
}

/*++

    Routine CContentBasedClassifier::UseRulesAndDefinitions

Description:

    This routine is called by the pipeline to specify the rules and property definitions
	this classifier will be called with. A classifier can cache these for future use.

Arguments:

    Rules				- The FSRM rules defined.
	propertyDefinitions - The FSRM classification properties defined.

Return value:

    HRESULT

Notes:
    
	For our sample classifier, we populate our map of <rule-guid,CFsrmClassificationRule>
	for those rules that have the additional classification parameter defined. These are the rules
	this classifier is interested in.

--*/

STDMETHODIMP
CContentBasedClassifier::UseRulesAndDefinitions(
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
	LONG			cRuleCount;
	SAFEARRAY *psaParams = NULL;
	SAFEARRAY *psaParsedParams = NULL;
	SAFEARRAYBOUND rgsaBound[1];
	LONG paramIndex;
	LONG lBoundParams;
	LONG uBoundParams;
	GUID idRule;

	m_mapFsrmClassificationRules.clear();

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

		// Get the additional classification parameters for this rule
		psaParams = NULL;
		hr = pFsrmRule->get_Parameters(&psaParams);
		if (FAILED(hr))
		{
			goto exit;
		}

		// Get the rule id guid
		::memset(&idRule, 0, sizeof(GUID));
		hr = pFsrmRule->get_Id(&idRule);
		if (FAILED(hr))
		{
			goto exit;
		}

		psaParsedParams = NULL;

		if (psaParams)
		{
			// The additional parameters array contains parameters in the following format
			// key=value
			// This classifier treats the key as a value as well since it needs to search for words
			// in the contents of files. 
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

			rgsaBound[0].cElements = ((uBoundParams - lBoundParams + 1) * 2);
			rgsaBound[0].lLbound = 0;
			
			paramIndex = 0;

			psaParsedParams = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);
			if (psaParsedParams == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto exit;
			}

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

				BSTR bstrKey = SysAllocString(pwszParamKey);
				BSTR bstrVal = SysAllocString(pwszParamVal);

				_variant_t vKey = bstrKey;
				_variant_t vVal = bstrVal;

				hr = SafeArrayPutElement(psaParsedParams, &paramIndex, &vKey);
				if (FAILED(hr))
				{
					goto exit;
				}

				paramIndex++;

				hr = SafeArrayPutElement(psaParsedParams, &paramIndex, &vVal);
				if (FAILED(hr))
				{
					goto exit;
				}

			}

			//create a new property/value pair to store
			CFsrmClassificationRule newRule( strPropertyName, strPropertyValue, &psaParsedParams );

			//make sure rule name is lower case for ease in matching
			strRuleNameLower = strRuleName;
			std::transform( strRuleNameLower.begin( ), strRuleNameLower.end( ), strRuleNameLower.begin( ), towlower );

			//add rule with property/value to map
			m_mapFsrmClassificationRules[idRule] = newRule;	

		}	
	}

exit:

	return hr;

}

/*++

    Routine CContentBasedClassifier::OnBeginFile

Description:

    This routine is called by the pipeline before processing each file in the scope.

Arguments:

    propertyBag				- The FSRM property bag for this file.
	
Return value:

    HRESULT

Notes:
    
	This classifier uses the property bag to initialize the FsrmTextReader.
	The filename and streaming interface is retrieved by the FsrmTextReader
	and used to initialize an IFilter registered for the file's extention type

--*/

STDMETHODIMP
CContentBasedClassifier::OnBeginFile(
		IFsrmPropertyBag * propertyBag,
                SAFEARRAY           *psaRuleIds
		)
{
	HRESULT hr = S_OK;

	try {

		hr = m_pTextTokenizer->InitializeWithPropertyBag(propertyBag);
		if (hr != S_OK)
		{
			goto exit;
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

/*++

    Routine CContentBasedClassifier::GetPropertyValueToApply

Description:

    This routine is called by the pipeline to determine what value to apply for a property.

Arguments:

    propertyBag				- The FSRM property bag for this file.
	
Return value:

    HRESULT

Notes:
    
	This classifier does not supply the property's value.

--*/

STDMETHODIMP
CContentBasedClassifier::GetPropertyValueToApply(
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

    Routine CContentBasedClassifier::DoesPropertyValueApply

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
	If the rule is present in the map, the classifier retrieves it.
	It uses the parameters specified in the rule as search-strings and passes these to the FsrmTextReader
	to search in the IFilter chunks.

--*/

STDMETHODIMP
CContentBasedClassifier::DoesPropertyValueApply(
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
	VARIANT_BOOL vSrchResult = VARIANT_FALSE;	

	try
	{
		iterRules = m_mapFsrmClassificationRules.find(idRule);
		if (iterRules == m_mapFsrmClassificationRules.end())
		{
			goto exit;
		}
		else
		{
			CFsrmClassificationRule &matchingRule = iterRules->second;

			hr = m_pTextTokenizer->DoesContainWordsFromList(matchingRule.m_saParams, &vSrchResult);
			if (FAILED(hr))
			{
				hr = S_OK;
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
	

	
exit:
	*applyValue = vSrchResult;
	return hr;	

}
	
/*++

    Routine CContentBasedClassifier::OnEndFile

Description:

    This routine is called at the end of processing a file

Arguments:

	None

Return value:

    HRESULT

Notes:

	Call cleanup on the ITextTokenizer to release the IFilter associated with stream

--*/

STDMETHODIMP
CContentBasedClassifier::OnEndFile(
		)
{
	HRESULT hr = S_OK;	
	hr = m_pTextTokenizer->Cleanup();
	return hr;
}

/*++

    Routine CContentBasedClassifier::OnLoad

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
	Also creates an object of ITextTokenizer

--*/

STDMETHODIMP
CContentBasedClassifier::OnLoad(
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


		hr = ::CoCreateInstance(
			__uuidof(TextTokenizer),
			NULL,
			CLSCTX_ALL,
			__uuidof(ITextTokenizer),
			(void **)&m_pTextTokenizer);

		if (FAILED(hr)) {
			goto exit;
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

/*++

    Routine CContentBasedClassifier::OnUnload

Description:

    This routine is called when the classifier is unloaded

Arguments:

	None

Return value:

    HRESULT

Notes:

	This classifier releases the ITextTokenizer object on unload

--*/

STDMETHODIMP
CContentBasedClassifier::OnUnload(
		)
{
	HRESULT hr = S_OK;

	m_pTextTokenizer.Release();

	return hr;

}