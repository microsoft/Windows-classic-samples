// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
*  MulDiv32.h
*
*  Description:
*      math routines for 32 bit signed and unsiged numbers.
*
*      MulDiv32(a,b,c) = (a * b) / c         (round down, signed)
*
*      MulDivRD(a,b,c) = (a * b) / c         (round down, unsigned)
*      MulDivRN(a,b,c) = (a * b + c/2) / c   (round nearest, unsigned)
*      MulDivRU(a,b,c) = (a * b + c-1) / c   (round up, unsigned)
*
*****************************************************************************/

#ifndef _INC_MULDIV32
#define _INC_MULDIV32


#ifndef INLINE
#define INLINE __inline
#endif


#ifdef _WIN32

    //----------------------------------------------------------------------;
    //
    //  Win 32
    //
    //----------------------------------------------------------------------;

    #ifdef _X86_
    
        //
        //  Use 32-bit x86 assembly.
        //

        #pragma warning(disable:4035 4704)

		#define MulDivRN MulDiv	// Doesn't work with /Ox; MSVC20 messes up 
								// the inline asm code, instead of including 
								// it prolog/verbatim/epilog. 
								// So use the Windows function for now
        #if 0
        INLINE LONG MulDiv32(LONG a,LONG b,LONG c)
        {
            _asm     mov     eax,dword ptr a  //  mov  eax, a
            _asm     mov     ebx,dword ptr b  //  mov  ebx, b
            _asm     mov     ecx,dword ptr c  //  mov  ecx, c
            _asm     imul    ebx              //  imul ebx
            _asm     idiv    ecx              //  idiv ecx
            _asm	 shld	 edx, eax, 16     //  shld edx, eax, 16

        } // MulDiv32()

        INLINE DWORD MulDivRN(DWORD a,DWORD b,DWORD c)
        {
            _asm     mov     eax,dword ptr a  //  mov  eax, a
            _asm     mov     ebx,dword ptr b  //  mov  ebx, b
            _asm     mov     ecx,dword ptr c  //  mov  ecx, c
            _asm     mul     ebx              //  mul  ebx
            _asm     mov     ebx,ecx          //  mov  ebx,ecx
            _asm     shr     ebx,1            //  sar  ebx,1
            _asm     add     eax,ebx          //  add  eax,ebx
            _asm     adc     edx,0            //  adc  edx,0
            _asm     div     ecx              //  div  ecx
            _asm     shld    edx, eax, 16     //  shld edx, eax, 16

        } // MulDiv32()

        INLINE DWORD MulDivRU(DWORD a,DWORD b,DWORD c)
        {
            _asm     mov     eax,dword ptr a  //  mov  eax, a
            _asm     mov     ebx,dword ptr b  //  mov  ebx, b
            _asm     mov     ecx,dword ptr c  //  mov  ecx, c
            _asm     mul     ebx              //  mul  ebx
            _asm     mov     ebx,ecx          //  mov  ebx,ecx
            _asm     dec     ebx              //  dec  ebx
            _asm     add     eax,ebx          //  add  eax,ebx
            _asm     adc     edx,0            //  adc  edx,0
            _asm     div     ecx              //  div  ecx
            _asm     shld    edx, eax, 16     //  shld edx, eax, 16

        } // MulDivRU32()

        INLINE DWORD MulDivRD(DWORD a,DWORD b,DWORD c)
        {
            _asm     mov     eax,dword ptr a  //  mov  eax, a
            _asm     mov     ebx,dword ptr b  //  mov  ebx, b
            _asm     mov     ecx,dword ptr c  //  mov  ecx, c
            _asm     mul     ebx              //  mul  ebx
            _asm     div     ecx              //  div  ecx
            _asm     shld    edx, eax, 16     //  shld edx, eax, 16

        } // MulDivRD32()
        #endif

        #pragma warning(default:4035 4704)


    #else

        //
        //  Use C9 __int64 support for Daytona RISC platforms.
        //

        INLINE LONG MulDiv32( LONG a, LONG b, LONG c )
        {
            return (LONG)( Int32x32To64(a,b) / c );
        }


        INLINE DWORD MulDivRD( DWORD a, DWORD b, DWORD c )
        {
            return (DWORD)( UInt32x32To64(a,b) / c );
        }


        INLINE DWORD MulDivRN( DWORD a, DWORD b, DWORD c )
        {
            return (DWORD)( (UInt32x32To64(a,b)+c/2) / c );
        }


        INLINE DWORD MulDivRU( DWORD a, DWORD b, DWORD c )
        {
            return (DWORD)( (UInt32x32To64(a,b)+c-1) / c );
        }

    #endif


