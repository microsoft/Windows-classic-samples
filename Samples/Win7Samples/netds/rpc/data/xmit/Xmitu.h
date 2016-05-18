/****************************************************************************
                   Microsoft RPC Version 2.0
           Copyright Microsoft Corp. 1992 - 2000
                        xmit Example

    FILE:       xmitu.h

    PURPOSE:    Function prototypes for functions in xmitu.c

****************************************************************************/

#ifndef _XMITU_H_
#define _XMITU_H_


DOUBLE_LINK_TYPE *
InsertNewNode(
    short sValue,
    DOUBLE_LINK_TYPE * pPrevious
    );

void
ArrayWalkProc(
    DOUBLE_XMIT_TYPE * pArray
    );

void
ListWalkProc(
    DOUBLE_LINK_TYPE * pList
    );


void __RPC_USER
DOUBLE_LINK_TYPE_to_xmit(
     DOUBLE_LINK_TYPE __RPC_FAR *             pList,
     DOUBLE_XMIT_TYPE __RPC_FAR * __RPC_FAR * ppArray
     );

void __RPC_USER
DOUBLE_LINK_TYPE_from_xmit(
     DOUBLE_XMIT_TYPE __RPC_FAR * pArray,
     DOUBLE_LINK_TYPE __RPC_FAR * pList);

void __RPC_USER
DOUBLE_LINK_TYPE_free_inst(
    DOUBLE_LINK_TYPE __RPC_FAR * pList
    );

void __RPC_USER
DOUBLE_LINK_TYPE_free_xmit(
    DOUBLE_XMIT_TYPE __RPC_FAR * pArray
    );


void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len);

void __RPC_USER midl_user_free(void __RPC_FAR * ptr);


#endif

/* end file xmitu.h */
