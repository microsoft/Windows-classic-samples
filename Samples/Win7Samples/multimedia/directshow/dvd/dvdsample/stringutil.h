//------------------------------------------------------------------------------
// File: StringUtil.h
//
// Desc: This header file contains several enum to string conversion functions
//       which are used throughout the rest of the program.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: AsStr()
// Desc: This overloaded function converts enumerations into human-readable 
//       text.  In most cases, it utilizes the ENUM_CASE macro to create
//       generic strings out of the enumerated types.  If you want more user-friendly
//       strings, use the form found in the DVD_TextStringType function.
//------------------------------------------------------------------------------

const TCHAR* AsStr( DVD_TextStringType IDCD );
const TCHAR* AsStr( DVD_AUDIO_LANG_EXT ext );
const TCHAR* AsStr( DVD_SUBPICTURE_LANG_EXT ext );
const TCHAR* AsStr( DVD_AUDIO_APPMODE ext );
const TCHAR* AsStr( DVD_AUDIO_FORMAT  ext );
const TCHAR* AsStr( DVD_KARAOKE_ASSIGNMENT ext );
const TCHAR* AsStr( DVD_SUBPICTURE_TYPE type );
const TCHAR* AsStr( DVD_SUBPICTURE_CODING type );
const TCHAR* AsStr( DVD_VIDEO_COMPRESSION type );
const TCHAR* AsStr( bool b );
const TCHAR* AsStr( BOOL b );
const TCHAR* KaraokeAsStr (WORD type);


