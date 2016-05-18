/*-----------------------------------------------------------------------*
 * This file is part of the Microsoft IMAPIv2 Code Samples.              *
 *                                                                       *
 * Copyright (C) Microsoft Corporation.  All rights reserved.            *
 *                                                                       *
 * This source code is intended only as a supplement to Microsoft IMAPI2 *
 * help and/or on-line documentation.  See these other materials for     *
 * detailed information regarding Microsoft code samples.                *
 *                                                                       *
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY  *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *-----------------------------------------------------------------------*/

using System;
using IMAPI2.Interop;

namespace IMAPIv2
{
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

//    using IMAPI2.Interop;

    [ClassInterface(ClassInterfaceType.None)]
    class Samples
    {
        /// <summary>
        /// Burns data files to disc in a single session using files from a 
        /// single directory tree.
        /// </summary>
        /// <param name="recorder">Burning Device. Must be initialized.</param>
        /// <param name="path">Directory of files to burn.</param>
        public void BurnDirectory(IDiscRecorder2 recorder, String path)
        {
            // Define the new disc format and set the recorder
            IDiscFormat2Data dataWriterImage = new MsftDiscFormat2Data();
            dataWriterImage.Recorder = recorder;

            if(!dataWriterImage.IsRecorderSupported(recorder))
            {
                Console.WriteLine("The recorder is not supported");
                return;
            }

            if (!dataWriterImage.IsCurrentMediaSupported(recorder))
            {
                Console.WriteLine("The current media is not supported");
                return;
            }

            dataWriterImage.ClientName = "IMAPI Sample";

            // Create an image stream for a specified directory.

            // Create a new file system image and retrieve the root directory
            IFileSystemImage fsi = new MsftFileSystemImage();

            // Set the media size
            fsi.FreeMediaBlocks = dataWriterImage.FreeSectorsOnMedia;

            // Use legacy ISO 9660 Format
            fsi.FileSystemsToCreate = FsiFileSystems.FsiFileSystemISO9660;

            // Add the directory to the disc file system
            IFsiDirectoryItem dir = fsi.Root;
            dir.AddTree(path, false);

            // Create an image from the file system
            Console.WriteLine("Writing content to disc...");
            IFileSystemImageResult result = fsi.CreateResultImage();

            // Data stream sent to the burning device
            IStream stream = result.ImageStream;

            DiscFormat2Data_Events progress = dataWriterImage as DiscFormat2Data_Events;
            progress.Update += new DiscFormat2Data_EventsHandler(DiscFormat2Data_ProgressUpdate);

            // Write the image stream to disc using the specified recorder.
            dataWriterImage.Write(stream);   // Burn the stream to disc

            progress.Update -= new DiscFormat2Data_EventsHandler(DiscFormat2Data_ProgressUpdate);

            Console.WriteLine("----- Finished writing content -----");
        }

        /// <summary>
        /// Burns a boot image and data files to disc in a single session 
        /// using files from a single directory tree.
        /// </summary>
        /// <param name="recorder">Burning Device. Must be initialized.</param>
        /// <param name="path">Directory of files to burn.
        /// \\winbuilds\release\winmain\latest.tst\amd64fre\en-us\skus.cmi\staged\windows
        /// </param>
        /// <param name="bootFile">Path and filename of boot image.
        /// \\winbuilds\release\winmain\latest.tst\x86fre\bin\etfsboot.com
        /// </param>
        public void CreateBootDisc(IDiscRecorder2 recorder, String path, String bootFile)
        {
            // -------- Adding Boot Image Code -----
            Console.WriteLine("Creating BootOptions");
            IBootOptions bootOptions = new MsftBootOptions();
            bootOptions.Manufacturer = "Microsoft";
            bootOptions.PlatformId = PlatformId.PlatformX86;
            bootOptions.Emulation = EmulationType.EmulationNone;

            // Need stream for boot image file
            Console.WriteLine("Creating IStream for file {0}", bootFile);
            IStream iStream = new AStream(
                              new FileStream(bootFile, FileMode.Open,
                                                       FileAccess.Read,
                                                       FileShare.Read));
            bootOptions.AssignBootImage(iStream);

            // Create disc file system image (ISO9660 in this example)
            IFileSystemImage fsi = new MsftFileSystemImage();
            fsi.FreeMediaBlocks = -1; // Enables larger-than-CD image
            fsi.FileSystemsToCreate = FsiFileSystems.FsiFileSystemISO9660 |
                                      FsiFileSystems.FsiFileSystemJoliet  |
                                      FsiFileSystems.FsiFileSystemUDF;

            // Hooking bootStream to FileSystemObject
            fsi.BootImageOptions = bootOptions;

            // Hooking content files FileSystemObject
            fsi.Root.AddTree(path, false);

            IFileSystemImageResult result = fsi.CreateResultImage();
            IStream stream= result.ImageStream;

            // Create and write stream to disc using the specified recorder.
            IDiscFormat2Data dataWriterBurn = new MsftDiscFormat2Data();
            dataWriterBurn.Recorder = recorder;
            dataWriterBurn.ClientName = "IMAPI Sample";
            dataWriterBurn.Write(stream);

            Console.WriteLine("----- Finished writing content -----");
        }