#else

    //----------------------------------------------------------------------;
    //
    //  Win 16
    //
    //----------------------------------------------------------------------;

    #pragma warning(disable:4035 4704)

    //
    //  Compile for 16-bit - we can use x86 with proper opcode prefixes
    //	    to get 32-bit instructions.
    //

    INLINE LONG MulDiv32(LONG a,LONG b,LONG c)
    {
        _asm _emit 0x66 _asm    mov     ax,word ptr a   //  mov  eax, a
        _asm _emit 0x66 _asm    mov     bx,word ptr b   //  mov  ebx, b
        _asm _emit 0x66 _asm    mov     cx,word ptr c   //  mov  ecx, c
        _asm _emit 0x66 _asm    imul    bx              //  imul ebx
        _asm _emit 0x66 _asm    idiv    cx              //  idiv ecx
        _asm _emit 0x66                                 //  shld edx, eax, 16
        _asm _emit 0x0F
        _asm _emit 0xA4
        _asm _emit 0xC2
        _asm _emit 0x10

    } // MulDiv32()

    INLINE DWORD MulDivRN(DWORD a,DWORD b,DWORD c)
    {
        _asm _emit 0x66 _asm    mov     ax,word ptr a   //  mov  eax, a
        _asm _emit 0x66 _asm    mov     bx,word ptr b   //  mov  ebx, b
        _asm _emit 0x66 _asm    mov     cx,word ptr c   //  mov  ecx, c
        _asm _emit 0x66 _asm    mul     bx              //  mul  ebx
        _asm _emit 0x66 _asm    mov     bx,cx           //  mov  ebx,ecx
        _asm _emit 0x66 _asm    shr     bx,1            //  sar  ebx,1
        _asm _emit 0x66 _asm    add     ax,bx           //  add  eax,ebx
        _asm _emit 0x66 _asm    adc     dx,0            //  adc  edx,0
        _asm _emit 0x66 _asm    div     cx              //  div  ecx
        _asm _emit 0x66                                 //  shld edx, eax, 16
        _asm _emit 0x0F
        _asm _emit 0xA4
        _asm _emit 0xC2
        _asm _emit 0x10

    } // MulDiv32()

    INLINE DWORD MulDivRU(DWORD a,DWORD b,DWORD c)
    {
        _asm _emit 0x66 _asm    mov     ax,word ptr a   //  mov  eax, a
        _asm _emit 0x66 _asm    mov     bx,word ptr b   //  mov  ebx, b
        _asm _emit 0x66 _asm    mov     cx,word ptr c   //  mov  ecx, c
        _asm _emit 0x66 _asm    mul     bx              //  mul  ebx
        _asm _emit 0x66 _asm    mov     bx,cx           //  mov  ebx,ecx
        _asm _emit 0x66 _asm    dec     bx              //  dec  ebx
        _asm _emit 0x66 _asm    add     ax,bx           //  add  eax,ebx
        _asm _emit 0x66 _asm    adc     dx,0            //  adc  edx,0
        _asm _emit 0x66 _asm    div     cx              //  div  ecx
        _asm _emit 0x66                                 //  shld edx, eax, 16
        _asm _emit 0x0F
        _asm _emit 0xA4
        _asm _emit 0xC2
        _asm _emit 0x10

    } // MulDivRU32()


    INLINE DWORD MulDivRD(DWORD a,DWORD b,DWORD c)
    {
        _asm _emit 0x66 _asm    mov     ax,word ptr a   //  mov  eax, a
        _asm _emit 0x66 _asm    mov     bx,word ptr b   //  mov  ebx, b
        _asm _emit 0x66 _asm    mov     cx,word ptr c   //  mov  ecx, c
        _asm _emit 0x66 _asm    mul     bx              //  mul  ebx
        _asm _emit 0x66 _asm    div     cx              //  div  ecx
        _asm _emit 0x66                                 //  shld edx, eax, 16
        _asm _emit 0x0F
        _asm _emit 0xA4
        _asm _emit 0xC2
        _asm _emit 0x10

    } // MulDivRD32()

    #pragma warning(default:4035 4704)

#endif


//
//  some code references these by other names.
//
#define muldiv32    MulDivRN
#define muldivrd32  MulDivRD
#define muldivru32  MulDivRU

#endif  // _INC_MULDIV32
