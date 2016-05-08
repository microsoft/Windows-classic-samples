// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SDKDIFFRC_H__
#define __SDKDIFFRC_H__

#define IDC_STATIC      -1

#define IDM_FILE        111
#define IDM_DIR         112
#define IDM_PRINT       113
#define IDM_EXIT        115
#define IDM_TIME        116
#define IDM_TRACE       117
#define IDM_TRACEOFF    118
#define IDM_OPEN        121
#define IDM_SAVELIST    122
#define IDM_EDITLEFT    123
#define IDM_EDITRIGHT   124
#define IDM_EDITCOMP    125
#define IDM_SETEDIT     126
#define IDM_COPYFILES   127
#define IDM_EDITCOPY    128

#define IDM_CONTENTS    130
#define IDM_ABOUT       131
#define IDD_ICON        132
#define IDD_VERSION     133
#define IDD_LABEL       136
#define IDD_DIR1        137
#define IDD_DIR2        138
#define IDD_LAB1        139
#define IDD_LAB2        140
#define IDD_LAB3        141
#define IDD_RECURSIVE   143
#define IDD_SERVER      145
#define IDD_PASSWORD    146
#define IDD_SUMS        147
#define IDD_FASTSCAN    148
#define IDD_AUTOCOPY    149
#define IDD_BOTHREMOTE  150
#define IDD_LOGFILE     151
#define IDD_LOGTEXT     152
#define IDD_FIND        153
#define IDC_DRD_FINDWHAT  154
#define IDC_OPT_UP        155
#define IDC_OPT_DOWN      156
#define IDC_CHK_MATCHCASE 157
#define IDC_CHK_WHOLEWORD 158

#define IDD_GOTOLINE    159
#define IDC_EDT_GOTOLINE 160

#define IDM_CLOSE       161

#define IDD_USAGE       170
#define IDC_USAGE_TEXT  171

#define IDM_OUTLINE     180
#define IDM_EXPAND      181
#define IDM_FCHANGE     182
#define IDM_LNRS        185
#define IDM_RNRS        186
#define IDM_NONRS       187
#define IDM_LONLY       188
#define IDM_RONLY       189
#define IDM_BOTHFILES   190
#define IDM_UPDATE      194
#define IDM_FPCHANGE    195
#define IDM_IGNBLANKS   196
#define IDM_PICTURE     197
#define IDM_LAZY        198
#define IDM_ALG2        199
#define IDM_OUTLINE_INCSAME     201
#define IDM_OUTLINE_INCLEFT     202
#define IDM_OUTLINE_INCRIGHT    203
#define IDM_OUTLINE_INCDIFFER   204
#define IDM_EXPAND_INCSAME     205
#define IDM_EXPAND_INCLEFT     206
#define IDM_EXPAND_INCRIGHT    207
#define IDM_EXPAND_INCMOVEDLEFT   208
#define IDM_EXPAND_INCMOVEDRIGHT  209
#define IDM_EXPAND_INCSIMILARLEFT   210
#define IDM_EXPAND_INCSIMILARRIGHT  211
#define IDM_MONOCOLS    212
#define IDM_RESCAN      213
#define IDM_SHOWWHITESPACE  214
#define IDM_FIND        215
#define IDM_FINDNEXT    216
#define IDM_FINDPREV    217
#define IDM_GOTOLINE    218
#define IDM_SETTABWIDTH	219
#define IDM_SETFONT		220
#define IDM_FCHANGE_LAURIE		221
#define IDM_FPCHANGE_LAURIE		222
#define IDM_TABWIDTH4   223
#define IDM_TABWIDTH8   224

#define IDM_MARK        300
#define IDM_MARKPATTERN 301
#define IDM_HIDEMARK    302
#define IDM_TOGGLEMARK  303
#define IDD_IGNOREMARK  304
#define IDD_ATTRIBUTES  305
#define IDM_TOMOVED     310

#define IDC_RCDISP1     501
#define IDC_BAR         510
#define IDC_STATUS      550
#define IDM_ABORT       551
#define IDL_STATLAB     552
#define IDL_NAMES       553

#define IDD_FILE        601
#define IDD_IDENTICAL   602
#define IDD_DIFFER      603
#define IDD_LEFT        604
#define IDD_RIGHT       605

#define IDD_FROMGROUP   610
#define IDD_FROMLEFT    611
#define IDD_FROMRIGHT   612

