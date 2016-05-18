/******************************************************************************\
*
*  MODULE:      PAINT.C
*
*  PURPOSE:     This is the module responsible for painting the SPINCUBE
*               custom control. When Paint() is called we retrieve a
*               pointer to a SPINCUBEINFO structure, and then use it's
*               current rotation & translation values to transform the
*               polyhedron described by gNormalizedVertices & gaiFacets.
*               Once we've transformed the vertices, we draw the
*               background, which consists of a grey rectangle and a few
*               black lines (a crass attempt to render a perspective
*               view into a "room"), on the offscreen bitmap associated
*               with the control (i.e. pSCI->hbmCompat). Then we walk the
*               facet list of the transformed polyhedron (gXformedVertices
*               & gaiFacets), drawing only those facets whose outward
*               normal faces us (again, drawing on pSCI->hbmCompat).
*               Finally, we BitBlt the appropriate rectangle from our
*               offscreen bitmap to the screen itself.
*
*               Drawing to the offscreen bitmap has two advantages over
*               drawing straight to the screen:
*
*                 1. The actual drawing the user sees consists of only
*                    a single BitBlt. Otherwise, the user would see us
*                    both erase the polyhedron in it's old position and
*                    draw it in it's new position (alot of flashing- not
*                    very smooth animation).
*
*                 2. When a spincube control with the SS_ERASE style
*                    is brought to the foreground, all it's contents
*                    i.e. the cube trails) are saved & can be re-Blted
*                    to the screen. Otherwise, all this info would be
*                    lost & there'd be a big blank spot in the middle
*                    of the control!
*
*               Interested persons should consult a text on 3 dimensional
*               graphics for more information (i.e. "Computer Graphics:
*               Principles and Practice", by Foley & van Dam).
*
*               Notes:
*
*               - A 3x2 tranformation matrix  is used instead of a  3x3
*                 matrix, since the transformed z-values aren't needed.
*                 (Normally these would be required for use in depth
*                 sorting  [for hidden surface removal], but  since we
*                 draw  only  a single convex polyhedron this  is not
*                 necessary.)
*
*               - A simplified perspective viewing transformation
*                 (which also  precludes the need for the transformed z
*                 coordinates). In a nutshell, the perspective  scale
*                 is as follows:
*
*                                    p' = S    x  p
*                                          per
*
*                 where:
*                        S    = WindowDepth /
*                         per      (WindowDepth + fCurrentZTranslation)
*
*                 (WindowDepth is  the greater of the  control's window
*                 height or window width.)
*
*
*  FUNCTIONS:   Paint()                         - the paint routine
*               TransformVertices()             - transforms vertices
*               ComputeRotationTransformation() - computes xformation
*                                                 based on current x, y
*                                                 and z rotation angles
*
*
*                           Microsoft Developer Support
*                  Copyright 1992 - 2000 Microsoft Corporation
*
\******************************************************************************/

#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include "spincube.h"
#include "paint.h"



/******************************************************************************\
*
*  FUNCTION:     Paint
*
*  INPUTS:       hwnd - Handle of the window to draw into.
*
*  COMMENTS:     Draws window background & a polyhedron in the window.
*
\******************************************************************************/

