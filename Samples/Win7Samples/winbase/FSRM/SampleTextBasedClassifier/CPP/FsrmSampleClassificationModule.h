

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0553 */
/* at Mon Feb 16 07:11:56 2009
 */
/* Compiler settings for .\FsrmSampleClassificationModule.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0553 
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __FsrmSampleClassificationModule_h__
#define __FsrmSampleClassificationModule_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __FsrmSampleClassifier_FWD_DEFINED__
#define __FsrmSampleClassifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class FsrmSampleClassifier FsrmSampleClassifier;
#else
typedef struct FsrmSampleClassifier FsrmSampleClassifier;
#endif /* __cplusplus */

#endif 	/* __FsrmSampleClassifier_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __FsrmSampleClassificationModuleLib_LIBRARY_DEFINED__
#define __FsrmSampleClassificationModuleLib_LIBRARY_DEFINED__

/* library FsrmSampleClassificationModuleLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_FsrmSampleClassificationModuleLib;

EXTERN_C const CLSID CLSID_FsrmSampleClassifier;

#ifdef __cplusplus

class DECLSPEC_UUID("0F5F3806-2DAD-4050-9D30-50C35FB7ED0E")
FsrmSampleClassifier;
#endif
#endif /* __FsrmSampleClassificationModuleLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


