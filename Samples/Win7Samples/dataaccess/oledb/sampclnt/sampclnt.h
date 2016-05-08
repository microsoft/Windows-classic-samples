//--------------------------------------------------------------------
// Microsoft OLE DB Sample Consumer
// (C) Copyright 1991 - 2000 Microsoft Corporation. All Rights Reserved.
//
// File name: SAMPCLNT.H
//
//      Declaration file for a simple OLE DB consumer.
//
//      See OLE DB SDK Guide for information on building and running 
//		this sample, as well as notes concerning the implementation of 
//		a simple OLE DB consumer.
//


#define WIN32_LEAN_AND_MEAN		// avoid the world
#define INC_OLE2				// tell windows.h to always include ole2.h

#include <windows.h>			// 
#include <ole2ver.h>			// OLE2.0 build version
#include <cguid.h>				// GUID_NULL
#include <stdio.h>				// vsnprintf, etc.
#include <stddef.h>				// offsetof
#include <stdarg.h>				// va_arg
#include <time.h>				// time
#include <assert.h>				// assert
#include <conio.h>				// _getch()

//	OLE DB headers
#include <oledb.h>
#include <oledberr.h>


//-----------------------------------
//	constants 
//------------------------------------

// Alignment for placement of each column within memory.
// Rule of thumb is "natural" boundary, i.e. 4-byte member should be
// aligned on address that is multiple of 4.
// Worst case is double or __int64 (8 bytes).
#define COLUMN_ALIGNVAL 8

#define MAX_GUID_STRING     42	// size of a GUID, in characters
#define MAX_NAME_STRING     60  // size of DBCOLOD name or propid string
#define MAX_BINDINGS       100	// size of binding array
#define NUMROWS_CHUNK       20	// number of rows to grab at a time
#define DEFAULT_CBMAXLENGTH 40	// cbMaxLength for binding


// for pretty printing
#define PRETTYPRINT_MAXTOTALWIDTH	200     // max entire width of printed row 
#define PRETTYPRINT_MINCOLWIDTH     6        // min width of printed column



//-----------------------------------
//	macros 
//------------------------------------


// Rounding amount is always a power of two.
#define ROUND_UP( Size, Amount ) (((DWORD)(Size) +  ((Amount) - 1)) & ~((Amount) - 1))

#ifndef  NUMELEM
# define NUMELEM(p) (sizeof(p)/sizeof(*p))
#endif

// usage: DUMPLINE();
#define DUMP_ERROR_LINENUMBER() DumpErrorMsg("Error at file: %s  line: %u  \n", __FILE__, __LINE__)



//-----------------------------------
//	type and structure definitions 
//------------------------------------

// How to lay out each column in memory.
// Issue? we depend on the dwLength field being first in memory (see assert)
// is there another way to handle this?
struct COLUMNDATA 
{
	DBLENGTH	uLength;	// length of data (not space allocated)
	DWORD		dwStatus;	// status of column
	BYTE		bData[1];	// data here and beyond
};


// Lists of value/string pairs.
typedef struct {
	DWORD dwFlag;
	char *szText;
} Note;

#define NOTE(s) { (DWORD) s, #s }




//-----------------------------------
//	global variables and functions that are private to the file 
//------------------------------------


DEFINE_GUID(CLSID_SampProv, 0xE8CCCB79L,0x7C36,0x101B,0xAC,0x3A,0x00,0xAA,0x00,0x44,0x77,0x3D);
DEFINE_GUID(DBINIT_OPT_SAMPPROV_PATH, 0xe9fbaf50, 0xd402, 0x11ce, 0xbe, 0xdc, 0x0, 0xaa, 0x0, 0xa1, 0x4d, 0x7d);

extern IMalloc*	g_pIMalloc;
extern FILE*    g_fpLogFile;






// function prototypes, sampclnt.cpp

void main();

HRESULT DoTests();


