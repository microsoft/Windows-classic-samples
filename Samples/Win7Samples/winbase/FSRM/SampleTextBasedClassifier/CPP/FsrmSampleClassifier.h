// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// FsrmSampleClassifier.h : Declaration of the CFsrmSampleClassifier

#pragma once
#include "resource.h"       // main symbols

#include "FsrmSampleClassificationModule.h"
#include <map>
#include <string>
#include <algorithm>
#include <fsrmpipeline.h>
#include <fsrmtlb.h>

using namespace std;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// Parameters are passed to the classifier in key value pairs
// You can set the parameters in the Classification Rule Properties UI
// Go to the Classification Tab, and click on Advanced, then go to Additional Classification Parameters
// You can enter the key/value pairs in the data grid control

// This paramter specifies the file name to look for
const WCHAR g_wszFileNameContainsKey[] = L"filenamecontains";

// This parameter specifies the string in the content of the files to search for
const WCHAR g_wszFileContentContainsKey[] = L"filecontentcontains";

/*++

    class CFsrmClassificationRule

Description:

    This is a container class for an Fsrm classification rule's property/value/paramter.

--*/

class CFsrmSampleClassificationRule
{
public:
	
	// The classification property's name
	CComBSTR m_strPropName;

	// The value to set for the classification property
	CComBSTR m_strPropValue;

	// The string to search for in the file's name
	CComBSTR m_strFileNameContains;

	// The string to search for in the file's contents
	CComBSTR m_strFileContentContains;

	CFsrmSampleClassificationRule(
		)
	{
	}

	CFsrmSampleClassificationRule(
		LPCWSTR     pwszPropName,
		LPCWSTR     pwszPropValue,
		LPCWSTR		pwszFileNameContains,
		LPCWSTR		pwszFileContentContains
		)
	{
		m_strPropName = pwszPropName;
		m_strPropValue = pwszPropValue;	
		m_strFileNameContains = pwszFileNameContains;
		m_strFileContentContains = pwszFileContentContains;
	}

};

// The classification rule is identified by a GUID
typedef GUID FSRM_OBJECT_ID;

// A comparison operator for use in the lookup map for classification rules
inline bool operator<(FSRM_OBJECT_ID guid1, FSRM_OBJECT_ID guid2)
{
    return memcmp(&guid1, &guid2, sizeof(FSRM_OBJECT_ID)) < 0;
}

// creating a std map to store <rule id, rule's property/value pair> pairs
// The key is a guid and needs a comparison operator defined
typedef std::map < FSRM_OBJECT_ID, CFsrmSampleClassificationRule > FsrmClassificationRulesMap;

/*++

    class CFsrmSampleClassifier

Description:

    This is the ATL wizard added class for the ATL object.
	It implements the IFsrmClassifierModuleImplementation interface methods.	

--*/

class ATL_NO_VTABLE CFsrmSampleClassifier :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFsrmSampleClassifier, &CLSID_FsrmSampleClassifier>,
	public IDispatchImpl<IFsrmClassifierModuleImplementation, &__uuidof(IFsrmClassifierModuleImplementation), &LIBID_FsrmLib, /* wMajor = */ 1>
{
public:
	CFsrmSampleClassifier(
		)
	{
	}

    DECLARE_GET_CONTROLLING_UNKNOWN()
	DECLARE_REGISTRY_RESOURCEID(IDR_FSRMSAMPLECLASSIFIER)
	DECLARE_NOT_AGGREGATABLE(CFsrmSampleClassifier)

	BEGIN_COM_MAP(CFsrmSampleClassifier)
		COM_INTERFACE_ENTRY(IFsrmClassifierModuleImplementation)
		COM_INTERFACE_ENTRY(IFsrmPipelineModuleImplementation)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct(
		)
	{
		return S_OK;
	}

	void FinalRelease(
		)
	{
	}

public:


public:


	// IFsrmClassifierModuleImplementation Methods
public:
	STDMETHOD(get_LastModified)(
		VARIANT * LastModified
		);

	STDMETHOD(UseRulesAndDefinitions)(
		IFsrmCollection * Rules, 
		IFsrmCollection * propertyDefinitions
		);

	STDMETHOD(OnBeginFile)(
		IFsrmPropertyBag * propertyBag,
                SAFEARRAY           *psaRuleIds
		);

	STDMETHOD(DoesPropertyValueApply)(
		BSTR property, 
		BSTR Value, 
		VARIANT_BOOL * applyValue, 
		GUID idRule, 
		GUID idPropDef
		);

	STDMETHOD(GetPropertyValueToApply)(
		BSTR property, 
		BSTR * Value, 
		GUID idRule, 
		GUID idPropDef
		);
	
	STDMETHOD(OnEndFile)(
		);

	// IFsrmPipelineModuleImplementation Methods
public:
	STDMETHOD(OnLoad)(
		IFsrmPipelineModuleDefinition * moduleDefinition, 
		IFsrmPipelineModuleConnector * * moduleConnector
		);
	
	STDMETHOD(OnUnload)(
		);


private:
	// to hold a reference to the Fsrm pipeline module
	CComPtr<IFsrmPipelineModuleDefinition> m_spDefinition;

	// map to hold all classification rules' property/value pairs
	FsrmClassificationRulesMap	m_mapFsrmClassificationRules;

	// save a reference to the current property bag
	CComPtr<IFsrmPropertyBag>   m_spCurrentPropertyBag;	

private:

	// This method searches the filename or the content of the file
	// for the parameters specified in the classification rule's parameter section
	HRESULT 
	NameOrContentContains(
		LPCWSTR szFileNameContains,
		LPCWSTR szFileContentContains,
		VARIANT_BOOL * bResult
		);
};

OBJECT_ENTRY_AUTO(__uuidof(FsrmSampleClassifier), CFsrmSampleClassifier)
