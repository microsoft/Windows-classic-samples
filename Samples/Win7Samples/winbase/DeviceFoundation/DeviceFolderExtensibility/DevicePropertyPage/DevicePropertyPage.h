

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0554 */
/* at Wed Mar 25 11:50:03 2009
 */
/* Compiler settings for .\DevicePropertyPage.idl:
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


#ifndef __DevicePropertyPage_h__
#define __DevicePropertyPage_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __DevicePropertyPage_FWD_DEFINED__
#define __DevicePropertyPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class DevicePropertyPage DevicePropertyPage;
#else
typedef struct DevicePropertyPage DevicePropertyPage;
#endif /* __cplusplus */

#endif 	/* __DevicePropertyPage_FWD_DEFINED__ */


/* header files for imported files */
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __DevicePropertyPageLib_LIBRARY_DEFINED__
#define __DevicePropertyPageLib_LIBRARY_DEFINED__

/* library DevicePropertyPageLib */
/* [version][helpstring][uuid] */ 


EXTERN_C const IID LIBID_DevicePropertyPageLib;

EXTERN_C const CLSID CLSID_DevicePropertyPage;

#ifdef __cplusplus

class DECLSPEC_UUID("f8ee49d1-02e4-472f-b55d-d3fc30f70de6")
DevicePropertyPage;
#endif
#endif /* __DevicePropertyPageLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


