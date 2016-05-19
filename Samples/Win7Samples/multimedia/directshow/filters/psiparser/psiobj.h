//------------------------------------------------------------------------------
// File: PsiObjs.h
//
// Desc: DirectShow sample code - PSIParser sample filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _PSI_OBJ_H_
#define _PSI_OBJ_H_

#include "mp2const.h"
#include "parsemac.h"

#include "dbg.h"


/**************************************************************************
  parse PAT and PMT tables
  section, program data store

  struct and classes to process pat and pmt sections received

  *************************************************************************/

// max section length = 1021 bytes
// total bytes in a pat section except program number and PID = 12 bytes
// tatol bytes of program_number and PID = 4 bytes
//
#define MAX_PROGRAM_NUMBER_IN_PAT_SECTION               252  // (1021-12)/4 = 252 

// max section length = 1021 bytes
// total bytes between program_number and program_info_length is 9
// CRC_32 is 4 bytes elementary inforamtion loop is 5 bytes each
//
#define MAX_ELEMENTARY_STREAM_NUMBER_IN_PMT_SECTION     202   //(1021-9-4)/5

#define PMT_BYTE_NUMBER_BEFORE_ES                       12
#define PMT_ES_BYTE_NUMBER_BEFORE_ESDESCRIPTOR          5
#define PAT_SECTION_LENGTH_EXCLUDE_PROGRAM_DESCRIPTOR   9
#define PAT_PROGRAM_DESCRIPTOR_LENGTH                   4

#define PMT_BYTE_NUMBER_BEFORE_PROGRAM_NUMBER           3
#define PMT_CRC_LENGTH                                  4

#define MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM          500  // (arbitrary)

#define PAT_TABLE_ID                                    0x00 // Table 2-26 ISO/IEC 13818-1:1996(E)
#define PMT_TABLE_ID                                    0x02 // (see above table)



// inside PAT section
typedef struct Program_Descriptor
{
        DWORD program_number;
        DWORD network_or_program_map_PID;

} PROGRAM_DESCRIPTOR;


// inside PMT section
typedef struct Elementary_Stream_Info
{
    DWORD stream_type;
    DWORD elementary_PID;
    DWORD ES_info_length;

} ELEMENTARY_STREAM_INFO;


// PSI section: common fields in PAT and PMT
struct MPEG2_PSI_SECTION
{
    DWORD table_id;
    BOOL  section_syntax_indicator;
    DWORD section_length;
    DWORD version_number;
    BOOL  current_next_indicator;
    DWORD section_number;
    DWORD last_section_number;

    LONGLONG CRC_32;        // ignore for now
};


// PAT section store object
struct MPEG2_PAT_SECTION :
    public MPEG2_PSI_SECTION
{

    DWORD transport_stream_id;

    int number_of_programs; // derived
    PROGRAM_DESCRIPTOR program_descriptor[MAX_PROGRAM_NUMBER_IN_PAT_SECTION];

    _inline BOOL Parse(IN BYTE * pbBuf // points to first byte of psi section
                        )
    {
        // common psi fields
        table_id                    =   PAT_TABLE_ID_VALUE                  (pbBuf) ;
        section_syntax_indicator    =   PAT_SECTION_SYNTAX_INDICATOR_BIT    (pbBuf) ;
        section_length              =   PAT_SECTION_LENGTH_VALUE            (pbBuf) ;
        version_number              =   PAT_VERSION_NUMBER_VALUE            (pbBuf) ;
        current_next_indicator      =   PAT_CURRENT_NEXT_INDICATOR_BIT      (pbBuf) ;
        section_number              =   PAT_SECTION_NUMBER_VALUE            (pbBuf) ;
        last_section_number         =   PAT_LAST_SECTION_NUMBER_VALUE       (pbBuf) ;
        Dump1(TEXT("pat version # = %d\n"), version_number);

        // pat specific fields
        transport_stream_id         =   PAT_TRANSPORT_STREAM_ID_VALUE       (pbBuf) ;
        Dump1(TEXT("transport_stream_id = %d\n"), transport_stream_id);
        number_of_programs          =   (section_length - PAT_SECTION_LENGTH_EXCLUDE_PROGRAM_DESCRIPTOR)/PAT_PROGRAM_DESCRIPTOR_LENGTH; 

        // parse PAT program descriptor
        for(int i=0; i< number_of_programs ; i++)
        {
            program_descriptor[i].program_number             = PAT_PROGRAM_DESCRIPTOR_PROGRAM_NUMBER_VALUE   (pbBuf,i);
            program_descriptor[i].network_or_program_map_PID = PAT_PROGRAM_DESCRIPTOR_PID_VALUE              (pbBuf,i);
            Dump1(TEXT("    program index               = %d\n"), i);
            Dump1(TEXT("    program_number              = %d\n"), program_descriptor[i].program_number);
            Dump1(TEXT("    network_or_program_map_PID  = %d\n\n"), program_descriptor[i].network_or_program_map_PID);
        } 

        return TRUE;
    }

    _inline BOOL Copy(IN MPEG2_PAT_SECTION* pSource)
    {       
        table_id                    = pSource->table_id;
        section_syntax_indicator    = pSource->section_syntax_indicator;
        section_length              = pSource->section_length;
        version_number              = pSource->version_number;
        current_next_indicator      = pSource->current_next_indicator;
        section_number              = pSource->section_number;
        last_section_number         = pSource->last_section_number;

        transport_stream_id         = pSource->transport_stream_id;
        number_of_programs          = pSource->number_of_programs;

        for(int i=0; i< number_of_programs ; i++)
        {
            program_descriptor[i].program_number             = pSource->program_descriptor[i].program_number;
            program_descriptor[i].network_or_program_map_PID = pSource->program_descriptor[i].network_or_program_map_PID;
        } 

        return TRUE;
    }

};


