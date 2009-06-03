
/* calendar */

GSM_Error N6510_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetCalendar3(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start);

GSM_Error N6510_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddCalendar3(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);

GSM_Error N6510_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status);

/* notes */

GSM_Error N6510_ReplyGetNoteInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetNote(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, gboolean start);

GSM_Error N6510_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Not);

GSM_Error N6510_ReplyGetNoteFirstLoc(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddNote(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Not);

GSM_Error N6510_GetNoteStatus(GSM_StateMachine *s, GSM_ToDoStatus *status);

/* todo */

GSM_Error N6510_ReplyGetToDoStatus2(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetToDoStatus1(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status);

GSM_Error N6510_ReplyGetToDo1(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetToDo2(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, gboolean refresh);

GSM_Error N6510_ReplyDeleteAllToDo1(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_DeleteAllToDo1(GSM_StateMachine *s);
GSM_Error N6510_DeleteToDo2(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);

GSM_Error N6510_ReplyGetToDoFirstLoc1(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetToDoFirstLoc2(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddToDo1(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddToDo2(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);

/* calendar settings */

GSM_Error N6510_ReplyGetCalendarSettings(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings);
