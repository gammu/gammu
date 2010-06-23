#ifndef sonyeric_h
#define sonyeric_h

GSM_Error SONYERIC_GetNextCalendar	(GSM_StateMachine *, GSM_CalendarEntry *, bool);
GSM_Error SONYERIC_GetNextToDo		(GSM_StateMachine *, GSM_ToDoEntry *, bool);
GSM_Error SONYERIC_GetToDoStatus	(GSM_StateMachine *, GSM_ToDoStatus *);
GSM_Error SONYERIC_AddCalendarNote	(GSM_StateMachine *, GSM_CalendarEntry *);
GSM_Error SONYERIC_AddToDo		(GSM_StateMachine *, GSM_ToDoEntry *);
GSM_Error SONYERIC_DeleteAllToDo	(GSM_StateMachine *);
GSM_Error SONYERIC_DelCalendarNote	(GSM_StateMachine *, GSM_CalendarEntry *);
GSM_Error SONYERIC_GetCalendarStatus	(GSM_StateMachine *, GSM_CalendarStatus *);

#endif
