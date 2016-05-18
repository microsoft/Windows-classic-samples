/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Persist.cpp

   Description:   CTSFEditWnd::_Save() and _Load() implementation

**************************************************************************/

/**************************************************************************
	#include statements
**************************************************************************/

#include "TSFEdit.h"
#include "PropLdr.h"

/**************************************************************************
	global variables and definitions
**************************************************************************/

/**************************************************************************
	local function prototypes
**************************************************************************/

#define BLOCK_SIZE  256

void CTSFEditWnd::_SaveToFile(LPTSTR pszFile)
{
    if(pszFile)
    {
        HANDLE  hFile;

        hFile = CreateFile( pszFile, 
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        
        if(INVALID_HANDLE_VALUE != hFile)
        {
            IStream *pStream;
            
            //create a stream on global memory
            if(SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
            {
                LARGE_INTEGER   li;

                _Save(pStream);

                //initialize the file
                SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                SetEndOfFile(hFile);
                
                //set the stream pointer to the start of the stream
                li.QuadPart = 0;
                pStream->Seek(li, STREAM_SEEK_SET, NULL);
                
                //write the contents of the stream to the file
                BYTE    buffer[BLOCK_SIZE];
                ULONG   uRead;
                HRESULT hr;

                uRead = 0;
                hr = pStream->Read(buffer, BLOCK_SIZE, &uRead);
                while(uRead > 0)
                {
                    DWORD   dwWritten;
                    
                    WriteFile(hFile, buffer, uRead, &dwWritten, NULL);
                    
                    uRead = 0;
                    hr = pStream->Read(buffer, BLOCK_SIZE, &uRead);
                }

                pStream->Release();
            }

            CloseHandle(hFile);
        }
    }
}

/**************************************************************************

	CTSFEditWnd::_Save()

**************************************************************************/

void CTSFEditWnd::_Save(IStream *pStream)
{
    if(pStream)
    {
        HRESULT hr;
        
        //write the plain UNICODE text into the stream
        LPWSTR          pwsz;
        LONG            cch;
        ULONG           uWritten;
        LARGE_INTEGER   li;
        ULONG           uSize;

        //set the stream pointer to the start of the stream
        li.QuadPart = 0;
        pStream->Seek(li, STREAM_SEEK_SET, NULL);
        
        //get the text
        if(SUCCEEDED(_GetText(&pwsz, &cch)))
        {
            TF_PERSISTENT_PROPERTY_HEADER_ACP   PropHeader;

            //write the size, in BYTES, of the text
            uSize = cch * sizeof(WCHAR);
            hr = pStream->Write(&uSize, sizeof(ULONG), &uWritten);

            //write the text, including the NULL_terminator, into the stream
            hr = pStream->Write(pwsz, uSize, &uWritten);

            //free the memory allocated by _GetText
            GlobalFree(pwsz);

            //enumerate the properties in the context
            IEnumTfProperties   *pEnumProps;
            hr = m_pContext->EnumProperties(&pEnumProps);
            if(SUCCEEDED(hr))
            {
                ITfProperty *pProp;
                ULONG       uFetched;

                while(SUCCEEDED(pEnumProps->Next(1, &pProp, &uFetched)) && uFetched)
                {
                    //enumerate all the ranges that contain the property
                    IEnumTfRanges   *pEnumRanges;
                    hr = pProp->EnumRanges(m_EditCookie, &pEnumRanges, NULL);
                    if(SUCCEEDED(hr))
                    {
                        IStream *pTempStream;

                        //create a temporary stream to write the property data to
                        hr = CreateStreamOnHGlobal(NULL, TRUE, &pTempStream);
                        if(SUCCEEDED(hr))
                        {
                            ITfRange    *pRange;

                            while(SUCCEEDED(pEnumRanges->Next(1, &pRange, &uFetched)) && uFetched)
                            {
                                //reset the temporary stream's pointer
                                li.QuadPart = 0;
                                pTempStream->Seek(li, STREAM_SEEK_SET, NULL);
                                
                                //get the property header and data for the range
                                hr = m_pServices->Serialize(pProp, pRange, &PropHeader, pTempStream);

                                /*
                                Write the property header into the primary stream. 
                                The header also contains the size of the property 
                                data.
                                */
                                hr = pStream->Write(&PropHeader, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP), &uWritten);

                                //reset the temporary stream's pointer
                                li.QuadPart = 0;
                                pTempStream->Seek(li, STREAM_SEEK_SET, NULL);

                                //copy the property data from the temporary stream into the primary stream
                                ULARGE_INTEGER  uli;
                                uli.QuadPart = PropHeader.cb;

                                hr = pTempStream->CopyTo(pStream, uli, NULL, NULL);

                                pRange->Release();
                            }
                            
                            pTempStream->Release();
                        }
                        
                        pEnumRanges->Release();
                    }
                    
                    pProp->Release();
                }
                
                pEnumProps->Release();
            }

            //write a property header with zero size and guid into the stream as a terminator
            ZeroMemory(&PropHeader, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP));
            hr = pStream->Write(&PropHeader, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP), &uWritten);
        }
    }
}

