//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module MyOSPObject.cpp | MyOSPObject (OLEDBSimpleProvider) object implementation
//
//

////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////

#include "Common.h"
#include "MyOSPObject.h"
#include <fstream>
using namespace std;
#include <stdio.h>
#include <tchar.h>

////////////////////////////////////////////////////////
// MyOSPObject
//
////////////////////////////////////////////////////////

MyOSPObject::MyOSPObject()
{
	//IUnknown
	m_cRef = 0;
	
	//Member variables
	m_rvTable		= NULL;
	m_cRows			= 0;
	m_cColumns		= 0;
	m_szFilePath	= NULL;
}


MyOSPObject::~MyOSPObject()
{
	DBROWCOUNT i, j;

	// Save the internal table to the data file:
	if (m_szFilePath != NULL)
		saveData();

	// Walk through the internal table.  Free any
	// variants, each row, and finally the table.
	if (m_rvTable != NULL)
	{
		for (i = 0; i <= m_cRows; i++)
		{
			for (j = 1; j <= m_cColumns; j++)
			{
				if (m_rvTable[i][j].vt == VT_BSTR)
					SysFreeString(V_BSTR(&m_rvTable[i][j]));
			}
			CoTaskMemFree(m_rvTable[i]);
		}
		CoTaskMemFree(m_rvTable);
	}

	//Remove all Listeners:
	while(!m_listListeners.IsEmpty())
		m_listListeners.RemoveHead()->Release();

}


HRESULT MyOSPObject::Init(WCHAR* pwszFilePath)
{
	// Store the name of the data file:
	m_szFilePath = (LPSTR)CoTaskMemAlloc(FILENAME_MAX);
	if (!m_szFilePath)
		return E_OUTOFMEMORY;

	size_t cbConverted;
	wcstombs_s(&cbConverted, m_szFilePath, FILENAME_MAX, pwszFilePath, _TRUNCATE);

	// Load the file into a table:
	return loadData();
}


HRESULT MyOSPObject::QueryInterface(REFIID riid, void** ppv)
{
	if (ppv == NULL)
		return E_INVALIDARG;
	
	// IUnknown:
	if (riid == IID_IUnknown)
		*ppv = this;
		
	// OLEDBSimpleProvider:
	else if (riid == IID_OLEDBSimpleProvider)
		*ppv = this;

	// Unsupported:
	else
	{
		*ppv = NULL;								 
		return E_NOINTERFACE;
	}

	SAFE_ADDREF((IUnknown*)*ppv);
	return S_OK;
}

////////////////////////////////////////////////////////
// Routines to load and save table to file
//
////////////////////////////////////////////////////////

HRESULT MyOSPObject::loadData()
{
	HRESULT		hr = E_OUTOFMEMORY;
	ifstream	myOSPFile;
	size_t		cChar;
	BSTR		pwszSeps;		// separator char in file
	BSTR		pwszBuffer = NULL;		// -> wide character buffer
	BSTR		pwszRow;		// -> wide character buffer (position)
	BSTR		pwszToken;
	LPTSTR		pvInput = NULL;		// -> input buffer
	LPSTR		pszPos;
	DBROWCOUNT	i, j;

	// Build the internal array by reading the file:
	myOSPFile.open(m_szFilePath, ios::in | ios::_Nocreate );
    if (!(myOSPFile.is_open()))
        return E_FAIL;

	// Line 1 contains cRows ; cColumns:
	pvInput = new char[MAX_INPUT_BUFFER ];
	CHECK_MEMORY(pvInput);

	myOSPFile.getline(pvInput, MAX_INPUT_BUFFER);
	pszPos = pvInput;
	m_cRows = strtol(pszPos, &pszPos, 10);
	pszPos++;	// consume delimiter
	m_cColumns = strtol(pszPos, &pszPos, 10);

	// allocate array of size cRows X cColumns:
	m_rvTable = (VARIANT**)CoTaskMemAlloc((m_cRows+1) * sizeof(VARIANT*));
	CHECK_MEMORY(m_rvTable);
	ZeroMemory(m_rvTable, (m_cRows+1) * sizeof(VARIANT*));

	for (i = 0; i <= m_cRows; i++)
	{
		m_rvTable[i] = (VARIANT*)CoTaskMemAlloc((m_cColumns+1) * sizeof(VARIANT));
		CHECK_MEMORY(m_rvTable[i]);
	}

	// read the rest of the file and stuff each line into the next row:
	i = 0;
	j = 0;
	pwszSeps = L";";
	pwszBuffer = (BSTR)CoTaskMemAlloc(MAX_WIDE_BUFFER);
	CHECK_MEMORY(pwszBuffer);

	do
	{
		myOSPFile.getline(pvInput, MAX_INPUT_BUFFER);
		pwszRow = pwszBuffer;
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pvInput, -1, pwszRow,
							MAX_WIDE_BUFFER);
		for (j = 1; j <= m_cColumns; j++)
		{
			if (pwszRow == NULL)
			{
				m_rvTable[i][j].vt = VT_NULL;   
				continue;
			}
			cChar = wcscspn(pwszRow, pwszSeps);
			if (cChar)
			{
				LPWSTR pNextToken;

				pwszToken = wcstok_s(pwszRow, pwszSeps, &pNextToken);
				m_rvTable[i][j].vt = VT_BSTR;
				V_BSTR(&m_rvTable[i][j]) = SysAllocString(pwszToken);
				pwszRow += (cChar+1);
			}
			else
			{
				m_rvTable[i][j].vt = VT_NULL;   
				pwszRow += 1;
			}
		}
		i++;
		if (i > m_cRows)
			break;
	}
	while (!(myOSPFile.eof()));

	hr = S_OK;

