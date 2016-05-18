//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module SCALAR.H | SCALAR header file for test modules.
//
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 09-06-95 | Microsoft | Updated
//

#ifndef _SCALAR_H_
#define _SCALAR_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

#define MAX_BUF		2000

#define NO_PREPARE	0
#define PREPARE		1

#define IDS_USER	0

#define idsFnLeft                                         (IDS_USER+0)
#define idsFnConcat                                       (IDS_USER+1)
#define idsFnInsert                                       (IDS_USER+2)
#define idsFnLtrim                                        (IDS_USER+3)
#define idsFnLength                                       (IDS_USER+4)
#define idsFnLocate                                       (IDS_USER+5)
#define idsFnLcase                                        (IDS_USER+6)
#define idsFnRepeat                                       (IDS_USER+7)
#define idsFnReplace                                      (IDS_USER+8)
#define idsFnRight                                        (IDS_USER+9)
#define idsFnRtrim                                        (IDS_USER+10)
#define idsFnUcase                                        (IDS_USER+11)
#define idsFnAcos                                         (IDS_USER+12)
#define idsFnAsin                                         (IDS_USER+13)
#define idsFnAtan                                         (IDS_USER+14)
#define idsFnAtan2                                        (IDS_USER+15)
#define idsFnCeiling                                      (IDS_USER+16)
#define idsFnCos                                          (IDS_USER+17)
#define idsFnCot                                          (IDS_USER+18)
#define idsFnExp                                          (IDS_USER+19)
#define idsFnFloor                                        (IDS_USER+20)
#define idsFnLog                                          (IDS_USER+21)
#define idsFnMod                                          (IDS_USER+22)
#define idsFnPi                                           (IDS_USER+23)
#define idsFnRand                                         (IDS_USER+24)
#define idsFnSign                                         (IDS_USER+25)
#define idsFnSin                                          (IDS_USER+26)
#define idsFnSqrt                                         (IDS_USER+27)
#define idsFnTan                                          (IDS_USER+28)
#define idsFnDayofweek                                    (IDS_USER+29)
#define idsFnDayofyear                                    (IDS_USER+30)
#define idsFnMonth                                        (IDS_USER+31)
#define idsFnQuarter                                      (IDS_USER+32)
#define idsFnWeek                                         (IDS_USER+33)
#define idsFnYear                                         (IDS_USER+34)
#define idsFnCurtime                                      (IDS_USER+35)
#define idsFnHour                                         (IDS_USER+36)
#define idsFnMinute                                       (IDS_USER+37)
#define idsFnSecond                                       (IDS_USER+38)
#define idsFnDatabase                                     (IDS_USER+39)
#define idsFnIfnull1                                      (IDS_USER+40)
#define idsFnIfnull2                                      (IDS_USER+41)
#define idsConvertNotSupported                            (IDS_USER+42)
#define idsTestConvertFunctions                           (IDS_USER+43)
#define idsProcedureNotSupported                          (IDS_USER+44)
#define idsTestProcedures                                 (IDS_USER+45)
#define idsTestOuterJoins                                 (IDS_USER+46)
#define idsFnDiff                                         (IDS_USER+47)
#define idsFnSoundex                                      (IDS_USER+48)
#define idsFnSpace                                        (IDS_USER+49)
#define idsFnDegrees                                      (IDS_USER+50)
#define idsFnLog10                                        (IDS_USER+51)
#define idsFnPower                                        (IDS_USER+52)
#define idsFnRadians                                      (IDS_USER+53)
#define idsFnRound                                        (IDS_USER+54)
#define idsFnTruncate                                     (IDS_USER+55)
#define idsFnDayname                                      (IDS_USER+56)
#define idsFnMonthname                                    (IDS_USER+57)
#define idsFnTimestampadd                                 (IDS_USER+58)
#define idsFnTimestampdiff                                (IDS_USER+59)
#define idsFnTimestamp                                    (IDS_USER+60)
#define idsFnDate                                         (IDS_USER+61)
#define idsFnTime                                         (IDS_USER+62)

