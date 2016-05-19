//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            wmstats.cpp
//
// Abstract:            This file contains the entry point to the sample,
//                      parsing the command line and calling into the
//                      CReader and CWriter classes to parse and print
//                      the Reader and Writer statistics
//
//*****************************************************************************

#include "stdafx.h"
#include "Reader.h"
#include "Writer.h"

#define FILE   0
#define CLIENT 1
#define SERVER 2


//
//  20 seconds delay
//
static DWORD s_dwStatsDelay = 1000 * 20;
static HANDLE hReaderStatsEvent = 0;
static HANDLE hWriterStatsEvent = 0;

///////////////////////////////////////////////////////////////
static void Usage();
static HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR* ptszInput, __out LPTSTR* ptszOutput, int &iMode, DWORD &dwPortNum );
#ifndef UNICODE
static HRESULT ConvertStrToUnicode( LPCTSTR    ptszInput, __out LPWSTR * pwszInput );
#endif
static void DisplayReaderStats( WM_READER_STATISTICS Stats );
static void DisplayWriterStats( WM_WRITER_STATISTICS Stats );
static DWORD WINAPI GetReaderStats( LPVOID lpParameter );
static DWORD WINAPI GetWriterStats( LPVOID lpParameter );

///////////////////////////////////////////////////////////////

int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    DWORD     dwPortNum     = -1 ;
    TCHAR*    ptszMode      = NULL ;
    TCHAR*    ptszInput     = NULL ;
    TCHAR*    ptszOutput    = NULL ;
    int       iMode         = FILE;
    HRESULT   hr            = S_OK ;

    hr = CommandLineParser( argc, argv, &ptszInput, &ptszOutput, iMode, dwPortNum );
    if ( FAILED( hr ) )
    {
        return hr;
    }

#ifndef UNICODE 
    WCHAR* pwszInput = NULL;
    if (NULL != ptszInput)
    {
        hr = ConvertStrToUnicode( ptszInput, &pwszInput );
        if ( FAILED( hr ) )
        {
            return hr;
        }
    }
    
    WCHAR* pwszOutput = NULL;
    if (NULL != ptszOutput)
    {
        hr = ConvertStrToUnicode( ptszOutput, &pwszOutput );
        if ( FAILED( hr ) )
        {
            return hr;
        }
    }
    
    
