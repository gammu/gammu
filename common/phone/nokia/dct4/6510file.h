
GSM_Error N6510_ReplyGetFileFolderInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error N6510_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start);

GSM_Error N6510_ReplyGetFileSystemStatus(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *status);

GSM_Error N6510_ReplyGetFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_GetFilePart(GSM_StateMachine *s, GSM_File *File);

GSM_Error N6510_ReplyAddFileHeader(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos);

GSM_Error N6510_ReplyDeleteFile(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_DeleteFile(GSM_StateMachine *s, unsigned char *ID);

GSM_Error N6510_ReplyAddFolder(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_AddFolder(GSM_StateMachine *s, GSM_File *File);
