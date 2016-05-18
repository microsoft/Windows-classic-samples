set quoted_identifier on
GO

if exists (select * from sysobjects where id = object_id('dbo.SecurityEvents') and sysstat & 0xf = 3)
	drop table "dbo"."SecurityEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.MethodEvents') and sysstat & 0xf = 3)
	drop table "dbo"."MethodEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.ObjectEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ObjectEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.AppEvents') and sysstat & 0xf = 3)
	drop table "dbo"."AppEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.ResourceEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ResourceEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.ThreadEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ThreadEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.TransactionEvents') and sysstat & 0xf = 3)
	drop table "dbo"."TransactionEvents"
GO


if exists (select * from sysobjects where id = object_id('dbo.ObjectPoolEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ObjectPoolEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.ActivityEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ActivityEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.ExceptionEvents') and sysstat & 0xf = 3)
	drop table "dbo"."ExceptionEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.QCEvents') and sysstat & 0xf = 3)
	drop table "dbo"."QCEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.CRMEvents') and sysstat & 0xf = 3)
	drop table "dbo"."CRMEvents"
GO

if exists (select * from sysobjects where id = object_id('dbo.LBEvents') and sysstat & 0xf = 3)
	drop table "dbo"."LBEvents"
GO

CREATE TABLE "dbo"."SecurityEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"ActivityID" char (40)  NULL ,
	"ObjectID" char (20) NOT NULL ,	
	"InterfaceID" char (40) NULL ,
	"Method" varchar(50) NULL,
	"OriginalUser" varchar (255) NULL ,
	"DirectUser" varchar (255) NULL ,
	"Impersonation" char (5)  NULL, 
	"ClientIP" char(20) NULL,
	"ServerIP" char(20) NULL,
	"URL" varchar (255) NULL
)
GO

CREATE TABLE "dbo"."MethodEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"ObjectID" char (20) NOT NULL ,
	"ClassID" char (40) NOT NULL ,
	"InterfaceID" char (40) NOT NULL ,
	"Method" varchar(50) NULL,
	"HResult" binary(4) NULL ,	
	"CallTime (ms)" char (16) NULL	
)
GO

CREATE TABLE "dbo"."ObjectEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,
	"ClassID" char (40) NULL ,		
	"ObjectID" char (20) NULL ,		
	"ActivityID" char (40) NULL ,
	"ContextID" char (20) NULL ,	
	"TSID" char (40) NULL ,	
	"ConstructString" varchar(100) NULL	
)
GO

CREATE TABLE "dbo"."AppEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL 	
)
GO

CREATE TABLE "dbo"."ResourceEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,	
	"ObjectID" char (20) NOT NULL ,	
	"Type" varchar (255) NOT NULL ,
	"ResourceID" char (20) NOT NULL ,	
	"Enlisted" char (8) NULL ,
	"NumRated" char (20) NULL ,
	"Rating" char (20) NULL ,
	"HResult" binary (4) NULL	
)
GO

CREATE TABLE "dbo"."ThreadEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,	
	"ThreadID" char (20) NULL ,	
	"W2KThreadID" char (12) NULL,
	"ActivityID" char (40) NULL,
	"ApartmentID" char (20) NULL ,
	"MsgWorkID" char (20) NULL ,	
	"ThreadCnt" "int" NULL,
	"ActivityCnt" "int" NULL,
	"QueueLen" "int" NULL,
	"ThreadNum" char (20) NULL 	
)
GO

CREATE TABLE "dbo"."TransactionEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,	
	"TransactionID" char (40) NOT NULL ,
	"TSID" char (40) NULL ,
	"Root" char(8) NULL ,
	"VoteYes" char(8) NULL	
)
GO


CREATE TABLE "dbo"."ObjectPoolEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"ClassID" char (40) NULL ,
	"ActivityID" char (40) NULL ,
	"TransactionID" char (40) NULL ,	
	"ObjectID" char (20) NULL,
	"CreatedObjs"	"int" NULL,
	"AvailableObjs" "int" NULL,	
	"ThreadsWaiting" "int" NULL,
	"Min"  "int" NULL,
	"Max"  "int" NULL,
	"Timeout"  "int" NULL
)
GO

CREATE TABLE "dbo"."ActivityEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"ActivityID" char (40) NULL ,
	"CurrentApp" char (40) NULL,	
	"EnteredApp" char (40) NULL,
	"LeftApp" char (40) NULL,
	"W2KThreadID" char (12) NULL,
	"Timeout"  "int" NULL,
	"CallDepth"  "int" NULL	
)
GO

CREATE TABLE "dbo"."ExceptionEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"Code" char (12) NOT NULL ,
	"Address" char (20) NULL,		
	"StackTrace" varchar (100) NULL,	
)
GO

CREATE TABLE "dbo"."QCEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,		
	"ObjectID" char (20) NULL ,
	"QueueName" varchar (60) NULL,	
	"QueueID" char (20) NULL,	
	"MsgID" char (40) NULL,	
	"WorkFlowID" char (40) NULL,	
	"HResult" binary (4) NULL,
	"RetryIndex" "int" NULL
)
GO

CREATE TABLE "dbo"."CRMEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,	
	"ClerkID" char (40) NULL,
	"ActivityID" char (40) NULL,
	"TransactionUOWID" char (40) NULL,
	"ProgIdCompensator" char (80) NULL ,
	"Description" char (80) NULL,	
	"CRMRecordType" char (12) NULL,
	"RecordSize" "int" NULL,
	"Variants" char (5) NULL	
)
GO

CREATE TABLE "dbo"."LBEvents" (
	"Count" "int" IDENTITY (0, 1) NOT NULL ,
	"Event" varchar (40) NOT NULL,
	"TickCount" "int" NOT NULL ,
	"Application" char (40) NOT NULL ,
	"ServerName" varchar (20) NULL,
	"ClsidEng" varchar (20) NULL	
)
GO