#endif // UNICODE

    HANDLE  hReaderThread = NULL;
	HANDLE  hWriterThread = NULL;
    DWORD   dwReaderThreadID = 0;
    DWORD   dwWriterThreadID = 0;

    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: hr = 0x%08x\n" ), hr );
        return hr ;
    }


    do {
        if (FILE == iMode)
        {
            CReader reader;
            CWriter writer;

            //
            // Create thread displaying statistics  
            //

            hReaderStatsEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
			if( NULL == hReaderStatsEvent )
            {
                hr = E_FAIL;
                break;
            }

            hReaderThread = CreateThread( NULL, 0, GetReaderStats,
                                           (LPVOID) &reader, CREATE_SUSPENDED , &dwReaderThreadID );
            if ( NULL == hReaderThread )
            {
                hr = E_FAIL;
                break;
            }
			
			hWriterStatsEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
			if( NULL == hWriterStatsEvent )
            {
                hr = E_FAIL;
                break;
            }

            hWriterThread = CreateThread( NULL, 0, GetWriterStats,
                                           (LPVOID) &writer, CREATE_SUSPENDED , &dwWriterThreadID );
            if ( NULL == hWriterThread )
            {
                hr = E_FAIL;
                break;
            }

            hr = writer.Init();
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Create file sink for writer 
            //
#ifndef UNICODE
            hr = writer.CreateFileSink( pwszOutput );
#else
            hr = writer.CreateFileSink( ptszOutput );
#endif
            if ( FAILED( hr ) )
            {
                break;
            }

            

            //
            // Initialize reader.
            //

            hr = reader.Init();
            if ( FAILED ( hr ) )
            {
                break;
            }

            //
            // Attach writer to reader.
            //

            hr = reader.AttachWriter( &writer );
            if ( FAILED( hr ) )
            {
                break;
            }

#ifndef UNICODE
            hr = reader.Configure( pwszInput );
#else
            hr = reader.Configure( ptszInput );
#endif

            if ( FAILED( hr ) )
            {
                break;
            }
            _tprintf( _T(  "WMStats initialized successfully. \n\n" ) );
            
            //
            // Resume statistics thread
            //
            ResumeThread( hReaderThread );
			ResumeThread( hWriterThread );
            

            //
            // Start reader. This is a synchronous function that waits until playback is finished
            //

            hr = reader.Start();
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Close reader
            //

            SetEvent( hReaderStatsEvent );
			SetEvent( hWriterStatsEvent );

            HANDLE hThreads[] = { hReaderThread, hWriterThread };

            hr = reader.Close( ( HANDLE *)hThreads, 2 );
            if ( FAILED( hr ) )
            {
                break;
            }
        }
        if (CLIENT == iMode)
        {
            //
            // Client: reads stream from a network or an ASF file and plays it on a wave output
            //

            CReader reader;

            //
            // Create thread displaying statistics  
            //

            hReaderStatsEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if( NULL == hReaderStatsEvent )
            {
                hr = E_FAIL;
                break;
            }

            hReaderThread = CreateThread( NULL, 0, GetReaderStats,
                                           (LPVOID) &reader, CREATE_SUSPENDED , &dwReaderThreadID );
            if ( NULL == hReaderThread )
            {
                hr = E_FAIL;
                break;
            }

            hr = reader.Init();
            if ( FAILED( hr ) )
            {
                break;
            }

#ifndef UNICODE
            hr = reader.Configure( pwszInput );
            if ( FAILED( hr ) )
            {
                break;
            }

            _tprintf( _T(  "WMStats initialized succefully as a CLIENT\n\n" ));
#else
            hr = reader.Configure( ptszInput );
            if ( FAILED( hr ) )
            {
                break;
            }

            _tprintf( _T(  "WMStats initialized succefully as a CLIENT\n\n" ));
#endif
            //
            // Resume statistics thread
            //

            ResumeThread( hReaderThread );

            //
            // Start reader. This is a synchronous function, which waits until playback is finished
            //

            hr = reader.Start();
            if ( FAILED( hr ) )
            {
                break;
            }
            //
            // Close reader
            //

            SetEvent( hReaderStatsEvent );

            hr = reader.Close( &hReaderThread, 1 );
            if ( FAILED( hr ) )
            {
                break;
            }
        }
        else if (SERVER == iMode)
        {
            //
            // Server : reads from an ASF file and sends it to a network
            //

            CReader reader;
            CWriter writer;

            //
            // Create thread displaying statistics  
            //

            hWriterStatsEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if( NULL == hWriterStatsEvent )
            {
                hr = E_FAIL;
                break;
            }

            hWriterThread = CreateThread( NULL, 0, GetWriterStats,
                                           (LPVOID) &writer, CREATE_SUSPENDED , &dwWriterThreadID );
            if ( NULL == hWriterThread )
            {
                hr = E_FAIL;
                break;
            }
            hr = writer.Init();
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Create network sink for writer for port dwPortNum and 50 concurrent users
            //

            hr = writer.CreateNetSink( dwPortNum, 50 );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Initialize reader
            //

            hr = reader.Init();
            if ( FAILED ( hr ) )
            {
                break;
            }

            //
            // Attach writer to reader. The reader will send stream to network
            //

            hr = reader.AttachWriter( &writer );
            if ( FAILED( hr ) )
            {
                break;
            }

#ifndef UNICODE
            hr = reader.Configure( pwszInput );
            if ( FAILED( hr ) )
            {
                break;
            }

            _tprintf( _T(  "WMStats initialized succefully as a SERVER;  port : %u :\n\n" ), dwPortNum );
#else
            hr = reader.Configure( ptszInput );
            if ( FAILED( hr ) )
            {
                break;
            }
            _tprintf( _T(  "WMStats initialized succefully as a SERVER;  port : %u \n\n" ), dwPortNum );
#endif
            //
            // Resume statistics thread
            //

            ResumeThread( hWriterThread );

            //
            // Start reader - this is synchronous function, it waits until playback is finished
            //

            hr = reader.Start();
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Close reader
            //

            SetEvent( hWriterStatsEvent );

            hr = reader.Close( &hWriterThread, 1 );
            if ( FAILED( hr ) )
            {
                break;
            }

        }
    }while( FALSE );

