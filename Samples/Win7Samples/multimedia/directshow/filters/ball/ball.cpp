//------------------------------------------------------------------------------
// File: Ball.cpp
//
// Desc: DirectShow sample code.  This sample illustrates a simple source 
//       filter that produces decompressed images showing a ball bouncing 
//       around. Each movement of the ball is done by generating a new image. 
//       We use the CSource and CSourceStream base classes to manage a source 
//       filter - we are a live source and so do not support any seeking.
//
//       The image stream is never-ending, with the ball color dependent on
//       bit depth of the current display device.  32, 24, 16 (555 and 565),
//       and 8 bit palettized types can be supplied.        
//
//       In implementation, the CSource and CSourceStream base classes from 
//       the SDK are used to implement some of the more tedious effort
//       associated with source filters.  In particular, the starting and
//       stopping of worker threads based upon overall activation/stopping 
//       is facilitated.  A worker thread sits in a loop asking for buffers
//       and then calls the PURE virtual FillBuffer method when it has a
//       buffer available to fill. 
//
//       The sample also has a simple quality management implementation in
//       the filter. With the exception of renderers (which normally initiate
//       it), this is controlled through IQualityControl.  In each frame it
//       is called for status.  Due to the straightforward nature of the 
//       filter, spacing of samples sent downward can be controlled so that
//       any CPU used runs flat out.
//
//       Demonstration instructions:
//
//       Start GraphEdit, which is available in the SDK DXUtils folder. Click
//       on the Graph menu and select "Insert Filters." From the dialog box,
//       double click on "DirectShow filters," then "Bouncing ball" and then 
//       dismiss the dialog. Go to the output pin of the filter box and 
//       right click, selecting "Render." A video renderer will be inserted 
//       and connected up (on some displays there may be a color space 
//       convertor put between them to get the pictures into a suitable 
//       format).  Then click "run" on GraphEdit and see the ball bounce 
//       around the window...
//
//       Files:
//
//       ball.cpp         Looks after drawing a moving bouncing ball
//       ball.h           Class definition for the ball drawing object
//       ball.rc          Version and title information resources
//       fball.cpp        The real filter class implementation
//       fball.h          Class definition for the main filter object
//       resource.h       A couple of identifiers for our resources
//
//       Base classes used:
//
//       CSource          Base class for a generic source filter
//       CSourceStream    A base class for a source filters stream
//
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include "ball.h"


//------------------------------------------------------------------------------
// Name: CBall::CBall(()
// Desc: Constructor for the ball class. The default arguments provide a
//       reasonable image and ball size.
//------------------------------------------------------------------------------
CBall::CBall(int iImageWidth, int iImageHeight, int iBallSize) :
    m_iImageWidth(iImageWidth),
    m_iImageHeight(iImageHeight),
    m_iBallSize(iBallSize),
    m_iAvailableWidth(iImageWidth - iBallSize),
    m_iAvailableHeight(iImageHeight - iBallSize),
    m_x(0),
    m_y(0),
    m_xDir(RIGHT),
    m_yDir(UP)
{
    // Check we have some (arbitrary) space to bounce in.
    ASSERT(iImageWidth > 2*iBallSize);
    ASSERT(iImageHeight > 2*iBallSize);

    // Random position for showing off a video mixer
    m_iRandX = rand();
    m_iRandY = rand();

} // (Constructor)


//------------------------------------------------------------------------------
// Name: CBall::PlotBall()
// Desc: Positions the ball on the memory buffer.
//       Assumes the image buffer is arranged as Row 1,Row 2,...,Row n
//       in memory and that the data is contiguous.
//------------------------------------------------------------------------------
void CBall::PlotBall(BYTE pFrame[], BYTE BallPixel[], int iPixelSize)
{
    ASSERT(m_x >= 0);
    ASSERT(m_x <= m_iAvailableWidth);
    ASSERT(m_y >= 0);
    ASSERT(m_y <= m_iAvailableHeight);
    ASSERT(pFrame != NULL);
    ASSERT(BallPixel != NULL);

    // The current byte of interest in the frame
    BYTE *pBack;
    pBack = pFrame;     

    // Plot the ball into the correct location
    BYTE *pBall = pFrame + ( m_y * m_iImageWidth * iPixelSize) + m_x * iPixelSize;

    for(int row = 0; row < m_iBallSize; row++)
    {
        for(int col = 0; col < m_iBallSize; col++)
        {
            // For each byte fill its value from BallPixel[]
            for(int i = 0; i < iPixelSize; i++)
            {  
                if(WithinCircle(col, row))
                {
                    *pBall = BallPixel[i];
                }
                pBall++;
            }
        }
        pBall += m_iAvailableWidth * iPixelSize;
    }

} // PlotBall


//------------------------------------------------------------------------------
// CBall::BallPosition()
// 
// Returns the 1-dimensional position of the ball at time t millisecs
//      (note that millisecs runs out after about a month!)
//------------------------------------------------------------------------------
int CBall::BallPosition(int iPixelTime, // Millisecs per pixel
                        int iLength,    // Distance between the bounce points
                        int time,       // Time in millisecs
                        int iOffset)    // For a bit of randomness
{
    // Calculate the position of an unconstrained ball (no walls)
    // then fold it back and forth to calculate the actual position

    int x = time / iPixelTime;
    x += iOffset;
    x %= 2 * iLength;

    // check it is still in bounds
    if(x > iLength)
    {    
        x = 2*iLength - x;
    }
    return x;

} // BallPosition


//------------------------------------------------------------------------------
// CBall::MoveBall()
//
// Set (m_x, m_y) to the new position of the ball.  move diagonally
// with speed m_v in each of x and y directions.
// Guarantees to keep the ball in valid areas of the frame.
// When it hits an edge the ball bounces in the traditional manner!.
// The boundaries are (0..m_iAvailableWidth, 0..m_iAvailableHeight)
//
//------------------------------------------------------------------------------
void CBall::MoveBall(CRefTime rt)
{
    m_x = BallPosition(10, m_iAvailableWidth, rt.Millisecs(), m_iRandX);
    m_y = BallPosition(10, m_iAvailableHeight, rt.Millisecs(), m_iRandY);

} // MoveBall


//------------------------------------------------------------------------------
// CBall:WithinCircle()
//
// Return TRUE if (x,y) is within a circle radius S/2, center (S/2, S/2)
//      where S is m_iBallSize else return FALSE
//------------------------------------------------------------------------------
inline BOOL CBall::WithinCircle(int x, int y)
{
    unsigned int r = m_iBallSize / 2;

    if((x-r)*(x-r) + (y-r)*(y-r)  < r*r)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

} // WithinCircle


