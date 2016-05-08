//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module SAMPPROV.H | Main include file
//
//
#ifndef _SAMPPROV_H_
#define _SAMPPROV_H_



// Defines -------------------------------------------------------------------
#ifdef DBINITCONSTANTS
#define GLOBAL_(type, name, val) 	extern type name = val
#else
#define GLOBAL_(type, name, val)	extern type name
#endif

#define OBJECT_CONSTRUCTED()		InterlockedIncrement(&g_cObj);
#define OBJECT_DESTRUCTED()			InterlockedDecrement(&g_cObj);

#ifndef  MAX
# define MIN(a,b)  ( (a) < (b) ? (a) : (b) )
# define MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#endif

#define	SAMPPROV_URL_PREFIX			L"sampprov"

////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////
//IMalloc Wrappers
#define CHECK_MEMORY(pv)			if(!(pv)) { hr = E_OUTOFMEMORY; goto CLEANUP; }

#define PROVIDER_ALLOC(cb)			CoTaskMemAlloc((ULONG)(cb));
#define PROVIDER_REALLOC(pv, cb)    CoTaskMemRealloc(pv, (ULONG)(cb));

#define SAFE_ALLOC(pv, type, cb)	{ pv = (type*)CoTaskMemAlloc((ULONG)(cb)*sizeof(type)); CHECK_MEMORY(pv); }
#define SAFE_REALLOC(pv, type, cb)	{ pv = (type*)CoTaskMemRealloc(pv, (ULONG)(cb)*sizeof(type)); CHECK_MEMORY(pv);}
#define SAFE_SYSALLOC(pv, bstr)		{ pv = SysAllocString(bstr); CHECK_MEMORY(pv); }

#define SAFE_FREE(pv)				{ CoTaskMemFree(pv); pv = NULL; }
#define SAFE_SYSFREE(bstr)			{ SysFreeString(bstr); bstr = NULL;}

#define SAFE_DELETE(pv)				{ if(pv) delete    (pv); pv = NULL; }
#define SAFE_DELETE_ARRAY(pv)		{ if(pv) delete [] (pv); pv = NULL; }

//IUnknown->Release Wrapper
#define SAFE_RELEASE(pv)			if((pv)) { ((IUnknown*)pv)->Release(); (pv) = NULL; }  
#define SAFE_ADDREF(pv)				if((pv)) { ((IUnknown*)pv)->AddRef();				}  
																									
//Test macros																					
#define TEST(exp)					{ if(FAILED(exp)) { ASSERT(!#exp); } }
#define TESTC(exp)					{ if(FAILED(exp)) { goto CLEANUP;  } }

#define NUMELEM(x)					(sizeof(x)/sizeof(*x))


#define MAX_HEAP_SIZE          		128000
#define MAX_TOTAL_ROWBUFF_SIZE 		(10*1024*1024)				// Max for all row buffers.
#define MAX_IBUFFER_SIZE       		2000000
#define MAX_BIND_LEN      			(MAX_IBUFFER_SIZE/10)

#define MAX_QUERY_LEN				4096
#define EOL							'\0'
#define wEOL						L'\0'
#define INT_DISPLAY_SIZE			80

#define STAT_ENDOFCURSOR            0x00000100	// for forward-only means fully materialized


//-----------------------------------------------------------------------------
// Memory alignment
//-----------------------------------------------------------------------------

//++
// Useful rounding macros.
// Rounding amount is always a power of two.
//--
#define ROUND_DOWN( Size, Amount )  ((DBLENGTH)(Size) & ~((DBLENGTH)(Amount) - 1))
#define ROUND_UP(   Size, Amount ) (((DBLENGTH)(Size) +  ((DBLENGTH)(Amount) - 1)) & ~((DBLENGTH)(Amount) - 1))

//++
// These macros are for aligment of ColumnData within the internal row buffer.
// COLUMN_ALIGN takes a ptr where you think data ought to go,
// and rounds up to the next appropriate address boundary.
//
// Rule of thumb is "natural" boundary, i.e. 4-byte member should be
// aligned on address that is multiple of 4.
//
// Most everything should be aligned to 32-bit boundary.
// But doubles should be aligned to 64-bit boundary, so let's play it safe.
// Also have __int64.
//--

