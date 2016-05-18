
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/* These defines determine the meaning of the fFlags variable.  The low byte
 * is used for the various types of "boxes" to draw.  The high byte is
 * available for special commands.
 */

#define SL_BOX    1             /* Draw a solid border around the rectangle  */
#define SL_BLOCK  2             /* Draw a solid rectangle                    */

#define SL_EXTEND 256           /* Extend the current pattern                */

#define SL_TYPE    0x00FF       /* Mask out everything but the type flags    */
#define SL_SPECIAL 0xFF00       /* Mask out everything but the special flags */

// Temporary porting macros

#define INT2DWORD               DWORD
typedef POINTS MPOINT;
#define MAKEMPOINT(l)           (*((MPOINT *)&(l)))
#define MPOINT2POINT(mpt, pt)   ((pt).x = (mpt).x, (pt).y = (mpt).y)
#define POINT2MPOINT(pt, mpt)   ((mpt).x = (SHORT)(pt).x, (mpt).y = (SHORT)(pt).y)


int APIENTRY StartSelection(HWND, MPOINT, LPRECT, int);
int APIENTRY UpdateSelection(HWND, MPOINT, LPRECT, int);
int APIENTRY EndSelection(MPOINT, LPRECT);
int APIENTRY ClearSelection(HWND, LPRECT, int);
