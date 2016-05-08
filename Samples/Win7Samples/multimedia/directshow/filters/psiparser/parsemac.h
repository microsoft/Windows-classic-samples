
/*++

    Copyright (c) Microsoft Corporation

    Module Name:

        parsemac.h

    Abstract:

        This module contains macros that are used to parse the various
        headers

--*/

#ifndef __mp2demux_parsemac_h
#define __mp2demux_parsemac_h

//  Remove our dependency on ws2_32.dll by implementing our own
//  little-endian <--> big-endian byte swapping.  The MPEG-2
//  systems spec is all big-endian, so we need these macros
//  when we don't run on big-endian architectures e.g. x86.

#ifndef BIG_ENDIAN
//  we're little-endian and need to swap bytes around

#define NTOH_LL(ll)                                             \
                    ( (((ll) & 0xFF00000000000000) >> 56)  |    \
                      (((ll) & 0x00FF000000000000) >> 40)  |    \
                      (((ll) & 0x0000FF0000000000) >> 24)  |    \
                      (((ll) & 0x000000FF00000000) >> 8)   |    \
                      (((ll) & 0x00000000FF000000) << 8)   |    \
                      (((ll) & 0x0000000000FF0000) << 24)  |    \
                      (((ll) & 0x000000000000FF00) << 40)  |    \
                      (((ll) & 0x00000000000000FF) << 56) )

#define NTOH_L(l)                                       \
                    ( (((l) & 0xFF000000) >> 24)    |   \
                      (((l) & 0x00FF0000) >> 8)     |   \
                      (((l) & 0x0000FF00) << 8)     |   \
                      (((l) & 0x000000FF) << 24) )

#define NTOH_S(s)                                   \
                    ( (((s) & 0xFF00) >> 8)     |   \
                      (((s) & 0x00FF) << 8) )

#else   //  BIG_ENDIAN
//  we're already big-endian, so we do nothing

#define NTOH_LL(ll)     (ll)
#define NTOH_L(l)       (l)
#define NTOH_S(s)       (s)

#endif  //  BIG_ENDIAN

//  swapping from host back to network order is the same
#define HTON_LL(ll)     NTOH_LL(ll)
#define HTON_L(l)       NTOH_L(l)
#define HTON_S(s)       NTOH_S(s)

//  i 0-based
#define BIT_VALUE(v,b)              ((v & (0x00 | (1 << b))) >> b)
#define BYTE_OFFSET(pb,i)           (& BYTE_VALUE((pb),i))
#define BYTE_VALUE(pb,i)            (((BYTE *) (pb))[i])
#define WORD_VALUE(pb,i)            (* (UNALIGNED WORD *) BYTE_OFFSET((pb),i))
#define DWORD_VALUE(pb,i)           (* (UNALIGNED DWORD *) BYTE_OFFSET((pb),i))
#define ULONGLONG_VALUE(pb,i)       (* (UNALIGNED ULONGLONG *) BYTE_OFFSET((pb),i))

//  ---------------------------------------------------------------------------
//  ===========================================================================
//      beginning of transport - related parsing
//  ===========================================================================
//  ---------------------------------------------------------------------------

//  all macros have either "_BIT" or "_VALUE" appended.  These indicate whether
//  the value of interest is a bit or a value.  A bit value is always shifted
//  so 1 means it's on, and 0 means it's off.  A value consists of a > 1 bits
//  of data.

//  all macros assume pb points to the beginning of the transport packet
//  i.e. points to the sync byte --> * pb = 0x47

//  in order of appearance in header

