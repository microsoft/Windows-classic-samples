using System;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace Storage.Interop
{
/*
    public enum StorageBusType 
    {  
        BusTypeUnknown = 0x00,  
        BusTypeScsi,  
        BusTypeAtapi,  
        BusTypeAta,  
        BusType1394,  
        BusTypeSsa,  
        BusTypeFibre,  
        BusTypeUsb,  
        BusTypeRAID,  
        BusTypeiScsi,  
        BusTypeSas,  
        BusTypeSata,  
        BusTypeSd,  
        BusTypeMmc,  
        BusTypeMax,  
        BusTypeMaxReserved = 0x7F
    };

    public struct StorageAdapterDescriptor 
    {  
        uint Version;
        uint Size;
        uint MaximumTransferLength;
        uint MaximumPhysicalPages;
        uint AlignmentMask;  
        [MarshalAs(UnmanagedType.U1)]
        bool AdapterUsesPio;  
        [MarshalAs(UnmanagedType.U1)]
        bool AdapterScansDown;  
        [MarshalAs(UnmanagedType.U1)]
        bool CommandQueueing;  
        [MarshalAs(UnmanagedType.U1)]
        bool AcceleratedTransfer;
        StorageBusType BusType;  
        ushort  BusMajorVersion;  
        ushort  BusMinorVersion;
    };

    public struct StorageDeviceDescriptor 
    {  
        uint Version;  
        uint Size;  
        byte DeviceType;  
        byte DeviceTypeModifier;  
        [MarshalAs(UnmanagedType.U1)]
        bool RemovableMedia;  
        [MarshalAs(UnmanagedType.U1)]
        bool CommandQueueing;  
        String VendorId;  
        String ProductId;  
        String ProductRevision;  
        String SerialNumber;  
        StorageBusType BusType;  
        byte[] RawDeviceProperties;
    }

    /// <summary>
    /// The State of Last Session specifies the recorded state of the last session, 
    /// regardless of the number of sessions on the disc.
    /// </summary>
    public enum StateLastSession:byte
    {
        EmptySession,
        /// <summary>
        /// When a disc is DVD-RW in restricted overwrite mode and the last session is 
        /// in the Intermediate State, IncompleteSession is returned.
        /// </summary>
        IncompleteSession,
        /// <summary>
        /// Valid only for DVD-R/-RW media
        /// </summary>
        Reserved_DamagedSession,
        /// <summary>
        /// Valid only for Disc Status
        /// </summary>
        CompleteSession
    }

    /// <summary>
    /// The Disc Status indicates the recorded status of the disc. 
    /// A Drive that does not have the ability to write the inserted medium shall 
    /// return only FinalizedDisc status.
    /// </summary>
    public enum DiscStatus
    {
        /// <summary>
        /// A recordable disc is present and is either logically or physically blank.
        /// </summary>
        EmptyDisc,
        /// <summary>
        /// The currently mounted disc is recorded/recordable serially in sessions. 
        /// The last session is either blank or partially recorded.
        /// </summary>
        IncompleteDisc,
        /// <summary>
        /// The currently mounted disc is recorded/recordable serially in sessions. 
        /// The last session is closed and there is no possibility of appending a new session.
        /// </summary>
        FinalizedDisc,
        /// <summary>
        /// The currently mounted disc supports only random access writing and is not recordable 
        /// serially in multiple sessions.
        /// </summary>
        Others
    }

    public enum BackgroundFormatStatus
    {
        /// <summary>
        /// At least one of the following is true:
        /// 1. The disc is neither CD-RW nor DVD+RW.
        /// 2. The disc is CD-RW, it is not formatted as CD-MRW.
        /// 3. If the disc is DVD+RW, it is blank.
        /// </summary>
        OneOfMany = 0x00,
        /// <summary>
        /// A background format was started but is not currently running and is not complete.
        /// </summary>
        StartedNotRunningNotComplete = 0x01,
        /// <summary>
        /// A background format is in progress. A format has been started or restarted and is not yet completed.
        /// </summary>
        InProgress = 0x02,
        /// <summary>
        /// Background formatting has completed.
        /// </summary>
        Completed = 0x03
    }

    public struct DiscInformation
    {
        /// <summary>
        /// The Erasable bit, when set to one, indicates that CD-RW, DVD-RAM, DVD-RW, 
        /// DVD+RW, HD DVD-RAM, or BD-RE media is present and the Drive is capable of 
        /// writing the media. If the Erasable bit is set to zero, then either the medium 
        /// is not erasable or the Drive is unable to write the media.
        /// </summary>
        bool IsErasable;

        /// <summary>
        /// The State of Last Session specifies the recorded state of the last session, 
        /// regardless of the number of sessions on the disc.
        /// </summary>
        StateLastSession LastSessionState;

        /// <summary>
        /// The Disc Status indicates the recorded status of the disc. 
        /// A Drive that does not have the ability to write the inserted medium shall 
        /// return only FinalizedDisc status.
        /// </summary>
        DiscStatus Status;

        /// <summary>
        /// The Number of First Track on Disc is the track number of the Logical Track that contains LBA 0. 
        /// The value reported in based upon media type and recorded status:
        /// a) For CD-ROM the value is the smallest track number recorded in the first TOC on the disc.
        /// b) For CD-R/RW recorded as ROM (i.e., the PMA is blank, but the first TOC is written), the value is the
        ///    smallest track number recorded in the first TOC on the disc.
        /// c) For CD-R/RW where the PMA is not blank, the value is the smallest track number recorded in the PMA.
        /// d) For CD-R/RW where the PMA is blank and the first TOC on the disc is also blank, the value is one (1).
        /// e) For all other media regardless of recording status, the value is one (1).
        /// </summary>
        byte NumberOfFirstTrackOnDisc;

        /// <summary>
        /// </summary>
        ushort NumberOfSessions;

        /// <summary>
        /// First Track Number in Last Session is the track number of the first Logical Track in the last session. This includes the incomplete track.
        /// </summary>
        ushort FirstTrackNumberInLastSession;
        
        /// <summary>
        /// 
        /// </summary>
        ushort LastTrackNumberInLastSession;

        /// <summary>
        /// Indicates that the Disc Identification field is valid.
        /// </summary>
        bool IsDiscIDValid;

        /// <summary>
        /// Indicates that the Disc Bar Code field is valid.
        /// </summary>
        bool IsDiscBarCodeValid;

        /// <summary>
        /// The URU (Unrestricted Use Disc) bit may be false for special use CD-R, CD-RW, or DVD-R, medium. 
        /// For all other media types, URU shall be set to true. When URU is false, the mounted disc is 
        /// defined for restricted use. Recording to a restricted use disc, required the appropriate Host 
        /// Application code set in the Write Parameters Page. When URU is set to true, the mounted medium 
        /// has unrestricted write use.
        /// </summary>
        bool UnrestrictedUseDisc;

        /// <summary>
        /// Indicates the validity of the Disc Application Code. If set to false, then the Disc Application 
        /// Code is not valid. If set to true, the Disc Application Code is valid.
        /// </summary>
        bool IsDiscApplicationCodeValid;

        /// <summary>
        /// If the disc is MRW formatted or MRW formatting, then this is a copy of the “dirty bit” from the 
        /// defect table. If this is false, then the MRW structures are current. If this is true, then the MRW 
        /// structures may not be current. When BG format status = 00b, this shall be set to false.
        /// </summary>
        bool AreMRWStructuresCurrent;
    }
*/

    /// <summary>
    /// This is a representation of an IO.Stream and IStream object. 
    /// </summary>
    public class AStream : Stream, IStream, IDisposable
    {
        /// <summary>
        /// Gets a value indicating whether the current stream supports reading.
        /// </summary>
        public override bool CanRead
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanRead;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the current stream supports seeking.
        /// </summary>
        public override bool CanSeek
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanSeek;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the current stream supports writing.
        /// </summary>
        public override bool CanWrite
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanWrite;
                }
            }
        }

        public override bool CanTimeout
        {
            get
            {
                if (TheIStream != null)
                {
                    return false;
                }
                else
                {
                    return TheStream.CanTimeout;
                }
            }
        }

        /// <summary>
        /// Gets the length in bytes of the stream.
        /// </summary>
        public override long Length
        {
            get
            {
                if (TheIStream != null)
                {
                    // Call IStream.Stat to retrieve info about the stream,
                    // which includes the length. STATFLAG_NONAME means that we don't
                    // care about the name (STATSTG.pwcsName), so there is no need for
                    // the method to allocate memory for the string.
                    System.Runtime.InteropServices.ComTypes.STATSTG statstg;
                    TheIStream.Stat(out statstg, 1);
                    return statstg.cbSize;
                }
                else
                {
                    return TheStream.Length;
                }
            }
        }

        /// <summary>
        /// Gets or sets the position within the current stream.
        /// </summary>
        public override long Position
        {
            get
            {
                if (TheIStream != null)
                {
                    return Seek(0, SeekOrigin.Current);
                }
                else
                {
                    return TheStream.Position;
                }
            }
            set
            {
                if (TheIStream != null)
                {
                    Seek(value, SeekOrigin.Begin);
                }
                else
                {
                    TheStream.Position = value;
                }
            }
        }

        /// <summary>
        /// Clears all buffers for this stream and causes any buffered data to be written 
        /// to the underlying device.
        /// </summary>
        public override void Flush()
        {
            if (TheIStream != null)
            {
                TheIStream.Commit(STGC_DEFAULT);
            }
            else
            {
                TheStream.Flush();
            }
        }

        /// <summary>
        /// Reads a sequence of bytes from the current stream and advances the position 
        /// within the stream by the number of bytes read.
        /// </summary>
        /// <param name="buffer">An array of bytes. When this method returns, the buffer contains the specified byte array with the values between offset and (offset + count - 1) replaced by the bytes read from the current source.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin storing the data read from the current stream.</param>
        /// <param name="count">The maximum number of bytes to be read from the current stream.</param>
        /// <returns>The total number of bytes read into the buffer. This can be less than the number of bytes requested if that many bytes are not currently available, or zero (0) if the end of the stream has been reached.</returns>
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (TheIStream != null)
            {
                if (offset != 0) throw new NotSupportedException("Only a zero offset is supported.");

                int bytesRead = 0;
                IntPtr br = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(int)));
                Marshal.WriteInt32(br, 0);

                // Include try catch for c++ eh exceptions. are they the same as comexceptions?
                TheIStream.Read(buffer, count, br);
                bytesRead = Marshal.ReadInt32(br);

                Marshal.FreeHGlobal(br);

                return bytesRead;
            }
            else
            {
                return TheStream.Read(buffer, offset, count);
            }
        }

        /// <summary>
        /// Sets the position within the current stream.
        /// </summary>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type SeekOrigin indicating the reference point used to obtain the new position.</param>
        /// <returns>The new position within the current stream.</returns>
        public override long Seek(long offset, System.IO.SeekOrigin origin)
        {
            if (TheIStream != null)
            {
                long position = 0;
                IntPtr pos = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(long)));
                Marshal.WriteInt64(pos, 0);

                // The enum values of SeekOrigin match the enum values of
                // STREAM_SEEK, so we can just cast the origin to an integer.
                TheIStream.Seek(offset, (int)origin, pos);
                position = Marshal.ReadInt64(pos);

                Marshal.FreeHGlobal(pos);

                return position;
            }
            else
            {
                return TheStream.Seek(offset, origin);
            }
        }

        /// <summary>
        /// Sets the length of the current stream.
        /// </summary>
        /// <param name="value">The desired length of the current stream in bytes.</param>
        public override void SetLength(long value)
        {
            if (TheIStream != null)
            {
                TheIStream.SetSize(value);
            }
            else
            {
                TheStream.SetLength(value);
            }
        }

        // Writes a sequence of bytes to the current stream and advances the
        // current position within this stream by the number of bytes written
        /// <summary>
        /// Writes a sequence of bytes to the current stream and advances the current position 
        /// within this stream by the number of bytes written.
        /// </summary>
        /// <param name="buffer">An array of bytes. This method copies count bytes from buffer to the current stream.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin copying bytes to the current stream.</param>
        /// <param name="count">The number of bytes to be written to the current stream.</param>
        public override void Write(byte[] buffer, int offset, int count)
        {
            if (TheIStream != null)
            {
                if (offset != 0) throw new NotSupportedException("Only a zero offset is supported.");

                // Pass "null" for the last parameter since we don't use the value
                TheIStream.Write(buffer, count, IntPtr.Zero);
            }
            else
            {
                TheStream.Write(buffer, offset, count);
            }
        }

        /// <summary>
        /// Creates a new stream object with its own seek pointer that references 
        /// the same bytes as the original stream.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="ppstm">When successful, pointer to the location of an IStream pointer to the new stream object.</param>
        ///<exception cref="NotSupportedException">The IO.Streamtream cannot be cloned.</exception>
        public void Clone(out IStream ppstm)
        {
            if (TheStream != null) throw new NotSupportedException("The Stream cannot be cloned.");

            TheIStream.Clone(out ppstm);
        }

        /// <summary>
        /// Ensures that any changes made to an stream object that is open in transacted 
        /// mode are reflected in the parent storage.
        /// </summary>
        /// <remarks>
        /// The <paramref name="grfCommitFlags"/> parameter is not used and this method only does Stream.Flush()
        /// </remarks>
        /// <param name="grfCommitFlags">Controls how the changes for the stream object are committed. 
        /// See the STGC enumeration for a definition of these values.</param>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        public void Commit(int grfCommitFlags)
        {
            // Clears all buffers for this stream and causes any buffered data to be written 
            // to the underlying device.
            if (TheStream != null)
            {
                TheStream.Flush();
            }
            else
            {
                TheIStream.Commit(grfCommitFlags);
            }
        }

        /// <summary>
        /// Copies a specified number of bytes from the current seek pointer in the stream 
        /// to the current seek pointer in another stream.
        /// </summary>
        /// <param name="pstm">
        /// The destination stream. The pstm stream  can be a new stream or a clone of the source stream.
        /// </param>
        /// <param name="cb">
        /// The number of bytes to copy from the source stream.
        /// </param>
        /// <param name="pcbRead">
        /// The actual number of bytes read from the source. 
        /// It can be set to IntPtr.Zero. 
        /// In this case, this method does not provide the actual number of bytes read.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt64</typeparam>
        /// <param name="pcbWritten">
        /// The actual number of bytes written to the destination. 
        /// It can be set this to IntPtr.Zero. 
        /// In this case, this method does not provide the actual number of bytes written.
        /// </param>
        /// <typeparam name="pcbWritten">Native UInt64</typeparam>
        /// <returns>
        /// The actual number of bytes read (<paramref name="pcbRead"/>) and written (<paramref name="pcbWritten"/>) from the source.
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void CopyTo(IStream pstm, long cb, IntPtr pcbRead, IntPtr pcbWritten)
        {
            if (TheStream != null)
            {
                byte[] sourceBytes = new byte[cb];
                int currentBytesRead = 0;
                long totalBytesRead = 0;
                int currentBytesWritten = 0;
                long totalBytesWritten = 0;

                IntPtr bw = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(int)));
                Marshal.WriteInt32(bw, 0);

                while (totalBytesWritten < cb)
                {
                    currentBytesRead = TheStream.Read(sourceBytes, 0, (int)(cb - totalBytesWritten));

                    // Has the end of the stream been reached?
                    if (currentBytesRead == 0) break;

                    totalBytesRead += currentBytesRead;

                    pstm.Write(sourceBytes, currentBytesRead, bw);
                    currentBytesWritten = Marshal.ReadInt32(bw);
                    if (currentBytesWritten != currentBytesRead)
                    {
                        Debug.WriteLine("ERROR!: The IStream Write is not writing all the bytes needed!");
                    }
                    totalBytesWritten += currentBytesWritten;
                }

                Marshal.FreeHGlobal(bw);

                if (pcbRead != IntPtr.Zero) Marshal.WriteInt64(pcbRead, totalBytesRead);
                if (pcbWritten != IntPtr.Zero) Marshal.WriteInt64(pcbWritten, totalBytesWritten);
            }
            else
            {
                TheIStream.CopyTo(pstm, cb, pcbRead, pcbWritten);
            }
        }

        /// <summary>
        /// Restricts access to a specified range of bytes in the stream.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="libOffset">Integer that specifies the byte offset for the beginning of the range.</param>
        /// <param name="cb">Integer that specifies the length of the range, in bytes, to be restricted.</param>
        /// <param name="dwLockType">Specifies the restrictions being requested on accessing the range.</param>
        ///<exception cref="NotSupportedException">The IO.Stream does not support locking.</exception>
        public void LockRegion(long libOffset, long cb, int dwLockType)
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support locking.");

            TheIStream.LockRegion(libOffset, cb, dwLockType);
        }

        /// <summary>
        /// Reads a specified number of bytes from the stream object 
        /// into memory starting at the current seek pointer.
        /// </summary>
        /// <param name="pv">The buffer which the stream data is read into.</param>
        /// <param name="cb">The number of bytes of data to read from the stream object.</param>
        /// <param name="pcbRead">
        /// A pointer to a ULONG variable that receives the actual number of bytes read from the stream object.
        /// It can be set to IntPtr.Zero. 
        /// In this case, this method does not return the number of bytes read.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt32</typeparam>
        /// <returns>
        /// The actual number of bytes read (<paramref name="pcbRead"/>) from the source.
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            if (TheStream != null)
            {
                if (pcbRead == IntPtr.Zero)
                {
                    // User isn't interested in how many bytes were read
                    TheStream.Read(pv, 0, cb);
                }
                else
                {
                    Marshal.WriteInt32(pcbRead, TheStream.Read(pv, 0, cb));
                }
            }
            else
            {
                TheIStream.Read(pv, cb, pcbRead);
            }
        }

        /// <summary>
        /// Discards all changes that have been made to a transacted 
        /// stream since the last stream.Commit call
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        ///<exception cref="NotSupportedException">The IO.Stream does not support reverting.</exception>
        public void Revert()
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support reverting.");

            TheIStream.Revert();
        }

        /// <summary>
        /// Changes the seek pointer to a new location relative to the beginning
        ///of the stream, the end of the stream, or the current seek pointer
        /// </summary>
        /// <param name="dlibMove">
        /// The displacement to be added to the location indicated by the dwOrigin parameter. 
        /// If dwOrigin is STREAM_SEEK_SET, this is interpreted as an unsigned value rather than a signed value.
        /// </param>
        /// <param name="dwOrigin">
        /// The origin for the displacement specified in dlibMove. 
        /// The origin can be the beginning of the file (STREAM_SEEK_SET), the current seek pointer (STREAM_SEEK_CUR), or the end of the file (STREAM_SEEK_END).
        /// </param>
        /// <param name="plibNewPosition">
        /// The location where this method writes the value of the new seek pointer from the beginning of the stream.
        /// It can be set to IntPtr.Zero. In this case, this method does not provide the new seek pointer.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt64</typeparam>
        /// <returns>
        /// Returns in <paramref name="plibNewPosition"/> the location where this method writes the value of the new seek pointer from the beginning of the stream.
        /// </returns>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            if (TheStream != null)
            {
                // The enum values of SeekOrigin match the enum values of
                // STREAM_SEEK, so we can just cast the dwOrigin to a SeekOrigin

                if (plibNewPosition == IntPtr.Zero)
                {
                    // User isn't interested in new position
                    TheStream.Seek(dlibMove, (SeekOrigin)dwOrigin);
                }
                else
                {
                    SeekOrigin origin = (SeekOrigin)dwOrigin;
                    if (origin != SeekOrigin.Begin &&
                        origin != SeekOrigin.Current &&
                        origin != SeekOrigin.End)
                    {
                        origin = SeekOrigin.Begin;
                    }
                    Marshal.WriteInt64(plibNewPosition, TheStream.Seek(dlibMove, origin));
                }
            }
            else
            {
                TheIStream.Seek(dlibMove, dwOrigin, plibNewPosition);
            }
        }

        /// <summary>
        /// Changes the size of the stream object.
        /// </summary>
        /// <param name="libNewSize">Specifies the new size of the stream as a number of bytes.</param>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void SetSize(long libNewSize)
        {
            if (TheStream != null)
            {
                // Sets the length of the current stream.
                TheStream.SetLength(libNewSize);
            }
            else
            {
                TheIStream.SetSize(libNewSize);
            }
        }

        /// <summary>
        /// Retrieves the STATSTG structure for this stream.
        /// </summary>
        /// <remarks>
        /// The <paramref name="grfStatFlag"/> parameter is not used
        /// </remarks>
        /// <param name="pstatstg">
        /// The STATSTG structure where this method places information about this stream object.
        /// </param>
        /// <param name="grfStatFlag">
        /// Specifies that this method does not return some of the members in the STATSTG structure, 
        /// thus saving a memory allocation operation. This parameter is not used internally.
        /// </param>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Stat(out System.Runtime.InteropServices.ComTypes.STATSTG pstatstg, int grfStatFlag)
        {
            if (TheStream != null)
            {
                pstatstg = new System.Runtime.InteropServices.ComTypes.STATSTG();
                pstatstg.type = STGTY_STREAM;
                // Gets the length in bytes of the stream.
                pstatstg.cbSize = TheStream.Length;
                pstatstg.grfMode = STGM_READWRITE;
                pstatstg.grfLocksSupported = LOCK_EXCLUSIVE;
            }
            else
            {
                TheIStream.Stat(out pstatstg, grfStatFlag);
            }
        }

        /// <summary>
        /// Removes the access restriction on a range of bytes previously 
        /// restricted with the LockRegion method.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="libOffset">Specifies the byte offset for the beginning of the range.</param>
        /// <param name="cb">Specifies, in bytes, the length of the range to be restricted.</param>
        /// <param name="dwLockType">Specifies the access restrictions previously placed on the range.</param>
        ///<exception cref="NotSupportedException">The IO.Stream does not support unlocking.</exception>
        public void UnlockRegion(long libOffset, long cb, int dwLockType)
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support unlocking.");

            TheIStream.UnlockRegion(libOffset, cb, dwLockType);
        }

        /// <summary>
        /// Writes a specified number of bytes into the stream object 
        ///starting at the current seek pointer.
        /// </summary>
        /// <param name="pv">The buffer that contains the data that is to be written to the stream. 
        /// A valid buffer must be provided for this parameter even when cb is zero.</param>
        /// <param name="cb">The number of bytes of data to attempt to write into the stream. This value can be zero.</param>
        /// <param name="pcbWritten">
        /// A variable where this method writes the actual number of bytes written to the stream object. 
        /// The caller can set this to IntPtr.Zero, in which case this method does not provide the actual number of bytes written.
        /// </param>
        /// <typeparam name="pcbWritten">Native UInt32</typeparam>
        /// <returns>
        /// The actual number of bytes written (<paramref name="pcbWritten"/>).
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The IO.Stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            if (TheStream != null)
            {
                if (pcbWritten == IntPtr.Zero)
                {
                    // User isn't interested in how many bytes were written
                    TheStream.Write(pv, 0, cb);
                }
                else
                {
                    long currentPosition = TheStream.Position;
                    TheStream.Write(pv, 0, cb);
                    Marshal.WriteInt32(pcbWritten, (int)(TheStream.Position - currentPosition));
                }
            }
            else
            {
                TheIStream.Write(pv, cb, pcbWritten);
            }
        }

        // Default constructor. Should not be used to create an AStream object.
        private AStream()
        {
            TheStream = null;
            TheIStream = null;
        }

        // Copy constructor. It is not safe to only pass the Stream and IStream.
        private AStream(AStream previousAStream)
        {
            TheStream = previousAStream.TheStream;
            TheIStream = previousAStream.TheIStream;
        }

        /// <summary>
        /// Initializes a new instance of the AStream class.
        /// </summary>
        /// <param name="stream">An IO.Stream</param>
        ///<exception cref="ArgumentNullException">Stream cannot be null</exception>
        public AStream(Stream stream)
        {
            TheStream = null;
            TheIStream = null;

            if (stream == null)
            {
                throw new ArgumentNullException("Stream cannot be null");
            }
            TheStream = stream;
        }

        /// <summary>
        /// Initializes a new instance of the AStream class.
        /// </summary>
        /// <param name="stream">A ComTypes.IStream</param>
        ///<exception cref="ArgumentNullException">Stream cannot be null</exception>
        public AStream(IStream stream)
        {
            TheStream = null;
            TheIStream = null;

            if (stream == null)
            {
                throw new ArgumentNullException("IStream cannot be null");
            }
            TheIStream = stream;
        }

        // Allows the Object to attempt to free resources and perform other 
        // cleanup operations before the Object is reclaimed by garbage collection. 
        // (Inherited from Object.)
        ~AStream()
        {
            if (TheStream != null)
            {
                TheStream.Close();
            }
        }

        /// <summary>
        /// Releases all resources used by the Stream object.
        /// </summary>
        void IDisposable.Dispose()
        {
            Close();
        }

        /// <summary>
        /// Closes the current stream and releases any resources 
        /// (such as the Stream) associated with the current IStream.
        /// </summary>
        /// <remarks>
        /// This method is not a member in IStream.
        /// </remarks>
        public override void Close()
        {
            if (TheStream != null)
            {
                TheStream.Close();
            }
            else
            {
                TheIStream.Commit(STGC_DEFAULT);
//                Marshal.ReleaseComObject(TheIStream);    // Investigate this because we cannot release an IStream to the stash file
            }
            GC.SuppressFinalize(this);
        }

