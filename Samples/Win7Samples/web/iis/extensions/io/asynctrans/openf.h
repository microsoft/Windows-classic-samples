/* Copyright(c) 1995-2002 Microsoft Corporation

	Module Name:

		openf.h

	Abstract:

		This module defines functions for opening and closing files
		and provides transparent caching for file handles
*/

# ifndef _OPENF_HXX_
# define _OPENF_HXX_

# include <windows.h>
# include <httpext.h>

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
		{RemoveEntryList((ListHead)->Flink)}

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
		{RemoveEntryList((ListHead)->Blink)}

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
                            }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
                                    }
#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
                                    }
/* Initialization and cleanup functions */

DWORD CleanupFileHandleCache(VOID);
HANDLE FcOpenFile(IN EXTENSION_CONTROL_BLOCK *pecb, IN LPCSTR pszFile);
DWORD FcCloseFile(IN HANDLE hFile);
BOOL FcReadFromFile(IN HANDLE hFile, OUT CHAR *pchBuffer, IN  DWORD dwBufferSize, OUT LPDWORD pcbRead, IN OUT LPOVERLAPPED pov);
VOID InitFileHandleCache(VOID);

# endif