#define SYNC_BYTE_VALUE(pb)                             (BYTE_VALUE((pb),0))
#define TRANSPORT_ERROR_INDICATOR_BIT(pb)               ((NTOH_S(WORD_VALUE(pb,1)) & 0x00008000) >> 15)
#define PAYLOAD_UNIT_START_INDICATOR_BIT(pb)            ((NTOH_S(WORD_VALUE(pb,1)) & 0x00004000) >> 14)
#define TRANSPORT_PRIORITY_BIT(pb)                      ((NTOH_S(WORD_VALUE(pb,1)) & 0x00002000) >> 13)
#define PID_VALUE(pb)                                   (NTOH_S(WORD_VALUE(pb,1)) & 0x00001fff)
#define TRANSPORT_SCRAMBLING_CONTROL_VALUE(pb)          ((BYTE_VALUE((pb),3) & 0x000000c0) >> 6)
#define ADAPTATION_FIELD_CONTROL_VALUE(pb)              ((BYTE_VALUE((pb),3) & 0x00000030) >> 4)
#define CONTINUITY_COUNTER_VALUE(pb)                    (BYTE_VALUE((pb),3) & 0x0000000f)

//  --------------------------------------------------------------------------------------
//  adaptation field parsing macros
//  pb points to adaptation field

#define ADAPTATION_FIELD_LENGTH_VALUE(pb)               (BYTE_VALUE((pb),0))
#define DISCONTINUITY_INDICATOR_BIT(pb)                 ((BYTE_VALUE((pb),1) & 0x00000080) >> 7)
#define RANDOM_ACCESS_INDICATOR_BIT(pb)                 ((BYTE_VALUE((pb),1) & 0x00000040) >> 6)
#define ELEMENTARY_STREAM_PRIORITY_INDICATOR_BIT(pb)    ((BYTE_VALUE((pb),1) & 0x00000020) >> 5)
#define PCR_FLAG_BIT(pb)                                ((BYTE_VALUE((pb),1) & 0x00000010) >> 4)
#define OPCR_FLAG_BIT(pb)                               ((BYTE_VALUE((pb),1) & 0x00000008) >> 3)
#define SPLICING_POINT_FLAG_BIT(pb)                     ((BYTE_VALUE((pb),1) & 0x00000004) >> 2)
#define TRANSPORT_PRIVATE_DATA_FLAG_BIT(pb)             ((BYTE_VALUE((pb),1) & 0x00000002) >> 1)
#define ADAPTATION_FIELD_EXTENSION_FLAG_BIT(pb)         (BYTE_VALUE((pb),1) & 0x00000001)

//  pb points to PCR_BASE block
#define PCR_BASE_VALUE(pb)                              (NTOH_LL(ULONGLONG_VALUE(pb,0)) >> 31)
#define PCR_EXT_VALUE(pb)                               ((DWORD) (NTOH_LL(ULONGLONG_VALUE(pb,0)) & 0x00000000000001ff))

//  pb points to OPCR_BASE block
#define OPCR_BASE_VALUE(pb)                             PCR_BASE_VALUE(pb)
#define OPCR_EXT_VALUE(pb)                              PCR_EXT_VALUE(pb)

//  pb points to splice_countdown block
#define SPLICE_COUNTDOWN_VALUE(pb)                      BYTE_VALUE(pb,0)

//  pb points to transport_private_data_length
#define TRANSPORT_PRIVATE_DATA_LENGTH_VALUE(pb)         BYTE_VALUE(pb,0)

//  pb points to adaptation_field_extension_length
#define ADAPTATION_FIELD_EXTENSION_LENGTH_VALUE(pb)     BYTE_VALUE(pb,0)

//  XXXX: we ignore the field extensions for now

//  ---------------------------------------------------------------------------
//  PSI generic information (except for pure private sections)
//  pb points to first byte of section (TS packet payload +
//  pointer_field offset)

#define PSI_TABLE_ID_VALUE(pb)                          (BYTE_VALUE((pb),0))
#define PSI_SECTION_SYNTAX_INDICATOR_BIT(pb)            ((BYTE_VALUE((pb),1) & 0x80) >> 7)
#define PSI_SECTION_LENGTH_VALUE(pb)                    (NTOH_S(WORD_VALUE(pb,1)) & 0x0fff)
#define PSI_VERSION_NUMBER_VALUE(pb)                    ((BYTE_VALUE((pb),5) >> 1) & 0x1f)
#define PSI_CURRENT_NEXT_INDICATOR_BIT(pb)              (BYTE_VALUE((pb),5) & 0x01)
#define PSI_SECTION_NUMBER_VALUE(pb)                    (BYTE_VALUE((pb),6))
#define PSI_LAST_SECTION_NUMBER_VALUE(pb)               (BYTE_VALUE((pb),7))

