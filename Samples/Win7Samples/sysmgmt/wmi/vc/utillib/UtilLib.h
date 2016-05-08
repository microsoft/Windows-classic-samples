// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  utillib.h
//
// Description:
//    Set of sample routines
//
// History:
//
// **************************************************************************

#define MAXITOA  19
#define FWPRINTF myFWPrintf
#define CLASSPROP L"__CLASS"
#define SERVERPROP L"__SERVER"
#define PATHPROP L"__PATH"
#define NAMESPACEPROP L"__NAMESPACE"
#define SYSTEMCLASS L"__SystemClass"
#define RELPATHPROP L"__RELPATH"
#define NAMEPROP L"Name"
#define CIMTYPEQUAL L"CIMTYPE"
#define KEYQUAL L"key"
#define SYSTEMPREFIX L"__"
#define CVTFAILED L"WideCharToMultiByte failed\n"
#define RELEASE(a) if (a) { (a)->Release(); (a)=NULL;}
#define BLOCKSIZE (32 * sizeof(WCHAR))
#define CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop  (this size stolen from cvt.h in c runtime library) */
#define PAGESIZE 4096
#define ERROR_MODE_PRINTFIELDS 0
#define ERROR_MODE_PRINTMOF 1
#define UNREFERENCED(x)

#define STRSAFE_NO_DEPRECATE

#include <comdef.h>
#include <strsafe.h>

_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemCallResult, __uuidof(IWbemCallResult));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemContext, __uuidof(IWbemContext));
_COM_SMARTPTR_TYPEDEF(IWbemObjectAccess, __uuidof(IWbemObjectAccess));
_COM_SMARTPTR_TYPEDEF(IWbemObjectSink, __uuidof(IWbemObjectSink));
_COM_SMARTPTR_TYPEDEF(IWbemQualifierSet, __uuidof(IWbemQualifierSet));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemStatusCodeText, __uuidof(IWbemStatusCodeText));

char *cvt(WCHAR *x, char **y);
double difftime(struct _timeb finish, struct _timeb start);
int myFWPrintf(FILE *f, WCHAR *fmt, ...);
const WCHAR *TypeToString(VARIANT *p);
const WCHAR *TypeToString(VARTYPE v);
const WCHAR *TypeToString(CIMTYPE v);
WCHAR *ValueToString(CIMTYPE dwType, VARIANT *pValue, WCHAR **pbuf, WCHAR *fnHandler(VARIANT *pv) = NULL);
BSTR WbemErrorString(SCODE sc);
void PrintErrorAndExit(char *pszFailureReason, SCODE sc, DWORD dwMode);
void PrintErrorAndAsk(char *pszFailureReason, SCODE sc, DWORD dwMode);
void PrintError(char *pszFailureReason, SCODE sc, DWORD dwMode);

class MyString {
private:
   WCHAR *pwszString;

public:
   MyString::MyString() 
   {
	   pwszString = (WCHAR *)calloc(1, sizeof(WCHAR));
   }
   MyString::~MyString() 
   {
	   free(pwszString);
   }
   WCHAR *GetString() {return pwszString;}
   void Empty() {free(pwszString);pwszString = (WCHAR *)calloc(1, sizeof(WCHAR));}
   WCHAR *GetCloneString()
   {
      WCHAR *buf = NULL;
	  if (pwszString)
	  {
		  int ibufSize = (wcslen(pwszString) + 1) * sizeof(WCHAR);
		  buf = (WCHAR *)malloc(ibufSize);
		  if (buf)
			StringCbCopyW(buf, ibufSize, pwszString);
	  }
      return buf;
   }
   const MyString& operator+=(const WCHAR *arg1) 
      {
		 size_t iHave;
         size_t iNeed;
		 WCHAR *pwsString;
         if (arg1)
         {
             iHave = wcslen(pwszString);
             iNeed = wcslen(arg1);
			 pwsString =pwszString;
             pwszString = (WCHAR *)realloc(pwszString, (iHave + iNeed + 1) * sizeof(WCHAR));
			 if (pwszString)
			  {
			     StringCbCatW(&pwszString[iHave], 
							((iHave + iNeed + 1) * sizeof(WCHAR)) - iHave,
							arg1);
			  }
			  else
			  {
				pwszString = pwsString;
			  }			

         }
         return *this;
      }

};

