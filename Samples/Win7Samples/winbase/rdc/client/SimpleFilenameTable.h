// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include <new>
#include "msrdc.h"
#include <hash_map>
#include <string>
#include <stdio.h>

#pragma once

using namespace std;

typedef stdext::hash_map<SimilarityFileIndexT, wstring> SAMPLEFILENAMETABLE;

/*+---------------------------------------------------------------------------
 
  Class:      SimpleFilenameTable
 
  Purpose:    Simple <file_id>:<file_name> table that can be saved to file.
 
  Notes:
 
----------------------------------------------------------------------------*/

class SimpleFilenameTable
{
public:
    SimpleFilenameTable()
    {
        m_table.clear();
    };
    void insert ( SimilarityFileIndexT index, const wstring &filename )
    {
        m_table[ index ] = filename;
    }
    void lookup ( SimilarityFileIndexT index, wstring &filename )
    {
        filename = m_table[ index ];
    }

    void serialize ( const wchar_t * const filename )
    {
        FILE * f = NULL;
        DebugHresult hr;
        hr = _wfopen_s ( &f, filename, L"w" );
        if ( !SUCCEEDED ( hr ) )
        {
            printf_s ( "Creating new simple table \"%S\".\n", filename );
        }
        SAMPLEFILENAMETABLE::iterator i;
        for ( i = m_table.begin(); i != m_table.end(); ++i )
        {
            fwprintf_s ( f, L"%20u %s\n", i->first, i->second.c_str() );
        }
        hr = fclose ( f );
    }

    void deserialize ( wchar_t *filename )
    {
        m_table.clear();
        FILE *f = NULL;
        errno_t err = _wfopen_s ( &f, filename, L"r" );
        if ( 0 != err )
        {
            printf_s ( "Unable to open \"%S\".\n", filename );
            return ;
        }
        SimilarityFileIndexT index = 0;
        wchar_t line[ MAX_PATH * 2 ];
        wchar_t *fileCharA = line + 21;
        while ( true )
        {
            wchar_t * temp = fgetws ( line, ARRAYSIZE ( line ), f );
            if ( NULL == temp )
            {
                break;
            }
            // MAX_PATH + 1 = 261
            // get index using scanf
            if ( EOF != swscanf_s ( line, L"%20u", &index ) )
            {
                // get file name by parsing the line manually
                wchar_t * temp = fileCharA;
                while ( *temp != L'\n' && *temp != L'\r' && *temp != L'\0' )
                {
                    ++temp;
                }
                *temp = '\0';
                m_table[ index ] = fileCharA;
            }
        }
        DebugHresult dHr = fclose ( f );
    }

private:
    SAMPLEFILENAMETABLE m_table;
};
