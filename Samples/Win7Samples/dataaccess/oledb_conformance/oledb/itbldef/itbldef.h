//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ITblDef.H | ITableDefinition header file for test modules.
//
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 09-06-95 | Microsoft | Updated
//

#ifndef _ITBLDEF_H_
#define _ITBLDEF_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------


typedef enum _tagOps{
	NONE = 0,
	ADD_CONSTRAINT,
	DROP_CONSTRAINT,
	CREATETABLE_CONSTRAINTS,
} OpsEnum;

class CThreadParam{
public:
	HRESULT	m_hr;
	OpsEnum	m_op;

	CThreadParam() {
		m_hr = E_FAIL;
		m_op = NONE;
	}
}; //CThreadParam

class CCTWCThreadParam : public CThreadParam{
public:
	IUnknown			*m_pUnkOuter;
	DBID				*m_pTableID;
	DBORDINAL			m_cColumnDescs;
	DBCOLUMNDESC		*m_rgColumnDescs;
	ULONG				m_cConstraintDescs;
	DBCONSTRAINTDESC	*m_rgConstraintDescs;
	IID					*m_piid;
	ULONG				m_cPropertySets;
	DBPROPSET			*m_rgPropertySets;
	DBID				**m_ppTableID;
	IUnknown			**m_ppRowset;

	CCTWCThreadParam() : m_pUnkOuter(NULL), m_pTableID(NULL), m_cColumnDescs(0),
		m_rgColumnDescs(NULL), m_piid(NULL), m_cPropertySets(0), m_rgPropertySets(NULL), 
		m_ppTableID(NULL), m_ppRowset(NULL)	{;}

	~CCTWCThreadParam() {
		if (m_ppTableID)
			ReleaseDBID(*m_ppTableID, TRUE);
		if (m_ppRowset)
			SAFE_RELEASE(*m_ppRowset);	
	}
}; //CCTWCThreadParam

#endif 	//_ITBLDEF_H_
