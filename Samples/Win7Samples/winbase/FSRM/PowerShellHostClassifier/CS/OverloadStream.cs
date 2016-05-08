// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.IO;

namespace Microsoft.Samples.Fsrm.PowerShellHostClassifier
{
    /// <summary>
    /// This class wraps the IStream interface into a Stream interface
    /// This is useful for IFsrmPropertyBag's Istream
    /// </summary>
    public class StreamWrapperForIStream : Stream
    {
        private System.Runtime.InteropServices.ComTypes.IStream m_cachedIstream;
        private long m_position;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="iStream">an Istream to encapsulate</param>
        public StreamWrapperForIStream(IStream istream)
        {
            m_cachedIstream = istream;
        }

        /// <summary>
        /// Deconstructor
        /// </summary>
        ~StreamWrapperForIStream()
        {
        }

        /// <summary>
        /// This stream can always be read so always returning true
        /// </summary>
        public override bool CanRead
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// This stream can always be seeked, so always returning true
        /// </summary>
        public override bool CanSeek
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// This stream can never be written to so always returning false
        /// </summary>
        public override bool CanWrite
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Not Implemented
        /// </summary>
        public override void Flush()
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        /// <summary>
        /// Get the length associated with the stream
        /// </summary>
        public override long Length
        {
            get
            {
                long length = 0;

                // not locking the m_LengthCached because there is nothing
                // wrong with 2 variables getting here simultaneosly
                // Note setting long in 64bit is atomic so this lock isn't needed
                lock (this)
                {
                    System.Runtime.InteropServices.ComTypes.STATSTG stat;

                    m_cachedIstream.Stat(out stat, 0);
                    length = stat.cbSize;
                }
                
                return length;
            }
        }

        /// <summary>
        /// Get the current position in the Stream
        /// Set the current position in the stream
        /// </summary>
        public override long Position
        {
            get
            {
                // this is getting locked even though this code will always run on 64bit
                // on 64bit updates/reads from longs are atomic therefore this lock is uncessary
                lock (this)
                {
                    return m_position;
                }
            }
            set
            {
                //operating outside lock due to seek being locked
                long newPosition = Seek(value, SeekOrigin.Begin);
                if (newPosition != value)
                {
                    throw new Exception("Seek failed");
                }
            }
        }

        /// <summary>
        /// Read from the current position in the stream, and advance the current position the number of bytes read
        /// </summary>
        /// <param name="buffer">The buffer to read the stream into</param>
        /// <param name="offset">The offset of buffer to start reading into, For performance try and always use 0</param>
        /// <param name="count">The number of bytes to read from the stream</param>
        /// <returns>The number of bytes read into buffer/the number of bytes the position in the stream is advanced</returns>
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (offset < 0 || offset + count > buffer.Length)
            {
                throw new AccessViolationException("Tried to read into an invalid index");
            }

            // ensure that length gets accessed outside the lock
            long length = this.Length;
            int internal_offset = offset;
            int totalBytesRead = 0;


            lock (this)
            {
                // ensure to follow Read's behavior of always filling the buffer if we are not reading till the end of the file

                int MaxBytesToRead = count;

                if ((length - m_position) < (long)MaxBytesToRead)
                {
                    MaxBytesToRead = (int)(length - m_position);
                }

                if (MaxBytesToRead == 0)
                    return 0;

                // if reading from offset 0 we can directly read into the buffer
                if (internal_offset == 0)
                {
                    
                    unsafe
                    {
                        // This memory will be fixed due to it being declared in unsafe and allow taking the address
                        UInt32 numBytesRead = 0;
                        
                        m_cachedIstream.Read(buffer, count, new IntPtr(&numBytesRead));
                        
                        //Note that numBytesRead is a UInt32
                        //But since I read only values of size int, I can can safely assume this value
                        //to be less than MaxInt
                        internal_offset += (Int32)numBytesRead;
                        totalBytesRead += (Int32)numBytesRead;
                    }
                }

                // we might need to read more data, and can't read directly into the buffer
                if (totalBytesRead < MaxBytesToRead)
                {
                    byte[] byteBuffer = new byte[MaxBytesToRead - totalBytesRead];

                    while (totalBytesRead < MaxBytesToRead)
                    {
                        unsafe
                        {
                            // This memory will be fixed due to it being declared in unsafe and allow taking the address
                            UInt32 numBytesRead = 0;

                            m_cachedIstream.Read(byteBuffer, MaxBytesToRead - totalBytesRead, new IntPtr(&numBytesRead));

                            //Note that numBytesRead is a UInt32
                            //But since I read only values of size int, I can can safely assume this value
                            //to be less than MaxInt
                            Array.Copy(byteBuffer, 0, buffer, internal_offset, (Int32)numBytesRead);
                            internal_offset += (Int32)numBytesRead;
                            totalBytesRead += (Int32)numBytesRead;
                        }
                    }
                }
                m_position += totalBytesRead;
            }
            return totalBytesRead;
        }

        /// <summary>
        /// Move the current pointer in the strea relative to the seek origin
        /// </summary>
        /// <param name="offset">The number of bytes to move from origin</param>
        /// <param name="origin">Move from begging, current cursor, or end of file</param>
        /// <returns></returns>
        public override long Seek(long offset, SeekOrigin origin)
        {
            int dwOrigin = 0;

            switch (origin)
            {
                case SeekOrigin.Begin:
                    dwOrigin = 0;
                    break;

                case SeekOrigin.Current:
                    dwOrigin = 1;
                    break;

                case SeekOrigin.End:
                    dwOrigin = 2;
                    break;

                default:
                    throw new ArgumentException( "Invalid seek origin" );
            }

            //ensure not seeking while reading
            lock (this)
            {
                unsafe
                {
                    // getting casting errors is unlikely as that would be millions of terrabytes for a single file
                    // This memory will be fixed due to it being declared in unsafe and allow taking the address
                    ulong ulongPosition = (ulong)m_position;

                    m_cachedIstream.Seek(offset, dwOrigin, new IntPtr(&ulongPosition));
                    
                    m_position = (long)ulongPosition;
                }
            }
            return m_position;
        }

        /// <summary>
        /// Not Implemented
        /// </summary>
        /// <param name="value"></param>
        public override void SetLength(long value)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        /// <summary>
        /// NotImplemented
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="offset"></param>
        /// <param name="count"></param>
        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }
    }

}
