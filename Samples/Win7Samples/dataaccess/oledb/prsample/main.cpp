//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module MAIN.CPP
//
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#define DBINITCONSTANTS		// Store all OLE DB consts inside this .obj file
#include "prsample.h"		// Programmer's Reference Sample includes


/////////////////////////////////////////////////////////////////
// Globals					 
//
/////////////////////////////////////////////////////////////////
DWORD g_dwFlags = USE_PROMPTDATASOURCE | DISPLAY_METHODCALLS;


/////////////////////////////////////////////////////////////////
// main
//
//	This is a simple OLE DB application that will display a
//	rowset and will allow basic navigation of that rowset by the
//	user.
//
//	In the sample, functions that begin with 'my' are implemented
//	in the sample code; all other functions are either OLE DB
//	methods or are standard system methods. In addition, two
//	macros are used repeatedly throughout the sample:
//	 - CHECK_HR(hr)  - this macro goes to the CLEANUP label if
//     FAILED(hr), where hr is usually a method call
//	 - XCHECK_HR(hr) - this macro prints the string
//     representation of hr to stderr and if FAILED(hr), attempts
//     to obtain and display any extended error information
//     posted by the last method call, then jumps to the CLEANUP
//     label
//
//	This is the entry point for the sample. This function will:
//	 - parse command line arguments passed to the sample
//	 - display appropriate instructions based on these arguments
//	 - create an OLE DB DataSource object for a user-chosen
//	   provider
//	 - create an OLE DB Session object from the provider's
//	   DataSource object
//	 - create an OLE DB Rowset object, over a table specified by
//	   the user, from the provider's Session object
//	 - display the rowset data and will allow the user to
//	   navigate over the rowset
//
/////////////////////////////////////////////////////////////////
int main()
{
	HRESULT					hr;
	IUnknown *				pUnkDataSource				= NULL;
	IUnknown *				pUnkSession					= NULL;
	IUnknown *				pUnkRowset					= NULL;

	// Parse command line arguments, if any; this will update
	// the value of g_dwFlags as appropriate for the arguments
	if( !myParseCommandLine() )
		return EXIT_FAILURE;

	// Display instructions for the given command line arguments
	myDisplayInstructions();

	// Initialize OLE
	hr = CoInitialize(NULL);
	if( FAILED(hr) )
		return EXIT_FAILURE;

	// Create the Data Source object using the OLE DB service components
	CHECK_HR(hr = myCreateDataSource(&pUnkDataSource));

	// Create a Session object from the Data Source object
	CHECK_HR(hr = myCreateSession(pUnkDataSource, &pUnkSession));

	// Create a Rowset object from the Session object, either directly
	// from the Session or through a Command object
	CHECK_HR(hr = myCreateRowset(pUnkSession, &pUnkRowset));

	// Display the Rowset object data to the user
	CHECK_HR(hr = myDisplayRowset(pUnkRowset, NULL, 0, NULL));

CLEANUP:
	if( pUnkRowset )
		pUnkRowset->Release();
	if( pUnkSession )
		pUnkSession->Release();
	if( pUnkDataSource )
		pUnkDataSource->Release();

	CoUninitialize();

	if( FAILED(hr) )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


/////////////////////////////////////////////////////////////////
// myParseCommandLine
//
//	This function parses the application's command line arguments
//	and sets the appropriate bits in g_dwFlags. If an invalid
//	argument is encountered, a usage message is displayed and
//	the function returns FALSE; otherwise TRUE is returned.
//
/////////////////////////////////////////////////////////////////
BOOL myParseCommandLine
	(
	void
	)
{
	int						iArg;
	CHAR *					psz;

	// Set the locale for all C runtime functions
	setlocale(LC_ALL, ".ACP");

	// Go through each command line argument and set the appropriate
	// bits in g_dwFlags, depending on the chosen options
	for( iArg = 1; iArg < __argc; iArg++ )
	{
		// Inspect the current argument string
		psz = __argv[iArg];

		// Valid options begin with '-' or '/'
		if( psz[0] == '-' || psz[0] == '/' )
		{
			// The next character is the option
			switch( tolower(psz[1]) )
			{
			case 'u':
				// Use the service components UI to prompt for and create
				// the DataSource object; the enumerator is not used
				g_dwFlags |= USE_PROMPTDATASOURCE;
				g_dwFlags &= ~USE_ENUMERATOR;
				continue;
			case 'e':
				// Use the enumerator to select the provider, then use
				// IDataInitialize to create the DataSource object;
				// don't use the UI to prompt for the DataSource
				g_dwFlags |= USE_ENUMERATOR;
				g_dwFlags &= ~USE_PROMPTDATASOURCE;
				continue;
			case 'c':
				// Use ICommand instead of IOpenRowset
				g_dwFlags |= USE_COMMAND;
				continue;
			case 'b':
				// Use ISequentialStream to fetch BLOB column data
				g_dwFlags |= USE_ISEQSTREAM;
				continue;
			case 'n':
				// Don't display method call strings as part of
				// the extended error checking macro
				g_dwFlags &= ~DISPLAY_METHODCALLS;
				continue;
			}
		}

		// Invalid argument; show the usage flags to the user
		fprintf(stderr, "Usage: %s [-u] [-e] [-c] [-b] [-n]\n\nWhere:\n\t" \
			"u = Use the Microsoft Data Links UI " \
				"to create the DataSource\n\t" \
			"e = Use the Enumerator and IDataInitialize " \
				"to create the DataSource\n\t" \
			"c = Use ICommand instead of IOpenRowset to create the Rowset\n\t" \
			"b = Use ISequentialStream for BLOB columns\n\t" \
			"n = Don't display method call strings\n",
			__argv[0]);

		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////
// myDisplayInstructions
//
//	This function asks the user whether they would like
//	instructions displayed for the application. If so, it
//	displays the instructions appropriate to the flags set
//	in g_dwFlags.
//
/////////////////////////////////////////////////////////////////
void myDisplayInstructions
	(
	void
	)
{
	CHAR					ch;

	// Display header and ask the user if they want instructions
	printf("\nOLE DB Programmer's Reference Sample\n" \
		   "====================================\n\n");
	printf("Display instructions [Y or N]? ");
	do
	{
		ch = myGetChar();
	}
	while( ch != 'y' && ch != 'n' );
	printf("%c\n\n", ch);

	// No instructions, so we're done
	if( ch == 'n' )
		return;

	// Display basic instructions
	printf("This application is a simple OLE DB sample that will display\n" \
		   "a rowset and will allow basic navigation of that rowset by\n" \
		   "the user. The application will perform the following steps:\n\n");

	// Display DataSource creation instructions
	if( g_dwFlags & USE_PROMPTDATASOURCE )
	{
		printf(" - Creates a DataSource object through the Microsoft Data\n" \
			   "   Links UI. This allows the user to select the OLE DB\n" \
			   "   provider to use and to set connection properties.\n");
	}
	else
	{
		printf(" - Creates a DataSource object through IDataInitialize::\n" \
			   "   CreateDBInstance, which allows the OLE DB service\n" \
			   "   component manager to add additional functionality to\n" \
			   "   the provider as requested. The user will select the\n" \
			   "   provider to use from a rowset obtained from the OLE DB\n" \
			   "   enumerator.\n");
	}

	// Display Session creation and table-selection instructions
	printf(" - Creates a Session object from the DataSource object.\n");
	printf(" - If the provider supports the schema rowset interface,\n" \
		   "   creates a TABLES schema rowset and allows the user to\n" \
		   "   select a table name from this rowset.\n");

	// Display Rowset creation instructions
	if( g_dwFlags & USE_COMMAND )
	{
		printf(" - Creates a Command object from the Session object and\n" \
			   "   allows the user to specify command text for this Command,\n" \
			   "   then executes the command to create the final rowset.\n");
	}
	else
	{
		printf(" - Creates the final rowset over the table specified by the\n" \
			   "   user.\n");
	}

	printf(" - Displays this rowset and allows the user to perform basic\n" \
		   "   navigation of that rowset.\n\n");

	// Wait for the user to press a key before continuing
	printf("Press a key to continue...");
	myGetChar();
	printf("\n\n");
}



/////////////////////////////////////////////////////////////////
// myGetInputFromUser
//
//	This function prompts the user with the contents of pwszFmt
//	and any accompanying variable arguments, then gets a string
//	as input from the user. If the string is non-empty, it is
//	copied into pwszInput and the function returns TRUE;
//	otherwise this function returns FALSE.
//
/////////////////////////////////////////////////////////////////
BOOL myGetInputFromUser
	(
	LPWSTR					pwszInput,
	size_t					nInputBufferSize,
	LPCWSTR					pwszFmt,
	...
	)
{
	va_list					vargs;
	WCHAR					wszBuffer[MAX_NAME_LEN + 1]	= {0};
	
	// Create the string with variable arguments...
	va_start(vargs, pwszFmt);
	_vsnwprintf_s(wszBuffer, _countof(wszBuffer), MAX_NAME_LEN, pwszFmt, vargs);
	va_end(vargs);

	// Output the string...
	wprintf(wszBuffer);
	
	// Now get the Input from the user...
	_getws_s(wszBuffer, _countof(wszBuffer));
	if( wszBuffer[0] )
	{
		wcscpy_s(pwszInput, nInputBufferSize, wszBuffer);
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////
// myGetChar
//
//	This function gets a character from the keyboard and
//	converts it to lowercase before returning it.
//
/////////////////////////////////////////////////////////////////
CHAR myGetChar
	(
	void
	)
{
	CHAR					ch;

	// Get a character from the keyboard
	ch = _getch();

	// Re-read for the actual key value if necessary
	if( !ch || ch == 0xE0 )
		ch = _getch();

	return tolower(ch);
}
