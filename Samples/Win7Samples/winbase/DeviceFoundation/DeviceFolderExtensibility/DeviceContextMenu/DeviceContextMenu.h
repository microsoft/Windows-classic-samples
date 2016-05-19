

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0554 */
/* at Wed Mar 25 11:50:34 2009
 */
/* Compiler settings for .\DeviceContextMenu.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0554 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __DeviceContextMenu_h__
#define __DeviceContextMenu_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __DeviceContextMenu_FWD_DEFINED__
#define __DeviceContextMenu_FWD_DEFINED__

#ifdef __cplusplus
typedef class DeviceContextMenu DeviceContextMenu;
#else
typedef struct DeviceContextMenu DeviceContextMenu;
#endif /* __cplusplus */

#endif 	/* __DeviceContextMenu_FWD_DEFINED__ */


/* header files for imported files */
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __DeviceContextMenuLib_LIBRARY_DEFINED__
#define __DeviceContextMenuLib_LIBRARY_DEFINED__

/* library DeviceContextMenuLib */
/* [version][helpstring][uuid] */ 


EXTERN_C const IID LIBID_DeviceContextMenuLib;

EXTERN_C const CLSID CLSID_DeviceContextMenu;

#ifdef __cplusplus

class DECLSPEC_UUID("a05d3c0d-590b-49e5-92d7-053917a218b6")
DeviceContextMenu;
#endif
#endif /* __DeviceContextMenuLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


