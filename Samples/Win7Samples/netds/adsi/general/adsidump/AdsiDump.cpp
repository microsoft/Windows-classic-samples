
/----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADSIDump.cxx
//
//
//----------------------------------------------------------------------------



#include "stdafx.h"
#include <stdio.h>
#include <string>
typedef std::basic_string<WCHAR> stringW;


//-----------------------------------------------------------------------------
//
//    Function:        CchSzLen
//
//    Description:    Returns the length of a wide string.
//
//    Return values:    INT (length of string, in characters, not counting null)
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
INT CchSzLen
(
CSZ cszString
)
    {
    Assert(cszString);

    const WCHAR * pwch = cszString;
    while (*pwch)
        pwch++;

    return (pwch - cszString);
    }


///////////////////////////////////////////////////////////////////////////////
//
//    class:        CStringException
//
//    Simple exception class.
//
//    hungarian:    strex
//
///////////////////////////////////////////////////////////////////////////////
class CStringException
{
public:
    CStringException(CSZ cszError) throw()
        {
        Assert(cszError);
        m_strError = cszError;
        }

    operator CSZ (VOID) throw()
        { return m_strError.c_str(); }

private:
    stringW m_strError;
};


// return-value checking
#define CheckHresultADs(hr) { HRESULT hr_ADSxyz = (hr);  if (!SUCCEEDED(hr_ADSxyz)) VThrowAdsError(); }
#define CheckHresult(hr,sz) { HRESULT hr_xyz = (hr);  \
    if (!SUCCEEDED(hr_xyz)) { \
        WCHAR szBuf[1024];  \
        swprintf_s(szBuf, 1024, L"Error: 0x%x  %s", hr, sz); \
    throw CStringException(szBuf);} }

//-----------------------------------------------------------------------------
//
//    Function:        VThrowAdsError
//
//    Description:    Throws a string with an ADs error description.
//
//    Return values:    none (always throws!)
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VThrowAdsError
(
VOID
)
    {
    DWORD dwCode = 0;
    const DWORD dwLen = 2048;
    WCHAR szError[dwLen];
    WCHAR szProvider[dwLen];
    if (SUCCEEDED(::ADsGetLastError(&dwCode,
                                    szError,
                                    dwLen,
                                    szProvider,
                                    dwLen)))
        {
        stringW strError;

        strError = TEXT("An ADSI error has occured.  Provider = '");
        strError += szProvider;
        strError += TEXT("'  Error message = \"");
        strError += szError;
        strError += TEXT("\"  Error code = 0x");

        TCHAR szNum[16];
        ::_ltow_s(dwCode, szNum, 16, 16);    // BASE-16
        strError += szNum;

        throw CStringException(strError.c_str());
        }
    else
        {
        throw CStringException(TEXT("Unknown ADSI error"));
        }
    }


typedef VOID (*pfnArrayCallback)(CSZ cszValue, IADs * piads, FILE * pfile);
//-----------------------------------------------------------------------------
//
//    Function:        VIterateSafeArray
//
//    Description:    Given a safe array, this function iterates through the
//                    array, and calls the specified callback for each element.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VIterateSafeArray
(
SAFEARRAY * psa,    // safe array to iterate
VARTYPE vt,            // type of array (VT_ARRAY | VT_BSTR, or something)
IADs * piads,        // directory services object
FILE * pfile,        // output file
pfnArrayCallback fnCallback    // callback function
)
    {
    Assert(psa);
    Assert(1 == ::SafeArrayGetDim(psa));
    Assert(VT_ARRAY == (vt & VT_ARRAY) && "Should be an array");
    Assert(piads);
    Assert(pfile);
    //Assert(pfnArrayCallback);

    vt -= VT_ARRAY;

    long lLBound, lUBound;
    Verify(SUCCEEDED(::SafeArrayGetLBound(psa, 1, &lLBound)));
    Verify(SUCCEEDED(::SafeArrayGetUBound(psa, 1, &lUBound)));

    switch (vt)
        {
        default:
#ifdef DEBUG
            Assert(false && "Unknown variant type");
            break;

        case VT_VARIANT:
#endif    // _DEBUG
            {
            for (long i = lLBound; i <= lUBound; i++)
                {
                CComVariant svar;
                Verify(SUCCEEDED(::SafeArrayGetElement(psa, &i, &svar)));
                if (VT_BSTR == svar.vt && svar.bstrVal)
                    fnCallback(svar.bstrVal, piads, pfile);
                }
            break;
            }

        case VT_BSTR:
            {
            for (long i = lLBound; i <= lUBound; i++)
                {
                CComBSTR sbstr;
                Verify(SUCCEEDED(::SafeArrayGetElement(psa, &i, &sbstr)));
                if (sbstr)
                    fnCallback(sbstr, piads, pfile);
                }
            break;
            }
        }
    }


