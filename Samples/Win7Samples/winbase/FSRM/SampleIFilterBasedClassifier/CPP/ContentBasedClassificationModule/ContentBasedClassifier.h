// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// ContentBasedClassifier.h : Declaration of the CContentBasedClassifier and CFsrmClassificationRule

#pragma once
#include "resource.h"       // main symbols

#include "ContentBasedClassificationModule.h"
#include <map>
#include <string>
#include <algorithm>
#include <exception>
#include <ntquery.h>
#include <strsafe.h>
#include <filter.h>
#include <fsrmtlb.h>
#include <fsrmpipeline.h>
#include "..\FsrmTextReader\fsrmtextreader.h"

using namespace std;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


/*++

    class CFsrmClassificationRule

Description:

    This is a container class for the classification rule
	storing the property name, value and associated parameters

--*/

class CFsrmClassificationRule
{
public:
	
	// The classification property's name
	CComBSTR  m_strPropName;

	// The classification property's value that will be applied
	CComBSTR m_strPropValue;

	// The additional classification parameters specified in the classification rule
	SAFEARRAY* m_saParams;

	CFsrmClassificationRule(
		)
	{
	}

	CFsrmClassificationRule(
		LPCWSTR     pwszPropName,
		LPCWSTR     pwszPropValue,
		SAFEARRAY** psaParams
		)
	{
		m_strPropName = pwszPropName;
		m_strPropValue = pwszPropValue;
		m_saParams = *psaParams;
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
typedef std::map < FSRM_OBJECT_ID, CFsrmClassificationRule > FsrmClassificationRulesMap;

/*++

    class CContentBasedClassifier

Description:

    This is the ATL wizard added class for the ATL object.
	It implements the IFsrmClassifierModuleImplementation interface methods.	

--*/

class ATL_NO_VTABLE CContentBasedClassifier :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CContentBasedClassifier, &CLSID_ContentBasedClassifier>,
	public IDispatchImpl<IFsrmClassifierModuleImplementation, &__uuidof(IFsrmClassifierModuleImplementation), &LIBID_FsrmLib, /* wMajor = */ 1>
{
public:
	CContentBasedClassifier()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_CONTENTBASEDCLASSIFIER)

	DECLARE_NOT_AGGREGATABLE(CContentBasedClassifier)

	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CContentBasedClassifier)
		COM_INTERFACE_ENTRY(IFsrmClassifierModuleImplementation)
		COM_INTERFACE_ENTRY(IFsrmPipelineModuleImplementation)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

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

	// TextTokenizer object that reads the stream into a registered IFilter
	// based on the file's extension and uses it for searching contents
	CComPtr<ITextTokenizer> m_pTextTokenizer;
	
};

OBJECT_ENTRY_AUTO(__uuidof(ContentBasedClassifier), CContentBasedClassifier)
