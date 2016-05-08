// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Diagnostics;

namespace Microsoft.Samples.Touch.MTScratchpadWMTouch
{
    // Stroke object represents a single stroke, trajectory of the finger from
    // touch-down to touch-up. Object has two properties: color of the stroke,
    // and ID used to distinguish strokes coming from different fingers.
    public class Stroke
    {
        // Stroke constructor.
        public Stroke()
        {
            points = new ArrayList();
        }

        // Draws the complete stroke.
        // in:
        //      graphics        the drawing surface
        public void Draw(Graphics graphics)
        {
            if ((points.Count < 2) || (graphics == null))
            {
                return;
            }

            Pen pen = new Pen(color, penWidth);
            graphics.DrawLines(pen, (Point[]) points.ToArray(typeof(Point)));
        }

        // Draws only last segment of the stroke.
        // in:
        //      graphics        the drawing surface
        public void DrawLast(Graphics graphics)
        {
            if ((points.Count < 2) || (graphics == null))
            {
                return;
            }

            Pen pen = new Pen(color, penWidth);
            pen.EndCap = System.Drawing.Drawing2D.LineCap.Round;
            graphics.DrawLine(pen, (Point)points[points.Count - 2], (Point)points[points.Count - 1]);
        }

        // Access to the property stroke color
        public Color Color
        {
            get { return color; }
            set { color = value; }
        }

        // Access to the property stroke ID
        public int Id
        {
            get { return id; }
            set { id = value; }
        }

        // Adds a point to the stroke.
        // in:
        //      pt          point to be added to the stroke
        public void Add(Point pt)
        {
            points.Add(pt);
        }

        // Attributes
        private ArrayList points;               // the array of stroke points
        private Color color;                    // the color of the stroke
        private int id;                         // stroke ID

        private const float penWidth = 3.0f;    // pen width for drawing the stroke
    }

    // CollectionOfStrokes object represents a collection of the strokes.
    // It supports add/remove stroke operations and finding a stroke by ID.
    public class CollectionOfStrokes
    {
        // CollectionOfStrokes constructor.
        public CollectionOfStrokes()
        {
            strokes = new ArrayList();
        }

        // Draw the collection of the strokes.
        // in:
        //      graphics        the drawing surface
        public void Draw(Graphics graphics)
        {
            foreach (Stroke stroke in strokes)
            {
                stroke.Draw(graphics);
            }
        }

        // Adds the stroke to the collection.
        // in:
        //      stroke          stroke to be added
        public void Add(Stroke stroke)
        {
            strokes.Add(stroke);
        }

        // Returns the stroke with the given ID.
        // in:
        //      id              stroke ID
        // returns:
        //      stroke, if found, or null
        public Stroke Get(int id)
        {
            int i = _IndexFromId(id);
            if (i == -1)
            {
                return null;
            }
            return (Stroke)strokes[i];
        }

        // Remove the stroke from the collection.
        // in:
        //      id              stroke ID
        // returns:
        //      stroke, if found, or null
        public Stroke Remove(int id)
        {
            int i = _IndexFromId(id);
            if (i == -1)
            {
                return null;
            }
            Stroke s = (Stroke)strokes[i];
            strokes.RemoveAt(i);
            return s;
        }

        public void Clear()
        {
            // Removes all the strokes from the collection.
            strokes.Clear();
        }

        // Search the collection for given ID.
        // in:
        //      id      stroke ID
        // returns:
        //      stroke index in the array
        private int _IndexFromId(int id)
        {
            for (int i = 0; i < strokes.Count; ++i)
            {
                Stroke stroke = (Stroke)strokes[i];
                if (id == stroke.Id)
                {
                    return i;
                }
            }
            return -1;
        }

        // Attributes
        private ArrayList strokes;          // the array of strokes
    }
}