CLEANUP:

	SAFE_FREE(pwszBuffer);
	if ( FAILED(hr) )
	{
		if (m_rvTable)
		{
			// Since m_rvTable is valid, m_cRows is valid
			for (int i = 0; i <= m_cRows; i++)
			{
				SAFE_FREE(m_rvTable[i]);
			}
			SAFE_FREE(m_rvTable);
		}
	}
	if(pvInput)
		delete [] pvInput;
	
	myOSPFile.close();

	return hr;
}


HRESULT MyOSPObject::saveData()
{
	HRESULT		hr = S_OK;
	ofstream	myOSPFile;
	int			nCnt;
	BSTR		pbstrCell;
	DBROWCOUNT	i, j;
	LPTSTR		pvOutput;		// -> output buffer

	// Open the file for output:
	myOSPFile.open(m_szFilePath, ios::out | ios::_Nocreate);
    if (!(myOSPFile.is_open()))
        return E_FAIL;

	pvOutput = new char[MAX_OUTPUT_BUFFER+1];
	if (!pvOutput)
	{
		hr = E_OUTOFMEMORY;
		goto EXIT;
	}

	// Write the first line containing "rows ; columns":
	wsprintf(pvOutput, "%li;%li\n", m_cRows, m_cColumns);
	myOSPFile.write(pvOutput, lstrlen(pvOutput));
	myOSPFile.flush();

	// Write the columns of each row on a line:
	for (i = 0; i <= m_cRows && hr == S_OK; i++)
	{
		nCnt = 0;
		for (j = 1; j <= m_cColumns; j++)
		{
			if (m_rvTable[i][j].vt == VT_NULL)
			{
				if ((nCnt + lstrlen(";")) >= MAX_OUTPUT_BUFFER)
				{
					hr = DB_E_DATAOVERFLOW;
					break;
				}
				nCnt += wsprintf(pvOutput+nCnt, ";");
			}
			else
			{
				pbstrCell = V_BSTR(&m_rvTable[i][j]);
				if ((nCnt + SysStringByteLen(pbstrCell)) >= MAX_OUTPUT_BUFFER)
				{
					hr = DB_E_DATAOVERFLOW;
					break;
				}
				nCnt += wsprintf(pvOutput+nCnt, "%ws;", pbstrCell);
			}
		}
		if ((nCnt + lstrlen("\n")) >= MAX_OUTPUT_BUFFER)
		{
			hr = DB_E_DATAOVERFLOW;
			break;
		}
		nCnt += wsprintf(pvOutput+nCnt, "\n");
		myOSPFile.write(pvOutput, nCnt);
		myOSPFile.flush();
	}

EXIT:
	// close the file
	myOSPFile.close();
	delete [] pvOutput;

	return hr;
}

////////////////////////////////////////////////////////
// Common notification routine:  send event to each
// Listener in the list.
////////////////////////////////////////////////////////