//  ---------------------------------------------------------------------------
//  Program Association Table (PAT) macros
//  pb points to the first byte of section

#define PAT_TABLE_ID_VALUE(pb)                          PSI_TABLE_ID_VALUE(pb)
#define PAT_SECTION_SYNTAX_INDICATOR_BIT(pb)            PSI_SECTION_SYNTAX_INDICATOR_BIT(pb)
#define PAT_SECTION_LENGTH_VALUE(pb)                    PSI_SECTION_LENGTH_VALUE(pb)
#define PAT_TRANSPORT_STREAM_ID_VALUE(pb)               (NTOH_S(WORD_VALUE(pb,3)))
#define PAT_VERSION_NUMBER_VALUE(pb)                    PSI_VERSION_NUMBER_VALUE(pb)
#define PAT_CURRENT_NEXT_INDICATOR_BIT(pb)              PSI_CURRENT_NEXT_INDICATOR_BIT(pb)
#define PAT_SECTION_NUMBER_VALUE(pb)                    PSI_SECTION_NUMBER_VALUE(pb)
#define PAT_LAST_SECTION_NUMBER_VALUE(pb)               PSI_LAST_SECTION_NUMBER_VALUE(pb)

//  PAT program descriptor parsing macros

//  pointer to the nth program descriptor in the section; n is 0-based; does
//  NO range checking on the validity of n; offsets past the header (8 bytes)
//  then into the program descriptors
#define PAT_PROGRAM_DESCRIPTOR(pbPAT,n)                 ((((BYTE *) (pbPAT)) + 8) + ((n) * 4))

//  nth program descriptor field extractions
#define PAT_PROGRAM_DESCRIPTOR_PROGRAM_NUMBER_VALUE(pbPAT,n)    (NTOH_S (WORD_VALUE(PAT_PROGRAM_DESCRIPTOR(pbPAT,n),0)))
#define PAT_PROGRAM_DESCRIPTOR_PID_VALUE(pbPAT,n)               (NTOH_S (WORD_VALUE(PAT_PROGRAM_DESCRIPTOR(pbPAT,n),2)) & 0x1fff)
#define PAT_PROGRAM_DESCRIPTOR_IS_PROGRAM(pbPAT,n)              (PAT_PROGRAM_DESCRIPTOR_PROGRAM_NUMBER_VALUE(pbPAT,n) != 0x0000)
#define PAT_PROGRAM_DESCRIPTOR_IS_NETWORK(pbPAT,n)              (PAT_PROGRAM_DESCRIPTOR_IS_PROGRAM(pbPAT,n) == FALSE)
#define PAT_PROGRAM_DESCRIPTOR_PROGRAM_PID_VALUE(pbPAT,n)       PAT_PROGRAM_DESCRIPTOR_PID_VALUE(pbPAT,n)
#define PAT_PROGRAM_DESCRIPTOR_NETWORK_PID_VALUE(pbPAT,n)       PAT_PROGRAM_DESCRIPTOR_PID_VALUE(pbPAT,n)

//  ---------------------------------------------------------------------------
//  Program Map Table (PMT) macros