#define idsSelectProcText                                 (IDS_USER+80)
#define idsProcCall                                       (IDS_USER+81)
#define idsDropProc                                       (IDS_USER+82)
#define idsLongProcOnly                                   (IDS_USER+83)

#define idsInsertLikeTbl                                  (IDS_USER+84)
#define idsSelectLikeShort                                (IDS_USER+85)
#define idsTestLikePreds                                  (IDS_USER+86)

#define idsLongLJoin                                      (IDS_USER+87)
#define idsShortLJoin                                     (IDS_USER+88)
#define idsLongRJoin                                      (IDS_USER+89)
#define idsShortRJoin                                     (IDS_USER+90)
#define idsNestedLongJoin                                 (IDS_USER+91)
#define idsNestedShortJoin                                (IDS_USER+92)
#define idsTestLeftOJSyntax                               (IDS_USER+93)
#define idsTestRightOJSyntax                              (IDS_USER+94)
#define idsTestNestedOJSyntax                             (IDS_USER+95)
#define idsInOutShortOJSyntax                             (IDS_USER+96)
#define idsInOutLongOJSyntax                              (IDS_USER+97)
#define idsNoLikePred                                     (IDS_USER+98)
#define idsJetDropProc                                    (IDS_USER+99)
#define idsProcCallQ                                      (IDS_USER+100)
#define idsProcCallQPP                                    (IDS_USER+101)

#define idsFnConcatNested                                 (IDS_USER+102)
#define idsFnLeftNested                                   (IDS_USER+103)
#define idsFnRightNested                                  (IDS_USER+104)
#define idsFnLocateNested                                 (IDS_USER+105)
#define idsFnSubstringNested                              (IDS_USER+106)
#define idsFnDiffNested                                   (IDS_USER+107)
#define idsFnInsertNested                                 (IDS_USER+108)
#define idsFnRepeatNested                                 (IDS_USER+109)
#define idsFnReplaceNested                                (IDS_USER+110)
#define idsFnMultiInsert                                  (IDS_USER+111)
#define idsFnMultiConcat                                  (IDS_USER+112)
#define idsFnStringAndNumeric                             (IDS_USER+113)
#define idsCallFailed                                     (IDS_USER+114)
#define idsSelectLikeEscape                               (IDS_USER+115)
#define idsSelectLikeNative                               (IDS_USER+116)
#define idsDropORAProc									  (IDS_USER+117)
#define idsSelectORAProcText                              (IDS_USER+118)
#define idsSelect											(IDS_USER+119)		
#define idsCanonShort     									(IDS_USER+120)		
#define	idsFromString 										(IDS_USER+121)	

#define   idsFnAscii               (IDS_USER+122)	
#define   idsFnChar                 (IDS_USER+123)	
#define   idsFnSubstring            (IDS_USER+124)	
#define   idsFnAbs                  (IDS_USER+125)	
#define   idsTestDateTimeFunctions  (IDS_USER+126)	

#define   idsFnNow                  (IDS_USER+127)	
#define   idsFnCurdate              (IDS_USER+128)	
#define   idsFnDayofmonth           (IDS_USER+129)	
#define   idsFnUser                 (IDS_USER+130)	
#define   idsFnCHAR                (IDS_USER+131)	
#define   idsFnVARCHAR             (IDS_USER+132)	
#define   idsFnLONGVARCHAR          (IDS_USER+133)	
#define   idsFnLONGVARBINARY        (IDS_USER+134)	
#define   idsFnDECIMAL              (IDS_USER+135)	
#define   idsFnNUMERIC              (IDS_USER+136)	
#define   idsFnBIT                  (IDS_USER+137)	
#define   idsFnTINYINT              (IDS_USER+138)	
#define   idsFnSMALLINT             (IDS_USER+139)	
#define   idsFnINTEGER              (IDS_USER+140)	
#define   idsFnBIGINT               (IDS_USER+141)	
#define   idsFnREAL                 (IDS_USER+142)	
#define   idsFnFLOAT                (IDS_USER+143)	
#define   idsFnDOUBLE               (IDS_USER+144)	
#define   idsFnBINARY               (IDS_USER+145)	
#define   idsFnVARBINARY            (IDS_USER+146)	
#define   idsFnDATE                 (IDS_USER+147)	
#define   idsFnTIME                 (IDS_USER+148)	
#define   idsFnTIMESTAMP            (IDS_USER+149)	
#define   idsFnWCHAR                (IDS_USER+150)	
#define   idsFnWVARCHAR             (IDS_USER+151)	
#define   idsFnWLONGVARCHAR          (IDS_USER+152)	
#define	  idsFnGUID					(IDS_USER+153)
	