/**************************************************************************

	CTSFEditWnd::_LoadFromFile()

**************************************************************************/

void CTSFEditWnd::_LoadFromFile(LPTSTR pszFile)
{
    if(pszFile)
    {
        HANDLE  hFile;

        hFile = CreateFile( pszFile, 
                            GENERIC_READ,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        
        if(INVALID_HANDLE_VALUE != hFile)
        {
            IStream *pStream;
            
            //create a stream on global memory
            if(SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
            {
                //read the contents of the file into the stream
                LARGE_INTEGER   li;

                //set the stream pointer to the start of the stream
                li.QuadPart = 0;
                pStream->Seek(li, STREAM_SEEK_SET, NULL);
                
                //write the contents of the stream to the file
                BYTE    buffer[BLOCK_SIZE];
                ULONG   uRead;
                HRESULT hr;

                uRead = 0;
                ReadFile(hFile, buffer, BLOCK_SIZE, &uRead, NULL);
                while(uRead > 0)
                {
                    ULONG   uWritten;

                    hr = pStream->Write(buffer, uRead, &uWritten);
                    
                    uRead = 0;
                    ReadFile(hFile, buffer, BLOCK_SIZE, &uRead, NULL);
                }

                _Load(pStream);

                pStream->Release();
            }

            CloseHandle(hFile);
        }
    }
}

/**************************************************************************

   CTSFEditWnd::_Load()

**************************************************************************/

void CTSFEditWnd::_Load(IStream *pStream)
{
    if(NULL == pStream)
    {
        return;
    }

    //can't do this if someone has a lock
    if(_IsLocked(TS_LF_READ))
    {
        return;
    }

    _ClearText();

    HRESULT         hr;
    ULONG           uRead;
    LARGE_INTEGER   li;
    ULONG           uSize;

    //set the stream pointer to the start of the stream
    li.QuadPart = 0;
    pStream->Seek(li, STREAM_SEEK_SET, NULL);

    //get the size of the text, in BYTES. This is the first ULONG in the stream
    hr = pStream->Read(&uSize, sizeof(ULONG), &uRead);
    if(SUCCEEDED(hr) && (sizeof(ULONG) == uRead))
    {
        LPWSTR  pwsz;
        
        //allocate a buffer for the text plus one NULL character
        pwsz = (LPWSTR)GlobalAlloc(GPTR, uSize + sizeof(WCHAR));
        if(NULL != pwsz)
        {
            //get the plain UNICODE text from the stream
            hr = pStream->Read(pwsz, uSize, &uRead);
            if(SUCCEEDED(hr) && (uSize == uRead))
            {
                TF_PERSISTENT_PROPERTY_HEADER_ACP   PropHeader;
                
                //put the text into the edit control, but don't send a change notification
                BOOL    fOldNotify = m_fNotify;
                m_fNotify = FALSE;
                SetWindowTextW(m_hwndEdit, pwsz);
                m_fNotify = fOldNotify;

                /*
                Read each property header and property data from the stream. The 
                list of properties is terminated by a TF_PERSISTENT_PROPERTY_HEADER_ACP 
                structure with a cb member of zero.
                */
                hr = pStream->Read(&PropHeader, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP), &uRead);
                while(  SUCCEEDED(hr) && 
                        (sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP) == uRead) && 
                        (0 != PropHeader.cb))
                {
                    ITfProperty *pProp;

                    hr = m_pContext->GetProperty(PropHeader.guidType, &pProp);
                    if(SUCCEEDED(hr))
                    {
                        /*
                        Have TSF read the property data from the stream. This call 
                        will request a read lock, so make sure it can be granted 
                        or else this method will fail.
                        */
                        CTSFPersistentPropertyLoader *pLoader = new CTSFPersistentPropertyLoader(&PropHeader, pStream);
                        hr = m_pServices->Unserialize(pProp, &PropHeader, NULL, pLoader);

                        pProp->Release();
                    }

                    hr = pStream->Read(&PropHeader, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP), &uRead);
                }
            }
            
            GlobalFree(pwsz);
        }
    }
}

