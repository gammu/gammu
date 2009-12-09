extern void RestoreSMS(int argc, char *argv[]);
extern void AddSMS(int argc, char *argv[]);
extern void BackupSMS(int argc, char *argv[]);
extern void AddNew(int argc, char *argv[]);
extern void Restore(int argc, char *argv[]);
extern void DoBackup(int argc, char *argv[]);
extern void SaveFile(int argc, char *argv[]);
extern GSM_Error ReadPhonebook(GSM_MemoryEntry **Phonebook, GSM_MemoryType MemoryType, const char *question, int max_entries);
