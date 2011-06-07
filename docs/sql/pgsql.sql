-- 
-- Database: "smsd"
-- 
-- CREATE USER "smsd" WITH NOCREATEDB NOCREATEUSER;
-- CREATE DATABASE "smsd" WITH OWNER = "smsd" ENCODING = 'UTF8';
-- \connect "smsd" "smsd"
-- COMMENT ON DATABASE "smsd" IS 'Gammu SMSD Database';

-- --------------------------------------------------------

--
-- Function declaration for updating timestamps
--
CREATE LANGUAGE plpgsql;
CREATE OR REPLACE FUNCTION update_timestamp() RETURNS trigger AS $update_timestamp$
  BEGIN
    NEW."UpdatedInDB" := LOCALTIMESTAMP(0);
    RETURN NEW;
  END;
$update_timestamp$ LANGUAGE plpgsql;

-- --------------------------------------------------------

--
-- Sequence declarations for tables' primary keys
--

--CREATE SEQUENCE inbox_ID_seq;

--CREATE SEQUENCE outbox_ID_seq;

--CREATE SEQUENCE outbox_multipart_ID_seq;

--CREATE SEQUENCE pbk_groups_ID_seq;

--CREATE SEQUENCE sentitems_ID_seq;

-- --------------------------------------------------------

--
-- Index declarations for tables' primary keys
--

--CREATE UNIQUE INDEX inbox_pkey ON inbox USING btree ("ID");

--CREATE UNIQUE INDEX outbox_pkey ON outbox USING btree ("ID");

--CREATE UNIQUE INDEX outbox_multipart_pkey ON outbox_multipart USING btree ("ID");

--CREATE UNIQUE INDEX pbk_groups_pkey ON pbk_groups USING btree ("ID");

--CREATE UNIQUE INDEX sentitems_pkey ON sentitems USING btree ("ID");

-- --------------------------------------------------------
-- 
-- Table structure for table "daemons"
-- 

CREATE TABLE daemons (
  "Start" text NOT NULL,
  "Info" text NOT NULL
);

-- 
-- Dumping data for table "daemons"
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table "gammu"
-- 

CREATE TABLE gammu (
  "Version" smallint NOT NULL DEFAULT '0'
);

-- 
-- Dumping data for table "gammu"
-- 

INSERT INTO gammu ("Version") VALUES (13);

-- --------------------------------------------------------

-- 
-- Table structure for table "inbox"
-- 

