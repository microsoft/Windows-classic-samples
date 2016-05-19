// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: Resource.h
//
// Desc: DirectShow sample code - DV control/capture example.
//------------------------------------------------------------------------------

#include <commctrl.h>

#define IDR_MENU                        100
#define IDB_TOOLBAR                     102
#define IDB_STATUS                      103
#define ID_TOOLBAR                      1
#define IDC_STATIC                      -1

// Menu
#define IDM_SEEKTIMECODE                1012
#define IDM_SETOUTPUT                   1020
#define IDM_SETINPUT                    1021
#define IDM_CAPSIZE                     1022
#define IDM_EXIT                        1023

#define IDM_PREVIEW                     1030
#define IDM_FILETODV                    1031
#define IDM_FILETODV_NOPRE              1032
#define IDM_DVTOFILE                    1033
#define IDM_DVTOFILE_NOPRE              1034
#define IDM_FILETODV_TYPE2              1035
#define IDM_FILETODV_NOPRE_TYPE2        1036
#define IDM_DVTOFILE_TYPE2              1037
#define IDM_DVTOFILE_NOPRE_TYPE2        1038
#define IDM_ABOUT                       1039
#define IDM_OPTIONS_SAVEGRAPH           1043

#define IDC_EDIT_HOUR                   2000
#define IDC_EDIT_MINUTE                 2001
#define IDC_EDIT_SECOND                 2002
#define IDC_EDIT_FRAME                  2003
#define IDC_TCCHECKBOX                  2004

#define IDC_RADIO_TIME                  2010
#define IDC_RADIO_SIZE                  2011
#define IDC_RADIO_NOLIMIT               2012
#define IDC_EDIT_TIME                   2013
#define IDC_EDIT_SIZE                   2014
#define IDC_SPIN_SIZE                   2015
#define IDC_SPIN_TIME                   2016
#define IDC_BUTTON_CAMERA               2017
#define IDC_BUTTON_VCR                  2018

#define IDC_RADIO_88x60                 3000
#define IDC_RADIO_180x120               3001
#define IDC_RADIO_360x240               3002
#define IDC_RADIO_720x480               3003

// Toolbar
#define IDM_STOP                        1000
#define IDM_PLAY                        1001
#define IDM_PAUSE                       1002
#define IDM_RECORD                      1003
#define IDM_FF                          1004
#define IDM_REW                         1005
#define IDM_PLAY_FAST_FF                1006
#define IDM_PLAY_FAST_REV               1007

#define IDM_STEP_FWD                    1010
#define IDM_STEP_REV                    1011

#define IDD_ABOUT                       101
#define IDD_DIALOG_CAPSIZE              104
#define IDD_DIALOG_CHOOSEMODE           105
#define IDD_DIALOG_DECODESIZE           106

#define IDM_CAPSIZE                     1022
#define IDM_REFRESHMODE                 1042
#define IDM_DECODESIZE                  1040
#define IDM_CHECKTAPE                   1041
#define IDM_FRAMERATE                   1044


#define DV_APPTITLE              TEXT("Digital Video Sample Application")
#define APPNAME                  TEXT("DV App")
#define DEFAULT_CAP_FILE_NAME    TEXT("c:\\DVApp.avi")
#define DEFAULT_FG_FILE_NAME     TEXT("c:\\DVApp.grf")
#define _MAX_SLEEP               500

#define WM_FGNOTIFY              WM_USER+1



// Toolbar buttons 

TBBUTTON g_rgTbButtons[] = 
{ 
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},     
    {8, IDM_STEP_REV,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {5, IDM_REW,            TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {9, IDM_PLAY_FAST_REV,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {0, IDM_PLAY,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0}, 
    {6, IDM_PLAY_FAST_FF,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {4, IDM_FF,             TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {7, IDM_STEP_FWD,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {1, IDM_PAUSE,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0}, 
    {2, IDM_STOP,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0},
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {3, IDM_RECORD,         TBSTATE_INDETERMINATE, TBSTYLE_BUTTON, 0,0},
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {0, 0,                  TBSTATE_ENABLED, TBSTYLE_SEP,    0,0},   
    {10, IDM_SEEKTIMECODE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0,0}
};


// Timer IDs
#define DV_TIMER_ATN             1L
#define DV_TIMER_CAPLIMIT        2L
#define DV_TIMER_FRAMES          3L

#define DV_TIMERFREQ             55   // milliseconds between timer ticks

#define DEFAULT_VIDEO_WIDTH      720  // full resolution width for NTSC
#define DEFAULT_VIDEO_HEIGHT     576  // full resolution height for PAL

#define WIDTH_EDGE               5
#define HEIGHT_EDGE              95