HRESULT	MyOSPObject::Notify(EOSPEVENTS eEvent, DBROWCOUNT iRow, DB_LORDINAL iColumn, DBROWCOUNT cRows)
{
	//no-op case
	if(m_listListeners.GetCount()==0)
		return S_OK;
	
	//obtain first element
	HRESULT hr = S_OK;
	POS pos = m_listListeners.GetHeadPosition();
	
	//otherwise notify for reason/phase
	switch(eEvent)
	{
		//Multi-Cast
		case CHANGECELL_ABOUTTODO:
			while(pos)
				hr = m_listListeners.GetNext(pos)->aboutToChangeCell(iRow, iColumn);
			break;

		//Multi-Cast
		case CHANGECELL_DIDEVENT:
			while(pos)
				hr = m_listListeners.GetNext(pos)->cellChanged(iRow, iColumn);
			break;

		//Multi-Cast
		case DELETEROWS_ABOUTTODO:
			while(pos)
				hr = m_listListeners.GetNext(pos)->aboutToDeleteRows(iRow, cRows);
			break;

		//Multi-Cast
		case DELETEROWS_DIDEVENT:
			while(pos)
				hr = m_listListeners.GetNext(pos)->deletedRows(iRow, cRows);
			break;

		//Multi-Cast
		case INSERTROWS_ABOUTTODO:
			while(pos)
				hr = m_listListeners.GetNext(pos)->aboutToInsertRows(iRow, cRows);
			break;

		//Multi-Cast
		case INSERTROWS_DIDEVENT:
			while(pos)
				hr = m_listListeners.GetNext(pos)->insertedRows(iRow, cRows);
			break;

		//Multi-Cast
		case ROWSAVAILABLE_DIDEVENT:
			while(pos)
				hr = m_listListeners.GetNext(pos)->rowsAvailable(iRow, cRows);
			break;
	
		//Multi-Cast
		case TRANSFERCOMPLETE_DIDEVENT:
			while(pos)
				hr = m_listListeners.GetNext(pos)->transferComplete(OSPXFER_COMPLETE);
			break;

		//Multi-Cast
		case TRANSFERCOMPLETE_FAILEDTODO:
			while(pos)
				hr = m_listListeners.GetNext(pos)->transferComplete(OSPXFER_ABORT);
			break;

		default:
			//Unknown event?
			break;
	};

	return hr;
}

////////////////////////////////////////////////////////
// OSPSimpleProvider implementation
//
////////////////////////////////////////////////////////

HRESULT MyOSPObject::addOLEDBSimpleProviderListener(OLEDBSimpleProviderListener* pospIListener)
{
	if(pospIListener==NULL)
		return E_INVALIDARG;

	//Add the Listener:
	pospIListener->AddRef();
	m_listListeners.AddTail(pospIListener);

	return S_OK;
}

HRESULT MyOSPObject::deleteRows(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT* pcRowsDeleted)
{
	DBROWCOUNT	cNewRows, i, j;
	VARIANT**	rvNewTable;

	// Doesn't handle label or wildcard rows
	if (iRow <= 0)
		return E_FAIL;
	
	// InvalidArg
	if (cRows < 0)
		return E_INVALIDARG;

	// Is starting row within range?
	if (iRow > m_cRows)
		return E_INVALIDARG;

	// Adjust row count to stay within range:
	if (iRow + cRows > m_cRows + 1)
		cRows = m_cRows - iRow + 1;

	// Notify our Listeners:
	Notify(DELETEROWS_ABOUTTODO, iRow, -1, cRows);

	// Allocate new array with reduced rows:
	cNewRows = m_cRows - cRows;
	rvNewTable = (VARIANT**)CoTaskMemAlloc((cNewRows+1) * sizeof(VARIANT*));
	if (!rvNewTable)
		return E_OUTOFMEMORY;

	j = 0;
	for (i = 0; i <= m_cRows; i++)
	{
		if (i < iRow || i > (iRow + cRows - 1))
			rvNewTable[j++] = m_rvTable[i];		// retain rows
		else
			CoTaskMemFree(m_rvTable[i]);		// free deleted row
	}

	// Free old table, point to new table and adjust count:
	CoTaskMemFree(m_rvTable);
	m_rvTable = rvNewTable;
	m_cRows = cNewRows;

	// Save the internal table to the data file:
	if (m_szFilePath != NULL)
		saveData();

	// Notify our Listeners:
	Notify(DELETEROWS_DIDEVENT, iRow, -1, cRows);

	// Return the count of rows deleted:
	*pcRowsDeleted = cRows;
    return S_OK;
}


