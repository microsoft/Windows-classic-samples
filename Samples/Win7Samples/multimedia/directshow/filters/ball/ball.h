//------------------------------------------------------------------------------
// File: Ball.h
//
// Desc: DirectShow sample code - header file for the bouncing ball
//       source filter.  For more information, refer to Ball.cpp.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Define GUIDS used in this sample
//------------------------------------------------------------------------------
// { fd501041-8ebe-11ce-8183-00aa00577da1 }
DEFINE_GUID(CLSID_BouncingBall,
0xfd501041, 0x8ebe, 0x11ce, 0x81, 0x83, 0x00, 0xaa, 0x00, 0x57, 0x7d, 0xa1);


//------------------------------------------------------------------------------
// Class CBall
//
// This class encapsulates the behavior of the bounching ball over time
//------------------------------------------------------------------------------
class CBall
{
public:

    CBall(int iImageWidth = 320, int iImageHeight = 240, int iBallSize = 10);

    // Plots the square ball in the image buffer, at the current location.
    // Use BallPixel[] as pixel value for the ball.
    // Plots zero in all 'background' image locations.
    // iPixelSize - the number of bytes in a pixel (size of BallPixel[])
    void PlotBall(BYTE pFrame[], BYTE BallPixel[], int iPixelSize);

    // Moves the ball 1 pixel in each of the x and y directions
    void MoveBall(CRefTime rt);

    int GetImageWidth() { return m_iImageWidth ;}
    int GetImageHeight() { return m_iImageHeight ;}

private:

    enum xdir { LEFT = -1, RIGHT = 1 };
    enum ydir { UP = 1, DOWN = -1 };

    // The dimensions we can plot in, allowing for the width of the ball
    int m_iAvailableHeight, m_iAvailableWidth;

    int m_iImageHeight;     // The image height
    int m_iImageWidth;      // The image width
    int m_iBallSize;        // The diameter of the ball
    int m_iRandX, m_iRandY; // For a bit of randomness
    xdir m_xDir;            // Direction the ball
    ydir m_yDir;            // Likewise vertically

    // The X position, in pixels, of the ball in the frame
    // (0 < x < m_iAvailableWidth)
    int m_x;

    // The Y position, in pixels, of the ball in the frame
    // (0 < y < m_iAvailableHeight)
    int m_y;    

    // Return the one-dimensional position of the ball at time T milliseconds
    int BallPosition(int iPixelTime, int iLength, int time, int iOffset);

    /// Tests a given pixel to see if it should be plotted
    BOOL WithinCircle(int x, int y);

}; // CBall
