// From Network Programming for Microsoft Windows, Second Edition by 
// Anthony Jones and James Ohlund.  
// Copyright 2002.   Reproduced by permission of Microsoft Press.  
// All rights reserved.
//
//
// Common routines for resolving addresses and hostnames
//
// Files:
//      resolve.h       - Header file for common routines
//
// Description:
//      This file contains common name resolution and name printing
//      routines and is used by many of the samples on this CD.
//
// Compile:
//      See ping.cpp
//
// Usage:
//      See ping.cpp
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