#define IDS_LINE_NOT_VISIBLE        650
#define IDS_ONLY_IN                 651
#define IDS_DIFFERENT               652
#define IDS_IDENTICAL               653
#define IDS_SAME_SIZE               654
#define IDS_CANT_OPEN_TMP_FILE      655
#define IDS_BAD_ARGUMENT            656
#define IDS_SELECT_FIRST_FILE       657
#define IDS_SELECT_SECOND_FILE      658
#define IDS_COULDNT_FIND            659
#define IDS_CANT_OPEN               660
#define IDS_IDENTICAL_COMMA         661
#define IDS_LEFT_ONLY_COMMA         662
#define IDS_RIGHT_ONLY_COMMA        663
#define IDS_DIFFERING               664
#define IDS_FILES_LISTED            665
#define IDS_ENTER_DIR_NAME          666
#define IDS_FAILED_TO_COPY          667
#define IDS_COPYING                 668
#define IDS_COPY_ABORTED            669
#define IDS_COPY_FAILED             670
#define IDS_COPY_COMPLETE           671
#define IDS_EXIT                    672
#define IDS_COULDNT_FIND_ANYTHING   673
#define IDS_SDKDIFF_USAGE           689
#define IDS_ERROR_L_OPTION          690
#define IDS_PAGE                    692
#define IDS_SENT_TO_PRINTER         693
#define IDS_UNABLE_TO_PRINT         694
#define IDS_SDKDIFF_ERROR           695
#define IDS_NO_MORE_CHANGES         696
#define IDS_NO_PREV_CHANGES         697
#define IDS_OUTLINE                 698
#define IDS_EXPAND                  699
#define IDS_FILE_DOESNT_EXIST       700
#define IDS_FAILED_TO_LAUNCH_EDT    701
#define IDS_COMPARING               702
#define IDS_SCANNING                703
#define IDS_ABORT                   704
#define IDS_PLEASE_WAIT             705
#define IDS_ABORT_PENDING           706
#define IDS_SECONDS                 707
#define IDS_TRACING_ENABLED         708
#define IDS_TRACING_DISABLED        709
#define IDS_CREATE_DIFF_LIST        710
#define IDS_TOOL_DESCRIPTION        711
#define IDS_EDITOR_COMMAND          712
#define IDS_HEADER_LINE_STR         713
#define IDS_IDENTICAL_TIMES         714
#define IDS_DIFF_BLANK_ONLY         715
#define IDS_ENTER_SUBSTRING1        716
#define IDS_ENTER_SUBSTRING2        717
#define IDS_MARK_FILES              718
#define IDS_ENTER_PASSWORD          719
#define IDS_COPYING_NFILES          720
#define IDS_BOTH_UNREADABLE         721
#define IDS_LEFT_UNREADABLE         722
#define IDS_RIGHT_UNREADABLE        723
#define IDS_UNREADABLE              724
#define IDS_IS_MORE_RECENT          725
#define IDS_IS_READONLY             726
#define IDS_COPY_FILES              727
#define IDS_NOTHING_RESCANNED       728
#define IDS_NOTHING_TO_EDIT         729
#define IDS_ERR_OPENING_FILE        730
#define IDS_SAVE_COMPFILE           731

#define IDS_USAGE_STR00             732
#define IDS_USAGE_STR01             733
#define IDS_USAGE_STR02             734
#define IDS_USAGE_STR03             735
#define IDS_USAGE_STR04             736
#define IDS_USAGE_STR05             737
#define IDS_USAGE_STR06             738
#define IDS_USAGE_STR07             739
#define IDS_USAGE_STR08             740
#define IDS_USAGE_STR08B            741
#define IDS_USAGE_STR09             742
#define IDS_USAGE_STR10             743
#define IDS_USAGE_STR11             744
#define IDS_USAGE_STR12             745
#define IDS_USAGE_STR12B            746
#define IDS_USAGE_STR12C            747
#define IDS_USAGE_STR13             748
#define IDS_USAGE_STR14             749
#define IDS_USAGE_STR15             750
#define IDS_USAGE_STR16             751
#define IDS_USAGE_STR17             752
#define IDS_USAGE_STR18             753
#define IDS_USAGE_STR19             754
#define IDS_USAGE_STR20             755
#define IDS_USAGE_STR21             756
#define IDS_USAGE_STR22             757
#define IDS_USAGE_STR23             758
#define IDS_USAGE_STR24             759
#define IDS_USAGE_STR25             760
#define IDS_USAGE_STR26             761

#define IDS_GOTOLINE_INVALIDSTRING  800
#define IDS_GOTOLINE_NOLINES        801
#define IDS_FIND_NOTFOUND           802
#define IDS_TABWIDTH				803
#define IDS_BAD_TABWIDTH			804
#define IDS_BAD_SD_INI				805
#define IDS_NOCOMPUNICODE			806
#define IDS_ERROR_LO_UNC            807
#define IDS_ERROR_OVSD_OPTION		808
#define IDS_ERROR_LD_OPTION			809
// --- UNUSED - RECYCLE ME ---          810
#define IDS_ERROR_IARGS				811
#define IDS_ERROR_IARGS_OPENFILE	812
#define IDS_ERROR_CANTLOADRICHEDIT	813

#endif
