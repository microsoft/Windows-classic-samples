//
// globals.h
//
// Global variable declarations.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <assert.h>
#include "msctf.h"

void DllAddRef();
void DllRelease();

void ToggleCase(TfEditCookie ec, ITfRange *range, BOOL fIgnoreRangeEnd);
void InsertTextAtSelection(TfEditCookie ec, ITfContext *pContext, const WCHAR *pchText, ULONG cchText);

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#define CASE_LANGID    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define LANGBAR_ITEM_DESC L"Case Menu" // max 32 chars!

#define CASE_ICON_INDEX  0

#define SafeRelease(punk)       \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
    }                           \
}                   

#define SafeReleaseClear(punk)  \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
        (punk) = NULL;          \
    }                           \
}                   

extern HINSTANCE g_hInst;

extern LONG g_cRefDll;

extern CRITICAL_SECTION g_cs;

extern const CLSID c_clsidCaseTextService;

extern const GUID c_guidCaseProfile;

extern const GUID c_guidLangBarItemButton;

#endif // GLOBALS_H
