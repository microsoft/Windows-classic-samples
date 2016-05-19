//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ServiceComp Header Module | 	This module contains header information
//					for the CServiceComp class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------
// file ServiceComp.h

#ifndef __SERVICECOMP_HPP__
#define __SERVICECOMP_HPP__

#include "windows.h"
#include "mtxdm.h"
#include "svcintfs.h"


#include "DataSource.hpp"

const ULONG g_cCreationMethods			= 4;

class CLightDataSource;

// this class store info about Service Components
// and offer means to get various interfaces and pool configs
class CServiceComp
{
	protected:
		IDataInitialize				*m_pIDataInit;
		IDBPromptInitialize			*m_pIDBPromptInit;
		static ULONG				s_SCRef;
		
		
		// says whether or not MTS is loaded in the current process
		static BOOL					IsMTSPresent();

		// says whether or not IIS is loaded in the current process
		static BOOL					IsIISPresent();

		// not sure this belong here
		static BOOL					IsWindows2000();

		// retrieve Dispenser Manager
		static HRESULT				GetDispenserManager(
										REFCLSID			rclsid, 
										IDispenserManager	**ppIDispenserMan);
		static HRESULT				GetDispenserManager(
										LPSTR swzLibName, 
										IDispenserManager **ppIDispenserMan);
		static HRESULT				GetDispMan(IDispenserManager **ppIDispenserMan);

		
	public:
									CServiceComp() : m_pIDataInit(NULL), 
										m_pIDBPromptInit(NULL){
									}

									~CServiceComp() {
										ReleaseAll();
									}

		static BOOL					CanGetShutdownGuaranteed();

		// these 2 methods create the required interface
		// and increments the SC counter
		// the interfaces are not cached in the CServiceComp object!
		static IDataInitialize		*pIDataInitialize();
		static IDBPromptInitialize	*pIDBPromptInitialize();

		IDataInitialize				*GetIDataInitialize() {
			if (!m_pIDataInit)
				m_pIDataInit = pIDataInitialize();
			return m_pIDataInit;
		}

		IDBPromptInitialize			*GetIDBPromptInitialize(){
			if (!m_pIDBPromptInit)
				m_pIDBPromptInit = pIDBPromptInitialize();
			return m_pIDBPromptInit;
		}


		DBORDINAL					ReleaseAll(){
			if (m_pIDataInit)
				ReleaseSCInterface(m_pIDataInit);
			m_pIDataInit = NULL;
			if (m_pIDBPromptInit)
				ReleaseSCInterface(m_pIDBPromptInit);
			m_pIDBPromptInit = NULL;
			return s_SCRef;
		}	
		static DBORDINAL			GetRefCount() {
			return s_SCRef;
		}


		// get the number of connections to the default server
		static LONG					GetServerConnNo();

		// use this method to release SC interfaces got from this class
		static ULONG				ReleaseSCInterface(IUnknown *pIUnknown){
			if (pIUnknown)
			{
				DBORDINAL	ulInUse	= 0;
				DBORDINAL	ulIdle	= 0;
				LONG		lConn	= 0;

				ASSERT(0 < s_SCRef);
				if (1 == s_SCRef)
				{
					Sleep(100);
					lConn = GetServerConnNo();
				}
				pIUnknown->Release();
			}
			return s_SCRef;
		}

		// create a datasource either by CoCreateInstance or by SC
		// CoCreateDSO doesn't use SC to create the instance
		static HRESULT				CoCreateDSO(
										CLightDataSource		*pDSO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);

		HRESULT						CreateDBInstance(
										CLightDataSource		*pDPO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);

		HRESULT						CreateDBInstanceEx(
										CLightDataSource		*pDPO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);

		HRESULT						GetDataSource(
										CLightDataSource		*pDPO,
										REFIID				riid,
										WCHAR				*pwszInitString,
										BOOL				fInitialize = FALSE
									);

		HRESULT						GetDataSource(
										CLightDataSource		*pDPO,
										REFIID				riid,
										CLSID				clsidProvider, 
										CPropSets			*pPropSets,
										BOOL				fInitialize = FALSE
									);

		HRESULT						GetDataSource(
										CLightDataSource		*pDPO,
										REFIID				riid,
										CPropSets			*pPropSets,
										BOOL				fInitialize = FALSE
									) {
										return GetDataSource(pDPO, riid, GUID_NULL, pPropSets, fInitialize);
		}

/*		HRESULT						PromptDataSource(
										CLightDataSource		*pDPO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);
*/

		// single interface for creating DPO usiong a certain method, or with random method
		HRESULT						CreateDPO(
										CREATIONMETHODSENUM	dwCreationMethod,
										CLightDataSource		*pDPO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);

		HRESULT						CreateDPO(
										CLightDataSource		*pDPO,
										CLSID				clsidProvider, 
										REFIID				riid,
										ULONG				cPropSets	= 0, 
										DBPROPSET			*rgPropSets	= NULL, 
										BOOL				fInitialize = FALSE
									);


		CServiceComp				*operator = (IDataInitialize *pIDataInitialize)
									{
										if (m_pIDataInit)
											ReleaseSCInterface(m_pIDataInit);
										pIDataInitialize->AddRef();
										m_pIDataInit = pIDataInitialize;
										s_SCRef++;
										return this;
									}

		static DBORDINAL			Get_SCRef(){
										return s_SCRef;
		}
}; //CServiceComp

BOOL __stdcall		GetComputerNameWRAP(LPWSTR lpBuffer, LPDWORD lpnSize);
BOOL __stdcall		GetComputerNameANSI(LPWSTR lpBuffer, LPDWORD lpnSize);

typedef	BOOL (__stdcall *LPFGetComputerName)( LPWSTR, LPDWORD );

#endif //__SERVICECOMP_HPP__