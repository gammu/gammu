#include <windows.h>
#include <stdio.h>
#include <unistd.h>

#define PACKET_SZ 32
int main(int argc,char *const argv[])
{
    int found=0;
    FILE *res_stream;
    FILE *stream;
    char list[PACKET_SZ];
    char stdoutfile[1024] = "";
    char returnfile[1024] = "";
    char cmd[4096] = "";
    int  i, numread, numwritten;
    sprintf(cmd, "./mingw/wine_linux_launcher.sh %s ", argv[0]);
    for(i=1; i<argc; i++)
        sprintf(cmd, "%s \"%s\"", cmd, argv[i]);
    sprintf(stdoutfile, "%s_stdout", argv[0]);
    sprintf(returnfile, "%s_return", argv[0]);
    remove(stdoutfile);
    remove(returnfile);
    system(cmd);
    while(!found){
        if( res_stream = fopen( returnfile, "r+t" ) )
        {
              found=1;
              numread=PACKET_SZ;
              stream = fopen( stdoutfile, "r+t" );
              while(numread == PACKET_SZ){
                 numread = fread( list, sizeof( char ), PACKET_SZ, stream );
                 printf( "%.*s", numread, list);
              }
              fclose(stream);
              numread = fread( list, sizeof( char ), PACKET_SZ, res_stream );
              fclose(res_stream);
              remove(stdoutfile);
              remove(returnfile);
              return atoi(list);
        } else {
            Sleep(100);
        }
    }
    return 0;
}
