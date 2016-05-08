




#ifndef __CREATEUSERHELPERS_H
#define __CREATEUSERHELPERS_H


void PrintIADSObject(IADs * pIADs);
void CheckADHRESULT(HRESULT passedhr,LPOLESTR pwReason);
WCHAR * CrackADError(HRESULT hResult);



#endif