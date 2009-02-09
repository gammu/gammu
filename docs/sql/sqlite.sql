CREATE TABLE daemons (
  Start TEXT NOT NULL,
  Info TEXT NOT NULL
);

CREATE TABLE gammu (
  Version INTEGER NOT NULL DEFAULT '0'
);

INSERT INTO gammu (Version) VALUES (10);

CREATE TABLE inbox (
  UpdatedInDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  ReceivingDateTime NUMERIC NOT NULL DEFAULT (datetime('now')),
  Text TEXT NOT NULL,
  SenderNumber TEXT NOT NULL DEFAULT '',
  Coding TEXT NOT NULL DEFAULT 'Default_No_Compression',
  UDH TEXT NOT NULL,
  SMSCNumber TEXT NOT NULL DEFAULT '',
  Class INTEGER NOT NULL DEFAULT '-1',
  TextDecoded TEXT NOT NULL DEFAULT '',
  ID INTEGER PRIMARY KEY AUTOINCREMENT,
  RecipientID TEXT NOT NULL,
  Processed TEXT NOT NULL DEFAULT 'false',
  CHECK (Coding IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')) 
);

CREATE TRIGGER update_inbox_time UPDATE ON inbox 
  BEGIN
    UPDATE inbox SET UpdatedInDB = datetime('now') WHERE ID = old.ID;
  END;

CREATE TABLE outbox (
  UpdatedInDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  InsertIntoDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  SendingDateTime NUMERIC NOT NULL DEFAULT (datetime('now')),
  Text TEXT,
  DestinationNumber TEXT NOT NULL DEFAULT '',
  Coding TEXT NOT NULL DEFAULT '8bit',
  UDH TEXT,
  Class INTEGER DEFAULT '-1',
  TextDecoded TEXT NOT NULL DEFAULT '',
  ID INTEGER PRIMARY KEY AUTOINCREMENT,
  MultiPart TEXT NOT NULL DEFAULT 'false',
  RelativeValidity INTEGER DEFAULT '-1',
  SenderID TEXT,
  SendingTimeOut NUMERIC NOT NULL DEFAULT (datetime('now')),
  DeliveryReport TEXT DEFAULT 'default',
  CreatorID TEXT NOT NULL,
  CHECK (Coding IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
  CHECK (DeliveryReport IN ('default','yes','no'))
);

CREATE TRIGGER update_outbox_time UPDATE ON outbox 
  BEGIN
    UPDATE outbox SET UpdatedInDB = datetime('now') WHERE ID = old.ID;
  END;

CREATE TABLE outbox_multipart (
  Text TEXT,
  Coding TEXT NOT NULL DEFAULT '8bit',
  UDH TEXT,
  Class INTEGER DEFAULT '-1',
  TextDecoded TEXT DEFAULT NULL,
  ID INTEGER,
  SequencePosition INTEGER NOT NULL DEFAULT '1',
  CHECK (Coding IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')),
 PRIMARY KEY (ID, SequencePosition)
);

CREATE TABLE pbk (
  GroupID INTEGER NOT NULL DEFAULT '-1',
  Name TEXT NOT NULL,
  Number TEXT NOT NULL
);

CREATE TABLE pbk_groups (
  Name TEXT NOT NULL,
  ID INTEGER PRIMARY KEY AUTOINCREMENT
);

CREATE TABLE phones (
  ID TEXT NOT NULL,
  UpdatedInDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  InsertIntoDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  TimeOut NUMERIC NOT NULL DEFAULT (datetime('now')),
  Send TEXT NOT NULL DEFAULT 'no',
  Receive TEXT NOT NULL DEFAULT 'no',
  IMEI TEXT PRIMARY KEY NOT NULL,
  Client TEXT NOT NULL,
  Battery INTEGER NOT NULL DEFAULT -1,
  Signal INTEGER NOT NULL DEFAULT -1,
  Sent INTEGER NOT NULL DEFAULT 0,
  Received INTEGER NOT NULL DEFAULT 0
);

CREATE TRIGGER update_phones_time UPDATE ON phones 
  BEGIN
    UPDATE phones SET UpdatedInDB = datetime('now') WHERE IMEI = old.IMEI;
  END;

CREATE TABLE sentitems (
  UpdatedInDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  InsertIntoDB NUMERIC NOT NULL DEFAULT (datetime('now')),
  SendingDateTime NUMERIC NOT NULL DEFAULT (datetime('now')),
  DeliveryDateTime NUMERIC NULL,
  Text TEXT NOT NULL,
  DestinationNumber TEXT NOT NULL DEFAULT '',
  Coding TEXT NOT NULL DEFAULT '8bit', 
  UDH TEXT NOT NULL,
  SMSCNumber TEXT NOT NULL DEFAULT '',
  Class INTEGER NOT NULL DEFAULT '-1',
  TextDecoded TEXT NOT NULL DEFAULT '',
  ID INTEGER,
  SenderID TEXT NOT NULL,
  SequencePosition INTEGER NOT NULL DEFAULT '1',
  Status TEXT NOT NULL DEFAULT 'SendingOK',
  StatusError INTEGER NOT NULL DEFAULT '-1',
  TPMR INTEGER NOT NULL DEFAULT '-1',
  RelativeValidity INTEGER NOT NULL DEFAULT '-1',
  CreatorID TEXT NOT NULL,
  CHECK (Status IN 
  ('SendingOK','SendingOKNoReport','SendingError','DeliveryOK','DeliveryFailed','DeliveryPending',
  'DeliveryUnknown','Error')),
  CHECK (Coding IN 
  ('Default_No_Compression','Unicode_No_Compression','8bit','Default_Compression','Unicode_Compression')) ,
 PRIMARY KEY (ID, SequencePosition)
);

CREATE TRIGGER update_sentitems_time UPDATE ON sentitems 
  BEGIN
    UPDATE sentitems SET UpdatedInDB = datetime('now') WHERE ID = old.ID;
  END;
