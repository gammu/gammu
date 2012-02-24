; /* --------------------------------------------------------
; HEADER SECTION
;*/
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )
;
;
FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )
;
;/* ------------------------------------------------------------------
; MESSAGE DEFINITION SECTION
;*/

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=EVENT_CAT_SMSD
Language=English
SMSD
.

MessageId=0x2
SymbolicName=EVENT_CAT_GAMMU
Language=English
Gammu
.

MessageId=0x3
SymbolicName=EVENT_CAT_SQL
Language=English
SQL
.

MessageIdTypedef=DWORD

MessageId=0x100
Severity=Error
Facility=Runtime
SymbolicName=EVENT_MSG_ERROR
Language=English
%0
.

MessageId=0x101
Severity=Success
Facility=Runtime
SymbolicName=EVENT_MSG_INFO
Language=English
%0
.

MessageId=0x102
Severity=Informational
Facility=Runtime
SymbolicName=EVENT_MSG_NOTICE
Language=English
%0
.

MessageId=0x103
Severity=Informational
Facility=Runtime
SymbolicName=EVENT_MSG_SQL
Language=English
%0
.

MessageId=0x104
Severity=Informational
Facility=Runtime
SymbolicName=EVENT_MSG_GAMMU
Language=English
%0
.

MessageId=0x105
Severity=Informational
Facility=Runtime
SymbolicName=EVENT_MSG_OTHER
Language=English
%0
.
