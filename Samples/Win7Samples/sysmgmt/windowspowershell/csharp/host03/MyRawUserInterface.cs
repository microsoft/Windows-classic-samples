//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
using System;
using System.Collections.Generic;
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Host;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Host
{
    /// <summary>
    /// A sample implementation of the PSHostRawUserInterface for a console
    /// application. Members of this class that map trivially to the .NET console
    /// class are implemented. More complex methods are not implemented and will
    /// throw a NotImplementedException.
    /// </summary>
    class MyRawUserInterface : PSHostRawUserInterface
    {
        /// <summary>
        /// Get and set the background color of text ro be written.
        /// This maps pretty directly onto the corresponding .NET Console
        /// property.
        /// </summary>
        public override ConsoleColor BackgroundColor
        {
            get { return Console.BackgroundColor; }
            set { Console.BackgroundColor = value; }
        }

        /// <summary>
        /// Return the host buffer size adapted from on the .NET Console buffer size
        /// </summary>
        public override Size BufferSize
        {
            get { return new Size(Console.BufferWidth, Console.BufferHeight); }
            set { Console.SetBufferSize(value.Width, value.Height); }
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        public override Coordinates CursorPosition
        {
            get { throw new NotImplementedException("The CursorPosition property is not implemented by MyRawUserInterface."); }
            set { throw new NotImplementedException("The CursorPosition property is not implemented by MyRawUserInterface."); }
        }

        /// <summary>
        /// Return the cursor size taken directly from the .NET Console cursor size.
        /// </summary>
        public override int CursorSize
        {
            get { return Console.CursorSize; }
            set { Console.CursorSize = value; }
        }

        /// <summary>
        /// This functionality is not currently implemented. The call simple returns silently.
        /// </summary>
        public override void FlushInputBuffer()
        {
            ;  //Do nothing...
        }

        /// <summary>
        /// Get and set the foreground color of text ro be written.
        /// This maps pretty directly onto the corresponding .NET Console
        /// property.
        /// </summary>
        public override ConsoleColor ForegroundColor
        {
            get { return Console.ForegroundColor; }
            set { Console.ForegroundColor = value; }
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="rectangle">Unused</param>
        /// <returns>Returns nothing - call fails.</returns>
        public override BufferCell[,] GetBufferContents(Rectangle rectangle)
        {
            throw new NotImplementedException("The GetBufferContents method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Map directly to the corresponding .NET Console property.
        /// </summary>
        public override bool KeyAvailable
        {
            get { return Console.KeyAvailable; }
        }

        /// <summary>
        /// Return the MaxPhysicalWindowSize size adapted from the .NET Console
        /// LargestWindowWidth and LargestWindowHeight.
        /// </summary>
        public override Size MaxPhysicalWindowSize
        {
            get { return new Size(Console.LargestWindowWidth, Console.LargestWindowHeight); }
        }


        /// <summary>
        /// Return the MaxWindowSize size adapted from the .NET Console
        /// LargestWindowWidth and LargestWindowHeight.
        /// </summary>
        public override Size MaxWindowSize
        {
            get { return new Size(Console.LargestWindowWidth, Console.LargestWindowHeight); }
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="options">Unused</param>
        /// <returns>Nothing</returns>
        public override KeyInfo ReadKey(ReadKeyOptions options)
        {
            throw new NotImplementedException("The ReadKey() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="source">Unused</param>
        /// <param name="destination">Unused</param>
        /// <param name="clip">Unused</param>
        /// <param name="fill">Unused</param>
        public override void ScrollBufferContents(Rectangle source, Coordinates destination, Rectangle clip, BufferCell fill)
        {
            throw new NotImplementedException("The ScrollBufferContents() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="origin">Unused</param>
        /// <param name="contents">Unused</param>
        public override void SetBufferContents(Coordinates origin, BufferCell[,] contents)
        {
            throw new NotImplementedException("The SetBufferContents() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        ///  This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="rectangle">Unused</param>
        /// <param name="fill">Unused</param>
        public override void SetBufferContents(Rectangle rectangle, BufferCell fill)
        {
            throw new NotImplementedException("The SetBufferContents() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Return the window position adapted from the Console window position information.
        /// </summary>
        public override Coordinates WindowPosition
        {
            get { return new Coordinates(Console.WindowLeft, Console.WindowTop); }
            set { Console.SetWindowPosition(value.X, value.Y); }
        }

        /// <summary>
        /// Return the window size adapted from the corresponding .NET Console calls.
        /// </summary>
        public override Size WindowSize
        {
            get { return new Size(Console.WindowWidth, Console.WindowHeight); }
            set { Console.SetWindowSize(value.Width, value.Height); }
        }

        /// <summary>
        /// Mapped to the Console.Title property.
        /// </summary>
        public override string WindowTitle
        {
            get { return Console.Title; }
            set { Console.Title = value; }
        }
    }
}

