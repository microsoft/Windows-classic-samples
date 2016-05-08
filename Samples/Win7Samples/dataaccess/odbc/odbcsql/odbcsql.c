/*******************************************************************************
/* ODBCSQL: a sample program that implements an ODBC command line intrepreter.
/*
/* USAGE:	ODBCSQL DSN=<dsn name>   or
/*			ODBCSQL FILEDSN=<file dsn> or
/*			ODBCSQL DRIVER={driver name}
/*
/*
/* Copyright(c) 1991 - 1999 by Microsoft Corporation.   This is an MDAC sample program and
/* is not suitable for use in production environments.   
/*
/******************************************************************************/
/* Modules:
/*		Main				Main driver loop, executes queries.
/*		DisplayResults		Display the results of the query if any
/*		AllocateBindings	Bind column data
/*		DisplayTitles		Print column titles
/*		SetConsole			Set console display mode
/*		HandleError			Show ODBC error messages
/******************************************************************************/
/* Change Log:
/*
/*	8/22/1997	Created
/******************************************************************************/


#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>

/*******************************************/
/* Cheesy macro to call ODBC functions and */
/* report an error on failure.			   */
/* Takes handle, handle type, and stmt     */
/*******************************************/

#define	TRYODBC(h,ht,x) {	RETCODE	iRet=x;\
							if (iRet != SQL_SUCCESS) \
							{ \
								HandleError(h,ht,iRet); \
							} \
							if (iRet == SQL_ERROR) \
							{ \
								fprintf(stderr,"Error in " #x "\n"); \
								goto Exit;	\
							}  \
						}
/******************************************/
/* Structure to store information about   */
/* a column.
/******************************************/

typedef struct STR_BINDING {
	SQLSMALLINT	siDisplaySize;			/* size to display  */
	TCHAR		*szBuffer;				/* display buffer   */
	SQLLEN		indPtr;					/* size or null     */
	BOOL		fChar;					/* character col?   */
	struct STR_BINDING	*sNext;	/* linked list		*/
} BINDING;



/******************************************/
/* Forward references                     */
/******************************************/

void HandleError(SQLHANDLE	hHandle,	
				SQLSMALLINT	hType,	
			    RETCODE	RetCode);
		
void DisplayResults(HSTMT		lpStmt,
			   SQLSMALLINT	cCols);

void AllocateBindings(HSTMT	lpStmt,
					  SQLSMALLINT	cCols,
					  BINDING		**lppBinding,
					  SQLSMALLINT	*lpDisplay);


void DisplayTitles(HSTMT		lpStmt,
					  DWORD		siDisplaySize,
					  BINDING	*pBinding);
	
void SetConsole(	DWORD      		siDisplaySize,
					BOOL			fInvert);

/*****************************************/
/* Some constants                        */
/*****************************************/


#define	DISPLAY_MAX	50			// Arbitrary limit on column width to display
#define	DISPLAY_FORMAT_EXTRA 3	// Per column extra display bytes (| <data> )
#define	DISPLAY_FORMAT		"%c %*.*s "
#define	DISPLAY_FORMAT_C	"%c %-*.*s "
#define	NULL_SIZE			6	// <NULL>
#define	SQL_QUERY_SIZE		1000 // Max. Num characters for SQL Query passed in.

#ifdef UNICODE
#define	PIPE				TEXT('|')
#else
#define PIPE				179 // |
#endif

SHORT	gHeight = 80;		// Users screen height

/***********************************************************************
/* Program to implement ODBC SQL command-line interpreter.
/*
/* Copyright (C) 1991 - 1999 by Microsoft Corporation
/*
/* This is a sample program, Microsoft assumes no liabilities for any
/* use of this program. 
/***********************************************************************
*/


