//+-------------------------------------------------------------------------
//
//      Copyright (C) Microsoft Corporation. All rights reserved.
//
//      File:           msistuff.cpp
//
//--------------------------------------------------------------------------

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\setup.exe\common.h"
#include "strsafe.h"

//=======================================================
// command line options and matching properties
// ORDER is IMPORTANT!
//=======================================================
TCHAR rgszCommandOptions[]= TEXT("udnviwpmo");
//////////////////////////        012345678

TCHAR* rgszResName[] = {
/* BaseURL                 */ ISETUPPROPNAME_BASEURL, 
/* Msi Package             */ ISETUPPROPNAME_DATABASE,
/* Product Name            */ ISETUPPROPNAME_PRODUCTNAME,
/* Minimum Msi Version     */ ISETUPPROPNAME_MINIMUM_MSI,
/* Update URL Location     */ ISETUPPROPNAME_UPDATELOCATION,
/* InstMsiW                */ ISETUPPROPNAME_UPDATE,
/* Properties              */ ISETUPPROPNAME_PROPERTIES,
/* Patch                   */ ISETUPPROPNAME_PATCH,
/* Operation               */ ISETUPPROPNAME_OPERATION
};
const int cStandardProperties = sizeof(rgszResName)/sizeof(TCHAR*);

TCHAR rgchResSwitch[] ={TEXT('u'),TEXT('d'),TEXT('n'),TEXT('v'),TEXT('i'),TEXT('w'),TEXT('p'),TEXT('m'),TEXT('o')};

const DWORD lcidLOCALE_INVARIANT = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

//=======================================================
// special options
//=======================================================
const TCHAR chProperties      = TEXT('p');
const TCHAR chMinMsiVer       = TEXT('v');
const TCHAR chOperation       = TEXT('o');
const TCHAR szInstall[]         = TEXT("INSTALL");
const TCHAR szInstallUpd[]      = TEXT("INSTALLUPD");
const TCHAR szMinPatch[]        = TEXT("MINPATCH");
const TCHAR szMajPatch[]        = TEXT("MAJPATCH");
const int   iMinMsiAllowedVer = 150;



//=======================================================
// function prototypes
//=======================================================

void           DisplayHelp();
bool           ParseCommandLine(__in_opt LPTSTR lpCommandLine);
TCHAR          SkipWhiteSpace(__inout_opt TCHAR*& rpch);
bool           SkipValue(__inout_opt TCHAR*& rpch);
bool           RemoveQuotes(LPCTSTR lpOriginal, __out_ecount(cchStripped) LPTSTR lpStripped, DWORD cchStripped);
bool           DisplayResources(LPCTSTR lpExecutable);
bool           DisplayInstallResource(HMODULE hExeModule, LPCTSTR lpszType, LPCTSTR lpszName);
BOOL  CALLBACK EnumResNamesProc(HMODULE hExeModule, LPCTSTR lpszType, __in LPTSTR lpszName, LONG_PTR lParam);

//=======================================================
// global constants
//=======================================================
int g_cResources = 0; // count of resources in setup.exe; information purposes only

//_____________________________________________________________________________________________________
//
// main 
//_____________________________________________________________________________________________________

extern "C" int __cdecl _tmain(int argc, __in_ecount(argc) TCHAR* argv[])
{
        if (argc <= 2)
        {
                if (1 == argc
                    || 0 == lstrcmp(argv[1], TEXT("/?"))
                    || 0 == lstrcmp(argv[1], TEXT("-?")))
                {
                        //
                        // display help

                        DisplayHelp();
                }
                else
                {
                        //
                        // display setup resources

                        TCHAR szExecutable[MAX_PATH] = {0};
                        if (FAILED(StringCchCopy(szExecutable, sizeof(szExecutable)/sizeof(szExecutable[0]), argv[1])))
                            return -1;
                        if (!DisplayResources(szExecutable))
                                return -1;
                }

        }
        else
        {
                //
                // set resource properties

                TCHAR *szCommandLine = GetCommandLine();
                if (!ParseCommandLine(szCommandLine))
                        return -1;
        }

        return 0;
}

//_____________________________________________________________________________________________________
//
// DisplayHelp
//_____________________________________________________________________________________________________

