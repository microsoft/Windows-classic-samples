// Rename.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"

int main(int argc, char* argv[])
{
HRESULT hr;
IADsContainer *pCont=NULL;
IDispatch *pDisp=NULL;

CoInitialize(NULL);

hr = ADsGetObject(L"LDAP://CN=Users,DC=Microsoft,DC=COM", IID_IADsContainer, (void**) &pCont);
if (FAILED(hr) )
{
	return 0;
}
    
hr = pCont->MoveHere(L"LDAP://CN=Jeff Smith,CN=Users,DC=Microsoft,DC=COM",L"CN=Jeffrey Smith", &pDisp );
if ( SUCCEEDED(hr) )
{
	pDisp->Release();
}

CoUninitialize();

return 0;
}
