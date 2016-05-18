// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// DrawingObject.cs
//
// This class handles manipulation of the drawing object (rectangle) as a 
// response to multi-touch gestures
//
// Initially, we define rectangle to be in the middle of the client area. 
// Whenever user resizes the window the rectangle is placed in the middle of the
// client area with the width set to a half of the width of the client area and 
// height set to a half of the height of the client area. The rectangle is placed
// in the center of the client area. 
//
// Through the gestures user can zoom in, zoom out, move or rotate the rectangle. 
// By invoking two finger tap the user can add/remove diagonals from the drawing.
//
// DrawingObject class holds information about the rectangle. Instead of storing
// two oposite corner points we do calculate them dynamically by using the other 
// information that we store in this class about the rectangle. This way we do 
// not get deformation of the rectangle due to precission issues.

using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace Microsoft.Samples.Touch.MTGestures
{
    public class DrawingObject
    {
        // This is an array of colors that we will shift through when enduser
        // generates press and tap gesture
        static private Color[] _arrColors = new Color[] { 
            Color.Black, 
            Color.Yellow, 
            Color.Red, 
            Color.Green, 
            Color.Blue 
        };

        // We do retain client area rectangle
        private Rectangle _rcClient;

        // We do retain center point of the rectangle (diagonals intersection)
        private Point _ptCenter;

        // Then we keep information about size of the rectangle
        private Size _szRectangle;

        // Zooming in/out will scale the width and the height by some factor
        private double _dScalingFactor;

        // Here we store total rotation angle of the rectangle (from x-axis)
        private double _dRotationAngle;

        // This variable triggers drawing of diagonals if set to true
        bool _bDrawDiagonals;

        // This variable defines the color of the rectangle (index)
        int _iColorIndex;

        // Constructor
        public DrawingObject()
        {
            // The main window form is responsible to invoke ResetObject 
            // method to initialize variables. This is done in 
            // OnSizeChanged method of MTGesturesForm class
        }

        // This method shifts color index by one
        public void ShiftColor()
        {
            _iColorIndex++;
            _iColorIndex %= _arrColors.Length;
        }

        // This method resets the rectangle object information and it is called
        // by the main app whenever the user resizes the client area
        // in:
        //      rcClient - client area rectangle
        public void ResetObject(Rectangle rcClient)
        {
            // Save client area rectangle
            _rcClient = rcClient;

            // Initial positon of the rectangle is the center of the client area
            _ptCenter.X = _rcClient.Width / 2;
            _ptCenter.Y = _rcClient.Height / 2;

            // Initial width and height are half of the size of the client area       
            _szRectangle.Width = _rcClient.Width / 2;
            _szRectangle.Height = _rcClient.Height / 2;

            // Initial scaling factor is 1.0 (no scaling)
            _dScalingFactor = 1.0;

            // Initial rotation angle is 0.0 (no rotation)
            _dRotationAngle = 0.0; // no rotation intially

            _bDrawDiagonals = false; // no drawing of the diagonals

            _iColorIndex = 0; // set initial color to black
        }

        // This mehod will be called by the MTGesturesForm whenever OnPaint event
        // handler is invoked. It is responsible to redraw the rectangle. Here we
        // calculate the positon of the rectangle corners.
        // in:
        //      graphics - Graphics object 
        public void Paint(Graphics graphics)
        {
            if (null == graphics)
            {
                // this code should never be executed
                throw new Exception("Invalid graphic object!");
            }

            // create new pen 
            Pen p = new Pen(_arrColors[_iColorIndex]);

            // first create a polyline that describes the rectangle scaled by the 
            // scaling factor
            Point[] arrPts = new Point[5];

            // upper left cofner
            arrPts[0].X = -(int)(_dScalingFactor * _szRectangle.Width / 2);
            arrPts[0].Y = -(int)(_dScalingFactor * _szRectangle.Height / 2);

            // upper right corner
            arrPts[1].X = (int)(_dScalingFactor * _szRectangle.Width / 2);
            arrPts[1].Y = arrPts[0].Y;

            // lower right corner
            arrPts[2].X = arrPts[1].X;
            arrPts[2].Y = (int)(_dScalingFactor * _szRectangle.Height / 2);

            // lower left corner
            arrPts[3].X = arrPts[0].X;
            arrPts[3].Y = arrPts[2].Y;

            // upper left corner, we are closing the rectangle
            arrPts[4] = arrPts[0];

            // now we should rotate and translate the rectangle
            double dCos = Math.Cos(_dRotationAngle);
            double dSin = Math.Sin(_dRotationAngle);

            for (int j = 0; j < 5; j++)
            {
                int idx = arrPts[j].X;
                int idy = arrPts[j].Y;

                // rotation
                arrPts[j].X = (int)(idx * dCos + idy * dSin);
                arrPts[j].Y = (int)(idy * dCos - idx * dSin);

                // translation
                arrPts[j].X += _ptCenter.X;
                arrPts[j].Y += _ptCenter.Y;
            }

            graphics.DrawLines(p, arrPts); 

            if(_bDrawDiagonals)
            {
                // draw diagonals
                graphics.DrawLine(p, arrPts[0], arrPts[2]);
                graphics.DrawLine(p, arrPts[1], arrPts[3]);
            }
        }

        // This method is responsible for translating the center of the rectangle.
        // in:
        //      deltaX - shift of the x-coordinate
        //      deltaY - shift of the y-coordinate        
        public void Move(int deltaX,int deltaY)
        {
            _ptCenter.X += deltaX;
            _ptCenter.Y += deltaY;
        }

        // This method toggles the drawing of the diagonals
        public void ToggleDrawDiagonals()
        {
            _bDrawDiagonals = !_bDrawDiagonals;
        }

        // This method zooms the rectangle in/out
        // in:
        //      zoomFactor   - scaling factor of the zoom
        //      coefficientX - x coordinate of the zoom center
        //      coefficientY - y coordinate of the zoom center
        public void Zoom(double zoomFactor,int coefficientX,int coefficientY)
        {
            _ptCenter.X = RoundAndCastValue(coefficientX*(1.0-zoomFactor)+_ptCenter.X*zoomFactor);
            _ptCenter.Y = RoundAndCastValue(coefficientY*(1.0-zoomFactor)+_ptCenter.Y*zoomFactor);

            _dScalingFactor *= zoomFactor;
        }

        // This method rotates the rectangle
        // in:
        //      angle   - angle of rotation in radians
        //      centerX - x-coordinate of the rotation center
        //      centerY - y-coordinate of the rotation center
        //
        // Note that during the rotation gesture the user defines center of rotation. 
        // This is going to move the center of the rectangle too. That is why we have to 
        // recalculate new center of the rectangle.
        public void Rotate(double angle,int centerX,int centerY)
        {
            double dCos = Math.Cos(angle);
            double dSin = Math.Sin(angle);

            int idx = _ptCenter.X - centerX;
            int idy = _ptCenter.Y - centerY;

            _ptCenter.X = centerX + RoundAndCastValue(idx * dCos + idy * dSin);
            _ptCenter.Y = centerY + RoundAndCastValue(idy * dCos - idx * dSin);

            _dRotationAngle += angle;
        }

        // This macro is used to round double and cast it to int       
        static protected int RoundAndCastValue(double number)
        {
            return (int)(Math.Floor(0.5 + number));
        }
    }
}