void DisplayHelp()
{
        TCHAR szHelp[] =
                                        TEXT("Copyright (c) Microsoft Corporation. All rights reserved.\n")
                                        TEXT("\n")
                                        TEXT("MsiStuff will display or update the resources \n")
                                        TEXT(" in the setup.exe boot strap executable\n")
                                        TEXT("\n")
                                        TEXT("[MsiStuff Command Line Syntax]\n")
                                        TEXT(" Display Properties->> msistuff setup.exe \n")
                                        TEXT(" Set Properties    ->> msistuff setup.exe option {data} ... \n")
                                        TEXT("\n")
                                        TEXT("[MsiStuff Options -- Multiple specifications are allowed]\n")
                                        TEXT(" BaseURL                             - /u {value} \n")
                                        TEXT(" Msi                                 - /d {value} \n")
                                        TEXT(" Product Name                        - /n {value} \n")
                                        TEXT(" Minimum Msi Version                 - /v {value} \n")
                                        TEXT(" Update URL Location                 - /i {value} \n")
                                        TEXT(" Windows Installer Update            - /w {value} \n")
                                        TEXT(" Patch                               - /m {value} \n")
                                        TEXT(" Operation                           - /o {value} \n")
                                        TEXT(" Properties (PROPERTY=VALUE strings) - /p {value} \n")
                                        TEXT("\n")
                                        TEXT("If an option is specified multiple times, the last one wins\n")
                                        TEXT("\n")
                                        TEXT("/p must be last on the command line.  The remainder of\n")
                                        TEXT("the command line is considered a part of the {value}\n")
                                        TEXT("This also means that /p cannot be specified multiple times\n");
        _tprintf(TEXT("%s"), szHelp);
}

//_____________________________________________________________________________________________________
//
// ParseCommandLine
//
//       If a property has a value that contains spaces, the value must be enclosed in quotation marks
//_____________________________________________________________________________________________________