        /// <summary>
        /// Examines and reports the burn device characteristics such 
        /// as Product ID, Revision Level, Feature Set and Profiles.
        /// </summary>
        /// <param name="recorder">Burning Device. Must be initialized.</param>
        public void DisplayRecorderCharacteristics(IDiscRecorder2 recorder)
        {
            // Create a DiscMaster2 object to obtain an inventory of CD/DVD drives.
            IDiscMaster2 discMaster = new MsftDiscMaster2();

            //*** - Formating the way to display info on the supported recoders
            Console.WriteLine("--------------------------------------------------------------------------------");
            Console.WriteLine("ActiveRecorderId: {0}".PadLeft(22), recorder.ActiveDiscRecorder);
            Console.WriteLine("Vendor Id: {0}".PadLeft(22), recorder.VendorId);
            Console.WriteLine("Product Id: {0}".PadLeft(22), recorder.ProductId);
            Console.WriteLine("Product Revision: {0}".PadLeft(22), recorder.ProductRevision);
            Console.WriteLine("VolumeName: {0}".PadLeft(22), recorder.VolumeName);
            Console.WriteLine("Can Load Media: {0}".PadLeft(22), recorder.DeviceCanLoadMedia);
            Console.WriteLine("Device Number: {0}".PadLeft(22), recorder.LegacyDeviceNumber);

            foreach (String mountPoint in recorder.VolumePathNames)
            {
                Console.WriteLine("Mount Point: {0}".PadLeft(22), mountPoint);
            }

            foreach (IMAPI_FEATURE_PAGE_TYPE supportedFeature in recorder.SupportedFeaturePages)
            {
                Console.WriteLine("Feature: {0}".PadLeft(22), supportedFeature.ToString("F"));
            }

            Console.WriteLine("Current Features");
            foreach (IMAPI_FEATURE_PAGE_TYPE currentFeature in recorder.CurrentFeaturePages)
            {
                Console.WriteLine("Feature: {0}".PadLeft(22), currentFeature.ToString("F"));
            }

            Console.WriteLine("Supported Profiles");
            foreach (IMAPI_PROFILE_TYPE supportedProfile in recorder.SupportedProfiles)
            {
                Console.WriteLine("Profile: {0}".PadLeft(22), supportedProfile.ToString("F"));
            }

            Console.WriteLine("Current Profiles");
            foreach (IMAPI_PROFILE_TYPE currentProfile in recorder.CurrentProfiles)
            {
                Console.WriteLine("Profile: {0}".PadLeft(22), currentProfile.ToString("F"));
            }

            Console.WriteLine("Supported Mode Pages");
            foreach (IMAPI_MODE_PAGE_TYPE supportedModePage in recorder.SupportedModePages)
            {
                Console.WriteLine("Mode Page: {0}".PadLeft(22), supportedModePage.ToString("F"));
            }

            Console.WriteLine("\n----- Finished content -----");
        }

