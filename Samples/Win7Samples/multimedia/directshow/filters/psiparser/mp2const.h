
/*++

    Copyright (c) Microsoft Corporation

    Module Name:

        mp2const.h

    Abstract:

        This module contains MPEG2-specific constants, macros, etc...

        The MPEG parsing macros are in parsemac.h.

        It is included in precomp.h so there's no reason to include
        it explicitely.

--*/


#ifndef __mp2demux_mp2const_h
#define __mp2demux_mp2const_h


//  transport stream - related

//  macro

//  MPEG-2 systems specification:
//      section_length is the bytes remaining AFTER the section_length field;
//      since all PSI has section_length at the same offset, we can add 3 bytes
//      to section_length and obtain the real PSI section length
#define COMPLETE_SECTION_LENGTH(sl)                     ((sl) + 3)

//  ----------------------------------------------------------------------------
//  general constants

//  transport
#define DISTINCT_PID_COUNT                              (1 << 13)
#define MAX_PID_VALUE                                   (DISTINCT_PID_COUNT - 1)
#define TS_PACKET_SIZE                                  188
#define TS_PACKET_SYNC_BYTE                             0x47
#define TS_SIMPLE_PACKET_HEADER_SIZE                    4
#define TS_NULL_PID                                     0x1fff
#define PCR_BIT_COUNT                                   48
#define PCR_PID_NOT_DEFINED_FOR_PROGRAM                 0x1fff

//  program
#define DISTINCT_STREAM_ID_COUNT                        (1 << 8)
#define MAX_STREAM_ID_VALUE                             (DISTINCT_STREAM_ID_COUNT - 1)

#define ATSC_BITRATE                                    19200000

//  MPEG-2 PSI constants

#define PAT_MAX_SECTION_LENGTH_FIELD_VALUE              0x3fd
#define PMT_MAX_SECTION_LENGTH_FIELD_VALUE              0x3fd
#define CAT_MAX_SECTION_LENGTH_FIELD_VALUE              0x3fd
#define PRIVATE_MAX_SECTION_LENGTH_FIELD_VALUE          0x3fd
#define PURE_PRIVATE_MAX_SECTION_LENGTH_FIELD_VALUE     0xffd
#define PSI_PAT_PID                                     0
#define PSI_MAX_SECTION_LENGTH_NUMBER                   PURE_PRIVATE_MAX_SECTION_LENGTH_FIELD_VALUE
#define PSI_MAX_SECTION_LENGTH                          COMPLETE_SECTION_LENGTH (PSI_MAX_SECTION_LENGTH_NUMBER)
#define PSI_NON_VERSIONED_HEADER_SIZE                   3
#define PSI_VERSIONED_HEADER_SIZE                       8

//  max size rounded to the next highest transport packet boundary
#define PSI_MAX_SECTION_LENGTH_ROUNDED                  (TS_PACKET_SIZE * DIV_ROUND_UP_MAYBE (PSI_MAX_SECTION_LENGTH, TS_PACKET_SIZE))

//  see table 2-26, MPEG-2 systems spec
#define PAT_TABLE_ID                                    0x00
#define CAT_TABLE_ID                                    0x01
#define PMT_TABLE_ID                                    0x02
#define FORBIDDEN_TABLE_ID                              0xff

//  internal parser types
#define MPEG2_MEDIA_ELEMENTARY_STREAM                   0
#define MPEG2_MEDIA_PES_STREAM                          1
#define MPEG2_MEDIA_TRANSPORT_PSI_PAT                   2
#define MPEG2_MEDIA_TRANSPORT_PSI_CAT                   3
#define MPEG2_MEDIA_TRANSPORT_PSI_PMT                   4
#define MPEG2_MEDIA_TRANSPORT_PSI_PRIVATE_PURE          5
#define MPEG2_MEDIA_TRANSPORT_PSI_PRIVATE_NOT_PURE      6
#define MPEG2_MEDIA_TRANSPORT_PCR                       7
#define MPEG2_MEDIA_TRANSPORT_PACKET                    8
#define MPEG2_MEDIA_TRANSPORT_PSI                       9
#define MPEG2_MEDIA_TRANSPORT_PAYLOAD                   10
#define MPEG2_MEDIA_PROGRAM_STREAM_MAP                  11
#define MPEG2_MEDIA_PROGRAM_SCR                         12
#define MPEG2_MEDIA_PROGRAM_DIRECTORY_PES_PACKET        13
#define MPEG2_MEDIA_PROGRAM_PACK_HEADER                 14
#define MPEG2_MEDIA_PROGRAM_SYSTEM_HEADER               15
#define MPEG2_MEDIA_PROGRAM_MACROVISION                 16

