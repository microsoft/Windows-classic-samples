
//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DBSESS.H | CDBSession base object and contained interface
// definitions
//
//
#ifndef _DBSESS_H_
#define _DBSESS_H_

#include "baseobj.h"


// Forward declarations ------------------------------------------------------

class   CImpIGetDataSource;
class   CImpIOpenRowset;
class	CImpISessionProperties;
class	CImpIDBCreateCommand;
class	CImpIBindResource;

typedef CImpIGetDataSource*		PIMPIGETDATASOURCE;
typedef CImpIOpenRowset*		PIMPIOPENROWSET;
typedef CImpISessionProperties*	PIMPISESSIONPROPERTIES;
typedef	CImpIDBCreateCommand*	PIMPIDBCREATECOMMAND;
typedef CImpIBindResource*		PIMPIBINDRESOURCE;

// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CDBSession | Containing class for all interfaces on the DBSession 
// Object
//
class CDBSession : public CBaseObj				//@base public | CBaseObj
{
    // contained interfaces are friends
    friend class CImpIGetDataSource;
    friend class CImpIOpenRowset;
    friend class CImpIDataSource;
    friend class CImpISessionProperties;
	friend class CImpICreateCommand;
	friend class CImpIBindResource;

	protected: //@access protected
		//@cmember Reference count
		DBREFCOUNT					m_cRef;						
		
		//@member Utility object to manage properties
        PCUTILPROP                  m_pUtilProp;

        //@member contained IOpenRowset
        PIMPIOPENROWSET             m_pIOpenRowset;

		//@member contained IGetDataSource
		PIMPIGETDATASOURCE			m_pIGetDataSource;
		
		//@member contained ISessionProperties
		PIMPISESSIONPROPERTIES		m_pISessionProperties;

		//@member contained IDBCreateCommand
		PIMPIDBCREATECOMMAND		m_pIDBCreateCommand;

		//@member contained IBindResource
		PIMPIBINDRESOURCE			m_pIBindResource;
		
	public: //@access public
		//@cmember Constructor		 
		 CDBSession(LPUNKNOWN);
		//@cmember Destructor
		~CDBSession(void);

		//@mcember parent data source object
		PCDATASOURCE				m_pCDataSource;

		//@cmember Intitialization Routine
		BOOL FInit( CDataSource	*pCDataSource );

		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);
};

typedef CDBSession *PCDBSESSION;


//----------------------------------------------------------------------------------------
// @class CImpIGetDataSource   | contained IGetDataSource class


class CImpIGetDataSource : public IGetDataSource      //@base public | IGetDataSource
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDBSession)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDBSession, CImpIGetDataSource);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

       
        // IGetDataSource method
        //@cmember GetDataSource
		STDMETHODIMP  GetDataSource( REFIID, IUnknown** );
};

//----------------------------------------------------------------------------------------
// @class CImpIOpenRowset   | contained IOpenRowset class


class CImpIOpenRowset : public IOpenRowset      //@base public | IOpenRowset
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDBSession)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDBSession, CImpIOpenRowset);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

       
        // IOpenRowset method
        //@cmember OpenRowset
		STDMETHODIMP  OpenRowset( IUnknown*, DBID*, DBID*, REFIID, ULONG, DBPROPSET[], IUnknown** );
};


//----------------------------------------------------------------------------
// @class CImpISessionProperties | Contained ISessionProperties class
//
class CImpISessionProperties : public ISessionProperties		//@base public | ISessionProperties
{
	private:        //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDBSession)

	public:         //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDBSession, CImpISessionProperties);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBProperties member functions

        //@cmember GetProperties method
        STDMETHODIMP GetProperties
		        	(
						ULONG				cPropertySets,		
				      	const DBPROPIDSET	rgPropertySets[], 	
			        	ULONG*              pcProperties, 	
					 	DBPROPSET**			prgProperties 	    
		        	);


        //@cmember SetProperties method
        STDMETHODIMP	SetProperties
				 	(
						ULONG				cPropertySets,		
					 	DBPROPSET			rgPropertySets[] 	    
					);
};



//----------------------------------------------------------------------------
// @class CImpIDBCreateCommand | Contained IDBCreateCommand class

class CImpIDBCreateCommand : public IDBCreateCommand	//@base public | IDBCreateCommand
{
	private:		//@access private data
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CDBSession);

	public:			//@access public 
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CDBSession, CImpIDBCreateCommand);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IDBCreateCommand member functions

		//@cmember CreateCommand Method
	    STDMETHODIMP	CreateCommand(IUnknown*, REFIID, IUnknown **);
};

#endif

