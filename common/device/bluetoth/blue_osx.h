
GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device);
GSM_Error bluetooth_findchannel(GSM_StateMachine *s);
int bluetooth_read(GSM_StateMachine *s, void *buf, size_t nbytes);
int bluetooth_write(GSM_StateMachine *s, const void *buf, size_t nbytes);
GSM_Error bluetooth_close(GSM_StateMachine *s);