#define MPEG2_DEFAULT_VIDEO_STREAM                      0x10000000
#define MPEG2_DEFAULT_AUDIO_STREAM                      0x20000000

//  max program stream PES packet length
#define MAX_PS_PES_PACKET_LENGTH                        0x10000

//  start codes, etc...
#define START_CODE_PREFIX                               0x000001
#define MPEG2_PACK_START_CODE                           0xBA
#define MPEG2_SYSTEM_HEADER_START_CODE                  0xBB
#define PS_PMT_START_CODE                               0XBC
#define PS_DIRECTORY_START_CODE                         0XFF
#define PROGRAM_END_CODE                                0xB9
#define MPEG2_SEQUENCE_HEADER_START_CODE                0xB3
#define MPEG2_EXTENSION_START_CODE                      0xB5

#define MPEG2_SEQUENCE_HEADER_LENGTH                    10

//  start_code_prefix + prefix
//  00 00 01
#define START_CODE_PREFIX_LENGTH                        3
//  xx
#define PREFIX_LENGTH                                   1
//  00 00 01 xx
#define START_CODE_LENGTH                               (START_CODE_PREFIX_LENGTH + PREFIX_LENGTH)

//  fields: [pack_start_code, pack_stuffing_length]
#define PACK_HEADER_CORE_LEN                            14

//  fields: [pack_start_code, marker_bit (4th)]
#define PACK_HEADER_SCR_LEN                             10

//  pack payload i.e. PES packet, PS_PMT packet, etc... have this core header
//  length
#define PACK_PAYLOAD_HEADER_LEN                         6

//  ----------------------------------------------------------------------------
//  PES packet - related

#define PES_START_CODE_PREFIX                           START_CODE_PREFIX

//  all PES headers start with these values
#define PES_COMMON_HEADER_FIELD_LEN                     (3 + 1 + 2)

//  tier1 req headers are packet_start_code_prefix -> PES_header_data_length
//  see MPEG2 systems spec, p. 30 for field widths
#define PES_TIER1_REQ_HEADER_FIELD_LEN                  (PES_COMMON_HEADER_FIELD_LEN + 1 + 1 + 1)

//  largest size 188 evenly divisible size <= 64K
#define PES_DEFAULT_BUFFER_SIZE_REQUEST                 65424

//  offset into PES header of last byte of PES_packet_length field
#define PES_PACKET_LENGTH_LAST_BYTE_OFFSET              6

//  tier_1 + all optional fields
#define MAX_PES_HEADER_LEN                              (PES_TIER1_REQ_HEADER_FIELD_LEN + 210)

//  PES stream id (table 2-18, MPEG2 Systems Specification, H.222.0)

#define STREAM_ID_MIN                                   0xBA
#define STREAM_ID_MAX                                   0xFF

#define STREAM_ID_PROGRAM_STREAM_MAP                    0xBC
#define STREAM_ID_PRIVATE_STREAM_1                      0xBD
#define STREAM_ID_PADDING_STREAM                        0xBE
#define STREAM_ID_PRIVATE_STREAM_2                      0xBF

#define STREAM_ID_AUDIO_STREAM_MIN                      0xC0
#define STREAM_ID_AUDIO_STREAM_MAX                      0xDF

#define STREAM_ID_VIDEO_STREAM_MIN                      0xE0
#define STREAM_ID_VIDEO_STREAM_MAX                      0xEF

#define STREAM_ID_ECM                                   0xF0
#define STREAM_ID_EMM                                   0xF1
#define STREAM_ID_DSMCC_STREAM                          0xF2
#define STREAM_ID_IEC_13522                             0xF3
#define STREAM_ID_H_222_1_TYPE_A                        0xF4
#define STREAM_ID_H_222_1_TYPE_B                        0xF5
#define STREAM_ID_H_222_1_TYPE_C                        0xF6
#define STREAM_ID_H_222_1_TYPE_D                        0xF7
#define STREAM_ID_H_222_1_TYPE_E                        0xF8
#define STREAM_ID_ANCILLARY                             0xF9
#define STREAM_ID_RESERVED_DATA_MIN                     0xFA
#define STREAM_ID_RESERVED_DATA_MAX                     0xFE
#define STREAM_ID_PROGRAM_STREAM_DIRECTORY              0xFF

