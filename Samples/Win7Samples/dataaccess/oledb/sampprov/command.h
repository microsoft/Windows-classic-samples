//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module COMMAND.H | CCommand base object and contained interface
// definitions
//
//

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "baseobj.h"
#include "dbsess.h"

// Forward declarations ------------------------------------------------------
class CImpIAccessor;
class CImpICommandText;
class CImpICommandProperties;
class CImpIColumnsInfo;
class CImpIConvertType;

typedef CImpICommandText*			PIMPICOMMANDTEXT;
typedef CImpICommandProperties*		PIMPICOMMANDPROPERTIES;
typedef CImpIColumnsInfo*			PIMPICOLUMNSINFO;
typedef CImpIConvertType*			PIMPICONVERTTYPE;

// Constants -----------------------------------------------------------------
// For CCommand::m_dwStatus
// These are bit masks.
enum COMMAND_STATUS_FLAG {
	// Command Object status flags
	CMD_STATUS_MASK			= 0x0000000F,
	CMD_INITIALIZED			= 0x00000001,
	CMD_TEXT_SET			= 0x00000002,
	CMD_EXECUTING			= 0x00000004,
	CMD_EXEC_CANCELED		= 0x10000000,
};

// Classes -------------------------------------------------------------------


//----------------------------------------------------------------------------
// @class CCommand | Containing class for all interfaces on the Command CoType Object
//
class CCommand : public CBaseObj				//@base public | CBaseObj
{
	//	Contained interfaces are friends
	friend class CImpIAccessor;
	friend class CImpICommandText;
	friend class CImpICommandProperties;
	friend class CImpIColumnsInfo;
	friend class CImpIConvertType;

	//@access protected
	protected:
		//@cmember Reference count
		DBREFCOUNT					m_cRef;						
		//@cmember Controlling IUnknown
		CImpIAccessor*				m_pIAccessor;
		//@cmember Contained ICommandText and ICommand
		PIMPICOMMANDTEXT			m_pICommandText;
		//@cmember Contained ICommandProperties
		PIMPICOMMANDPROPERTIES		m_pICommandProperties;
		//@cmember Contained IColumnsInfo
		PIMPICOLUMNSINFO			m_pIColumnsInfo;			
		//@cmember Contained IConvertType
		PIMPICONVERTTYPE			m_pIConvertType;

		//@cmember Execution Status Flags
		UDWORD						m_dwStatus;
		//@cmember Count of Active Rowsets on this command object
		ULONG						m_cRowsetsOpen;

		//@cmember Impersonation GUID
		GUID						m_guidImpersonate;

		//@cmember GUID for dialect of current text or tree
		GUID						m_guidCmdDialect;
		//@cmember current file to open, if any
		WCHAR *						m_strCmdText;
		//@member Utility object to manage properties
		PCUTILPROP					m_pUtilProp;

	//@access public
	public:
		
		//@cmember Constructor
		 CCommand(CDBSession* pCSession, LPUNKNOWN pUnkOuter);
		
		 //@cmember Destructor
		~CCommand(void);

		//@cmember Parent Session Object
		CDBSession*					m_pCSession;

		//@cmember Initialization Routine
		HRESULT FInit();

		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);

		//@cmember Used to cancel currently executing commands
		HRESULT CheckCanceledHelper();

		inline void SetImpersonateIID(const GUID* pGuid) { 
			assert(pGuid);
			m_guidImpersonate = *pGuid; 
		};

		inline WCHAR * const GetCommandText() {
			return m_strCmdText; 
		};

		inline BOOL IsCommandSet() {
			return !!(m_dwStatus & CMD_TEXT_SET); 
		};

		inline void DecrementOpenRowsets() {  
			InterlockedDecrement( (LONG*) &m_cRowsetsOpen );
			ASSERT( m_cRowsetsOpen != (ULONG)-1 );
		};
		
		inline void IncrementOpenRowsets() {
			InterlockedIncrement( (LONG*) &m_cRowsetsOpen );
		};

		inline BOOL IsRowsetOpen() {
			return (m_cRowsetsOpen > 0) ? TRUE : FALSE; 
		};
};

typedef CCommand*			PCCOMMAND;


//----------------------------------------------------------------------------
// @class CImpICommandText | Contained ICommandText class
// Object
//
class CImpICommandText : public ICommandText	//@base public | ICommandText
{
	private: //@access Private Data Members
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CCommand)

	public: //@access Public Functions
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CCommand, CImpICommandText);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	ICommandText members
		//@cmember GetCommandText Method
	    STDMETHODIMP			GetCommandText(GUID* pguidDialect, LPOLESTR* ppwszCommand);
		//@cmember SetCommandText Method
		STDMETHODIMP			SetCommandText(REFGUID rguidDialect, LPCOLESTR pwszCommand);
		//@cmember Cancel Method, inherited from ICommand
	    STDMETHODIMP			Cancel();
		//@cmember Execute Method, inherited from ICommand
		STDMETHODIMP			Execute(IUnknown* pUnkOuter, 
								REFIID riid, DBPARAMS* pParams, 
								DBROWCOUNT* pcRowsAffected, IUnknown** ppRowset);
		//@cmember GetDBSession Method
		STDMETHODIMP			GetDBSession(REFIID riid, IUnknown** ppSession);
};



//-----------------------------------------------------------------------------
// @class CImpICommandProperties | Base class to implement ICommandProperties.
//
//
class CImpICommandProperties : public ICommandProperties	//@base public | ICommandProperties
{
	private: //@access Private Data Members
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CCommand)

	public: //@access Public Functions
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CCommand, CImpICommandProperties);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE
		
		//	ICommandProperties members
		//@cmember GetProperties Method
        STDMETHODIMP	GetProperties(const ULONG cPropertySets, 
						const DBPROPIDSET rgPropertySets[], ULONG* pcProperties, 
						DBPROPSET** prgProperties);
		//@cmember SetProperties Method
        STDMETHODIMP	SetProperties
				 	(
						ULONG				cPropertySets,		
					 	DBPROPSET			rgPropertySets[] 	    
					);
};

#endif __COMMAND_H__