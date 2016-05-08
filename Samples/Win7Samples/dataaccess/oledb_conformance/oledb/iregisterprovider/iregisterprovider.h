//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module IREGISTERPROVIDER.H | Header file for IRegisterProvider test module.
//
// @rev 01 | 10-23-98 | Microsoft | Created
//

#ifndef _IREGISTERPROVIDER_H_
#define _IREGISTERPROVIDER_H_


//////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library


///////////////////////////////////////////////////////////////////////
//Defines
///////////////////////////////////////////////////////////////////////
#define		CHECK_FAIL(x)								\
			if(TESTB==TEST_FAIL)					\
				odtLog<<L"INFO: Failed on URL "<<g_rgURLSchemes[x].pwszURLScheme<<".\n";


const GUID	CLSID_IREGPROV1	=	{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_IREGPROV2	=	{ 0x2799690, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_IREGPROV3	=	{ 0x2799691, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_IREGPROV4	=	{ 0x2799692, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_IREGPROV7	=	{ 0x2799696, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_IREGPROV9	=	{ 0x2799698, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };

///////////////////////////////////////////////////////////////////////
//Const Static Arrays
///////////////////////////////////////////////////////////////////////
struct tagURLSchemes
{
	WCHAR*		pwszProgID;		// Prog ID. Not used in test.
	WCHAR*		pwszURLScheme;	// URL scheme (and prefix).
	CLSID		clsid;			// clsid of the provider this scheme will get mapped to.
	WCHAR*		pwszExt1;		// An interesting extension of URL in pwszURLScheme.
	WCHAR*		pwszExt2;		// Another interesting extension of URL in pwszURLScheme.
} g_rgURLSchemes[] = 
{
	// NOTE :-
	// Add new entries to the end of array.
	// For new entries do not use existing URL schemes.
	// ALL URLs in this table are valid URLs. Donot add invalid URLs.

	// ProgID		URL Scheme						clsid																					URL Extension1			URL Extension2
	L"IREGPROV1",	L"X-IRPabcd:",					{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPabcd:123%20@abc",	L"X-IRPabcd:abcd/extension",
	L"IREGPROV1",	L"X-IRPabcd://wxyz",			{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPabcd://wxyz/a:b",	L"X-IRPabcd://wxyz/a?b$s~",
	L"IREGPROV2",	L"X-IRPabcd://klmn",			{ 0x2799690, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPabcd://klmn/1.2+3",	L"X-IRPabcd://klmn/;xyz_/,{}",
	L"IREGPROV3",	L"X-IRPabcd://klmn/wxyz",		{ 0x2799691, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPabcd://klmn/wxyz/#",	L"X-IRPabcd://klmn/wxyz/(abc]",
	L"IREGPROV4",	L"X-IRPabcd://hijk",			{ 0x2799692, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPabcd://hijk/wxyz",	L"X-IRPabcd://hijk/klmn/wxyz",
	L"IREGPROV1",	L"X-IRPklmn://klmn",			{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPklmn://klmn/hijk",	L"X-IRPklmn://klmn/X-IRPabcd:",
	L"IREGPROV5",	L"X-IRPklmn://klmn/wxyz",		{ 0x2799694, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPklmn://klmn/wxyz/<",	L"X-IRPklmn://klmn/wxyz\\abc",
	L"IREGPROV6",	L"X-IRPklmn://hijk",			{ 0x2799695, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPklmn://hijk/wxyz",	L"X-IRPklmn://hijk/wxyz/hijk",
	L"IREGPROV7",	L"X-IRPwxyz:",					{ 0x2799696, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPwxyz:<1+2.3>&^",		L"X-IRPwxyz:=12/13//",
	L"IREGPROV8",	L"X-IRPwxyz://abcd",			{ 0x2799697, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPwxyz://abcd/klmn",	L"X-IRPwxyz://abcd/wxyz/#",
	L"IREGPROV9",	L"X-IRPwxyz://a/b/c/d",			{ 0x2799698, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPwxyz://a/b/c/d\\e",	L"X-IRPwxyz://a/b/c/d/xyz",
	L"IREGPROV1",	L"X-IRPwxyz://abcd/xyz",		{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPwxyz://abcd/xyz/klmn",	L"X-IRPwxyz://abcd/xyz\\",
	L"IREGPROV9",	L"X-IRPasp://rr/ss/tt",			{ 0x2799698, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPasp://rr/ss/tt/uu",	L"X-IRPasp://rr/ss/tt/uu/vvv",
	L"IREGPROV1",	L"X-IRPasp://rr/ss/tt/uu/vv",	{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } },	L"X-IRPasp://rr/ss/tt/uu/vv/.",	L"X-IRPasp://rr/ss/tt/uu/vv/rr/ss/tt",
};

const ULONG g_cURLSchemes = NUMELEM(g_rgURLSchemes);

#endif 	//_IREGISTERPROVIDER_H_
