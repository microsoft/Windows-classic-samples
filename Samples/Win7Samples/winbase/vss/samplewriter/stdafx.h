/*
    Copyright (c) 2009 Microsoft Corporation

    Module Name:
        stdafx.hxx
*/


#pragma once


#include <wchar.h>
#include <stdio.h>
#include <windows.h>
#include <vss.h>
#include <vswriter.h>

#ifndef DEBUG
#define _ASSERTE(__exp)
#else
#define _ASSERTE(__exp) { if( !__exp) DebugBreak(); }
#endif

#include <atlbase.h>
#include <atlconv.h>

#include <strsafe.h>


