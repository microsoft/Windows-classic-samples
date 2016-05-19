#pragma once

#include "logging.h"


// Common macros

// SAFE_RELEASE template.
// Releases a COM pointer if the pointer is not NULL, and sets the pointer to NULL.

#ifndef SAFE_RELEASE
template <class T>
inline void SAFE_RELEASE(T*& p)
{
    if (p)
    {
        p->Release();
        p = NULL;
    }
}
#endif

// SAFE_ADDREF macro.
// AddRef's a COM pointer if the pointer is not NULL.

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(x) if (x) { x->AddRef(); }
#endif

// SAFE_DELETE macro.
// Deletes a pointer allocated with new.

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x) { delete x; x = NULL; }
#endif

// CopyComPointer
// Assigns a COM pointer to another COM pointer.
template <class T>
void CopyComPointer(T* &dest, T *src)
{
    if (dest)
    {
        dest->Release();
    }
    dest = src;
    if (dest)
    {
        dest->AddRef();
    }
}

// SAFE_ARRAY_DELETE macro.
// Deletes an array allocated with new [].

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) if (x) { delete [] x; x = NULL; }
#endif

// ARRAY_SIZE macro.
// Returns the size of an array (on the stack only)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]) )
#endif

// IF_FAILED_GOTO macro.
// Jumps to 'label' on failure.
#ifndef IF_FAILED_GOTO
#define IF_FAILED_GOTO(hr, label) if (FAILED(hr)) { goto label; }
#endif

// CheckPointer macro.
// Returns 'hr' if pointer 'x' is NULL.
#ifndef CheckPointer
#define CheckPointer(x, hr) if (x == NULL) { return hr; }
#endif

///////////////////////////////////////////////////////////////////////
// Name: AreCOMObjectsEqual [template]
// Desc: Tests two COM pointers for equality.
///////////////////////////////////////////////////////////////////////

template <class T1, class T2>
bool AreComObjectsEqual(T1 *p1, T2 *p2)
{
    bool bResult = false;
    if (p1 == NULL && p2 == NULL)
    {
        // Both are NULL
        bResult = true;
    }
    else if (p1 == NULL || p2 == NULL)
    {
        // One is NULL and one is not
        bResult = false;
    }
    else 
    {
        // Both are not NULL. Compare IUnknowns.
        IUnknown *pUnk1 = NULL;
        IUnknown *pUnk2 = NULL;
        if (SUCCEEDED(p1->QueryInterface(IID_IUnknown, (void**)&pUnk1)))
        {
            if (SUCCEEDED(p2->QueryInterface(IID_IUnknown, (void**)&pUnk2)))
            {
                bResult = (pUnk1 == pUnk2);
                pUnk2->Release();
            }
            pUnk1->Release();
        }
    }
    return bResult;
}

#include <assert.h>

#include "mfutils.h"
#include "asyncCB.h"
#include "BufferLock.h"
#include "ClassFactory.h"
#include "critsec.h"
#include "GrowArray.h"
#include "linklist.h"
#include "mediatype.h"
#include "propvar.h"
#include "TinyMap.h"
#include "trace.h"
