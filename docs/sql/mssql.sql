--
-- Database tables for Gammu SMSD on Microsoft SQL Server
--
-- Use a UTF-8 collation for the database because SMSD sends UTF-8 text
-- through the ODBC narrow-character interface.
--

SET NOCOUNT ON;
SET QUOTED_IDENTIFIER ON;

DROP TABLE IF EXISTS sentitems;
DROP TABLE IF EXISTS phones;
DROP TABLE IF EXISTS outbox_multipart;
DROP TABLE IF EXISTS outbox;
DROP TABLE IF EXISTS inbox;
DROP TABLE IF EXISTS gammu;
GO

CREATE TABLE gammu (
  "Version" smallint NOT NULL DEFAULT 0 PRIMARY KEY
);

INSERT INTO gammu ("Version") VALUES (18);
GO

CREATE TABLE inbox (
  "UpdatedInDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "ReceivingDateTime" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "Text" varchar(max) NOT NULL,
  "SenderNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" varchar(max) NOT NULL,
  "SMSCNumber" varchar(20) NOT NULL DEFAULT '',
  "Class" integer NOT NULL DEFAULT -1,
  "TextDecoded" varchar(max) NOT NULL DEFAULT '',
  "ID" integer IDENTITY(1, 1) PRIMARY KEY,
  "MessageID" integer NOT NULL DEFAULT 0,
  "SequencePosition" integer NOT NULL DEFAULT 1,
  "PartCount" integer NOT NULL DEFAULT 1,
  "RecipientID" varchar(max) NOT NULL,
  "Processed" bit NOT NULL DEFAULT 0,
  "Status" integer NOT NULL DEFAULT -1,
  CHECK ("Coding" IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression'))
);
GO

CREATE INDEX inbox_message ON inbox("MessageID", "SequencePosition");
GO

CREATE TRIGGER inbox_update_timestamp ON inbox AFTER UPDATE AS
BEGIN
  SET NOCOUNT ON;
  IF TRIGGER_NESTLEVEL() > 1 RETURN;
  UPDATE target
     SET "UpdatedInDB" = CURRENT_TIMESTAMP
    FROM inbox AS target
    JOIN inserted AS changed ON target."ID" = changed."ID";
END;
GO

CREATE TABLE outbox (
  "UpdatedInDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "InsertIntoDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "SendingDateTime" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "SendBefore" time(0) NOT NULL DEFAULT '23:59:59',
  "SendAfter" time(0) NOT NULL DEFAULT '00:00:00',
  "SendDays" integer NOT NULL DEFAULT 127,
  "Text" varchar(max) NULL,
  "DestinationNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" varchar(max) NULL,
  "Class" integer DEFAULT -1,
  "TextDecoded" varchar(max) NOT NULL DEFAULT '',
  "ID" integer IDENTITY(1, 1) PRIMARY KEY,
  "MultiPart" bit NOT NULL DEFAULT 0,
  "RelativeValidity" integer DEFAULT -1,
  "SenderID" varchar(255) NULL,
  "SendingTimeOut" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "DeliveryReport" varchar(10) DEFAULT 'default',
  "CreatorID" varchar(max) NOT NULL,
  "Retries" integer DEFAULT 0,
  "Priority" integer DEFAULT 0,
  "Status" varchar(255) NOT NULL DEFAULT 'Reserved',
  "StatusCode" integer NOT NULL DEFAULT -1,
  CHECK ("Coding" IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CHECK ("DeliveryReport" IN ('default','yes','no')),
  CHECK ("Status" IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error','Reserved'))
);

CREATE INDEX outbox_date ON outbox("SendingDateTime", "SendingTimeOut");
CREATE INDEX outbox_sender ON outbox("SenderID");
GO

CREATE TRIGGER outbox_update_timestamp ON outbox AFTER UPDATE AS
BEGIN
  SET NOCOUNT ON;
  IF TRIGGER_NESTLEVEL() > 1 RETURN;
  UPDATE target
     SET "UpdatedInDB" = CURRENT_TIMESTAMP
    FROM outbox AS target
    JOIN inserted AS changed ON target."ID" = changed."ID";
END;
GO

CREATE TABLE outbox_multipart (
  "Text" varchar(max) NULL,
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" varchar(max) NULL,
  "Class" integer DEFAULT -1,
  "TextDecoded" varchar(max) NULL,
  "ID" integer NOT NULL DEFAULT 0,
  "SequencePosition" integer NOT NULL DEFAULT 1,
  "Status" varchar(255) NOT NULL DEFAULT 'Reserved',
  "StatusCode" integer NOT NULL DEFAULT -1,
  PRIMARY KEY ("ID", "SequencePosition"),
  CHECK ("Coding" IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CHECK ("Status" IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error','Reserved'))
);
GO

CREATE TABLE phones (
  "ID" varchar(max) NOT NULL,
  "UpdatedInDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "InsertIntoDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "TimeOut" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "Send" varchar(3) NOT NULL DEFAULT 'no',
  "Receive" varchar(3) NOT NULL DEFAULT 'no',
  "IMEI" varchar(35) NOT NULL PRIMARY KEY,
  "IMSI" varchar(35) NOT NULL,
  "NetCode" varchar(10) DEFAULT 'ERROR',
  "NetName" varchar(35) DEFAULT 'ERROR',
  "Client" varchar(max) NOT NULL,
  "Battery" integer NOT NULL DEFAULT -1,
  "Signal" integer NOT NULL DEFAULT -1,
  "Sent" integer NOT NULL DEFAULT 0,
  "Received" integer NOT NULL DEFAULT 0,
  CHECK ("Send" IN ('yes','no')),
  CHECK ("Receive" IN ('yes','no'))
);
GO

CREATE TRIGGER phones_update_timestamp ON phones AFTER UPDATE AS
BEGIN
  SET NOCOUNT ON;
  IF TRIGGER_NESTLEVEL() > 1 RETURN;
  UPDATE target
     SET "UpdatedInDB" = CURRENT_TIMESTAMP
    FROM phones AS target
    JOIN inserted AS changed ON target."IMEI" = changed."IMEI";
END;
GO

CREATE TABLE sentitems (
  "UpdatedInDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "InsertIntoDB" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "SendingDateTime" datetime2(0) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  "DeliveryDateTime" datetime2(0) NULL,
  "Text" varchar(max) NOT NULL,
  "DestinationNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" varchar(max) NOT NULL,
  "SMSCNumber" varchar(20) NOT NULL DEFAULT '',
  "Class" integer NOT NULL DEFAULT -1,
  "TextDecoded" varchar(max) NOT NULL DEFAULT '',
  "ID" integer NOT NULL DEFAULT 0,
  "SenderID" varchar(255) NOT NULL,
  "SequencePosition" integer NOT NULL DEFAULT 1,
  "Status" varchar(255) NOT NULL DEFAULT 'SendingOK',
  "StatusError" integer NOT NULL DEFAULT -1,
  "TPMR" integer NOT NULL DEFAULT -1,
  "RelativeValidity" integer NOT NULL DEFAULT -1,
  "CreatorID" varchar(max) NOT NULL,
  "StatusCode" integer NOT NULL DEFAULT -1,
  PRIMARY KEY ("ID", "SequencePosition"),
  CHECK ("Status" IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error')),
  CHECK ("Coding" IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression'))
);

CREATE INDEX sentitems_date ON sentitems("DeliveryDateTime");
CREATE INDEX sentitems_tpmr ON sentitems("TPMR");
CREATE INDEX sentitems_dest ON sentitems("DestinationNumber");
CREATE INDEX sentitems_sender ON sentitems("SenderID");
GO

CREATE TRIGGER sentitems_update_timestamp ON sentitems AFTER UPDATE AS
BEGIN
  SET NOCOUNT ON;
  IF TRIGGER_NESTLEVEL() > 1 RETURN;
  UPDATE target
     SET "UpdatedInDB" = CURRENT_TIMESTAMP
    FROM sentitems AS target
    JOIN inserted AS changed
      ON target."ID" = changed."ID"
     AND target."SequencePosition" = changed."SequencePosition";
END;
GO
