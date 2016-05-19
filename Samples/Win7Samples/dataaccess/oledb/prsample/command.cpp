//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module COMMAND.CPP
//			
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "prsample.h"		// Programmer's Reference Sample includes



/////////////////////////////////////////////////////////////////
// myCreateCommand
//
//	This function takes an IUnknown pointer on a Session object
//	and attempts to create a Command object using the Session's
//	IDBCreateCommand interface. Since this interface is optional,
//	this may fail.
//
/////////////////////////////////////////////////////////////////
HRESULT	myCreateCommand
	(
	IUnknown *				pUnkSession,
	IUnknown **				ppUnkCommand
	)
{
	HRESULT					hr;
	IDBCreateCommand *		pIDBCreateCommand			= NULL;
	
	// Attempt to create a Command object from the Session object
	XCHECK_HR(hr = pUnkSession->QueryInterface(
				IID_IDBCreateCommand, (void**)&pIDBCreateCommand));
	XCHECK_HR(hr = pIDBCreateCommand->CreateCommand(
				NULL,				//pUnkOuter
				IID_ICommand,		//riid
				ppUnkCommand		//ppCommand
				));

CLEANUP:
	if( pIDBCreateCommand )
		pIDBCreateCommand->Release();
	return hr;
}




/////////////////////////////////////////////////////////////////
// myExecuteCommand
//
//	This function takes an IUnknown pointer on a Command object
//	and performs the following steps to create a new Rowset
//	object:
//	 - sets the given properties on the Command object; these
//	   properties will be applied by the provider to any Rowset
//	   created by this Command
//	 - sets the given command text for the Command
//	 - executes the command to create a new Rowset object
//
/////////////////////////////////////////////////////////////////
HRESULT	myExecuteCommand
	(
	IUnknown *				pUnkCommand,
	WCHAR *					pwszCommandText,
	ULONG					cPropSets,
	DBPROPSET*				rgPropSets,
	IUnknown **				ppUnkRowset
	)
{
	HRESULT					hr;
	ICommandText *			pICommandText				= NULL;
	ICommandProperties *	pICommandProperties			= NULL;

	// Set the properties on the Command object
	XCHECK_HR(hr = pUnkCommand->QueryInterface(
				IID_ICommandProperties, (void**)&pICommandProperties));
	XCHECK_HR(hr = pICommandProperties->SetProperties(cPropSets, rgPropSets));

	// Set the text for this Command, using the default command text
	// dialect. All providers that support commands must support this
	// dialect and providers that support SQL must be able to recognize
	// an SQL command as SQL when this dialect is specified
	XCHECK_HR(hr = pUnkCommand->QueryInterface(
				IID_ICommandText, (void**)&pICommandText));
	XCHECK_HR(hr = pICommandText->SetCommandText(
				DBGUID_DEFAULT,		//guidDialect
				pwszCommandText		//pwszCommandText
				));

	// And execute the Command. Note that the user could have
	// entered a non-row returning command, so we will check for
	// that and return failure to prevent the display of the
	// non-existant rowset by the caller
	XCHECK_HR(hr = pICommandText->Execute(	
				NULL,			//pUnkOuter
				IID_IRowset,	//riid
				NULL,			//pParams
				NULL,			//pcRowsAffected
				ppUnkRowset		//ppRowset
				));
		
	if( !*ppUnkRowset )
	{
		printf("\nThe command executed successfully, but did not " \
			   "return a rowset.\nNo rowset will be displayed.\n");
		hr = E_FAIL;
	}


CLEANUP:
	if( pICommandText )
		pICommandText->Release();
	if( pICommandProperties )
		pICommandProperties->Release();
	return hr;
}