struct MPEG2_PMT_SECTION :
    public MPEG2_PSI_SECTION
{
    DWORD program_number;
    DWORD PCR_PID;
    DWORD program_info_length;

    DWORD number_of_elementary_streams; //derived
    ELEMENTARY_STREAM_INFO elementary_stream_info[MAX_ELEMENTARY_STREAM_NUMBER_IN_PMT_SECTION];

    _inline BOOL Parse(IN BYTE * pbBuf) // points to first byte of psi section)
    {
        // common psi fields
        table_id                    =   PMT_TABLE_ID_VALUE                  (pbBuf) ;
        section_syntax_indicator    =   PMT_SECTION_SYNTAX_INDICATOR_BIT    (pbBuf) ;
        section_length              =   PMT_SECTION_LENGTH_VALUE            (pbBuf) ;
        version_number              =   PMT_VERSION_NUMBER_VALUE            (pbBuf) ;
        current_next_indicator      =   PMT_CURRENT_NEXT_INDICATOR_BIT      (pbBuf) ;
        section_number              =   PMT_SECTION_NUMBER                  (pbBuf) ;
        last_section_number         =   PMT_LAST_SECTION_NUMBER             (pbBuf) ;

        // pmt specific fields
        program_number              =   PMT_PROGRAM_NUMBER_VALUE            (pbBuf) ;
        PCR_PID                     =   PMT_PCR_PID_VALUE                   (pbBuf) ;
        program_info_length         =   PMT_PROGRAM_INFO_LENGTH_VALUE       (pbBuf) ;

        BYTE* pStartEs  = pbBuf + PMT_BYTE_NUMBER_BEFORE_ES + program_info_length;//points to the stream_type of the 1st elementary stream
        BYTE* pEndEs    = pbBuf + section_length + PMT_BYTE_NUMBER_BEFORE_PROGRAM_NUMBER - PMT_CRC_LENGTH   ;   //points to the byte before CRS_323, 4
        BYTE* pTmp ;

        unsigned short EsNumber = 0;
        int offset = 0;

        for(pTmp = pStartEs; pTmp < pEndEs; pTmp += offset)
        {
            offset = PMT_ES_BYTE_NUMBER_BEFORE_ESDESCRIPTOR + PMT_STREAM_RECORD_ES_INFO_LENGTH      (pTmp); //5+ES_info_length
            elementary_stream_info[EsNumber].stream_type    = PMT_STREAM_RECORD_STREAM_TYPE_VALUE   (pTmp);
            Dump1(TEXT("**stream_type = %#x\n"), elementary_stream_info[EsNumber].stream_type);

            elementary_stream_info[EsNumber].elementary_PID = PMT_STREAM_RECORD_ELEMENTARY_PID      (pTmp);
            Dump1(TEXT("**elementary_PID = %#x\n"), elementary_stream_info[EsNumber].elementary_PID);

            elementary_stream_info[EsNumber].ES_info_length = PMT_STREAM_RECORD_ES_INFO_LENGTH      (pTmp);
        
            EsNumber++;
        }

        number_of_elementary_streams = EsNumber;

        return TRUE;
    }

    _inline BOOL Copy(IN MPEG2_PMT_SECTION* pSource)
    {
        
        table_id                    = pSource->table_id;
        section_syntax_indicator    = pSource->section_syntax_indicator;
        section_length              = pSource->section_length;
        version_number              = pSource->version_number;
        current_next_indicator      = pSource->current_next_indicator;
        section_number              = pSource->section_number;
        last_section_number         = pSource->last_section_number;

        program_number              = pSource->program_number;
        PCR_PID                     = pSource->PCR_PID;
        program_info_length         = pSource->program_info_length;

        number_of_elementary_streams= pSource->number_of_elementary_streams;

        for(int i=0; i< (int)number_of_elementary_streams ; i++)
        {
            elementary_stream_info[i].stream_type    = pSource->elementary_stream_info[i].stream_type;
            elementary_stream_info[i].elementary_PID = pSource->elementary_stream_info[i].elementary_PID;
            elementary_stream_info[i].ES_info_length = pSource->elementary_stream_info[i].ES_info_length;
        }

        return TRUE;
    }


};