void Paint (HWND hwnd)
{
  PSPINCUBEINFO  pSCI;
  RECT           rect;
  int            i;
  LONG           lScaleFactor;
  PAINTSTRUCT    ps;
  HRGN           hrgnClip;
  HBRUSH         hBrush, hBrushSave;
  int            iX, iY, iCX, iCY;
  int            facetIndex, numPoints;
  POINT          polygn[MAX_POINTS_PER_FACET];
  POINT          vector1, vector2;
  COLORREF       acrColor[6] = { 0x0000ff, 0x00ff00, 0xff0000,
                                 0x00ffff, 0xff00ff, 0xffff00 };

  pSCI = (PSPINCUBEINFO) GetWindowLongPtr (hwnd, GWL_SPINCUBEDATA);

  BeginPaint (hwnd, &ps);

  if (memcmp((void *)&ps.rcPaint, (void *)&pSCI->rcCubeBoundary, sizeof(RECT))
      & !REPAINT_BKGND(pSCI))

  {
    //
    // We're not here because it's time to animate (i.e. this paint isn't
    //   the result of a WM_TIMER), so just do the Blt & blow out of here...
    //

    BitBlt (ps.hdc,
            ps.rcPaint.left,
            ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            pSCI->hdcCompat, ps.rcPaint.left,
            ps.rcPaint.top, SRCCOPY);

    EndPaint (hwnd, &ps);
    return;
  }


  //
  // The rectangle we get back is in Desktop coordinates, so we need to
  //   modify it to reflect coordinates relative to this window.
  //

  GetWindowRect (hwnd, &rect);

  rect.right  -= rect.left;
  rect.bottom -= rect.top;
  rect.left = rect.top = 0;


  //
  // Determine a "best fit" scale factor for our polyhedron
  //

  if (!(lScaleFactor = rect.right > rect.bottom ?
                       rect.bottom/12 : rect.right/12))

    lScaleFactor = 1;


  TransformVertices (hwnd, &rect, pSCI, lScaleFactor);


  //
  // Draw the window frame & background
  //
  // Note: The chances are that we are coming through here because we
  //   got a WM_TIMER message & it's time to redraw the cube to simulate
  //   animation. In that case all we want to erase/redraw is that small
  //   rectangle which bounded the polyhedron the last time. The less
  //   drawing that actually gets done the better, since we wnat to
  //   minimize the flicker on the screen. __BeginPaint__ is perfect for
  //   this because it causes all drawing outside of the invalid region
  //   to be "clipped" (no drawing is performed outside of the invalid
  //   region), and it also validates the invalid region.
  //

  if (DO_ERASE(hwnd) || REPAINT_BKGND(pSCI))
  {
    hrgnClip = CreateRectRgnIndirect (&ps.rcPaint);
    SelectClipRgn (pSCI->hdcCompat, hrgnClip);
    DeleteObject (hrgnClip);

    SelectObject (pSCI->hdcCompat, GetStockObject (GRAY_BRUSH));
    Rectangle    (pSCI->hdcCompat, (int)rect.left, (int)rect.top,
                  (int)rect.right, (int)rect.bottom);

    iX = (rect.right  - rect.left) / 4;
    iY = (rect.bottom - rect.top ) / 4;

    MoveToEx (pSCI->hdcCompat, (int)rect.left, (int)rect.top, NULL);
    LineTo   (pSCI->hdcCompat, (int)rect.left + iX, (int)rect.top + iY);
    LineTo   (pSCI->hdcCompat, (int)rect.left + iX, (int)rect.bottom - iY);
    LineTo   (pSCI->hdcCompat, (int)rect.left,      (int)rect.bottom);

    MoveToEx (pSCI->hdcCompat, (int)rect.right, (int)rect.top, NULL);
    LineTo   (pSCI->hdcCompat, (int)rect.right - iX, (int)rect.top + iY);
    LineTo   (pSCI->hdcCompat, (int)rect.right - iX, (int)rect.bottom- iY);
    LineTo   (pSCI->hdcCompat, (int)rect.right,      (int)rect.bottom);

    MoveToEx (pSCI->hdcCompat, (int)rect.left + iX,  (int)rect.top + iY, NULL);
    LineTo   (pSCI->hdcCompat, (int)rect.right - iX, (int)rect.top + iY);

    MoveToEx (pSCI->hdcCompat, (int)rect.left + iX,  (int)rect.bottom - iY, NULL);
    LineTo   (pSCI->hdcCompat, (int)rect.right - iX, (int)rect.bottom - iY);

    SelectClipRgn (pSCI->hdcCompat, NULL);

    pSCI->iOptions &= ~SPINCUBE_REPAINT_BKGND;
  }


  //
  // Draw the polyhedron. We'll walk through the facets list and compute
  //   the normal for each facet- if the normal has z > 0, then the facet
  //   faces us and we'll draw it. Note that this algorithim is ONLY valid
  //   for scenes with a single, convex polyhedron.
  //
  // Note: Use GetDC here because the above call to BeginPaint will
  //   probably not give us a DC with access to as much real estate as
  //   we'd like (we wouldn't be able to draw outside of the invalid
  //   region). We can party on the entire control window with the DC
  //   returned by GetDC.
  //

  for (i = 0, facetIndex = 0; i < NUMFACETS; i++)
  {
    vector1.x = gXformedVertices[gaiFacets[facetIndex + 1]].x -
                gXformedVertices[gaiFacets[facetIndex]].x;
    vector1.y = gXformedVertices[gaiFacets[facetIndex + 1]].y -
                gXformedVertices[gaiFacets[facetIndex]].y;
    vector2.x = gXformedVertices[gaiFacets[facetIndex + 2]].x -
                gXformedVertices[gaiFacets[facetIndex + 1]].x;
    vector2.y = gXformedVertices[gaiFacets[facetIndex + 2]].y -
                gXformedVertices[gaiFacets[facetIndex + 1]].y;

    for (numPoints = 0; gaiFacets[facetIndex] != -1; numPoints++, facetIndex++)
    {
      polygn[numPoints].x = gXformedVertices[gaiFacets[facetIndex]].x;
      polygn[numPoints].y = gXformedVertices[gaiFacets[facetIndex]].y;
    }

    facetIndex++; /* skip over the -1's in the facets list */
    if ((vector1.x*vector2.y - vector1.y*vector2.x) > 0)
    {
      hBrush     = CreateSolidBrush (acrColor[i]);
      hBrushSave = (HBRUSH) SelectObject (pSCI->hdcCompat, hBrush);

      Polygon (pSCI->hdcCompat, &polygn[0], numPoints);

      SelectObject (pSCI->hdcCompat, hBrushSave);
      DeleteObject (hBrush);
    }
  }

  iX  = pSCI->rcCubeBoundary.left < ps.rcPaint.left ?
        pSCI->rcCubeBoundary.left : ps.rcPaint.left;
  iY  = pSCI->rcCubeBoundary.top  < ps.rcPaint.top  ?
        pSCI->rcCubeBoundary.top  : ps.rcPaint.top;

  iCX = (pSCI->rcCubeBoundary.right > ps.rcPaint.right ?
         pSCI->rcCubeBoundary.right : ps.rcPaint.right) - iX;

  iCY = (pSCI->rcCubeBoundary.bottom > ps.rcPaint.bottom ?
         pSCI->rcCubeBoundary.bottom : ps.rcPaint.bottom) - iY;

  EndPaint (hwnd, &ps);

  ps.hdc = GetDC (hwnd);

  BitBlt (ps.hdc, iX, iY, iCX, iCY, pSCI->hdcCompat, iX, iY, SRCCOPY);

  ReleaseDC (hwnd, ps.hdc);

}