//-----------------------------------------------------------------------------
//
//    Function:        VDumpArray
//
//    Description:    Given a value, write it out to the provided text file.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VDumpArray
(
CSZ cszValue,    // value
IADs * piads,    // object to query (not used for this callback)
FILE * pfile    // output file
)
    {
    Assert(cszValue);
    Assert(pfile);

    UNUSED(piads);

    ::fprintf(pfile, "  \"%S\"", cszValue);
    }


//-----------------------------------------------------------------------------
//
//    Function:        VDumpAttribute
//
//    Description:    Given an object and an attribute name, dumps the attribute
//                    to a text file.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VDumpAttribute
(
CSZ cszAttribute,    // name of attribute
IADs * piads,        // object to query
FILE * pfile        // output file
)
    {
    Assert(piads);
    Assert(cszAttribute);
    Assert(pfile);

    CComVariant svar;
    CComBSTR sbstr(cszAttribute);
    HRESULT hr = S_OK;
    hr = piads->Get(sbstr, &svar);
    if (E_ADS_PROPERTY_NOT_FOUND != hr) CheckHresult(hr, TEXT("Failed to Get value of an attribute"));
    
    ::fprintf(pfile, "\n    %S : ", (SZ) sbstr);

    switch (svar.vt)
        {
        default:
            ::fprintf(pfile, "(Unknown variant type)");
            break;

        case VT_BSTR:
            ::fprintf(pfile, "(BSTR) \"%S\"", svar.bstrVal);
            break;

        case (VT_ARRAY | VT_BSTR):
        case (VT_ARRAY | VT_VARIANT):
            {
            if (svar.parray)
                {
                ::fprintf(pfile, "(ARRAY) [");
                ::VIterateSafeArray(svar.parray,
                                    svar.vt,
                                    piads,
                                    pfile,
                                    VDumpArray);
                ::fprintf(pfile, "]");
                }
            break;
            }

        case VT_I1:
        case VT_I4:
        case VT_UI1:
        case VT_UI2:
        case VT_INT:
        case VT_UINT:
            fprintf(pfile, "(INT) %d", svar.intVal);
            break;

        case VT_I8:
        case VT_UI8:
            ::fprintf(pfile, "(INT8) %ld", svar.uintVal);
            break;

        case VT_BOOL:
            ::fprintf(pfile, "(BOOL) %s", (svar.boolVal) ? "TRUE" : "FALSE");
            break;
        }
    }


//-----------------------------------------------------------------------------
//
//    Function:        VDumpObject
//
//    Description:    Given an IADs object, its parent and an output file, this
//                    routine writes out all of the attributes it can find.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VDumpObject
(
CSZ cszParent,        // name of parent (can be null)
IADs * piads,        // object to dump
FILE * pfile        // output file
)
    {
    Assert(piads);
    Assert(pfile);

    // pull down information from server
    CheckHresultADs(piads->GetInfo());

    // start dumping
    ::fprintf(pfile, "\n\n=======================================================\n");

    // name
    CComBSTR sbstrName;
    CheckHresultADs(piads->get_Name(&sbstrName));
    ::fprintf(pfile, "\n***  %S  ***", (SZ) sbstrName);
    ::wprintf(L"\n  %s...", sbstrName);

    // parent
    if (cszParent)
        ::fprintf(pfile, "\n  Child of %S", cszParent);
    else
        ::fprintf(pfile, "\n  ROOT OBJECT");

    // full path
    CComBSTR sbstrPath;
    CheckHresultADs(piads->get_ADsPath(&sbstrPath));
    ::fprintf(pfile, "\n  Full ADs path: \"%S\"", (SZ) sbstrPath);

    // class
    CComBSTR sbstrClass;
    CheckHresultADs(piads->get_Class(&sbstrClass));
    ::fprintf(pfile, "\n  Class: %S", (SZ) sbstrClass);

    // schema
    CComBSTR sbstrSchema;
    CheckHresultADs(piads->get_Schema(&sbstrSchema));
    ::fprintf(pfile, "\n  Schema: %S", (SZ) sbstrSchema);

    // attributes
    ::fprintf(pfile, "\n  Attributes -------------------");

    // open the class, get all the attributes
    CComPtr<IADsClass> srpiaclass;
    CheckHresultADs(::ADsGetObject(sbstrSchema,
                                     IID_IADsClass,
                                     (PVOID *) &srpiaclass));
    Assert(srpiaclass);

    VARIANT var;
    var.vt = VT_EMPTY;
    var.parray = NULL;
    CheckHresultADs(srpiaclass->get_MandatoryProperties(&var));
    if (VT_ARRAY == (var.vt & VT_ARRAY))
        {
        Assert(var.parray);
        ::VIterateSafeArray(var.parray, var.vt, piads, pfile, VDumpAttribute);
        ::SafeArrayDestroy(var.parray);
        }

    var.vt = VT_EMPTY;
    var.parray = NULL;
    CheckHresultADs(srpiaclass->get_OptionalProperties(&var));
    if (VT_ARRAY == (var.vt & VT_ARRAY))
        {
        Assert(var.parray);
        ::VIterateSafeArray(var.parray, var.vt, piads, pfile, VDumpAttribute);
        ::SafeArrayDestroy(var.parray);
        }

    ::fprintf(pfile, "\n");
    }