CREATE TABLE inbox (
  "UpdatedInDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "ReceivingDateTime" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "Text" text NOT NULL,
  "SenderNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" text NOT NULL,
  "SMSCNumber" varchar(20) NOT NULL DEFAULT '',
  "Class" integer NOT NULL DEFAULT '-1',
  "TextDecoded" text NOT NULL DEFAULT '',
  "ID" serial PRIMARY KEY,
  "RecipientID" text NOT NULL,
  "Processed" boolean NOT NULL DEFAULT 'false',
  CHECK ("Coding" IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')) 
);

-- 
-- Dumping data for table "inbox"
-- 

-- --------------------------------------------------------

--
-- Create trigger for table "inbox"
--

CREATE TRIGGER update_timestamp BEFORE UPDATE ON inbox FOR EACH ROW EXECUTE PROCEDURE update_timestamp();

-- --------------------------------------------------------

-- 
-- Table structure for table "outbox"
-- 

CREATE TABLE outbox (
  "UpdatedInDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "InsertIntoDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "SendingDateTime" timestamp NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "SendBefore" time NOT NULL DEFAULT '23:59:59',
  "SendAfter" time NOT NULL DEFAULT '00:00:00',
  "Text" text,
  "DestinationNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" text,
  "Class" integer DEFAULT '-1',
  "TextDecoded" text NOT NULL DEFAULT '',
  "ID" serial PRIMARY KEY,
  "MultiPart" boolean NOT NULL DEFAULT 'false',
  "RelativeValidity" integer DEFAULT '-1',
  "SenderID" varchar(255),
  "SendingTimeOut" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "DeliveryReport" varchar(10) DEFAULT 'default',
  "CreatorID" text NOT NULL,
  CHECK ("Coding" IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CHECK ("DeliveryReport" IN ('default','yes','no'))
);

CREATE INDEX outbox_date ON outbox("SendingDateTime", "SendingTimeOut");
CREATE INDEX outbox_sender ON outbox("SenderID");

-- 
-- Dumping data for table "outbox"
-- 

-- --------------------------------------------------------

--
-- Create trigger for table "outbox"
--

CREATE TRIGGER update_timestamp BEFORE UPDATE ON outbox FOR EACH ROW EXECUTE PROCEDURE update_timestamp();

-- --------------------------------------------------------

-- 
-- Table structure for table "outbox_multipart"
-- 

CREATE TABLE outbox_multipart (
  "Text" text,
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" text,
  "Class" integer DEFAULT '-1',
  "TextDecoded" text DEFAULT NULL,
  "ID" serial,
  "SequencePosition" integer NOT NULL DEFAULT '1',
  PRIMARY KEY ("ID", "SequencePosition"),
  CHECK ("Coding" IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression'))
);

-- 
-- Dumping data for table "outbox_multipart"
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table "pbk"
-- 

CREATE TABLE pbk (
  "ID" serial PRIMARY KEY,
  "GroupID" integer NOT NULL DEFAULT '-1',
  "Name" text NOT NULL,
  "Number" text NOT NULL
);

-- 
-- Dumping data for table "pbk"
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table "pbk_groups"
-- 

CREATE TABLE pbk_groups (
  "Name" text NOT NULL,
  "ID" serial PRIMARY KEY
);

-- 
-- Dumping data for table "pbk_groups"
-- 


-- --------------------------------------------------------

-- 
-- Table structure for table "phones"
-- 

CREATE TABLE phones (
  "ID" text NOT NULL,
  "UpdatedInDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "InsertIntoDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "TimeOut" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "Send" boolean NOT NULL DEFAULT 'no',
  "Receive" boolean NOT NULL DEFAULT 'no',
  "IMEI" varchar(35) PRIMARY KEY NOT NULL,
  "Client" text NOT NULL,
  "Battery" integer NOT NULL DEFAULT -1,
  "Signal" integer NOT NULL DEFAULT -1,
  "Sent" integer NOT NULL DEFAULT 0,
  "Received" integer NOT NULL DEFAULT 0
);

-- 
-- Dumping data for table "phones"
-- 

-- --------------------------------------------------------

--
-- Create trigger for table "phones"
--

CREATE TRIGGER update_timestamp BEFORE UPDATE ON phones FOR EACH ROW EXECUTE PROCEDURE update_timestamp();

-- --------------------------------------------------------

-- 
-- Table structure for table "sentitems"
-- 

CREATE TABLE sentitems (
  "UpdatedInDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "InsertIntoDB" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "SendingDateTime" timestamp(0) WITHOUT time zone NOT NULL DEFAULT LOCALTIMESTAMP(0),
  "DeliveryDateTime" timestamp(0) WITHOUT time zone NULL,
  "Text" text NOT NULL,
  "DestinationNumber" varchar(20) NOT NULL DEFAULT '',
  "Coding" varchar(255) NOT NULL DEFAULT 'Default_No_Compression',
  "UDH" text NOT NULL,
  "SMSCNumber" varchar(20) NOT NULL DEFAULT '',
  "Class" integer NOT NULL DEFAULT '-1',
  "TextDecoded" text NOT NULL DEFAULT '',
  "ID" serial,
  "SenderID" varchar(255) NOT NULL,
  "SequencePosition" integer NOT NULL DEFAULT '1',
  "Status" varchar(255) NOT NULL DEFAULT 'SendingOK',
  "StatusError" integer NOT NULL DEFAULT '-1',
  "TPMR" integer NOT NULL DEFAULT '-1',
  "RelativeValidity" integer NOT NULL DEFAULT '-1',
  "CreatorID" text NOT NULL,
  CHECK ("Status" IN 
  ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
  'DeliveryUnknown','Error')),
  CHECK ("Coding" IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  PRIMARY KEY ("ID", "SequencePosition")
);

CREATE INDEX sentitems_date ON sentitems("DeliveryDateTime");
CREATE INDEX sentitems_tpmr ON sentitems("TPMR");
CREATE INDEX sentitems_dest ON sentitems("DestinationNumber");
CREATE INDEX sentitems_sender ON sentitems("SenderID");

-- 
-- Dumping data for table "sentitems"
-- 

-- --------------------------------------------------------

--
-- Create trigger for table "sentitems"
--

CREATE TRIGGER update_timestamp BEFORE UPDATE ON sentitems FOR EACH ROW EXECUTE PROCEDURE update_timestamp();