#ifndef UNICODE
    if (NULL != pwszInput)
    {
        delete[] pwszInput;
        pwszInput = NULL;
    }
    if (NULL != pwszOutput)
    {
        delete[] pwszOutput;
        pwszOutput = NULL;
    }
#endif
    
    if (FAILED( hr ) )
    {
        _tprintf( _T(  "WMStats failed (hr=0x%08x).\n" ), hr );

    }


    if( NULL != hWriterStatsEvent )
    {
        CloseHandle( hWriterStatsEvent );
    }
	if( NULL != hReaderStatsEvent )
    {
        CloseHandle( hReaderStatsEvent );
    }

    if ( NULL != hWriterThread )
    {
        CloseHandle( hWriterThread );
    }
    if ( NULL != hReaderThread )
    {
        CloseHandle( hReaderThread );
    }

    CoUninitialize();

    return( 0 );

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// This function gives the correct usage,                                   //
// if the user gives invalid arguments.                                     //
//////////////////////////////////////////////////////////////////////////////
void Usage()
{
   _tprintf( _T( "wmstats -m <mode> -i <infile> -o <output> [-s <statistics_delay>]\n" ) );
   _tprintf( _T( "\tmode\t = mode : f- file, s - server or c - client\n" ) );
   _tprintf( _T( "\tinfile\t = WMV file name or URL for server or client mode\n" ) );
   _tprintf( _T( "\toutput\t = for file   : WMV output file name\n" ) );
   _tprintf( _T( "\t\t   for server : port number\n" ) );
   _tprintf( _T( "\t\t   for client : not valid\n" ) );
   _tprintf( _T( "\tstatistics_delay = [sec] delay of statistics report; default 20 seconds\n" ) );
   _tprintf( _T( "\n" ) );
   _tprintf( _T( "Samples :\n" ) );
   _tprintf( _T( "file                    : wmstats -m f -i c:\\wmroot\\welcome2.asf -o c:\\wmroot\\test.asf\n" ) );
   _tprintf( _T( "client (input: network) : wmstats -m c -i http://banhammer:32800\n" ) );
   _tprintf( _T( "client (input: file)    : wmstats -m c -i c:\\wmroot\\welcome2.asf\n" ) );
   _tprintf( _T( "server                  : wmstats -m s -i c:\\wmroot\\welcome2.asf -o 32800\n" ) );

}

///////////////////////////////////////////////////////////////
////////
HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR * ptszInput, __out LPTSTR * ptszOutput, int &iMode, DWORD &dwPortNum )
{

    if ( argc < 1 || NULL == argv || NULL == ptszInput)
    {
        return( E_INVALIDARG );
    }


    TCHAR*    ptszMode    = NULL;
    HRESULT hr            = S_OK;
    *ptszInput            = NULL; 
    *ptszOutput           = NULL;
    iMode                 = FILE;
    dwPortNum             = -1;


    for( int i = 1; i < argc; i ++ )
    {
        if ( 0 == _tcsicmp( argv[i], _T( "-m" ) ) )
        {
            i++ ;

            if ( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }
            
            ptszMode = argv[i];
            continue ;

        }

        if ( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++;

            if ( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }

            *ptszInput = argv[i];
            continue;
        }

        if( 0 == _tcsicmp( argv[i] , _T( "-o" ) ) )
        {
            i++ ;

            if(i == argc)
            {
                Usage() ;
                return ( E_INVALIDARG );
            }

            *ptszOutput = argv[i];
            continue;
        }

        if( 0 == _tcsicmp( argv[i] , _T( "-t" ) ) )
        {
            i++ ;

            if(i == argc)
            {
                Usage() ;
                return ( E_INVALIDARG );
            }
            int nDelay = 0;
            int ret = _stscanf_s( argv[i], _T( "%d" ), &nDelay ) ;
            if (EOF == ret || 0 == ret || nDelay <= 0 || nDelay > MAX_STATS_DELAY)
            {
                Usage();
                return( E_INVALIDARG );
            }

            s_dwStatsDelay = nDelay * 1000;

            continue;
        }

    }

    if ( NULL == ptszMode || NULL == *ptszInput )
    {
        Usage();
        return( E_INVALIDARG );
    }

    
    if ('f' == *ptszMode || 'F' == *ptszMode)
    {
        iMode = FILE;    //file
        if (NULL == *ptszOutput)
        {
            Usage();
            return( E_INVALIDARG );
        }
    }
    else if ('c' == *ptszMode || 'C' == *ptszMode)
    {
        iMode = CLIENT;    //client
    }
    else if ('s' == *ptszMode || 'S' == *ptszMode)
    {
        iMode = SERVER;    //server

        if (NULL != *ptszOutput)
		{
			int ret = _stscanf_s( *ptszOutput, _T( "%d" ), &dwPortNum ) ;
			if (EOF == ret || 0 == ret || dwPortNum > 0xffff || dwPortNum == -1)
			{
				Usage();
				return( E_INVALIDARG );
			}
		}
    }
    else
    {
        Usage();
        return( E_INVALIDARG );
    }

    return S_OK;
}