//  pb points to the first byte of section
#define PMT_TABLE_ID_VALUE(pb)                          BYTE_VALUE(pb,0)
#define PMT_SECTION_SYNTAX_INDICATOR_BIT(pb)            PSI_SECTION_SYNTAX_INDICATOR_BIT(pb)
#define PMT_SECTION_LENGTH_VALUE(pb)                    PSI_SECTION_LENGTH_VALUE(pb)
#define PMT_PROGRAM_NUMBER_VALUE(pb)                    NTOH_S(WORD_VALUE(pb,3))
#define PMT_VERSION_NUMBER_VALUE(pb)                    PSI_VERSION_NUMBER_VALUE(pb)
#define PMT_CURRENT_NEXT_INDICATOR_BIT(pb)              PSI_CURRENT_NEXT_INDICATOR_BIT(pb)
#define PMT_SECTION_NUMBER(pb)                          PSI_SECTION_NUMBER_VALUE(pb)
#define PMT_LAST_SECTION_NUMBER(pb)                     PSI_LAST_SECTION_NUMBER_VALUE(pb)
#define PMT_PCR_PID_VALUE(pb)                           (NTOH_S(WORD_VALUE(pb,8)) & 0x1fff)
#define PMT_PROGRAM_INFO_LENGTH_VALUE(pb)               (NTOH_S(WORD_VALUE(pb,10)) & 0x0fff)

//  pb points to the stream record block (stream_type, etc...)
#define PMT_STREAM_RECORD_STREAM_TYPE_VALUE(pb)         BYTE_VALUE(pb,0)
#define PMT_STREAM_RECORD_ELEMENTARY_PID(pb)            (NTOH_S(WORD_VALUE(pb,1)) & 0x1fff)
#define PMT_STREAM_RECORD_ES_INFO_LENGTH(pb)            (NTOH_S(WORD_VALUE(pb,3)) & 0x0fff)

//  ---------------------------------------------------------------------------
//  Private section
//  pb points to the TS packet payload ! (first byte of section)

#define PRIVATE_SECTION_SYNTAX_INDICATOR_BIT(pb)        PSI_SECTION_SYNTAX_INDICATOR_BIT(pb)

//  ===========================================================================

//  pb points to prefix i.e. pb = { 0x00, 0x00, 0x01, ....}
#define START_CODE_PREFIX_VALUE(pb)                     (NTOH_L(DWORD_VALUE(pb,0)) >> 8)
#define START_CODE_VALUE(pb)                            (BYTE_VALUE(pb,3))
#define PACKET_LENGTH_VALUE(pb)                         (NTOH_S(WORD_VALUE(pb,4)))

//  ----------------------------------------------------------------------------
//  pack header parsing

//  pb points to pack_start_code

#define PACK_START_CODE_VALUE(pb)                       NTOH_L(DWORD_VALUE(pb,0))
#define PACK_HEADER_SCR_BASE(pb)                        (((NTOH_LL(ULONGLONG_VALUE(pb,1)) & 0x000000000003FFF8) >> 3) | \
                                                         ((NTOH_LL(ULONGLONG_VALUE(pb,1)) & 0x00000003FFF80000) >> 4) | \
                                                         ((NTOH_LL(ULONGLONG_VALUE(pb,1)) & 0x0000003800000000) >> 5))
#define PACK_HEADER_SCR_EXT(pb)                         ((NTOH_LL(ULONGLONG_VALUE(pb,2)) & 0x00000000000003FE) >> 1)
#define PACK_PROGRAM_MUX_RATE(pb)                       (((NTOH_L(DWORD_VALUE(pb,10)) >> 8) & 0xFFFFFC0) >> 2)
#define PACK_STUFFING_LENGTH(pb)                        (BYTE_VALUE(pb,13) & 0x03)

//  ===========================================================================

//  ---------------------------------------------------------------------------
//  ===========================================================================
//      PES - related parsing
//  ===========================================================================
//  ---------------------------------------------------------------------------

#define PES_PACKET_START_CODE_PREFIX_VALUE(pb)          START_CODE_PREFIX_VALUE(pb)
#define PES_STREAM_ID_VALUE(pb)                         START_CODE_VALUE(pb)
#define PES_PACKET_LENGTH_VALUE(pb)                     PACKET_LENGTH_VALUE(pb)

