--
-- Database tables for Gammu SMSD on Oracle Database
--
-- Oracle treats empty strings as NULL, so text columns which can contain an
-- empty SMSD value are nullable even where other database schemas use NOT NULL.
--

WHENEVER SQLERROR EXIT SQL.SQLCODE

BEGIN
  FOR object_to_drop IN (
    SELECT object_name
      FROM user_objects
     WHERE object_type = 'TABLE'
       AND object_name IN (
         'GAMMU', 'INBOX', 'OUTBOX', 'OUTBOX_MULTIPART', 'PHONES', 'SENTITEMS'
       )
  ) LOOP
    EXECUTE IMMEDIATE
      'DROP TABLE ' || DBMS_ASSERT.SIMPLE_SQL_NAME(object_to_drop.object_name) ||
      ' CASCADE CONSTRAINTS PURGE';
  END LOOP;

  FOR object_to_drop IN (
    SELECT object_name
      FROM user_objects
     WHERE object_type = 'SEQUENCE'
       AND object_name IN ('INBOX_ID_SEQ', 'OUTBOX_ID_SEQ')
  ) LOOP
    EXECUTE IMMEDIATE
      'DROP SEQUENCE ' || DBMS_ASSERT.SIMPLE_SQL_NAME(object_to_drop.object_name);
  END LOOP;
END;
/

CREATE TABLE gammu (
  Version NUMBER(5) DEFAULT 0 NOT NULL PRIMARY KEY
);

INSERT INTO gammu (Version) VALUES (18);

CREATE SEQUENCE inbox_ID_seq START WITH 1 INCREMENT BY 1
  MAXVALUE 9223372036854775807 NOCACHE;