#ifndef UNICODE
///////////////////////////////////////////////////////////////
///////
HRESULT ConvertStrToUnicode( LPCTSTR ptszInput, __out LPWSTR * ppwszInput )
{
    HRESULT hr = S_OK;
    int nSizeCount = 0 ;
    
    if ( NULL == ptszInput || NULL == ppwszInput)
    {
        return( E_INVALIDARG );
    }

    //
    // Make wide char string of the file name
    //
    nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 ) ;
    if( 0 ==  nSizeCount )
    {
        _tprintf( _T(  "Internal error %lu\n" ), GetLastError() );
        return ( E_UNEXPECTED );
    }

    *ppwszInput = new WCHAR[ nSizeCount ] ;

    if( NULL == *ppwszInput)
    {
        _tprintf( _T( "Internal Error %lu\n" ), GetLastError() ) ;
        return ( E_OUTOFMEMORY );
    }

    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *ppwszInput, nSizeCount ) )
    {
        _tprintf( _T(  "Internal error %lu\n" ), GetLastError() );
        delete[] *ppwszInput ;
        *ppwszInput = NULL;
        return ( E_UNEXPECTED );
    }        

    return( hr );
}
#endif
///////////////////////////////////////////////////////////////
////////
void DisplayReaderStats( WM_READER_STATISTICS Stats )
{

    _tprintf( _T( "\n------------- READER STATISTICS ------------\n" ) );
    _tprintf( _T( "\tBandwidth [bps]     : %d\n"), Stats.dwBandwidth);
    _tprintf( _T( "\tPacketsReceived     : %d\n"), Stats.cPacketsReceived);
    _tprintf( _T( "\tPacketsRecovered    : %d\n"), Stats.cPacketsRecovered);
    _tprintf( _T( "\tPacketsLost         : %d\n"), Stats.cPacketsLost);
    _tprintf( _T( "\tQuality [%]         : %d\n"), Stats.wQuality);
    _tprintf( _T( "-------------------------------------------------------\n\n" ) );
}