#define	CHECKSTR					1
#define	CHECKINT					2
#define	CHECKFLOAT					3
#define	CHECKISTR					4
#define CHECKTIMESTAMP				5
#define	CHECKREAL					6	



//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
typedef struct tagStringList {
	UWORD                   id; // non-essential, but friendly field 
	WCHAR *					wszItem;
	} StringList;

const StringList pwszStringList[] =
{    
	{idsFnLeft               ,L"fn LEFT('aBCdef',2)"},
	{idsFnConcat             ,L"fn CONCAT('Fire','Truck')"},
	{idsFnInsert             ,L"fn INSERT('FireTruck',5,4,'hose')"},
	{idsFnLtrim              ,L"fn LTRIM('     a BCdef')"},
	{idsFnLength             ,L"fn LENGTH('     a BCdef   ')"},
	{idsFnLocate             ,L"fn LOCATE('st','forest estates',6)"},
	{idsFnLcase              ,L"fn LCASE('fIrEtRucK')"},
	{idsFnRepeat             ,L"fn REPEAT('t q',3)"},
	{idsFnReplace            ,L"fn REPLACE('forest estates','es','truck')"},
	{idsFnRight              ,L"fn RIGHT('forest estates',7)"},
	{idsFnRtrim              ,L"fn RTRIM('  forest estates  ')"},
	{idsFnUcase              ,L"fn UCASE('  forest estates  ')"},
	{idsFnAcos               ,L"fn ACOS(1)"},
	{idsFnAsin               ,L"fn ASIN(-1)"},
	{idsFnAtan               ,L"fn ATAN(2)"},
	{idsFnAtan2              ,L"fn ATAN2(2,1)"},
	{idsFnCeiling            ,L"fn CEILING(1.09)"},
	{idsFnCos                ,L"fn COS(0)"},
	{idsFnCot                ,L"fn COT(.5)"},
	{idsFnExp                ,L"fn EXP(1)"},
	{idsFnFloor              ,L"fn FLOOR(1.09)"},
	{idsFnLog                ,L"fn LOG(10)"},
	{idsFnMod                ,L"fn MOD(10,6)"},
	{idsFnPi                 ,L"fn PI()"},
	{idsFnRand               ,L"fn RAND(10)"},
	{idsFnSign               ,L"fn SIGN(10)"},
	{idsFnSin                ,L"fn SIN(.05)"},
	{idsFnSqrt               ,L"fn SQRT(9)"},
	{idsFnTan                ,L"fn TAN(2)"},
	{idsFnDayofweek          ,L"fn DAYOFWEEK({d '1991-02-26'})"},
	{idsFnDayofyear          ,L"fn DAYOFYEAR({d '1991-02-26'})"},
	{idsFnMonth              ,L"fn MONTH({d '1991-02-26'})"},
	{idsFnQuarter            ,L"fn QUARTER({d '1991-02-26'})"},
	{idsFnWeek               ,L"fn WEEK({d '1991-02-26'})"},
	{idsFnYear               ,L"fn YEAR({d '1991-02-26'})"},
	{idsFnCurtime            ,L"fn CURTIME()"},
	{idsFnHour               ,L"fn HOUR({t '14:49:19'})"},
	{idsFnMinute             ,L"fn MINUTE({t '14:49:19'})"},
	{idsFnSecond             ,L"fn SECOND({t '14:49:19'})"},
	{idsFnDatabase           ,L"fn DATABASE()"},
	{idsFnIfnull1            ,L"fn IFNULL(NULL,4)"},
	{idsFnIfnull2            ,L"fn IFNULL(-1,4)"},
	{idsFnDiff		   ,L"fn DIFFERENCE('c','ab')"},
	{idsFnSoundex		   ,L"fn SOUNDEX('ab')" },
	{idsFnSpace		   ,L"fn SPACE(5)"},						
	{idsFnDegrees		   ,L"fn DEGREES(3)"},
	{idsFnLog10		   ,L"fn LOG10(97.1)"},
	{idsFnPower		   ,L"fn POWER(9.600,3)"},
	{idsFnRadians		   ,L"fn RADIANS(8.5)"},
	{idsFnRound		   ,L"fn ROUND(97.56,1)"},
	{idsFnTruncate	   ,L"fn TRUNCATE(97.56,1)"}	,
	{idsFnDayname		   ,L"fn DAYNAME({d '1994-12-12'})"},
	{idsFnMonthname	   ,L"fn MONTHNAME({d '1994-12-12'})"},
	{idsFnTimestampadd	  ,L"fn TIMESTAMPADD(SQL_TSI_DAY,2,{d '1994-12-12'})"}	 	,
	{idsFnTimestampdiff	  ,L"fn TIMESTAMPDIFF(SQL_TSI_MONTH,{d '1994-11-15'},{d '1994-12-12'})"},
	{idsFnTimestamp	   ,L"ts '1994-12-12 01:12:56'"	},
	{idsFnDate		   ,L"d '1995-01-15'"},
	{idsFnTime		   ,L"t '07:15:26'"},

	{idsFnConcatNested	,L"fn concat({fn ucase('aaa')},{fn ucase('bbb')})"},
	{idsFnLeftNested		,L"fn left('abcdefg',{fn ceiling(2)})"	},
	{idsFnRightNested	,L"fn right('abcdefg',{fn ceiling(2)})"},
	{idsFnLocateNested	,L"fn locate({fn lcase('A')},'bbac')"},
	{idsFnSubstringNested	,L"fn substring('abcdefg',{fn floor(2)},{fn ceiling(2)})"},
	{idsFnDiffNested		,L"fn DIFFERENCE({fn lcase('A')},{fn lcase('BC')})"},
	{idsFnInsertNested	,L"fn INSERT('abcdefg',2,3,{fn ucase('xy')})"},
	{idsFnRepeatNested	,L"fn REPEAT({fn lcase('ABC')},3)"},
	{idsFnReplaceNested	,L"fn REPLACE('acbcdc',{fn lcase('C')},{fn ucase('x')})"},
	{idsFnMultiInsert		,L"fn INSERT({fn LCASE({fn CONCAT('12345678',{fn SPACE(4)})})},4,1,'XWZ')"},
	{idsFnMultiConcat		,L"fn CONCAT({fn UCASE({fn CHAR(99)})}, {fn SPACE(4)})"},
	{idsFnStringAndNumeric	,L"fn CHAR({fn ABS({fn FLOOR(99)})})"},
	
	{idsConvertNotSupported     ,L"Convert functions are not supported"},
	{idsTestConvertFunctions    ,L"Data Type Conversions"},
	{idsProcedureNotSupported   ,L"Procedures are not supported"},
	{idsTestProcedures          ,L"Procedure tests"},
	{idsTestOuterJoins          ,L"OuterJoin tests"},
	{idsSelectProcText	      	,L"create proc %s as select * from %s"	},
	{idsProcCall		      		,L"{call  %s}"},
	{idsDropProc		      		,L"drop proc %s"},
	{idsDropORAProc	      		,L"drop procedure %s"},
	{idsSelectORAProcText      	,L"create or replace procedure %s as begin select * from %s; end;"	},
	{idsLongProcOnly	      	,L"--(* vendor(Microsoft),product(ODBC) call %s *)--"	},

	{idsInsertLikeTbl	      	,L"insert into %s values ('%%a')"},
	{idsSelectLikeEscape	      ,L"select col1 from %s where col1 like '%s%%a__%%' --(*vendor(Microsoft),product(ODBC) escape '%s'*)--"},
	{idsSelectLikeShort	      ,L"select col1 from %s where col1 like '%s%%a__%%' {escape '%s'}"},
	{idsSelectLikeNative	      ,L"select col1 from %s where col1 like '%s%%a__%%' ESCAPE '%s'"},

	{idsTestLikePreds	      	,L"LIKE Predicate tests"	},

	{idsLongLJoin	             ,L"select * from --(*vendor(Microsoft),product(ODBC) oj %s left outer join %s on %s.%s%s%s.%s*)--"},
	{idsShortLJoin	     			,L"select * from {oj %s left outer join %s on %s.%s%s%s.%s}"	},
	{idsLongRJoin	             ,L"select * from --(*vendor(Microsoft),product(ODBC) oj %s right outer join %s on %s.%s%s%s.%s*)--"},
	{idsShortRJoin	     			,L"select * from {oj %s right outer join %s on %s.%s%s%s.%s}"	},
	{idsNestedLongJoin         	,L"select %s.%s from --(*vendor(Microsoft),product(ODBC) oj %s left outer join %s left outer join %s on %s.%s%s%s.%s on %s.%s%s%s.%s*)--"},
	{idsNestedShortJoin	     	,L"select %s.%s from {oj %s left outer join %s left outer join %s on %s.%s%s%s.%s on %s.%s%s%s.%s}"},
	{idsTestLeftOJSyntax	     	,L"Left Outer Join escape syntax"	},
	{idsTestRightOJSyntax	     	,L"Right Outer Join escape syntax"	},
	{idsTestNestedOJSyntax     	,L"Nested Outer Join escape syntax"	},
	{idsInOutShortOJSyntax     	,L"select  * from %s, {oj %s left outer join %s on %s.%s%s%s.%s}  where %s.%s%s%s.%s" 	},
	{idsInOutLongOJSyntax	     	,L"select  * from %s, --(*vendor(Microsoft),product(ODBC) oj %s left outer join %s on %s.%s%s%s.%s*)-- where %s.%s%s%s.%s"						},
	{idsNoLikePred					,L"LIKE Predicate not supported"},
	{idsJetDropProc					,L"drop table %s"			},
	{idsProcCallQ 	 				,L"{?=call  %s}"},
	{idsProcCallQPP	 				,L"{?=call  %s()}"},

	{idsSelect                 ,L"Select "},
	{idsFnAscii                ,L"fn ASCII('Bz')"},
	{idsFnChar                 ,L"fn CHAR(102)"},
	{idsFnSubstring            ,L"fn SUBSTRING('  forest estates',5,4)"},
	{idsFnAbs                  ,L"fn ABS(-123)"},
	{idsTestDateTimeFunctions  ,L"Time and Date Functions"},

	{idsFnNow                  ,L"fn NOW()"},
   {idsFnCurdate              ,L"fn CURDATE()"},
   {idsFnDayofmonth           ,L"fn DAYOFMONTH({d '1991-02-26'})"},
   {idsFnUser                 ,L"fn USER()"},
   {idsFnCHAR                 ,L"fn CONVERT(%s,SQL_CHAR)"},
   {idsFnVARCHAR              ,L"fn CONVERT(%s,SQL_VARCHAR)"},
   {idsFnLONGVARCHAR          ,L"fn CONVERT(%s,SQL_LONGVARCHAR)"},
   {idsFnLONGVARBINARY        ,L"fn CONVERT(%s,SQL_LONGVARBINARY)"},
   {idsFnDECIMAL              ,L"fn CONVERT(%s,SQL_DECIMAL)"},
   {idsFnNUMERIC              ,L"fn CONVERT(%s,SQL_NUMERIC)"},
   {idsFnBIT                  ,L"fn CONVERT(%s,SQL_BIT)"},
   {idsFnTINYINT              ,L"fn CONVERT(%s,SQL_TINYINT)"},
   {idsFnSMALLINT             ,L"fn CONVERT(%s,SQL_SMALLINT)"},
   {idsFnINTEGER              ,L"fn CONVERT(%s,SQL_INTEGER)"},
   {idsFnBIGINT               ,L"fn CONVERT(%s,SQL_BIGINT)"},
   {idsFnREAL                 ,L"fn CONVERT(%s,SQL_REAL)"},
   {idsFnFLOAT                ,L"fn CONVERT(%s,SQL_FLOAT)"},
   {idsFnDOUBLE               ,L"fn CONVERT(%s,SQL_DOUBLE)"},
   {idsFnBINARY               ,L"fn CONVERT(%s,SQL_BINARY)"},
   {idsFnVARBINARY            ,L"fn CONVERT(%s,SQL_VARBINARY)"},
   {idsFnDATE                 ,L"fn CONVERT(%s,SQL_DATE)"},
   {idsFnTIME                 ,L"fn CONVERT(%s,SQL_TIME)"},
	{idsFnTIMESTAMP            ,L"fn CONVERT(%s,SQL_TIMESTAMP)"},
	{idsFnWCHAR                 ,L"fn CONVERT(%s,SQL_WCHAR)"},
   {idsFnWVARCHAR              ,L"fn CONVERT(%s,SQL_WVARCHAR)"},
   {idsFnWLONGVARCHAR          ,L"fn CONVERT(%s,SQL_WLONGVARCHAR)"},
   {idsFnGUID			       ,L"fn CONVERT(%s,SQL_GUID)"},


    {idsCallFailed		,L"ExecDirect call failed even though the string function was supported."},
	{idsCanonShort     ,L"{%s}"},
	{idsFromString           ,L" from %s"}
};