CREATE TABLE inbox (
  UpdatedInDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  ReceivingDateTime TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  InsertIntoDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  Text CLOB,
  SenderNumber VARCHAR2(20 CHAR),
  Coding VARCHAR2(255 CHAR) DEFAULT 'Default_No_Compression' NOT NULL,
  UDH CLOB,
  SMSCNumber VARCHAR2(20 CHAR),
  Class NUMBER(10) DEFAULT -1 NOT NULL,
  TextDecoded CLOB,
  ID NUMBER(19) NOT NULL PRIMARY KEY,
  MessageID NUMBER(19) DEFAULT 0 NOT NULL,
  SequencePosition NUMBER(10) DEFAULT 1 NOT NULL,
  PartCount NUMBER(10) DEFAULT 1 NOT NULL,
  RecipientID VARCHAR2(255 CHAR),
  Processed NUMBER(1) DEFAULT 0 NOT NULL,
  Status NUMBER(10) DEFAULT -1 NOT NULL,
  CONSTRAINT inbox_coding_check CHECK (Coding IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CONSTRAINT inbox_id_check CHECK (ID BETWEEN 0 AND 9223372036854775807),
  CONSTRAINT inbox_message_id_check CHECK (MessageID BETWEEN 0 AND 9223372036854775807),
  CONSTRAINT inbox_processed_check CHECK (Processed IN (0, 1))
);

CREATE INDEX inbox_message ON inbox(MessageID, SequencePosition);
CREATE INDEX inbox_insert ON inbox(InsertIntoDB);

CREATE OR REPLACE TRIGGER inbox_biu
BEFORE INSERT OR UPDATE ON inbox
FOR EACH ROW
BEGIN
  IF INSERTING AND :NEW.ID IS NULL THEN
    SELECT inbox_ID_seq.NEXTVAL INTO :NEW.ID FROM dual;
  END IF;
  IF UPDATING THEN
    :NEW.UpdatedInDB := CURRENT_TIMESTAMP;
  END IF;
END;
/

CREATE SEQUENCE outbox_ID_seq START WITH 1 INCREMENT BY 1
  MAXVALUE 9223372036854775807 NOCACHE;

CREATE TABLE outbox (
  UpdatedInDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  InsertIntoDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  SendingDateTime TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  SendBefore VARCHAR2(8 CHAR) DEFAULT '23:59:59' NOT NULL,
  SendAfter VARCHAR2(8 CHAR) DEFAULT '00:00:00' NOT NULL,
  SendDays NUMBER(3) DEFAULT 127 NOT NULL,
  Text CLOB,
  DestinationNumber VARCHAR2(20 CHAR),
  Coding VARCHAR2(255 CHAR) DEFAULT 'Default_No_Compression' NOT NULL,
  UDH CLOB,
  Class NUMBER(10) DEFAULT -1,
  TextDecoded CLOB,
  ID NUMBER(19) NOT NULL PRIMARY KEY,
  MultiPart NUMBER(1) DEFAULT 0 NOT NULL,
  RelativeValidity NUMBER(10) DEFAULT -1,
  SenderID VARCHAR2(255 CHAR),
  SendingTimeOut TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP,
  DeliveryReport VARCHAR2(10 CHAR) DEFAULT 'default',
  CreatorID VARCHAR2(255 CHAR),
  Retries NUMBER(10) DEFAULT 0,
  Priority NUMBER(10) DEFAULT 0,
  Status VARCHAR2(255 CHAR) DEFAULT 'Reserved' NOT NULL,
  StatusCode NUMBER(10) DEFAULT -1 NOT NULL,
  CONSTRAINT outbox_coding_check CHECK (Coding IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CONSTRAINT outbox_delivery_check CHECK (DeliveryReport IN ('default','yes','no')),
  CONSTRAINT outbox_id_check CHECK (ID BETWEEN 0 AND 9223372036854775807),
  CONSTRAINT outbox_multipart_check CHECK (MultiPart IN (0, 1)),
  CONSTRAINT outbox_status_check CHECK (Status IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error','Reserved'))
);

CREATE INDEX outbox_date ON outbox(SendingDateTime, SendingTimeOut);
CREATE INDEX outbox_sender ON outbox(SenderID);

CREATE OR REPLACE TRIGGER outbox_biu
BEFORE INSERT OR UPDATE ON outbox
FOR EACH ROW
BEGIN
  IF INSERTING AND :NEW.ID IS NULL THEN
    SELECT outbox_ID_seq.NEXTVAL INTO :NEW.ID FROM dual;
  END IF;
  IF UPDATING THEN
    :NEW.UpdatedInDB := CURRENT_TIMESTAMP;
  END IF;
END;
/

CREATE TABLE outbox_multipart (
  Text CLOB,
  Coding VARCHAR2(255 CHAR) DEFAULT 'Default_No_Compression' NOT NULL,
  UDH CLOB,
  Class NUMBER(10) DEFAULT -1,
  TextDecoded CLOB,
  ID NUMBER(19) DEFAULT 0 NOT NULL,
  SequencePosition NUMBER(10) DEFAULT 1 NOT NULL,
  Status VARCHAR2(255 CHAR) DEFAULT 'Reserved' NOT NULL,
  StatusCode NUMBER(10) DEFAULT -1 NOT NULL,
  PRIMARY KEY (ID, SequencePosition),
  CONSTRAINT outbox_mp_coding_check CHECK (Coding IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CONSTRAINT outbox_mp_id_check CHECK (ID BETWEEN 0 AND 9223372036854775807),
  CONSTRAINT outbox_mp_status_check CHECK (Status IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error','Reserved'))
);

CREATE TABLE phones (
  ID VARCHAR2(255 CHAR),
  UpdatedInDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  InsertIntoDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  TimeOut TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  Send VARCHAR2(3 CHAR) DEFAULT 'no' NOT NULL,
  Receive VARCHAR2(3 CHAR) DEFAULT 'no' NOT NULL,
  IMEI VARCHAR2(35 CHAR) NOT NULL PRIMARY KEY,
  IMSI VARCHAR2(35 CHAR),
  NetCode VARCHAR2(10 CHAR) DEFAULT 'ERROR',
  NetName VARCHAR2(35 CHAR) DEFAULT 'ERROR',
  Client VARCHAR2(255 CHAR),
  Battery NUMBER(10) DEFAULT -1 NOT NULL,
  Signal NUMBER(10) DEFAULT -1 NOT NULL,
  Sent NUMBER(10) DEFAULT 0 NOT NULL,
  Received NUMBER(10) DEFAULT 0 NOT NULL,
  CONSTRAINT phones_send_check CHECK (Send IN ('yes','no')),
  CONSTRAINT phones_receive_check CHECK (Receive IN ('yes','no'))
);

CREATE OR REPLACE TRIGGER phones_bu
BEFORE UPDATE ON phones
FOR EACH ROW
BEGIN
  :NEW.UpdatedInDB := CURRENT_TIMESTAMP;
END;
/

CREATE TABLE sentitems (
  UpdatedInDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  InsertIntoDB TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  SendingDateTime TIMESTAMP(0) DEFAULT CURRENT_TIMESTAMP NOT NULL,
  DeliveryDateTime TIMESTAMP(0),
  Text CLOB,
  DestinationNumber VARCHAR2(20 CHAR),
  Coding VARCHAR2(255 CHAR) DEFAULT 'Default_No_Compression' NOT NULL,
  UDH CLOB,
  SMSCNumber VARCHAR2(20 CHAR),
  Class NUMBER(10) DEFAULT -1 NOT NULL,
  TextDecoded CLOB,
  ID NUMBER(19) DEFAULT 0 NOT NULL,
  SenderID VARCHAR2(255 CHAR),
  SequencePosition NUMBER(10) DEFAULT 1 NOT NULL,
  Status VARCHAR2(255 CHAR) DEFAULT 'SendingOK' NOT NULL,
  StatusError NUMBER(10) DEFAULT -1 NOT NULL,
  TPMR NUMBER(10) DEFAULT -1 NOT NULL,
  RelativeValidity NUMBER(10) DEFAULT -1 NOT NULL,
  CreatorID VARCHAR2(255 CHAR),
  StatusCode NUMBER(10) DEFAULT -1 NOT NULL,
  PRIMARY KEY (ID, SequencePosition),
  CONSTRAINT sentitems_id_check CHECK (ID BETWEEN 0 AND 9223372036854775807),
  CONSTRAINT sentitems_status_check CHECK (Status IN
    ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
     'DeliveryUnknown','Error')),
  CONSTRAINT sentitems_coding_check CHECK (Coding IN
    ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression'))
);

CREATE INDEX sentitems_date ON sentitems(DeliveryDateTime);
CREATE INDEX sentitems_tpmr ON sentitems(TPMR);
CREATE INDEX sentitems_dest ON sentitems(DestinationNumber);
CREATE INDEX sentitems_sender ON sentitems(SenderID);

CREATE OR REPLACE TRIGGER sentitems_bu
BEFORE UPDATE ON sentitems
FOR EACH ROW
BEGIN
  :NEW.UpdatedInDB := CURRENT_TIMESTAMP;
END;
/

COMMIT;
