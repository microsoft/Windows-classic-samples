// <copyright file="MyRawUserInterface.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Host
{
    using System;
    using System.Management.Automation.Host;

    /// <summary>
    /// A sample implementation of the PSHostRawUserInterface for a console
    /// application. Members of this class that map trivially to the .NET console
    /// class are implemented. More complex methods are not implemented and will
    /// throw a NotImplementedException.
    /// </summary>
    internal class MyRawUserInterface : PSHostRawUserInterface
    {
        /// <summary>
        /// Gets or sets the background color of text to be written.
        /// This maps pretty directly onto the corresponding .NET Console
        /// property.
        /// </summary>
        public override ConsoleColor BackgroundColor
        {
            get { return Console.BackgroundColor; }
            set { Console.BackgroundColor = value; }
        }

        /// <summary>
        /// Gets or sets the host buffer size adapted from on the .NET Console buffer size
        /// </summary>
        public override Size BufferSize
        {
            get { return new Size(Console.BufferWidth, Console.BufferHeight); }
            set { Console.SetBufferSize(value.Width, value.Height); }
        }

        /// <summary>
        /// Gets or sets the cursor position. This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        public override Coordinates CursorPosition
        {
            get { throw new NotImplementedException("The CursorPosition property is not implemented by MyRawUserInterface."); }
            set { throw new NotImplementedException("The CursorPosition property is not implemented by MyRawUserInterface."); }
        }

        /// <summary>
        /// Gets or sets the cursor size taken directly from the .NET Console cursor size.
        /// </summary>
        public override int CursorSize
        {
            get { return Console.CursorSize; }
            set { Console.CursorSize = value; }
        }

        /// <summary>
        /// Gets or sets the foreground color of the text to be written.
        /// This maps pretty directly onto the corresponding .NET Console
        /// property.
        /// </summary>
        public override ConsoleColor ForegroundColor
        {
            get { return Console.ForegroundColor; }
            set { Console.ForegroundColor = value; }
        }

        /// <summary>
        /// Gets a value indicating whether a key is available. This implementation 
        /// maps directly to the corresponding .NET Console property.
        /// </summary>
        public override bool KeyAvailable
        {
            get { return Console.KeyAvailable; }
        }

        /// <summary>
        /// Gets the maximum physical size of the window adapted from the  
        /// .NET Console LargestWindowWidth and LargestWindowHeight properties.
        /// </summary>
        public override Size MaxPhysicalWindowSize
        {
            get { return new Size(Console.LargestWindowWidth, Console.LargestWindowHeight); }
        }

        /// <summary>
        /// Gets the maximum window size adapted from the .NET Console
        /// LargestWindowWidth and LargestWindowHeight properties.
        /// </summary>
        public override Size MaxWindowSize
        {
            get { return new Size(Console.LargestWindowWidth, Console.LargestWindowHeight); }
        }

        /// <summary>
        /// Gets or sets the window position adapted from the Console window position 
        /// information.
        /// </summary>
        public override Coordinates WindowPosition
        {
            get { return new Coordinates(Console.WindowLeft, Console.WindowTop); }
            set { Console.SetWindowPosition(value.X, value.Y); }
        }

        /// <summary>
        /// Gets or sets the window size adapted from the corresponding .NET Console calls.
        /// </summary>
        public override Size WindowSize
        {
            get { return new Size(Console.WindowWidth, Console.WindowHeight); }
            set { Console.SetWindowSize(value.Width, value.Height); }
        }

        /// <summary>
        /// Gets or sets the title of the window mapped to the Console.Title property.
        /// </summary>
        public override string WindowTitle
        {
            get { return Console.Title; }
            set { Console.Title = value; }
        }
        
        /// <summary>
        /// Resets the input buffer. This functionality is not implemented. 
        /// The call simple returns silently.
        /// </summary>
        public override void FlushInputBuffer()
        {
          // Do nothing.
        }

        /// <summary>
        /// This functionality is not currently implemented. The call fails with an exception.
        /// </summary>
        /// <param name="rectangle">This parameter is not used.</param>
        /// <returns>Throws a NotImplementedException exception.</returns>
        public override BufferCell[,] GetBufferContents(Rectangle rectangle)
        {
            throw new NotImplementedException("The GetBufferContents method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Reads a pressed, released, or pressed and released keystroke 
        /// from the keyboard device, blocking processing until a keystroke 
        /// is typed that matches the specified keystroke options. This 
        /// functionality is not implemented. The call fails with an 
        /// exception.
        /// </summary>
        /// <param name="options">The parameter is not used.</param>
        /// <returns>Throws a NotImplementedException exception.</returns>
        public override KeyInfo ReadKey(ReadKeyOptions options)
        {
            throw new NotImplementedException("The ReadKey() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Crops a region of the screen buffer. This functionality is not 
        /// implemented. The call fails with an exception.
        /// </summary>
        /// <param name="source">The parameter is not used.</param>
        /// <param name="destination">The parameter is not used.</param>
        /// <param name="clip">The parameter is not used.</param>
        /// <param name="fill">The parameter is not used.</param>
        public override void ScrollBufferContents(Rectangle source, Coordinates destination, Rectangle clip, BufferCell fill)
        {
            throw new NotImplementedException("The ScrollBufferContents() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Copies a given character, foreground color, and background color to a 
        /// region of the screen buffer based on the coordinates of the region. This 
        /// functionality is not implemented. The call fails with an exception.
        /// </summary>
        /// <param name="origin">The parameter is not used.</param>
        /// <param name="contents">The parameter is not used.</param>
        public override void SetBufferContents(Coordinates origin, BufferCell[,] contents)
        {
            throw new NotImplementedException("The SetBufferContents() method is not implemented by MyRawUserInterface.");
        }

        /// <summary>
        /// Copies a given character, foreground color, and background color to a 
        /// rectangle of the screen buffer. This functionality is not implemented. 
        /// The call fails with an exception.
        /// </summary>
        /// <param name="origin">The parameter is not used.</param>
        /// <param name="contents">The parameter is not used.</param>
        public override void SetBufferContents(Rectangle rectangle, BufferCell fill)
        {
            throw new NotImplementedException("The SetBufferContents() method is not implemented by MyRawUserInterface.");
        }
    }
}