//-----------------------------------------------------------------------------
//
//    Function:        VDumpTree
//
//    Description:    Recursively dumps all of the children of this node.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
VOID VDumpTree
(
CSZ cszParent,    // name of parent object
IADs * piads,    // object to dump (dump its children as well)
FILE * pfile    // file to which we dump
)
    {
    Assert(piads);
    Assert(pfile);

    // dump this object
    ::VDumpObject(cszParent, piads, pfile);

    // get name
    CComBSTR sbstrName;
    CheckHresultADs(piads->get_Name(&sbstrName));

    // iterate through children, and dump them
    CComPtr<IADsContainer> srpiac;
    HRESULT hr = S_OK;
    CheckHresult(piads->QueryInterface(IID_IADsContainer,(PVOID *) &srpiac),
                    TEXT("Failed to QI to IADsContainer interface"));

    if (srpiac)
        {
        CComPtr<IEnumVARIANT> srpiev;
        CheckHresult(srpiac->get__NewEnum((IUnknown **) &srpiev), TEXT("Failed to get_NewEnum"));
            
        if (srpiev)
            {
            while (true)
                {
                CComVariant svar;
                ULONG cFetched = 0;
                hr = srpiev->Next(1, &svar, &cFetched);
                if (S_OK != hr)
                    break;

                Assert(1 == cFetched);
                Assert(VT_DISPATCH == svar.vt);
                Assert(svar.pdispVal);

                CComPtr<IADs> srpiads;
                CheckHresultADs(svar.pdispVal->QueryInterface(IID_IADs,
                                (PVOID *) &srpiads));
                Assert(srpiads);

                // dump the object and its children
                ::VDumpTree(sbstrName, srpiads, pfile);
                }
            }
        }
    }


//-----------------------------------------------------------------------------
//
//    Function:        main
//
//    Description:    Entry point of the executable.  Reads the command-line
//                    paramters, and starts the dump.
//
//    Return values:    none
//
//    Threadsafe:        no
//
//-----------------------------------------------------------------------------
void main
(
int argc,
char* argv[]
)
    {
    FILE * pfile = NULL;

    try
        {

        // initialize COM
        CheckHresult(::CoInitialize(NULL), TEXT("Could not initialize COM"));

        // validate input
        if (3 != argc)
            {
            MessageBox(NULL,
                       TEXT("Usage is AdsiDump <ADSI path> <Output file>"),
                       TEXT("Usage"),
                       MB_OK);
            return;
            }

        // convert LDAP path to ANSI
        UINT cchLdapPath = ::strlen(argv[1]);
        SZ szLdapPath = (SZ) alloca((cchLdapPath + 1) * sizeof(WCHAR));
        ::MultiByteToWideChar(CP_ACP,
                              0,
                              argv[1],
                              -1,
                              szLdapPath,
                              cchLdapPath + 1);

        // open the file
		errno_t status = 0;
        status = ::fopen_s(&pfile, argv[2], "w");
        if (status)
            {
            ::MessageBox(NULL,
                         TEXT("Could not open output file for writing"),
                         TEXT("Error"),
                         MB_OK);
			::CoUninitialize();
            return;
            }

        // open the object
        CComPtr<IADs> srpiads;
        CheckHresult(::ADsGetObject(szLdapPath,
                                    IID_IADs,
                                    (PVOID *) &srpiads),
                     TEXT("Could not open LDAP path"));
        Assert(srpiads);

        // dump the tree
        ::VDumpTree(NULL, srpiads, pfile);

        }
    catch (CStringException& rstrex)
        {
        ::MessageBox(NULL, rstrex, TEXT("Error"), MB_OK);
        }

    // close the file
    if (pfile)
        ::fclose(pfile);

	::CoUninitialize();

    ::printf("\n");
    }