struct ValueInfo
{
	DBSTATUS		dbsStatus;
	DBLENGTH		cbLength;
	void			*pValue;
};


/* SQL_STRING_FUNCTIONS functions */

#define SQL_FN_STR_CONCAT                   0x00000001L
#define SQL_FN_STR_INSERT                   0x00000002L
#define SQL_FN_STR_LEFT                     0x00000004L
#define SQL_FN_STR_LTRIM                    0x00000008L
#define SQL_FN_STR_LENGTH                   0x00000010L
#define SQL_FN_STR_LOCATE                   0x00000020L
#define SQL_FN_STR_LCASE                    0x00000040L
#define SQL_FN_STR_REPEAT                   0x00000080L
#define SQL_FN_STR_REPLACE                  0x00000100L
#define SQL_FN_STR_RIGHT                    0x00000200L
#define SQL_FN_STR_RTRIM                    0x00000400L
#define SQL_FN_STR_SUBSTRING                0x00000800L
#define SQL_FN_STR_UCASE                    0x00001000L
#define SQL_FN_STR_ASCII                    0x00002000L
#define SQL_FN_STR_CHAR                     0x00004000L
#define SQL_FN_STR_DIFFERENCE               0x00008000L
#define SQL_FN_STR_LOCATE_2                 0x00010000L
#define SQL_FN_STR_SOUNDEX                  0x00020000L
#define SQL_FN_STR_SPACE                    0x00040000L
#define SQL_FN_STR_BIT_LENGTH				0x00080000L
#define SQL_FN_STR_CHAR_LENGTH				0x00100000L
#define SQL_FN_STR_CHARACTER_LENGTH			0x00200000L
#define SQL_FN_STR_OCTET_LENGTH				0x00400000L
#define SQL_FN_STR_POSITION					0x00800000L