//        public static IStream ToIStream(object stream)
//        {
//            IntPtr ppv;
//            IntPtr pUnk = Marshal.GetIUnknownForObject(stream);
//            Object iSteam = null;
//            Guid iid = Marshal.GenerateGuidForType(typeof(IStream));   // ComTypes.IStream GUID

//            if (Marshal.QueryInterface(pUnk, ref iid, out ppv) == 0)
//                iSteam = Marshal.GetUniqueObjectForIUnknown(ppv);

//            return (System.Runtime.InteropServices.ComTypes.IStream)iSteam;
//        }

        public static IStream ToIStream(object stream)
        {
            if (stream is Stream)
            {
                return new AStream(stream as Stream);
            }

            if (stream is IStream)
            {
                return stream as IStream;
            }

            return null;
        }

        public static Stream ToStream(object stream)
        {
            if (stream is Stream)
            {
                return stream as Stream;
            }

            if (stream is IStream)
            {
                return new AStream(stream as IStream);
            }

            return null;
        }

        private const int STGTY_STREAM   = 2;
        private const int STGM_READWRITE = 2;
        private const int LOCK_EXCLUSIVE = 2;
        private const int STGC_DEFAULT   = 0;

        private Stream TheStream;   // The Stream being wrapped
        private IStream TheIStream; // The IStream being wrapped
    }
}
