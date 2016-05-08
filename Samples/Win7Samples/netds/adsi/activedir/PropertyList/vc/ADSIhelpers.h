


#ifndef ADSIHELLPERS_H
#define ADSIHELLPERS_H

#include <activeds.h>
#include <adsiid.h>


// Function declarations
void			PrintIADSObject(IADs * pIADs);
void			CheckHRESULT(HRESULT hr, const char *pszCause);
const char *	GetVariantStyle(VARTYPE vt);
HRESULT			GetIADsPropertyValueAsBSTR(BSTR * pbsRet,IADsPropertyEntry *pAdsEntry, IADsPropertyValue * pAdsPV);


// Macros

// Simple Macro to simplyfy reading a bitflag
#define HAS_BIT_STYLE(val, style) \
((val & style) == style)


#endif
