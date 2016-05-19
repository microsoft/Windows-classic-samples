MultiChan (MFC)

Copyright (c) Microsoft Corporation. All rights reserved.



What does MultiChan do
=========================

	MultiChan is a MFC based app that allows the user to create > 2 channel .wav fiels from standard 1 and 2 channel PCM ones. 
The user has the ability to select the number of channels, sample rate, bit depth and speaker configuration of the generated resource. 
The resulting .wav files are WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, depending on the number of channels.

	The waveform resources can be rendered (played/previewed) and saved to a file. 


How to build the sample
=========================
	Install the Platform SDK, run setenv.bat and then run nmake.exe in this directory.


How to run the sample
=========================

	Run MultiChan.exe. 
	From the File section of the menu, open any number of 1- or 2- channel .wav files - these will be split in mono .wav files. Sources can be previewed and can be assigned specific speaker positions.
	On the "destination" dialog, select the characteristics of the output file (bit depth, sample rate, format tag and valid bits per sample) and remix the opened sources into destination. The destination mix can be rendered and/or written to disk.


Targeted platforms
=========================

	MultiChan is runnable on WinME, Windows2000 and Whistler (i386 and ia64) platforms.
	

APIs used in the sample
=========================

	MMIO functions : mmioOpen, mmioAscend, mmioDescend, mmioClose
	Win32 IO functions : CreateFile, WriteFile, CloseHandle
	ACM functions : acmStreamOpen, acmStreamSize, acmStreamPrepareHeader, acmStreamUnprepareHeader, acmStreamConvert, acmStreamClose
	WAVE functions : waveOutOpen, waveOutPrepareHeader, waveOutUnprepareHeader, waveOutWrite, waveOutClose

	The app also makes use of the following : WAVEFORMATEX struct, WAVE_FORMAT_EXTENSIBLE, WAVE_FORMAT_PCM types, waveOutProc callback function.
