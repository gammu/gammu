#ifndef WMX_SIM

#define WMX_SIM

void simCommand_data	(unsigned char ins, unsigned char type, unsigned char *buffer, size_t length);
void simResponse_Process(unsigned char type, unsigned char *buffer, size_t length);
void simAnswer_Process	(unsigned char type, unsigned char *buffer, size_t length);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