//  pb points to TIER_1 header
#define PES_SCRAMBLING_CONTROL_VALUE(pb)                ((BYTE_VALUE(pb,0) & 0x30) >> 4)
#define PES_PRIORITY_BIT(pb)                            BIT_VALUE(BYTE_VALUE(pb,0),3)
#define PES_DATA_ALIGNMENT_INDICATOR_BIT(pb)            BIT_VALUE(BYTE_VALUE(pb,0),2)
#define PES_COPYRIGHT_BIT(pb)                           BIT_VALUE(BYTE_VALUE(pb,0),1)
#define PES_ORIGINAL_OR_COPY_BIT(pb)                    BIT_VALUE(BYTE_VALUE(pb,0),0)
#define PES_PTS_DTS_FLAGS_VALUE(pb)                     ((BYTE_VALUE(pb,1) & 0xc0) >> 6)
#define PES_ESCR_FLAG_BIT(pb)                           BIT_VALUE(BYTE_VALUE(pb,1),5)
#define PES_ES_RATE_FLAG_BIT(pb)                        BIT_VALUE(BYTE_VALUE(pb,1),4)
#define PES_DSM_TRICK_MODE_FLAG_BIT(pb)                 BIT_VALUE(BYTE_VALUE(pb,1),3)
#define PES_ADDITIONAL_COPY_INFO_FLAG_BIT(pb)           BIT_VALUE(BYTE_VALUE(pb,1),2)
#define PES_CRC_FLAG_BIT(pb)                            BIT_VALUE(BYTE_VALUE(pb,1),1)
#define PES_EXTENSION_FLAG_BIT(pb)                      BIT_VALUE(BYTE_VALUE(pb,1),0)
#define PES_PES_HEADER_DATA_LENGTH_VALUE(pb)            BYTE_VALUE(pb,2)

//  pb points to the PTS field
#define PES_PTS_VALUE(pb)                               ((((NTOH_LL(ULONGLONG_VALUE(pb,0)) >> 24) & 0x000000000000FFFE) >> 1) |   \
                                                         (((NTOH_LL(ULONGLONG_VALUE(pb,0)) >> 24) & 0x00000000FFFE0000) >> 2) |   \
                                                         (((NTOH_LL(ULONGLONG_VALUE(pb,0)) >> 24) & 0x0000000E00000000) >> 3))
#define PES_DTS_VALUE(pb)                               PES_PTS_VALUE(pb)

//  ============================================================================
//      macrovision parsing
//          pb points to PES header
//  ============================================================================

#define PES_MV_SUBSTREAM_ID(pb)                         BYTE_VALUE(pb,PES_COMMON_HEADER_FIELD_LEN)
#define PES_MV_BITS(pb)                                 ((NTOH_S (WORD_VALUE(pb,MACROVISION_WORD_OFFSET))) >> 14)


//  general MPEG-2 - related macros follow

//  ---------------------------------------------------------------------------
//  general macros

//  TRUE/FALSE if the PSI header is valid; can be used for PAT, PMT, CAT,
//  non pure-private sections; checks the following items:
//      1. section_length should not exceed maximum allowed
//      2. section_length should include at least remaining header
//      3. last_section_number should be >= section_number
//      4. syntax_indicator bit should be '1'
//      5. section_length should have first 2 bits == '00'
#define VALID_PSI_HEADER(pbSection)                                                                 \
            ( (PSI_SECTION_LENGTH_VALUE(pbSection) <= 0x3fd)                                    &&  \
              (PSI_SECTION_LENGTH_VALUE(pbSection) >= 5)                                        &&  \
              (PSI_LAST_SECTION_NUMBER_VALUE(pbSection) >= PSI_SECTION_NUMBER_VALUE(pbSection)) &&  \
              (PSI_SECTION_SYNTAX_INDICATOR_BIT(pbSection) == TRUE)                             &&  \
              (((PSI_SECTION_LENGTH_VALUE(pbSection) & 0xc000)) == 0x0000) )