HRESULT GetSampprovDataSource
	(
	IDBInitialize**	ppIDBInitialize_out
	);


HRESULT GetDBSessionFromDataSource
    (
    IDBInitialize*      pIDBInitialize,     
    IOpenRowset**       ppIOpenRowset_out   
    );


HRESULT GetRowsetFromDBSession
    (
    IOpenRowset*   pIOpenRowset,   
    LPWSTR         pwszTableName,      
    IRowset**      ppIRowset_out       
    );

    
HRESULT GetDataFromRowset
	(
	IRowset*	pIRowset
	);
    
    
HRESULT GetColumnsInfo
	(
	IRowset*		pIRowset,
	DBORDINAL*		pcCol_out,
	DBCOLUMNINFO**	ppColumnInfo_out,
	WCHAR**			ppStringsBuffer_out
	);

   
HRESULT SetupBindings
	(
	DBORDINAL		cCol,
	DBCOLUMNINFO*	pColumnInfo,
	DBBINDING*		rgBind_out,
	DBCOUNTITEM*	cBind_out,
    DBLENGTH*		pcMaxRowSize_out
	);

    
HRESULT CreateAccessor
	(
	IRowset*	pIRowset,
	DBBINDING*	rgBind,
	DBCOUNTITEM	cBind,
	HACCESSOR*	phAccessor_out
	);

    
HRESULT GetData
	(
	IRowset*	pIRowset,
	DBLENGTH   	cMaxRowSize,
    HACCESSOR	hAccessor,
    DBBINDING*	    rgBind,			// needed for pretty printing
    DBCOUNTITEM	    cBind,	    	// for pretty printing
    DBCOLUMNINFO*	pColumnInfo, 	// for pretty printing
    DBORDINAL		cCol			// for pretty printing		
	);


HRESULT CleanupRowset
	(
	IRowset*	pIRowset,
	HACCESSOR 	hAccessor
	);
    
    
    
// function prototypes, dump.cpp

void DumpErrorMsg
	(
    const char* format,
    ...
	);


void DumpStatusMsg
	(
    const char* format,
    ...
	);


HRESULT DumpErrorHResult
	(
	HRESULT      hr_return,
	const char  *format,
	... 
	);


void DumpColumnsInfo
	(
    DBCOLUMNINFO*	pColInfo,
    DBORDINAL		cCol
    );



void WriteColumnInfo
	(
	FILE*			fp,
	DBCOLUMNINFO*	p 
	);
    

char* GetNoteString
    ( 
	Note * rgNote, 
	int    cNote,
	DWORD  dwValue 
	);


    
char* GetNoteStringBitvals
	(
	Note* 	rgNote,
	int     cNote,
	DWORD   dwValue 
	);


DBLENGTH CalcPrettyPrintMaxColWidth
    (
    DBBINDING*	rgBind,
    DBCOUNTITEM	cBind
    );
 
    
void DumpColumnHeadings
	(
	DBBINDING*		rgBind, 
	DBCOUNTITEM		cBind, 
	DBCOLUMNINFO* 	pColInfo, 
	DBORDINAL		cCol,
    DBLENGTH		cMaxColWidth
	);


WCHAR* LookupColumnName
	(
	DBCOLUMNINFO*	rgColInfo,
	DBORDINAL 			cCol,
	DBORDINAL 			iCol 
	);

void DumpRow
	(
    DBBINDING* 	rgBind,
    DBCOUNTITEM	cBind,
    DBLENGTH	cMaxColWidth,
    BYTE* 		pData
    );


void PrintColumn
	(
	COLUMNDATA		*pColumn,
	DBBINDING		*rgBind,
	DBCOUNTITEM		iBind,
	DBLENGTH		cMaxColWidth 
	);

    
void tfprintf
	(
	FILE*		fp,
	const char* format,
	... 
	);


void tvfprintf
	(
	FILE*		fp,
	const char* format,
	va_list		argptr 
	);







