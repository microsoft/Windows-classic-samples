// Delete.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"

int main(int argc, char* argv[])
{
	HRESULT hr;
	IADsContainer *pCont=NULL;

	CoInitialize(NULL);

	hr = ADsGetObject(L"WinNT://INDEPENDENCE", IID_IADsContainer, (void**) &pCont);
	if ( !SUCCEEDED(hr) )
	{
		return 0;
	}

	///////////////////////////////////////////////////
	// Using IADsContainer::Delete to delete a user
	//////////////////////////////////////////////////
	hr = pCont->Delete(L"user", L"AliceW");
	pCont->Release();

	/////////////////////////////////////////////////////////////
	// Using IDirectoryObject::DeleteDSObject to delete a user
	//////////////////////////////////////////////////////////////
	IDirectoryObject *pDirObject=NULL;

	hr = ADsGetObject(L"LDAP://OU=testOU,DC=testDom1,DC=testDom2,DC=microsoft,DC=com",
                      IID_IDirectoryObject, (void**) &pDirObject );

	if ( SUCCEEDED(hr) )
	{
		hr = pDirObject->DeleteDSObject(L"CN=Mike Smith");
		pDirObject->Release();
	}
	

	CoUninitialize();

	return 0;
}
