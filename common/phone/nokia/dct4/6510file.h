
GSM_Error N6510_GetNextFileFolder		(GSM_StateMachine *s, GSM_File *File, bool start);
GSM_Error N6510_GetFileSystemStatus		(GSM_StateMachine *s, GSM_FileSystemStatus *status);
GSM_Error N6510_GetFilePart			(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size);
GSM_Error N6510_AddFilePart			(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle);
GSM_Error N6510_DeleteFile			(GSM_StateMachine *s, unsigned char *ID);
GSM_Error N6510_AddFolder			(GSM_StateMachine *s, GSM_File *File);

GSM_Error N6510_ReplyGetFileCRC12		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplySetAttrib2			(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplySetFileDate2		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyOpenFile2			(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetFileFolderInfo1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetFileFolderInfo2		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetFileFolderListing2	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetFileSystemStatus1	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyGetFilePart12		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddFileHeader1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddFilePart1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyDeleteFile2		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyDeleteFile1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N6510_ReplyAddFolder1			(GSM_Protocol_Message msg, GSM_StateMachine *s);
