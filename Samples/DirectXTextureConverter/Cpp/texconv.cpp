//--------------------------------------------------------------------------------------
// File: TexConv.cpp
//
// DirectX 11 Texture Converer
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <dxgiformat.h>

#include "directxtex.h"

using namespace DirectX;

enum OPTIONS    // Note: dwOptions below assumes 32 or less options.
{
    OPT_WIDTH = 1,
    OPT_HEIGHT,
    OPT_MIPLEVELS,
    OPT_FORMAT,
    OPT_FILTER,
    OPT_SRGBI,
    OPT_SRGBO,
    OPT_SRGB,
    OPT_PREFIX,
    OPT_SUFFIX,
    OPT_OUTPUTDIR,
    OPT_FILETYPE,
    OPT_HFLIP,
    OPT_VFLIP,
    OPT_DDS_DWORD_ALIGN,
    OPT_USE_DX10,
    OPT_NOLOGO,
    OPT_SEPALPHA,
    OPT_TYPELESS_UNORM,
    OPT_TYPELESS_FLOAT,
    OPT_PREMUL_ALPHA,
    OPT_EXPAND_LUMINANCE,
    OPT_TA_WRAP,
    OPT_TA_MIRROR,
    OPT_FORCE_SINGLEPROC,
};

struct SConversion
{
    WCHAR szSrc [MAX_PATH];
    WCHAR szDest[MAX_PATH];

    SConversion *pNext;
};

