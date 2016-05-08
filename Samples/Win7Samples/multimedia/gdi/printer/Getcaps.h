
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/******************************************************************************\
*
*                               GETCAPS.H
*
\******************************************************************************/


/******************************************************************************\
*                           SYMBOLIC CONSTANTS
\******************************************************************************/

#define MAX_DEVICE_CAPS     34     // num entries in gaCaps table

#define MAX_TECHNOLOGY_CAPS 7      // num entries in gaTechnologyCaps table
#define MAX_CURVE_CAPS      10     // num entries in gaCurveCaps table
#define MAX_LINE_CAPS       8      // num entries in gaLineCaps table
#define MAX_POLYGON_CAPS    10     // num entries in gaPolygonCaps table
#define MAX_TEXT_CAPS       16     // num entries in gaTextCaps table
#define MAX_RASTER_CAPS     17     // num entries in gaRasterCaps table
#define MAX_CLIP_CAPS       3      // num entries in gaClipCaps table

#define BLANKS              "                 \t" // see gaCaps to get spacing

#define ERR_MOD_NAME       IDS_ERR_GETCAPS



/******************************************************************************\
*                                TYPEDEFS
\******************************************************************************/

typedef struct tagCAPSLOOKUP
{
  int     iValue;
  LPTSTR  szValue;

} CAPSLOOKUP;



/******************************************************************************\
*                            GLOBAL VARIABLES
\******************************************************************************/

HWND ghwndDevCaps;

CAPSLOOKUP gaCaps[] =

    { { DRIVERVERSION,   "DRIVERVERSION:   \t%ld" },
      { TECHNOLOGY,      "TECHNOLOGY:      \t"    },
      { HORZSIZE,        "HORZSIZE:        \t%ld" },
      { VERTSIZE,        "VERTSIZE:        \t%ld" },
      { HORZRES,         "HORZRES:         \t%ld" },
      { VERTRES,         "VERTRES:         \t%ld" },
      { BITSPIXEL,       "BITSPIXEL:       \t%ld" },
      { PLANES,          "PLANES:          \t%ld" },
      { NUMBRUSHES,      "NUMBRUSHES:      \t%ld" },
      { NUMPENS,         "NUMPENS:         \t%ld" },
      { NUMMARKERS,      "NUMMARKERS:      \t%ld" },
      { NUMFONTS,        "NUMFONTS:        \t%ld" },
      { NUMCOLORS,       "NUMCOLORS:       \t%ld" },
      { PDEVICESIZE,     "PDEVICESIZE:     \t%ld" },
      { CURVECAPS,       "CURVECAPS:       \t"    },
      { LINECAPS,        "LINECAPS:        \t"    },
      { POLYGONALCAPS,   "POLYGONALCAPS:   \t"    },
      { TEXTCAPS,        "TEXTCAPS:        \t"    },
      { CLIPCAPS,        "CLIPCAPS:        \t"    },
      { RASTERCAPS,      "RASTERCAPS:      \t"    },
      { ASPECTX,         "ASPECTX:         \t%ld" },
      { ASPECTY,         "ASPECTY:         \t%ld" },
      { ASPECTXY,        "ASPECTXY:        \t%ld" },
      { LOGPIXELSX,      "LOGPIXELSX:      \t%ld" },
      { LOGPIXELSY,      "LOGPIXELSY:      \t%ld" },
      { SIZEPALETTE,     "SIZEPALETTE:     \t%ld" },
      { NUMRESERVED,     "NUMRESERVED:     \t%ld" },
      { COLORRES,        "COLORRES:        \t%ld" },
      { PHYSICALWIDTH,   "PHYSICALWIDTH:   \t%ld" },
      { PHYSICALHEIGHT,  "PHYSICALHEIGHT:  \t%ld" },
      { PHYSICALOFFSETX, "PHYSICALOFFSETX: \t%ld" },
      { PHYSICALOFFSETY, "PHYSICALOFFSETY: \t%ld" },
      { SCALINGFACTORX,  "SCALINGFACTORX:  \t%ld" },
      { SCALINGFACTORY,  "SCALINGFACTORY:  \t%ld" }  };


CAPSLOOKUP gaTechnologyCaps[] =

    { { DT_PLOTTER,      "DT_PLOTTER "      },
      { DT_RASDISPLAY,   "DT_RASDISPLAY "   },
      { DT_RASPRINTER,   "DT_RASPRINTER "   },
      { DT_RASCAMERA,    "DT_RASCAMERA "    },
      { DT_CHARSTREAM,   "DT_CHARSTREAM "   },
      { DT_METAFILE,     "DT_METAFILE "     },
      { DT_DISPFILE,     "DT_DISPFILE "     } };

CAPSLOOKUP gaCurveCaps[] =

    { { CC_NONE,         "CC_NONE "         },
      { CC_CIRCLES,      "CC_CIRCLES "      },
      { CC_PIE,          "CC_PIE "          },
      { CC_CHORD,        "CC_CHORD "        },
      { CC_ELLIPSES,     "CC_ELLIPSES "     },
      { CC_WIDE,         "CC_WIDE "         },
      { CC_STYLED,       "CC_STYLED "       },
      { CC_WIDESTYLED,   "CC_WIDESTYLED "   },
      { CC_INTERIORS,    "CC_INTERIORS "    },
      { CC_ROUNDRECT,    "CC_ROUNDRECT "    } };

