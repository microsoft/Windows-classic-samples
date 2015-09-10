#include "FileStreamWithEPending.hpp"

HRESULT FileStreamWithEPending::OpenFile(LPCWSTR pName, IStream ** ppStream, bool fWrite)
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (NULL == ppStream || NULL == pName)
        HR(E_POINTER);

    (*ppStream) = NULL;

    hFile = ::CreateFile(pName, fWrite ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ,
        NULL, fWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        HR(HRESULT_FROM_WIN32(GetLastError()));

    (*ppStream) = new FileStreamWithEPending(hFile);
    if(NULL == (*ppStream))
        HR(E_OUTOFMEMORY);

CleanUp:
    if (FAILED(hr) && INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return hr;
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (!ppvObject)
        return E_INVALIDARG;
    (*ppvObject) = nullptr;

    if (iid == __uuidof(IUnknown)
        || iid == __uuidof(IStream)
        || iid == __uuidof(ISequentialStream))
    {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    } 
    else
    {
        return E_NOINTERFACE; 
    }
}

// ISequentialStream Interface
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Read(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbRead)
{
    if (_raiseEPending)
        return ReadRaiseEPending(pv, cb, pcbRead);
    else
        return ReadNotRaiseEPending(pv, cb, pcbRead);
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::ReadNotRaiseEPending(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, ULONG cb, _Out_opt_ ULONG* pcbRead)
{
    BOOL rc = ReadFile(_hFile, pv, cb, pcbRead, NULL);
    if (rc)
    {
        if (cb > (*pcbRead))
        {
            return S_FALSE;
        }
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::ReadRaiseEPending(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, ULONG cb, _Out_opt_ ULONG* pcbRead)
{
    if (!pcbRead)
        return E_INVALIDARG;
    (*pcbRead) = 0;

    HRESULT hr = S_OK;
    BOOL rc;
    ULONG byteRead = 0;
    LARGE_INTEGER liDistanceToMove;

    BYTE *buff = new BYTE[cb + DELIMIT_LEN + 1];
    if (NULL == buff)
        return E_OUTOFMEMORY;

    rc = ReadFile(_hFile, buff, cb + DELIMIT_LEN, &byteRead, NULL);
    if (rc)
    {
        buff[byteRead] = '\0';

        BYTE* delimit = (BYTE*)strstr((char*)buff, (const char*)DELIMIT);
        if (NULL == delimit)    //not find delimit
        {
            if (byteRead < cb + DELIMIT_LEN)    // end of file is reached.
            {
                ULONG byteCount = min(cb, byteRead);
                memcpy(pv, buff, byteCount);
                (*pcbRead) = byteCount;
                if (cb > byteRead)
                    HR(S_FALSE);

                liDistanceToMove.QuadPart = -((long long)(byteRead - byteCount));
                if (liDistanceToMove.QuadPart != 0)
                    CHKHR(Seek(liDistanceToMove, STREAM_SEEK_CUR, NULL));
            }
            else
            {
                memcpy(pv, buff, byteRead - DELIMIT_LEN);
                (*pcbRead) = byteRead - DELIMIT_LEN;

                liDistanceToMove.QuadPart = -((long long)DELIMIT_LEN);
                CHKHR(Seek(liDistanceToMove, STREAM_SEEK_CUR, NULL));
            }
        }
        else    // find delimit
        {
            (*pcbRead) = (ULONG)(delimit - buff);
            if (cb < (*pcbRead))
                HR(E_FAIL);

            memcpy(pv, buff, (*pcbRead));

            liDistanceToMove.QuadPart = -((long long)(byteRead - (*pcbRead)));
            CHKHR(Seek(liDistanceToMove, STREAM_SEEK_CUR, NULL));

            if ((*pcbRead) != cb)
                hr = E_PENDING;
        }
    }
    else
    {
        HR(HRESULT_FROM_WIN32(GetLastError()));
    }

CleanUp:
    if (buff)
        delete []buff;
    return hr;
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Write(_In_reads_bytes_(cb) const void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbWritten)
{
    BOOL rc = WriteFile(_hFile, pv, cb, pcbWritten, NULL);
    return rc ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

// IStream Interface
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::SetSize(ULARGE_INTEGER)
{ 
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::CopyTo(_In_ IStream*, ULARGE_INTEGER, _Out_opt_ ULARGE_INTEGER*, _Out_opt_ ULARGE_INTEGER*)
{ 
    return E_NOTIMPL;
}
    
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Commit(DWORD)
{ 
    return E_NOTIMPL;   
}
    
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Revert(void)
{ 
    return E_NOTIMPL;
}
    
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{ 
    return E_NOTIMPL;
}
    
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{ 
    return E_NOTIMPL;
}
    
HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Clone(__RPC__deref_out_opt IStream **)
{ 
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER* lpNewFilePointer)
{ 
    DWORD dwMoveMethod;

    switch(dwOrigin)
    {
    case STREAM_SEEK_SET:
        dwMoveMethod = FILE_BEGIN;
        break;
    case STREAM_SEEK_CUR:
        dwMoveMethod = FILE_CURRENT;
        break;
    case STREAM_SEEK_END:
        dwMoveMethod = FILE_END;
        break;
    default:   
        return STG_E_INVALIDFUNCTION;
        break;
    }

    if (SetFilePointerEx(_hFile, liDistanceToMove, (PLARGE_INTEGER) lpNewFilePointer, dwMoveMethod) == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FileStreamWithEPending::Stat(__RPC__out STATSTG* pStatstg, DWORD grfStatFlag) 
{
    if (!pStatstg || (grfStatFlag != STATFLAG_DEFAULT && grfStatFlag != STATFLAG_NONAME))
        return E_INVALIDARG;

    if (GetFileSizeEx(_hFile, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}
