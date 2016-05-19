// Parent.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	HRESULT hr;

	CoInitialize(NULL);

	/////////////////////////////////////////
	//Bind to an object
	/////////////////////////////////////////
	IADs *pADs = NULL;
	hr = ADsGetObject(L"WinNT://INDEPENDENCE/JJohnson", IID_IADs, (void**) &pADs );
	if (!SUCCEEDED(hr) )
	{
		return hr;
	}

	BSTR bstrParent;
	IADs *pParent=NULL;

	//////////////////////////////
	// Get the ADs Parent's Path
	//////////////////////////////
	pADs->get_Parent(&bstrParent);
	pADs->Release();

	////////////////////////////////
	// Bind to the Parent
	////////////////////////////////
	hr = ADsGetObject( bstrParent, IID_IADs, (void**) &pParent );
	SysFreeString(bstrParent);


	if (SUCCEEDED(hr) )
	{
		// do something with pParent...
		pParent->Release();
	}


	CoUninitialize();
	return 0;
}