/* SQL_SQL92_STRING_FUNCTIONS */
#define SQL_SSF_CONVERT						0x00000001L	
#define SQL_SSF_LOWER						0x00000002L
#define SQL_SSF_UPPER						0x00000004L
#define SQL_SSF_SUBSTRING					0x00000008L
#define SQL_SSF_TRANSLATE					0x00000010L
#define SQL_SSF_TRIM_BOTH					0x00000020L
#define SQL_SSF_TRIM_LEADING				0x00000040L
#define SQL_SSF_TRIM_TRAILING				0x00000080L


/* SQL_NUMERIC_FUNCTIONS functions */
#define SQL_FN_NUM_ABS                      0x00000001L
#define SQL_FN_NUM_ACOS                     0x00000002L
#define SQL_FN_NUM_ASIN                     0x00000004L
#define SQL_FN_NUM_ATAN                     0x00000008L
#define SQL_FN_NUM_ATAN2                    0x00000010L
#define SQL_FN_NUM_CEILING                  0x00000020L
#define SQL_FN_NUM_COS                      0x00000040L
#define SQL_FN_NUM_COT                      0x00000080L
#define SQL_FN_NUM_EXP                      0x00000100L
#define SQL_FN_NUM_FLOOR                    0x00000200L
#define SQL_FN_NUM_LOG                      0x00000400L
#define SQL_FN_NUM_MOD                      0x00000800L
#define SQL_FN_NUM_SIGN                     0x00001000L
#define SQL_FN_NUM_SIN                      0x00002000L
#define SQL_FN_NUM_SQRT                     0x00004000L
#define SQL_FN_NUM_TAN                      0x00008000L
#define SQL_FN_NUM_PI                       0x00010000L
#define SQL_FN_NUM_RAND                     0x00020000L
#define SQL_FN_NUM_DEGREES                  0x00040000L
#define SQL_FN_NUM_LOG10                    0x00080000L
#define SQL_FN_NUM_POWER                    0x00100000L
#define SQL_FN_NUM_RADIANS                  0x00200000L
#define SQL_FN_NUM_ROUND                    0x00400000L
#define SQL_FN_NUM_TRUNCATE                 0x00800000L


