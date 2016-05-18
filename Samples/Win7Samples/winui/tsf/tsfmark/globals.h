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

LONG DllAddRef();
LONG DllRelease();

BOOL AdviseSink(IUnknown *pSource, IUnknown *pSink, REFIID riid, DWORD *pdwCookie);
void UnadviseSink(IUnknown *pSource, DWORD *pdwCookie);

BOOL AdviseSingleSink(TfClientId tfClientId, IUnknown *pSource, IUnknown *pSink, REFIID riid);
void UnadviseSingleSink(TfClientId tfClientId, IUnknown *pSource, REFIID riid);

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);

BOOL IsEqualUnknown(IUnknown *interface1, IUnknown *interface2);

#define MARK_LANGID    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define LANGBAR_ITEM_DESC L"Mark Menu" // max 32 chars!

#define MARK_ICON_INDEX  0

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

extern const CLSID c_clsidMarkTextService;
extern const GUID c_guidMarkProfile;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidMarkDisplayAttribute;
extern const GUID c_guidMarkContextCompartment;
extern const GUID c_guidMarkGlobalCompartment;
extern const GUID c_guidCaseProperty;
extern const GUID c_guidCustomProperty;

#endif // GLOBALS_H
