//
// This file contains function prototypes for resolve.cpp
//
// Description:
//    These are common routines for resolving and printing IPv4 and IPv6
//    addresses.
//
// NOTE:
//    From Network Programming for Microsoft Windows, Second Edition 
//    by Anthony Jones and James Ohlund.  Copyright 2002.  
//    Reproduced by permission of Microsoft Press.  All rights reserved. 
//
#ifndef _RESOLVE_H_
#define _RESOLVE_H_

#ifdef _cplusplus
extern "C" {
#endif

int              PrintAddress(SOCKADDR *sa, int salen);
int              FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen);
int              ReverseLookup(SOCKADDR *sa, int salen, char *namebuf, int namebuflen);
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto);

#ifdef _cplusplus
}
#endif

#endif