/* SQL_TIMEDATE_FUNCTIONS functions */

#define SQL_FN_TD_NOW                       0x00000001L
#define SQL_FN_TD_CURDATE                   0x00000002L
#define SQL_FN_TD_DAYOFMONTH                0x00000004L
#define SQL_FN_TD_DAYOFWEEK                 0x00000008L
#define SQL_FN_TD_DAYOFYEAR                 0x00000010L
#define SQL_FN_TD_MONTH                     0x00000020L
#define SQL_FN_TD_QUARTER                   0x00000040L
#define SQL_FN_TD_WEEK                      0x00000080L
#define SQL_FN_TD_YEAR                      0x00000100L
#define SQL_FN_TD_CURTIME                   0x00000200L
#define SQL_FN_TD_HOUR                      0x00000400L
#define SQL_FN_TD_MINUTE                    0x00000800L
#define SQL_FN_TD_SECOND                    0x00001000L
#define SQL_FN_TD_TIMESTAMPADD              0x00002000L
#define SQL_FN_TD_TIMESTAMPDIFF             0x00004000L
#define SQL_FN_TD_DAYNAME                   0x00008000L
#define SQL_FN_TD_MONTHNAME                 0x00010000L
#define SQL_FN_TD_CURRENT_DATE				0x00020000L
#define SQL_FN_TD_CURRENT_TIME				0x00040000L
#define SQL_FN_TD_CURRENT_TIMESTAMP			0x00080000L
#define SQL_FN_TD_EXTRACT					0x00100000L