CAPSLOOKUP gaLineCaps[] =

    { { LC_NONE,         "LC_NONE "         },
      { LC_POLYLINE,     "LC_POLYLINE "     },
      { LC_MARKER,       "LC_MARKER "       },
      { LC_POLYMARKER,   "LC_POLYMARKER "   },
      { LC_WIDE,         "LC_WIDE "         },
      { LC_STYLED,       "LC_STYLED "       },
      { LC_WIDESTYLED,   "LC_WIDESTYLED "   },
      { LC_INTERIORS,    "LC_INTERIORS "    } };

CAPSLOOKUP gaPolygonCaps[] =

    { { PC_NONE,         "PC_NONE "         },
      { PC_POLYGON,      "PC_POLYGON "      },
      { PC_RECTANGLE,    "PC_RECTANGLE "    },
      { PC_WINDPOLYGON,  "PC_WINDPOLYGON"   },
      { PC_TRAPEZOID,    "PC_TRAPEZOID"     },
      { PC_SCANLINE,     "PC_SCANLINE"      },
      { PC_WIDE,         "PC_WIDE"          },
      { PC_STYLED,       "PC_STYLED"        },
      { PC_WIDESTYLED,   "PC_WIDESTYLED"    },
      { PC_INTERIORS,    "PC_INTERIORS"     } };

CAPSLOOKUP gaTextCaps[] =

    { { TC_OP_CHARACTER, "TC_OP_CHARACTER"  },
      { TC_OP_STROKE,    "TC_OP_STROKE"     },
      { TC_CP_STROKE,    "TC_CP_STROKE"     },
      { TC_CR_90,        "TC_CR_90"         },
      { TC_CR_ANY,       "TC_CR_ANY"        },
      { TC_SF_X_YINDEP,  "TC_SF_X_YINDEP "  },
      { TC_SA_DOUBLE,    "TC_SA_DOUBLE "    },
      { TC_SA_INTEGER,   "TC_SA_INTEGER "   },
      { TC_SA_CONTIN,    "TC_SA_CONTIN "    },
      { TC_EA_DOUBLE,    "TC_EA_DOUBLE "    },
      { TC_IA_ABLE,      "TC_IA_ABLE "      },
      { TC_UA_ABLE,      "TC_UA_ABLE "      },
      { TC_SO_ABLE,      "TC_SO_ABLE "      },
      { TC_RA_ABLE,      "TC_RA_ABLE "      },
      { TC_VA_ABLE,      "TC_VA_ABLE "      },
      { TC_RESERVED,     "TC_RESERVED "     },
      { TC_SCROLLBLT,    "TC_SCROLLBLT "     } };

CAPSLOOKUP gaRasterCaps[] =

    { { 0,               "RC_NONE "         }, // !!! RC_NONE not #def'd
      { RC_BITBLT,       "RC_BITBLT "       },
      { RC_BANDING,      "RC_BANDING "      },
      { RC_SCALING,      "RC_SCALING "      },
      { RC_BITMAP64,     "RC_BITMAP64 "     },
      { RC_GDI20_OUTPUT, "RC_GDI20_OUTPUT " },
      { RC_GDI20_STATE,  "RC_GDI20_STATE "  },
      { RC_SAVEBITMAP,   "RC_SAVEBITMAP "   },
      { RC_DI_BITMAP,    "RC_DI_BITMAP "    },
      { RC_PALETTE,      "RC_PALETTE "      },
      { RC_DIBTODEV,     "RC_DIBTODEV "     },
      { RC_BIGFONT,      "RC_BIGFONT "      },
      { RC_STRETCHBLT,   "RC_STRETCHBLT "   },
      { RC_FLOODFILL,    "RC_FLOODFILL "    },
      { RC_STRETCHDIB,   "RC_STRETCHDIB "   },
      { RC_OP_DX_OUTPUT, "RC_OP_DX_OUTPUT " },
      { RC_DEVBITS,      "RC_DEVBITS "      } };

CAPSLOOKUP gaClipCaps[] =

    { { CP_NONE,         "CP_NONE "         },
      { CP_RECTANGLE,    "CP_RECTANGLE "    },
      { CP_REGION,       "CP_REGION "       } };



/******************************************************************************\
*                          EXTERNAL VARIABLES
\******************************************************************************/

extern HWND ghwndMain;
extern char gszDeviceName [];
extern char gszDriverName [];
extern char gszPort       [];



/******************************************************************************\
*                          FUNCTION PROTOTYPES
\******************************************************************************/

BOOL DisplayDeviceCapsInfo (void);
void TranslateDeviceCaps   (int, int, int);
void ComplexDeviceCapsLine (char *, CAPSLOOKUP *, int, int, int);