int _tmain(int argc, TCHAR **argv)
{
	SQLHENV		lpEnv = NULL;
	SQLHDBC		lpDbc = NULL;
	SQLHSTMT	lpStmt = NULL;
	TCHAR		*pszConnStr;
	TCHAR		szInput[SQL_QUERY_SIZE];

	// Allocate an environment

	if (SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&lpEnv) == SQL_ERROR)
	{
		fprintf(stderr,"Unable to allocate an environment handle\n");
		exit(-1);
	}

	// Register this as an application that expects 2.x behavior,
	// you must register something if you use AllocHandle

	TRYODBC(lpEnv,
			SQL_HANDLE_ENV,
			SQLSetEnvAttr(lpEnv,
						  SQL_ATTR_ODBC_VERSION,
						  (SQLPOINTER)SQL_OV_ODBC2,
						  0));

	// Allocate a connection

	TRYODBC(lpEnv,
			SQL_HANDLE_ENV,
			SQLAllocHandle(SQL_HANDLE_DBC,lpEnv,&lpDbc));

	

	if (argc > 1)
	{
		pszConnStr = *++argv;
	} else
	{
		pszConnStr = NULL;
	}

	// Connect to the driver.  Use the connection string if supplied
	// on the input, otherwise let the driver manager prompt for input.

	TRYODBC(lpDbc,
			SQL_HANDLE_DBC,
			SQLDriverConnect(lpDbc,
							 GetDesktopWindow(),
							 pszConnStr,
							 SQL_NTS,
							 NULL,
							 0,
							 NULL,
							 SQL_DRIVER_COMPLETE));

	fprintf(stderr,"Connected!\n");

	TRYODBC(lpDbc,
			SQL_HANDLE_DBC,
			SQLAllocHandle(SQL_HANDLE_STMT,lpDbc,&lpStmt));


	printf("Enter SQL commands, type (control)Z to exit\nSQL COMMAND>");

	// Loop to get input and execute queries

	while(_fgetts(szInput, SQL_QUERY_SIZE-1, stdin))
	{
		RETCODE		RetCode;
		SQLSMALLINT	sNumResults;

		// Execute the query

		if (!(*szInput))
		{
			printf("SQL COMMAND>");
			continue;
		}
		RetCode = SQLExecDirect(lpStmt,szInput,SQL_NTS);

		switch(RetCode)
		{
			case	SQL_SUCCESS_WITH_INFO:
			{
				HandleError(lpStmt,SQL_HANDLE_STMT,RetCode);
				// fall through
			}
			case	SQL_SUCCESS:
			{
				// If this is a row-returning query, display
				// results
				TRYODBC(lpStmt,
						SQL_HANDLE_STMT,
						SQLNumResultCols(lpStmt,&sNumResults));

				if (sNumResults > 0)
				{
					DisplayResults(lpStmt,sNumResults);
				} else
				{
					SQLLEN		siRowCount;

					TRYODBC(lpStmt,
							SQL_HANDLE_STMT,
							SQLRowCount(lpStmt,&siRowCount));

					if (siRowCount >= 0)
					{
						_tprintf(TEXT("%d %s affected\n"),
								siRowCount,
								siRowCount == 1 ? TEXT("row") : TEXT("rows"));
					}
				}
				break;
			}

			case	SQL_ERROR:
			{
				HandleError(lpStmt,SQL_HANDLE_STMT,RetCode);
				break;
			}

			default:
					fprintf(stderr,"Unexpected return code %d!\n",RetCode);

		}
		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLFreeStmt(lpStmt,SQL_CLOSE));

		printf("SQL COMMAND>");
	}

Exit:

	// Free ODBC handles and exit

	if (lpDbc)
	{
		SQLDisconnect(lpDbc);
		SQLFreeConnect(lpDbc);
	}
	if (lpEnv)
		SQLFreeEnv(lpEnv);

	printf("\nDisconnected.");

	return 0;

}

/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/* 		lpStmt		ODBC statement handle
/*		cCols		Count of columns
/************************************************************************/

void DisplayResults(HSTMT		lpStmt,
			   		SQLSMALLINT	cCols)
{
	BINDING			*pFirstBinding, *pThisBinding;			
	SQLSMALLINT		siDisplaySize;
	RETCODE			RetCode;
	int				iCount = 0;

	// Allocate memory for each column 

	AllocateBindings(lpStmt,cCols,&pFirstBinding, &siDisplaySize);

	// Set the display mode and write the titles

	DisplayTitles(lpStmt,siDisplaySize+1, pFirstBinding);


	// Fetch and display the data

	do {
		// Fetch a row

		if (iCount++ >= gHeight - 2)
		{
			int 	nInputChar;

			while(1)
			{	
				printf("              ");
				SetConsole(siDisplaySize+2,TRUE);
				printf("   Press ENTER to continue, Q to quit (height:%d)", gHeight);
				SetConsole(siDisplaySize+2,FALSE);

				nInputChar = _getch();
				printf("\n");
				if ((nInputChar == 'Q') || (nInputChar == 'q'))
				{
					goto Exit;
				}
				else if ('\r' == nInputChar)
				{
					break;
				}
				// else loop back to display prompt again
			}

			iCount = 1;
			DisplayTitles(lpStmt,siDisplaySize+1, pFirstBinding);
		}

		TRYODBC(lpStmt,SQL_HANDLE_STMT, RetCode = SQLFetch(lpStmt));

		if (RetCode == SQL_NO_DATA_FOUND)
			break;
			

		// Display the data.   Ignore truncations

		for (pThisBinding = pFirstBinding;
			 pThisBinding;
			 pThisBinding = pThisBinding->sNext)
		{
			if (pThisBinding->indPtr != SQL_NULL_DATA)
			{
				_tprintf(pThisBinding->fChar ? TEXT(DISPLAY_FORMAT_C):
											   TEXT(DISPLAY_FORMAT),
						PIPE,
						pThisBinding->siDisplaySize,
						pThisBinding->siDisplaySize,
						pThisBinding->szBuffer);
			} else
			{
				_tprintf(TEXT(DISPLAY_FORMAT_C),
						PIPE,
						pThisBinding->siDisplaySize,
						pThisBinding->siDisplaySize,
						"<NULL>");
			}
		}
		_tprintf(TEXT(" %c\n"),PIPE);


	} while ( 1);

	SetConsole(siDisplaySize+2,TRUE);
	printf("%*.*s",siDisplaySize+2,siDisplaySize+2," ");
	SetConsole(siDisplaySize+2,FALSE);
	printf("\n");

Exit:
	// Clean up the allocated buffers

	while (pFirstBinding)
	{
		pThisBinding=pFirstBinding->sNext;
		free(pFirstBinding->szBuffer);
		free(pFirstBinding);
		pFirstBinding=pThisBinding;
	}

}