/* SQL_SYSTEM_FUNCTIONS functions */
#define SQL_FN_SYS_USERNAME                 0x00000001L
#define SQL_FN_SYS_DBNAME                   0x00000002L
#define SQL_FN_SYS_IFNULL                   0x00000004L

/* SQL_OJ_CAPABILITIES bitmasks */
/* NB: this means 'outer join', not what  you may be thinking */
#define SQL_OJ_LEFT                         0x00000001L
#define SQL_OJ_RIGHT                        0x00000002L
#define SQL_OJ_FULL                         0x00000004L
#define SQL_OJ_NESTED                       0x00000008L
#define SQL_OJ_NOT_ORDERED                  0x00000010L
#define SQL_OJ_INNER                        0x00000020L
#define SQL_OJ_ALL_COMPARISON_OPS           0x00000040L


/* SQL_CONVERT_*  return value bitmasks */

#define SQL_CVT_CHAR                        0x00000001L
#define SQL_CVT_NUMERIC                     0x00000002L
#define SQL_CVT_DECIMAL                     0x00000004L
#define SQL_CVT_INTEGER                     0x00000008L
#define SQL_CVT_SMALLINT                    0x00000010L
#define SQL_CVT_FLOAT                       0x00000020L
#define SQL_CVT_REAL                        0x00000040L
#define SQL_CVT_DOUBLE                      0x00000080L
#define SQL_CVT_VARCHAR                     0x00000100L
#define SQL_CVT_LONGVARCHAR                 0x00000200L
#define SQL_CVT_BINARY                      0x00000400L
#define SQL_CVT_VARBINARY                   0x00000800L
#define SQL_CVT_BIT                         0x00001000L
#define SQL_CVT_TINYINT                     0x00002000L
#define SQL_CVT_BIGINT                      0x00004000L
#define SQL_CVT_DATE                        0x00008000L
#define SQL_CVT_TIME                        0x00010000L
#define SQL_CVT_TIMESTAMP                   0x00020000L
#define SQL_CVT_LONGVARBINARY               0x00040000L
#define SQL_CVT_INTERVAL_YEAR_MONTH	    	0x00080000L
#define SQL_CVT_INTERVAL_DAY_TIME	    	0x00100000L
#define	SQL_CVT_WCHAR						0x00200000L
#define	SQL_CVT_WLONGVARCHAR				0x00400000L
#define	SQL_CVT_WVARCHAR					0x00800000L
#define	SQL_CVT_GUID						0x01000000L	// MAY NEED TO CHANGE PENDING ODBC HEADER  

