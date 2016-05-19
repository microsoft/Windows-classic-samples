#ifndef _RCVALL_H_
#define _RCVALL_H_

//
// Size defines
//
#define MAX_IP_SIZE        65535
#define MIN_IP_HDR_SIZE       20

//
// Macros to extract the high and low order 4-bits from a byte
//
#define HI_BYTE(byte)    (((byte) >> 4) & 0x0F)
#define LO_BYTE(byte)    ((byte) & 0x0F)

//
// Used to indicate to parser what fields to filter on
//
#define FILTER_MASK_SOURCE_ADDRESS      0x01
#define FILTER_MASK_SOURCE_PORT         0x02
#define FILTER_MASK_DESTINATION_ADDRESS 0x04
#define FILTER_MASK_DESTINATION_PORT    0x08

// Prints a sequence of raw bytes to the display
void 
PrintRawBytes(
    BYTE *ptr, 
    DWORD len
    );

int  
DecodeIGMPHeader(
    char *buf, 
    DWORD iphdrlen
    );

int  
DecodeUDPHeader(
    char *buf, 
    DWORD iphdrlen
    );

int 
DecodeTCPHeader(
    char *buf, 
    DWORD iphdrlen
    );

int 
DecodeIPHeader(
    char *buf, 
    int buflen,
    unsigned long filtermask, 
    SOCKADDR *srcfilter, 
    SOCKADDR *destfilter
    );

#endif