HRESULT MyOSPObject::find(DBROWCOUNT iRowStart, DB_LORDINAL iColumn, VARIANT val, OSPFIND findFlags, OSPCOMP compType, DBROWCOUNT* piRowFound)
{
	// Check arguments:
	if (piRowFound == NULL)
		return E_INVALIDARG;
	if (iRowStart <= 0 || iRowStart > m_cRows)
		return E_FAIL;
	if (iColumn <= 0 || iColumn > m_cColumns)
		return E_FAIL;

	// Determine search order:
	DBROWCOUNT	iRowStop = (findFlags == OSPFIND_UP || findFlags == OSPFIND_UPCASESENSITIVE) ? 0 : m_cRows+1;
	BOOL	fCaseSensitive = (findFlags == OSPFIND_CASESENSITIVE || findFlags == OSPFIND_UPCASESENSITIVE);

	// Linear search through all rows:
	while (iRowStart != iRowStop)
	{
		// Although CompareVariant deosn't work for integers relationship(<, ==, >),
		// it works for VT_BSTR. That suffices for this program.
		LONG iCompVal = CompareVariant(&m_rvTable[iRowStart][iColumn], &val, fCaseSensitive);
		switch(compType)
		{
			case OSPCOMP_EQ:
				if (iCompVal ==	0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;
				
			case OSPCOMP_LT:
				if (iCompVal < 0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;

			case OSPCOMP_LE:
				if (iCompVal <= 0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;
			
			case OSPCOMP_GE:
				if (iCompVal >= 0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;
			
			case OSPCOMP_GT:
				if (iCompVal > 0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;
			
			case OSPCOMP_NE:
				if (iCompVal != 0)
				{
					*piRowFound = iRowStart;
					return S_OK;
				}
				break;

			default:
				break;
		};

		if (findFlags == OSPFIND_UP || findFlags == OSPFIND_UPCASESENSITIVE)
			iRowStart--;
		else
			iRowStart++;
	}
	*piRowFound = -1;
	return S_FALSE;
}


HRESULT MyOSPObject::insertRows(DBROWCOUNT iRow, DBROWCOUNT cRows, DBROWCOUNT* pcRowsInserted)
{
	DBROWCOUNT	cNewRows, i, j, k;
	VARIANT**	rvNewTable;

	// Doesn't handle label or wildcard rows
	if (iRow <= 0)
		return E_FAIL;
	
	// InvalidArg
	if (cRows < 0)
		return E_INVALIDARG;

	// If called to insert after the end, adjust to insert at the end:
	if (iRow > m_cRows)
		iRow = m_cRows + 1;

	// Notify our Listeners:
	Notify(INSERTROWS_ABOUTTODO, iRow, -1, cRows);

	// Allocate new array with more rows:
	cNewRows = m_cRows + cRows;
	rvNewTable = (VARIANT**)CoTaskMemAlloc((cNewRows+1) * sizeof(VARIANT*));
	if (rvNewTable)
		return E_OUTOFMEMORY;

	j = 0;
	for (i = 0; i <= cNewRows; i++)
	{
		if (i < iRow || i > (iRow + cRows - 1))
			rvNewTable[i] = m_rvTable[j++];		// retain rows
		else
		{										// insert new empty row
			rvNewTable[i] = (VARIANT*)CoTaskMemAlloc((m_cColumns+1) * sizeof(VARIANT));
			if (!rvNewTable[i])
			{
				for (k = iRow; k < i; k++)
				{
					SAFE_FREE(rvNewTable[k]);
				}
				SAFE_FREE(rvNewTable);
				return E_OUTOFMEMORY;
			}

			for (k = 0; k <= m_cColumns; k++)	// initialize new columns
				rvNewTable[i][k].vt = VT_NULL;
		}
	}

	// Free old table, point to new table and adjust count:
	CoTaskMemFree(m_rvTable);
	m_rvTable = rvNewTable;
	m_cRows = cNewRows;

	// Save the internal table to the data file:
	if (m_szFilePath != NULL)
		saveData();

	// Notify our Listeners:
	Notify(INSERTROWS_DIDEVENT, iRow, -1, cRows);

	// Return the count of rows inserted:
	*pcRowsInserted = cRows;
	return S_OK;
}


HRESULT MyOSPObject::getColumnCount(DB_LORDINAL* pcColumns)
{
	if (pcColumns == NULL)
		return E_INVALIDARG;
	*pcColumns = m_cColumns;
	return S_OK;
}


HRESULT MyOSPObject::getEstimatedRows(DBROWCOUNT* piRows)
{
	return getRowCount(piRows);
}


HRESULT MyOSPObject::getLocale(BSTR* pbstrLocale)
{
	if (pbstrLocale == NULL)
		return E_INVALIDARG;

	*pbstrLocale = NULL;

	// Return default locale:
	TCHAR* pszLocale;
	int cchData = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SLANGUAGE, NULL, 0);
	pszLocale = (TCHAR*)CoTaskMemAlloc(cchData * sizeof(TCHAR));
	if (!pszLocale)
		return E_OUTOFMEMORY;

	BSTR bstr = SysAllocStringLen(NULL, cchData);
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SLANGUAGE, pszLocale, cchData);
#if !defined(_UNICODE)
	bstr = SysAllocStringLen(NULL, cchData);
	size_t cchConverted;
	mbstowcs_s(&cchConverted, bstr, cchData+1, pszLocale, cchData);
#else 
	bstr = SysAllocStringLen(pszLocale, cchData);
#endif
	*pbstrLocale = bstr;

	return S_OK;
}


HRESULT MyOSPObject::getRowCount(DBROWCOUNT* pcRows)
{
	if (pcRows == NULL)
		return E_INVALIDARG;
	*pcRows = m_cRows;
	return S_OK;
}


HRESULT MyOSPObject::getRWStatus(DBROWCOUNT iRow, DB_LORDINAL iColumn, OSPRW* prwStatus)
{
	if (prwStatus == NULL)
		return E_INVALIDARG;

	// In this sample, column 1 is read-only.  All other columns may be changed:
	if (iColumn == 1)
		*prwStatus = OSPRW_READONLY;
	else
		*prwStatus = OSPRW_READWRITE;

	return S_OK;
}


HRESULT MyOSPObject::getVariant(DBROWCOUNT iRow, DB_LORDINAL iColumn, OSPFORMAT format, VARIANT* pVar)
{
	//INVALIDARG
	if (pVar == NULL)
		return E_INVALIDARG;

	//Doesn't handle wildcards
	if (iRow < 0 || iColumn < 0)
		return E_FAIL;
	
	//Get Column Value
	VariantCopy(pVar, &m_rvTable[iRow][iColumn]);
	return S_OK;
}


HRESULT MyOSPObject::isAsync(BOOL* pbAsynch)
{
	if(pbAsynch==NULL)
		return E_INVALIDARG;

	*pbAsynch = FALSE;
	return S_OK;
}


HRESULT MyOSPObject::removeOLEDBSimpleProviderListener(OLEDBSimpleProviderListener* pospIListener)
{
	if(pospIListener==NULL)
		return E_INVALIDARG;

	//Find the Listener in the list:
	POS pos = m_listListeners.Find(pospIListener);
	if (pos == NULL)
		return E_FAIL;

	//Remove the Listener from the list and release it:
	SAFE_RELEASE(pospIListener);
	m_listListeners.RemoveAt(pos);

	return S_OK;
}

HRESULT MyOSPObject::setVariant(DBROWCOUNT iRow, DB_LORDINAL iColumn, OSPFORMAT format, VARIANT Var)
{
	// Doesn't handle wildcards
	if (iRow < 0 || iColumn < 0)
		return E_FAIL;

	// Notify our Listeners:
	Notify(CHANGECELL_ABOUTTODO, iRow, iColumn, 1);

	// Set Column Value
	VariantCopy(&m_rvTable[iRow][iColumn], &Var);

	// Save the internal table to the data file:
	if (m_szFilePath != NULL)
		saveData();

	// Notify our Listeners:
	Notify(CHANGECELL_DIDEVENT, iRow, iColumn, 1);

	return S_OK;
}


HRESULT MyOSPObject::stopTransfer()
{
	// Do nothing because we are already populated
	return S_OK;
}
