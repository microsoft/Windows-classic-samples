
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Sun Jun 25 19:31:00 2000
 */
/* Compiler settings for C:\Apps\OleDB\Cases\Provider\OmniProv\OmniProv2.0\TheProvider.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __TheProvider_h__
#define __TheProvider_h__

/* Forward Declarations */ 

#ifndef __MSOmniProv_FWD_DEFINED__
#define __MSOmniProv_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSOmniProv MSOmniProv;
#else
typedef struct MSOmniProv MSOmniProv;
#endif /* __cplusplus */

#endif 	/* __MSOmniProv_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __THEPROVIDERLib_LIBRARY_DEFINED__
#define __THEPROVIDERLib_LIBRARY_DEFINED__

/* library THEPROVIDERLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_THEPROVIDERLib;

EXTERN_C const CLSID CLSID_MSOmniProv;

#ifdef __cplusplus

class DECLSPEC_UUID("B14C8EB2-3632-11D3-AC81-00C04F8DB3D5")
MSOmniProv;
#endif
#endif /* __THEPROVIDERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