// *********************program store object
typedef struct Mpeg2_Program
{
    DWORD               program_number;
    DWORD               network_or_program_map_PID;
    DWORD               number_of_ESs;

    MPEG2_PAT_SECTION   mpeg2_pat_section;
    MPEG2_PMT_SECTION   mpeg2_pmt_section;
}MPEG2_PROGRAM;


struct CPrograms
{
    MPEG2_PROGRAM*  m_programs[MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM];
    int             m_ProgramCount;

    _inline void init_programs()
    {
        m_ProgramCount = 0;
    }

    // is the program existing in the array already?
    _inline BOOL find_program(const DWORD dwProgramNumber, int *pIndex)
    {
        for (int i = 0; i<m_ProgramCount; i++){
            if(m_programs[i]->program_number == dwProgramNumber){
                *pIndex = i;
                return TRUE;
            }
        }
        return FALSE;
    }

    // add a program from a PAT section, if existing in the array, update it
    _inline BOOL add_program_from_pat(MPEG2_PAT_SECTION* pmpeg2_pat_section, int SectionIndex)
    {
        MPEG2_PROGRAM *tmp;

        // if already exit, update it:
        int index;
        if(find_program(pmpeg2_pat_section->program_descriptor[SectionIndex].program_number, &index)){
            return update_program_from_pat(index, pmpeg2_pat_section, SectionIndex);
        }

        // no more room, failed
        if(m_ProgramCount + 1 > MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM)
            return FALSE;

        // copy program from pat section to the allocated space:
        tmp = (MPEG2_PROGRAM *)HeapAlloc( GetProcessHeap(),
                                          HEAP_ZERO_MEMORY,
                                          sizeof( MPEG2_PROGRAM )  );
        if( tmp == NULL) {
            return FALSE;
        }

        copy_program_from_pat(pmpeg2_pat_section, SectionIndex, tmp);  

        // put new program on array
        m_programs[m_ProgramCount] = tmp;
        m_ProgramCount++;

        return TRUE;
    }

    // Update a program from a PMT section. The program should have been stored in the array
    // when processing pat. The sequence is: 
    //     0. we initially receive pat only by mapping pat pid to demux output pin
    //     1. a program's pmt pid was read from pat
    //     2. that program's pmt pid was then mapped to demux output pin
    //     3. PSI parser filter then began to receive and process pmt
    _inline BOOL update_program_from_pmt(MPEG2_PMT_SECTION* pmpeg2_pmt_section)
    {
        // if already exit, update it:
        int index;
        if(find_program(pmpeg2_pmt_section->program_number, &index)){
            m_programs[index]->number_of_ESs = pmpeg2_pmt_section->number_of_elementary_streams;

            m_programs[index]->mpeg2_pmt_section.Copy(pmpeg2_pmt_section);

            return TRUE;
        }   
        return FALSE;
    }

    _inline BOOL copy_program_from_pat(MPEG2_PAT_SECTION * pSection, 
                                       int SectionIndex, MPEG2_PROGRAM *pDest)
    {
        pDest->program_number               = pSection->program_descriptor[SectionIndex].program_number;
        pDest->network_or_program_map_PID   = pSection->program_descriptor[SectionIndex].network_or_program_map_PID;
        pDest->mpeg2_pat_section.Copy(pSection);

        return TRUE;
    }

    _inline BOOL update_program_from_pat(int ProgramIndex, 
                                         MPEG2_PAT_SECTION* pmpeg2_pat_section, 
                                         int SectionIndex)
    {
        return copy_program_from_pat(pmpeg2_pat_section, SectionIndex, m_programs[ProgramIndex]);
    }

    _inline BOOL free_programs()
    {
        for(int i= 0; i<m_ProgramCount; i++){
            if(!HeapFree(GetProcessHeap(),  HEAP_ZERO_MEMORY, (LPVOID) m_programs[i] ))
                return FALSE;
        }

        return TRUE;
    }

};


#endif