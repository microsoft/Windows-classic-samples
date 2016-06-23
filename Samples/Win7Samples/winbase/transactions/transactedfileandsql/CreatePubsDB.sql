USE [master]
GO
CREATE DATABASE [pubs]  
GO
USE [pubs]
GO
CREATE TABLE [dbo].[jobs](
	[job_id] [smallint] IDENTITY (1, 1) NOT NULL,
	[job_desc] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[min_lvl] [tinyint] NOT NULL,
	[max_lvl] [tinyint] NOT NULL
) ON [PRIMARY]