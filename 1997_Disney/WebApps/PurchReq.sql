/* Microsoft SQL Server - Scripting			*/
/* Server: FIS-WEB					*/
/* Database: PurchReq					*/
/* Creation Date 12/3/1998 12:58:08 PM 			*/

/****** Object:  User KGuerra    Script Date: 12/3/1998 12:58:09 PM ******/
if not exists (select * from sysusers where name = 'KGuerra' and uid < 16382)
	EXEC sp_adduser 'KGuerra', 'KGuerra', 'public'
GO

/****** Object:  User PurchReq    Script Date: 12/3/1998 12:58:09 PM ******/
if not exists (select * from sysusers where name = 'PurchReq' and uid < 16382)
	EXEC sp_adduser 'PurchReq', 'PurchReq', 'public'
GO

/****** Object:  User PurchReqAdmin    Script Date: 12/3/1998 12:58:09 PM ******/
if not exists (select * from sysusers where name = 'PurchReqAdmin' and uid < 16382)
	EXEC sp_adduser 'PurchReqAdmin', 'PurchReqAdmin', 'public'
GO

GRANT  CREATE TABLE  TO PurchReqAdmin
GO

/****** Object:  Stored Procedure dbo.sp_UpdateTables    Script Date: 12/3/1998 12:58:09 PM ******/
if exists (select * from sysobjects where id = object_id('dbo.sp_UpdateTables') and sysstat & 0xf = 4)
	drop procedure dbo.sp_UpdateTables
GO

/****** Object:  View dbo.qryPurchReqDetail    Script Date: 12/3/1998 12:58:09 PM ******/
if exists (select * from sysobjects where id = object_id('dbo.qryPurchReqDetail') and sysstat & 0xf = 2)
	drop view dbo.qryPurchReqDetail
GO

/****** Object:  View dbo.qryPurchReqResult    Script Date: 12/3/1998 12:58:09 PM ******/
if exists (select * from sysobjects where id = object_id('dbo.qryPurchReqResult') and sysstat & 0xf = 2)
	drop view dbo.qryPurchReqResult
GO

/****** Object:  Table PurchReqAdmin.tblPurchReq    Script Date: 12/3/1998 12:58:09 PM ******/
if exists (select * from sysobjects where id = object_id('PurchReqAdmin.tblPurchReq') and sysstat & 0xf = 3)
	drop table PurchReqAdmin.tblPurchReq
GO

/****** Object:  Table PurchReqAdmin.tblPurchReqInput    Script Date: 12/3/1998 12:58:09 PM ******/
if exists (select * from sysobjects where id = object_id('PurchReqAdmin.tblPurchReqInput') and sysstat & 0xf = 3)
	drop table PurchReqAdmin.tblPurchReqInput
GO

/****** Object:  Table PurchReqAdmin.tblPurchReq    Script Date: 12/3/1998 12:58:09 PM ******/
CREATE TABLE PurchReqAdmin.tblPurchReq (
	PONo varchar (25) NULL ,
	ReqNo varchar (25) NULL ,
	BuyerName varchar (100) NULL ,
	VendorName varchar (100) NULL ,
	ClientName varchar (100) NULL ,
	DeliverToName varchar (100) NULL ,
	POType varchar (25) NULL ,
	EntryDate smalldatetime NULL ,
	DueDate smalldatetime NULL ,
	ExpirationDate smalldatetime NULL ,
	OrigNo1 varchar (10) NULL ,
	DeptNo1 varchar (10) NULL ,
	OrigNo2 varchar (10) NULL ,
	DeptNo2 varchar (10) NULL ,
	OrigNo3 varchar (10) NULL ,
	DeptNo3 varchar (10) NULL 
)
GO

 CREATE  INDEX idxPurchReq ON PurchReqAdmin.tblPurchReq(VendorName, ClientName)
GO

 CREATE  UNIQUE  CLUSTERED  INDEX idxPurchReqPONo ON PurchReqAdmin.tblPurchReq(PONo)
GO

GRANT  SELECT  ON tblPurchReq  TO PurchReq
GO

