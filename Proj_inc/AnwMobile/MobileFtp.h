// For ASUS Manager
int WINAPI DownloadJava(char *jadfilename, char *jarfilename);
int WINAPI BackupMobile(char *filename);
int WINAPI ResotreMobile(char *filename);


typedef int (WINAPI* DownloadJavaProc)(char *jadfilename, char *jarfilename);
typedef int (WINAPI* BackupMobileProc)(char *filename);
typedef int (WINAPI* ResotreMobileProc)(char *filename);



extern DownloadJavaProc DownloadJavafn;
extern BackupMobileProc BackupMobilefn;
extern ResotreMobileProc ResotreMobilefn;