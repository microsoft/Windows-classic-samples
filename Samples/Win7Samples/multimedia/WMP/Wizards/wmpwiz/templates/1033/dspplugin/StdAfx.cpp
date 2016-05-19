// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
[!if VSNET]
[!else]
#include <statreg.cpp>
[!endif]
#endif

[!if VSNET]
[!else]
#include <atlimpl.cpp>
[!endif]
