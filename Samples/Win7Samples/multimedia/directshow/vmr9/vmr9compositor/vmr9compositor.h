#pragma once

// Windows Header Files:
#include <tchar.h>
#include <intsafe.h>
#include <objbase.h>
#include <windows.h>
#include <assert.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <commdlg.h>
#include <Commctrl.h>

// DirectShow Header Files
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>


#include "resource.h"

// Common files
#include "smartptr.h"


#define FAIL_RET(x) do {if( FAILED( hr = ( x  ) ) ) { \
                            TCHAR achMsg[MAX_PATH];/*ASSERT( SUCCEEDED( hr ) );*/ \
                            HRESULT hrT = StringCchPrintf( achMsg, NUMELMS(achMsg), TEXT("Error code 0x%08x\r\n"), hr); \
                            OutputDebugString( achMsg ); \
                        return hr; \
                    }} while(0)


#ifndef ASSERT
#define ASSERT assert
#endif