/******************************************************************************\
*
*  FUNCTION:     TransformVertices
*
*  INPUTS:       hwnd         - control window handle
*                pWindowRect  - pointer to RECT describing control's dimensions
*                pSCI         - pointer to control's SPINCUBEINFO structure
*                fScaleFactor - scale factor for use in this window
*
\******************************************************************************/

void TransformVertices (HWND hwnd,          RECT  *pWindowRect,
                        PSPINCUBEINFO pSCI, LONG  lScaleFactor)
{
  int    i;
  int    iWindowDepth = pWindowRect->right > pWindowRect->bottom ?
                        pWindowRect->right : pWindowRect->bottom;
  RECT   WindowRect;
  float  fDepthScale;
  int    iNewTranslationInc = (rand() % 10) + 2;
  float  fNewRotationInc    = (float) 0.01 * ((rand() % 30) + 2);

  WindowRect.left = - (WindowRect.right  = pWindowRect->right  >> 1);
  WindowRect.top  = - (WindowRect.bottom = pWindowRect->bottom >> 1);

  //
  // Initiailize the bounding rectangle with max/min vals
  //

  pSCI->rcCubeBoundary.left   =
  pSCI->rcCubeBoundary.top    = 100000; // big positive value
  pSCI->rcCubeBoundary.right  =
  pSCI->rcCubeBoundary.bottom = -100000; // small negative value


  //
  // First scale, then rotate, then translate each vertex.
  //   Keep track of the maximum & minimum values bounding the
  //   vertices in the x,y plane for use later in bounds checking.
  //
  // Note: we don't bother computing z values after the scale,
  //   as they are only really necessary for the rotation. If we
  //   were doing real bounds checking we'd need it, but this code
  //   simply uses the pSCI->iCurrentZTranslation to determine
  //   the z-boundaries.
  //

  for (i = 0; i < NUMVERTICES; i++)
  {
    LONG tempX;

    //
    // Copy the static vertices into a temp array
    //

    gXformedVertices[i] = gNormalizedVertices[i];

    //
    // The scale...
    //

    gXformedVertices[i].x *= lScaleFactor;
    gXformedVertices[i].y *= lScaleFactor;
    gXformedVertices[i].z *= lScaleFactor;

    //
    // The rotation...
    //

    ComputeRotationTransformation (pSCI->fCurrentXRotation,
                                   pSCI->fCurrentYRotation,
                                   pSCI->fCurrentZRotation);

    tempX   =               (LONG) (gM[0][0] * gXformedVertices[i].x +
                                    gM[0][1] * gXformedVertices[i].y +
                                    gM[0][2] * gXformedVertices[i].z);

    gXformedVertices[i].y = (LONG) (gM[1][0] * gXformedVertices[i].x +
                                    gM[1][1] * gXformedVertices[i].y +
                                    gM[1][2] * gXformedVertices[i].z);
    gXformedVertices[i].x = tempX;

    //
    // The translation...
    //

    gXformedVertices[i].x += pSCI->iCurrentXTranslation;
    gXformedVertices[i].y += pSCI->iCurrentYTranslation;

    //
    // Check if we have new max or min vals
    //

    if (pSCI->rcCubeBoundary.left > gXformedVertices[i].x)

      pSCI->rcCubeBoundary.left = gXformedVertices[i].x;

    if (pSCI->rcCubeBoundary.right < gXformedVertices[i].x)

      pSCI->rcCubeBoundary.right = gXformedVertices[i].x;

    if (pSCI->rcCubeBoundary.top > gXformedVertices[i].y)

      pSCI->rcCubeBoundary.top = gXformedVertices[i].y;

    if (pSCI->rcCubeBoundary.bottom < gXformedVertices[i].y)

      pSCI->rcCubeBoundary.bottom = gXformedVertices[i].y;
  }


  //
  // Now for some bounds checking, change translation & rotation increments
  //   if we hit a "wall". Also so the gbHitBoundary flag so we remember
  //   to flash the cube when we draw it.
  //

  if (pSCI->rcCubeBoundary.left < WindowRect.left)
  {
    pSCI->iCurrentXTranslationInc = iNewTranslationInc;
    pSCI->fCurrentZRotationInc    = fNewRotationInc;
  }

  else if (pSCI->rcCubeBoundary.right > WindowRect.right)
  {
    pSCI->iCurrentXTranslationInc = -iNewTranslationInc;
    pSCI->fCurrentZRotationInc    = -fNewRotationInc;
  }

  if (pSCI->rcCubeBoundary.top < WindowRect.top)
   {
    pSCI->iCurrentYTranslationInc = iNewTranslationInc;
    pSCI->fCurrentXRotationInc    = fNewRotationInc;
  }

  else if (pSCI->rcCubeBoundary.bottom > WindowRect.bottom)
  {
    pSCI->iCurrentYTranslationInc = -iNewTranslationInc;
    pSCI->fCurrentXRotationInc    = -fNewRotationInc;
  }

  if (pSCI->iCurrentZTranslation < (int) lScaleFactor<<1)
  {
    pSCI->iCurrentZTranslationInc = iNewTranslationInc;
    pSCI->fCurrentYRotationInc    = fNewRotationInc;
  }

  else if (pSCI->iCurrentZTranslation > (iWindowDepth - (int) lScaleFactor))
  {
    pSCI->iCurrentZTranslationInc = -iNewTranslationInc;
    pSCI->fCurrentYRotationInc    = -fNewRotationInc;
  }


  //
  // Now a kludgy scale based on depth (iCurrentZTranslation) of the center
  //   point of the polyhedron
  //

  fDepthScale =  ((float) iWindowDepth) /
                 ((float) (iWindowDepth + pSCI->iCurrentZTranslation));

  pSCI->rcCubeBoundary.left  = (LONG)(fDepthScale* pSCI->rcCubeBoundary.left  );
  pSCI->rcCubeBoundary.right = (LONG)(fDepthScale* pSCI->rcCubeBoundary.right );
  pSCI->rcCubeBoundary.top   = (LONG)(fDepthScale* pSCI->rcCubeBoundary.top   );
  pSCI->rcCubeBoundary.bottom= (LONG)(fDepthScale* pSCI->rcCubeBoundary.bottom);

  for (i = 0; i < NUMVERTICES; i++)
  {
    gXformedVertices[i].x = (LONG) (fDepthScale * gXformedVertices[i].x);
    gXformedVertices[i].y = (LONG) (fDepthScale * gXformedVertices[i].y);
  }


  //
  // If currently in motion then increment the current rotation & tranlation
  //

  if (IN_MOTION(hwnd))
  {
    pSCI->fCurrentXRotation += pSCI->fCurrentXRotationInc;
    pSCI->fCurrentYRotation += pSCI->fCurrentYRotationInc;
    pSCI->fCurrentZRotation += pSCI->fCurrentZRotationInc;

    pSCI->iCurrentXTranslation += pSCI->iCurrentXTranslationInc;
    pSCI->iCurrentYTranslation += pSCI->iCurrentYTranslationInc;
    pSCI->iCurrentZTranslation += pSCI->iCurrentZTranslationInc;
  }


  //
  // Up to this point all coordinates are relative to a window whose
  //   center is at (0,0). Now we'll translate appropriately...
  //

  pSCI->rcCubeBoundary.left   += pWindowRect->right  >> 1;
  pSCI->rcCubeBoundary.right  += pWindowRect->right  >> 1;
  pSCI->rcCubeBoundary.top    += pWindowRect->bottom >> 1;
  pSCI->rcCubeBoundary.bottom += pWindowRect->bottom >> 1;

  for (i = 0; i < NUMVERTICES; i++)
  {
    gXformedVertices[i].x += pWindowRect->right  >> 1;
    gXformedVertices[i].y += pWindowRect->bottom >> 1;
  }


  //
  // Since FillRect's are inclusive-exclusive (there'll be leftovers
  //   from the last cube we drew otherwise)...
  //

  pSCI->rcCubeBoundary.right++;
  pSCI->rcCubeBoundary.bottom++;


  //
  // Finally, adjust the rcCubeBoundary such that it fits entirely within
  //   the acutal control window. The reason for this is that when calling
  //   InvalidateRect from SpincubeWndProc\case_WM_TIMER we may get
  //   a different PAINSTRUCT.rcPaint (since InvalidateRect clips the passed
  //   in rect to the window bounds) and our abouve test to memcmp() will
  //   fail.
  //

  if (pSCI->rcCubeBoundary.left < 0)

    pSCI->rcCubeBoundary.left = 0;

  if (pSCI->rcCubeBoundary.top < 0)

    pSCI->rcCubeBoundary.top = 0;

  if (pSCI->rcCubeBoundary.right > pWindowRect->right)

    pSCI->rcCubeBoundary.right = pWindowRect->right;

  if (pSCI->rcCubeBoundary.bottom > pWindowRect->bottom)

    pSCI->rcCubeBoundary.bottom = pWindowRect->bottom;
}