//  checks that he PAT section given is valid; checks the following items:
//      1. valid PSI header
//      2. even number of 4 byte program descriptors (section_length - 5 bytes
//          remaining in header - 4 byte CRC_32)
//      3. table_id is 0x00 (see table 2-26)
#define VALID_PAT_SECTION(pbPAT)                                            \
            ( VALID_PSI_HEADER(pbPAT)                                   &&  \
              (((PAT_SECTION_LENGTH_VALUE(pbPAT) - 4 - 5) % 4) == 0)    &&  \
              (PAT_TABLE_ID_VALUE(pbPAT) == PAT_TABLE_ID) )

//  returns the number of programs described in the passed PAT section
#define NUMBER_PROGRAMS_IN_PAT_SECTION(pbPAT)   ((DWORD) ((PAT_SECTION_LENGTH_VALUE(pbPAT) - 5 - 4) / 4))

//  checks that he PAT section given is value; checks the following items:
//      1. valid PSI header
//      2. section_length, program_info_length are valid wrt each other; checked as
//          follows: section_length >= program_info_length + 9 + 4, where
//              9 is the number of bytes that FOLLOW section_length and include program_info_length
//              4 is the size of the CRC_32 value at the end of the section
//      3. table_id is PMT_TABLE_ID
#define VALID_PMT_SECTION(pbPMT)                                                                    \
            ( VALID_PSI_HEADER(pbPMT)                                                           &&  \
              (PMT_SECTION_LENGTH_VALUE(pbPMT) >= PMT_PROGRAM_INFO_LENGTH_VALUE(pbPMT) + 9 + 4) &&  \
              (PMT_TABLE_ID_VALUE(pbPMT) == PMT_TABLE_ID) )

//  confirms that the PID value does not have bits higher than the first 13
//  are set
#define VALID_PID(pid)                          (((pid) & 0xffffe000) == 0x00000000)

//  confirms the continuity_counter holds a valid value
#define VALID_CONTINUITY_COUNTER(counter)       (((counter) & 0xffffff00) == 0x00000000)

//  see page 20 of the MPEG-2 systems spec
#define EXPECT_CONTINUITY_COUNTER_INCREMENT(pid, adaptation_field_control)                      \
                                                 ((pid) != TS_NULL_PID &&                       \
                                                  (adaptation_field_control) != 0x00000000 &&   \
                                                  (adaptation_field_control) != 0x00000002)

//  increments and takes care of the wrapping case
#define INCREMENTED_CONTINUITY_COUNTER_VALUE(c) ((BYTE) (((c) + 1) & 0x0000000f))

//  adaptation field macros follow --

//  pp. 20 of the MPEG-2 systems spec
//      adaptation_field_control == '11' ==> adaptation_field_length in [0, 182]
//      adaptation_field_control == '10' ==> adaptation_field_length == 183
#define VALID_ADAPTATION_FIELD(adaptation_field_control, adaptation_field_length)    \
                                                (((adaptation_field_control) == 0x3 && (adaptation_field_length) <= 182) ||   \
                                                 ((adaptation_field_control) == 0x2 && (adaptation_field_length) == 183))

//  pp. 41 of the MPEG-2 systems spec
//      make sure that the pointer_field field that precedes a PSI section is valid
//      i.e. doesn't index us off the end of the packet; we check validity by making
//      sure that it falls in the range [0, TransportPayloadLength - 1); 0 means
//      the section follows immediately; if it's > 0, it must be
//      < TransportPayloadLength - 1 because TransportPayloadLength - 1 is the payload
//      that remains AFTER pointer_field, and we need at least 1 byte of section data
#define VALID_POINTER_FIELD(pointer_field, TransportPayloadLength)      ((pointer_field) < (TransportPayloadLength) - 1)

#define IS_AV_STREAM(stream_type)               (((stream_type) == 0x00000001) ||   \
                                                 ((stream_type) == 0x00000002) ||   \
                                                 ((stream_type) == 0x00000003) ||   \
                                                 ((stream_type) == 0x00000004))