# define COLUMN_ALIGNVAL 8		// venerable 80x86


// Typedefs ------------------------------------------------------------------
typedef VOID**	LPLPVOID;

typedef long int            SDWORD;
typedef short int           SWORD;
typedef unsigned long int   UDWORD;
typedef unsigned short int  UWORD;
typedef signed long 		SLONG;
typedef unsigned long		ULONG;
typedef unsigned short		USHORT;



// Accessor Structure
typedef struct tagACCESSOR
{
    DBACCESSORFLAGS dwAccessorFlags;
	DBREFCOUNT		cRef;
	DBCOUNTITEM		cBindings;
	DBBINDING		rgBindings[1];
} ACCESSOR, *PACCESSOR;




//-----------------------------------------------------------------------------
// Macros for interface classes -- IUnknown methods and constructor\destructor
// Use these in the class definition for an interface.
// The code is defined in the header (definition) part because
// it is easier to do so here, and easier to use.
//
//
// @func void | DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA |
// Macro to define default IUnknown member data.
//
// @parm BaseClass | BaseClass | . | Class that is the OLE object.
//
// @ex Example usage|
//
//	class CImpISomething : public ISomething {
//	private:
//		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CSomethingBase)
//	public:
//		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CSomethingBase, CImpISomething)
//		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE
//		... implementation-specific methods ...
//	};
//
#define DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(BaseClass)						\
		DEBUGCODE(ULONG m_cRef);											\
		BaseClass		*m_pObj;											\
		LPUNKNOWN		m_pUnkOuter;

// @func void | DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR | 
// Macro to define default IUnknown Ctor/Dtor.
//
// @parm BaseClass | BaseClass      | . | Class that is the OLE object.
//
// @parm Any class | InterfaceClass | . | OLE interface class that this is part of.

#define DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(BaseClass, InterfaceClass)		\
	InterfaceClass( BaseClass *pObj, IUnknown *pUnkOuter )					\
		{																	\
			DEBUGCODE(m_cRef = 0L);											\
			m_pObj		= pObj;												\
			m_pUnkOuter	= pUnkOuter;										\
		}																	\
	~InterfaceClass()														\
		{																	\
		}


// @func void | DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE |
// Macro to define default IUnknown AddRef/Release.

#define DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE								\
	STDMETHODIMP_(ULONG)	AddRef(void)									\
		{																	\
			DEBUGCODE( ++m_cRef );											\
			return m_pUnkOuter->AddRef();									\
		}																	\
	STDMETHODIMP_(ULONG)	Release(void)									\
		{																	\
			ASSERT( m_cRef > 0 );											\
			DEBUGCODE( --m_cRef );											\
			return m_pUnkOuter->Release();									\
		}																	\
	STDMETHODIMP			QueryInterface(REFIID riid, LPVOID *ppv)		\
		{																	\
			return m_pUnkOuter->QueryInterface(riid, ppv);					\
		}



// @func void | DEFINE_ADDREF_RELEASE |
// Macro to define default IUnknown AddRef/Release.

#define DEFINE_ADDREF_RELEASE												\
	STDMETHODIMP_(ULONG)	AddRef(void)									\
		{																	\
			InterlockedIncrement((LONG*)&m_cRef);							\
			return m_cRef;													\
		}																	\
	STDMETHODIMP_(ULONG)	Release(void)									\
		{																	\
			if(InterlockedDecrement((LONG*)&m_cRef))						\
				return m_cRef;												\
																			\
			delete this;													\
			return 0;														\
		}																	


// Globals -------------------------------------------------------------------

GLOBAL_(LONG, g_cObj, 0L);						// # of outstanding objects
GLOBAL_(LONG, g_cLock, 0L);						// # of explicit locks set
GLOBAL_(DWORD, g_cAttachedProcesses, 0L);		// # of attached processes
GLOBAL_(DWORD, g_dwPageSize, 0L);				// System page size
GLOBAL_(HINSTANCE, g_hInstance, 0L);			// Instance Handle
GLOBAL_(IDataConvert *, g_pIDataConvert, NULL);	// IDataConvert pointer

extern WCHAR g_wszDataSourceKeyword[];
extern WCHAR g_wszFileKeyword[];
extern WCHAR g_wszRowKeyword[];

#endif