        /// <summary>
        /// Examines and reports the media characteristics.
        /// </summary>
        /// <param name="recorder">Burning Device. Must be initialized.</param>
        public void DisplayMediaCharacteristics(IDiscRecorder2 recorder)
        {
            // Define the new disc format and set the recorder
            IDiscFormat2Data mediaImage = new MsftDiscFormat2Data();
            mediaImage.Recorder = recorder;

            // *** Validation methods inherited from IMAPI2.MsftDiscFormat2
            bool boolResult = mediaImage.IsRecorderSupported(recorder);
            if (boolResult)
            {
                Console.WriteLine("--- Current recorder IS supported. ---");
            }
            else
            {
                Console.WriteLine("--- Current recorder IS NOT supported. ---");
            }

            boolResult = mediaImage.IsCurrentMediaSupported(recorder);
            if (boolResult)
            {
                Console.WriteLine("--- Current media IS supported. ---");
            }
            else
            {
                Console.WriteLine("--- Current media IS NOT supported. ---");
            }

            Console.WriteLine("ClientName = {0}", mediaImage.ClientName);

            // Check a few CurrentMediaStatus possibilities. Each status is associated 
            // with a bit and some combinations are legal.
            uint curMediaStatus = (uint)mediaImage.CurrentMediaStatus;
            Console.WriteLine("Checking Current Media Status");

            if (curMediaStatus == (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN)
            {
                Console.WriteLine("\tMedia state is unknown.");
            }
            else
            {
                if ((curMediaStatus & (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY) != 0)
                {
                    Console.WriteLine("\tCurrently, only overwriting is supported.");
                }
                if ((curMediaStatus & (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_RANDOMLY_WRITABLE) != 0)
                {
                    Console.WriteLine("\tCurrently, media supports random writing.");
                }
                if ((curMediaStatus & (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_APPENDABLE) != 0)
                {
                    Console.WriteLine("\tMedia is currently appendable.");
                }
                if ((curMediaStatus & (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_FINAL_SESSION) != 0)
                {
                    Console.WriteLine("\tMedia is in final writing session.");
                }
                if ((curMediaStatus & (uint)IMAPI_FORMAT2_DATA_MEDIA_STATE.IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED) != 0)
                {
                    Console.WriteLine("\tMedia is damaged.");
                }
            }

            IMAPI_MEDIA_PHYSICAL_TYPE mediaType = mediaImage.CurrentPhysicalMediaType;
            Console.Write("Current Media Type");
            DisplayMediaType(mediaType);

            Console.WriteLine("SupportedMediaTypes in the device: ");
            foreach (IMAPI_MEDIA_PHYSICAL_TYPE supportedMediaType in mediaImage.SupportedMediaTypes)
            {
                DisplayMediaType(supportedMediaType);
            }

            Console.Write("\n----- Finished -----");
        }

        /// <summary>
        /// Helper Function: Displays the string of the specified IMAPI_MEDIA_PHYSICAL_TYPE
        /// </summary>
        /// <param name="mediaType">The IMAPI_MEDIA_PHYSICAL_TYPE to display</param>
        private void DisplayMediaType(IMAPI_MEDIA_PHYSICAL_TYPE mediaType)
        {
            switch (mediaType)
            {
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_UNKNOWN:
                    {
                        Console.WriteLine("\tEmpty device or an unknown disc type.");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_CDROM:
                    {
                        Console.WriteLine("\tCD-ROM");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_CDR:
                    {
                        Console.WriteLine("\tCD-R");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_CDRW:
                    {
                        Console.WriteLine("\tCD-RW");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDROM:
                    {
                        Console.WriteLine("\tRead-only DVD drive and/or disc");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDRAM:
                    {
                        Console.WriteLine("\tDVD-RAM");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDPLUSR:
                    {
                        Console.WriteLine("\tDVD+R");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDPLUSRW:
                    {
                        Console.WriteLine("\tDVD+RW");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER:
                    {
                        Console.WriteLine("\tDVD+R Dual Layer media");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDDASHR:
                    {
                        Console.WriteLine("\tDVD-R");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDDASHRW:
                    {
                        Console.WriteLine("\tDVD-RW");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER:
                    {
                        Console.WriteLine("\tDVD-R Dual Layer media");
                    } break;
                case IMAPI_MEDIA_PHYSICAL_TYPE.IMAPI_MEDIA_TYPE_DISK:
                    {
                        Console.WriteLine("\tRandomly-writable, hardware-defect managed media type" +
                                          " that reports the \"Disc\" profile as current.");
                    } break;
                default:
                    {
                        Console.WriteLine("Error!: MediaPhysicalType");
                    } break;
            }
        }

        /// <summary>
        /// Event handler - Progress updates when writing data
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args">Argument of type IDiscFormat2DataEventArgs</param>
        public void DiscFormat2Data_ProgressUpdate([In, MarshalAs(UnmanagedType.IDispatch)] object sender, [In, MarshalAs(UnmanagedType.IDispatch)] object args)
        {
            IDiscFormat2DataEventArgs progress = args as IDiscFormat2DataEventArgs;

            String timeStatus = String.Format("Time: {0} / {1} ({2})", 
                                progress.ElapsedTime,
                                progress.TotalTime,
                                progress.ElapsedTime / progress.TotalTime);
            switch (progress.CurrentAction)
            {
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA:
                    {
                        Console.Write("Validating media. ");
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA:
                    {
                        Console.Write("Formatting media. ");
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE:
                    {
                        Console.Write("Initializing Hardware. ");
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER:
                    {
                        Console.Write("Calibrating Power (OPC). ");
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA:
                    {
                        int totalSectors = progress.SectorCount;
                        int writtenSectors = progress.LastWrittenLba - progress.StartLba;
                        int percentDone = writtenSectors / totalSectors;
                        Console.Write("Progress: {0} - ", percentDone);
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION:
                    {
                        Console.Write("Finishing the writing. ");
                    } break;
                case IMAPI_FORMAT2_DATA_WRITE_ACTION.IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED:
                    {
                        Console.Write("Completed the burn.");
                    } break;
                default:
                    {
                        Console.Write("Error!!!! Unknown Action: 0x{0:X}", progress.CurrentAction);
                    } break;
            }
            Console.WriteLine(timeStatus);
        }
    }
}

class IMAPIv2Samples
{
    static void Main(string[] args)
    {
        // Index to recording drive.
        int index = 0;

        // Create a DiscMaster2 object to connect to CD/DVD drives.
        IDiscMaster2 discMaster = new MsftDiscMaster2();

        // Initialize the DiscRecorder object for the specified burning device.
        IDiscRecorder2 recorder = new MsftDiscRecorder2();
        recorder.InitializeDiscRecorder(discMaster[index]);

        IMAPIv2.Samples samples = new IMAPIv2.Samples();
        
        samples.BurnDirectory(recorder, @"D:\Utils");
    }
}
