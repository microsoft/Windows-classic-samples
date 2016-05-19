
#ifndef __USERPROPS_H
#define __USERPROPS_H

BOOL MapMandatoryUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle);
BOOL MapOptionalUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle);
BOOL MapUserAttribToType(LPWSTR pwszAttrib,LPWSTR *ppwszRetAttType, LPWSTR *ppwszRetAttribMultiOrSingle);

ADSTYPE MapAdsTypeFromString(LPWSTR pwszAdsType);

ADSTYPE MapTypeToADSTYPE(LPWSTR pwszName);

 

#endif