//  ---------------------------------------------------------------------------
//  PES packet - related

//  TRUE/FALSE if the stream is a padding stream
#define IS_PADDING_STREAM(stream_id)            ((stream_id) == STREAM_ID_PADDING_STREAM)

//  TRUE/FALSE if stream is tier_1 (see first conditional in table 2-17,
//  MPEG2 systems specification, p. 30)
#define IS_TIER1_STREAM(stream_id)      (((stream_id) != STREAM_ID_PROGRAM_STREAM_MAP)          &&  \
                                         ((stream_id) != STREAM_ID_PADDING_STREAM)              &&  \
                                         ((stream_id) != STREAM_ID_PRIVATE_STREAM_2)            &&  \
                                         ((stream_id) != STREAM_ID_ECM)                         &&  \
                                         ((stream_id) != STREAM_ID_EMM)                         &&  \
                                         ((stream_id) != STREAM_ID_PROGRAM_STREAM_DIRECTORY)    &&  \
                                         ((stream_id) != STREAM_ID_DSMCC_STREAM)                &&  \
                                         ((stream_id) != STREAM_ID_H_222_1_TYPE_E))



//  TRUE/FALSE if stream is tier_2 (see second conditional in table 2-17,
//  MPEG2 systems specification, p. 32)
#define IS_TIER2_STREAM(stream_id)      (((stream_id) == STREAM_ID_PROGRAM_STREAM_MAP)          ||  \
                                         ((stream_id) == STREAM_ID_PRIVATE_STREAM_2)            ||  \
                                         ((stream_id) == STREAM_ID_ECM)                         ||  \
                                         ((stream_id) == STREAM_ID_EMM)                         ||  \
                                         ((stream_id) == STREAM_ID_PROGRAM_STREAM_DIRECTORY)    ||  \
                                         ((stream_id) == STREAM_ID_DSMCC_STREAM)                ||  \
                                         ((stream_id) == STREAM_ID_H_222_1_TYPE_E))

#define IS_TIER3_STREAM(stream_id)      ((stream_id) == STREAM_ID_PADDING_STREAM)

//  total PES header length (includes usual fields, optional fields,
//  stuffing bytes); add the return value to a byte pointer to PES header
//  to point to the byte that follows
//  ho = PES_header_data_length
#define PES_HEADER_TOTAL_LENGTH(ho)     (PES_TIER1_REQ_HEADER_FIELD_LEN + (ho))

//  returns the total PES packet length, or -1 if pl (PES_packet_length) is 0
#define TOTAL_PES_PACKET_LENGTH(pl)     ((pl) != 0 ? ((pl) + PES_ALWAYS_HEADER_FIELD_LEN) : -1)

//  TRUE/FALSE if packet has video
#define IS_PES_VIDEO(id)                IN_RANGE(id,STREAM_ID_VIDEO_STREAM_MIN,STREAM_ID_VIDEO_STREAM_MAX)

//  TRUE/FALSE if the packet has audio
#define IS_PES_AUDIO(id)                IN_RANGE(id,STREAM_ID_AUDIO_STREAM_MIN,STREAM_ID_AUDIO_STREAM_MAX)

//  ---------------------------------------------------------------------------
//  macrovision-related

//  PES_packet_length defines the number of bytes that *follow* the field
#define PES_PACKET_LENGTH_IS_MIN_VALID_MACROVISION(pbPES)   (PES_PACKET_LENGTH_VALUE(pbPES) >= (1 + 4 + 2))

#define SUBSTREAMID_IS_MACROVISION(pbPES)                   (PES_MV_SUBSTREAM_ID(pbPES) == SUBSTREAM_ID_MACROVISION)
#define IS_VALID_MACROVISION_PACKET(pbPES)                  (PES_PACKET_LENGTH_IS_MIN_VALID_MACROVISION(pbPES) && SUBSTREAMID_IS_MACROVISION(pbPES))

#endif  //  __mp2demux_parsemac_h