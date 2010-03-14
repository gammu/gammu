typedef void (*sigcallback)(char);
typedef void (*keepalive)();

int OpenConnection(char *szPort, sigcallback fn, keepalive ka);
int CloseConnection();
int WriteCommBlock(LPSTR lpByte, DWORD dwBytesToWrite);

