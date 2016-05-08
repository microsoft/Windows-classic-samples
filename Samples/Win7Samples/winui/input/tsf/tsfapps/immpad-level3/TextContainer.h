
#pragma once

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextContainer
{
public:
    CTextContainer() 
    {
        _psz = NULL;
        _nBufferSize = 0;
        _nTextSize = 0;
    }

    ~CTextContainer() 
    {
        if (_psz)
            LocalFree(_psz);
    }

    BOOL InsertText(int nPos, const WCHAR *psz, UINT nCnt);
    BOOL RemoveText(int nPos, UINT nCnt);
    BOOL GetText(int nPos, WCHAR *psz, UINT nBuffSize);

    UINT GetTextLength() {return _nTextSize;}
    const WCHAR *GetTextBuffer() {return _psz;}

private:
    BOOL EnsureBuffer(UINT nNewTextSize);

    WCHAR *_psz;
    UINT _nBufferSize;
    UINT _nTextSize;
};