#define SQL_CONVERT_BIGINT                  53
#define SQL_CONVERT_BINARY                  54
#define SQL_CONVERT_BIT                     55
#define SQL_CONVERT_CHAR                    56
#define SQL_CONVERT_DATE                    57
#define SQL_CONVERT_DECIMAL                 58
#define SQL_CONVERT_DOUBLE                  59
#define SQL_CONVERT_FLOAT                   60
#define SQL_CONVERT_INTEGER                 61
#define SQL_CONVERT_LONGVARCHAR             62
#define SQL_CONVERT_NUMERIC                 63
#define SQL_CONVERT_REAL                    64
#define SQL_CONVERT_SMALLINT                65
#define SQL_CONVERT_TIME                    66
#define SQL_CONVERT_TIMESTAMP               67
#define SQL_CONVERT_TINYINT                 68
#define SQL_CONVERT_VARBINARY               69
#define SQL_CONVERT_VARCHAR                 70
#define SQL_CONVERT_LONGVARBINARY           71
#define SQL_CONVERT_GUID					72   // MAY NEED TO CHANGE PENDING ODBC HEADER
#define	SQL_CONVERT_WCHAR						122
#define	SQL_CONVERT_WVARCHAR					126
#define	SQL_CONVERT_WLONGVARCHAR				125

#define SQL_CHAR            1
#define SQL_NUMERIC         2
#define SQL_DECIMAL         3
#define SQL_INTEGER         4
#define SQL_SMALLINT        5
#define SQL_FLOAT           6
#define SQL_REAL            7
#define SQL_DOUBLE          8
#define SQL_VARCHAR        12
#define SQL_DATE                                9
#define SQL_TIME                                10
#define SQL_TIMESTAMP                           11
#define SQL_LONGVARCHAR                         (-1)
#define SQL_BINARY                              (-2)
#define SQL_VARBINARY                           (-3)
#define SQL_LONGVARBINARY                       (-4)
#define SQL_BIGINT                              (-5)
#define SQL_TINYINT                             (-6)
#define SQL_BIT                                 (-7)
#define SQL_WCHAR		 	(-8)
#define SQL_WVARCHAR	 	(-9)
#define SQL_WLONGVARCHAR 	(-10)
#define SQL_GUID				(-11)

#define DRVR_SQLSRVR 0  // SQL Server
#define DRVR_ORACLE 1	// Oracle
#define DRVR_QJET	2	// Access


#endif 	//_SCALAR_H_