//  ----------------------------------------------------------------------------
//  macrovision stream_id

#define STREAM_ID_MACROVISION                           STREAM_ID_PRIVATE_STREAM_2
#define SUBSTREAM_ID_MACROVISION                        (0)
#define MACROVISION_WORD_OFFSET                         (PES_COMMON_HEADER_FIELD_LEN + 1 + 4)
#define MIN_VALID_MACROVISION_PES_PACKET_LENGTH         (MACROVISION_WORD_OFFSET + 2)

//  ---------------------------------------------------------------------------
//  timing constants

//  PCRs are in adaptation fields ==> transport streams
//  H.222.0, D.0.2 "Audio and Video Presentation Synchronization", pp. 95
#define MAX_INTRA_PTS_INTERVAL_MILLIS                   700

//  H.222.0, D.0.3 "System Time Clock recovery in the decoder", pp. 96
#define MAX_INTRA_PCR_INTERVAL_MILLIS                   100
#define MAX_PCR_RATE                                    (MILLISECONDS_PER_SECOND / MAX_INTRA_PCR_INTERVAL_MILLIS)

//  SCRs are in pack headers ==> program streams
//  H.222.0, D.0.2 "Audio and Video Presentation Synchronization", pp. 95
#define MAX_INTRA_SCR_INTERVAL_MILLIS                   700

//  SCRs are held in a 48 bit counter
#define MAX_SCR_VALUE                                   ((1 << 48) - 1)

//
//  H.222.0, 2.4.2.1 places the following restraint on rate of change of the
//  system clock :
//
//      rate of change of system_clock_frequency <= 75 x 10^-3 Hz/s
//
//  since we use QPCs to expose a clock (that in turn is slaved to PCRs), the
//  above constraint must be enforced against the QPC frequency on the local
//  host, over time
//
//  our slaving schema is to compute a scaling value (slope) over time, which
//  we multiply against a QPC value to skew the value appropriately (greater,
//  or less, depending on the PCR-QPC relationship)
//
//  thus, given QPC(f) as being the QPC frequency (Hz/s) on the local host,
//  and N(i) being a current "skewing" value that changes over time as we
//  slave to the PCRs, our maximum allowable rate of change is computed as
//  follows [QPC(f) changes from host to host, but is constant on a single
//  host]:
//
//      [i : seconds]
//
//      abs (N(i) * QPC(f) - N(i+1) * QPC(f)) <= 0.075
//  or
//      abs (N(i) - N(i+1)) * QPC(f) <= 0.075
//  or
//      abs (N(i) - N(i+1)) <= 0.075 / QPC(f)
//
//  we can then use the above formula, coupled with the maximum rate of
//  arrival of PCRs, to compute a maximum "clock step" with which to
//  correct our scaling value:
//
//      [k : PCRs]
//
//      PCR_rate = MILLISECONDS_PER_SECOND / MAX_INTRA_PCR_INTERVAL_MILLIS
//
//      abs (N(k) - N(k+1)) <= (0.075 / QPC(f)) / PCR_rate
//  or
//      abs (N(k) - N(k+1)) <= 0.075 / (QPC(f) * PCR_rate)
//

//  macro yields a double value that is the maximum rate change, from
//  pcr to pcr; mult is a registry supplied value that defaults to 1,
//  but can be increased to increase the rate of closure between what we
//  are observing and what we are using to slave with
#define MAX_SLOPE_STEP_VALUE(qpc_freq,mult)            ( (double) (((0.075) * (double) (mult)) / (double (qpc_freq) * double (MAX_PCR_RATE))) )

//  this is a multiplier that is used in the allowable error bracket, which
//  is a multiplier, within which we allow the clock to "jitter" i.e. drift
//  up and down without triggering a correction
#define ACCEPTABLE_CLOCK_JITTER_MULTIPLIER              10.0

//  macro yields the max correction threshold frame size;
#define MAX_ALLOWABLE_ERROR_BRACKET(max_clock_step)     ((max_clock_step) * ACCEPTABLE_CLOCK_JITTER_MULTIPLIER)

//  macro yields the correction frame size degradation; current frame
//  size degrades when no corrections are made;
#define ERROR_BRACKET_GRANULARITY                       1000.0
#define ERROR_BRACKET_DEGRADATION(max_clock_step)       (MAX_ALLOWABLE_ERROR_BRACKET(max_clock_step) / ERROR_BRACKET_GRANULARITY)

#endif  //  __mp2demux_mp2const_h