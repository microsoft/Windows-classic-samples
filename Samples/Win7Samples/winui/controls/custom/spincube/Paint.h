/******************************************************************************\
*
*                               MODULE: PAINT.H
*
\******************************************************************************/



/******************************************************************************\
*                             SYMBOLIC CONSTANTS
\******************************************************************************/

#define  NUMVERTICES  8 /* number of polyhedron vertices */
#define  NUMFACETS    6 /* number of polyhedron facets   */

#define  MAX_POINTS_PER_FACET 4



/******************************************************************************\
*                                  TYPEDEFs
\******************************************************************************/

typedef struct
{
  LONG x, y, z;

} POINT3D;



/******************************************************************************\
*                          GLOBAL VARIABLES
\******************************************************************************/

//
//  This particular set of vertices "gNormalizedVertices" and corresponding
//    facets "gaiFacets" describe a normalized cube centered about the
//    origin ([0,0,0] in 3-space). The gaiFacet array is made up of a series
//    of indices into the array of vertices, each describing an individual
//    facet (eg. a polygon), and are separated by -1. Note that the facets
//    are decribed in COUNTERCLOCKWISE (relative to the viewer) order so we
//    can consistently find the normal to any given facet. (The normal
//    is used to determine facet visibilty.)
//

static POINT3D gNormalizedVertices[NUMVERTICES] = {{ 1, 1, 1}, { 1,-1, 1},
                                                   {-1,-1, 1}, {-1, 1, 1},
                                                   { 1, 1,-1}, { 1,-1,-1},
                                                   {-1,-1,-1}, {-1, 1,-1}  };


static int gaiFacets[30] = { 3, 2, 1, 0, -1,
                             4, 5, 6, 7, -1,
                             0, 1, 5, 4, -1,
                             6, 2, 3, 7, -1,
                             7, 3, 0, 4, -1,
                             5, 1, 2, 6, -1  };

POINT3D gXformedVertices[NUMVERTICES];

float   gM[2][3]; /* the transformation matrix */



/******************************************************************************\
*                         FUNCTION PROTOTYPES
\******************************************************************************/

void TransformVertices (HWND, RECT *, PSPINCUBEINFO, LONG);
void ComputeRotationTransformation (float, float, float);
