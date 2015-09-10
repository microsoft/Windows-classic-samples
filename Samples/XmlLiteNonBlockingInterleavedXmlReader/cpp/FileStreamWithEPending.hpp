#pragma once

#include <Objidl.h>

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0) 
#define ASSERT(stmt)            do { if(!(stmt)) { DbgRaiseAssertionFailure(); } } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

const BYTE DELIMIT[] = "&NON_XML_CONTENT";
#define DELIMIT_LEN 16
#define MAX_COUNT 256

class FileStreamWithEPending : public IStream
{
protected:
    FileStreamWithEPending(HANDLE hFile) 
    {
        _refcount = 1;
        _hFile = hFile;
        _raiseEPending = TRUE;
    }

    ~FileStreamWithEPending()
    {
        if (_hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(_hFile);
        }
    }

public:
    HRESULT static OpenFile(LPCWSTR pName, IStream ** ppStream, bool fWrite);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef(void) 
    { 
        return (ULONG)InterlockedIncrement(&_refcount); 
    }

    virtual ULONG STDMETHODCALLTYPE Release(void) 
    {
        ULONG res = (ULONG) InterlockedDecrement(&_refcount);
        if (0 == res)
        {
            delete this;
        }
        return res;
    }

    // ISequentialStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE Read(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbRead);

    virtual HRESULT STDMETHODCALLTYPE Write(_In_reads_bytes_(cb) const void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbWritten);

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER);
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(_In_ IStream*, ULARGE_INTEGER, _Out_opt_ ULARGE_INTEGER*, _Out_opt_ ULARGE_INTEGER*);
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void);
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IStream **);

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER* lpNewFilePointer);

    virtual HRESULT STDMETHODCALLTYPE Stat(__RPC__out STATSTG* pStatstg, DWORD grfStatFlag);

public:
    BOOL GetRaisePendingFlag() { return _raiseEPending; }

    void SetRaisePendingFlag(BOOL raiseEPending) { _raiseEPending = raiseEPending; }

private:
    virtual HRESULT STDMETHODCALLTYPE ReadRaiseEPending(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, ULONG cb, _Out_opt_ ULONG* pcbRead);

    virtual HRESULT STDMETHODCALLTYPE ReadNotRaiseEPending(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, ULONG cb, _Out_opt_ ULONG* pcbRead);

    HANDLE _hFile;
    LONG _refcount;
    BOOL _raiseEPending;
};