///////////////////////////////////////////////////////////////
//////
void DisplayWriterStats( DWORD dwClientNum, int nStatsNum, CWriterStats *pStats, CWriterStatsEx *pStatsEx )
{
    if ( !pStats )
        return;

    _tprintf( _T( "\n------------- WRITER STATISTICS ------------\n" ) );
    _tprintf( _T( "\tConnected clients                              : %d\n"), dwClientNum);

    for ( int i = 0; i < nStatsNum; i++ )
    {
        _tprintf( _T( "\n"));

        if ( 0 == pStats->m_nStreamNum )
        {
            _tprintf( _T( "\tSUMMARY STATISTICS: \n"));
        }
        else
        {
            _tprintf( _T( "\tStream Number                              : %d\n"), pStats->m_nStreamNum);
        }

        _tprintf( _T( "\tSampleCount                                : %d\n"), pStats->qwSampleCount);
        _tprintf( _T( "\tByteCount                                  : %d\n"), pStats->qwByteCount);
        _tprintf( _T( "\tDroppedSampleCount [1000 * (samples/s)]    : %d\n"), pStats->qwDroppedSampleCount);
        _tprintf( _T( "\tDroppedByteCount                           : %d\n"), pStats->qwDroppedByteCount);
        _tprintf( _T( "\tCurrentBitrate [bps]                       : %d\n"), pStats->dwCurrentBitrate);
        _tprintf( _T( "\tAverageBitrate [bps]                       : %d\n"), pStats->dwAverageBitrate);
        _tprintf( _T( "\tExpectedBitrate [bps]                      : %d\n"), pStats->dwExpectedBitrate);
        _tprintf( _T( "\tCurrentSampleRate [1000 * (samples/s)]     : %d\n"), pStats->dwCurrentSampleRate);
        _tprintf( _T( "\tAverageSampleRate [1000 * (samples/s)]     : %d\n"), pStats->dwAverageSampleRate);
        _tprintf( _T( "\tExpectedSampleRate [1000 * (samples/s)]    : %d\n"), pStats->dwExpectedSampleRate);
        pStats++;

    }
		_tprintf( _T( "-------------------------------------------------------\n\n" ) );
		_tprintf( _T( "\tBitratePlusOverhead                        : %d\n"), pStatsEx->dwBitratePlusOverhead);
		_tprintf( _T( "\tCurrentSampleDropRateInQueue               : %d\n"), pStatsEx->dwCurrentSampleDropRateInQueue);
		_tprintf( _T( "\tCurrentSampleDropRateInCodec               : %d\n"), pStatsEx->dwCurrentSampleDropRateInCodec);
		_tprintf( _T( "\tCurrentSampleDropRateInMultiplexer         : %d\n"), pStatsEx->dwCurrentSampleDropRateInMultiplexer);
		_tprintf( _T( "\tTotalSampleDropsInQueue                    : %d\n"), pStatsEx->dwTotalSampleDropsInQueue);
		_tprintf( _T( "\tTotalSampleDropsInCodec                    : %d\n"), pStatsEx->dwTotalSampleDropsInCodec);
		_tprintf( _T( "\tTotalSampleDropsInMultiplexer              : %d\n"), pStatsEx->dwTotalSampleDropsInMultiplexer);
		_tprintf( _T( "-------------------------------------------------------\n\n" ) );
}

///////////////////////////////////////////////////////////////
// Get reader stats after s_dwStatsDelay seconds
//
static DWORD WINAPI GetReaderStats( LPVOID lpParameter )
{
    if ( NULL != lpParameter )
    {
        WM_READER_STATISTICS Stats;
        CReader             *reader = (CReader*)lpParameter;
        DWORD               dwStatus = 0;

        //
        // Wait for s_dwStatsDelay seconds
        //

        while( TRUE )
        {
            dwStatus = WaitForSingleObject( hReaderStatsEvent, s_dwStatsDelay );
            if ( FAILED(reader->GetStats( &Stats ) ) )
            {
                _tprintf( _T( "\n------  CANNOT GET READER STATISTICS -------\n" ) );
                return 0;
            }
            DisplayReaderStats( Stats );

            if( WAIT_OBJECT_0 == dwStatus )
            {
                ExitThread( 0 );
            }
        }
    }
    return( 0 );
}

///////////////////////////////////////////////////////////////
// Get writer stats after s_dwStatsDelay seconds
//
static DWORD WINAPI GetWriterStats( LPVOID lpParameter )
{

    if ( NULL != lpParameter )
    {
        CWriterStats    *pStats = NULL;
		CWriterStatsEx  *pStatsEx = NULL;
        DWORD           dwClientNum = 0;
        DWORD           dwStatus = 0;
        int             nStatsNum = 0;
        CWriter         *writer = (CWriter*)lpParameter;

        //
        // Wait for s_dwStatsDelay seconds
        //

        while( TRUE )
        {
            dwStatus = WaitForSingleObject( hWriterStatsEvent, s_dwStatsDelay );

            if (FAILED ( writer->GetStats( dwClientNum, nStatsNum, &pStats, &pStatsEx ) ) )
            {
                _tprintf( _T( "\n----- CANNOT GET WRITER STATISTICS --------\n" ) );
                return 0;
            }

            DisplayWriterStats( dwClientNum, nStatsNum, pStats, pStatsEx );

            if ( NULL != pStats )
            {
                delete [] pStats;
                pStats = NULL;
            }

            if( WAIT_OBJECT_0 == dwStatus )
            {
                ExitThread( 0 );
            }
        }
    }
    return( 0 );
}