/****** Object:  Table PurchReqAdmin.tblPurchReqInput    Script Date: 12/3/1998 12:58:10 PM ******/
CREATE TABLE PurchReqAdmin.tblPurchReqInput (
	PONo varchar (25) NULL ,
	ReqNo varchar (25) NULL ,
	BuyerName varchar (100) NULL ,
	VendorName varchar (100) NULL ,
	ClientName varchar (100) NULL ,
	DeliverToName varchar (100) NULL ,
	POType varchar (25) NULL ,
	EntryDate smalldatetime NULL ,
	DueDate smalldatetime NULL ,
	ExpirationDate smalldatetime NULL ,
	OrigNo1 varchar (10) NULL ,
	DeptNo1 varchar (10) NULL ,
	OrigNo2 varchar (10) NULL ,
	DeptNo2 varchar (10) NULL ,
	OrigNo3 varchar (10) NULL ,
	DeptNo3 varchar (10) NULL 
)
GO

GRANT  SELECT  ON tblPurchReqInput  TO PurchReq
GO

/****** Object:  View dbo.qryPurchReqDetail    Script Date: 12/3/1998 12:58:10 PM ******/
CREATE VIEW qryPurchReqDetail AS

SELECT
	PurchReqAdmin.tblPurchReq.BuyerName,
	CONVERT(char(10), PurchReqAdmin.tblPurchReq.EntryDate, 101) AS 'EntryDate',
	PurchReqAdmin.tblPurchReq.ReqNo,
	PurchReqAdmin.tblPurchReq.ClientName,
	PurchReqAdmin.tblPurchReq.DeliverToName,
	PurchReqAdmin.tblPurchReq.OrigNo1,
	PurchReqAdmin.tblPurchReq.DeptNo1,
	PurchReqAdmin.tblPurchReq.OrigNo2,
	PurchReqAdmin.tblPurchReq.DeptNo2,
	PurchReqAdmin.tblPurchReq.OrigNo3,
	PurchReqAdmin.tblPurchReq.DeptNo3,
	PurchReqAdmin.tblPurchReq.VendorName,
	PurchReqAdmin.tblPurchReq.PONo,
	PurchReqAdmin.tblPurchReq.POType,
	CONVERT(char(10), PurchReqAdmin.tblPurchReq.ExpirationDate, 101) AS 'ExpirationDate',
	CONVERT(char(10), PurchReqAdmin.tblPurchReq.DueDate, 101) AS 'DueDate'

FROM PurchReqAdmin.tblPurchReq
GO

GRANT  SELECT  ON dbo.qryPurchReqDetail  TO PurchReq
GO

/****** Object:  View dbo.qryPurchReqResult    Script Date: 12/3/1998 12:58:10 PM ******/
CREATE VIEW qryPurchReqResult AS

SELECT
	PurchReqAdmin.tblPurchReq.ReqNo, 
	PurchReqAdmin.tblPurchReq.PONo, 
	PurchReqAdmin.tblPurchReq.VendorName, 
	PurchReqAdmin.tblPurchReq.ClientName, 
	CONVERT(char(10), PurchReqAdmin.tblPurchReq.DueDate, 101) AS 'DueDate'

FROM PurchReqAdmin.tblPurchReq
GO

GRANT  SELECT  ON dbo.qryPurchReqResult  TO PurchReq
GO

/****** Object:  Stored Procedure dbo.sp_UpdateTables    Script Date: 12/3/1998 12:58:10 PM ******/
CREATE PROCEDURE sp_UpdateTables AS

DROP INDEX tblPurchReq.idxPurchReqPONo
DROP INDEX tblPurchReq.idxPurchReq

TRUNCATE TABLE tblPurchReq

EXECUTE sp_rename tblPurchReq, tblPurchReqTemp
EXECUTE sp_rename tblPurchReqInput, tblPurchReq
EXECUTE sp_rename tblPurchReqTemp, tblPurchReqInput

CREATE UNIQUE CLUSTERED INDEX idxPurchReqPONo
	ON tblPurchReq (PONo)

CREATE INDEX idxPurchReq
	ON tblPurchReq (VendorName, ClientName)
GO

GRANT  EXECUTE  ON dbo.sp_UpdateTables  TO PurchReqAdmin
GO