struct SValue
{
    LPCWSTR pName;
    DWORD dwValue;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SValue g_pOptions[] = 
{
    { L"w",             OPT_WIDTH     },
    { L"h",             OPT_HEIGHT    },
    { L"m",             OPT_MIPLEVELS },
    { L"f",             OPT_FORMAT    },
    { L"if",            OPT_FILTER    },
    { L"srgbi",         OPT_SRGBI     },
    { L"srgbo",         OPT_SRGBO     },
    { L"srgb",          OPT_SRGB      },
    { L"px",            OPT_PREFIX    },
    { L"sx",            OPT_SUFFIX    },
    { L"o",             OPT_OUTPUTDIR },
    { L"ft",            OPT_FILETYPE  },
    { L"hflip",         OPT_HFLIP     },
    { L"vflip",         OPT_VFLIP     },
    { L"dword",         OPT_DDS_DWORD_ALIGN },
    { L"dx10",          OPT_USE_DX10  },
    { L"nologo",        OPT_NOLOGO    },
    { L"sepalpha",      OPT_SEPALPHA  },
    { L"tu",            OPT_TYPELESS_UNORM },
    { L"tf",            OPT_TYPELESS_FLOAT },
    { L"pmalpha",       OPT_PREMUL_ALPHA },
    { L"xlum",          OPT_EXPAND_LUMINANCE },
    { L"wrap",          OPT_TA_WRAP },
    { L"mirror",        OPT_TA_MIRROR },
    { L"singleproc",    OPT_FORCE_SINGLEPROC },
    { nullptr,          0             }
};

#define DEFFMT(fmt) { L#fmt, DXGI_FORMAT_ ## fmt }

SValue g_pFormats[] = 
{
    // List does not include _TYPELESS or depth/stencil formats
    DEFFMT(R32G32B32A32_FLOAT), 
    DEFFMT(R32G32B32A32_UINT), 
    DEFFMT(R32G32B32A32_SINT), 
    DEFFMT(R32G32B32_FLOAT), 
    DEFFMT(R32G32B32_UINT), 
    DEFFMT(R32G32B32_SINT), 
    DEFFMT(R16G16B16A16_FLOAT), 
    DEFFMT(R16G16B16A16_UNORM), 
    DEFFMT(R16G16B16A16_UINT), 
    DEFFMT(R16G16B16A16_SNORM), 
    DEFFMT(R16G16B16A16_SINT), 
    DEFFMT(R32G32_FLOAT), 
    DEFFMT(R32G32_UINT), 
    DEFFMT(R32G32_SINT), 
    DEFFMT(R10G10B10A2_UNORM), 
    DEFFMT(R10G10B10A2_UINT), 
    DEFFMT(R11G11B10_FLOAT), 
    DEFFMT(R8G8B8A8_UNORM), 
    DEFFMT(R8G8B8A8_UNORM_SRGB), 
    DEFFMT(R8G8B8A8_UINT), 
    DEFFMT(R8G8B8A8_SNORM), 
    DEFFMT(R8G8B8A8_SINT), 
    DEFFMT(R16G16_FLOAT), 
    DEFFMT(R16G16_UNORM), 
    DEFFMT(R16G16_UINT), 
    DEFFMT(R16G16_SNORM), 
    DEFFMT(R16G16_SINT), 
    DEFFMT(R32_FLOAT), 
    DEFFMT(R32_UINT), 
    DEFFMT(R32_SINT), 
    DEFFMT(R8G8_UNORM), 
    DEFFMT(R8G8_UINT), 
    DEFFMT(R8G8_SNORM), 
    DEFFMT(R8G8_SINT), 
    DEFFMT(R16_FLOAT), 
    DEFFMT(R16_UNORM), 
    DEFFMT(R16_UINT), 
    DEFFMT(R16_SNORM), 
    DEFFMT(R16_SINT), 
    DEFFMT(R8_UNORM), 
    DEFFMT(R8_UINT), 
    DEFFMT(R8_SNORM), 
    DEFFMT(R8_SINT), 
    DEFFMT(A8_UNORM), 
    //DEFFMT(R1_UNORM)
    DEFFMT(R9G9B9E5_SHAREDEXP), 
    DEFFMT(R8G8_B8G8_UNORM), 
    DEFFMT(G8R8_G8B8_UNORM), 
    DEFFMT(BC1_UNORM), 
    DEFFMT(BC1_UNORM_SRGB), 
    DEFFMT(BC2_UNORM), 
    DEFFMT(BC2_UNORM_SRGB), 
    DEFFMT(BC3_UNORM), 
    DEFFMT(BC3_UNORM_SRGB), 
    DEFFMT(BC4_UNORM), 
    DEFFMT(BC4_SNORM), 
    DEFFMT(BC5_UNORM), 
    DEFFMT(BC5_SNORM),
    DEFFMT(B5G6R5_UNORM),
    DEFFMT(B5G5R5A1_UNORM),

    // DXGI 1.1 formats
    DEFFMT(B8G8R8A8_UNORM),
    DEFFMT(B8G8R8X8_UNORM),
    DEFFMT(R10G10B10_XR_BIAS_A2_UNORM),
    DEFFMT(B8G8R8A8_UNORM_SRGB),
    DEFFMT(B8G8R8X8_UNORM_SRGB),
    DEFFMT(BC6H_UF16),
    DEFFMT(BC6H_SF16),
    DEFFMT(BC7_UNORM),
    DEFFMT(BC7_UNORM_SRGB),

    // DXGI 1.2 formats
    DEFFMT(B4G4R4A4_UNORM),

    { nullptr, DXGI_FORMAT_UNKNOWN }
};

SValue g_pFilters[] = 
{
    { L"POINT",                     TEX_FILTER_POINT },
    { L"LINEAR",                    TEX_FILTER_LINEAR },
    { L"CUBIC",                     TEX_FILTER_CUBIC },
    { L"FANT",                      TEX_FILTER_FANT },
    { L"BOX",                       TEX_FILTER_BOX },
    { L"TRIANGLE",                  TEX_FILTER_TRIANGLE },
    { L"POINT_DITHER",              TEX_FILTER_POINT  | TEX_FILTER_DITHER },
    { L"LINEAR_DITHER",             TEX_FILTER_LINEAR | TEX_FILTER_DITHER },
    { L"CUBIC_DITHER",              TEX_FILTER_CUBIC  | TEX_FILTER_DITHER },
    { L"FANT_DITHER",               TEX_FILTER_FANT   | TEX_FILTER_DITHER },
    { L"BOX_DITHER",                TEX_FILTER_BOX    | TEX_FILTER_DITHER },
    { L"TRIANGLE_DITHER",           TEX_FILTER_TRIANGLE | TEX_FILTER_DITHER },
    { L"POINT_DITHER_DIFFUSION",    TEX_FILTER_POINT  | TEX_FILTER_DITHER_DIFFUSION },
    { L"LINEAR_DITHER_DIFFUSION",   TEX_FILTER_LINEAR | TEX_FILTER_DITHER_DIFFUSION },
    { L"CUBIC_DITHER_DIFFUSION",    TEX_FILTER_CUBIC  | TEX_FILTER_DITHER_DIFFUSION },
    { L"FANT_DITHER_DIFFUSION",     TEX_FILTER_FANT   | TEX_FILTER_DITHER_DIFFUSION },
    { L"BOX_DITHER_DIFFUSION",      TEX_FILTER_BOX    | TEX_FILTER_DITHER_DIFFUSION },
    { L"TRIANGLE_DITHER_DIFFUSION", TEX_FILTER_TRIANGLE | TEX_FILTER_DITHER_DIFFUSION },
    { nullptr,                      TEX_FILTER_DEFAULT                              }
};

#define CODEC_DDS 0xFFFF0001 
#define CODEC_TGA 0xFFFF0002

SValue g_pSaveFileTypes[] =     // valid formats to write to
{
    { L"BMP",           WIC_CODEC_BMP  },
    { L"JPG",           WIC_CODEC_JPEG },
    { L"JPEG",          WIC_CODEC_JPEG },
    { L"PNG",           WIC_CODEC_PNG  },
    { L"DDS",           CODEC_DDS      },
    { L"TGA",           CODEC_TGA      },
    { L"TIF",           WIC_CODEC_TIFF },
    { L"TIFF",          WIC_CODEC_TIFF },
    { L"WDP",           WIC_CODEC_WMP  },
    { L"HDP",           WIC_CODEC_WMP  },
    { nullptr,          CODEC_DDS      }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#pragma warning( disable : 6211 )

inline static bool ispow2(size_t x)
{
    return ((x != 0) && !(x & (x - 1)));
}

#pragma prefast(disable : 26018, "Only used with static internal arrays")

DWORD LookupByName(const WCHAR *pName, const SValue *pArray)
{
    while(pArray->pName)
    {
        if(!_wcsicmp(pName, pArray->pName))
            return pArray->dwValue;

        pArray++;
    }

    return 0;
}

const WCHAR* LookupByValue(DWORD pValue, const SValue *pArray)
{
    while(pArray->pName)
    {
        if(pValue == pArray->dwValue)
            return pArray->pName;

        pArray++;
    }

    return L"";
}

void PrintFormat(DXGI_FORMAT Format)
{
    for(SValue *pFormat = g_pFormats; pFormat->pName; pFormat++)
    {
        if((DXGI_FORMAT) pFormat->dwValue == Format)
        {
            wprintf( pFormat->pName );
            break;
        }
    }
}

void PrintInfo( const TexMetadata& info )
{
    wprintf( L" (%Iux%Iu", info.width, info.height);

    if ( TEX_DIMENSION_TEXTURE3D == info.dimension )
        wprintf( L"x%Iu", info.depth);

    if ( info.mipLevels > 1 )
        wprintf( L",%Iu", info.mipLevels);

    wprintf( L" ");
    PrintFormat( info.format );

    switch ( info.dimension )
    {
    case TEX_DIMENSION_TEXTURE1D:
        wprintf( (info.arraySize > 1) ? L" 1DArray" : L" 1D" );
        break;

    case TEX_DIMENSION_TEXTURE2D:
        if ( info.IsCubemap() )
        {
            wprintf( (info.arraySize > 6) ? L" CubeArray" : L" Cube" );
        }
        else
        {
            wprintf( (info.arraySize > 1) ? L" 2DArray" : L" 2D" );
        }
        break;

    case TEX_DIMENSION_TEXTURE3D:
        wprintf( L" 3D");
        break;
    }

    wprintf( L")");
}


void PrintList(size_t cch, SValue *pValue)
{
    while(pValue->pName)
    {
        size_t cchName = wcslen(pValue->pName);
        
        if(cch + cchName + 2>= 80)
        {
            wprintf( L"\n      ");
            cch = 6;
        }

        wprintf( L"%s ", pValue->pName );
        cch += cchName + 2;
        pValue++;
    }

    wprintf( L"\n");
}


void PrintLogo()
{
    wprintf( L"Microsoft (R) DirectX 11 Texture Converter (DirectXTex version)\n");
    wprintf( L"Copyright (C) Microsoft Corp. All rights reserved.\n");
    wprintf( L"\n");
}


void PrintUsage()
{
    PrintLogo();

    wprintf( L"Usage: texconv <options> <files>\n");
    wprintf( L"\n");
    wprintf( L"   -w <n>              width\n");
    wprintf( L"   -h <n>              height\n");
    wprintf( L"   -d <n>              depth\n");
    wprintf( L"   -m <n>              miplevels\n");
    wprintf( L"   -f <format>         format\n");
    wprintf( L"   -if <filter>        image filtering\n");
    wprintf( L"   -srgb{i|o}          sRGB {input, output}\n");
    wprintf( L"   -px <string>        name prefix\n");
    wprintf( L"   -sx <string>        name suffix\n");
    wprintf( L"   -o <directory>      output directory\n");
    wprintf( L"   -ft <filetype>      output file type\n");
    wprintf( L"   -hflip              horizonal flip of source image\n");
    wprintf( L"   -vflip              vertical flip of source image\n");
    wprintf( L"   -sepalpha           resize/generate mips alpha channel separately\n");
    wprintf( L"                       from color channels\n");
    wprintf( L"   -wrap, -mirror      texture addressing mode (wrap, mirror, or clamp)\n");
    wprintf( L"   -pmalpha            convert final texture to use premultiplied alpha\n");
    wprintf( L"\n                       (DDS input only)\n");
    wprintf( L"   -t{u|f}             TYPELESS format is treated as UNORM or FLOAT\n");
    wprintf( L"   -dword              Use DWORD instead of BYTE alignment\n");
    wprintf( L"   -xlum               expand legacy L8, L16, and A8P8 formats\n");
    wprintf( L"\n                       (DDS output only)\n");
    wprintf( L"   -dx10               Force use of 'DX10' extended header\n");
    wprintf( L"\n   -nologo             suppress copyright message\n");
#ifdef _OPENMP
    wprintf( L"   -singleproc         Do not use multi-threaded compression\n");
#endif

    wprintf( L"\n");
    wprintf( L"   <format>: ");
    PrintList(13, g_pFormats);

    wprintf( L"\n");
    wprintf( L"   <filter>: ");
    PrintList(13, g_pFilters);

    wprintf( L"\n");
    wprintf( L"   <filetype>: ");
    PrintList(15, g_pSaveFileTypes);
}


//--------------------------------------------------------------------------------------
// Entry-point
//--------------------------------------------------------------------------------------
#pragma prefast(disable : 28198, "Command-line tool, frees all memory on exit")

int __cdecl wmain(_In_ int argc, _In_z_count_(argc) wchar_t* argv[])
{
    // Parameters and defaults
    HRESULT hr;
    INT nReturn;

    size_t width = 0;
    size_t height = 0; 
    size_t mipLevels = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    DWORD dwFilter = TEX_FILTER_DEFAULT;
    DWORD dwSRGB = 0;
    DWORD dwFilterOpts = 0;
    DWORD FileType = CODEC_DDS;

    WCHAR szPrefix   [MAX_PATH];
    WCHAR szSuffix   [MAX_PATH];
    WCHAR szOutputDir[MAX_PATH];

    szPrefix[0]    = 0;
    szSuffix[0]    = 0;
    szOutputDir[0] = 0;

    // Initialize COM (needed for WIC)
    if( FAILED( hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED) ) )
    {
        wprintf( L"Failed to initialize COM (%08X)\n", hr);
        return 1;
    }

    // Process command line
    DWORD dwOptions = 0;
    SConversion *pConversion = nullptr;
    SConversion **ppConversion = &pConversion;

    for(int iArg = 1; iArg < argc; iArg++)
    {
        PWSTR pArg = argv[iArg];

        if(('-' == pArg[0]) || ('/' == pArg[0]))
        {
            pArg++;
            PWSTR pValue;

            for(pValue = pArg; *pValue && (':' != *pValue); pValue++);

            if(*pValue)
                *pValue++ = 0;

            DWORD dwOption = LookupByName(pArg, g_pOptions);

            if(!dwOption || (dwOptions & (1 << dwOption)))
            {
                PrintUsage();
                return 1;
            }

            dwOptions |= 1 << dwOption;

            if( (OPT_NOLOGO != dwOption) && (OPT_TYPELESS_UNORM != dwOption) && (OPT_TYPELESS_FLOAT != dwOption)
                && (OPT_SEPALPHA != dwOption) && (OPT_PREMUL_ALPHA != dwOption) && (OPT_EXPAND_LUMINANCE != dwOption)
                && (OPT_TA_WRAP != dwOption) && (OPT_TA_MIRROR != dwOption)
                && (OPT_FORCE_SINGLEPROC != dwOption)
                && (OPT_SRGB != dwOption) && (OPT_SRGBI != dwOption) && (OPT_SRGBO != dwOption)
                && (OPT_HFLIP != dwOption) && (OPT_VFLIP != dwOption)
                && (OPT_DDS_DWORD_ALIGN != dwOption) && (OPT_USE_DX10 != dwOption) )
            {
                if(!*pValue)
                {
                    if((iArg + 1 >= argc))
                    {
                        PrintUsage();
                        return 1;
                    }

                    iArg++;
                    pValue = argv[iArg];
                }
            }

            switch(dwOption)
            {
            case OPT_WIDTH:
                if (swscanf_s(pValue, L"%Iu", &width) != 1)
                {
                    wprintf( L"Invalid value specified with -w (%s)\n", pValue);
                    wprintf( L"\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_HEIGHT:
                if (swscanf_s(pValue, L"%Iu", &height) != 1)
                {
                    wprintf( L"Invalid value specified with -h (%s)\n", pValue);
                    printf("\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_MIPLEVELS:
                if (swscanf_s(pValue, L"%Iu", &mipLevels) != 1)
                {
                    wprintf( L"Invalid value specified with -m (%s)\n", pValue);
                    wprintf( L"\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_FORMAT:
                format = (DXGI_FORMAT) LookupByName(pValue, g_pFormats);
                if ( !format )
                {
                    wprintf( L"Invalid value specified with -f (%s)\n", pValue);
                    wprintf( L"\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_FILTER:
                dwFilter = LookupByName(pValue, g_pFilters);
                if ( !dwFilter )
                {
                    wprintf( L"Invalid value specified with -if (%s)\n", pValue);
                    wprintf( L"\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_SRGBI:
                dwSRGB |= TEX_FILTER_SRGB_IN;
                break;

            case OPT_SRGBO:
                dwSRGB |= TEX_FILTER_SRGB_OUT;
                break;

            case OPT_SRGB:
                dwSRGB |= TEX_FILTER_SRGB;
                break;

            case OPT_SEPALPHA:
                dwFilterOpts |= TEX_FILTER_SEPARATE_ALPHA;
                break;

            case OPT_PREFIX:
                wcscpy_s(szPrefix, MAX_PATH, pValue);
                break;

            case OPT_SUFFIX:
                wcscpy_s(szSuffix, MAX_PATH, pValue);
                break;

            case OPT_OUTPUTDIR:
                wcscpy_s(szOutputDir, MAX_PATH, pValue);
                break;

            case OPT_FILETYPE:
                FileType = LookupByName(pValue, g_pSaveFileTypes);
                if ( !FileType )
                {
                    wprintf( L"Invalid value specified with -ft (%s)\n", pValue);
                    wprintf( L"\n");
                    PrintUsage();
                    return 1;
                }
                break;

            case OPT_TA_WRAP:
                if ( dwFilterOpts & TEX_FILTER_MIRROR )
                {
                    wprintf( L"Can't use -wrap and -mirror at same time\n\n");
                    PrintUsage();
                    return 1;
                }
                dwFilterOpts |= TEX_FILTER_WRAP;
                break;

            case OPT_TA_MIRROR:
                if ( dwFilterOpts & TEX_FILTER_WRAP )
                {
                    wprintf( L"Can't use -wrap and -mirror at same time\n\n");
                    PrintUsage();
                    return 1;
                }
                dwFilterOpts |= TEX_FILTER_MIRROR;
                break;
            }
        }
        else
        {         
            SConversion *pConv = new SConversion;
            if ( !pConv )
                return 1;

            wcscpy_s(pConv->szSrc, MAX_PATH, pArg);

            pConv->szDest[0] = 0;
            pConv->pNext = nullptr;

            *ppConversion = pConv;
            ppConversion = &pConv->pNext;
        }
    }

    if(!pConversion)
    {
        PrintUsage();
        return 0;
    }

    if(~dwOptions & (1 << OPT_NOLOGO))
        PrintLogo();

    // Work out out filename prefix and suffix
    if(szOutputDir[0] && (L'\\' != szOutputDir[wcslen(szOutputDir) - 1]))
        wcscat_s( szOutputDir, MAX_PATH, L"\\" );

    if(szPrefix[0])
        wcscat_s(szOutputDir, MAX_PATH, szPrefix);

    wcscpy_s(szPrefix, MAX_PATH, szOutputDir);

    const WCHAR* fileTypeName = LookupByValue(FileType, g_pSaveFileTypes);

    if (fileTypeName)
    {
        wcscat_s(szSuffix, MAX_PATH, L".");
        wcscat_s(szSuffix, MAX_PATH, fileTypeName);
    }
    else
    {
        wcscat_s(szSuffix, MAX_PATH, L".unknown");
    }

    if (FileType != CODEC_DDS)
    {
        mipLevels = 1;
    }

    // Convert images
    bool nonpow2warn = false;
    SConversion *pConv;

    for(pConv = pConversion; pConv; pConv = pConv->pNext)
    {
        // Load source image
        if(pConv != pConversion)
            wprintf( L"\n");

        wprintf( L"reading %s", pConv->szSrc );
        fflush(stdout);

        WCHAR ext[_MAX_EXT];
        WCHAR fname[_MAX_FNAME];
        _wsplitpath_s( pConv->szSrc, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT );

        TexMetadata info;
        ScratchImage *image = new ScratchImage;

        if ( !image )
        {
            wprintf( L" ERROR: Memory allocation failed\n" );
            goto LError;
        }

        if ( _wcsicmp( ext, L".dds" ) == 0 )
        {
            DWORD ddsFlags = DDS_FLAGS_NONE;
            if ( dwOptions & (1 << OPT_DDS_DWORD_ALIGN) )
                ddsFlags |= DDS_FLAGS_LEGACY_DWORD;
            if ( dwOptions & (1 << OPT_EXPAND_LUMINANCE) )
                ddsFlags |= DDS_FLAGS_EXPAND_LUMINANCE;

            hr = LoadFromDDSFile( pConv->szSrc, ddsFlags, &info, *image );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED (%x)\n", hr);
                delete image;
                continue;
            }

            if ( IsTypeless( info.format ) )
            {
                if ( dwOptions & (1 << OPT_TYPELESS_UNORM) )
                {
                    info.format = MakeTypelessUNORM( info.format );
                }
                else if ( dwOptions & (1 << OPT_TYPELESS_FLOAT) )
                {
                    info.format = MakeTypelessFLOAT( info.format );
                }

                if ( IsTypeless( info.format ) )
                {
                    wprintf( L" FAILED due to Typeless format %d\n", info.format );
                    delete image;
                    continue;
                }

                image->OverrideFormat( info.format );
            }
        }
        else if ( _wcsicmp( ext, L".tga" ) == 0 )
        {
            hr = LoadFromTGAFile( pConv->szSrc, &info, *image );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED (%x)\n", hr);
                delete image;
                continue;
            }
        }
        else
        {
            // WIC shares the same filter values for mode and dither
            static_assert( WIC_FLAGS_DITHER == TEX_FILTER_DITHER, "WIC_FLAGS_* & TEX_FILTER_* should match" );
            static_assert( WIC_FLAGS_DITHER_DIFFUSION == TEX_FILTER_DITHER_DIFFUSION, "WIC_FLAGS_* & TEX_FILTER_* should match"  );
            static_assert( WIC_FLAGS_FILTER_POINT == TEX_FILTER_POINT, "WIC_FLAGS_* & TEX_FILTER_* should match"  );
            static_assert( WIC_FLAGS_FILTER_LINEAR == TEX_FILTER_LINEAR, "WIC_FLAGS_* & TEX_FILTER_* should match"  );
            static_assert( WIC_FLAGS_FILTER_CUBIC == TEX_FILTER_CUBIC, "WIC_FLAGS_* & TEX_FILTER_* should match"  );
            static_assert( WIC_FLAGS_FILTER_FANT == TEX_FILTER_FANT, "WIC_FLAGS_* & TEX_FILTER_* should match"  );

            hr = LoadFromWICFile( pConv->szSrc, dwFilter, &info, *image );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED (%x)\n", hr);
                delete image;
                continue;
            }
        }

        PrintInfo( info );

        size_t twidth = ( !width ) ? info.width : width;
        size_t theight = ( !height ) ? info.height : height;
        size_t tMips = ( !mipLevels && info.mipLevels > 1 ) ? info.mipLevels : mipLevels;
        DXGI_FORMAT tformat = ( format == DXGI_FORMAT_UNKNOWN ) ? info.format : format;

        // Convert texture
        wprintf( L" as");
        fflush(stdout);

        // --- Decompress --------------------------------------------------------------
        if ( IsCompressed( info.format ) )
        {
            const Image* img = image->GetImage(0,0,0);
            assert( img );
            size_t nimg = image->GetImageCount();

            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            hr = Decompress( img, nimg, info, DXGI_FORMAT_UNKNOWN /* picks good default */, *timage );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [decompress] (%x)\n", hr);
                delete timage;
                delete image;
                continue;
            }

            const TexMetadata& tinfo = timage->GetMetadata();

            info.format = tinfo.format;

            assert( info.width == tinfo.width );
            assert( info.height == tinfo.height );
            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.mipLevels == tinfo.mipLevels );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Flip/Rotate -------------------------------------------------------------
        if ( dwOptions & ( (1 << OPT_HFLIP) | (1 << OPT_VFLIP) ) )
        {
            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            DWORD dwFlags = 0;

            if ( dwOptions & (1 << OPT_HFLIP) )
                dwFlags |= TEX_FR_FLIP_HORIZONTAL;

            if ( dwOptions & (1 << OPT_VFLIP) )
                dwFlags |= TEX_FR_FLIP_VERTICAL;

            assert( dwFlags != 0 );

            hr = FlipRotate( image->GetImages(), image->GetImageCount(), image->GetMetadata(), dwFlags, *timage );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [fliprotate] (%x)\n", hr);
                delete timage;
                delete image;
                goto LError;
            }

            const TexMetadata& tinfo = timage->GetMetadata();

            assert( tinfo.width == twidth && tinfo.height == theight );

            info.width = tinfo.width;
            info.height = tinfo.height;

            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.mipLevels == tinfo.mipLevels );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.format == tinfo.format );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Resize ------------------------------------------------------------------
        if ( info.width != twidth || info.height != theight )
        {
            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            hr = Resize( image->GetImages(), image->GetImageCount(), image->GetMetadata(), twidth, theight, dwFilter | dwFilterOpts, *timage );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [resize] (%x)\n", hr);
                delete timage;
                delete image;
                goto LError;
            }

            const TexMetadata& tinfo = timage->GetMetadata();

            assert( tinfo.width == twidth && tinfo.height == theight && tinfo.mipLevels == 1 );
            info.width = tinfo.width;
            info.height = tinfo.height;
            info.mipLevels = 1;

            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.format == tinfo.format );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Convert -----------------------------------------------------------------
        if ( info.format != tformat && !IsCompressed( tformat ) )
        {
            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            hr = Convert( image->GetImages(), image->GetImageCount(), image->GetMetadata(), tformat, dwFilter | dwFilterOpts | dwSRGB, 0.5f, *timage );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [convert] (%x)\n", hr);
                delete timage;
                delete image;
                goto LError;
            }

            const TexMetadata& tinfo = timage->GetMetadata();

            assert( tinfo.format == tformat );
            info.format = tinfo.format;

            assert( info.width == tinfo.width );
            assert( info.height == tinfo.height );
            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.mipLevels == tinfo.mipLevels );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Generate mips -----------------------------------------------------------
        if ( !ispow2(info.width) || !ispow2(info.height) || !ispow2(info.depth) )
        {
            if ( info.dimension == TEX_DIMENSION_TEXTURE3D )
            {
                if ( !tMips )
                {
                    tMips = 1;
                }
                else
                {
                    wprintf( L" ERROR: Cannot generate mips for non-power-of-2 volume textures\n" );
                    delete image;
                    goto LError;
                }
            }
            else if ( !tMips || info.mipLevels != 1 )
            {
                nonpow2warn = true;
            }
        }

        if ( (!tMips || info.mipLevels != tMips) && ( info.mipLevels != 1 ) )
        {
            // Mips generation only works on a single base image, so strip off existing mip levels
            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            TexMetadata mdata = info;
            mdata.mipLevels = 1;
            hr = timage->Initialize( mdata );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [copy to single level] (%x)\n", hr);
                delete timage;
                delete image;
                goto LError;
            }

            if ( info.dimension == TEX_DIMENSION_TEXTURE3D )
            {
                for( size_t d = 0; d < info.depth; ++d )
                {
                    hr = CopyRectangle( *image->GetImage( 0, 0, d ), Rect( 0, 0, info.width, info.height ),
                                        *timage->GetImage( 0, 0, d ), TEX_FILTER_DEFAULT, 0, 0 );
                    if ( FAILED(hr) )
                    {
                        wprintf( L" FAILED [copy to single level] (%x)\n", hr);
                        delete timage;
                        delete image;
                        goto LError;
                    }
                }
            }
            else
            {
                for( size_t i = 0; i < info.arraySize; ++i )
                {
                    hr = CopyRectangle( *image->GetImage( 0, i, 0 ), Rect( 0, 0, info.width, info.height ),
                                        *timage->GetImage( 0, i, 0 ), TEX_FILTER_DEFAULT, 0, 0 );
                    if ( FAILED(hr) )
                    {
                        wprintf( L" FAILED [copy to single level] (%x)\n", hr);
                        delete timage;
                        delete image;
                        goto LError;
                    }
                }
            }

            delete image;
            image = timage;

            const TexMetadata& tinfo = timage->GetMetadata();
            info.mipLevels = tinfo.mipLevels;
        }

        if ( !tMips || info.mipLevels != tMips )
        {
            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            if ( info.dimension == TEX_DIMENSION_TEXTURE3D )
            {
                hr = GenerateMipMaps3D( image->GetImages(), image->GetImageCount(), image->GetMetadata(), dwFilter | dwFilterOpts, tMips, *timage );
            }
            else
            {
                hr = GenerateMipMaps( image->GetImages(), image->GetImageCount(), image->GetMetadata(), dwFilter | dwFilterOpts, tMips, *timage );
            }
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [mipmaps] (%x)\n", hr);
                delete timage;
                delete image;
                goto LError;
            }

            const TexMetadata& tinfo = timage->GetMetadata();
            info.mipLevels = tinfo.mipLevels;

            assert( info.width == tinfo.width );
            assert( info.height == tinfo.height );
            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.mipLevels == tinfo.mipLevels );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Premultiplied alpha (if requested) --------------------------------------
        if ( ( dwOptions & (1 << OPT_PREMUL_ALPHA) )
             && HasAlpha( info.format )
             && info.format != DXGI_FORMAT_A8_UNORM )
        {
            if ( info.IsPMAlpha() )
            {
                printf("WARNING: Image is already using premultiplied alpha\n");
            }
            else
            {
                const Image* img = image->GetImage(0,0,0);
                assert( img );
                size_t nimg = image->GetImageCount();

                ScratchImage *timage = new ScratchImage;
                if ( !timage )
                {
                    wprintf( L" ERROR: Memory allocation failed\n" );
                    delete image;
                    goto LError;
                }

                hr = PremultiplyAlpha( img, nimg, info, *timage );
                if ( FAILED(hr) )
                {
                    wprintf( L" FAILED [premultiply alpha] (%x)\n", hr);
                    delete timage;
                    delete image;
                    continue;
                }

                const TexMetadata& tinfo = timage->GetMetadata();
                info.miscFlags2 = tinfo.miscFlags2;
 
                assert( info.width == tinfo.width );
                assert( info.height == tinfo.height );
                assert( info.depth == tinfo.depth );
                assert( info.arraySize == tinfo.arraySize );
                assert( info.mipLevels == tinfo.mipLevels );
                assert( info.miscFlags == tinfo.miscFlags );
                assert( info.miscFlags2 == tinfo.miscFlags2 );
                assert( info.dimension == tinfo.dimension );

                delete image;
                image = timage;
            }
        }

        // --- Compress ----------------------------------------------------------------
        if ( IsCompressed( tformat ) && (FileType == CODEC_DDS) )
        {
            const Image* img = image->GetImage(0,0,0);
            assert( img );
            size_t nimg = image->GetImageCount();

            ScratchImage *timage = new ScratchImage;
            if ( !timage )
            {
                wprintf( L" ERROR: Memory allocation failed\n" );
                delete image;
                goto LError;
            }

            DWORD cflags = TEX_COMPRESS_DEFAULT;
#ifdef _OPENMP
            switch( tformat )
            {
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                if ( !(dwOptions & (1 << OPT_FORCE_SINGLEPROC) ) )
                {
                    cflags |= TEX_COMPRESS_PARALLEL;
                }
                break;
            }
#endif

            hr = Compress( img, nimg, info, tformat, cflags, 0.5f, *timage );
            if ( FAILED(hr) )
            {
                wprintf( L" FAILED [compress] (%x)\n", hr);
                delete timage;
                delete image;
                continue;
            }

            const TexMetadata& tinfo = timage->GetMetadata();

            info.format = tinfo.format;
            assert( info.width == tinfo.width );
            assert( info.height == tinfo.height );
            assert( info.depth == tinfo.depth );
            assert( info.arraySize == tinfo.arraySize );
            assert( info.mipLevels == tinfo.mipLevels );
            assert( info.miscFlags == tinfo.miscFlags );
            assert( info.miscFlags2 == tinfo.miscFlags2 );
            assert( info.dimension == tinfo.dimension );

            delete image;
            image = timage;
        }

        // --- Set alpha mode ----------------------------------------------------------
        if ( HasAlpha( info.format )
             && info.format != DXGI_FORMAT_A8_UNORM )
        {
            if ( image->IsAlphaAllOpaque() )
            {
                info.SetAlphaMode(TEX_ALPHA_MODE_OPAQUE);
            }
            else if ( info.IsPMAlpha() )
            {
                // Aleady set TEX_ALPHA_MODE_PREMULTIPLIED
            }
            else if ( dwOptions & (1 << OPT_SEPALPHA) )
            {
                info.SetAlphaMode(TEX_ALPHA_MODE_CUSTOM);
            }
            else
            {
                info.SetAlphaMode(TEX_ALPHA_MODE_STRAIGHT);
            }
        }
        else
        {
            info.miscFlags2 &= ~TEX_MISC2_ALPHA_MODE_MASK;
        }

        // --- Save result -------------------------------------------------------------
        {
            const Image* img = image->GetImage(0,0,0);
            assert( img );
            size_t nimg = image->GetImageCount();

            PrintInfo( info );
            wprintf( L"\n");

            // Figure out dest filename
            WCHAR *pchSlash, *pchDot;

            wcscpy_s(pConv->szDest, MAX_PATH, szPrefix);

            pchSlash = wcsrchr(pConv->szSrc, L'\\');
            if(pchSlash != 0)
                wcscat_s(pConv->szDest, MAX_PATH, pchSlash + 1);
            else
                wcscat_s(pConv->szDest, MAX_PATH, pConv->szSrc);

            pchSlash = wcsrchr(pConv->szDest, '\\');
            pchDot = wcsrchr(pConv->szDest, '.');

            if(pchDot > pchSlash)
                *pchDot = 0;

            wcscat_s(pConv->szDest, MAX_PATH, szSuffix);

            // Write texture
            wprintf( L"writing %s", pConv->szDest);
            fflush(stdout);

            switch( FileType )
            {
            case CODEC_DDS:
                hr = SaveToDDSFile( img, nimg, info,
                                    (dwOptions & (1 << OPT_USE_DX10) ) ? (DDS_FLAGS_FORCE_DX10_EXT|DDS_FLAGS_FORCE_DX10_EXT_MISC2) : DDS_FLAGS_NONE, 
                                    pConv->szDest );
                break;

            case CODEC_TGA:
                hr = SaveToTGAFile( img[0], pConv->szDest );
                break;

            default:
                hr = SaveToWICFile( img, nimg, WIC_FLAGS_ALL_FRAMES, GetWICCodec( static_cast<WICCodecs>(FileType) ), pConv->szDest );
                break;
            }

            if(FAILED(hr))
            {
                wprintf( L" FAILED (%x)\n", hr);
                delete image;
                continue;
            }
            wprintf( L"\n");
        }

        delete image;
    }

    if ( nonpow2warn )
        wprintf( L"\n WARNING: Not all feature levels support non-power-of-2 textures with mipmaps\n" );

    nReturn = 0;

    goto LDone;

LError:
    nReturn = 1;

LDone:

    while(pConversion)
    {
        pConv = pConversion;
        pConversion = pConversion->pNext;
        delete pConv;
    }

    return nReturn;
}