bool ParseCommandLine(__in_opt LPTSTR lpCommandLine)
{
        if (!lpCommandLine)
            return false;

        TCHAR szSetupEXE[MAX_PATH] = {0};
        TCHAR szFullPath[2*MAX_PATH] = {0};

        TCHAR  chNextCommand;
        TCHAR *pchCommandLine = lpCommandLine;

        // skip over module name and subsequent white space
        if (!SkipValue(pchCommandLine))
            return false;
        chNextCommand = SkipWhiteSpace(pchCommandLine);
        
        TCHAR* pchCommandData = pchCommandLine;
        if (!SkipValue(pchCommandLine))
            return false;
        if (!RemoveQuotes(pchCommandData, szSetupEXE, sizeof(szSetupEXE)/sizeof(szSetupEXE[0])))
            return false;

        // handle possibility of a relative path
        LPTSTR lpszFilePart = 0;
        int cchFullPath = 0;
        if (0 == (cchFullPath = GetFullPathName(szSetupEXE, sizeof(szFullPath)/sizeof(TCHAR), szFullPath, &lpszFilePart)))
        {
                // error
                _tprintf(TEXT("Unable to obtain fullpath for %s\n"), szSetupEXE);
                return false;
        }
        else if (cchFullPath > sizeof(szFullPath)/sizeof(TCHAR))
        {
            // szFullPath buffer is too small; we could resize, but we'll simply fail here
            _tprintf(TEXT("szFullPath buffer is insufficiently sized for obtaining the full path for %s\n"), szSetupEXE);
            return false;
        }

        _tprintf(TEXT("\nModifying setup properties in:\n\t<%s>\n\n"), szFullPath);

        // make sure the EXE is not already loaded,in use, or read-only
        HANDLE hInUse = CreateFile(szFullPath, GENERIC_WRITE, (DWORD)0, (LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
        if (INVALID_HANDLE_VALUE == hInUse)
        {
                // error
                _tprintf(TEXT("Unable to obtain file handle for %s.  The file probably does not exist, is marked read-only, or is in use.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }
        CloseHandle(hInUse);

        // begin update resource
        HANDLE hUpdate = BeginUpdateResource(szFullPath, /* bDeleteExistingResources = */ FALSE);
        if ( !hUpdate )
        {
                // error
                _tprintf(TEXT("Unable to update resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        while ((chNextCommand = SkipWhiteSpace(pchCommandLine)) != 0)
        {
                if (chNextCommand == TEXT('/') || chNextCommand == TEXT('-'))
                {
                        TCHAR *szOption = pchCommandLine++;  // save for error msg
                        TCHAR  chOption = (TCHAR)(*pchCommandLine++ | 0x20); // lower case flag
                        chNextCommand = SkipWhiteSpace(pchCommandLine);
                        pchCommandData = pchCommandLine;

                        const TCHAR* pchOptions;
                        for (pchOptions = rgszCommandOptions; *pchOptions; pchOptions++)
                        {
                                if (*pchOptions == chOption)
                                        break;
                        }

                        if (*pchOptions)
                        {
                                bool fSkipValue   = true; // whether or not to look for next option in command line; (true = look)
                                bool fDeleteValue = false;// whether to delete the value

                                // option is recognized
                                const TCHAR chIndex = (TCHAR)(pchOptions - rgszCommandOptions);
                                if (chIndex >= cStandardProperties)
                                {
                                        // error
                                        _tprintf(TEXT("Invalid index (chIndex = %d, chOption = %c)!!!\n"), chIndex, chOption);
                                        return false;
                                }

                                if (chOption == chProperties)
                                {
                                        fSkipValue = false;
                                        // special for /p -- remainder of command line is part of property
                                        TCHAR chNext = *pchCommandData;
                                        if (chNext == 0 || chNext == TEXT('/') || chNext == TEXT('-'))
                                        {
                                                // no value present
                                                fDeleteValue = true;
                                        }
                                        else
                                        {
                                                // set value to remainder of command line, i.e. all of pchCommandData
                                                // with enclosing quotes stripped -- we do this by telling the command
                                                // line processor to not attempt to look for other options on the
                                                // command line; the remainder is part of this property
                                                fSkipValue = false;
                                        }
                                }

                                if (fSkipValue)
                                        fDeleteValue = (SkipValue(pchCommandLine)) ? false : true;

                                if (fDeleteValue)
                                {
                                        // delete value and reset to ensure resource is properly removed
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), NULL, 0)
                                                || !EndUpdateResource(hUpdate, /* fDiscard = */ FALSE)
                                                || (!(hUpdate = BeginUpdateResource(szFullPath, /* bDeleteExistingResources = */ FALSE))))
                                        {
                                                // error
                                                _tprintf(TEXT("Unable to delete resource %s in %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, GetLastError());
                                                return false;
                                        }
                                        _tprintf(TEXT("Removing '%s' . . .\n"), rgszResName[chIndex]);
                                }
                                else
                                {
                                        TCHAR szValueBuf[1024] = {0};
                                        if (!RemoveQuotes(pchCommandData, szValueBuf, sizeof(szValueBuf)/sizeof(szValueBuf[0])))
                                            return false;

                                        if (chOption == chMinMsiVer && (_ttoi(szValueBuf) < iMinMsiAllowedVer))
                                        {
                                                // extra validation: must be >= iMinAllowedVer
                                                _tprintf(TEXT("Skipping option %c with data %s. Data value must be >= %d. . .\n"), chOption, szValueBuf, iMinMsiAllowedVer);
                                                continue;
                                        }

                                        if (chOption == chOperation
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szInstall, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szInstallUpd, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szMinPatch, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szMajPatch, -1))
                                        {
                                            // extra validation: must be one of those values
                                            _tprintf(TEXT("Skipping option %c with data %s. Data value must be INSTALL, INSTALLUPD, MINPATCH, or MAJPATCH...\n"), chOption, szValueBuf);
                                            continue;
                                        }

                                        // update value -- this is our own custom resource, so to make it easier on ourselves, we will pack in the NULL as well
                                #ifdef UNICODE
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), szValueBuf, (lstrlen(szValueBuf) + 1)*sizeof(TCHAR)))
                                        {
                                                // error
                                                _tprintf(TEXT("Unable to update resource %s in %s with value %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, szValueBuf, GetLastError());
                                                return false;
                                        }
                                #else // !UNICODE
                                        // must convert value to Unicode
                                        int cchWide = MultiByteToWideChar(CP_ACP, 0, szValueBuf, -1, NULL, 0);
                                        WCHAR* wszValueBuf = new WCHAR[cchWide];
                                        if (!wszValueBuf)
                                            return false; // out of memory
                                        ZeroMemory(wszValueBuf, cchWide*sizeof(WCHAR));
                                        if (0 == MultiByteToWideChar(CP_ACP, 0, szValueBuf, -1, wszValueBuf, cchWide))
                                        {
                                            // error
                                            _tprintf(TEXT("Unable to convert value to Unicode. LastError = %d\n"), GetLastError());
                                            delete [] wszValueBuf;
                                            return false;
                                        }
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), wszValueBuf, (lstrlenW(wszValueBuf) +1)*sizeof(WCHAR)))
                                        {
                                                // error
                                                _tprintf(TEXT("Unable to update resource %s in %s with value %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, szValueBuf, GetLastError());
                                                delete [] wszValueBuf;
                                                return false;
                                        }
                                        delete [] wszValueBuf;
                                #endif // UNICODE
                                        _tprintf(TEXT("Setting '%s' to '%s' . . .\n"), rgszResName[chIndex], szValueBuf);
                                        if (!fSkipValue)
                                                break; // done processing
                                }
                        }
                        else
                        {
                                // invalid option
                                _tprintf(TEXT("Skipping invalid option %c . . .\n"), chOption);
                                SkipValue(pchCommandLine);
                                continue;
                        }
                }
                else
                {
                        // error
                        _tprintf(TEXT("Switch is missing\n"));
                        return false;
                }
        }

        // persist changes
        if (!EndUpdateResource(hUpdate, /* fDiscard = */ FALSE))
        {
                // error
                _tprintf(TEXT("Unable to update resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        return true;
}

//_____________________________________________________________________________________________________
//
// SkipWhiteSpace
//
//       Skips whitespace in the string and returns next non-tab non-whitespace charcter 
//_____________________________________________________________________________________________________

TCHAR SkipWhiteSpace(__inout_opt TCHAR*& rpch)
{
        TCHAR ch = 0;
        if (rpch)
        {
            for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
                    ;
        }
        return ch;
}

//_____________________________________________________________________________________________________
//
// SkipValue
//
//       Skips over the value of a switch and returns true if a value was present. Handles value enclosed
//       in quotation marks
//_____________________________________________________________________________________________________

bool SkipValue(__inout_opt TCHAR*& rpch)
{
        if (!rpch)
            return false;

        TCHAR ch = *rpch;
        if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
                return false;   // no value present
        for (int i = 0; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch = CharNext(rpch), i++)
        {
                if (0 == i && *rpch == TEXT('"'))
                {
                        rpch++; // for '"'
                        for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch++)
                                ;
                        ch = *(++rpch);
                        break;
                }
        }
        if (ch != 0)
        {
                TCHAR* pch = rpch;
                rpch = CharNext(rpch);
                *pch = 0;
        }
        return true;
}

//_____________________________________________________________________________________________________
//
// RemoveQuotes
//
//  Removes enclosing quotation marks for the value. Returns copy of original string
//  if no enclosing quotation mark at front of string.  Returns false on error.
//_____________________________________________________________________________________________________

bool RemoveQuotes(LPCTSTR lpOriginal, __out_ecount(cchStripped) LPTSTR lpStripped, DWORD cchStripped)
{
        if (!lpOriginal)
            return false;

        bool fEnclosedInQuotes = false;

        const TCHAR *pchOrig = lpOriginal;

        // check for "
        if (*pchOrig == TEXT('"'))
        {
                fEnclosedInQuotes = true;
                pchOrig++;
        }
        
        if (FAILED(StringCchCopy(lpStripped, cchStripped, pchOrig)))
            return false;

        if (!fEnclosedInQuotes)
                return true;

        TCHAR *pch = lpStripped + lstrlen(lpStripped) + 1; // start at NULL

        pch = CharPrev(lpStripped, pch);

        // look for trailing "
        while (pch != lpStripped)
        {
                if (*pch == TEXT('"'))
                {
                        *pch = 0;
                        break; // only care about trailing ", and not quotes in middle
                }
                pch = CharPrev(lpStripped, pch);
        }

        return true;
}

//_____________________________________________________________________________________________________
//
// DisplayInstallResource
//_____________________________________________________________________________________________________

bool DisplayInstallResource(HMODULE hExeModule, LPCTSTR lpszType, LPCTSTR lpszName)
{
        HRSRC   hRsrc   = 0;
        HGLOBAL hGlobal = 0;
        WCHAR   *pch    = 0;

        if ((hRsrc = FindResource(hExeModule, lpszName, lpszType)) != 0
                && (hGlobal = LoadResource(hExeModule, hRsrc)) != 0
                && (pch = (WCHAR*)LockResource(hGlobal)) != 0)
        {
                // resource exists
                g_cResources++;

                if (!pch)
                        _tprintf(TEXT("%s = NULL\n"), lpszName);
                else
                {
                #ifdef UNICODE
                        _tprintf(TEXT("%s = %s\n"), lpszName, pch);
                #else // !UNICODE
                        unsigned int cch = WideCharToMultiByte(CP_ACP, 0, pch, -1, NULL, 0, NULL, NULL);
                        char *szValue = new char[cch];
                        if (!szValue)
                        {
                                _tprintf(TEXT("Error -- out of memory\n"));
                                return false;
                        }
                        ZeroMemory(szValue, cch * sizeof(char));

                        if (0 == WideCharToMultiByte(CP_ACP, 0, pch, -1, szValue, cch, NULL, NULL))
                        {
                            // error -- could not convert
                            _tprintf(TEXT("Unable to convert value.  LastError = %d\n"), GetLastError());
                            delete [] szValue;
                            return false;
                        }

                        _tprintf(TEXT("%s = %s\n"), lpszName, szValue);
                        delete [] szValue;
                #endif // UNICODE
                }
        }

        return true;
}

//_____________________________________________________________________________________________________
//
// EnumResNamesProc
//_____________________________________________________________________________________________________

BOOL CALLBACK EnumResNamesProc(HMODULE hExeModule, LPCTSTR lpszType, __in LPTSTR lpszName, LONG_PTR /*lParam*/)
{
        if (!DisplayInstallResource(hExeModule, lpszType, lpszName))
                return FALSE;
        return TRUE;
}

//_____________________________________________________________________________________________________
//
// DisplayResources
//_____________________________________________________________________________________________________

bool DisplayResources(LPCTSTR szExecutable)
{
        // handle possibility of a relative path
        TCHAR szFullPath[2*MAX_PATH] = {0};
        LPTSTR lpszFilePart = 0;
        int cchFullPath = 0;
        if (0 == (cchFullPath = GetFullPathName(szExecutable, sizeof(szFullPath)/sizeof(TCHAR), szFullPath, &lpszFilePart)))
        {
                // error
                _tprintf(TEXT("Unable to obtain full file path for %s.  LastError = %d\n"), szExecutable, GetLastError());
                return false;
        }
        else if (cchFullPath > sizeof(szFullPath)/sizeof(TCHAR))
        {
            // szFullPath buffer is too small; we could resize, but we'll simply fail here
            _tprintf(TEXT("szFullPath buffer is insufficiently sized for obtaining the full path for %s\n"), szExecutable);
            return false;
        }

        _tprintf(TEXT("\n<%s>\n\n"), szFullPath);

        HMODULE hExeModule = LoadLibraryEx(szFullPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (NULL == hExeModule)
        {
                // error
                _tprintf(TEXT("Unable to load %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        // only enumerate on RT_INSTALL_PROPERTY type
        if (!EnumResourceNames(hExeModule, RT_INSTALL_PROPERTY, EnumResNamesProc, (LPARAM)0))
        {
                DWORD dwLastErr = GetLastError();
                if (ERROR_RESOURCE_TYPE_NOT_FOUND == dwLastErr)
                        _tprintf(TEXT("No RT_INSTALL_PROPERTY resources were found.\n"));
                else if (ERROR_RESOURCE_DATA_NOT_FOUND == dwLastErr)
                        _tprintf(TEXT("This file does not have a resource section.\n"));
                else
                {
                        // error
                        _tprintf(TEXT("Failed to enumerate all resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                        FreeLibrary(hExeModule);
                        return false;
                }
        }

        if (g_cResources)
        {
                if (1 == g_cResources)
                        _tprintf(TEXT("\n\n 1 RT_INSTALL_PROPERTY resource was found.\n"));
                else
                        _tprintf(TEXT("\n\n %d RT_INSTALL_PROPERTY resources were found.\n"), g_cResources);
        }

        if (!FreeLibrary(hExeModule))
        {
                // error
                _tprintf(TEXT("Failed to unload %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        return true;
}
