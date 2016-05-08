/******************************************************************************\
*
*                                  SPINCUBE.H
*
\******************************************************************************/

#include <custcntl.h>



/******************************************************************************\
*                               SYMBOLIC CONSTANTS
\******************************************************************************/

#define SPINCUBECLASS           TEXT("Spincube")
#define SPINCUBEDESCRIPTION     TEXT("An animated control")
#define SPINCUBEDEFAULTTEXT     TEXT(":-)")

#define CCHSTYLE                20  // size of style string, i.e. "SS_ERASE"

#define NUM_SPINCUBE_STYLES     2

#define SS_ERASE                0x0001  // spincube window styles
#define SS_INMOTION             0x0002


#define SPINCUBE_EXTRA          4   // number of extra bytes for spincube class

#define SPIN_EVENT              1   // timer event id to repaint control
#ifdef _ALPHA_
#define SPIN_INTERVAL           0   // milliseconds between repaints. what
                                    //   would be really cool is a way to
                                    //   dynamically adjust the interval-
                                    //   on real fast machines we might
                                    //   almost be able to get decent-looking
                                    //   animation! :)

#else
#define SPIN_INTERVAL           75  // milliseconds between repaints. what
                                    //   would be really cool is a way to
                                    //   dynamically adjust the interval-
                                    //   on real fast machines we might
                                    //   almost be able to get decent-looking
                                    //   animation! :)

#endif
#define DID_ERASE               101 // dialog control id's
#define DID_INMOTION            102
#define DID_OK                  103

#define GWL_SPINCUBEDATA        0   // offset of control's instance data

#define SPINCUBE_REPAINT_BKGND  0x00000001

#define DO_ERASE(hwnd)          GetWindowLongPtr(hwnd,GWL_STYLE) & SS_ERASE \
                                           ? TRUE : FALSE

#define IN_MOTION(hwnd)         GetWindowLongPtr(hwnd,GWL_STYLE) & SS_INMOTION \
                                           ? TRUE : FALSE

#define REPAINT_BKGND(pSCI)     pSCI->iOptions&SPINCUBE_REPAINT_BKGND \
                                           ? TRUE : FALSE

#define IDS_REGCLASSFAIL      16
#define IDS_UNREGFAIL         17
#define IDS_DLGBOXFAIL        18
#define IDS_ALLOCFAIL         19
#define IDS_CREATEDCFAIL      20
#define IDS_CREATEBITMAPFAIL  21


/******************************************************************************\
*                                    TYPEDEFs
\******************************************************************************/

typedef struct
{
  HDC      hdcCompat;               // the DC that will contain our off-screen
                                    //   image
  HBITMAP  hbmSave;                 // Save previous selected bitmap
  HBITMAP  hbmCompat;               // The bitmap that will contain the actual
                                    //   image, i.e. we will always do our
                                    //   drawing on this bmp & then blt the
                                    //   result to the screen.

  float    fCurrentXRotation;       // Angle (in radians) to rotate cube about
  float    fCurrentYRotation;       //   x, y, z axis
  float    fCurrentZRotation;

  float    fCurrentXRotationInc;    // Amount to inc rotation angle each
  float    fCurrentYRotationInc;    //   time we repaint (and are in motion)
  float    fCurrentZRotationInc;

  int      iCurrentXTranslation;    // Distance (in pels) to translate cube
  int      iCurrentYTranslation;
  int      iCurrentZTranslation;

  int      iCurrentXTranslationInc; // Amount to inc translation distance each
  int      iCurrentYTranslationInc; //   time we repaint (and are in motion)
  int      iCurrentZTranslationInc;

  RECT     rcCubeBoundary;          // Bounding rectangle (in 2D) of the last
                                    //   cube drawn.  We invalidate only this
                                    //   region when we're doing animation
                                    //   and get the WM_TIMER- it's alot more
                                    //   efficient that invalidating the whole
                                    //   control (there's less screen flashing.

  int      iOptions;                // Contains the current options for this
                                    //   ctrl, i.e. erase background.

} SPINCUBEINFO, *PSPINCUBEINFO;



/******************************************************************************\
*                                FUNCTION PROTOTYPES
\******************************************************************************/

INT     CALLBACK SpincubeSizeToText (DWORD, DWORD, HFONT,  LPTSTR);
BOOL    CALLBACK SpincubeStyle      (HWND,  LPCCSTYLE);
LRESULT CALLBACK SpincubeWndProc    (HWND,  UINT,  WPARAM, LPARAM);
LRESULT CALLBACK SpincubeDlgProc    (HWND,  UINT,  WPARAM, LPARAM);