/************************************************************************
/* AllocateBindings:  Get column information and allocate bindings
/* for each column.  
/*
/* Parameters:
/*		lpStmt		Statement handle
/*		cCols		Number of columns in the result set
/*		*lppBinding	Binding pointer (returned)
/*		lpDisplay	Display size of one line
/************************************************************************/

void AllocateBindings(HSTMT			lpStmt,
					  SQLSMALLINT	cCols,
					  BINDING		**lppBinding,
					  SQLSMALLINT	*lpDisplay)
{
	SQLSMALLINT		iCol;
	BINDING			*lpThisBinding, *lpLastBinding;
	SQLLEN			cchDisplay, ssType;
	SQLSMALLINT		cchColumnNameLength;

	*lpDisplay = 0;

	for (iCol = 1; iCol <= cCols; iCol++)
	{
		lpThisBinding = (BINDING *)(malloc(sizeof(BINDING)));
		if (!(lpThisBinding))
		{
			fprintf(stderr,"Out of memory!\n");
			exit(-100);
		}

		if (iCol == 1)
		{
			*lppBinding = lpThisBinding;
		}
		else
		{
			lpLastBinding->sNext = lpThisBinding;
		}
		lpLastBinding=lpThisBinding;


		// Figure out the display length of the column (we will
		// bind to char since we are only displaying data, in general
		// you should bind to the appropriate C type if you are going
		// to manipulate data since it is much faster...)
		
		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLColAttribute(lpStmt,
							   iCol,
							   SQL_DESC_DISPLAY_SIZE,
							   NULL,
							   0,
							   NULL,
							   &cchDisplay));


		// Figure out if this is a character or numeric column; this is
		// used to determine if we want to display the data left- or right-
		// aligned.

		// !! Note a bug in the 3.x documentation.  We claim that 
		// SQL_DESC_TYPE is a 1.x feature.   That is not true, SQL_DESC_TYPE
		// is a 3.x feature.   SQL_DESC_CONCISE_TYPE maps to the 1.x 
		// SQL_COLUMN_TYPE.   This is what you must use if you want to work
		// against a 2.x driver.  Sorry for the inconvenience...

		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLColAttribute(lpStmt,
								iCol,
								SQL_DESC_CONCISE_TYPE,
								NULL,
								0,
								NULL,
								&ssType));


		lpThisBinding->fChar = (ssType == SQL_CHAR ||
								ssType == SQL_VARCHAR ||
								ssType == SQL_LONGVARCHAR);

		lpThisBinding->sNext = NULL;

		// Arbitrary limit on display size
		if (cchDisplay > DISPLAY_MAX)
			cchDisplay = DISPLAY_MAX;

		// Allocate a buffer big enough to hold the text representation
		// of the data.  Add one character for the null terminator

		lpThisBinding->szBuffer = (TCHAR *)malloc((cchDisplay+1) * sizeof(TCHAR));

		if (!(lpThisBinding->szBuffer))
		{
			fprintf(stderr,"Out of memory!\n");
			exit(-100);
		}

		// Map this buffer to the driver's buffer.   At Fetch time,
		// the driver will fill in this data.  Note that the size is 
		// count of bytes (for Unicode).  All ODBC functions that take
		// SQLPOINTER use count of bytes; all functions that take only
		// strings use count of characters.


		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLBindCol(lpStmt,
						   iCol,
						   SQL_C_TCHAR,
						   (SQLPOINTER) lpThisBinding->szBuffer,
						   (cchDisplay + 1) * sizeof(TCHAR),
						   &lpThisBinding->indPtr));


		// Now set the display size that we will use to display
		// the data.   Figure out the length of the column name

		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLColAttribute(lpStmt,
								iCol,
								SQL_DESC_NAME,
								NULL,
								0,
								&cchColumnNameLength,
								NULL));

		lpThisBinding->siDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
		if (lpThisBinding->siDisplaySize < NULL_SIZE)
			lpThisBinding->siDisplaySize = NULL_SIZE;

		*lpDisplay += lpThisBinding->siDisplaySize + DISPLAY_FORMAT_EXTRA;
		
	}

	Exit:
		// Not really a good error exit handler, we should free
		// up any memory that we allocated.   But this is a sample...

		return;
}


