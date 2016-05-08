//---------------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-1998 Microsoft Corporation.  
//
// @doc 
//
// @module Version Header for Modules | Version numbers included by ALL components.
//
// @comm
// Special Notes:   No other version information should be used outside of
//                  this file.  This file is a place holder.  The current
//					date is used in the actual build.
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//  [01] 08-12-94   Microsoft   Created file.
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//	
//---------------------------------------------------------------------------

#ifndef VERSION_DEFS
#define VERSION_DEFS

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define VER_FILEVERSION 2,70,8019.0
#define VER_FILEVERSION_STR "2.70.8019.0\0"
#define VER_PRODUCTVERSION 2,70,8019.0
#define VER_PRODUCTVERSION_STR "2.70.8019.0\0"

#define VER_FILEFLAGSMASK		(VS_FF_DEBUG | VS_FF_PRERELEASE)

#ifdef DEBUG
	#define VER_FILEFLAGS			(VS_FF_DEBUG)
#else
	#define VER_FILEFLAGS			(0)
#endif

#define VER_FILEOS				VOS_NT_WINDOWS32
#define VER_COMPANYNAME_STR		"Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR		"Microsoft OLE DB\0"
#define VER_LEGALCOPYRIGHT_STR	"Copyright \251 Microsoft Corporation 1995-2000\0"

#endif // VERSION_DEFS