/******************************************************************************\
*
*  FUNCTION:    ComputeRotationTransformation
*
*  INPUTS:      fRotationX - Angle to rotate about X axis.
*               fRotationY - Angle to rotate about Y axis.
*               fRotationZ - Angle to rotate about Z axis.
*
*  COMMENTS:    Computes a 3x2 tranformation matrix which rotates about
*               the Z axis, the Y axis, and the X axis, respectively.
*
\******************************************************************************/

void ComputeRotationTransformation (float fRotationX,
                                    float fRotationY,
                                    float fRotationZ)
{
  float sinX, cosX, sinY, cosY, sinZ, cosZ;

  sinX = (float) sin ((double) fRotationX);
  cosX = (float) cos ((double) fRotationX);
  sinY = (float) sin ((double) fRotationY);
  cosY = (float) cos ((double) fRotationY);
  sinZ = (float) sin ((double) fRotationZ);
  cosZ = (float) cos ((double) fRotationZ);

  gM[0][0] =  cosY*cosZ;
  gM[0][1] = -cosY*sinZ;
  gM[0][2] =  sinY;
  gM[1][0] =  sinX*sinY*cosZ + cosX*sinZ;
  gM[1][1] = -sinX*sinY*sinZ + cosX*cosZ;
  gM[1][2] = -sinX*cosY;
}