/************************************************************************
/* DisplayTitles: print the titles of all the columns and set the 
/*			      shell window's width
/*
/* Parameters:
/*		lpStmt	  		Statement handle
/*		siDisplaySize	Total display size
/*		pBinding		list of binding information
/************************************************************************/

void	DisplayTitles(HSTMT		lpStmt,
					  DWORD		siDisplaySize,
					  BINDING	*pBinding)
{
	TCHAR			szTitle[DISPLAY_MAX];
	SQLSMALLINT		iCol = 1;

	SetConsole(siDisplaySize+2,TRUE);


	for (;pBinding;pBinding=pBinding->sNext)
	{
		TRYODBC(lpStmt,
				SQL_HANDLE_STMT,
				SQLColAttribute(lpStmt,
								iCol++,
								SQL_DESC_NAME,
								szTitle,
								sizeof(szTitle),	// Note count of bytes!
								NULL,
								NULL));

		_tprintf(TEXT(DISPLAY_FORMAT_C), PIPE,
				 pBinding->siDisplaySize,
				 pBinding->siDisplaySize,
				 szTitle);

	}

Exit:

	_tprintf(TEXT(" %c"),PIPE);
	SetConsole(siDisplaySize+2,FALSE);
	_tprintf(TEXT("\n"));

}


/************************************************************************
/* SetConsole: sets console display size and video mode
/*
/* 	Parameters
/*		siDisplaySize	Console display size
/*		fInvert			Invert video?
/************************************************************************/

void	SetConsole(	DWORD      		siDisplaySize,
					BOOL			fInvert)
{
	HANDLE							hConsole;
	CONSOLE_SCREEN_BUFFER_INFO		csbInfo;


    // A little bit of fun here -- reset the console screen
    // buffer size if necessary


    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole != INVALID_HANDLE_VALUE)
    {
		if (GetConsoleScreenBufferInfo(hConsole, &csbInfo))
		{
			if (csbInfo.dwSize.X <  (SHORT) siDisplaySize)
			{
				csbInfo.dwSize.X =  (SHORT) siDisplaySize;
                SetConsoleScreenBufferSize(hConsole,csbInfo.dwSize);
			}

			gHeight = csbInfo.dwSize.Y;
		}

		if (fInvert)
			SetConsoleTextAttribute(hConsole, (WORD)(csbInfo.wAttributes |
												BACKGROUND_BLUE));
		else
			SetConsoleTextAttribute(hConsole,(WORD)(csbInfo.wAttributes & ~(
												BACKGROUND_BLUE)));
	}
}


/************************************************************************
/* HandleError: display error information
/*
/* Parameters:
/*		hHandle		ODBC handle
/*		hType		Type of handle (HANDLE_STMT,HANDLE_ENV,HANDLE_DBC
/*		RetCode		Return code of failing command
/************************************************************************/

void HandleError(SQLHANDLE	hHandle,	
				SQLSMALLINT	hType,	
			    RETCODE	RetCode)
{
	SQLSMALLINT	iRec = 0;
	SQLINTEGER	iError;
	TCHAR		szMessage[1000];
	TCHAR		szState[SQL_SQLSTATE_SIZE+1];


	if (RetCode == SQL_INVALID_HANDLE)
	{
		fprintf(stderr,"Invalid handle!\n");
		return;
	}


	while (SQLGetDiagRec(hType,
						 hHandle,
						 ++iRec,
						 szState,
						 &iError,
						 szMessage,
						 (SQLSMALLINT)(sizeof(szMessage) / sizeof(TCHAR)),
						 (SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		
		// Hide data truncated..
		if (_tcsncmp(szState,TEXT("01004"),5))
			_ftprintf(stderr,TEXT("[%5.5s] %s (%d)\n"),szState,szMessage,iError);
	}

}
